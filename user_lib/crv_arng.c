/******************************************************************************
* Crv_arng.c - curve arrangement topology.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, November 2009.				      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "trim_lib.h"
#include "allocate.h"
#include "user_loc.h"

#define USER_CA_MAX_CRV_AT_VRTX		10
#define USER_CA_GET_OTHER_CRV(Crvs, Crv)   (Crvs[0] == Crv ? Crvs[1] : Crvs[0])
#define USER_CA_PLANE_FIT_ITERS	100
#define USER_CA_CCI_TOLERANCE	1e-10
#define USER_CA_COLIN_TOLERANCE	1e-10
#define USER_CA_CCI_BNDRY_TOL	1e-3


#ifdef DEBUG
#define DEBUG_PRINT_CURVE_ADJ_INFO() { int _i; \
    fprintf(stderr, "\n"); \
    for (_i = 0; _i < CA -> NumOfCrvs; _i++) { \
        fprintf(stderr, "Curve %2d (%d loops), Forward curve is %3d, backward is %3d", \
		_i, CA -> Crvs[_i].NumOfCmplxLoops, CrvsFrwd[_i], CrvsBkwd[_i]); \
	if (CA -> Crvs[_i].NumOfCmplxLoops != \
				  (CrvsFrwd[_i] >= 0) + (CrvsBkwd[_i] >= 0)) \
	    fprintf(stderr, "INCONSISTENT\n"); \
        else \
	    fprintf(stderr, "\n"); \
    } }
#define DEBUG_PRINT_NEW_REGION(n, Frwd) { \
    UserCARefCrvStruct *RefCrvs = CA -> Regions[n] -> RefCrvs; \
    fprintf(stderr, "Region %d: ", n); \
    for ( ; RefCrvs != NULL; RefCrvs = RefCrvs -> Pnext) { \
        fprintf(stderr, " %s%d", \
		RefCrvs -> Inverted ? "-" : "", RefCrvs -> RefCrv -> Idx); \
    fprintf(stderr, "\n"); \
    } }
#else
#define DEBUG_PRINT_CURVE_ADJ_INFO()
#define DEBUG_PRINT_NEW_REGION(n, Frwd)
#endif /* DEBUG_PRINT_CURVE_ADJ_INFO */

enum {
    USER_CA_PT_UNUSED = -1,
    USER_CA_PT_MERGED = -2,
    USER_CA_PT_SELFLOOP = -3
};
	        
IRIT_STATIC_DATA int
    GlblMapCrvsXYCurves = 0,
    GlblMapCrvsZOffsetCount = 0;
IRIT_STATIC_DATA IrtRType
    GlblMapCrvsZOffset = 0.0;
IRIT_STATIC_DATA IPObjectStruct
    *GlblPointOrientationList = NULL;

typedef struct UserCAPtStruct {
    int Idx;                        /* In the points' array, -1 if inactive. */
    CagdPType Pt, XYPt;
    /* Refs to curves whose end points are at this pt: */
    struct UserCACrvStruct *RefCrvs[USER_CA_MAX_CRV_AT_VRTX];
    int NumOfRefCrvs;
} UserCAPtStruct;

/* A curve's topology includes the curve and references to its 2 end points. */
typedef struct UserCACrvStruct {
    int Idx;                        /* In the curves' array, -1 if inactive. */
    int NumOfCmplxLoops;          /*# of loops this curve is in: 0, 1, or 2. */
    UserCAObjType Type;
    CagdCrvStruct *Crv, *XYCrv;	       /* Original and rotated to XY curves. */
    UserCAPtStruct *RefStartPt, *RefEndPt;
} UserCACrvStruct;

/* A reference to a curve. */
typedef struct UserCARefCrvStruct {
    struct UserCARefCrvStruct *Pnext;
    UserCACrvStruct *RefCrv;
    int Inverted;         /* If TRUE the curve is to be considered inverted. */
} UserCARefCrvStruct;

/* A regions which can consist of a single list of curves (if a simple or a  */
/* closed loop, or can consist of several such list if complex.		     */
typedef struct UserCARegionStruct {
    UserCARefCrvStruct *RefCrvs;
    UserCAObjType Type;
    CagdRType Area;
    int ContainedIn;
    struct UserCACrvRegionStruct *Pnext;
} UserCARegionStruct;

static CagdCrvStruct *UserCABreakLiNCrvsAtAngularDev(CagdCrvStruct *Crvs,
						     IrtRType AngularDeviation);
static CagdCrvStruct *UserCAMergeCrvsAtAngularDev(CagdCrvStruct *Crvs,
						  IrtRType AngularDeviation,
						  IrtRType PtPtEps);
static CagdCrvStruct *UserCrvArngmntFilterDups(CagdCrvStruct *Crvs,
					       CagdBType UpdateEndPts,
					       CagdRType EndPtEndPtTol,
					       CagdRType Eps);
static int UserCrvArngmntFilterTans(UserCrvArngmntStruct *CA,
				    CagdRType FilterTans);
static int UserCrvArngmntModifyTangent(CagdCrvStruct *Crv, int StartLocation);
static void UserCrvArngmntUpdateEndPts(CagdCrvStruct *Crvs,
				       CagdRType EndPtEndPtTol,
				       CagdCrvStruct *OldCrv,
				       CagdCrvStruct *NewCrv);
static CagdCrvStruct *UserCrvArngmntSplitAtPts(CagdCrvStruct *Crvs,
					       const IPObjectStruct *PtsObj,
					       CagdRType Eps);
static CagdCrvStruct *UserCrvArngmntLinearCrvsFitC1(CagdCrvStruct *Crvs,
						    int FitSize);
static IrtRType UserCrvArngmntFindPlane(CagdCrvStruct *Crvs,
					IrtRType PlanarityTol,
					IrtPlnType Pln);
static CagdCrvStruct *UserCrvArngmntProcessIntersections(CagdCrvStruct *Crvs,
							 CagdRType Tolerance);
static CagdCrvStruct *UserCrvArngmntProcessSpecialPts(CagdCrvStruct *Crvs,
						      CagdRType Tolerance,
						     UserCASplitType CrvSplit);
static int UserCrvArngmntPrepEval(UserCrvArngmntStruct *CA);
static UserCARegionStruct *UserCANewRegion(UserCARefCrvStruct *RefCrvs);
static UserCARefCrvStruct *UserCANewRefCrv(UserCACrvStruct *Crv);
static void UserCAReleaseRefCrvList(UserCARefCrvStruct *RefList,
				    int ResetUndef);
static UserCARefCrvStruct *UserCrvArngmntFetchSimpleRegion(
						     UserCrvArngmntStruct *CA,
						     int CrvIdx);
static int UserCrvArngmntFetchComplexRegions(UserCrvArngmntStruct *CA,
					     int IgnoreInteriorHangingCrvs);
static UserCARefCrvStruct *UserCrvArngmntCollectCmplxLoop(UserCrvArngmntStruct
								          *CA,
							  int StartCrvIdx,
							  int CrvReversed,
							  int CollectFrwd,
							  int *CrvsFrwd,
							  int *CrvsBkwd);
static int UserCrvArngmntFindFrwdBkwdCrv(const UserCrvArngmntStruct *CA,
					 int CrvIdx,
					 int DoForward,
					 int IgnoreInteriorHangingCrvs);
static UserCARefCrvStruct *UserCrvArngmntReverseRefCrvList(UserCARefCrvStruct
							         *CrvRefList);
static int UserCrvArngmntClassifyContainments(const UserCrvArngmntStruct *CA);
static const CagdCrvStruct *UserCrvArngmntFindUnusedCurve(
					       const UserCrvArngmntStruct *CA,
					       int Rgn1,
					       int Rgn2);
static IPObjectStruct *UserCrvArngmntRegion2Curves(const UserCrvArngmntStruct
						                          *CA,
						   UserCARefCrvStruct
								    *CARefCrv,
						   int Merge);
static void MapCrvsInPlace(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void EvalOrientationPoint(IPObjectStruct *PObj, IrtHmgnMatType XYMat);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Main entry points of CA.  gets the desired operation to perform, the CA  M
* to operate on (or NULL if new one) and parameters that depends on the      M
* specific desired operation:                                                M
*	 								     M
*   USER_CA_OPER_CREATE - Creates a new CA				     V
*       Params[0] = List of curves/polylines/trimed surfaces to extract the  V
*                   curves/linear curves/trimming curves for the arrangment. V
*       Params[1] = Tolerance for considering end points equal.		     V
*       Params[2] = Planarity tolerance to consider arrangement planar.	     V
*       Params[3] = TRUE to project all curves to be on computed plane.	     V
*       Params[4] = Mask for input type to consider:                         V
*                   0x01 to handle polylines.				     V
*                   0x02 to handle curves.				     V
*                   0x04 to handle trimming curves in trimmed surfaces.      V
*   USER_CA_OPER_COPY - Creates a new CA				     V
*       None.								     V
*   USER_CA_OPER_FILTER_DUP - Creates a new CA				     V
*       Params[0] = Epsilon to consider the curves the same.		     V
*       Params[1] = TRUE to update end points to be the same.		     V
*   USER_CA_OPER_FILTER_TAN - Creates a new CA				     V
*       Params[0] = Epsilon angle in degrees to consider two curves with     V
*                   the same tangent.					     V
*   USER_CA_OPER_SPLIT_CRV - Creates a new CA				     V
*       Params[0] = Mask for splitting type to consider:                     V
*                   USER_CA_SPLIT_INFLECTION_PTS to split at inflection pts. V
*                   USER_CA_SPLIT_MAX_CRVTR_PTS to split at max curvatures.  V
*                   USER_CA_SPLIT_C1DISCONT_PTS to split at C1 disconts.     V
*       Params[1] = Tolerance of splitting computation.			     V
*   USER_CA_OPER_BREAK_LIN - Creates a new CA				     V
*       Params[0] = Angular deviation (in degrees) to split linear curves at.V
*   USER_CA_OPER_BREAK_INTER - Creates a new CA				     V
*       Params[0] = Intersection computation tolerance.			     V
*   USER_CA_OPER_BREAK_NEAR_PTS - Creates a new CA			     V
*       Params[0] = Number of points to split curves at.		     V
*       Params[1] = A list object of pts to examine and slit if near them.   V
*       Params[2] = Tolerance to consider a point near/on a curve.	     V
*   USER_CA_OPER_UNION_CRV - Creates a new CA				     V
*       Params[0] = Angular deviation (in degrees) to merge C1 discont       V
*                   curves at.						     V
*   USER_CA_OPER_LSTSQR_CRV - Creates a new CA				     V
*       Params[0] = Fitting Parameter to fit smooth quadratic C1 curves to   V
*                   linear curves.  Higher order curves are not affected.    V
*                       If Params[0] positive, the fitted curve size is set  V
*                   to InputCrvSize * FitC1Crv / 100 (i.e. Params[0] serves  V
*                   as percetange of input size.			     V
*                       If Params[0] negative, the Fitted curve size is      V
*                   simply set to ABS(Params[0]).			     V
*   USER_CA_OPER_EVAL_CA - Operates on input CA in place		     V
*       Params[0] = TRUE to ignore hanging curves that join other curves at  V
*		    only one of their end points.			     V
*   USER_CA_OPER_CLASSIFY - Operates on input CA in place		     V
*	None.								     V
*   USER_CA_OPER_REPORT - Operates on input CA in place			     V
*       Params[0] = A mask of desired report:				     V
*		    0x01 to dump info on crvs. 0x02 to also dump the crvs.   V
*		    0x04 to report end pts in arrangment if evaluated.	     V
*		    0x08 to report regions in arrangment if evaluated.       V
*   USER_CA_OPER_OUTPUT - Operates on input CA in place 		     V
*       Params[0] = Style of expected output:				     V
*                   1 for individual crv segs in each region (loop etc.),    V
*                   2 for merged curves so every region is one curve,        V
*                   3 for topology as an ordered list of curve segments and  V
*                     each region is a list of indices into the first list.  V
*                     A negative -i index means index i but a reversed crv.  V
*                   101, 102, 103: same as 1,2,3 but pt is evaluated at 1/13 V
*                     of curve parameteric domain to identify orientation.   V
*       Params[1] = Tolerance of topology reconstruction (in case 3 only).   V
*       Params[2] = Zoffset in Z for the i'th region, by amount i*ZOffset.   V
*   USER_CA_OPER_FREE - Operates on input CA in place			     V
*	None.								     V
*                                                                            *
* PARAMETERS:                                                                M
*   Operation:   Create, BreakCrv, Report, Free, etc. (See UserCAOpType).    M
*   CA:          To operate on its copy or NULL if a new CA.                 M
*   Params:      An array of params dependening on Operation.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserCrvArngmntStruct *:   Constructed or updated CA.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmnt                                                           M
*****************************************************************************/
UserCrvArngmntStruct *UserCrvArngmnt(UserCAOpType Operation,
				     const UserCrvArngmntStruct *CA,
				     const void *Params[])
{
    UserCASplitType CrvSplit;
    int ProjectOnPlane, InputMaskType, FitC1Info, UpdateEndPts,
        OutputStyle, IgnoreInteriorHangingCrvs, ReportInfo;
    CagdRType PlanarityTol, EndPtEndPtTol, SplitTol, InterTol,
        ZOffset, Tolerance, FilterDupEps, FilterTanEps, AngularDeviation,
        PtsTol;
    const IPObjectStruct *PCrvs, *PtsObj;
    UserCrvArngmntStruct
        *NewCA = NULL;

    switch (Operation) {
	case USER_CA_OPER_CREATE:
	    /* Create a new CA. */
	    PCrvs = (const IPObjectStruct *) Params[0];
            EndPtEndPtTol = *((const CagdRType *) Params[1]);
            PlanarityTol = *((const CagdRType *) Params[2]);
            ProjectOnPlane = (int) *((const CagdRType *) Params[3]);
            InputMaskType = (int) *((const CagdRType *) Params[4]);

	    if ((NewCA = UserCrvArngmntCreate(PCrvs, EndPtEndPtTol,
					      PlanarityTol, ProjectOnPlane,
					      InputMaskType)) != NULL) {
	    }
	    break;
	case USER_CA_OPER_COPY:
	    NewCA = UserCrvArngmntCopy(CA);
	    break;
	case USER_CA_OPER_FILTER_DUP:
	    NewCA = UserCrvArngmntCopy(CA);
            FilterDupEps = *((const CagdRType *) Params[0]);
            UpdateEndPts = (int) *((const CagdRType *) Params[1]);
	    if (FilterDupEps > 0.0)
	        NewCA -> CagdCrvs =
		    UserCrvArngmntFilterDups(NewCA -> CagdCrvs,
					     UpdateEndPts,
					     NewCA -> EndPtEndPtTol,
					     FilterDupEps);
	    break;
	case USER_CA_OPER_FILTER_TAN:
	    NewCA = UserCrvArngmntCopy(CA);
            FilterTanEps = *((const CagdRType *) Params[0]);
	    if (FilterTanEps > 0.0)
	        UserCrvArngmntFilterTans(NewCA, FilterTanEps);
	    break;
	case USER_CA_OPER_SPLIT_CRV:
	    NewCA = UserCrvArngmntCopy(CA);
            CrvSplit = (UserCASplitType) *((const CagdRType *) Params[0]);
            SplitTol = *((const CagdRType *) Params[1]);
	    NewCA -> CagdCrvs =
	        UserCrvArngmntProcessSpecialPts(NewCA -> CagdCrvs,
						SplitTol,
						CrvSplit);
	    break;
	case USER_CA_OPER_BREAK_INTER:
	    NewCA = UserCrvArngmntCopy(CA);
            InterTol = *((const CagdRType *) Params[0]);
	    NewCA -> CagdCrvs =
	        UserCrvArngmntProcessIntersections(NewCA -> CagdCrvs,
						   InterTol);
	    break;
	case USER_CA_OPER_BREAK_LIN:
	    NewCA = UserCrvArngmntCopy(CA);
            AngularDeviation = *((const CagdRType *) Params[0]);
	    NewCA -> CagdCrvs =
	        UserCABreakLiNCrvsAtAngularDev(NewCA -> CagdCrvs,
					       AngularDeviation);
	    break;
	case USER_CA_OPER_BREAK_NEAR_PTS:
	    NewCA = UserCrvArngmntCopy(CA);
            PtsObj = (const IPObjectStruct *) Params[0];
            PtsTol = *((const CagdRType *) Params[1]);
	    if ((NewCA -> CagdCrvs =
		 UserCrvArngmntSplitAtPts(NewCA -> CagdCrvs, PtsObj,
					  PtsTol)) == NULL)
	        NewCA -> Error = "Possibly invalid input list of points";
	    break;
	case USER_CA_OPER_UNION_CRV:
	    NewCA = UserCrvArngmntCopy(CA);
            AngularDeviation = *((const CagdRType *) Params[0]);
	    NewCA -> CagdCrvs =
	        UserCAMergeCrvsAtAngularDev(NewCA -> CagdCrvs,
					    AngularDeviation,
					    NewCA -> EndPtEndPtTol);
	    break;
	case USER_CA_OPER_LSTSQR_CRV:
	    NewCA = UserCrvArngmntCopy(CA);
	    FitC1Info = (int) *((const CagdRType *) Params[0]);
	    NewCA -> CagdCrvs =
	        UserCrvArngmntLinearCrvsFitC1(NewCA -> CagdCrvs,
					      FitC1Info);
	    break;
	case USER_CA_OPER_EVAL_CA:
	    NewCA = UserCrvArngmntCopy(CA);
            IgnoreInteriorHangingCrvs = (int) *((const CagdRType *) Params[0]);
	    /* The arrangment evaluation computation: */
	    UserCrvArngmntPrepEval(NewCA);
	    UserCrvArngmntProcessEndPts(NewCA);
	    UserCrvArngmntClassifyConnectedRegions(NewCA,
						   IgnoreInteriorHangingCrvs);
	    break;
	case USER_CA_OPER_CLASSIFY:
	    /* Classy containment of each region in other regions. */
	    UserCrvArngmntClassifyContainments(CA);
	    break;
	case USER_CA_OPER_REPORT:
            ReportInfo = (int) *((const CagdRType *) Params[0]);
	    UserCrvArngmntReport(CA, ReportInfo & 0x03, ReportInfo & 0x04,
				 ReportInfo & 0x08, (ReportInfo & 0x100) == 0);
	    break;
	case USER_CA_OPER_OUTPUT:
	    /* Note output operates on the orignal CA. */
	    NewCA = UserCrvArngmntCopy(CA);
            OutputStyle = (int) *((const CagdRType *) Params[0]);
            Tolerance = *((const CagdRType *) Params[1]);
            ZOffset = *((const CagdRType *) Params[2]);
	    UserCrvArngmntOutput(CA, OutputStyle, Tolerance, ZOffset);
	    NewCA -> Output = CA -> Output;
	    ((UserCrvArngmntStruct *) CA) -> Output = NULL;
	    break;
	case USER_CA_OPER_FREE:
	    /* While CA is declared constant we allow freeing it... */
	    UserCrvArngmntFree((UserCrvArngmntStruct *) CA);
	    break;
	case USER_CA_OPER_NONE:
        default:
	    break;
    }

    if (NewCA != NULL)
        NewCA -> NumOfCrvs = CagdListLength(NewCA -> CagdCrvs);

    return NewCA;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a new curves' arrangement data structure.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrvs:             Input to derive its planar arrangement. Can be        M
*		       curves, polylines, or trimming curves in trimmed      M
*                      surfaces.				             M
*   EndPtEndPtTol:     Tolerance to consider two (end) points the same.      M
*   PlanarityTol:      Tolerance to accept all curves as planar.             M
*   ProjectOnPlane:    TRUE to project off-plane curves onto the plane.      M
*   InputMaskType:     Bit mask controlling the type of entities to process: M
*                         0x01 - process polylines in the input.             M
*                         0x02 - process curves in the input.                M
*                         0x04 - process trimming curves in trimmed surfaces M
*                                in the input.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserCrvArngmntStruct *: A curves' arrangement structure, or NULL if err. M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntCreate                                                     M
*****************************************************************************/
UserCrvArngmntStruct *UserCrvArngmntCreate(const IPObjectStruct *PCrvs,
					   CagdRType EndPtEndPtTol,
					   CagdRType PlanarityTol,
					   int ProjectOnPlane,
					   int InputMaskType)
{
    int j, NumOfOrigCrvs;
    CagdRType InternalTol;
    IrtPlnType Pln;
    IrtHmgnMatType Mat, InvMat;
    CagdCrvStruct *Crvs, *Crv;
    UserCrvArngmntStruct *CA;
    CagdBBoxStruct BBox;

    if (IP_IS_CRV_OBJ(PCrvs)) {
        Crvs = CagdCrvCopyList(PCrvs -> U.Crvs);
    }
    else if (IP_IS_OLST_OBJ(PCrvs)) {
        const IPObjectStruct *PTmp;

	Crvs = NULL;
	for (PTmp = IPListObjectGet(PCrvs, j = 0);
	     PTmp != NULL;
	     PTmp = IPListObjectGet(PCrvs, ++j)) {
	    if (IP_IS_CRV_OBJ(PTmp)) {
	        if (InputMaskType & 0x02)
		    Crvs = CagdListAppend(CagdCrvCopyList(PTmp -> U.Crvs),
					  Crvs);
	    }
	    else if (IP_IS_POLY_OBJ(PTmp) && IP_IS_POLYLINE_OBJ(PTmp)) {
	        if (InputMaskType & 0x01)
		    Crvs = CagdListAppend(
			     UserPolylines2LinBsplineCrvs(PTmp -> U.Pl, TRUE),
			     Crvs);
	    }
	    else if (IP_IS_TRIMSRF_OBJ(PTmp)) {
	        /* Extract the trimming curves and use them instead. */
	        if (InputMaskType & 0x04)
		    Crvs = CagdListAppend(
			      TrimGetTrimmingCurves(PTmp -> U.TrimSrfs,
						    FALSE, TRUE),
			      Crvs);
	    }
	    else {
	        /* Skip. */
	    }
	}
    }
    else
	Crvs = NULL;

    if (Crvs == NULL)
	return NULL;

    NumOfOrigCrvs = CagdListLength(Crvs);

    CagdCrvListBBox(Crvs, &BBox);
    InternalTol = IRIT_MAX(BBox.Max[0] - BBox.Min[0],
		           BBox.Max[1] - BBox.Min[1]) * USER_CA_CCI_TOLERANCE;

    /* Compute the plane holding all these curves. */
    if (UserCrvArngmntFindPlane(Crvs, PlanarityTol, Pln) > PlanarityTol) {
        return NULL;
    }
    GMGenMatrixZ2Dir(Mat, Pln);
    MatInverseMatrix(Mat, InvMat);

    /* Add an index to all curves so we can identify the source of each    */
    /* curve segment, even after they are split.			   */
    for (Crv = Crvs, j = 0; Crv != NULL; Crv = Crv -> Pnext, j++) {
        AttrSetIntAttrib(&Crv -> Attr, "CAIndex", j);
    }

    CA = (UserCrvArngmntStruct *) IritMalloc(sizeof(UserCrvArngmntStruct));

    /* Allocate and initialize the curves' vector. */
    CA -> NumOfOrigCrvs = NumOfOrigCrvs;
    CA -> NumOfCrvs = CagdListLength(Crvs);
    CA -> CagdCrvs = Crvs;
    CA -> InternalTol = InternalTol;
    CA -> AllocSizeCrvs = CA -> NumOfCrvs * 2;
    IRIT_HMGN_MAT_COPY(CA -> XYZ2XYMat, InvMat);
    IRIT_HMGN_MAT_COPY(CA -> XY2XYZMat, Mat);
    IRIT_PLANE_COPY(CA -> CrvsPlane, Pln);

    CA -> Pts = NULL;
    CA -> Crvs = NULL;
    CA -> Regions = NULL;
    CA -> NumOfPts = 0;
    CA -> NumOfCrvs = 0;
    CA -> NumOfRegions = 0;
    CA -> AllocSizeCrvs = 0;
    CA -> AllocSizePts = 0;

    CA -> Output = NULL;
    CA -> EndPtEndPtTol = EndPtEndPtTol;
    CA -> PlanarityTol = PlanarityTol;
    CA -> ProjectOnPlane = ProjectOnPlane;
    CA -> Error = NULL;

    return CA;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merge given curves that share an end point if their angular deviation is *
* smaller than AngularDeviation.	                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:             Input curves.  Updated in place.			     *
*   AngularDeviation: Maximal angular deviation to allow, in degrees.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  The input curves that are merged in place.             *
*****************************************************************************/
static CagdCrvStruct *UserCAMergeCrvsAtAngularDev(CagdCrvStruct *Crvs,
						  IrtRType AngularDeviation,
						  IrtRType PtPtEps)
{
    CagdCrvStruct *Crv1, *Crv2, *TCrv, *TCrv2,
        *NewCrvs = NULL;

    AngularDeviation = cos(IRIT_DEG2RAD(AngularDeviation));

    while (Crvs != NULL) {
        int CAIdx1, CAIdx2;
        CagdRType *R, TMin, TMax;
	CagdPType Pt1Start, Pt1End, Pt2Start, Pt2End;
	CagdVecStruct Tn1Start, Tn1End, Tn2Start, Tn2End;

        IRIT_LIST_POP(Crv1, Crvs);
	if (AttrGetIntAttrib(Crv1 -> Attr, "_Used") == TRUE) {
	    CagdCrvFree(Crv1);
	    continue;
	}

	/* Evalues end points and tangents. */
	CagdCrvDomain(Crv1, &TMin, &TMax);
	CAIdx1 = AttrGetIntAttrib(Crv1 -> Attr, "CAIndex");

	R = CagdCrvEval(Crv1, TMin);
	CagdCoerceToE3(Pt1Start, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv1, TMax);
	CagdCoerceToE3(Pt1End, &R, -1, Crv1 -> PType);

	Tn1Start = *CagdCrvTangent(Crv1, TMin, TRUE);
	Tn1End = *CagdCrvTangent(Crv1, TMax, TRUE);
	
	/* Go over rest of curves and seek curves with similar end pt. */
	for (Crv2 = Crvs; Crv2 != NULL; ) {
	    int Merged;

	    if (AttrGetIntAttrib(Crv2 -> Attr, "_Used") == TRUE)
	        continue;

	    /* Evalues end points and tangents. */
	    CagdCrvDomain(Crv2, &TMin, &TMax);
	    CAIdx2 = AttrGetIntAttrib(Crv2 -> Attr, "CAIndex");

	    R = CagdCrvEval(Crv2, TMin);
	    CagdCoerceToE3(Pt2Start, &R, -1, Crv2 -> PType);
	    R = CagdCrvEval(Crv2, TMax);
	    CagdCoerceToE3(Pt2End, &R, -1, Crv2 -> PType);

	    Tn2Start = *CagdCrvTangent(Crv2, TMin, TRUE);
	    Tn2End = *CagdCrvTangent(Crv2, TMax, TRUE);

	    if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2Start, PtPtEps) &&
		-IRIT_DOT_PROD(Tn1Start.Vec, Tn2Start.Vec) > AngularDeviation) {
	        /* Merge start of one curve with start of 2nd curve. */
	        Merged = TRUE;
		TCrv = CagdCrvReverse(Crv1);
		CagdCrvFree(Crv1);
		Crv1 = CagdMergeCrvCrv(TCrv, Crv2, FALSE);
	    }
	    else if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2End, PtPtEps) &&
		     IRIT_DOT_PROD(Tn1Start.Vec, Tn2End.Vec) > AngularDeviation) {
	        /* Merge start of one curve with end of 2nd curve. */
	        Merged = TRUE;
		TCrv = CagdMergeCrvCrv(Crv2, Crv1, FALSE);
		CagdCrvFree(Crv1);
		Crv1 = TCrv;
	    }		
	    else if (IRIT_PT_APX_EQ_EPS(Pt1End, Pt2Start, PtPtEps) &&
		     IRIT_DOT_PROD(Tn1End.Vec, Tn2Start.Vec) > AngularDeviation) {
	        /* Merge end of one curve with start of 2nd curve. */
	        Merged = TRUE;
		TCrv = CagdMergeCrvCrv(Crv1, Crv2, FALSE);
		CagdCrvFree(Crv1);
		Crv1 = TCrv;
	    }
	    else if (IRIT_PT_APX_EQ_EPS(Pt1End, Pt2End, PtPtEps) &&
		     -IRIT_DOT_PROD(Tn1End.Vec, Tn2End.Vec) > AngularDeviation) {
	        /* Merge end of one curve with start of 2nd curve. */
	        Merged = TRUE;
		TCrv = CagdCrvReverse(Crv2);
		TCrv2 = CagdMergeCrvCrv(Crv1, TCrv, FALSE);
		CagdCrvFree(TCrv);
		CagdCrvFree(Crv1);
		Crv1 = TCrv2;
	    }
	    else
	        Merged = FALSE;

	    if (Merged) {
	        if (CAIdx1 == CAIdx2)
		    AttrSetIntAttrib(&Crv1 -> Attr, "CAIndex", CAIdx1);

	        AttrSetIntAttrib(&Crv2 -> Attr, "_Used", TRUE);
		Crv2 = Crvs;      /* Restart the search from the beginning. */
	    }
	    else
	        Crv2 = Crv2 -> Pnext;
	}

	if (Crv1 != NULL) {
	    IRIT_LIST_PUSH(Crv1, NewCrvs);
	}
    }

    return CagdListReverse(NewCrvs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Break given linear curves at any C1 discont. that is showing an angular  *
* deviation larger than AngularDeviation.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:             Input curves.  Updated in place.			     *
*   AngularDeviation: Maximal angular deviation to allow, in degrees.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  The input curves that are split in place.              *
*****************************************************************************/
static CagdCrvStruct *UserCABreakLiNCrvsAtAngularDev(CagdCrvStruct *Crvs,
						     IrtRType AngularDeviation)
{
    CagdCrvStruct *Crv, *TCrvs,
        *NewCrvs = NULL;

    AngularDeviation = cos(IRIT_DEG2RAD(AngularDeviation));

    while (Crvs != NULL) {
        int i, n, Proximity;
        CagdRType *C1Disconts;
	CagdPtStruct *Pt,
	    *Pts = NULL;

        IRIT_LIST_POP(Crv, Crvs);

	if ((C1Disconts = BspKnotAllC1Discont(Crv -> KnotVector,
					      Crv -> Order,
					      Crv -> Length, &n)) == NULL) {
	    IRIT_LIST_PUSH(Crv, NewCrvs);
	    continue;
	}

	for (i = 0; i < n; i++) {
	    CagdVecStruct V1, V2;

	    V1 = *CagdCrvTangent(Crv, C1Disconts[i] - IRIT_EPS, TRUE);
	    V2 = *CagdCrvTangent(Crv, C1Disconts[i] + IRIT_EPS, TRUE);
	    if (IRIT_DOT_PROD(V1.Vec, V2.Vec) < AngularDeviation) {
	        Pt = CagdPtNew();
		Pt -> Pt[0] = C1Disconts[i];
		IRIT_LIST_PUSH(Pt, Pts);
	    }
	}

	if (Pts != NULL) {
	    /* Pts are reversed - reorder them. */
	    Pts = CagdListReverse(Pts);
	    TCrvs = CagdCrvSubdivAtParams(Crv, Pts, IRIT_EPS,
					  &Proximity);

	    CagdPtFreeList(Pts);
	    NewCrvs = CagdListAppend(TCrvs, NewCrvs);
	    CagdCrvFree(Crv);
	}
	else
	    IRIT_LIST_PUSH(Crv, NewCrvs);

	IritFree(C1Disconts);
    }

    return CagdListReverse(NewCrvs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Filters out curves are are duplicated, leaving only one instance.        *
*   Also seeks partial overlaps and filter such cases as well.               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:           Input curves to filter (partial) duplicates, in place.   *
*   UpdateEndPts:   TRUE to update end points at purged segemnts to the      *
*                   precise position of the retained segment.		     *
*   EndPtEndPtTol:  Tolerance to consider two (end) points the same.         *
*   Eps:      To consider curves same and Pt-Crv distances to consider same. *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Filtered curves, in place.		             *
*****************************************************************************/
static CagdCrvStruct *UserCrvArngmntFilterDups(CagdCrvStruct *Crvs,
					       CagdBType UpdateEndPts,
					       CagdRType EndPtEndPtTol,
					       CagdRType Eps)
{
    int CAIdx1, CAIdx2;
    CagdCrvStruct *Crv, *TCrv, *RCrv,
	*NewCrvs = NULL;

    /* First step - compare all curves against existing curves and purge    */
    /* curves that are are found already existing.			    */
    while (Crvs != NULL) {
        CagdBType
	    Reversed = FALSE;
        CagdRType TMin, TMax, *R;
	CagdPType CrvEnd1, CrvEnd2;

        IRIT_LIST_POP(Crv, Crvs);
	CAIdx1 = AttrGetIntAttrib(Crv -> Attr, "CAIndex");
	RCrv = CagdCrvReverse(Crv);

	CagdCrvDomain(Crv, &TMin, &TMax);
	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE3(CrvEnd1, &R, -1, Crv -> PType);
	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE3(CrvEnd2, &R, -1, Crv -> PType);

	for (TCrv = NewCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
	    int AreSame = FALSE;
	    CagdPType TCrvEnd1, TCrvEnd2;

	    /* Do not filter out a crv if both from the same original crv. */
	    CAIdx2 = AttrGetIntAttrib(TCrv -> Attr, "CAIndex");
	    if (CAIdx1 == CAIdx2)
	        continue;

	    CagdCrvDomain(TCrv, &TMin, &TMax);
	    R = CagdCrvEval(TCrv, TMin);
	    CagdCoerceToE3(TCrvEnd1, &R, -1, TCrv -> PType);
	    R = CagdCrvEval(TCrv, TMax);
	    CagdCoerceToE3(TCrvEnd2, &R, -1, TCrv -> PType);

	    if (IRIT_PT_APX_EQ_EPS(CrvEnd1, TCrvEnd1, Eps) &&
		IRIT_PT_APX_EQ_EPS(CrvEnd2, TCrvEnd2, Eps)) {
	        CagdCrvStruct *SameCrv, *SameTCrv;

	        /* End points are the same. Compare the curves after        */
	        /* bringing them to a common space.                         */
		SameCrv = CagdCrvCopy(Crv);
		SameTCrv = CagdCrvCopy(TCrv);
		AreSame = CagdMakeCrvsCompatible(&SameCrv, &SameTCrv,
						 TRUE, TRUE) &&
		          CagdCrvsSame(SameCrv, SameTCrv, Eps);
		CagdCrvFree(SameCrv);
		CagdCrvFree(SameTCrv);
		Reversed = FALSE;
	    }
	    else if (IRIT_PT_APX_EQ_EPS(CrvEnd1, TCrvEnd2, Eps) &&
		     IRIT_PT_APX_EQ_EPS(CrvEnd2, TCrvEnd1, Eps)) {
	        CagdCrvStruct *SameRCrv, *SameTCrv;

	        /* End points are the same (but in reverse). Compare the    */
	        /* curves after bringing them to a common space.            */
		SameRCrv = CagdCrvCopy(RCrv);
		SameTCrv = CagdCrvCopy(TCrv);
		AreSame = CagdMakeCrvsCompatible(&SameRCrv, &SameTCrv,
						 TRUE, TRUE) &&
		          CagdCrvsSame(SameRCrv, SameTCrv, Eps);
		CagdCrvFree(SameRCrv);
		CagdCrvFree(SameTCrv);
		Reversed = TRUE;
	    }

	    if (AreSame)
	        break;
	}

	if (TCrv == NULL) {
	    IRIT_LIST_PUSH(Crv, NewCrvs);
	}
	else {
	    if (UpdateEndPts)
	        UserCrvArngmntUpdateEndPts(Crvs, EndPtEndPtTol,
					   Reversed ? RCrv : Crv, TCrv);
	    CagdCrvFree(Crv);
	}

	CagdCrvFree(RCrv);
    }

    return NewCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans all shared end points and modify end curves so no two curves will  *
* be tangent at the end points.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:		The curve arrangement structure to modify tangencies.        *
*   FilterTans: Tolerance in degrees of two angles to be the same (tangent). *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if successful, FALSE otherwise.                            *
*****************************************************************************/
static int UserCrvArngmntFilterTans(UserCrvArngmntStruct *CA,
				    CagdRType FilterTans)
{
    CagdRType
        PtPtEps = CA -> EndPtEndPtTol,
        AngularTol = cos(IRIT_DEG2RAD(FilterTans));
    CagdCrvStruct *Crv1, *Crv2,
        *Crvs = CA -> CagdCrvs;

    for (Crv1 = Crvs; Crv1 != NULL; Crv1 = Crv1 -> Pnext) {
        CagdRType *R, TMin1, TMax1, TMin2, TMax2;
        CagdPType Pt1Start, Pt1End, Pt2Start, Pt2End;
	CagdVecStruct Tn1Start, Tn1End, Tn2Start, Tn2End;

	CagdCrvDomain(Crv1, &TMin1, &TMax1);
	R = CagdCrvEval(Crv1, TMin1);
	CagdCoerceToE3(Pt1Start, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv1, TMax1);
	CagdCoerceToE3(Pt1End, &R, -1, Crv1 -> PType);

	Tn1Start = *CagdCrvTangent(Crv1, TMin1, TRUE);
	Tn1End = *CagdCrvTangent(Crv1, TMax1, TRUE);

        for (Crv2 = Crv1 -> Pnext; Crv2 != NULL; Crv2 = Crv2 -> Pnext) {
	    CagdCrvDomain(Crv2, &TMin2, &TMax2);
	    R = CagdCrvEval(Crv2, TMin2);
	    CagdCoerceToE3(Pt2Start, &R, -1, Crv2 -> PType);
	    R = CagdCrvEval(Crv2, TMax2);
	    CagdCoerceToE3(Pt2End, &R, -1, Crv2 -> PType);

	    Tn2Start = *CagdCrvTangent(Crv2, TMin2, TRUE);
	    Tn2End = *CagdCrvTangent(Crv2, TMax2, TRUE);

	    if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2Start, PtPtEps) &&
		IRIT_DOT_PROD(Tn1Start.Vec, Tn2Start.Vec) > AngularTol) {
	        /* Modify the tangent at this location. */
	        UserCrvArngmntModifyTangent(Crv1, TRUE);
	        UserCrvArngmntModifyTangent(Crv2, TRUE);
	    }
	    else if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2End, PtPtEps) &&
		     -IRIT_DOT_PROD(Tn1Start.Vec, Tn2End.Vec) > AngularTol) {
	        /* Modify the tangent at this location. */
	        UserCrvArngmntModifyTangent(Crv1, TRUE);
	        UserCrvArngmntModifyTangent(Crv2, FALSE);
	    }		
	    else if (IRIT_PT_APX_EQ_EPS(Pt1End, Pt2Start, PtPtEps) &&
		     -IRIT_DOT_PROD(Tn1End.Vec, Tn2Start.Vec) > AngularTol) {
	        /* Modify the tangent at this location. */
	        UserCrvArngmntModifyTangent(Crv1, FALSE);
	        UserCrvArngmntModifyTangent(Crv2, TRUE);
	    }
	    else if (IRIT_PT_APX_EQ_EPS(Pt1End, Pt2End, PtPtEps) &&
		     IRIT_DOT_PROD(Tn1End.Vec, Tn2End.Vec) > AngularTol) {
	        /* Modify the tangent at this location. */
	        UserCrvArngmntModifyTangent(Crv1, FALSE);
	        UserCrvArngmntModifyTangent(Crv2, FALSE);
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Modify the tangent at the end point of the curve.                        *
*   Compute a vector from 2nd ctlpt to the shortest point on the line from   *
* 1st ctlpt to 3rd ctlpt and move 2nd ctlpt half distance along that vector. *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:           Curve to modify its tangent at one of its end points.     *
*   StartLocation: TRUE to modify start location, FALSE for end location.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if tangent was modified, FALSE otherwise.                    *
*****************************************************************************/
static int UserCrvArngmntModifyTangent(CagdCrvStruct *Crv, int StartLocation)
{
    int Idx,
	Len = Crv -> Length;
    CagdRType **Points, *Pt1, *Pt2, *Pt3;
    CagdPType ClosestPoint;
    CagdVType Dir;
    CagdPolylineStruct
        *Pl = CagdCrv2CtrlPoly(Crv);

    if (Len <= 2)
        return FALSE;

    if (StartLocation) {
        Pt1 = Pl -> Polyline[0].Pt;
        Pt2 = Pl -> Polyline[1].Pt;
        Pt3 = Pl -> Polyline[2].Pt;
	Idx = 1;
    }
    else {
        Pt1 = Pl -> Polyline[Len - 1].Pt;
        Pt2 = Pl -> Polyline[Len - 2].Pt;
        Pt3 = Pl -> Polyline[Len - 3].Pt;
	Idx = Len - 2;
    }

    IRIT_VEC_SUB(Dir, Pt3, Pt1);
    GMPointFromPointLine(Pt2, Pt1, Dir, ClosestPoint);	
    IRIT_VEC_SUB(Dir, ClosestPoint, Pt2);
    IRIT_VEC_SCALE(Dir, 0.5);
    if (IRIT_PT_APX_EQ_ZERO_EPS(Dir, IRIT_EPS))
        return FALSE;

    Points = Crv -> Points;
    /* Add Dir to second control point. */
    if (CAGD_IS_RATIONAL_CRV(Crv)) {
        Points[1][Idx] = (Pt2[0] + Dir[0]) * Points[0][Idx];
	Points[2][Idx] = (Pt2[1] + Dir[1]) * Points[0][Idx];
    }
    else {
        Points[1][Idx] = Pt2[0] + Dir[0];
	Points[2][Idx] = Pt2[1] + Dir[1];
    }

    CagdPolylineFree(Pl);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Go over all curves in Crvs, and move end points similar to OldCrv to     *
* end poinst similar to New Crv.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:   Input curves to process and update end points, in place.         *
*   EndPtEndPtTol:  Tolerance to consider two (end) points the same.         *
*   OldCrv: Curve to be removed from list of curves.  Similar to NewCrv.     *
*   NewCrv: New curve to set old points similar to OldCrv end points to it.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserCrvArngmntUpdateEndPts(CagdCrvStruct *Crvs,
				       CagdRType EndPtEndPtTol,
				       CagdCrvStruct *OldCrv,
				       CagdCrvStruct *NewCrv)
{
    CagdRType *R, TMin, TMax;
    CagdPType StrtPt, EndPt, OldStrtPt, OldEndPt, NewStrtPt, NewEndPt;
    CagdCrvStruct *Crv;

    CagdCrvDomain(OldCrv, &TMin, &TMax);
    R = CagdCrvEval(OldCrv, TMin);
    CagdCoerceToE3(OldStrtPt, &R, -1, OldCrv -> PType);
    R = CagdCrvEval(OldCrv, TMax);
    CagdCoerceToE3(OldEndPt, &R, -1, OldCrv -> PType);

    CagdCrvDomain(NewCrv, &TMin, &TMax);
    R = CagdCrvEval(NewCrv, TMin);
    CagdCoerceToE3(NewStrtPt, &R, -1, NewCrv -> PType);
    R = CagdCrvEval(NewCrv, TMax);
    CagdCoerceToE3(NewEndPt, &R, -1, NewCrv -> PType);

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdCrvDomain(Crv, &TMin, &TMax);
	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE3(StrtPt, &R, -1, OldCrv -> PType);
	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE3(EndPt, &R, -1, OldCrv -> PType);

	if (IRIT_PT_APX_EQ_EPS(OldStrtPt, StrtPt, EndPtEndPtTol)) {
	    /* Update curve's start point with NewStrtPt. */
	    switch (Crv -> PType) {
	         case CAGD_PT_E3_TYPE:
		     Crv -> Points[3][0] = NewStrtPt[2];
	         case CAGD_PT_E2_TYPE:
		     Crv -> Points[2][0] = NewStrtPt[1];
		     Crv -> Points[1][0] = NewStrtPt[0];
		     break;
	         case CAGD_PT_P3_TYPE:
		     Crv -> Points[3][0] = NewStrtPt[2] * Crv -> Points[0][0];
	         case CAGD_PT_P2_TYPE:
		     Crv -> Points[2][0] = NewStrtPt[1] * Crv -> Points[0][0];
		     Crv -> Points[1][0] = NewStrtPt[0] * Crv -> Points[0][0];
		     break;
	         default:
		     assert(0);
	    }
	}
	if (IRIT_PT_APX_EQ_EPS(OldEndPt, EndPt, EndPtEndPtTol)) {
	    int l = Crv -> Length - 1;

	    /* Update curve's end point with NewEndPt. */
	    switch (Crv -> PType) {
	         case CAGD_PT_E3_TYPE:
		     Crv -> Points[3][l] = NewEndPt[2];
	         case CAGD_PT_E2_TYPE:
		     Crv -> Points[2][l] = NewEndPt[1];
		     Crv -> Points[1][l] = NewEndPt[0];
		     break;
	         case CAGD_PT_P3_TYPE:
		     Crv -> Points[3][l] = NewEndPt[2] * Crv -> Points[0][l];
	         case CAGD_PT_P2_TYPE:
		     Crv -> Points[2][l] = NewEndPt[1] * Crv -> Points[0][l];
		     Crv -> Points[1][l] = NewEndPt[0] * Crv -> Points[0][l];
		     break;
	         default:
		     assert(0);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine given points against a given set of locations.  Split curves     *
* if some points is on the curve, at the point(s) location(s).		     *
*   If Pts is NULL, examines the end points of the given curves.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:   Input curves to filter (partial) duplicates, in place.           *
*   PtsObj: List of points to examine of on curves and if so split curves    *
*           there.  if NULL uses all end points in all given curves.         *
*   Eps:    to consider curves same and Pt-Crv distances to consider same.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Split curves, in place.	    			     *
*****************************************************************************/
static CagdCrvStruct *UserCrvArngmntSplitAtPts(CagdCrvStruct *Crvs,
					       const IPObjectStruct *PtsObj,
					       CagdRType Eps)
{   
    int i, CrvsEndPts, NumPts,
	m = 0;
    CagdRType TMin, TMax, *R;
    CagdPType *Pts;
    CagdCrvStruct *Crv, *RCrv,
	*NewCrvs = NULL;

    if ((CrvsEndPts = (PtsObj == NULL)) != FALSE ||
	(IP_IS_OLST_OBJ(PtsObj) && IPListObjectLength(PtsObj) == 0)) {
        /* Build a vector of points from curves' end points. */
        NumPts = CagdListLength(Crvs) * 2;
	Pts = (CagdPType *) IritMalloc(sizeof(CagdPType) * NumPts);

	for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext) {
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    R = CagdCrvEval(Crv, TMin);
	    CagdCoerceToE3(Pts[i++], &R, -1, Crv -> PType);
	    R = CagdCrvEval(Crv, TMax);
	    CagdCoerceToE3(Pts[i++], &R, -1, Crv -> PType);
	}
	assert(i == NumPts);
    }
    else {
        IPObjectStruct *PTmp;

        /* Build a vector of points from input list object of points. */
        if (!IP_IS_OLST_OBJ(PtsObj))
	    return NULL;

	Pts = (CagdPType *) IritMalloc(IPListObjectLength(PtsObj) *
							  sizeof(CagdPType));

	for (i = 0; (PTmp = IPListObjectGet(PtsObj, i)) != NULL; i++) {
	    if (!IP_IS_POINT_OBJ(PTmp) && !IP_IS_VEC_OBJ(PTmp)) {
	        IritFree(Pts);
		return NULL;
	    }
	    IRIT_PT_COPY(Pts[i], PTmp -> U.Pt);
	}
	NumPts = i;
    }

    while (Crvs != NULL) {
	CagdPType Pt, Pt1, Pt2;
	CagdBBoxStruct BBox;

	IRIT_LIST_POP(Crv, Crvs);

	CagdCrvBBox(Crv, &BBox);
	BBox.Min[0] -= Eps;
	BBox.Max[0] += Eps;
	BBox.Min[1] -= Eps;
	BBox.Max[1] += Eps;
	    
	CagdCrvDomain(Crv, &TMin, &TMax);
	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);
	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE3(Pt2, &R, -1, Crv -> PType);

	for (i = 0; i < NumPts; i++) {
	    if (Pts[i][0] < BBox.Min[0] ||
		Pts[i][1] < BBox.Min[1] ||
		Pts[i][0] > BBox.Max[0] ||
		Pts[i][1] > BBox.Max[1]) {
		continue;
	    }
	    if (!IRIT_PT_APX_EQ_EPS(Pt1, Pts[i], Eps) ||
		!IRIT_PT_APX_EQ_EPS(Pt2, Pts[i], Eps)) {
	        /* See if Pts[i] is on Crv: */
	        CagdRType
		    t = SymbDistCrvPoint(Crv, Pts[i], TRUE,
					 USER_CA_CCI_TOLERANCE);

		R = CagdCrvEval(Crv, t);
		CagdCoerceToE3(Pt, &R, -1, Crv -> PType);
		if (!IRIT_APX_EQ_EPS(t, TMin, IRIT_EPS) &&
		    !IRIT_APX_EQ_EPS(t, TMax, IRIT_EPS) &&
		    IRIT_PT_APX_EQ_EPS(Pt, Pts[i], Eps)) {
		    /* Found a match - split Crv at t. */
		    m++;
		    RCrv = CagdCrvSubdivAtParam(Crv, t);

		    CagdCrvFree(Crv);

		    /* Typically we will have two curves after a split. */
		    if (RCrv -> Pnext == NULL) {
		        IRIT_LIST_PUSH(RCrv, Crvs);
		    }
		    else {
		        RCrv -> Pnext -> Pnext = Crvs;
			Crvs = RCrv;
		    }
		    break; /* Need to examine the pieces against Pts. */
		}
	    }
	}

	if (i >= NumPts) {
	    /* This curve contains no Pts in its interior. */
	    IRIT_LIST_PUSH(Crv, NewCrvs);
	}
    }

    IritFree(Pts);

    return NewCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Approximate linear curves using quadratic C1 curves, in place.           *
*   Other curves or linear curves with size smaller that (negative) FitSize  *
* are transferred to the output as is.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      Curves to least square fit quadratic curves.                  *
*   FitSize:     If FitC1Crv positive, the Fitted curve size is set to       *
*              InputCrvSize * FitC1Crv / 100 (i.e. FitSize serves as         *
*              percetange of input size).				     *
*                 If FitSize negative, the Fitted curve size is simply set   *
*              to ABS(FitC1Crv).					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Fitted curves.                                        *
*****************************************************************************/
static CagdCrvStruct *UserCrvArngmntLinearCrvsFitC1(CagdCrvStruct *Crvs,
						    int FitSize)
{
    int NewLength;
    IrtRType Err;
    CagdCrvStruct *Crv, *NewCrv,
	*NewCrvs = NULL;

    while (Crvs != NULL) {
        IRIT_LIST_POP(Crv, Crvs);

	NewLength = FitSize > 0 ? Crv -> Length * FitSize / 100 : -FitSize;

	if (Crv -> Order == 2 &&
	    NewLength >= 3 &&
	    Crv -> Length >= NewLength &&
	    (NewCrv = BspCrvFitLstSqr(Crv, 3, NewLength, CAGD_CHORD_LEN_PARAM,
				      TRUE, TRUE, &Err)) != NULL) {
	    int CAIdx = AttrGetIntAttrib(Crv -> Attr, "CAIndex");

	    CagdCrvFree(Crv);
	    Crv = NewCrv;
	    AttrSetRealAttrib(&Crv -> Attr, "FitError", Err);
	    AttrSetIntAttrib(&Crv -> Attr, "CAIndex", CAIdx);
	}

	IRIT_LIST_PUSH(Crv, NewCrvs);
    }

    return CagdListReverse(NewCrvs); /* Recover the original curves' order. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copy all the data structure used in the curve arrangement.               M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:   The curve arrangement structure to copy.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserCrvArngmntStruct *:   NULL if error or new copy of CA.               M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntCopy                                                       M
*****************************************************************************/
UserCrvArngmntStruct *UserCrvArngmntCopy(const UserCrvArngmntStruct *CA)
{
    UserCrvArngmntStruct
       *NewCA = (UserCrvArngmntStruct *) IritMalloc(sizeof(UserCrvArngmntStruct));

    IRIT_GEN_COPY(NewCA, CA, sizeof(UserCrvArngmntStruct));
    NewCA -> Error = NULL;

    NewCA -> Pts = NULL;
    NewCA -> Crvs = NULL;
    NewCA -> Regions = NULL;
    NewCA -> Output = NULL;

    /* Copy the original Cagd curve. */
    NewCA -> CagdCrvs = CagdCrvCopyList(CA -> CagdCrvs);

    return NewCA;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Delete all the data structure used in the curve arrangement.             M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:   The curve arrangement structure to free.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntFree                                                       M
*****************************************************************************/
int UserCrvArngmntFree(UserCrvArngmntStruct *CA)
{
    int i;

    if (CA -> Regions != NULL) {
	for (i = 0; i < CA -> NumOfRegions; i++) {
	UserCAReleaseRefCrvList(CA -> Regions[i] -> RefCrvs, FALSE);
	    IritFree(CA -> Regions[i]);
	}
	IritFree(CA -> Regions);
    }

    if (CA -> Output != NULL)
        IPFreeObject(CA -> Output);

    if (CA -> Crvs != NULL) {
        for (i = 0; i < CA -> NumOfCrvs; i++) {
	    CagdCrvFree(CA -> Crvs[i].Crv);
	    CagdCrvFree(CA -> Crvs[i].XYCrv);
	}
	IritFree(CA -> Crvs);
    }

    CagdCrvFreeList(CA -> CagdCrvs);

    if (CA -> Pts != NULL)
	IritFree(CA -> Pts);

    IritFree(CA);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a list of curves, compute the best (least squares) fitted plane.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:              List of curves to fit a plane through.                *
*   PlanarityTol:      Tolerance to accept all curves as planar.             *
*   Pln:               The fitted plane.			             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  Maximal deviation of some control point from fitted plane.    *
*****************************************************************************/
static IrtRType UserCrvArngmntFindPlane(CagdCrvStruct *Crvs,
					IrtRType PlanarityTol,
					IrtPlnType Pln)
{
    int i;
    IrtRType R, MaxDist;
    CagdCrvStruct *Crv;
    IPVertexStruct
	*V = NULL;
    IPPolygonStruct
        *Pl = IPAllocPolygon(0, NULL, NULL);

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        CagdPolylineStruct
	    *CagdPl = CagdCrv2CtrlPoly(Crv);

	for (i = 0; i < CagdPl -> Length; i++) {
	    V = IPAllocVertex2(NULL);
	    IRIT_PT_COPY(V -> Coord, CagdPl -> Polyline[i].Pt);
	    IRIT_LIST_PUSH(V, Pl -> PVertex);
	}
	CagdPolylineFree(CagdPl);
    }
    IPGetLastVrtx(V) -> Pnext = V;	        /* Make vertex list cyclic. */

    /* Derive the best-fit plane and make sure it is normalized. */
    GMFitObjectWithOutliers(Pl, GM_FIT_PLANE, Pln, PlanarityTol,
			    USER_CA_PLANE_FIT_ITERS);
    R = IRIT_PT_LENGTH(Pln);
    if (R < IRIT_UEPS) {
        /* Something is wrong - failed to compute the plane. */
        return IRIT_INFNTY;
    }
    R = 1.0 / R;
    IRIT_PLANE_SCALE(Pln, R);
    
    MaxDist = 0.0;
    V = Pl -> PVertex; 
    do {
	R = IRIT_FABS(IRIT_DOT_PROD(V -> Coord, Pln) + Pln[3]);
	if (MaxDist < R)
	    MaxDist = R;

	V = V -> Pnext;
    }
    while (V != Pl -> PVertex);

    IPFreePolygon(Pl);

    if (MaxDist < IRIT_UEPS &&
	IRIT_APX_EQ_EPS(Pln[0], 0.0, IRIT_UEPS) &&
	IRIT_APX_EQ_EPS(Pln[1], 0.0, IRIT_UEPS)) {
        /* We have data in the XY Plane. Make sure we assume +Z as axis. */
        Pln[0] = Pln[1] = Pln[3] = 0.0;
	Pln[2] = 1.0;
    }

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes all intersections in the given list of curves in XY plane and   *
* split intersecting curves at the intersection locations, in place.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       List of curves to seek and split at intersection locations.  *
*   Tolerance:  Tolerance of intersection locations' computation.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Same list after the splits at intersections, in place. *
*****************************************************************************/
static CagdCrvStruct *UserCrvArngmntProcessIntersections(CagdCrvStruct *Crvs,
							 CagdRType Tolerance)
{
    Crvs = CagdCrvCrvInterArrangment(Crvs, TRUE, Tolerance);

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits all input curves at special locations such as C1 discontinuities  *
* and inflections points, in place.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       List of curves to seek and split at special pts, in place.   *
*   Tolerance:  Tolerance of intersection locations' computation.	     *
*   CrvSplit:   A mask setting the type pf points to split at.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Same list after the splits at special pts, in place.  *
*****************************************************************************/
static CagdCrvStruct *UserCrvArngmntProcessSpecialPts(CagdCrvStruct *Crvs,
						      CagdRType Tolerance,
						      UserCASplitType CrvSplit)
{
    int Proximity;
    CagdCrvStruct *Crv, *NewCrvs, *TCrvs;
    CagdPtStruct *Pts;

    if (CrvSplit & USER_CA_SPLIT_C1DISCONT_PTS) {
        NewCrvs = NULL;

	while (Crvs != NULL) {
	    IRIT_LIST_POP(Crv, Crvs);

	    TCrvs = CagdCrvSubdivAtAllC1Discont(Crv);

	    CagdCrvFree(Crv);

	    NewCrvs = CagdListAppend(TCrvs, NewCrvs);
	}

	Crvs = NewCrvs;
    }

    if (CrvSplit & USER_CA_SPLIT_INFLECTION_PTS) {
        NewCrvs = NULL;

	while (Crvs != NULL) {
	    IRIT_LIST_POP(Crv, Crvs);
	    if ((Pts = SymbCrv2DInflectionPts(Crv, Tolerance)) != NULL) {
	        TCrvs = CagdCrvSubdivAtParams(Crv, Pts,
					      USER_CA_CCI_BNDRY_TOL,
					      &Proximity);

		CagdPtFreeList(Pts);
		CagdCrvFree(Crv);
		NewCrvs = CagdListAppend(NewCrvs, TCrvs);
	    }
	    else {
	        IRIT_LIST_PUSH(Crv, NewCrvs);
	    }
	}

	Crvs = NewCrvs;
    }

    if (CrvSplit & USER_CA_SPLIT_MAX_CRVTR_PTS) {
        NewCrvs = NULL;

	while (Crvs != NULL) {
	    IRIT_LIST_POP(Crv, Crvs);
	    if ((Pts = SymbCrvExtremCrvtrPts(Crv, Tolerance)) != NULL) {
	        TCrvs = CagdCrvSubdivAtParams(Crv, Pts,
					      USER_CA_CCI_BNDRY_TOL,
					      &Proximity);

		CagdPtFreeList(Pts);
		CagdCrvFree(Crv);
		NewCrvs = CagdListAppend(NewCrvs, TCrvs);
	    }
	    else {
	        IRIT_LIST_PUSH(Crv, NewCrvs);
	    }
	}

	Crvs = NewCrvs;
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepare the CA for full evaluation - build the Pts and Crv vectors.      *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:   Curves' arrangement to process.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if successful, FALSE otherwise.                             *
*****************************************************************************/
static int UserCrvArngmntPrepEval(UserCrvArngmntStruct *CA)
{
    int i, j;
    CagdCrvStruct *Crv, *XYCrv,
        *Crvs = CagdCrvCopyList(CA -> CagdCrvs);

    CA -> AllocSizeCrvs = CA -> NumOfCrvs * 2;
    CA -> Crvs = (UserCACrvStruct *) IritMalloc(sizeof(UserCACrvStruct) *
						CA -> AllocSizeCrvs);
    IRIT_ZAP_MEM(CA -> Crvs, sizeof(UserCACrvStruct) * CA -> AllocSizeCrvs);
    for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++) {
        CA -> Crvs[i].Idx = i;
	CA -> Crvs[i].NumOfCmplxLoops = 0;
        CA -> Crvs[i].Crv = Crv;       /* Recall we work on a copied list. */
        CA -> Crvs[i].XYCrv = CagdCrvMatTransform(Crv, CA -> XYZ2XYMat);

	if (CA -> ProjectOnPlane) {
	    /* Coerce transformed to XY crvs to be exactly in plane (Z = 0). */
	    if (CAGD_IS_RATIONAL_CRV(CA -> Crvs[i].XYCrv))
	        XYCrv = CagdCoerceCrvTo(CA -> Crvs[i].XYCrv,
					CAGD_PT_P2_TYPE, FALSE);
	    else
	        XYCrv = CagdCoerceCrvTo(CA -> Crvs[i].XYCrv,
					CAGD_PT_E2_TYPE, FALSE);

	    CagdCrvFree(CA -> Crvs[i].XYCrv);
	    CA -> Crvs[i].XYCrv = XYCrv;
	}
    }
    for ( ; i < CA -> AllocSizeCrvs; i++)
        CA -> Crvs[i].Idx = USER_CA_PT_UNUSED;
 
    /* Allocate and initialize the end points vector. */
    CA -> AllocSizePts = CA -> AllocSizeCrvs * 2;
    CA -> Pts = (UserCAPtStruct *) IritMalloc(sizeof(UserCAPtStruct) *
					      CA -> AllocSizePts);
    IRIT_ZAP_MEM(CA -> Pts, sizeof(UserCAPtStruct) * CA -> AllocSizePts);
    for (i = 0; i < CA -> NumOfCrvs; i++) {
        CagdRType *R, TMin, TMax;

	Crv = CA -> Crvs[i].Crv;
	XYCrv = CA -> Crvs[i].XYCrv;

	CagdCrvDomain(Crv, &TMin, &TMax);

	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE3(CA -> Pts[i * 2].Pt, &R, -1, Crv -> PType);
	R = CagdCrvEval(XYCrv, TMin);
	CagdCoerceToE3(CA -> Pts[i * 2].XYPt, &R, -1, XYCrv -> PType);
	CA -> Pts[i * 2].Idx = i * 2;
	CA -> Pts[i * 2].RefCrvs[0] = &CA -> Crvs[i];
	CA -> Pts[i * 2].NumOfRefCrvs = 1;
	CA -> Crvs[i].RefStartPt = &CA -> Pts[i * 2];

	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE3(CA -> Pts[i * 2 + 1].Pt, &R, -1, Crv -> PType);
	R = CagdCrvEval(XYCrv, TMax);
	CagdCoerceToE3(CA -> Pts[i * 2 + 1].XYPt, &R, -1, XYCrv -> PType);
	CA -> Pts[i * 2 + 1].Idx = i * 2 + 1;
	CA -> Pts[i * 2 + 1].RefCrvs[0] = &CA -> Crvs[i];
	CA -> Pts[i * 2 + 1].NumOfRefCrvs = 1;
	CA -> Crvs[i].RefEndPt = &CA -> Pts[i * 2 + 1];
    }
    for (j = i * 2; j < CA -> AllocSizePts; j++)
        CA -> Pts[j].Idx = USER_CA_PT_UNUSED;
    CA -> NumOfPts = i * 2;

    /* Allocate enough regions. */
    CA -> Regions = (UserCARegionStruct **) IritMalloc(sizeof(UserCARegionStruct *)
						       * CA -> AllocSizeCrvs);
    CA -> NumOfRegions = 0;

    CA -> Output = NULL;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detect, up to EndPtTol, end points of curves that can be considered the  M
* same and merge these points.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:   Curves' arrangement to process.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntProcessEndPts                                              M
*****************************************************************************/
int UserCrvArngmntProcessEndPts(UserCrvArngmntStruct *CA)
{
    int i, j, k, m, n1, n2, n;
    UserCAPtStruct
	*Pts = CA -> Pts;
    UserCACrvStruct
	*Crvs = CA -> Crvs;
    CagdRType
        EndPtTol = CA -> EndPtEndPtTol;

    /* First search for simple loops of size one - one closed curve. */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
        if (IRIT_PT_APX_EQ_E2_EPS(Crvs[i].RefStartPt -> XYPt,
				  Crvs[i].RefEndPt -> XYPt, EndPtTol)) {
	    Crvs[i].RefStartPt -> Idx =
	        Crvs[i].RefEndPt -> Idx = USER_CA_PT_SELFLOOP;
	}
    }

    /* Time for global merge. */
    for (i = 0; Pts[i].Idx != USER_CA_PT_UNUSED; i++) {
        if (Pts[i].Idx == USER_CA_PT_MERGED ||
	    Pts[i].Idx == USER_CA_PT_SELFLOOP)
	    continue;

        for (j = i + 1; Pts[j].Idx != USER_CA_PT_UNUSED; j++) {
	    if (Pts[j].Idx == USER_CA_PT_MERGED ||
		Pts[j].Idx == USER_CA_PT_SELFLOOP)
	        continue;

	    if (IRIT_PT_APX_EQ_E2_EPS(Pts[i].XYPt, Pts[j].XYPt, EndPtTol)) {
	        /* Merge point at index j to the point at index i. */

	        /* Count how many curves are going to end in this pt. */
	        n1 = Pts[j].NumOfRefCrvs;
	        n2 = Pts[i].NumOfRefCrvs;
		n = n1 + n2;

		if (n >= USER_CA_MAX_CRV_AT_VRTX) {
		    CA -> Error = "Maximum curves meeting at a vertex reached.";
		    return FALSE;
		}

		/* Merge the j's curves into i. */
		for (m = 0, k = n2; k < n; )
		    Pts[i].RefCrvs[k++] = Pts[j].RefCrvs[m++];
		Pts[i].NumOfRefCrvs += n1;

		/* Make the curves in the j list point to point i. */
		for (k = 0; k < n1; k++) {
		    UserCACrvStruct
		        *CACrv = Pts[j].RefCrvs[k];

		    if (CACrv -> RefStartPt == &Pts[j])
		        CACrv -> RefStartPt = &Pts[i];
		    else if (CACrv -> RefEndPt == &Pts[j])
		        CACrv -> RefEndPt = &Pts[i];
		    else
		        assert(0);
		}

	        Pts[j].Idx = USER_CA_PT_MERGED;          /* Mark as merged. */
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Create regions (loops, etc.) from the given arrangement and classify     M
* them.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:   Curves' arrangement to process.                                    M
*   IgnoreInteriorHangingCrvs:  TRUE to ignore hanging curves.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntClassifyConnectedRegions                                   M
*****************************************************************************/
int UserCrvArngmntClassifyConnectedRegions(UserCrvArngmntStruct *CA,
					   int IgnoreInteriorHangingCrvs)
{
    int i;

    /* Mark all curves as undefined types. */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
        UserCACrvStruct
	    *CACrv = &CA -> Crvs[i];

	CACrv -> Type = USER_CA_UNDEF_TYPE;
    }

    /* Do the easy part first: fetch the simple and loop regions.  In such */
    /* regions, every point is shared by exactly two curves, but,	   */
    /* possibly, one of the two end points.		                   */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
        UserCACrvStruct
	    *CACrv = &CA -> Crvs[i];

	if (CACrv -> Type == USER_CA_UNDEF_TYPE) {
	    UserCARefCrvStruct
	        *CrvsRef = UserCrvArngmntFetchSimpleRegion(CA, i);

	    if (CrvsRef != NULL)
	        CA -> Regions[CA -> NumOfRegions++] = UserCANewRegion(CrvsRef);
	}
    }

    /* Now we must process complex regions. */
    if (!UserCrvArngmntFetchComplexRegions(CA, IgnoreInteriorHangingCrvs))
        return FALSE;

    /* Append all left overs as hanging curves. */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
        UserCACrvStruct
	    *CACrv = &CA -> Crvs[i];

	if (CACrv -> Type == USER_CA_UNDEF_TYPE) {
	    UserCARefCrvStruct
		*CARefCrv = UserCANewRefCrv(CACrv);

	    CACrv -> Type = USER_CA_LEFTOVERS_TYPE;
	    CA -> Regions[CA -> NumOfRegions++] = UserCANewRegion(CARefCrv);
	}
    }

    /* Orientation stage - A loop should be orient CW and hanging/simple  */
    /* region should end NE with respect to its starting point.		  */
    for (i = 0; i < CA -> NumOfRegions; i++) {
	CagdRType *R, TMin, TMax;
	CagdPType StartPt, EndPt;
        UserCARegionStruct
	    *Region = CA -> Regions[i];
        UserCARefCrvStruct
	    *CARefCrvs = Region -> RefCrvs;
	IPObjectStruct
	    *PCrv = UserCrvArngmntRegion2Curves(CA, CARefCrvs, TRUE);
	CagdCrvStruct
	    *Crv = PCrv -> U.Crvs;

	CagdCrvDomain(Crv, &TMin, &TMax);
	R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE2(StartPt, &R, -1, Crv -> PType);
	R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE2(EndPt, &R, -1, Crv -> PType);

	if (Region -> Type != USER_CA_LOOP_TYPE &&
	    IRIT_PT_APX_EQ_E2_EPS(StartPt, EndPt, CA -> EndPtEndPtTol)) {
	    UserCARefCrvStruct
	        *CARefCrv = Region -> RefCrvs;

	    /* It is actually a loop of a single curve (self-loop). */
	    Region -> Type = USER_CA_LOOP_TYPE;

	    for ( ; CARefCrv != NULL; CARefCrv = CARefCrv -> Pnext)
	        CARefCrv -> RefCrv -> Type = USER_CA_LOOP_TYPE;
	}

	if (Region -> Type == USER_CA_LOOP_TYPE) {
	    Region -> Area = SymbCrvEnclosedAreaEval(PCrv -> U.Crvs);

	    if (!TrimClassifyTrimCurveOrient(Crv)) {
	        /* Reverse the curves of this loop region. */
	        CARefCrvs = UserCrvArngmntReverseRefCrvList(CARefCrvs);
	    }
	}
	else if (Region -> Type == USER_CA_SIMPLE_TYPE ||
		 Region -> Type == USER_CA_HANGING_TYPE) {
	    if (EndPt[0] < StartPt[0] ||
		(EndPt[0] == StartPt[0] && EndPt[1] < StartPt[1])) {
	        /* Reverse the curves of this simple region. */
	        CARefCrvs = UserCrvArngmntReverseRefCrvList(CARefCrvs);
	    }
	}

	IPFreeObject(PCrv);

	Region -> RefCrvs = CARefCrvs;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocate a region struct. 			                             *
*                                                                            *
* PARAMETERS:                                                                *
*   RefCrvs:  A list of curves that define a region.  Used in-place.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserCARefCrvStruct *:  A new  region structure.		             *
*****************************************************************************/
static UserCARegionStruct *UserCANewRegion(UserCARefCrvStruct *RefCrvs)
{
    UserCARegionStruct
	*Rgn = (UserCARegionStruct *) IritMalloc(sizeof(UserCARegionStruct));

    assert(RefCrvs != NULL);

    Rgn -> RefCrvs = RefCrvs;
    Rgn -> Type = RefCrvs -> RefCrv -> Type;
    Rgn -> Area = 0.0;
    Rgn -> ContainedIn = -1;
    Rgn -> Pnext = NULL;

    return Rgn;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocate a reference to a curve struct.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:  Curve to create a reference structure for.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserCARefCrvStruct *:  A new reference to a curve structure.             *
*****************************************************************************/
static UserCARefCrvStruct *UserCANewRefCrv(UserCACrvStruct *Crv)
{
    UserCARefCrvStruct
	*RCrv = (UserCARefCrvStruct *) IritMalloc(sizeof(UserCARefCrvStruct));

    RCrv -> RefCrv = Crv;
    RCrv -> Inverted = FALSE;
    RCrv -> Pnext = NULL;

    return RCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Release a reference list to curves.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   RefList:     List to free.                                               *
*   ResetUndef:  TRUE to also reselt the list of curves to UNDEF state.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserCAReleaseRefCrvList(UserCARefCrvStruct *RefList,
				    int ResetUndef)
{
    UserCARefCrvStruct *R;

    while (RefList != NULL) {
        IRIT_LIST_POP(R, RefList);

	if (ResetUndef)
	    R -> RefCrv -> Type = USER_CA_UNDEF_TYPE;/* Recover UNDEF state. */

	IritFree(R);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch a simple linear poly-curve that can also be a loop, starting from  *
* curve index i.  A simple region (or a loop) is a region where all curves   *
* are connected to at most one other curve.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:           Curves' arrangement to process.                            *
*   CrvIdx:       Index of curve in simple region to start the search from.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserCARefCrvStruct *:  A list of curves forming the simple region.       *
*****************************************************************************/
static UserCARefCrvStruct *UserCrvArngmntFetchSimpleRegion(
						     UserCrvArngmntStruct *CA,
						     int CrvIdx)
{
    int End1Complex = FALSE,
        End2Complex = FALSE;
    UserCACrvStruct  *NextCACrv, *PrevCACrv;
    UserCARefCrvStruct *Crnt, *Next, *Prev,
        *Base = UserCANewRefCrv(&CA -> Crvs[CrvIdx]);
    UserCAPtStruct *CrntPt;
    UserCAObjType
        Type = USER_CA_UNDEF_TYPE;

    Base -> RefCrv -> Type = USER_CA_SIMPLE_TYPE;

    /* Marge forward as much as possible. */
    Crnt = Next = Base;
    CrntPt = Crnt -> RefCrv -> RefEndPt;
    while (Crnt != NULL) {
	if (CrntPt -> NumOfRefCrvs == 1) {
	    /* Point only at this curve - this can be a (open) simple loop. */
	    break;
	}
	else if (CrntPt -> NumOfRefCrvs == 2) {
	    /* Find other curve and march into it. */
	    NextCACrv = USER_CA_GET_OTHER_CRV(CrntPt -> RefCrvs,
					      Crnt -> RefCrv);
	    if (NextCACrv -> Type == USER_CA_UNDEF_TYPE) {
		NextCACrv -> Type = USER_CA_SIMPLE_TYPE;
		Next = UserCANewRefCrv(NextCACrv);
		Crnt -> Pnext = Next;
		Crnt = Next;
		CrntPt = CrntPt == NextCACrv -> RefStartPt ?
						NextCACrv -> RefEndPt :
						NextCACrv -> RefStartPt;
	    }
	    else
	        break;
	}
	else {
	    /* This is a complex joint with more than 2 curves meeting. */
	    End1Complex = TRUE;
	    break;
	}
    }

    /* Marge backward as much as possible. */
    Crnt = Prev = Base;
    CrntPt = Crnt -> RefCrv -> RefStartPt;
    while (Crnt != NULL) {
	if (CrntPt -> NumOfRefCrvs == 1) {
	    /* Point of only this curve - this can be a (open) simple loop. */
	    break;
	}
	else if (CrntPt -> NumOfRefCrvs == 2) {
	    /* Find other curve and march into it. */
	    PrevCACrv = USER_CA_GET_OTHER_CRV(CrntPt -> RefCrvs, Crnt -> RefCrv);
	    if (PrevCACrv -> Type == USER_CA_UNDEF_TYPE) {
		PrevCACrv -> Type = USER_CA_SIMPLE_TYPE;
                Prev = UserCANewRefCrv(PrevCACrv);
		Prev -> Pnext = Crnt;
		Crnt = Prev;
		CrntPt = CrntPt == PrevCACrv -> RefStartPt ?
						PrevCACrv -> RefEndPt :
						PrevCACrv -> RefStartPt;
	    }
	    else
	        break;
	}
	else {
	    /* This is a complex joint with more than 2 curves meeting. */
	    End2Complex = TRUE;
	    break;
	}
    }

    if (End1Complex || End2Complex) {
        if (End1Complex && End2Complex) {
	    /* A complex region to be handled later - unroll and abort. */
	    UserCAReleaseRefCrvList(Prev, TRUE);
	    return NULL;
	}
	else {
	    /* This is a simple list of curves, but hanging at one end. */
	    Type = USER_CA_HANGING_TYPE;
	}
    }

    /* If we are here we have simple regions (or loop).   A loop will have  */
    /* the first (and the last) curves connected.  Check and also orient.   */
    if (Prev -> Pnext == NULL) {      /* A simple region of one curve only. */
	if (Type != USER_CA_UNDEF_TYPE)
	    Prev -> RefCrv -> Type = Type;
        return Prev;
    }

    if (Type == USER_CA_UNDEF_TYPE) {
        if (Next -> RefCrv -> RefStartPt -> NumOfRefCrvs == 2 &&
	    Next -> RefCrv -> RefEndPt -> NumOfRefCrvs == 2 &&
	    Prev -> RefCrv -> RefStartPt -> NumOfRefCrvs == 2 &&
	    Prev -> RefCrv -> RefEndPt -> NumOfRefCrvs == 2)
	    Type = USER_CA_LOOP_TYPE;
	else
	    Type = USER_CA_SIMPLE_TYPE;
    }

    /* Now orientation stage  - all curves oriented currently along region. */
    /* Also, as a side effect, update the orientation slot of the curves.   */
    if (Prev -> RefCrv -> RefStartPt == Prev -> Pnext -> RefCrv -> RefEndPt ||
	Prev -> RefCrv -> RefStartPt == Prev -> Pnext -> RefCrv -> RefStartPt)
        Prev -> Inverted = TRUE;
    else
        Prev -> Inverted = FALSE;

    for (Crnt = Prev; Crnt != NULL; Crnt = Next) {
        if ((Next = Crnt -> Pnext) == NULL)
	    break;

	Crnt -> RefCrv -> Type = Type;

        if (Crnt -> Inverted) {
	    if (Crnt -> RefCrv -> RefStartPt == Next -> RefCrv -> RefStartPt)
	        Next -> Inverted = FALSE;
	    else {
	        Next -> Inverted = TRUE;
		assert(Crnt -> RefCrv -> RefStartPt == Next -> RefCrv -> RefEndPt);
	    }
	}
	else {
	    if (Crnt -> RefCrv -> RefEndPt == Next -> RefCrv -> RefStartPt)
	        Next -> Inverted = FALSE;
	    else {
	        Next -> Inverted = TRUE;
		assert(Crnt -> RefCrv -> RefEndPt == Next -> RefCrv -> RefEndPt);
	    }
	}
    }

#ifdef DEBUG
    for (Crnt = Prev; Crnt != NULL; Crnt = Next)
        assert(Crnt -> RefCrv -> Type == Type);
#endif /* DEBUG */

    return Prev;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch the complex regions from what is left (no simple regions any more).*
*                                                                            *
* PARAMETERS:                                                                *
*   CA:           Curves' arrangement to process.                            *
*   IgnoreInteriorHangingCrvs:  TRUE to ignore hanging curves.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if succesful, FALSE otherwise.                                *
*****************************************************************************/
static int UserCrvArngmntFetchComplexRegions(UserCrvArngmntStruct *CA,
					     int IgnoreInteriorHangingCrvs)
{
    int i, *CrvsFrwd, *CrvsBkwd;

    /* CrvsFrwd and CrvsBkwd will hold, for any curve in a complex domain,  */
    /* the next/prev curve in the complex domain, if any, when moving       */
    /* clockwise from the curve forward or backward.  Recall that a curve   */
    /* in a complex domain can be shared by at most two regions.	    */
    CrvsFrwd = (int *) IritMalloc(sizeof(int) * CA -> NumOfCrvs);
    CrvsBkwd = (int *) IritMalloc(sizeof(int) * CA -> NumOfCrvs);

    for (i = 0; i < CA -> NumOfCrvs; i++) {
        if (CA -> Crvs[i].Type == USER_CA_UNDEF_TYPE) {
	    CrvsFrwd[i] = UserCrvArngmntFindFrwdBkwdCrv(CA, i, TRUE,
							IgnoreInteriorHangingCrvs);
	    CrvsBkwd[i] = UserCrvArngmntFindFrwdBkwdCrv(CA, i, FALSE,
							IgnoreInteriorHangingCrvs);

	    if (CrvsFrwd[i] == -2 || CrvsBkwd[i] == -2) {  /* HIt tangency. */
	        IritFree(CrvsFrwd);
		IritFree(CrvsBkwd);
		return FALSE;
	    }

	    /* Remember the number of loops this curves participates in. */
	    CA -> Crvs[i].NumOfCmplxLoops =
				      (CrvsFrwd[i] >= 0) + (CrvsBkwd[i] >= 0);
	}
	else {
	    CrvsFrwd[i] = CrvsBkwd[i] = -1;    /* Init. with invalid index. */
	    CA -> Crvs[i].NumOfCmplxLoops = 0;
	}
    }

    DEBUG_PRINT_CURVE_ADJ_INFO();

    /*   Now scan for curves with forward or backward links and positive    */
    /* NumOfCmplxLoops and chain them into closed loops until no more valid */
    /* links are detected.  						    */
    /*   Then, NumOfCmplxLoops should go down to zero in all curves.        */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
	UserCARefCrvStruct *CrvsRef;

        if (CA -> Crvs[i].Type == USER_CA_UNDEF_TYPE ||
	    (CA -> Crvs[i].Type == USER_CA_COMPLEX_TYPE &&
	     CA -> Crvs[i].NumOfCmplxLoops > 0)) {
	    /* Start chaining a loop from this curve forward, if possible. */
	    if (CrvsFrwd[i] >= 0) {
	        CrvsRef = UserCrvArngmntCollectCmplxLoop(CA, i, FALSE, TRUE,
							 CrvsFrwd, CrvsBkwd);
		if (CrvsRef != NULL) {
		    CA -> Regions[CA -> NumOfRegions++] = 
				   UserCANewRegion(CagdListReverse(CrvsRef));

		    DEBUG_PRINT_NEW_REGION(CA -> NumOfRegions - 1, TRUE);
		}
	    }

	    /* Start chaining a loop from this curve backward, if possible. */
	    if (CrvsBkwd[i] >= 0) {
	        CrvsRef = UserCrvArngmntCollectCmplxLoop(CA, i, TRUE, FALSE,
							 CrvsFrwd, CrvsBkwd);
		if (CrvsRef != NULL) {
		    CA -> Regions[CA -> NumOfRegions++] = 
				   UserCANewRegion(CagdListReverse(CrvsRef));

		    DEBUG_PRINT_NEW_REGION(CA -> NumOfRegions - 1, FALSE);
		}
	    }

	    DEBUG_PRINT_CURVE_ADJ_INFO();

	    assert(CA -> Crvs[i].NumOfCmplxLoops == 0);
	}
    }

    DEBUG_PRINT_CURVE_ADJ_INFO();

    IritFree(CrvsFrwd);
    IritFree(CrvsBkwd);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Collect curves in a closed complex loop starting from CrntCrvIdx.        *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:           Curves' arrangement to process.                            *
*   CrntCrvIdx:   Index of first curve to collect a loop through.            *
*   CrvReversed:  TRUE if this first curve is reversed in this loop.         *
*   CollectFrwd:  TRUE to collect the loop forward from CrntCrvIdx,	     *
*		  FALSE to collect it backward.                              *
*   CrvsFrwd:     Vector of next curves, moving forwards from current one.   *
*   CrvsBkwd:     Vector of prev curves, moving backwards from current one.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserCARegionStruct *:  The collected loop or NULL if failed (not error). *
*****************************************************************************/
static UserCARefCrvStruct *UserCrvArngmntCollectCmplxLoop(UserCrvArngmntStruct
								          *CA,
							  int StartCrvIdx,
							  int CrvReversed,
							  int CollectFrwd,
							  int *CrvsFrwd,
							  int *CrvsBkwd)
{
    int NextCrvIdx,
        CrntCrvIdx = StartCrvIdx,
	AbortLoop = FALSE;
    UserCARefCrvStruct
        *CrvRef = NULL,
        *CrvsRef = NULL;
    UserCAPtStruct *SharedPt;

    if (CollectFrwd) {
        SharedPt = CA -> Crvs[CrntCrvIdx].RefEndPt;
	NextCrvIdx = CrvsFrwd[CrntCrvIdx];
    }
    else {
        SharedPt = CA -> Crvs[CrntCrvIdx].RefStartPt;
	NextCrvIdx = CrvsBkwd[CrntCrvIdx];
    }

    do {
        CrvRef = UserCANewRefCrv(&CA -> Crvs[CrntCrvIdx]);
	CrvRef -> Inverted = CrvReversed;
	CA -> Crvs[CrntCrvIdx].NumOfCmplxLoops--;
	CA -> Crvs[CrntCrvIdx].Type = USER_CA_COMPLEX_TYPE;
	IRIT_LIST_PUSH(CrvRef, CrvsRef);
		    
	/* Move to the curve that shares SharedPt. */
	if (SharedPt == CA -> Crvs[NextCrvIdx].RefStartPt) {
	    /* We should move forward in curve CA -> Crvs[Idx]. */
	    CrvReversed = FALSE;
	    SharedPt = CA -> Crvs[NextCrvIdx].RefEndPt;

	    AbortLoop = CrvsFrwd[NextCrvIdx] < 0;
	    CrntCrvIdx = NextCrvIdx;
	    NextCrvIdx = CrvsFrwd[CrntCrvIdx];
	    CrvsFrwd[CrntCrvIdx] = -1; /* Disable. */
	}
	else {
	    assert(SharedPt == CA -> Crvs[NextCrvIdx].RefEndPt);

	    /* We should move backward in curve CA -> Crvs[Idx].*/
	    CrvReversed = TRUE;
	    SharedPt = CA -> Crvs[NextCrvIdx].RefStartPt;

	    AbortLoop = CrvsBkwd[NextCrvIdx] < 0;
	    CrntCrvIdx = NextCrvIdx;
	    NextCrvIdx = CrvsBkwd[CrntCrvIdx];
	    CrvsBkwd[CrntCrvIdx] = -1; /* Disable. */
	}

	if (AbortLoop) {
	    /* This collection is not a closed loop but a portion on the */
	    /* outside of the complex domain and will not yield a closed */
	    /* loop.  Terminate this collection.		         */
	    UserCAReleaseRefCrvList(CrvsRef, FALSE);

	    /* We did not close a loop.  Start Frwd/Bkwd must be cleared: */
	    if (CollectFrwd)
		CrvsFrwd[StartCrvIdx] = -1;
	    else
		CrvsBkwd[StartCrvIdx] = -1;

	    CrvsRef = NULL;
	    break;
	}

    }
    while (CrntCrvIdx != StartCrvIdx);

    return CrvsRef;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the clockwise neighbor to CA curve index CrvIdx when we move    *
* forward/backward along the curve, if any.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:        Curves' arrangement to process.                               *
*   CrvIdx:    Index of curve in CA to seek its CW forward/backward neighbor.*
*   DoForward: TRUE to seek the forward neighbor, FALSE for backward.        *
*   IgnoreInteriorHangingCrvs:  TRUE to ignore hanging curves.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     Computed index of next curve in CW motion forward/backward,     *
*            or -1 if None found, -2 if tangency detected.		     *
*****************************************************************************/
static int UserCrvArngmntFindFrwdBkwdCrv(const UserCrvArngmntStruct *CA,
					 int CrvIdx,
					 int DoForward,
					 int IgnoreInteriorHangingCrvs)
{
    int i,
        CWIdx = -1;
    CagdBType
        CWStartPt = FALSE;
    CagdRType TMin, TMax, Angle,
        CWAngle = 360;
    CagdCrvStruct
        *Crv = CA -> Crvs[CrvIdx].XYCrv;
    CagdVecStruct CrvTan, OCrvTan;
    UserCAPtStruct
	*Pt = DoForward ? CA -> Crvs[CrvIdx].RefEndPt
			: CA -> Crvs[CrvIdx].RefStartPt;

    /* Compute the tangent vector at the end point of Crv toward Crv. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    if (DoForward) {
        CrvTan = *CagdCrvTangent(Crv, TMax, FALSE);
	IRIT_VEC_SCALE(CrvTan.Vec, -1.0);
    }
    else
        CrvTan = *CagdCrvTangent(Crv, TMin, FALSE);

    /* Search all the curves that meet at Crv's RefEndPt and find the one  */
    /* that is the clockwise motion forward from Crv.			   */
    for (i = 0; i < Pt -> NumOfRefCrvs; i++) {
        UserCACrvStruct
	    *OCACrv = Pt -> RefCrvs[i];
	CagdCrvStruct
	    *OCrv = OCACrv -> XYCrv;

	if (OCrv == Crv)
	    continue;

	if (IgnoreInteriorHangingCrvs &&
	    OCACrv -> Type != USER_CA_UNDEF_TYPE)
	    continue;

	CagdCrvDomain(OCrv, &TMin, &TMax);
	if (OCACrv -> RefStartPt == Pt)
	    OCrvTan = *CagdCrvTangent(OCrv, TMin, FALSE);
	else {
	    assert(OCACrv -> RefEndPt == Pt);
	    OCrvTan = *CagdCrvTangent(OCrv, TMax, FALSE);
	    IRIT_VEC_SCALE(OCrvTan.Vec, -1.0);
	}

	/* Compute the angles between the two vectors, in degrees: */
	Angle = GMPlanarVecVecAngle(CrvTan.Vec, OCrvTan.Vec,  TRUE);
	if (Angle < 0.0)
	    Angle += 360.0;

	/* Compare angles. If almost same examine neighborhood of curves. */
	if (IRIT_APX_EQ(Angle, CWAngle)) {
	    static char Err[IRIT_LINE_LEN];

	    sprintf(Err, "Tangency contact location was detected at XY position (%.5f %.5f)\n",
		    Pt -> XYPt[0], Pt -> XYPt[1]);
	    ((UserCrvArngmntStruct *) CA) -> Error = Err;
	    return -2;
	}
	else if (Angle < CWAngle) {
	    CWAngle = Angle;
	    CWIdx = OCACrv -> Idx;
	    CWStartPt = OCACrv -> RefStartPt == Pt;
	}
    }

    /* If CW neighbor is not an UNDEF (unused so far) type, invalidate. */
    if (CA -> Crvs[CWIdx].Type != USER_CA_UNDEF_TYPE)
        return -1;

    return CWIdx;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reverses a list of CA references to curves, in place.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvRefList:   Input list to reverse, in place.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserCARefCrvStruct *:   Reversed list.                                   *
*****************************************************************************/
static UserCARefCrvStruct *UserCrvArngmntReverseRefCrvList(UserCARefCrvStruct
							         *CrvRefList)
{
    UserCARefCrvStruct *CrvRef,
        *NewCrvRefList = NULL;

    while (CrvRefList) {
        IRIT_LIST_POP(CrvRef, CrvRefList);
	CrvRef -> Inverted = !CrvRef -> Inverted;
	IRIT_LIST_PUSH(CrvRef, NewCrvRefList);
    }

    return NewCrvRefList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   ClassifyContainments of loops in other loops.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:   Crv arrangment to consider.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if succesful, FALSE otherwise.                               *
*****************************************************************************/
static int UserCrvArngmntClassifyContainments(const UserCrvArngmntStruct *CA)
{
    int i, j;
    CagdCrvStruct **Loops;

    if (CA -> Regions == NULL)
        return FALSE;

    Loops = (CagdCrvStruct **)
                    IritMalloc(CA -> NumOfRegions * sizeof(CagdCrvStruct *));

    for (i = 0; i < CA -> NumOfRegions; i++) {
        CA -> Regions[i] -> ContainedIn = -1;
        if (CA -> Regions[i] -> Type == USER_CA_LOOP_TYPE) {
	    IPObjectStruct
	        *PCrv = UserCrvArngmntRegion2Curves(CA,
						    CA -> Regions[i] -> RefCrvs,
						    TRUE);
	  
	    Loops[i] = PCrv -> U.Crvs;
	    PCrv -> U.Crvs = NULL;
	    IPFreeObject(PCrv);
	}
	else
	    Loops[i] = NULL;
    }

    for (i = 0; i < CA -> NumOfRegions; i++) {
        if (Loops[i] == NULL)
	    continue;

        for (j = 0; j < CA -> NumOfRegions; j++) {
	    const CagdCrvStruct *UnUsedCrv;
	  
	    if (i == j || Loops[j] == NULL)
	        continue;

    
	    /*   Find out if loop i is contained in loop j. If so and     */
	    /* loop i is already inside some loop k, pick the loop that   */
	    /* contains the other two.					  */
	    if ((UnUsedCrv = UserCrvArngmntFindUnusedCurve(CA, i, j))
		                                                   != NULL &&
	        UserCrvArngmntIsContained(CA, UnUsedCrv, Loops[j])) {
	        int k = CA -> Regions[i] -> ContainedIn;
	      
	        if (k == -1 ||
		    ((UnUsedCrv = UserCrvArngmntFindUnusedCurve(CA, j, k))
		                                                   != NULL &&
		     UserCrvArngmntIsContained(CA, UnUsedCrv, Loops[k]) == 1)) {
		    /* We "do not" modify CA here: */
		    CA -> Regions[i] -> ContainedIn = j;
		}
	    }
        }
    }

    for (i = 0; i < CA -> NumOfRegions; i++) {
	if (Loops[i] != NULL)
	    CagdCrvFree(Loops[i]);
    }
    IritFree(Loops);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Seeks a curve that is in region Rng1 but not in region Rng2.             *
*                                                                            *
* PARAMETERS:                                                                *
*   CA:      Curves' arrangement to process.                                 *
*   Rgn1:    Index of region to search for a curve not in region Rgn2.       *
*   Rgn2:    Region to compare against, for a curve not in it.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   A pointer to a curve in region i but not j or NULL    *
*                      if none.                                              *
*****************************************************************************/
static const CagdCrvStruct *UserCrvArngmntFindUnusedCurve(
					       const UserCrvArngmntStruct *CA,
					       int Rgn1,
					       int Rgn2)
{
    const UserCARegionStruct *Region1, *Region2;
    UserCARefCrvStruct *Ref1, *Ref2;

    if (CA -> Regions == NULL)
        return NULL;

    assert(Rgn1 >= 0 && Rgn1 < CA -> NumOfRegions &&
	   Rgn2 >= 0 && Rgn2 < CA -> NumOfRegions);

    if ((Region1 = CA -> Regions[Rgn1]) == NULL ||
	(Region2 = CA -> Regions[Rgn2]) == NULL)
       return NULL;
    
    for (Ref1 = Region1 -> RefCrvs; Ref1 != NULL; Ref1 = Ref1 -> Pnext) {
        int Idx = Ref1 -> RefCrv -> Idx;

	for (Ref2 = Region2 -> RefCrvs; Ref2 != NULL; Ref2 = Ref2 -> Pnext)
	    if (Ref2 -> RefCrv -> Idx == Idx)
	        break;

	if (Ref2 == NULL)
	    return Ref1 -> RefCrv -> XYCrv;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if OuterLoop contains the given InnerShape.  Both inputs should be  M
* merged regions in CA.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to process.                            M
*   InnerShape:   A shape to test if contained in OuterLoop.                 M
*   OuterLoop:    A closed loop region to test if contains InnerShape.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if OuterLoop contains InnerShape, FALSE otherwise.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPointInclusion                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntIsContained                                                M
*****************************************************************************/
int UserCrvArngmntIsContained(const UserCrvArngmntStruct *CA,
			      const CagdCrvStruct *InnerShape,
			      const CagdCrvStruct *OuterLoop)
{
    CagdRType TMin, TMax, *R;
    CagdPType Pt;

    Pt[2] = 0.0;

    CagdCrvDomain(InnerShape, &TMin, &TMax);
    R = CagdCrvEval(InnerShape, (TMin + TMax) * 0.5);
    CagdCoerceToE2(Pt, &R, -1, InnerShape -> PType);

    return UserCrvArngmntIsContained2(CA, Pt, OuterLoop);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if Loop contains the given Point Pt.  Input should be of merged     M
* region in CA.		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:       Curves' arrangement to process.                                M
*   Pt:       A Point to test for inclusion in Loop.                         M
*   Loop:     A closed loop region to test if contains Pt.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if Loop contains Pt, FALSE otherwise.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPointInclusion                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntIsContained2                                               M
*****************************************************************************/
int UserCrvArngmntIsContained2(const UserCrvArngmntStruct *CA,
			       const CagdPType Pt,
			       const CagdCrvStruct *Loop)
{
    return SymbCrvPointInclusion(Loop, Pt, CA -> InternalTol) == 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert one internal region as lists of curves, into (a merged) Bspline  M
* curve(s).								     M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to process.                            M
*   CARefCrv:     The regions as a list of references to curves.	     M
*   Merge:        If TRUE, merge all curves in a region into one.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A (list of) curve(s), or NULL if error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvArngmntRegion2Curves                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntRegion2Curves 	                                     M
*****************************************************************************/
static IPObjectStruct *UserCrvArngmntRegion2Curves(const UserCrvArngmntStruct
						                         *CA,
						   UserCARefCrvStruct
						                   *CARefCrv,
						   int Merge)
{
    CagdCrvStruct *Crv;
    IPObjectStruct *PCrv;

    if (CARefCrv == NULL) {
        ((UserCrvArngmntStruct *) CA) 
	           -> Error = "Inconsistent data structure - undefined error";
	return NULL;				    /* Should never happen. */
    }

    if (CARefCrv -> Inverted)
        Crv = CagdCrvReverse(CARefCrv -> RefCrv -> XYCrv);
    else
        Crv = CagdCrvCopy(CARefCrv -> RefCrv -> XYCrv);

    if (CARefCrv -> Pnext == NULL) {
        PCrv = IPGenCrvObject("CACurve", Crv, NULL);
    }
    else {
        CagdCrvStruct *TCrv,
	    *AllCrvs = Crv;

	for (CARefCrv = CARefCrv -> Pnext;
	     CARefCrv != NULL;
	     CARefCrv = CARefCrv -> Pnext) {
	    if (CARefCrv -> Inverted)
	        Crv = CagdCrvReverse(CARefCrv -> RefCrv -> XYCrv);
	    else
	        Crv = CagdCrvCopy(CARefCrv -> RefCrv -> XYCrv);

	    /* Merge current curve with next one: */
	    if (Merge) {
	        TCrv = CagdMergeCrvCrv(AllCrvs, Crv, FALSE);
		CagdCrvFree(Crv);
		CagdCrvFree(AllCrvs);
		AllCrvs = TCrv;
	    }
	    else {
	        AllCrvs = CagdListAppend(AllCrvs, Crv);
	    }
	}
	if (Merge)
	    PCrv = IPGenCrvObject("CACurves", AllCrvs, NULL);
	else {
	    int i;
	    IPObjectStruct *PTmp;

	    PCrv = IPLnkListToListObject(AllCrvs, IP_OBJ_CURVE);
	    IP_SET_OBJ_NAME2(PCrv, "CACurves");
	    for (i = 0; (PTmp = IPListObjectGet(PCrv, i++)) != NULL; ) {
	        char Name[IRIT_LINE_LEN];

		sprintf(Name, "CACurve%d", i);
		IP_SET_OBJ_NAME2(PTmp, Name);
	    }
	}
    }

    return PCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches the curves of the arrangement.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to process.                            M
*   XYCurves:     TRUE to return curves in XY plane, FASLE to recover the    M
*		  original orientation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Fetched curves.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntGetCurves                                                  M
*****************************************************************************/
CagdCrvStruct *UserCrvArngmntGetCurves(UserCrvArngmntStruct *CA, int XYCurves)
{
    int i;
    CagdCrvStruct *Crv,
        *Crvs = NULL;

    for (i = 0; i < CA -> NumOfCrvs; i++) {
	UserCACrvStruct
	    *CACrv = &CA -> Crvs[i];

        Crv = CagdCrvCopy(XYCurves ? CACrv -> XYCrv : CACrv -> Crv);

	IRIT_LIST_PUSH(Crv, Crvs);
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert internal regions as lists of curves, into merged Bspline curves. M
*   Output is placed on the Output slot of CA.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to process.                            M
*   Merge:        If TRUE, merge all curves in a region into one.            M
*   XYCurves:     TRUE to return regions in XY plane, FASLE to recover the   M
*		  original orientation.					     M
*   ZOffset:      if positive, offset the n'th region in Z by n*ZOffset.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntRegions2Curves 	                                     M
*****************************************************************************/
int UserCrvArngmntRegions2Curves(const UserCrvArngmntStruct *CA,
				 int Merge,
				 int XYCurves,
				 IrtRType ZOffset)
{
    char PlnStr[IRIT_LINE_LEN_LONG];
    int i;
    IPObjectStruct *PCrv,
        *PCrvList = IPGenListObject("CACurves", NULL, NULL);

    if (CA -> Output != NULL)
        IPFreeObject(((UserCrvArngmntStruct *) CA) -> Output);
    ((UserCrvArngmntStruct *) CA) -> Output = NULL;

    sprintf(PlnStr, "[%8.5f %8.5f %8.5f %8.5f]",
	    CA -> CrvsPlane[0], CA -> CrvsPlane[1],
	    CA -> CrvsPlane[2], CA -> CrvsPlane[3]);

    for (i = 0; i < CA -> NumOfRegions; i++) {
        PCrv = UserCrvArngmntRegion2Curves(CA, CA -> Regions[i] -> RefCrvs,
					   Merge);

	switch (CA -> Regions[i] -> Type) {
	    case USER_CA_SIMPLE_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "simple");
		break;
	    case USER_CA_LOOP_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "loop");
	        AttrSetObjectRealAttrib(PCrv, "area", CA -> Regions[i]
						                    -> Area);
		break;
	    case USER_CA_HANGING_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "hanging");
		break;
	    case USER_CA_COMPLEX_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "complex");
		break;
	    case USER_CA_LEFTOVERS_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "leftover");
		break;
	    default:
	        assert(0);
	    case USER_CA_UNDEF_TYPE:
	        AttrSetObjectStrAttrib(PCrv, "type", "undef");
		break;
	}
	AttrSetObjectStrAttrib(PCrv, "plane", PlnStr);
	AttrSetObjectIntAttrib(PCrv, "ContainedIn",
			       CA -> Regions[i] -> ContainedIn);

	if (IP_IS_OLST_OBJ(PCrv)) {
	    AttrPropagateAttr(PCrv, "type");
	    AttrPropagateAttr(PCrv, "plane");
	    AttrPropagateAttr(PCrv, "ContainedIn");
	}

	IPListObjectAppend(PCrvList, PCrv);
    }

    ((UserCrvArngmntStruct *) CA) -> Output = PCrvList;

    /* Map all curves to original 3-space position and/or offset. */
    if (!XYCurves || ZOffset > 0.0) {
	GlblMapCrvsXYCurves = XYCurves;
	GlblMapCrvsZOffset = ZOffset;
	GlblMapCrvsZOffsetCount = 0;

	IPTraverseObjHierarchy(((UserCrvArngmntStruct *) CA) -> Output, NULL,
			       MapCrvsInPlace,
			       ((UserCrvArngmntStruct *) CA) -> XY2XYZMat,
			       FALSE);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert internal regions into full topology as a list of two lists:      M
* 1. First list is the list of all curves in the arranments' output.         M
* 2. Second list is a list of entities each of which holds a list of indices M
*    into the first list.  Negative indices indicates the curve should be    M
*    reversed.  Index of first curve is 1.				     M
*   Output is placed on the Output slot of CA.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to process.			     M
*   XYCurves:     TRUE to return regions in XY plane, FASLE to recover the   M
*		  original orientation.					     M
*   ZOffset:      if positive, offset the n'th region in Z by n*ZOffset.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntRegionsTopology 	                                     M
*****************************************************************************/
int UserCrvArngmntRegionsTopology(const UserCrvArngmntStruct *CA,
				  int XYCurves,
				  IrtRType ZOffset)
{
    char Name[IRIT_LINE_LEN], PlnStr[IRIT_LINE_LEN_LONG];
    int i, j;
    IPObjectStruct *PRegion, *PIdx,
	*PCrv = NULL,
        *PCrvList = IPGenListObject("CACurves", NULL, NULL),
        *PRegionList =  IPGenListObject("CARegions", NULL, NULL);

    if (CA -> Output != NULL)
        IPFreeObject(((UserCrvArngmntStruct *) CA) -> Output);
    ((UserCrvArngmntStruct *) CA) -> Output = NULL;

    sprintf(PlnStr, "[%8.5f %8.5f %8.5f %8.5f]",
	    CA -> CrvsPlane[0], CA -> CrvsPlane[1],
	    CA -> CrvsPlane[2], CA -> CrvsPlane[3]);

    /* Copy all curves. */
    for (i = 0; i < CA -> NumOfCrvs; i++) {
	CagdCrvStruct
	    *Crv = XYCurves ? CagdCrvCopy(CA -> Crvs[i].XYCrv)
			    : CagdCrvCopy(CA -> Crvs[i].Crv);
	sprintf(Name, "CACrv%d", i);
	PCrv = IPGenCrvObject(Name, Crv, NULL);
	IPListObjectAppend(PCrvList, PCrv);
    }

    /* Copy all regions */
    for (i = 0; i < CA -> NumOfRegions; i++) {
	UserCARefCrvStruct
	    *CARefCrv  = CA -> Regions[i] -> RefCrvs;

	/* Collect indices of crvs of this region, negated if crv inverted. */
	sprintf(Name, "CARegion%d",i + 1);
	PRegion = IPGenListObject(Name, NULL, NULL);
	for (j = 0 ; CARefCrv != NULL; CARefCrv = CARefCrv -> Pnext) {
	    int Idx = CARefCrv -> RefCrv -> Idx + 1;
	    IrtRType
	        R = CARefCrv ->Inverted? -Idx : Idx;

	    sprintf(Name, "CARegion%d_Crv%d",i + 1, j++);
	    PIdx = IPGenNumObject(Name, &R, NULL);
	    IPListObjectAppend(PRegion, PIdx);
	}

	switch (CA -> Regions[i] -> Type) {
	    case USER_CA_SIMPLE_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "simple");
		break;
	    case USER_CA_LOOP_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "loop");
	        AttrSetObjectRealAttrib(PRegion, "area",
					CA -> Regions[i] -> Area);
		break;
	    case USER_CA_HANGING_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "hanging");
		break;
	    case USER_CA_COMPLEX_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "complex");
		break;
	    case USER_CA_LEFTOVERS_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "leftover");
		break;
	    default:
	        assert(0);
	    case USER_CA_UNDEF_TYPE:
	        AttrSetObjectStrAttrib(PRegion, "type", "undef");
		break;
	}
	AttrSetObjectStrAttrib(PRegion, "plane", PlnStr);
	AttrSetObjectIntAttrib(PRegion, "ContainedIn",
			       CA -> Regions[i] -> ContainedIn);

	AttrPropagateAttr(PRegion, "type");
	AttrPropagateAttr(PRegion, "plane");
	AttrPropagateAttr(PRegion, "ContainedIn");

	IPListObjectAppend(PRegionList, PRegion);
    }

    ((UserCrvArngmntStruct *) CA) -> Output =
        IPGenListObject("CA", PCrvList, NULL);
    IPListObjectAppend(((UserCrvArngmntStruct *) CA) -> Output, PRegionList);

    /* Map all curves to original 3-space position and/or offset. */
    if (!XYCurves || ZOffset > 0.0) {
	GlblMapCrvsXYCurves = XYCurves;
	GlblMapCrvsZOffset = ZOffset;
	GlblMapCrvsZOffsetCount = 0;

	IPTraverseObjHierarchy(PCrvList, NULL,
			       MapCrvsInPlace,
			       ((UserCrvArngmntStruct *) CA) -> XY2XYZMat,
			       FALSE);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back of IPTraverseObjHierarchy.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to transform according to Mat.                            *
*   Mat:    Transform to apply to the curve in PObj.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MapCrvsInPlace(IPObjectStruct *PObj, IrtHmgnMatType XYMat)
{
    if (IP_IS_CRV_OBJ(PObj)) {
        IrtHmgnMatType Mat;
        CagdCrvStruct *Crv;

	if (!GlblMapCrvsXYCurves && GlblMapCrvsZOffset > 0.0) {
	    MatGenMatTrans(0.0, 0.0,
			   GlblMapCrvsZOffset * GlblMapCrvsZOffsetCount++,
			   Mat);
	    MatMultTwo4by4(Mat, XYMat, Mat);
	}
	else if (!GlblMapCrvsXYCurves)
	    IRIT_HMGN_MAT_COPY(Mat, XYMat);
	else /* GlblMapCrvsZOffset > 0.0 */
	    MatGenMatTrans(0.0, 0.0,
			   GlblMapCrvsZOffset * GlblMapCrvsZOffsetCount++,
			   Mat);

	Crv = CagdCrvMatTransform(PObj -> U.Crvs, Mat);

	CagdCrvFree(PObj -> U.Crvs);
	PObj -> U.Crvs = Crv;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dumps to stdout, the content of the curve arrangement.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           To print to stdout.                                        M
*   DumpCurves:   1 to dump info crvs in the CA. 2 to also dump the crvs.    M
*   DumpPts:      TRUE to dump the points in the CA.			     M
*   DumpRegions:  TRUE to dump the regions in the CA.			     M
*   DumpXYData: TRUE to dump XY data, FALSE for 3-space data (crvs and pts). M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntReport                                                     M
*****************************************************************************/
void UserCrvArngmntReport(const UserCrvArngmntStruct *CA,
			  int DumpCurves,
			  int DumpPts,
			  int DumpRegions,
			  int DumpXYData)
{
    int i, j;

    printf("*****************************************************************\n");
    printf("Curve Arrangement (%d original curves): %d curves, %d pts, %d regions.\n",
	   CA -> NumOfOrigCrvs, CA -> NumOfCrvs, CA -> NumOfPts, CA -> NumOfRegions);
    printf("Pt-Pt Tol=%g, Planarity Tol=%g, Plane = [%.4f %.4f %.4f %.4f].\n",
	   CA -> EndPtEndPtTol, CA -> PlanarityTol, CA -> CrvsPlane[0],
	   CA -> CrvsPlane[1], CA -> CrvsPlane[2], CA -> CrvsPlane[3]);

    if (DumpCurves && CA -> Crvs != NULL) {
        for (i = 0; i < CA -> NumOfCrvs; i++) {
	    char *ErrStr;
	    UserCACrvStruct
	        *CACrv = &CA -> Crvs[i];

	    printf("Curve %d (Pts indices (%d, %d):\n", i,
		   CACrv -> RefStartPt -> Idx, CACrv -> RefEndPt -> Idx);

	    if (DumpCurves > 1) {
	        if (DumpXYData)
		    CagdCrvWriteToFile3(CACrv -> XYCrv, stdout, 4, "", &ErrStr);
		else
		    CagdCrvWriteToFile3(CACrv -> Crv, stdout, 4, "", &ErrStr);
	    }
	}
    }

    if (DumpPts && CA -> Pts != NULL) {
        for (i = 0; i < CA -> NumOfPts; i++) {
	    UserCAPtStruct
		*CAPt = &CA -> Pts[i];

	    if (DumpXYData)
	        printf("Point %d [%.5f %.5f], Curves:", i,
		       CAPt -> XYPt[0], CAPt -> XYPt[1]);
	    else
	        printf("Point %d [%.5f %.5f %.5f], Curves:", i,
		       CAPt -> Pt[0], CAPt -> Pt[1], CAPt -> Pt[2]);

	    if (CAPt -> Idx == USER_CA_PT_MERGED) {
	        printf(" Merged\n");
	    }
	    else if (CAPt -> Idx == USER_CA_PT_SELFLOOP) {
	        printf(" Self loop\n");
	    }
	    else {
	        for (j = 0; j < CAPt -> NumOfRefCrvs; j++)
		    printf(" %d", CAPt -> RefCrvs[j] -> Idx);
		printf("\n");
	    }
	}
    }

    if (DumpRegions && CA -> Regions != NULL) {
        for (i = 0; i < CA -> NumOfRegions; i++) {
	    const char *p;
	    UserCARefCrvStruct
	        *CARefCrv = CA -> Regions[i] -> RefCrvs;

	    switch (CA -> Regions[i] -> Type) {
	        default:
	        case USER_CA_UNDEF_TYPE:
		    p = "undef";
		    break;
	        case USER_CA_HANGING_TYPE:
		    p = "hanging";
		    break;
	        case USER_CA_SIMPLE_TYPE:
		    p = "simple";
		    break;
	        case USER_CA_LOOP_TYPE:
		    p = "loop";
		    break;
	        case USER_CA_COMPLEX_TYPE:
		    p = "complex";
		    break;
	        case USER_CA_LEFTOVERS_TYPE:
		    p = "leftover";
		    break;
	    }

	    printf("Rgn %2d: area = %6.4f, contained in loop %d, type = %s, ",
		   i, CA -> Regions[i] -> Area,
		   CA -> Regions[i] -> ContainedIn, p);

	    printf("Crvs: ");
	    for ( ; CARefCrv != NULL; CARefCrv = CARefCrv -> Pnext) {
	        printf(" %s%d", CARefCrv -> Inverted ? "-" : "",
		       CARefCrv -> RefCrv -> Idx);
	    }
	    printf("\n");
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back of IPTraverseObjHierarchy.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to evaluate (if a curve) at 1/13 of domain.               *
*   Mat:    Ignored.					                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EvalOrientationPoint(IPObjectStruct *PObj, IrtHmgnMatType XYMat)
{
    if (IP_IS_CRV_OBJ(PObj)) {
        CagdRType *R, TMin, TMax;
	CagdPType PtE3;
	char Name[IRIT_LINE_LEN_LONG];
	IPObjectStruct *PtObj;

        CagdCrvDomain(PObj -> U.Crvs, &TMin, &TMax);

	R = CagdCrvEval(PObj -> U.Crvs, 
			TMin * (12 / 13.0) + TMax * (1 / 13.0));
	CagdCoerceToE3(PtE3, &R, -1, PObj -> U.Crvs -> PType);

	sprintf(Name, "%sOrientPt", PObj -> ObjName);
	PtObj = IPGenPtObject(Name, &PtE3[0], &PtE3[1], &PtE3[2], NULL);
	IPListObjectAppend(GlblPointOrientationList, PtObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emit the result of the curve's arrangement computations.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   CA:           Curves' arrangement to output.                             M
*                 Output slot of CA will be updated with result.             M
*   OutputStyle:  1 for individual curve segments in each region (loop etc.),M
*                 2 for merged curves so every region is one curve,          M
*                 3 for topology as an ordered list of curve segments and    M
*                   each region is a list of indices into the first list.    M
*                     A negative -i index means index i but a reversed curve.M
*                 101, 102, 103 - same as 1,2,3 but a pt is evaluated at     M
*                 1/13 of curve parameteric domain to identify orientation.  M
*   Tolerance:  Tolerance of intersection locations' computation.	     *
*   ZOffset:      if positive, offset the n'th region in Z by n*ZOffset.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvArngmntOutput                                                     M
*****************************************************************************/
int UserCrvArngmntOutput(const UserCrvArngmntStruct *CA,
			 int OutputStyle,
			 CagdRType Tolerance,
			 CagdRType ZOffset)
{
    int RetVal,
        AddOrientPts = FALSE;

    if (OutputStyle > 100) {
        /* Add evaluated curve points at 1/13 of the domain to identify    */
        /* orientations of the output.				           */
        AddOrientPts = TRUE;
	OutputStyle -= 100;
    }

    switch (OutputStyle) {
        case 1:
	    RetVal = UserCrvArngmntRegions2Curves(CA, FALSE, FALSE, ZOffset);
	    break;
        case 2:
	    RetVal = UserCrvArngmntRegions2Curves(CA, TRUE, FALSE, ZOffset);
	    break;
        case 3:
	    RetVal = UserCrvArngmntRegionsTopology(CA, FALSE, Tolerance);
	    break;
        default:
	    return FALSE;
    }

    if (AddOrientPts &&
	RetVal &&
	CA -> Output != NULL &&
	IP_IS_OLST_OBJ(CA -> Output)) {
        IrtHmgnMatType UnitMat;
      
	MatGenUnitMat(UnitMat);

        GlblPointOrientationList = IPGenLISTObject(NULL);
        IPTraverseObjHierarchy(CA -> Output, NULL,
			       EvalOrientationPoint,
			       UnitMat,
			       FALSE);

	IPListObjectAppend(CA -> Output, GlblPointOrientationList);
	GlblPointOrientationList = NULL;
    }

    return RetVal;
}
