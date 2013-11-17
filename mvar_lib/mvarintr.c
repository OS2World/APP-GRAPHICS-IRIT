/******************************************************************************
* MvarIntr.c - intersections and self-intersections functionality.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 2006.					      *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"

static MvarPtStruct *MvarCrvCrvInterET(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol);
static MvarPtStruct *MvarCrvCrvInterMV(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol);

static MvarPtStruct *MvarSrfSrfInterET(const CagdSrfStruct *Srf1,
				       const CagdSrfStruct *Srf2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol);
static MvarPtStruct *MvarSrfSrfInterMV(const CagdSrfStruct *Srf1,
				       const CagdSrfStruct *Srf2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol);

static MvarPtStruct *MvarSrfSrfSrfInterET(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2,
					  const CagdSrfStruct *Srf3,
					  CagdRType SubdivTol,
					  CagdRType NumericTol);
static MvarPtStruct *MvarSrfSrfSrfInterMV(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2,
					  const CagdSrfStruct *Srf3,
					  CagdRType SubdivTol,
					  CagdRType NumericTol);

static MvarPtStruct *MvarSrfSrfContactET(const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 const CagdCrvStruct *Srf1Motion,
					 const CagdCrvStruct *Srf1Scale,
					 CagdRType SubdivTol,
					 CagdRType NumericTol);
static MvarPtStruct *MvarSrfSrfContactMV(const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 const CagdCrvStruct *Srf1Motion,
					 const CagdCrvStruct *Srf1Scale,
					 CagdRType SubdivTol,
					 CagdRType NumericTol);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection locations of two planar curves, possibly with  M
* multivariate expression trees.  Expression trees could be beneficial	     M
* comptationally when the geometry is complex (i.e. dozens of control points M
* or more).							             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   Two curves to intersect.                                   M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*   UseExprTree:  TRUE to use expression trees in the computation, FALSE to  M
*		  use regular multivariate expressions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:     List of intersection points, as parameter pairs into M
*			the two curves domains.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvCrvInter, SymbCrvCrvInter, SymbSrfSrfSrfInter, MvarSrfSrfContact  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvCrvInter                                                          M
*****************************************************************************/
MvarPtStruct *MvarCrvCrvInter(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2,
			      CagdRType SubdivTol,
			      CagdRType NumericTol,
			      CagdBType UseExprTree)
{
    return UseExprTree ? MvarCrvCrvInterET(Crv1, Crv2, SubdivTol, NumericTol)
		       : MvarCrvCrvInterMV(Crv1, Crv2, SubdivTol, NumericTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of two planar curves, using          *
* multivariates' expression trees.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:   Two curves to intersect.                                   *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:     List of intersection points, as parameter pairs into *
*			the two curves domains.   Points in R^2.             *
*****************************************************************************/
static MvarPtStruct *MvarCrvCrvInterET(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol)
{
    CagdCrvStruct *TCrv, *Crv1W, *Crv1X, *Crv1Y, *Crv1Z,
			 *Crv2W, *Crv2X, *Crv2Y, *Crv2Z;
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[2];
    MvarExprTreeStruct *MVETs[2];

    SymbCrvSplitScalar(Crv1, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);
    SymbCrvSplitScalar(Crv2, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);

    if (Crv1Z != NULL)
	CagdCrvFree(Crv1Z);
    if (Crv2Z != NULL)
	CagdCrvFree(Crv2Z);

    if (Crv1Y == NULL || Crv2Y == NULL) {
        MVAR_FATAL_ERROR(MVAR_ERR_ONLY_2D);
	return NULL;
    }

    /* Handle rational curves if indeed rational. */
    if (Crv1W != NULL) {
        TCrv = SymbCrvMergeScalar(Crv1W, Crv1X, NULL, NULL);
	CagdCrvFree(Crv1X);
	Crv1X = TCrv;
        TCrv = SymbCrvMergeScalar(Crv1W, Crv1Y, NULL, NULL);
	CagdCrvFree(Crv1Y);
	Crv1Y = TCrv;
	CagdCrvFree(Crv1W);
    }

    if (Crv2W != NULL) {
        TCrv = SymbCrvMergeScalar(Crv2W, Crv2X, NULL, NULL);
	CagdCrvFree(Crv2X);
	Crv2X = TCrv;
        TCrv = SymbCrvMergeScalar(Crv2W, Crv2Y, NULL, NULL);
	CagdCrvFree(Crv2Y);
	Crv2Y = TCrv;
	CagdCrvFree(Crv2W);
    }

    /* Convert into multivariates and build the expression trees. */
    MVETs[0] = MvarExprTreeSub(MvarExprTreeFromCrv(Crv1X, 2, 0),
			       MvarExprTreeFromCrv(Crv2X, 2, 1));
    MVETs[1] = MvarExprTreeSub(MvarExprTreeFromCrv(Crv1Y, 2, 0),
			       MvarExprTreeFromCrv(Crv2Y, 2, 1));

    CagdCrvFree(Crv1X);
    CagdCrvFree(Crv1Y);
    CagdCrvFree(Crv2X);
    CagdCrvFree(Crv2Y);

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarExprTreesZeros(MVETs, Constraints, 2, SubdivTol, NumericTol);

    MvarExprTreeFree(MVETs[0], FALSE);
    MvarExprTreeFree(MVETs[1], FALSE);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of two planar curves, using          *
* multivariates' regular MV expression.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:   Two curves to intersect.                                   *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:     List of intersection points, as parameter pairs into *
*			the two curves domains.  Points in R^2.              *
*****************************************************************************/
static MvarPtStruct *MvarCrvCrvInterMV(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol)
{
    int i;
    CagdRType Min, Max;
    CagdCrvStruct *Crvs1[4], *Crvs2[4];
    MvarMVStruct *MVs[2], *TMV, *MVs1[3], *MVs2[3];
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[2];

    SymbCrvSplitScalar(Crv1, &Crvs1[0], &Crvs1[1], &Crvs1[2], &Crvs1[3]);
    SymbCrvSplitScalar(Crv2, &Crvs2[0], &Crvs2[1], &Crvs2[2], &Crvs2[3]);

    if (Crvs1[3] != NULL)
	CagdCrvFree(Crvs1[3]);
    if (Crvs2[3] != NULL)
	CagdCrvFree(Crvs2[3]);

    if (Crvs1[2] == NULL || Crvs2[2] == NULL) {
        MVAR_FATAL_ERROR(MVAR_ERR_ONLY_2D);
	return NULL;
    }

    /* Convert to multivariates. */
    MVs1[0] = NULL;
    for (i = Crvs1[0] == NULL; i < 3; i++) {
	TMV = MvarCrvToMV(Crvs1[i]);
	CagdCrvFree(Crvs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 2, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = Crvs2[0] == NULL; i < 3; i++) {
	TMV = MvarCrvToMV(Crvs2[i]);
	CagdCrvFree(Crvs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 2, 1);
	MvarMVFree(TMV);
    }

    /* Make domains consistent. */
    MvarMVDomain(MVs1[1], &Min, &Max, 0);
    if (!IRIT_APX_EQ(Min, 0.0) || !IRIT_APX_EQ(Max, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = MVs2[0] == NULL; i < 3; i++)
	    MVs2[i] = MvarMVSetDomain(MVs2[i], Min, Max, 0, TRUE);
    }

    MvarMVDomain(MVs2[1], &Min, &Max, 1);
    if (!IRIT_APX_EQ(Min, 0.0) || !IRIT_APX_EQ(Max, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = MVs1[0] == NULL; i < 3; i++)
            MVs1[i] = MvarMVSetDomain(MVs1[i], Min, Max, 1, TRUE);
    }

    /* Build the X/Y equality constraint. */
    MVs[0] = MvarMVRtnlMult(MVs1[1], MVs1[0], MVs2[1], MVs2[0], FALSE);
    MVs[1] = MvarMVRtnlMult(MVs1[2], MVs1[0], MVs2[2], MVs2[0], FALSE);

    for (i = MVs1[0] == NULL; i < 3; i++)
	MvarMVFree(MVs1[i]);
    for (i = MVs2[0] == NULL; i < 3; i++)
	MvarMVFree(MVs2[i]);

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumericTol);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection locations of two surfaces in R^3, possibly     M
* with multivariate expression trees.  Expression trees could be beneficial  M
* comptationally when the geometry is complex (i.e. dozens of control points M
* or more, in each directions).					             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:    Two surface to intersect in R^3.                          M
*   SubdivTol:     Tolerance of the solution.  This tolerance is measured    M
*		   in the parametric space of the surfaces.		     M
*   NumericTol:    Numeric tolerance of a possible numeric improvment        M
*		   stage.  The numeric stage is employed if		     M
*		   NumericTol < SubdivTol.				     M
*   UseExprTree:   TRUE to use expression trees in the computation,          M
*		   FALSE to use regular multivariate expressions.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:     List of intersection points, as parameter pairs into M
*			the two surfaces domains.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvInter, MvarSrfSrfSrfInter, MvarSrfSrfContact                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfInter                                                          M
*****************************************************************************/
MvarPtStruct *MvarSrfSrfInter(const CagdSrfStruct *Srf1,
			      const CagdSrfStruct *Srf2,
			      CagdRType SubdivTol,
			      CagdRType NumericTol,
			      CagdBType UseExprTree)
{
    return UseExprTree ? MvarSrfSrfInterET(Srf1, Srf2,
					   SubdivTol, NumericTol)
		       : MvarSrfSrfInterMV(Srf1, Srf2,
					   SubdivTol, NumericTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of two surfaces, using               *
* multivariates' expression trees.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:   The two surfaces to intersect.                             *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    pairs into the two surfaces' domains. Points in R^4.     *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfInterET(const CagdSrfStruct *Srf1,
				       const CagdSrfStruct *Srf2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol)
{
    int i;
    CagdSrfStruct *TSrf, *Srf1W, *Srf1X, *Srf1Y, *Srf1Z,
			 *Srf2W, *Srf2X, *Srf2Y, *Srf2Z;
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[3];
    MvarExprTreeStruct *MVETs[3];

    SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
    SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);

    /* Handle rational curves if indeed rational. */
    if (Srf1W != NULL) {
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1X, NULL, NULL);
	CagdSrfFree(Srf1X);
	Srf1X = TSrf;
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1Y, NULL, NULL);
	CagdSrfFree(Srf1Y);
	Srf1Y = TSrf;
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1Z, NULL, NULL);
	CagdSrfFree(Srf1Z);
	Srf1Z = TSrf;
	CagdSrfFree(Srf1W);
    }
    if (Srf2W != NULL) {
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2X, NULL, NULL);
	CagdSrfFree(Srf2X);
	Srf2X = TSrf;
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2Y, NULL, NULL);
	CagdSrfFree(Srf2Y);
	Srf2Y = TSrf;
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2Z, NULL, NULL);
	CagdSrfFree(Srf2Z);
	Srf2Z = TSrf;
	CagdSrfFree(Srf2W);
    }


    /* Convert into multivariates and build the expression trees. */
    MVETs[0] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1X, 4, 0),
			       MvarExprTreeFromSrf(Srf2X, 4, 2));
    MVETs[1] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Y, 4, 0),
			       MvarExprTreeFromSrf(Srf2Y, 4, 2));
    MVETs[2] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Z, 4, 0),
			       MvarExprTreeFromSrf(Srf2Z, 4, 2));

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf1Z);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(Srf2Z);

    /* Solve the constraints. */
    for (i = 0; i < 3; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarExprTreesZeros(MVETs, Constraints, 3, SubdivTol, NumericTol);

    for (i = 0; i < 3; i++)
        MvarExprTreeFree(MVETs[i], FALSE);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of two surfaces, using		     *
* multivariates' regular MV expression.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:   The two surfaces to intersect.                             *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    pairs into the two surfaces' domains. Points in R^4.     *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfInterMV(const CagdSrfStruct *Srf1,
				       const CagdSrfStruct *Srf2,
				       CagdRType SubdivTol,
				       CagdRType NumericTol)
{
    int i;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *Srfs1[4], *Srfs2[4];
    MvarMVStruct *MVs[6], *TMV, *MVs1[4], *MVs2[4];
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[6];

    SymbSrfSplitScalar(Srf1, &Srfs1[0], &Srfs1[1], &Srfs1[2], &Srfs1[3]);
    SymbSrfSplitScalar(Srf2, &Srfs2[0], &Srfs2[1], &Srfs2[2], &Srfs2[3]);

    /* Convert to multivariates. */
    MVs1[0] = NULL;
    for (i = Srfs1[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs1[i]);
	CagdSrfFree(Srfs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 4, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = Srfs2[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs2[i]);
	CagdSrfFree(Srfs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 4, 2);
	MvarMVFree(TMV);
    }

    /* Make domains consistent. */
    MvarMVDomain(MVs1[1], &UMin, &UMax, 0);
    MvarMVDomain(MVs1[1], &VMin, &VMax, 1);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, 1.0) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = 0; i < 4; i++) {
	    if (MVs2[i] != NULL) {
	        MVs2[i] = MvarMVSetDomain(MVs2[i], UMin, UMax, 0, TRUE);
		MVs2[i] = MvarMVSetDomain(MVs2[i], VMin, VMax, 1, TRUE);
	    }
	}
    }
    MvarMVDomain(MVs2[1], &UMin, &UMax, 2);
    MvarMVDomain(MVs2[1], &VMin, &VMax, 3);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, 1.0) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = 0; i < 4; i++) {
	    if (MVs1[i] != NULL) {
	        MVs1[i] = MvarMVSetDomain(MVs1[i], UMin, UMax, 2, TRUE);
		MVs1[i] = MvarMVSetDomain(MVs1[i], VMin, VMax, 3, TRUE);
	    }
	}
    }

    /* Build the Srf1 == Srf2 equality constraint. */
    MVs[0] = MvarMVRtnlMult(MVs1[1], MVs1[0], MVs2[1], MVs2[0], FALSE);
    MVs[1] = MvarMVRtnlMult(MVs1[2], MVs1[0], MVs2[2], MVs2[0], FALSE);
    MVs[2] = MvarMVRtnlMult(MVs1[3], MVs1[0], MVs2[3], MVs2[0], FALSE);

    for (i = 0; i < 4; i++) {
        if (MVs1[i] != NULL)
	    MvarMVFree(MVs1[i]);
        if (MVs2[i] != NULL)
	    MvarMVFree(MVs2[i]);
    }

    /* Solve the constraints. */
    for (i = 0; i < 3; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 3, SubdivTol, NumericTol);

    for (i = 0; i < 3; i++)
        MvarMVFree(MVs[i]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection locations of three surfaces in R^3, possibly   M
* with multivariate expression trees.  Expression trees could be beneficial  M
* comptationally when the geometry is complex (i.e. dozens of control points M
* or more, in each directions).					             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2, Srf3: Three surface to intersect in R^3.                     M
*   SubdivTol:        Tolerance of the solution.  This tolerance is measured M
*		      in the parametric space of the surfaces.		     M
*   NumericTol:       Numeric tolerance of a possible numeric improvment     M
*		      stage.  The numeric stage is employed if		     M
*		      NumericTol < SubdivTol.				     M
*   UseExprTree:      TRUE to use expression trees in the computation,       M
*		      FALSE to use regular multivariate expressions.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:     List of intersection points, as parameter pairs into M
*			the three surfaces domains.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvInter, MvarSrfSrfInter, MvarSrfSrfContact	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfSrfInter                                                       M
*****************************************************************************/
MvarPtStruct *MvarSrfSrfSrfInter(const CagdSrfStruct *Srf1,
				 const CagdSrfStruct *Srf2,
				 const CagdSrfStruct *Srf3,
				 CagdRType SubdivTol,
				 CagdRType NumericTol,
				 CagdBType UseExprTree)
{
    return UseExprTree ? MvarSrfSrfSrfInterET(Srf1, Srf2, Srf3,
					      SubdivTol, NumericTol)
		       : MvarSrfSrfSrfInterMV(Srf1, Srf2, Srf3,
					      SubdivTol, NumericTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of three surfaces, using             *
* multivariates' expression trees.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2, Srf3:  The three surfaces to intersect.                      *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    triples into the three surfaces' domains. Points in R^6. *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfSrfInterET(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2,
					  const CagdSrfStruct *Srf3,
					  CagdRType SubdivTol,
					  CagdRType NumericTol)
{
    int i;
    CagdSrfStruct *TSrf, *Srf1W, *Srf1X, *Srf1Y, *Srf1Z,
			 *Srf2W, *Srf2X, *Srf2Y, *Srf2Z,
			 *Srf3W, *Srf3X, *Srf3Y, *Srf3Z;
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[6];
    MvarExprTreeStruct *MVETs[6];

    SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
    SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);
    SymbSrfSplitScalar(Srf3, &Srf3W, &Srf3X, &Srf3Y, &Srf3Z);

    /* Handle rational curves if indeed rational. */
    if (Srf1W != NULL) {
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1X, NULL, NULL);
	CagdSrfFree(Srf1X);
	Srf1X = TSrf;
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1Y, NULL, NULL);
	CagdSrfFree(Srf1Y);
	Srf1Y = TSrf;
        TSrf = SymbSrfMergeScalar(Srf1W, Srf1Z, NULL, NULL);
	CagdSrfFree(Srf1Z);
	Srf1Z = TSrf;
	CagdSrfFree(Srf1W);
    }
    if (Srf2W != NULL) {
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2X, NULL, NULL);
	CagdSrfFree(Srf2X);
	Srf2X = TSrf;
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2Y, NULL, NULL);
	CagdSrfFree(Srf2Y);
	Srf2Y = TSrf;
        TSrf = SymbSrfMergeScalar(Srf2W, Srf2Z, NULL, NULL);
	CagdSrfFree(Srf2Z);
	Srf2Z = TSrf;
	CagdSrfFree(Srf2W);
    }
    if (Srf3W != NULL) {
        TSrf = SymbSrfMergeScalar(Srf3W, Srf3X, NULL, NULL);
	CagdSrfFree(Srf3X);
	Srf3X = TSrf;
        TSrf = SymbSrfMergeScalar(Srf3W, Srf3Y, NULL, NULL);
	CagdSrfFree(Srf3Y);
	Srf3Y = TSrf;
        TSrf = SymbSrfMergeScalar(Srf3W, Srf3Z, NULL, NULL);
	CagdSrfFree(Srf3Z);
	Srf3Z = TSrf;
	CagdSrfFree(Srf3W);
    }

    /* Convert into multivariates and build the expression trees. */
    MVETs[0] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1X, 6, 0),
			       MvarExprTreeFromSrf(Srf2X, 6, 2));
    MVETs[1] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Y, 6, 0),
			       MvarExprTreeFromSrf(Srf2Y, 6, 2));
    MVETs[2] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Z, 6, 0),
			       MvarExprTreeFromSrf(Srf2Z, 6, 2));

    MVETs[3] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1X, 6, 0),
			       MvarExprTreeFromSrf(Srf3X, 6, 4));
    MVETs[4] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Y, 6, 0),
			       MvarExprTreeFromSrf(Srf3Y, 6, 4));
    MVETs[5] = MvarExprTreeSub(MvarExprTreeFromSrf(Srf1Z, 6, 0),
			       MvarExprTreeFromSrf(Srf3Z, 6, 4));

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf1Z);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(Srf2Z);
    CagdSrfFree(Srf3X);
    CagdSrfFree(Srf3Y);
    CagdSrfFree(Srf3Z);

    /* Solve the constraints. */
    for (i = 0; i < 6; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarExprTreesZeros(MVETs, Constraints, 6, SubdivTol, NumericTol);

    for (i = 0; i < 6; i++)
        MvarExprTreeFree(MVETs[i], FALSE);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmputes the intersection locations of three surfaces, using	     *
* multivariates' regular MV expression.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2, Srf3:  The three surfaces to intersect.                      *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    triples into the three surfaces' domains. Points in R^6. *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfSrfInterMV(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2,
					  const CagdSrfStruct *Srf3,
					  CagdRType SubdivTol,
					  CagdRType NumericTol)
{
    int i;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *Srfs1[4], *Srfs2[4], *Srfs3[4];
    MvarMVStruct *MVs[6], *TMV, *MVs1[4], *MVs2[4], *MVs3[4];
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[6];

    SymbSrfSplitScalar(Srf1, &Srfs1[0], &Srfs1[1], &Srfs1[2], &Srfs1[3]);
    SymbSrfSplitScalar(Srf2, &Srfs2[0], &Srfs2[1], &Srfs2[2], &Srfs2[3]);
    SymbSrfSplitScalar(Srf3, &Srfs3[0], &Srfs3[1], &Srfs3[2], &Srfs3[3]);

    /* Convert to multivariates. */
    MVs1[0] = NULL;
    for (i = Srfs1[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs1[i]);
	CagdSrfFree(Srfs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 6, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = Srfs2[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs2[i]);
	CagdSrfFree(Srfs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 6, 2);
	MvarMVFree(TMV);
    }
    MVs3[0] = NULL;
    for (i = Srfs3[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs3[i]);
	CagdSrfFree(Srfs3[i]);
	MVs3[i] = MvarPromoteMVToMV2(TMV, 6, 4);
	MvarMVFree(TMV);
    }

    /* Make domains consistent. */
    MvarMVDomain(MVs1[1], &UMin, &UMax, 0);
    MvarMVDomain(MVs1[1], &VMin, &VMax, 1);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, 1.0) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = 0; i < 4; i++) {
	    if (MVs2[i] != NULL) {
	        MVs2[i] = MvarMVSetDomain(MVs2[i], UMin, UMax, 0, TRUE);
		MVs2[i] = MvarMVSetDomain(MVs2[i], VMin, VMax, 1, TRUE);
	    }
	    if (MVs3[i] != NULL) {
	        MVs3[i] = MvarMVSetDomain(MVs3[i], UMin, UMax, 0, TRUE);
		MVs3[i] = MvarMVSetDomain(MVs3[i], VMin, VMax, 1, TRUE);
	    }
	}
    }
    MvarMVDomain(MVs2[1], &UMin, &UMax, 2);
    MvarMVDomain(MVs2[1], &VMin, &VMax, 3);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, 1.0) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = 0; i < 4; i++) {
	    if (MVs1[i] != NULL) {
	        MVs1[i] = MvarMVSetDomain(MVs1[i], UMin, UMax, 2, TRUE);
		MVs1[i] = MvarMVSetDomain(MVs1[i], VMin, VMax, 3, TRUE);
	    }
	    if (MVs3[i] != NULL) {
	        MVs3[i] = MvarMVSetDomain(MVs3[i], UMin, UMax, 2, TRUE);
		MVs3[i] = MvarMVSetDomain(MVs3[i], VMin, VMax, 3, TRUE);
	    }
	}
    }
    MvarMVDomain(MVs3[1], &UMin, &UMax, 4);
    MvarMVDomain(MVs3[1], &VMin, &VMax, 5);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, 1.0) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, 1.0)) {
	/* Do nothing for [0, 1] domain as it is the default and more over  */
	/* Beziers do not have knot sequences to update...		    */
        for (i = 0; i < 4; i++) {
	    if (MVs1[i] != NULL) {
	        MVs1[i] = MvarMVSetDomain(MVs1[i], UMin, UMax, 4, TRUE);
		MVs1[i] = MvarMVSetDomain(MVs1[i], VMin, VMax, 5, TRUE);
	    }
	    if (MVs2[i] != NULL) {
	        MVs2[i] = MvarMVSetDomain(MVs2[i], UMin, UMax, 4, TRUE);
		MVs2[i] = MvarMVSetDomain(MVs2[i], VMin, VMax, 5, TRUE);
	    }
	}
    }

    /* Build the Srf1 == Srf2 equality constraint. */
    MVs[0] = MvarMVRtnlMult(MVs1[1], MVs1[0], MVs2[1], MVs2[0], FALSE);
    MVs[1] = MvarMVRtnlMult(MVs1[2], MVs1[0], MVs2[2], MVs2[0], FALSE);
    MVs[2] = MvarMVRtnlMult(MVs1[3], MVs1[0], MVs2[3], MVs2[0], FALSE);

    /* Build the Srf1 == Srf3 equality constraint. */
    MVs[3] = MvarMVRtnlMult(MVs1[1], MVs1[0], MVs3[1], MVs3[0], FALSE);
    MVs[4] = MvarMVRtnlMult(MVs1[2], MVs1[0], MVs3[2], MVs3[0], FALSE);
    MVs[5] = MvarMVRtnlMult(MVs1[3], MVs1[0], MVs3[3], MVs3[0], FALSE);

    for (i = 0; i < 4; i++) {
        if (MVs1[i] != NULL)
	    MvarMVFree(MVs1[i]);
        if (MVs2[i] != NULL)
	    MvarMVFree(MVs2[i]);
        if (MVs3[i] != NULL)
	    MvarMVFree(MVs3[i]);
    }

    /* Solve the constraints. */
    for (i = 0; i < 6; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 6, SubdivTol, NumericTol);

    for (i = 0; i < 6; i++)
        MvarMVFree(MVs[i]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the contact locations of two C^1 surfaces in R^3, possibly      M
* with multivariate expression trees.  Expression trees could be beneficial  M
* comptationally when the geometry is complex (i.e. dozens of control points M
* or more, in each directions).					             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:    Two surface to compute contacts over time in R^3.         M
*   MotionSrf1:    The motion over time Srf1 undergoes.  Can be NULL.        M
*   ScaleSrf1:     The scale over time Srf1 undergoes.  Can either be a      M
*		   scalar function, or vector function in R^3.  Can be NULL. M
*                  If both MotionSrf1 and Srf1Scale are defined, they better M
*		   share their domains.					     M
*   SubdivTol:     Tolerance of the solution.  This tolerance is measured    M
*		   in the parametric space of the surfaces.		     M
*   NumericTol:    Numeric tolerance of a possible numeric improvment        M
*		   stage.  The numeric stage is employed if		     M
*		   NumericTol < SubdivTol.				     M
*   UseExprTree:   TRUE to use expression trees in the computation,          M
*		   FALSE to use regular multivariate expressions.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:     List of intersection points, as parameter pairs into M
*			the two surfaces domains.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvInter, MvarSrfSrfInter, MvarSrfSrfSrfInter                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSrfContact                                                        M
*****************************************************************************/
MvarPtStruct *MvarSrfSrfContact(const CagdSrfStruct *Srf1,
				const CagdSrfStruct *Srf2,
				const CagdCrvStruct *MotionSrf1,
				const CagdCrvStruct *ScaleSrf1,
				CagdRType SubdivTol,
				CagdRType NumericTol,
				CagdBType UseExprTree)
{
    return UseExprTree ? MvarSrfSrfContactET(Srf1, Srf2, MotionSrf1,
					     ScaleSrf1, SubdivTol, NumericTol)
		       : MvarSrfSrfContactMV(Srf1, Srf2, MotionSrf1,
					     ScaleSrf1, SubdivTol, NumericTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the contact locations of two surfaces, using	             *
* multivariates' expression trees.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:   The two surfaces to computes contacts over time.           *
*   MotionSrf1:   The motion over time Srf1 undergoes.  Can be NULL.         *
*   Srf1Scale:    The scale over time Srf1 undergoes.  Can either be a       *
*		  scalar function, or vector function in R^3.  Can be NULL.  *
*                 If both MotionSrf1 and Srf1Scale are defined, they better  *
*		  share their domains.					     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    pairs into the two surfaces' domains. Points in R^4.     *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfContactET(const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 const CagdCrvStruct *MotionSrf1,
					 const CagdCrvStruct *ScaleSrf1,
					 CagdRType SubdivTol,
					 CagdRType NumericTol)
{
    int i;
    CagdRType Min[5], Max[5];
    MvarMVStruct *TMV, *TMV1, *TMV2, *TMV3, *MV1, *MV1Split[MVAR_MAX_PT_SIZE],
      *MV2, *MV2Split[MVAR_MAX_PT_SIZE];
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[6];
    MvarExprTreeStruct *MVETs[6], *MVETs1[4], *MVETs2[4], *MVETTmp1;

    if (CAGD_IS_BEZIER_SRF(Srf1) || CAGD_IS_BEZIER_SRF(Srf2)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BSPLINE_EXPECTED);
	return NULL;
    }

    /* Convert to multivariates. */
    TMV = MvarSrfToMV(Srf1);
    MV1 = MvarPromoteMVToMV2(TMV, 5, 0);
    MvarMVFree(TMV);

    TMV = MvarSrfToMV(Srf2);
    MV2 = MvarPromoteMVToMV2(TMV, 5, 2);
    MvarMVFree(TMV);

    /* Make domains consistent. */
    MvarMVDomain(MV1, &Min[0], &Max[0], 0);
    MvarMVDomain(MV1, &Min[1], &Max[1], 1);
    MvarMVDomain(MV2, &Min[2], &Max[2], 2);
    MvarMVDomain(MV2, &Min[3], &Max[3], 3);
    if (ScaleSrf1 != NULL)
        CagdCrvDomain(ScaleSrf1, &Min[4], &Max[4]);
    else if (MotionSrf1 != NULL)
        CagdCrvDomain(MotionSrf1, &Min[4], &Max[4]);
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_CRV);
	return NULL;
    }

    /* Set the domains and compute derivatives/normal field. Note that the  */
    /* scale or translate of Srf1 will not affect its derivatives/normals.  */
    MV1 = MvarMVSetAllDomains(MV1, Min, Max, TRUE);
    MV2 = MvarMVSetAllDomains(MV2, Min, Max, TRUE);

    TMV1 = MvarMVDerive(MV1, 0);
    TMV2 = MvarMVDerive(MV1, 1);
    TMV = MvarMVCrossProd(TMV1, TMV2);
    MvarMVFree(TMV1);
    MvarMVFree(TMV2);

    TMV1 = MvarMVDerive(MV2, 2);
    TMV2 = MvarMVDerive(MV2, 3);
    TMV3 = MvarMVCrossProd(TMV1, TMV2);

    /* Compute the last two tangential constraint. */
    MVETs[3] = MvarExprTreeDotProd(MvarExprTreeFromMV(TMV, 5, 0),
				   MvarExprTreeFromMV(TMV1, 5, 0));
    MVETs[4] = MvarExprTreeDotProd(MvarExprTreeFromMV(TMV, 5, 0),
				   MvarExprTreeFromMV(TMV2, 5, 0));
    MVETs[5] = MvarExprTreeDotProd(MvarExprTreeFromMV(TMV, 5, 0),
				   MvarExprTreeFromMV(TMV3, 5, 0));

    MvarMVFree(TMV);
    MvarMVFree(TMV1);
    MvarMVFree(TMV2);
    MvarMVFree(TMV3);

    /* Split the two into scalar functions and convert to expression trees. */
    IRIT_GEN_COPY(MV1Split, MvarMVSplitScalar(MV1),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    IRIT_GEN_COPY(MV2Split, MvarMVSplitScalar(MV2),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MvarMVFree(MV1);
    MvarMVFree(MV2);

    for (i = 0; i < 4; i++) {
        if (MV1Split[i] != NULL) {
	    MVETs1[i] = MvarExprTreeFromMV(MV1Split[i], 5, 0);
	    MvarMVFree(MV1Split[i]);
	}
	else
	    MVETs1[i] = NULL;

        if (MV2Split[i] != NULL) {
	    MVETs2[i] = MvarExprTreeFromMV(MV2Split[i], 5, 0);
	    MvarMVFree(MV2Split[i]);
	}
	else
	    MVETs2[i] = NULL;
    }

    /* If we have a scale function over time - apply it. */
    if (ScaleSrf1 != NULL) {
	CagdCrvStruct *SclSplit[4];

        SymbCrvSplitScalar(ScaleSrf1, &SclSplit[0], &SclSplit[1],
			              &SclSplit[2], &SclSplit[3]);

	/* If rational. */
	if (SclSplit[0] != NULL || MVETs1[0] != NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	    return NULL;
	}

	if (SclSplit[2] == NULL) {
	    /* Uniform scale. */
	    TMV = MvarCrvToMV(SclSplit[1]);
	    TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
	    TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
	    MVETTmp1 = MvarExprTreeFromMV(TMV2, 5, 0);
	    MvarMVFree(TMV);
	    MvarMVFree(TMV2);
	    for (i = 0; i < 4; i++) {
	        if (MVETs1[i] != NULL) {
		    MVETs1[i] = MvarExprTreeMult(MVETs1[i],
				     MvarExprTreeCopy(MVETTmp1, FALSE, TRUE));
		}
	    }
	    MvarExprTreeFree(MVETTmp1, FALSE);
	}
	else if (MVETs1[3] == NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_1D_OR_3D);
	    return NULL;
	}
	else {
	    /* Non-uniform scale. */
	    for (i = 1; i < 4; i++) {
	        TMV = MvarCrvToMV(SclSplit[i]);
		TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
		TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
		MVETTmp1 = MvarExprTreeFromMV(TMV2, 5, 0);
		MvarMVFree(TMV);
		MvarMVFree(TMV2);

		MVETs1[i] = MvarExprTreeMult(MVETs1[i], MVETTmp1);
	    }
	}

	for (i = 0; i < 4; i++)
	    if (SclSplit[i] != NULL)
	        CagdCrvFree(SclSplit[i]);
    }

    /* If we have a translate function over time - apply it. */
    if (MotionSrf1 != NULL) {
	CagdCrvStruct *MtnSplit[4];

        SymbCrvSplitScalar(MotionSrf1, &MtnSplit[0], &MtnSplit[1],
				       &MtnSplit[2], &MtnSplit[3]);

	/* If rational. */
	if (MtnSplit[0] != NULL || MVETs1[0] != NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	    return NULL;
	}
	else if (MtnSplit[3] == NULL || MVETs[3] == NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_ONLY_3D);
	    return NULL;
	}

	/* Apply the motion. */
	for (i = 1; i < 4; i++) {
	    TMV = MvarCrvToMV(MtnSplit[i]);
	    TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
	    TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
	    MVETTmp1 = MvarExprTreeFromMV(TMV2, 5, 0);

	    MvarMVFree(TMV);
	    MvarMVFree(TMV2);

	    MVETs1[i] = MvarExprTreeAdd(MVETs1[i], MVETTmp1);
	}

	for (i = 0; i < 4; i++)
	    if (MtnSplit[i] != NULL)
	        CagdCrvFree(MtnSplit[i]);
    }

    /* Build the Srf1 == Srf2 equality constraint, in XYZ. */
    MVETs[0] = MvarExprTreeSub(MVETs1[1], MVETs2[1]);
    MVETs[1] = MvarExprTreeSub(MVETs1[2], MVETs2[2]);
    MVETs[2] = MvarExprTreeSub(MVETs1[3], MVETs2[3]);

    /* Solve the constraints, adding the tangential constraints on the fly. */
    for (i = 0; i < 5; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    Constraints[5] = MVAR_CNSTRNT_NEGATIVE;

    MVPts = MvarExprTreesZeros(MVETs, Constraints, 6, SubdivTol, NumericTol);

    for (i = 0; i < 6; i++)
        MvarExprTreeFree(MVETs[i], FALSE);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the contact locations of two surfaces, S1 and S2, using         *
* multivariates' regular MV expression.  Motion curve is by M.		     *
*   5 equations (and 5 knowns) for M[S1] == S2 (3 equations) and	     *
* <N1, dS2/du> and <N1, dS2/dv>.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:   The two surfaces to compute contacts over time.            *
*   MotionSrf1:   The motion over time Srf1 undergoes.  Can be NULL.         *
*   ScaleSrf1:    The scale over time Srf1 undergoes.  Can either be a       *
*		  scalar function, or vector function in R^3.  Can be NULL.  *
*                 If both MotionSrf1 and ScaleSrf1 are defined, they better  *
*		  share their domains.					     *
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  *
*		  the parametric space of the surfaces.			     *
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  *
*		  The numeric stage is employed if NumericTol < SubdivTol.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         *
*		    pairs into the two surfaces' domains. Points in R^4.     *
*****************************************************************************/
static MvarPtStruct *MvarSrfSrfContactMV(const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 const CagdCrvStruct *MotionSrf1,
					 const CagdCrvStruct *ScaleSrf1,
					 CagdRType SubdivTol,
					 CagdRType NumericTol)
{
    int i;
    CagdRType Min[5], Max[5];
    CagdSrfStruct *Srfs1[4], *Srfs2[4];
    MvarMVStruct *MVs[6], *TMV, *DuTMV, *DvTMV, *TMV2, *TMV3,
	*MVs1[4], *MVs2[4], *ScalarMVs[MVAR_MAX_PT_COORD + 1];
    MvarPtStruct *MVPts;
    MvarConstraintType Constraints[6];

    if (CAGD_IS_BEZIER_SRF(Srf1) || CAGD_IS_BEZIER_SRF(Srf2)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BSPLINE_EXPECTED);
	return NULL;
    }

    IRIT_ZAP_MEM(ScalarMVs, sizeof(MvarMVStruct *) * (MVAR_MAX_PT_COORD + 1));

    SymbSrfSplitScalar(Srf1, &Srfs1[0], &Srfs1[1], &Srfs1[2], &Srfs1[3]);
    SymbSrfSplitScalar(Srf2, &Srfs2[0], &Srfs2[1], &Srfs2[2], &Srfs2[3]);

    /* Convert to multivariates. */
    MVs1[0] = NULL;
    for (i = Srfs1[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs1[i]);
	CagdSrfFree(Srfs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 5, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = Srfs2[0] == NULL; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs2[i]);
	CagdSrfFree(Srfs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 5, 2);
	MvarMVFree(TMV);
    }

    /* Make domains consistent. */
    MvarMVDomain(MVs1[1], &Min[0], &Max[0], 0);
    MvarMVDomain(MVs1[1], &Min[1], &Max[1], 1);
    MvarMVDomain(MVs2[1], &Min[2], &Max[2], 2);
    MvarMVDomain(MVs2[1], &Min[3], &Max[3], 3);
    if (ScaleSrf1 != NULL)
        CagdCrvDomain(ScaleSrf1, &Min[4], &Max[4]);
    else if (MotionSrf1 != NULL)
        CagdCrvDomain(MotionSrf1, &Min[4], &Max[4]);
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_CRV);
	return NULL;
    }

    for (i = 0; i < 4; i++) {
	if (MVs1[i] != NULL)
	    MVs1[i] = MvarMVSetAllDomains(MVs1[i], Min, Max, TRUE);
	if (MVs2[i] != NULL)
	    MVs2[i] = MvarMVSetAllDomains(MVs2[i], Min, Max, TRUE);
    }

    /* If we have a scale function over time - apply it. */
    if (ScaleSrf1 != NULL) {
	CagdCrvStruct *SclSplit[4];

        SymbCrvSplitScalar(ScaleSrf1, &SclSplit[0], &SclSplit[1],
				      &SclSplit[2], &SclSplit[3]);
	if (SclSplit[2] == NULL) {
	    /* Uniform scale. */
	    TMV = MvarCrvToMV(SclSplit[1]);
	    TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
	    MvarMVFree(TMV);
	    for (i = 1; i < 4; i++) {
	        TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
		TMV = MvarMVMult(MVs1[i], TMV2);
		MvarMVFree(MVs1[i]);
		MVs1[i] = TMV;
	    }
	    MvarMVFree(TMV2);
	}
	else if (Srfs1[3] == NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_1D_OR_3D);
	    return NULL;
	}
	else {
	    /* Non-uniform scale. */
	    for (i = 1; i < 4; i++) {
	        TMV = MvarCrvToMV(SclSplit[i]);
		TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
		MvarMVFree(TMV);
	        TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
		TMV = MvarMVMult(MVs1[i], TMV2);
		MvarMVFree(TMV2);
		MvarMVFree(MVs1[i]);
		MVs1[i] = TMV;
	    }
	}

	/* If rational. */
	if (SclSplit[0] != NULL && MVs1[0] != NULL) {
	    TMV2 = MvarCrvToMV(SclSplit[0]);
	    TMV = MvarPromoteMVToMV2(TMV2, 5, 4);
	    MvarMVFree(TMV2);
	    TMV = MvarMVSetAllDomains(TMV, Min, Max, TRUE);
	    TMV2 = MvarMVMult(MVs1[0], TMV);
	    MvarMVFree(TMV);
	    MvarMVFree(MVs1[0]);
	    MVs1[i] = TMV2;
	}
	else if (SclSplit[0] != NULL) {
	    TMV = MvarCrvToMV(SclSplit[0]);
	    MVs1[i] = MvarPromoteMVToMV2(TMV, 5, 4);
	    MvarMVFree(TMV);
	    MVs[1] = MvarMVSetAllDomains(MVs1[i], Min, Max, TRUE);
	}

	for (i = 0; i < 4; i++)
	    if (SclSplit[i] != NULL)
	        CagdCrvFree(SclSplit[i]);
    }

    /* If we have a translate function over time - apply it. */
    if (MotionSrf1 != NULL) {
	CagdCrvStruct *MtnSplit[4];

        SymbCrvSplitScalar(MotionSrf1, &MtnSplit[0], &MtnSplit[1],
				       &MtnSplit[2], &MtnSplit[3]);

	/* If rational. */
	if (MtnSplit[0] != NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
	    return NULL;
	}
	else if (MtnSplit[3] == NULL || MVs1[3] == NULL) {
	    MVAR_FATAL_ERROR(MVAR_ERR_ONLY_3D);
	    return NULL;
	}

	/* Apply the motion. */
	for (i = 1; i < 4; i++) {
	    TMV = MvarCrvToMV(MtnSplit[i]);
	    TMV2 = MvarPromoteMVToMV2(TMV, 5, 4);
	    MvarMVFree(TMV);
	    TMV2 = MvarMVSetAllDomains(TMV2, Min, Max, TRUE);
	    if (MVs1[0] != NULL) {
	        TMV = MvarMVMult(MVs1[0], TMV2);
		MvarMVFree(TMV2);
		TMV2 = TMV;
	    }
	    TMV = MvarMVAdd(MVs1[i], TMV2);
	    MvarMVFree(TMV2);
	    MvarMVFree(MVs1[i]);
	    MVs1[i] = TMV;
	}

	for (i = 0; i < 4; i++)
	    if (MtnSplit[i] != NULL)
	        CagdCrvFree(MtnSplit[i]);
    }

    /* Build the Srf1 == Srf2 equality constraint. */
    MVs[0] = MvarMVRtnlMult(MVs1[1], MVs1[0], MVs2[1], MVs2[0], FALSE);
    MVs[1] = MvarMVRtnlMult(MVs1[2], MVs1[0], MVs2[2], MVs2[0], FALSE);
    MVs[2] = MvarMVRtnlMult(MVs1[3], MVs1[0], MVs2[3], MVs2[0], FALSE);

    /* Build the tangency contact constraints. */
    ScalarMVs[0] = NULL;
    ScalarMVs[1] = MVs1[1];
    ScalarMVs[2] = MVs1[2];
    ScalarMVs[3] = MVs1[3];
    ScalarMVs[4] = NULL;
    TMV = MvarMVMergeScalar(ScalarMVs);
    DuTMV = MvarMVDerive(TMV, 0);
    DvTMV = MvarMVDerive(TMV, 1);
    MvarMVFree(TMV);
    TMV = MvarMVCrossProd(DuTMV, DvTMV);
    MvarMVFree(DuTMV);
    MvarMVFree(DvTMV);

    ScalarMVs[0] = NULL;
    ScalarMVs[1] = MVs2[1];
    ScalarMVs[2] = MVs2[2];
    ScalarMVs[3] = MVs2[3];
    ScalarMVs[4] = NULL;
    TMV2 = MvarMVMergeScalar(ScalarMVs);
    DuTMV = MvarMVDerive(TMV2, 2);
    DvTMV = MvarMVDerive(TMV2, 3);
    MvarMVFree(TMV2);
    TMV3 = MvarMVCrossProd(DuTMV, DvTMV);

    MVs[3] = MvarMVDotProd(TMV, DuTMV);
    MVs[4] = MvarMVDotProd(TMV, DvTMV);
    MVs[5] = MvarMVDotProd(TMV, TMV3);
    MvarMVFree(DuTMV);
    MvarMVFree(DvTMV);
    MvarMVFree(TMV);
    MvarMVFree(TMV3);

    for (i = 0; i < 4; i++) {
        if (MVs1[i] != NULL)
	    MvarMVFree(MVs1[i]);
        if (MVs2[i] != NULL)
	    MvarMVFree(MVs2[i]);
    }

    /* Solve the constraints. */
    for (i = 0; i < 5; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    Constraints[5] = MVAR_CNSTRNT_NEGATIVE;

    MVPts = MvarMVsZeros(MVs, Constraints, 6, SubdivTol, NumericTol);

#ifdef DEBUG
    MvarMVsZerosVerifier(MVs, 5, MVPts, NumericTol);
#endif /* DEBUG */

    for (i = 0; i < 6; i++)
        MvarMVFree(MVs[i]);

    return MVPts;
}
