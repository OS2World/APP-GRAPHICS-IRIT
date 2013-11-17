/******************************************************************************
* fitting.c - Fitting of clouds of points representing geometric primitves.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Ronen Lev, January 2003                                          *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "extra_fn.h"
#include "iritprsr.h"
#include "attribut.h"
#include "iritprsr.h"
#include "geom_loc.h"

#define OUTLIER_MAX_TOL_RATIO   2.5
#define CRVTR_ANALYSIS_NUM_OF_RINGS 1
#define NORMAL_EVAL_MAX_ANGLE 180.0 

static IrtRType NonLinearFitDataAux(
			       IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			       const GMFitFittingShapeStruct *FittingShapeInfo,
			       IrtRType ModelIntParams[],
			       IrtRType Tolerance);
static IrtRType NonLinearEstFitDataAux(
			       IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			       const GMFitFittingShapeStruct *FittingShapeInfo,
			       IrtRType ModelIntParams[],
			       IrtRType Tolerance);
static IrtRType LinearFitDataAux(
			       IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			       const GMFitFittingShapeStruct* FittingShapeInfo,
			       IrtRType ModelIntParams[]);
static IrtRType FitDataIntAux(IrtRType **PointData,
			      unsigned int NumberOfPointsToFit,
			      const GMFitFittingShapeStruct *FittingShapeInfo,
			      IrtRType ModelIntParams[],
			      IrtRType Tolerance);
static int IrtRTypeCompareAux(const void *PElem1, const void *PElem2);
static int ValidateCurvatureAnalysisAux (IPPolygonStruct *PPoly);
static int ValidateVertexNormalAux (IPPolygonStruct *PPoly);
static IrtRType TripleProductAux (IrtVecType Vec1,
				  IrtVecType Vec2,
				  IrtVecType Vec3);
static void RotationAxisFittingFuncAux(IrtRType *CurPoint,
				       IrtRType ModelParams[],
				       IrtRType *YPointer,
				       IrtRType YdParams[]);
static int ApproxCollinear3PtsAux(IrtPtType Pt1, IrtPtType Pt2,
				  IrtPtType Pt3, IrtRType Eps);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function finds the best model params in the least-squares sense.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PointData:           List of data points.                                M
*   NumberOfPointsToFit: The length of the list of points.                   M
*   FittingModel:        An enumerator indicating which shape type to fit.   M
*   ModelExtParams:      The resulting external params of the shape.         M
*   Tolerance:           If the error is smaller then Tolerance return.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The avereage squared error.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFitData                                                                M
*****************************************************************************/
IrtRType GMFitData(IrtRType** PointData,
		   unsigned int NumberOfPointsToFit,
		   GMFittingModelType FittingModel,
		   IrtRType ModelExtParams[],
		   IrtRType Tolerance)
{
    IrtRType Result;
    const GMFitFittingShapeStruct
        *FittingShapeInfo = _GMFitGetFittingModel(FittingModel);

    if (FittingShapeInfo == NULL)
        IRIT_FATAL_ERROR("No appropriate fitting model struct.");

    if ((FittingShapeInfo -> IntrnalToExternalFunc) != NULL) {
	IrtRType *ModelIntParams;
		
	ModelIntParams = (IrtRType *)
	    IritMalloc(sizeof(IrtRType) *
		       (FittingShapeInfo -> NumOfIntModelParams));
		
	if (ModelIntParams == NULL) {
	    IRIT_FATAL_ERROR("Unable to allocate memory.");
	}
	Result = FitDataIntAux(PointData, NumberOfPointsToFit,
			       FittingShapeInfo, ModelIntParams, Tolerance);
	FittingShapeInfo -> IntrnalToExternalFunc(ModelIntParams,
						  ModelExtParams);
	IritFree(ModelIntParams);
    }
    else
        Result =  FitDataIntAux(PointData, NumberOfPointsToFit, 
				FittingShapeInfo, ModelExtParams, Tolerance);
	return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function fits the data to the internal parameters model.            *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:                List of data points.                           *
*   NumberOfPointsToFit:      The length of the list of points.              *
*   GMFitFittingShapeStruct*: Struct which constains information regarding   *
*	            	      the shape to be fitted.                        *
*   ModelIntParams:           The resulting internal params of the shape.    *
*   Tolerance:                If the error is smaller then Tolerance return. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The avereage squared error.                                    *
*****************************************************************************/
static IrtRType FitDataIntAux(IrtRType **PointData,
			      unsigned int NumberOfPointsToFit,
			      const GMFitFittingShapeStruct *FittingShapeInfo,
			      IrtRType ModelIntParams[],
			      IrtRType Tolerance)
{
    if (FittingShapeInfo -> IsLinearFittingProblem)
        return LinearFitDataAux(PointData, NumberOfPointsToFit,
				FittingShapeInfo, ModelIntParams);
    else
        return NonLinearFitDataAux(PointData, NumberOfPointsToFit, 
				   FittingShapeInfo, ModelIntParams,
				   Tolerance);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    This function finds the best model params in the least-squares sense,   *
* for non-linear models.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:                 Pointer to a list of data.                    *
*   NumberOfPointsToFit:       The length of the list of points.             *
*   GMFitFittingShapeStruct *: Struct which constains information regarding  *
*	            	       the shape to be fitted.                       *
*   ModelIntParams:           The internal params of the shape (in/out).     *
*   Tolerance:                 If the error is smaller then Tolerance return.*
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The avereage squared error.                                    *
*****************************************************************************/
static IrtRType NonLinearEstFitDataAux(
			       IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			      const GMFitFittingShapeStruct* FittingShapeInfo,
			       IrtRType ModelIntParams[],
			       IrtRType Tolerance)
{
    unsigned int i;
    IrtRType Result, *ExpectedDistVec;

    assert(!(FittingShapeInfo -> IsLinearFittingProblem));

    /* If no valid estimation - abort. */
    if ((FittingShapeInfo -> ModelValidatorFunc != NULL) &&
	(!(FittingShapeInfo -> ModelValidatorFunc(ModelIntParams)))) 
        return IRIT_INFNTY;
    ExpectedDistVec = (IrtRType *) IritMalloc(sizeof(IrtRType) *
					      NumberOfPointsToFit);
    if (ExpectedDistVec == NULL)
	IRIT_FATAL_ERROR ("Unable to allocate memory.");

    for (i = 0; i < NumberOfPointsToFit; )
        ExpectedDistVec[i++] = 0.0;

    Result = IritLevenMarMinSig1(PointData, ExpectedDistVec,
				 NumberOfPointsToFit, ModelIntParams,
				 FittingShapeInfo -> ShapeFunc,
				 FittingShapeInfo -> NumericalProtectionFunc,
				 FittingShapeInfo -> ModelValidatorFunc,
				 FittingShapeInfo -> NumOfIntModelParams,
				 Tolerance);

    IritFree(ExpectedDistVec);
    Result /= NumberOfPointsToFit;

    return Result;
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*    This function finds the best model params in the least-squares sense,   *
* for non-linear models.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:                 Pointer to a list of data.                    *
*   NumberOfPointsToFit:       The length of the list of points.             *
*   GMFitFittingShapeStruct *: Struct which constains information regarding  *
*	            	       the shape to be fitted.                       *
*   ModelIntParams:            The resulting params of the shape.            *
*   Tolerance:                 If the error is smaller then Tolerance return.*
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The avereage squared error.                                    *
*****************************************************************************/
static IrtRType NonLinearFitDataAux(
                              IrtRType **PointData,
			      unsigned int NumberOfPointsToFit,
			      const GMFitFittingShapeStruct* FittingShapeInfo,
			      IrtRType ModelIntParams[],
			      IrtRType Tolerance)
{
    if ((FittingShapeInfo -> ExternalToIntrnalFunc) != NULL) {
	IrtRType *ExternalModelParams;
	
	ExternalModelParams = (IrtRType *) IritMalloc(sizeof(IrtRType) * 
				   (FittingShapeInfo -> NumOfExtModelParams));
	
	if (ExternalModelParams == NULL)
	  IRIT_FATAL_ERROR ("Unable to allocate memory.");
	
	if (!FittingShapeInfo -> InitialEstimateFunc(PointData, 
						     NumberOfPointsToFit,
						     ExternalModelParams)) {
	    return IRIT_INFNTY;
	}
	FittingShapeInfo -> ExternalToIntrnalFunc(ExternalModelParams,
						    ModelIntParams);
	IritFree(ExternalModelParams);
    }
    else {
	if (!FittingShapeInfo -> InitialEstimateFunc(PointData, 
						     NumberOfPointsToFit,
						     ModelIntParams)){
	    return IRIT_INFNTY;
	}
    }

    return NonLinearEstFitDataAux(PointData,  NumberOfPointsToFit, 
				  FittingShapeInfo, ModelIntParams, Tolerance);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    This function finds the best model params in the least-squares sense,   *
* for linear models.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:                Pointer to a list of data.                     *
*   NumberOfPointsToFit:      The length of the list of points.              *
*   GMFitFittingShapeStruct*: Struct which constains information regarding   *
*	            	                            the shape to be fitted.  *
*   ModelParams:              The resulting params of the shape.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The avereage squared error.                                    *
*****************************************************************************/
static IrtRType LinearFitDataAux(
			     IrtRType **PointData,
			     unsigned int NumberOfPointsToFit,
			     const GMFitFittingShapeStruct* FittingShapeInfo,
			     IrtRType ModelIntParams[])
{
    unsigned int i, k, NumOfEq, IndexMaxResult;
    IrtRType Result, MaxResult, *ExpectedDistVec, *A;

    assert(FittingShapeInfo -> IsLinearFittingProblem);

    /* Protect against collinears. */
    if ((NumberOfPointsToFit == 3) && 
	ApproxCollinear3PtsAux(PointData[0], PointData[1],
			       PointData[2], IRIT_EPS))
	return IRIT_INFNTY;

    if ((FittingShapeInfo -> AdditionalConstraintFunc) == NULL)
        NumOfEq = NumberOfPointsToFit;
    else
        NumOfEq = NumberOfPointsToFit + 1;

    ExpectedDistVec = (IrtRType *)IritMalloc(sizeof(IrtRType) * NumOfEq);
    A = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumOfEq * 
				      FittingShapeInfo -> NumOfIntModelParams);

    if  ((ExpectedDistVec == NULL) || (A == NULL)) {
	IRIT_FATAL_ERROR ("Unable to allocate memory.");
    }
    for (i = 0; i < NumberOfPointsToFit; ++i) {
	FittingShapeInfo -> LinearBaseShapeFunc(PointData[i], 
			      &A[i * FittingShapeInfo -> NumOfIntModelParams]);
	ExpectedDistVec[i] = 0.0;
    }

    /* Try to apply an additional constraint, several times. */
    IndexMaxResult = -1;
    MaxResult = 0.0;
    for (k = 0; k < 3; k++) {
        if ((FittingShapeInfo -> AdditionalConstraintFunc) != NULL)
	    FittingShapeInfo -> AdditionalConstraintFunc(
		&A[NumberOfPointsToFit *
				     FittingShapeInfo -> NumOfIntModelParams],
		&ExpectedDistVec[NumberOfPointsToFit], k);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugSVDInputPrint, FALSE) {
	        for (i = 0; i < NumOfEq; ++i) {
		    unsigned int j;

		    for (j = 0;
			 j < FittingShapeInfo -> NumOfIntModelParams;
			 j++)
		        IRIT_INFO_MSG_PRINTF("%10.7f  ",
			  A[i * FittingShapeInfo -> NumOfIntModelParams + j]);
		    IRIT_INFO_MSG_PRINTF("=  %10.7f\n", ExpectedDistVec[i]);
		}
	    }
	}
#	endif /* DEBUG */

	Result = SvdLeastSqr(A, NULL, NULL, NumOfEq, 
			     FittingShapeInfo -> NumOfIntModelParams);
	if (MaxResult < IRIT_FABS(Result)) {
	    IndexMaxResult = k;
	    MaxResult = IRIT_FABS(Result);
	}
    }
    if (MaxResult > IRIT_UEPS) {
	if (IndexMaxResult != k - 1) {
	    /* Recompute the best SVD decomposition. */
	    if ((FittingShapeInfo -> AdditionalConstraintFunc) != NULL)
	        FittingShapeInfo -> AdditionalConstraintFunc(
		     &A[NumberOfPointsToFit *
			          FittingShapeInfo -> NumOfIntModelParams],
		     &ExpectedDistVec[NumberOfPointsToFit], IndexMaxResult);
	    Result = SvdLeastSqr(A, NULL, NULL, NumOfEq, 
				 FittingShapeInfo -> NumOfIntModelParams);
	    assert(IRIT_APX_EQ(MaxResult, IRIT_FABS(Result)));
	}

	SvdLeastSqr(NULL, ModelIntParams, ExpectedDistVec, 
		    NumOfEq, FittingShapeInfo -> NumOfIntModelParams);
    }
    else {
        IritFree(A);
	IritFree(ExpectedDistVec);
	IRIT_WARNING_MSG("SvdLeastSqr failed.");
	return IRIT_INFNTY;
    }

    Result = 0.0;
    for (i = 0; i < NumberOfPointsToFit; ++i) {
	Result += IRIT_SQR(FittingShapeInfo ->
		      CalcApproxFittingErrorFunc(PointData[i],
						 ModelIntParams));
    }
    Result /= NumberOfPointsToFit;	

    IritFree(A);
    IritFree(ExpectedDistVec);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function finds the best model params in the minimal median least    M
* squares sense.                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PointData:           List of data points.                                M
*   NumberOfPointsToFit: The length of the list of points.                   M
*   FittingModel:        An enumerator indicating which shape type to fit.   M
*   ModelExtParams:      The resulting params of the shape.                  M
*   Tolerance:           If the error is smaller then Tolerance return.      M
*   NumOfChecks:         The number of attempts to calculate best sigma.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The median squared error.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMFitData                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFitDataWithOutliers, Outliers                                          M
*****************************************************************************/
IrtRType GMFitDataWithOutliers(IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			       GMFittingModelType FittingModel,
			       IrtRType ModelExtParams[],
			       IrtRType Tolerance,
			       unsigned int NumOfChecks)	
{
    const GMFitFittingShapeStruct
	*FittingShapeInfo = _GMFitGetFittingModel(FittingModel);

    if (FittingShapeInfo == NULL) {
	IRIT_FATAL_ERROR("No appropriate fitting model struct.");
    }
	
    /* Don't even try to detect outliers if not enough data. */
    if (NumberOfPointsToFit < (2 * (FittingShapeInfo -> NumOfRequiredPoints)))
	return GMFitData(PointData, NumberOfPointsToFit,FittingModel, 
			 ModelExtParams, Tolerance);
    else {
	unsigned int i, CurAttempt;
	IrtRType CurBestDist, *BestIntModelParams, *PossibleIntModelParams,
	    *DistVector, **ScrambledPointData;

	PossibleIntModelParams = (IrtRType *) IritMalloc(sizeof(IrtRType) *
				   (FittingShapeInfo -> NumOfIntModelParams));
	BestIntModelParams = (IrtRType *) IritMalloc(sizeof(IrtRType)*
				   (FittingShapeInfo -> NumOfIntModelParams));
	
	DistVector = (IrtRType *) IritMalloc(sizeof(IrtRType) *
						     NumberOfPointsToFit);
	ScrambledPointData = (IrtRType **)
			 IritMalloc(sizeof(IrtRType *) * NumberOfPointsToFit);

	if ((ScrambledPointData == NULL) || 
	    (PossibleIntModelParams == NULL)||
	    (BestIntModelParams == NULL)) {
	    IRIT_FATAL_ERROR("Unable to allocate memory.");
	}
	for (i = 0; i < NumberOfPointsToFit; ++i) 
	    ScrambledPointData[i] = PointData[i];
		
	CurBestDist = IRIT_INFNTY;

	for (CurAttempt = 0; 
	     (CurAttempt < NumOfChecks) && (CurBestDist > Tolerance); 
	     ++CurAttempt) {
	    unsigned int 
		MaxSelectInd = NumberOfPointsToFit - 1,
		First = (unsigned int) floor(
                                    IritRandom(0.0, MaxSelectInd - IRIT_EPS));

		IRIT_SWAP(IrtRType *, ScrambledPointData[0], 
		     ScrambledPointData[First]);

	    while (MaxSelectInd > 0) {
	        unsigned int
		    RandPointInd = 1 + (unsigned int) 
		              floor(IritRandom(0.0, MaxSelectInd - IRIT_EPS));
			
		if (IRIT_PT_PT_DIST_SQR(ScrambledPointData[0], 
				   ScrambledPointData[RandPointInd])
							    > IRIT_SQR(IRIT_EPS)) {
		    IRIT_SWAP(IrtRType *, ScrambledPointData[1],
			 ScrambledPointData[RandPointInd]);
		    --MaxSelectInd;
		    break;
		}
		IRIT_SWAP(IrtRType *, ScrambledPointData[MaxSelectInd],
				 ScrambledPointData[RandPointInd]);
		--MaxSelectInd;
	    }

	    for (i = 2; 
		 (i < FittingShapeInfo -> NumOfRequiredPoints) &&
		 (MaxSelectInd > 0);
		 ++i, --MaxSelectInd) {
		unsigned int
		    RandPointInd = (unsigned int)
		              floor(IritRandom(0.0, MaxSelectInd - IRIT_EPS));
		
		IRIT_SWAP(IrtRType *, ScrambledPointData[i],
		     ScrambledPointData[i + RandPointInd]);
	    }
	    --i;
		
	    while (MaxSelectInd > 0) {
		unsigned int
		    RandPointInd = (unsigned int)
		              floor(IritRandom(0.0, MaxSelectInd - IRIT_EPS));
					
		if (!ApproxCollinear3PtsAux(
			    ScrambledPointData[0], ScrambledPointData[1],
			    ScrambledPointData[RandPointInd + i],
			    IRIT_EPS)) {
		    IRIT_SWAP(IrtRType*, ScrambledPointData[i],
			 ScrambledPointData[i + RandPointInd]);
		    assert( i + RandPointInd < NumberOfPointsToFit);
		    break;
		}
		IRIT_SWAP(IrtRType *, 
		     ScrambledPointData[i + MaxSelectInd - 1],
		     ScrambledPointData[i + RandPointInd]);
		--MaxSelectInd;
	    }
			
	    if (MaxSelectInd == 0) {
		IRIT_WARNING_MSG("All points are collinear.");
		IritFree(PossibleIntModelParams);
		IritFree(BestIntModelParams);
		IritFree(DistVector);
		IritFree(ScrambledPointData);
		return IRIT_INFNTY;
	    }
	    if (FitDataIntAux(ScrambledPointData,
			      FittingShapeInfo -> NumOfRequiredPoints,
			      FittingShapeInfo, PossibleIntModelParams,
			      OUTLIER_MAX_TOL_RATIO * Tolerance)
							< IRIT_INFNTY) {
		unsigned int i;
		IrtRType CurDist;
		
		for (i = 0; i < NumberOfPointsToFit; ++i)
		    DistVector[i] = FittingShapeInfo ->
			CalcApproxFittingErrorFunc(PointData[i], 
						   PossibleIntModelParams);
		qsort(DistVector, NumberOfPointsToFit, 
		      sizeof(IrtRType), IrtRTypeCompareAux);

		CurDist = DistVector[NumberOfPointsToFit >> 1];
		if (CurDist < CurBestDist) {
		    IRIT_GEN_COPY(BestIntModelParams,
			     PossibleIntModelParams,
			     sizeof(IrtRType) *
			     (FittingShapeInfo -> NumOfIntModelParams));
		    CurBestDist = CurDist;
		}
	    }
	} /* For (CurAttempt = 0; CurAttempt < NumOfChecks; ... */

	if (CurBestDist < IRIT_INFNTY) {
	    unsigned int 
		NumberOfPointsToUse = 0;
	    IrtRType CurDist,
	    MaxDist = CurBestDist < IRIT_EPS ? 
		                IRIT_EPS : OUTLIER_MAX_TOL_RATIO * CurBestDist;
			
	    IRIT_GEN_COPY(PossibleIntModelParams, BestIntModelParams, 
		     sizeof(IrtRType) *
		         (FittingShapeInfo -> NumOfIntModelParams));
	    for (i = 0; i < NumberOfPointsToFit; ++i)
		if ((FittingShapeInfo ->
		     CalcApproxFittingErrorFunc(PointData[i],
						BestIntModelParams)) < MaxDist)
		    ScrambledPointData[NumberOfPointsToUse++] =
			PointData[i];

	    if (FittingShapeInfo -> IsLinearFittingProblem)
		CurDist = LinearFitDataAux(ScrambledPointData, 
					   NumberOfPointsToUse,
					   FittingShapeInfo, 
					   PossibleIntModelParams);
	    else
		CurDist = NonLinearEstFitDataAux(ScrambledPointData,
						 NumberOfPointsToUse,
						 FittingShapeInfo,
						 BestIntModelParams, 
						 Tolerance);

	    for (i = 0; i < NumberOfPointsToFit; ++i)
		DistVector[i] = FittingShapeInfo ->
		    CalcApproxFittingErrorFunc(PointData[i], 
					       PossibleIntModelParams);
	    qsort(DistVector, NumberOfPointsToFit, 
		  sizeof(IrtRType), IrtRTypeCompareAux);
			
	    CurDist = DistVector[NumberOfPointsToFit >> 1];

	    if (CurDist < CurBestDist) {
		IRIT_GEN_COPY(BestIntModelParams, PossibleIntModelParams,
			 sizeof(IrtRType) *
			 (FittingShapeInfo -> NumOfIntModelParams));
		CurBestDist = CurDist;
	    }
	    if ((FittingShapeInfo -> IntrnalToExternalFunc) != NULL)
		FittingShapeInfo ->
		    IntrnalToExternalFunc(BestIntModelParams,
					  ModelExtParams);
	    else
		IRIT_GEN_COPY(ModelExtParams, BestIntModelParams,
			 sizeof(IrtRType) *
			 (FittingShapeInfo -> NumOfIntModelParams));

	    if (FittingShapeInfo -> CalcFittingErrorFunc != NULL) {
		for (i = 0; i < NumberOfPointsToFit; ++i)
		    DistVector[i] = FittingShapeInfo ->
			CalcFittingErrorFunc(PointData[i],
					     ModelExtParams);
		qsort(DistVector, NumberOfPointsToFit, 
		      sizeof(IrtRType), IrtRTypeCompareAux);
		
		CurBestDist = DistVector[NumberOfPointsToFit >> 1];
	    }
	    
	} /* If (CurBestDist < IRIT_INFNTY) */

	IritFree(PossibleIntModelParams);
	IritFree(BestIntModelParams);
	IritFree(DistVector);
	IritFree(ScrambledPointData);
	
	return CurBestDist;
    } /* If (NumberOfPointsToFit < ... */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This is a comparison function for the usage of qsort.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PElem1: Pointer to the first element.                                    *
*   PElem2: Pointer to the second element.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: 1 if *PElem2<*PElem1, -1 if *PElem2>*PElem1, 0 otherwise.           *
*****************************************************************************/
static int IrtRTypeCompareAux(const void* PElem1,const void* PElem2)
{
    IrtRType 
        Elem1 = * (IrtRType *) PElem1,
        Elem2 = * (IrtRType *) PElem2;
    
    if (Elem1 < Elem2)
        return -1;
    else if (Elem1 > Elem2)
        return 1;
    else
        return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function finds the best model params in the minimal median least    M
* squares sense.                                                             M
* Warning: This function is NOT thread-safe.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:               Pointer the object to estimate.                     M
*   FittingModel:        An enumerator indicating which shape type to fit.   M
*   ModelExtParams:      The resulting params of the shape.                  M
*                        if FittingModel is				     M
*                          GM_FIT_PLANE -   A, B, C, D of plane equation.    M
*                          GM_FIT_SPHERE -  Xcntr, Ycntr, Zcntr, Radius.     M
*                          GM_FIT_CYLINDER - Xcntr, Ycntr, Zcntr,            M
*                                            Xdir, Ydir, Zdir, Radius.	     M
*			   GM_FIT_CIRCLE -  Xcntr, Ycntr, Radius.            M
*			   GM_FIT_CONE -    Xapex, Yapex, Zapex, apex semi   M
*					    angle,			     M
*                                           Xdir, Ydir, Zdir.        	     M
*			   GM_FIT_TORUS -   Xpnt, Ypnt, Zpnt, DiscRad,       M
*                                           ExtRad, Xdir, Ydir, Zdir. 	     M
*   Tolerance:           If the error is smaller than Tolerance return.      M
*   NumOfChecks:         The number of attempts to calculate best sigma,     M
*			 100 is a good start.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The median squared error.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMFitDataWithOutliers                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFitObjectWithOutliers                                                  M
*****************************************************************************/
IrtRType GMFitObjectWithOutliers(IPPolygonStruct *PPoly,
				 GMFittingModelType FittingModel,
				 IrtRType ModelExtParams[],
				 IrtRType Tolerance,
				 unsigned int NumOfChecks)	
{
    int i;
    IrtRType Result, **FitData; 
    IPObjectStruct *PObj; 
    IPPolyVrtxIdxStruct *PolyVrtxIdxPtr;
    
    if ((FittingModel != GM_FIT_PLANE) && 
	(!ValidateCurvatureAnalysisAux(PPoly)))
	GMPlCrvtrSetCurvatureAttr(PPoly, CRVTR_ANALYSIS_NUM_OF_RINGS, TRUE);

    if (!ValidateVertexNormalAux(PPoly))
	GMBlendNormalsToVertices(PPoly, NORMAL_EVAL_MAX_ANGLE);

    PObj = IPGenPOLYObject(PPoly);
    PolyVrtxIdxPtr = IPCnvPolyToPolyVrtxIdxStruct(PObj, FALSE, 0);
    PObj -> U.Pl = NULL;
    FitData = (IrtRType **)
	IritMalloc(sizeof(IrtRType *) * PolyVrtxIdxPtr -> NumVrtcs);

    if (FitData == NULL)
	IRIT_FATAL_ERROR("Unable to allocate memory.");

    if (FittingModel == GM_FIT_PLANE) {
	for (i = 0; i < PolyVrtxIdxPtr -> NumVrtcs; ++i) {
	    FitData[i] = 
	        (IrtRType *) IritMalloc(sizeof(IrtRType) * 3);

	    IRIT_PT_COPY(FitData[i], PolyVrtxIdxPtr -> Vertices[i] -> Coord); 
	}
    }
    else {
	for (i = 0; i < PolyVrtxIdxPtr -> NumVrtcs; ++i) {
	    const char *StrPtr;
	    IPVertexStruct 
	        *Vertex = PolyVrtxIdxPtr -> Vertices[i];

	    FitData[i] = (IrtRType *) IritMalloc(sizeof(IrtRType) * 14);
	    IRIT_PT_COPY(FitData[i], Vertex -> Coord);
	    
	    FitData[i][3] = AttrGetRealAttrib(Vertex -> Attr, "K1Curv");
	    FitData[i][4] = AttrGetRealAttrib(Vertex -> Attr, "K2Curv");
	    
	    StrPtr = AttrGetStrAttrib(Vertex -> Attr, "D1");
	    if ((StrPtr == NULL ) ||
		(sscanf(StrPtr, "%lf, %lf, %lf", 
			&FitData[i][5],
			&FitData[i][6],
			&FitData[i][7]) != 3))
	        IRIT_FATAL_ERROR("Curvature analysis attributes are invalid.");
	    
	    StrPtr = AttrGetStrAttrib(Vertex -> Attr, "D2");
	    if ((StrPtr == NULL ) ||
		(sscanf(StrPtr, "%lf, %lf, %lf", 
			&FitData[i][8],
			&FitData[i][9],
			&FitData[i][10]) != 3))
	        IRIT_FATAL_ERROR("Curvature analysis attributes are invalid.");
	    
	    assert(IP_HAS_NORMAL_VRTX(Vertex));
	    FitData[i][11] = Vertex -> Normal[0];
	    FitData[i][12] = Vertex -> Normal[1];
	    FitData[i][13] = Vertex -> Normal[2];
	}
    }

    Result = GMFitDataWithOutliers(FitData, PolyVrtxIdxPtr -> NumVrtcs,
				   FittingModel, ModelExtParams, 
				   Tolerance, NumOfChecks);
    
    /* Clean the allocated memory. */
    for (i = 0; i < PolyVrtxIdxPtr -> NumVrtcs; ++i)
	IritFree(FitData[i]);
    IritFree(FitData);

    IPPolyVrtxIdxFree(PolyVrtxIdxPtr);

    IPFreeObject(PObj);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function validates that the object has curvature analysis data.     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPoly: Pointer to the polygon to check.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE iff all the vertexes have curvature analysis data.             *
*****************************************************************************/
static int ValidateCurvatureAnalysisAux(IPPolygonStruct *PPoly)
{
    IPPolygonStruct *CurPoly;
    
    for (CurPoly = PPoly;
	 CurPoly != NULL; 
	 CurPoly = CurPoly -> Pnext) {
        IPVertexStruct *CurVertex;
	
        for (CurVertex = CurPoly -> PVertex;
	     CurVertex != NULL;
	     CurVertex = CurVertex -> Pnext) {
	    if (AttrGetStrAttrib(CurVertex -> Attr, "K1Curv") == NULL ||
		AttrGetStrAttrib(CurVertex -> Attr, "K2Curv") == NULL ||
		AttrGetStrAttrib(CurVertex -> Attr, "D1") == NULL ||
		AttrGetStrAttrib(CurVertex -> Attr, "D2") == NULL)
	      return FALSE;
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function estimates a rotation axis of a surfuce of revolution.      M
*                                                                            *
* PARAMETERS:                                                                M
*   PointsOnObject:        Points on the surface.                            M
*   Normals:               Corrosponding normals.                            M
*   NumberOfPoints:        The number of points/normals.                     M
*   PointOnRotationAxis:   The result.                                       M
*   RotationAxisDirection: The result.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The error.                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFitEstimateRotationAxis                                                M
*****************************************************************************/
IrtRType GMFitEstimateRotationAxis(IrtPtType *PointsOnObject,
				   IrtVecType *Normals,
				   unsigned int NumberOfPoints, 
				   IrtPtType PointOnRotationAxis,
				   IrtVecType RotationAxisDirection)
{
    unsigned int i, j, NumberOfEq;
    IrtRType Alpha[9], Beta[3], RotationAxisModel[17], Error, **XVals,
	*YVals;
    IrtVecType TempVec;
    IrtPtType TempPoint;

    if (NumberOfPoints < 5) {
	IRIT_WARNING_MSG("Not enough points to estimate rotation axis.");
	return IRIT_INFNTY;
    }
    
    NumberOfEq = NumberOfPoints - 2;
    XVals = (IrtRType **) IritMalloc(sizeof(IrtRType *) * NumberOfEq);
    YVals = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfEq);
    if ((XVals == NULL) || (YVals == NULL)) {
	IRIT_FATAL_ERROR("Unable to allocate memory.");
	return IRIT_INFNTY;
    }
    IRIT_ZAP_MEM(YVals, sizeof(IrtRType) * NumberOfEq);
    
    for (i = 0, j = 2; j < NumberOfPoints; ++i, ++j) {
	XVals[i] = (IrtRType *) IritMalloc(sizeof(IrtRType) * 6);
	if (XVals[i] == NULL) {
	    IRIT_FATAL_ERROR("Unable to allocate memory.");
	    return IRIT_INFNTY;
	}
	XVals[i][0]= PointsOnObject[j][0];
	XVals[i][1]= PointsOnObject[j][1];
	XVals[i][2]= PointsOnObject[j][2];
	XVals[i][3]= Normals[j][0];
	XVals[i][4]= Normals[j][1];
	XVals[i][5]= Normals[j][2];
    }
    IRIT_PT_SUB(TempVec, PointsOnObject[1], PointsOnObject[0]);
    for (i = 0; i < 3; ++i) {
	IrtVecType TempVec1, TempVec2;
	
	IRIT_PT_SUB(TempVec1, PointsOnObject[i + 2], PointsOnObject[1]);
	IRIT_PT_SUB(TempVec2, PointsOnObject[0], PointsOnObject[i + 2]);
	
	Alpha[3 * i] = 
	  TripleProductAux(Normals[0], Normals[1], Normals[i + 2]);
	Alpha[3 * i + 1] = 
	  TripleProductAux(TempVec1, Normals[0], Normals[i + 2]);
	Alpha[3 * i + 2] =
	  TripleProductAux(TempVec2, Normals[1], Normals[i + 2]);
	Beta[i] = 
	  TripleProductAux(TempVec2, TempVec, Normals[i + 2]);
    }
    IritGaussJordan(Alpha, Beta, 3, 1);
    RotationAxisModel[0] = Beta[1];
    RotationAxisModel[1] = Beta[2];
    RotationAxisModel[2] = PointsOnObject[0][0];
    RotationAxisModel[3] = PointsOnObject[0][1];
    RotationAxisModel[4] = PointsOnObject[0][2];
    RotationAxisModel[5] = PointsOnObject[1][0];
    RotationAxisModel[6] = PointsOnObject[1][1];
    RotationAxisModel[7] = PointsOnObject[1][2];
    RotationAxisModel[8] = PointsOnObject[1][0] - PointsOnObject[0][0];
    RotationAxisModel[9] = PointsOnObject[1][1] - PointsOnObject[0][1];
    RotationAxisModel[10] = PointsOnObject[1][2] - PointsOnObject[0][2];
    RotationAxisModel[11] = Normals[0][0];
    RotationAxisModel[12] = Normals[0][1];
    RotationAxisModel[13] = Normals[0][2];
    RotationAxisModel[14] = Normals[1][0];
    RotationAxisModel[15] = Normals[1][1];
    RotationAxisModel[16] = Normals[1][2];

    Error = IritLevenMarMinSig1(XVals, YVals, NumberOfPoints - 2, 
				RotationAxisModel, RotationAxisFittingFuncAux,
				NULL, NULL, 17 ,IRIT_EPS);

    IRIT_VEC_SCALE2(TempVec, Normals[0], RotationAxisModel[0]);
    IRIT_PT_ADD(PointOnRotationAxis, PointsOnObject[0], TempVec);
    IRIT_VEC_SCALE2(TempVec, Normals[1], RotationAxisModel[1]);
    IRIT_PT_ADD(TempPoint, PointsOnObject[1], TempVec);
    IRIT_PT_SUB(RotationAxisDirection, TempPoint, PointOnRotationAxis);
    IRIT_VEC_SAFE_NORMALIZE(RotationAxisDirection);
    
    for (i = 0; i < NumberOfEq;) 
	IritFree(XVals[i++]);
    IritFree(XVals);
    IritFree(YVals);
    
    return Error;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate a triple product which is defined as:                          *
* <Vec1 x Vec2, Vec3>.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Vec1: The first vector.                                                  *
*   Vec2: The second vector.                                                 *
*   Vec3: The third vector.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The result of the triple product.                              *
*****************************************************************************/
static IrtRType TripleProductAux(IrtVecType Vec1,
				 IrtVecType Vec2,
				 IrtVecType Vec3)
{
    IrtVecType TempVec;
    
    IRIT_CROSS_PROD(TempVec, Vec1, Vec2);

    return IRIT_DOT_PROD(TempVec, Vec3);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function checks if each vertex in the polygonal model has a         *
* normal.                                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PPoly: The polygonal model to validate it's normals                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE iff each vertex has a normal, otherwise FALSE.                 *
*****************************************************************************/
static int ValidateVertexNormalAux(IPPolygonStruct *PPoly)
{
    IPPolygonStruct *CurPoly;
       

    for (CurPoly = PPoly; CurPoly != NULL; 
	 CurPoly = CurPoly -> Pnext) {
	IPVertexStruct 
	  *CurVert = CurPoly -> PVertex;
	
	assert(CurVert != NULL); 
	do {
	    if (!IP_HAS_NORMAL_VRTX(CurVert))
	      return FALSE;
	    CurVert = CurVert -> Pnext;
	}
	while ((CurVert != NULL) && (CurVert != CurPoly -> PVertex));
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivatives of the    *
* current point and the specified rotation axis.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:    The current point to be calculated.                         *
*   ModelParams: The current model params (internal representation).         *
*   YPointer:    The distance of the point from the cylinder.                *
*   YdParams:    The derivitives of the distance in the model params.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RotationAxisFittingFuncAux(IrtRType *CurPoint,
				       IrtRType ModelParams[],
				       IrtRType *YPointer,
				       IrtRType YdParams[])
{
    IrtRType T0, T1, TriProd1, TriProd2, TriProd3;
    IrtPtType P0, P1, Pi, PiSP1, P0SPi, P1SP0;
    IrtVecType N0, N1, Ni;

    T0 = ModelParams[0];
    T1 = ModelParams[1];

    P0[0] = ModelParams[2];
    P0[1] = ModelParams[3];
    P0[2] = ModelParams[4];

    P1[0] = ModelParams[5];
    P1[1] = ModelParams[6];
    P1[2] = ModelParams[7];
    
    P1SP0[0] = ModelParams[8];
    P1SP0[1] = ModelParams[9];
    P1SP0[2] = ModelParams[10];

    N0[0] = ModelParams[11];
    N0[1] = ModelParams[12];
    N0[2] = ModelParams[13];

    N1[0] = ModelParams[14];
    N1[1] = ModelParams[15];
    N1[2] = ModelParams[16];

    Pi[0] = CurPoint[0];
    Pi[1] = CurPoint[1];
    Pi[2] = CurPoint[2];

    Ni[0] = CurPoint[3];
    Ni[1] = CurPoint[4];
    Ni[2] = CurPoint[5];
    
    IRIT_PT_SUB(PiSP1, Pi, P1);
    IRIT_PT_SUB(P0SPi, P0, Pi);

    TriProd1 = TripleProductAux(N0, N1, Ni);
    TriProd2 = TripleProductAux(PiSP1, N0, Ni);
    TriProd3 = TripleProductAux(P0SPi, N1, Ni);

    IRIT_ZAP_MEM(YdParams, sizeof(IrtRType) * 17);

    *YPointer = TriProd1 * T0 * T1 + TriProd2 * T0 + TriProd3 * T1 +
      TripleProductAux(P0SPi, P1SP0, Ni);
    
    YdParams[0] = TriProd1 * T1 + TriProd2;
    YdParams[1] = TriProd1 * T0 + TriProd3;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Verifies the collinearity of three points approximatly.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2, Pt3: Three points to verify for collinearity.                  *
*   Eps: The epsilon of the test.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if collinear, FALSE otherwise.                            *
*****************************************************************************/
static int ApproxCollinear3PtsAux(IrtPtType Pt1, IrtPtType Pt2,
				  IrtPtType Pt3, IrtRType Eps)
{
    IrtVecType V1, V2, V3;
    IrtRType L1Sqr, L2Sqr;

    IRIT_PT_SUB(V1, Pt1, Pt2);
    IRIT_PT_SUB(V2, Pt2, Pt3);

    L1Sqr = IRIT_PT_SQR_LENGTH(V1);
    L2Sqr = IRIT_PT_SQR_LENGTH(V2);
    if (L1Sqr < IRIT_SQR(Eps) || L2Sqr < IRIT_SQR(Eps))
	return TRUE;

    IRIT_CROSS_PROD(V3, V1, V2);

    return IRIT_PT_SQR_LENGTH(V3) < (L1Sqr * L2Sqr) * IRIT_SQR(Eps);
}
