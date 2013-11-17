/******************************************************************************
* A marching tool to march along trivariate's iso surfaces.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*						Gershon Elber, Mar 1997.      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "mrchcube.h"
#include "triv_loc.h"

#define MAX_ITERS		20    /* Max number of numerical iterations. */
#define PT_COVER_TOL		1e-5	  /* Accuracy of point set coverage. */
#define MAXIMUM_CONTOUR_DEVIATE 1e-1   /* Max deviation long contouring dir. */
#define SIL_DIR_GRAD_EPS	1e-7
#define SIL_STROKE_BASE		10       /* Strokes emphasizing silhouettes. */
#define SIL_STROKE_TYPE(StrokeType)	((StrokeType) >= SIL_STROKE_BASE && \
					 (StrokeType) < (SIL_STROKE_BASE + 10))

IRIT_STATIC_DATA TrivTVStruct
     *GlblTV = NULL,
     *GlblTVGradient[3] = { NULL, NULL, NULL };
IRIT_STATIC_DATA CagdBType
     GlblHaveGradient = FALSE;
IRIT_STATIC_DATA CagdPType GlblMinDomain, GlblMaxDomain;

static IPVertexStruct *TracePrincipalDirOnIsoSrf(CagdRType IsoVal,
						 CagdRType *Pos,
						 CagdBType DoMaximalCurv,
						 CagdBType Forward,
						 CagdRType Length,
						 CagdRType StepSize);
static IPVertexStruct *TraceConstantAxisOnIsoSrf(CagdRType IsoVal,
						 CagdRType *Pos,
						 int Dir,
						 CagdBType Forward,
						 CagdRType Length,
						 CagdRType StepSize);
static IPVertexStruct *TraceSilhouetteLinesOnIsoSrf(CagdRType IsoVal,
						    CagdRType *Pos,
						    int Sils,
						    CagdBType Forward,
						    CagdRType Length,
						    CagdRType StepSize,
						    CagdVType ViewDir);
static CagdCrvStruct *ChainTraceIntoCurve(IPVertexStruct *V1,
					  IPVertexStruct *V2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release all allocated auxiliary trivariate derivatives, for fast         M
* marching.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCImprovePointOnIsoSrfPrelude, MCImprovePointOnIsoSrf		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCImprovePointOnIsoSrfPostlude                                           M
*****************************************************************************/
void MCImprovePointOnIsoSrfPostlude(void)
{
    int i;

    if (GlblTV != NULL) {
	TrivTVFree(GlblTV);
	GlblTV = NULL;
    }

    for (i = 0; i < 3; i++) {
        if (GlblTVGradient[i] != NULL) {
	    TrivTVFree(GlblTVGradient[i]);
	    GlblTVGradient[i] = NULL;
	}
    }

    GlblHaveGradient = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepare the necessary derivative vector fields of TV for fast marching   M
* on the trivariate, later on.			                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       to process and prepare for further iso surface marching.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCImprovePointOnIsoSrfPostlude, MCImprovePointOnIsoSrf		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCImprovePointOnIsoSrfPrelude                                            M
*****************************************************************************/
CagdBType MCImprovePointOnIsoSrfPrelude(const TrivTVStruct *TV)
{
    /* In case of something from previous evaluation that must freed. */
    TrivEvalTVCurvaturePostlude();

    if (CAGD_NUM_OF_PT_COORD(TV -> PType) != 1) {
        TRIV_FATAL_ERROR(TRIV_ERR_SCALAR_PT_EXPECTED);
        return FALSE;
    }

    GlblTV = TrivTVCopy(TV);

    GlblHaveGradient =
	 /* Compute first order derivatives (the Gradient) */
        ((GlblTVGradient[0] = TrivTVDerive(TV, TRIV_CONST_U_DIR)) != NULL &&
	 (GlblTVGradient[1] = TrivTVDerive(TV, TRIV_CONST_V_DIR)) != NULL &&
	 (GlblTVGradient[2] = TrivTVDerive(TV, TRIV_CONST_W_DIR)) != NULL);

    TrivTVDomain(GlblTV, &GlblMinDomain[0], &GlblMaxDomain[0],
		         &GlblMinDomain[1], &GlblMaxDomain[1],
		         &GlblMinDomain[2], &GlblMaxDomain[2]);

    return GlblHaveGradient;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Improves a given Pt to "sit" exactly on top of the iso surface, by       M
* along the gradient of the trivariate.  Assume TV has been preprocessed     M
* by MCImprovePointOnIsoSrfPrelude.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Position to improve.                                         M
*   CubeDim:    Size of a single cell in the trivariate volume.              M
*   IsoVal:     Of iso surface extracted from TV that Pt is approximately on.M
*   Tolerance:  Requested accuracy.					     M
*   AllowedError:  Maximally allowed error to be considered valid. If zero   M
*               it is ignored.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCImprovePointOnIsoSrfPrelude, MCImprovePointOnIsoSrfPostlude,           M
*   MCExtractIsoSurface2                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCImprovePointOnIsoSrf                                                   M
*****************************************************************************/
int MCImprovePointOnIsoSrf(IrtPtType Pt,
			   const IrtPtType CubeDim,
			   CagdRType IsoVal,
			   CagdRType Tolerance,
			   CagdRType AllowedError)
{
    int i,
	NumIters = 0;
    CagdRType
	OrigError = 0.0;

    if (!GlblHaveGradient)
	return FALSE;

    for (i = 0; i < 3; i++)
        Pt[i] *= CubeDim[i];

    do {
	CagdVType Grad;
	CagdRType Val2, Step,
	    *R = TrivTVEval(GlblTV, Pt[0], Pt[1], Pt[2]),
	    Val1 = R[1],
	    Err = IRIT_FABS(Val1 - IsoVal);

	if (AllowedError > IRIT_UEPS && Err > AllowedError)
	    return FALSE;

	if (NumIters == 0)
	    OrigError = Err;

	if (Err < Tolerance) {
	    for (i = 0; i < 3; i++)
	        Pt[i] /= CubeDim[i];

	    return TRUE;
	}

	if (NumIters > 0 && Err > OrigError)
	    return FALSE;

	/* Compute the gradient. */
	for (i = 0; i < 3; i++) {
	    R = TrivTVEval(GlblTVGradient[i], Pt[0], Pt[1], Pt[2]);
	    Grad[i] = R[1];
	}
	IRIT_PT_NORMALIZE(Grad);

	/* Evaluate the trivariate epsilon along the gradient: */
	R = TrivTVEval(GlblTV, Pt[0] + Grad[0] * IRIT_EPS,
		               Pt[1] + Grad[1] * IRIT_EPS,
			       Pt[2] + Grad[2] * IRIT_EPS);
	Val2 = R[1];

	if (Val1 == Val2)
	    return FALSE;
	else
	    Step = IRIT_EPS * (Val1 - IsoVal) / (Val1 - Val2);

	for (i = 0; i < 3; i++) {
	    Pt[i] += Grad[i] * Step;
	    if (Pt[i] < GlblMinDomain[i])
		Pt[i] = GlblMinDomain[i] + IRIT_EPS;
	    if (Pt[i] > GlblMaxDomain[i])
		Pt[i] = GlblMaxDomain[i] - IRIT_EPS;
	}
    }
    while (NumIters++ < MAX_ITERS);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a coverage of an iso surface at IsoVal of the trivariate TV     M
* using curves along principal curvatures.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CTV:           To cover with strokes along principal curvatures of iso   M
*		   surface of value IsoVal.				     M
*   NumStrokes:    Number of strokes to distribute on the implicit surface.  M
*   StrokeType:    1 - Draw strokes along minimal principal curvature.	     M
*		   2 - Draw strokes along maximal principal curvature.	     M
*		   3 - Draw strokes along both principal curvatures.	     M
*		   4 - Draw strokes along constant X planes.		     M
*		   5 - Draw strokes along constant Y planes.		     M
*		   6 - Draw strokes along constant Z planes.		     M
*                  7 - Draw strokes along silhouette lines.		     M
*                  8 - Draw strokes orthogonal to silhouette lines.	     M
*                  9 - Draw strokes along both silhouette lines and lines    M
*		       orthogonal to silhouette lines.			     M
*		     StrokesType >= 10 equals StrokeType < 10 but also       M
*		   emphasizes the silhouette areas setting longer edges      M
*		   along silhouettes.					     M
*   MinMaxPwrLen:  Arc length of each stroke (randomized in between).	     M
*		   a triplet of the form (Min, Max, Power) that determines   M
*		   the length of each stroke as 			     M
*			Avg = (Max + Min) / 2,      Dev = (Max - Min) / 2    V
*		        Length = Avg + Dev * Random(0, 1)^ Pwr		     V
*   StepSize:	   Steps to take in the piecewise linear approximation.      M
*   IsoVal:        Of iso surface of TV that coverage is to be computed for. M
*   ViewDir:	   Direction of view, used for silhouette computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A list of curves forming the coverage or NULL if error. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCmprovePointOnIsoSrf, MCExtractIsoSurface2                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCoverIsoSurfaceUsingStrokes                                          M
*****************************************************************************/
CagdCrvStruct *TrivCoverIsoSurfaceUsingStrokes(TrivTVStruct *CTV,
					       int NumStrokes,
					       int StrokeType,
					       CagdPType MinMaxPwrLen,
					       CagdRType StepSize,
					       CagdRType IsoVal,
					       CagdVType ViewDir)
{
    IRIT_STATIC_DATA IrtPtType
	CubeDim = { 1.0, 1.0, 1.0 };
    CagdBType IsSilStroke;
    IPObjectStruct *PolyObj, *PtsObj;
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax,
	AvgLength = (MinMaxPwrLen[1] + MinMaxPwrLen[0]) * 0.5,
	DevLength = (MinMaxPwrLen[1] - MinMaxPwrLen[0]) * 0.5;
    IPVertexStruct *V;
    CagdCrvStruct
	*Coverage = NULL;
    TrivTVStruct *TV;

    /* Make sure the domain is consistent with the data size. */
    TV = TrivTVCopy(CTV);
    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);
    if (!IRIT_APX_EQ(UMin, 0.0) || !IRIT_APX_EQ(UMax, TV -> ULength - 1) ||
	!IRIT_APX_EQ(VMin, 0.0) || !IRIT_APX_EQ(VMax, TV -> VLength - 1) ||
	!IRIT_APX_EQ(WMin, 0.0) || !IRIT_APX_EQ(WMax, TV -> WLength - 1)) {
	BspKnotAffineTrans2(TV -> UKnotVector, TV -> ULength + TV -> UOrder,
			    0.0, TV -> ULength - 1.0);
	BspKnotAffineTrans2(TV -> VKnotVector, TV -> VLength + TV -> VOrder,
			    0.0, TV -> VLength - 1.0);
	BspKnotAffineTrans2(TV -> WKnotVector, TV -> WLength + TV -> WOrder,
			    0.0, TV -> WLength - 1.0);
    }

    if ((PolyObj = MCExtractIsoSurface2(TV, 1, FALSE,
					CubeDim, 1, 1.0, IsoVal)) == NULL) {
	TrivTVFree(TV);
	return NULL;
    }

    PtsObj = GMPointCoverOfPolyObj(PolyObj, NumStrokes, NULL, NULL);

    IPFreeObject(PolyObj);

    /* Improve upon the accuracy of the covering points. */
    if (!MCImprovePointOnIsoSrfPrelude(TV)) {
	IPFreeObject(PtsObj);
	TrivTVFree(TV);
	return NULL;
    }

    for (V = PtsObj -> U.Pl -> PVertex; V != NULL; V = V -> Pnext) {
	if (!(GlblMinDomain[0] <= V -> Coord[0] &&
	      GlblMaxDomain[0] >= V -> Coord[0] &&
	      GlblMinDomain[1] <= V -> Coord[1] &&
	      GlblMaxDomain[1] >= V -> Coord[1] &&
	      GlblMinDomain[2] <= V -> Coord[2] &&
	      GlblMaxDomain[2] >= V -> Coord[2]))
	    /* Out of the parametric domain. */
	    V -> Coord[0] = IRIT_INFNTY;        /* Mark as invalid location. */
	else
	    MCImprovePointOnIsoSrf(V -> Coord, CubeDim, IsoVal,
				   PT_COVER_TOL, 0.0);
    }

    /* For each such point, it is time to trace the strokes. */
    if (SIL_STROKE_TYPE(StrokeType)) {
	IsSilStroke = TRUE;
	StrokeType -= SIL_STROKE_BASE;
    }
    else {
	IsSilStroke = TRUE;
    }

    if ((StrokeType == 1 || StrokeType == 2 || StrokeType == 3) &&
	!TrivEvalTVCurvaturePrelude(TV)) {
	IPFreeObject(PtsObj);
	TrivTVFree(TV);
	return NULL;
    }

    for (V = PtsObj -> U.Pl -> PVertex; V != NULL; V = V -> Pnext) {
	CagdRType Length;
	IPVertexStruct *V1, *V2;
	CagdCrvStruct *Crv;

	if (V -> Coord[0] == IRIT_INFNTY)
	    continue;

	if (IsSilStroke) {
	    int i;
	    CagdVType Grad;
	    CagdRType *R;

	    /* Compute the gradient. */
	    for (i = 0; i < 3; i++) {
	        R = TrivTVEval(GlblTVGradient[i],
			       V -> Coord[0], V -> Coord[1], V -> Coord[2]);
		Grad[i] = R[1];
	    }
	    IRIT_PT_NORMALIZE(Grad);

	    Length = AvgLength + DevLength *
			    pow(1.0 - IRIT_FABS(IRIT_DOT_PROD(Grad, ViewDir)),
				MinMaxPwrLen[2]);
	}
	else {
	    Length = AvgLength + DevLength * pow(IritRandom(0.0, 1.0),
							     MinMaxPwrLen[2]);
	}

	if (Length < 2 * StepSize + IRIT_EPS)
	    Length = 2 * StepSize + IRIT_EPS;

	switch (StrokeType) {
	    case 1:
	        /* Trace a stroke along the minimal principal curvature. */
		V1 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, FALSE,
					       TRUE, Length * 0.5, StepSize);
		V2 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, FALSE,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 2:
		/* Trace a stroke along the maximal principal curvature. */
		V1 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, TRUE,
					       TRUE, Length * 0.5, StepSize);
		V2 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, TRUE,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 3:
	        /* Trace a stroke along the minimal principal curvature. */
		V1 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, FALSE,
					       TRUE, Length * 0.5, StepSize);
		V2 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, FALSE,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		/* Trace a stroke along the maximal principal curvature. */
		V1 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, TRUE,
					       TRUE, Length * 0.5, StepSize);
		V2 = TracePrincipalDirOnIsoSrf(IsoVal, V -> Coord, TRUE,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 4:
		/* Trace strokes along constant X planes. */
		V1 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 1,
					       TRUE, Length * 0.5, StepSize);
		V2 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 1,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 5:
		/* Trace strokes along constant X planes. */
		V1 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 2,
					       TRUE, Length * 0.5, StepSize);
		V2 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 2,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 6:
		/* Trace strokes along constant X planes. */
		V1 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 3,
					       TRUE, Length * 0.5, StepSize);
		V2 = TraceConstantAxisOnIsoSrf(IsoVal, V -> Coord, 3,
					       FALSE, Length * 0.5, StepSize);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 7:
		/* Trace strokes along Silhouette lines. */
		V1 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  TRUE, TRUE, Length * 0.5,
						  StepSize, ViewDir);
		V2 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  TRUE, FALSE, Length * 0.5,
						  StepSize, ViewDir);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 8:
		/* Trace strokes along lines orthogonal to Silhouette lines. */
		V1 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  FALSE, TRUE, Length * 0.5,
						  StepSize, ViewDir);
		V2 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  FALSE, FALSE, Length * 0.5,
						  StepSize, ViewDir);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    case 9:
		/* Trace strokes along Silhouette lines. */
		V1 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  TRUE, TRUE, Length * 0.5,
						  StepSize, ViewDir);
		V2 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  TRUE, FALSE, Length * 0.5,
						  StepSize, ViewDir);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);

		/* Trace strokes along lines orthogonal to Silhouette lines. */
		V1 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  FALSE, TRUE, Length * 0.5,
						  StepSize, ViewDir);
		V2 = TraceSilhouetteLinesOnIsoSrf(IsoVal, V -> Coord,
						  FALSE, FALSE, Length * 0.5,
						  StepSize, ViewDir);
		if ((Crv = ChainTraceIntoCurve(V1, V2)) != NULL)
		    IRIT_LIST_PUSH(Crv, Coverage);
		break;
	    default:
		MCImprovePointOnIsoSrfPostlude();
		TrivEvalTVCurvaturePostlude();
		TRIV_FATAL_ERROR(TRIV_ERR_INVALID_STROKE_TYPE);
		IPFreeObject(PtsObj);
		TrivTVFree(TV);
		return NULL;
	}
    }

    IPFreeObject(PtsObj);
    TrivTVFree(TV);

    MCImprovePointOnIsoSrfPostlude();
    if (StrokeType == 1 || StrokeType == 2 || StrokeType == 3)
	TrivEvalTVCurvaturePostlude();

    return Coverage;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traces along an iso surface along a principal direction.                 *
* Assumes TrivEvalTVCurvaturePrelude has been invoked before for this TV.    *
*                                                                            *
* PARAMETERS:                                                                *
*   IsoVal:    Of iso surface of TV that trace is computed for.		     *
*   Pos:       Starting position.                                            *
*   DoMaximalCurv:     TRUE for (absolute) maximal curvature, FALSE for      *
*		minimal curvature.					     *
*   Forward:    TRUE for principal direction, FALSE for opposite direction.  *
*   Length:     Of stroke to trace.                                          *
*   StepSize:   To take at a time, (Length/StepSize steps overwhole).        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   List of points on iso surface forming the trace.     *
*****************************************************************************/
static IPVertexStruct *TracePrincipalDirOnIsoSrf(CagdRType IsoVal,
						 CagdRType *Pos,
						 CagdBType DoMaximalCurv,
						 CagdBType Forward,
						 CagdRType Length,
						 CagdRType StepSize)
{
    IRIT_STATIC_DATA CagdPType CrntPos,
	CubeDim = { 1.0, 1.0, 1.0 };
    IPVertexStruct
	*VList = NULL;

    IRIT_PT_COPY(CrntPos, Pos);
    do {
	CagdRType PCurv1, PCurv2;
	CagdVType PDir1, PDir2, Dir;

	VList = IPAllocVertex2(VList);
	IRIT_PT_COPY(VList -> Coord, CrntPos);

	if (!TrivEvalCurvature(CrntPos, &PCurv1, &PCurv2, PDir1, PDir2)) {
	    IPFreeVertexList(VList);
	    return NULL;
	}

	if (DoMaximalCurv) {
	    if (PCurv1 > PCurv2)
		IRIT_PT_COPY(Dir, PDir1);
	    else
		IRIT_PT_COPY(Dir, PDir2);
	}
	else {
	    if (PCurv1 > PCurv2)
		IRIT_PT_COPY(Dir, PDir2);
	    else
		IRIT_PT_COPY(Dir, PDir1);
	}
	IRIT_PT_NORMALIZE(Dir);
	if (Forward)
	    IRIT_PT_SCALE(Dir, StepSize)
	else
	    IRIT_PT_SCALE(Dir, -StepSize);

	IRIT_PT_ADD(CrntPos, CrntPos, Dir);
	if (!(GlblMinDomain[0] <= CrntPos[0] &&
	      GlblMaxDomain[0] >= CrntPos[0] &&
	      GlblMinDomain[1] <= CrntPos[1] &&
	      GlblMaxDomain[1] >= CrntPos[1] &&
	      GlblMinDomain[2] <= CrntPos[2] &&
	      GlblMaxDomain[2] >= CrntPos[2]) ||  /* Exit parametric domain. */
	    !MCImprovePointOnIsoSrf(CrntPos, CubeDim, IsoVal,
				    PT_COVER_TOL, IRIT_FABS(IsoVal / 10.0)))
	    return VList;

	Length -= StepSize;
    }
    while (Length > 0.0);

    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traces along an iso surface along a principal direction.                 *
* Assumes TrivEvalTVCurvaturePrelude has been invoked before for this TV.    *
*                                                                            *
* PARAMETERS:                                                                *
*   IsoVal:    Of iso surface of TV that trace is computed for.		     *
*   Pos:       Starting position.                                            *
*   Dir:       1 for constant X, 2 for constant Y, 3 for constant Z.	     *
*   Forward:   TRUE for principal direction, FALSE for opposite direction.   *
*   Length:    Of stroke to trace.                                           *
*   StepSize:  To take at a time, (Length/StepSize steps overwhole).         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   List of points on iso surface forming the trace.     *
*****************************************************************************/
static IPVertexStruct *TraceConstantAxisOnIsoSrf(CagdRType IsoVal,
						 CagdRType *Pos,
						 int Dir,
						 CagdBType Forward,
						 CagdRType Length,
						 CagdRType StepSize)
{
    IRIT_STATIC_DATA CagdPType
	CubeDim = { 1.0, 1.0, 1.0 };
    CagdPType CrntPos;
    CagdVType AxisDir;
    IPVertexStruct
	*VList = NULL;

    IRIT_PT_RESET(AxisDir);
    AxisDir[--Dir] = 1.0;

    IRIT_PT_COPY(CrntPos, Pos);
    do {
	int i;
	CagdRType *R;
	CagdVType Grad, AdvanceDir;

	VList = IPAllocVertex2(VList);
	IRIT_PT_COPY(VList -> Coord, CrntPos);

	/* Compute the gradient. */
	for (i = 0; i < 3; i++) {
	    R = TrivTVEval(GlblTVGradient[i],
			   CrntPos[0], CrntPos[1], CrntPos[2]);
	    Grad[i] = R[1];
	}
	IRIT_PT_NORMALIZE(Grad);

	IRIT_CROSS_PROD(AdvanceDir, Grad, AxisDir);
	if (IRIT_PT_LENGTH(AdvanceDir) < IRIT_EPS)
	    return VList;
	IRIT_PT_NORMALIZE(AdvanceDir);
	if (Forward)
	    IRIT_PT_SCALE(AdvanceDir, StepSize)
	else
	    IRIT_PT_SCALE(AdvanceDir, -StepSize);

	IRIT_PT_ADD(CrntPos, CrntPos, AdvanceDir);
	if (!(GlblMinDomain[0] <= CrntPos[0] &&
	      GlblMaxDomain[0] >= CrntPos[0] &&
	      GlblMinDomain[1] <= CrntPos[1] &&
	      GlblMaxDomain[1] >= CrntPos[1] &&
	      GlblMinDomain[2] <= CrntPos[2] &&
	      GlblMaxDomain[2] >= CrntPos[2]) ||  /* Exit parametric domain. */
	    !MCImprovePointOnIsoSrf(CrntPos, CubeDim, IsoVal,
				    PT_COVER_TOL, IRIT_FABS(IsoVal / 10.0)))
	    return VList;

	if (!IRIT_APX_EQ_EPS(CrntPos[Dir], Pos[Dir],
			MAXIMUM_CONTOUR_DEVIATE))
	    return VList;

	Length -= StepSize;
    }
    while (Length > 0.0);

    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traces along an iso surface along a principal direction.                 *
* Assumes TrivEvalTVCurvaturePrelude has been invoked before for this TV.    *
*                                                                            *
* PARAMETERS:                                                                *
*   IsoVal:    Of iso surface of TV that trace is computed for.		     *
*   Pos:       Starting position.                                            *
*   Sils:      TRUE for silhouettes, FALSE lines orthogonal to silhouettes.  *
*	       silhouette lines.	                                     *
*   Forward:   TRUE for principal direction, FALSE for opposite direction.   *
*   Length:    Of stroke to trace.                                           *
*   StepSize:  To take at a time, (Length/StepSize steps overwhole).         *
*   ViewDir:   Direction of view, used for silhouette computation.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   List of points on iso surface forming the trace.     *
*****************************************************************************/
static IPVertexStruct *TraceSilhouetteLinesOnIsoSrf(CagdRType IsoVal,
						    CagdRType *Pos,
						    int Sils,
						    CagdBType Forward,
						    CagdRType Length,
						    CagdRType StepSize,
						    CagdVType ViewDir)
{
    IRIT_STATIC_DATA CagdPType
	CubeDim = { 1.0, 1.0, 1.0 };
    CagdPType CrntPos;
    IPVertexStruct
	*VList = NULL;

    IRIT_PT_COPY(CrntPos, Pos);
    do {
	int i;
	CagdRType *R, GradViewOrig, GradDView1, GradDView2;
	CagdVType TmpGrad, Grad, AdvanceDir, AdvanceDir2;

	VList = IPAllocVertex2(VList);
	IRIT_PT_COPY(VList -> Coord, CrntPos);

	/* Compute the gradient. */
	for (i = 0; i < 3; i++) {
	    R = TrivTVEval(GlblTVGradient[i],
			   CrntPos[0], CrntPos[1], CrntPos[2]);
	    Grad[i] = R[1];
	}
	IRIT_PT_NORMALIZE(Grad);

	/* Find the projection of the viewing direction on the iso-surface's */
	/* tangent plane.						     */
	GradViewOrig = IRIT_DOT_PROD(Grad, ViewDir);
	IRIT_CROSS_PROD(AdvanceDir, ViewDir, Grad);
	if (IRIT_PT_LENGTH(AdvanceDir) < IRIT_EPS)
	    return VList;
	IRIT_PT_NORMALIZE(AdvanceDir);

	/* Evaluate the change in the Z component of gradient by marching    */
	/* along AdvanceDir and orthogonal to it.			     */
	for (i = 0; i < 3; i++) {
	    R = TrivTVEval(GlblTVGradient[i],
			   CrntPos[0] + AdvanceDir[0] * SIL_DIR_GRAD_EPS,
			   CrntPos[1] + AdvanceDir[1] * SIL_DIR_GRAD_EPS,
			   CrntPos[2] + AdvanceDir[2] * SIL_DIR_GRAD_EPS);
	    TmpGrad[i] = R[1];
	}
	IRIT_PT_NORMALIZE(TmpGrad);
	GradDView1 = IRIT_DOT_PROD(TmpGrad, ViewDir) - GradViewOrig;

	IRIT_CROSS_PROD(AdvanceDir2, Grad, AdvanceDir);
	for (i = 0; i < 3; i++) {
	    R = TrivTVEval(GlblTVGradient[i],
			   CrntPos[0] + AdvanceDir2[0] * SIL_DIR_GRAD_EPS,
			   CrntPos[1] + AdvanceDir2[1] * SIL_DIR_GRAD_EPS,
			   CrntPos[2] + AdvanceDir2[2] * SIL_DIR_GRAD_EPS);
	    TmpGrad[i] = R[1];
	}
	IRIT_PT_NORMALIZE(TmpGrad);
	GradDView2 = IRIT_DOT_PROD(TmpGrad, ViewDir) - GradViewOrig;

	for (i = 0; i < 3; i++)
	    AdvanceDir[i] = AdvanceDir[i] * GradDView1 +
			    AdvanceDir2[i] * GradDView2;
	IRIT_PT_NORMALIZE(AdvanceDir);

	if (Sils) {
	    IRIT_CROSS_PROD(AdvanceDir2, Grad, AdvanceDir);
	    if (IRIT_PT_LENGTH(AdvanceDir2) < IRIT_EPS)
	        return VList;
	    IRIT_PT_NORMALIZE(AdvanceDir2);
	    IRIT_PT_COPY(AdvanceDir, AdvanceDir2);
	}
	if (Forward)
	    IRIT_PT_SCALE(AdvanceDir, StepSize)
	else
	    IRIT_PT_SCALE(AdvanceDir, -StepSize);

	IRIT_PT_ADD(CrntPos, CrntPos, AdvanceDir);
	if (!(GlblMinDomain[0] <= CrntPos[0] &&
	      GlblMaxDomain[0] >= CrntPos[0] &&
	      GlblMinDomain[1] <= CrntPos[1] &&
	      GlblMaxDomain[1] >= CrntPos[1] &&
	      GlblMinDomain[2] <= CrntPos[2] &&
	      GlblMaxDomain[2] >= CrntPos[2]) ||  /* Exit parametric domain. */
	    !MCImprovePointOnIsoSrf(CrntPos, CubeDim, IsoVal,
				    PT_COVER_TOL, IRIT_FABS(IsoVal / 10.0)))
	    return VList;

	Length -= StepSize;
    }
    while (Length > 0.0);

    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs a piecewise linear curve out of the given two traces.         *
*                                                                            *
* PARAMETERS:                                                                *
*   V1:      First list to be reversed.                                      *
*   V2:      Second list to be taken as is.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  A piecewise linear curve, NULL if error.               *
*****************************************************************************/
static CagdCrvStruct *ChainTraceIntoCurve(IPVertexStruct *V1,
					  IPVertexStruct *V2)
{
    int i = 0,
	Length1 = IPVrtxListLen(V1),
	Length2 = IPVrtxListLen(V2),
	CrvLength = Length1 + Length2 - 1;	  
    CagdCrvStruct *Crv;
    CagdRType **Points;
    IPVertexStruct *V;

    if (CrvLength < 2) {
	IPFreeVertexList(V1);
	IPFreeVertexList(V2);
	return NULL;
    }

    Crv = BspCrvNew(CrvLength, 2, CAGD_PT_E3_TYPE);
    Points = Crv -> Points;

    BspKnotUniformOpen(CrvLength, 2, Crv -> KnotVector);

    if (V1 != NULL) {
	for (V = V1; V != NULL; V = V -> Pnext, i++) {
	    Points[1][i] = V -> Coord[0];
	    Points[2][i] = V -> Coord[1];
	    Points[3][i] = V -> Coord[2];
	}
    }

    if (V2 != NULL && V2 -> Pnext != NULL) {
	V2 = IPReverseVrtxList2(V2);
	for (V = V2 -> Pnext; V != NULL; V = V -> Pnext, i++) {
	    Points[1][i] = V -> Coord[0];
	    Points[2][i] = V -> Coord[1];
	    Points[3][i] = V -> Coord[2];
	}
    }

    IPFreeVertexList(V1);
    IPFreeVertexList(V2);

    return Crv;
}
