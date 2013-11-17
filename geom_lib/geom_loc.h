/******************************************************************************
* Geom_loc.h - header file for the geometry library.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 98.					      *
******************************************************************************/

#ifndef GEOM_LOC_H
#define GEOM_LOC_H

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call GeomFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define GEOM_FATAL_ERROR(Msg)	GeomFatalError(Msg)

#define GEOM_GEN_COPY(Dst, Src, Size) memcpy((char *) (Dst), (char *) (Src), \
					     Size)

#include "geom_lib.h"

/* Primitive fitting to point clouds. */

typedef IrtRType GMFitCalcApproxFittingErrorFuncType(IrtRType *PointData,
					       IrtRType InternalModelParams[]);
typedef int GMFitInitialEstimateFuncType(IrtRType **PointList,
					 unsigned int NumberOfPointsInList,
					 IrtRType ModelInitialParams[]);
typedef void GMFitExternalToIntrnalParamFuncType(IrtRType ExternalModelParams[],
						 IrtRType InternalModelParams[]);

typedef void GMFitIntrnalToExternalParamFuncType(IrtRType InternalModelParams[],
						 IrtRType ExternalModelParams[]);

typedef void GMFitLinearBaseShapeFuncType(IrtPtType CurPoint,
					  IrtRType BaseFuncs[]);
typedef void GMFitShapeResultPrintfFuncType(FILE *OutputFile,
					    IrtRType ExternalModelParams[]);
typedef void GMFitAdditionalConstraintFuncType(IrtRType Combination[],
					       IrtRType *ExpectedResult,
					       int Trial);
typedef IrtRType GMFitCalcFittingErrorFuncType(IrtRType *PointData,
					       IrtRType ExternalModelParams[]);

typedef struct GMFitFittingShapeStruct {
    unsigned int NumOfRequiredPoints, NumOfExtModelParams, NumOfIntModelParams;
    int IsLinearFittingProblem;

    GMFitLinearBaseShapeFuncType *LinearBaseShapeFunc;/* Linear Shapes only. */
    IritLevenEvalFuncType *ShapeFunc;                /* !Linear Shapes only. */
    GMFitInitialEstimateFuncType *InitialEstimateFunc; /*!Linear Shapes only.*/
    GMFitExternalToIntrnalParamFuncType *ExternalToIntrnalFunc;
    GMFitIntrnalToExternalParamFuncType *IntrnalToExternalFunc;
    GMFitCalcApproxFittingErrorFuncType *CalcApproxFittingErrorFunc;
    GMFitShapeResultPrintfFuncType *ShapeResultPrintfFunc;
    GMFitAdditionalConstraintFuncType *AdditionalConstraintFunc; /* Linear Shapes only. */
    IritLevenIsModelValidFuncType *ModelValidatorFunc; /*!Linear Shapes only.*/
    IritLevenNumerProtectionFuncType *NumericalProtectionFunc;
	GMFitCalcFittingErrorFuncType *CalcFittingErrorFunc;
} GMFitFittingShapeStruct;

const GMFitFittingShapeStruct *_GMFitGetFittingModel(GMFittingModelType FittingModel);

#define W 0	 /* Positions of points in Points array (see structs below). */
#define X 1
#define Y 2
#define Z 3

#endif /* GEOM_LOC_H */
