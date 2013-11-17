/******************************************************************************
* fit2pts.c - Implementation of fitting primitives related functions          *
*   Based on an article by G. Lukacs, A. D. Marshall, and R. R. Martin        *
* called: "Geometric least-squares fitting of spheres, cylinders, cones and   *
* tori", http://citeseer.nj.nec.com/482177.html                               *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Ronen Lev, January 2003                                          *
******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include "irit_sm.h"
#include "geom_loc.h"

#define ESTIMATE_USING_PRINCIPAL_DIR

#define NUM_OF_REQUIRED_POINTS_FOR_TORUS 7
#define NUM_OF_REQUIRED_POINTS_FOR_CONE 6
#define NUM_OF_REQUIRED_POINTS_FOR_CYLINDER 5
#define NUM_OF_REQUIRED_POINTS_FOR_SPHERE 4
#define NUM_OF_REQUIRED_POINTS_FOR_CIRCLE 3

#define NUM_OF_TORUS_INTERNAL_PARAMS 10

/* Torus functions. */
static int TorusInitialEstimateAux(IrtRType **PointList,
				   unsigned int NumberOfPointsInList,
				   IrtRType ModelInitialParams[]);
static IrtRType CalcTorusFittingErrorAux(IrtRType *PointData,
					 IrtRType InternalModelParams[]);
static IrtRType CalcTorusFittingErrorWEpsAux(IrtRType *PointData,
					     IrtRType InternalModelParams[],
					     IrtRType Eps, 
					     IrtRType SinPhi, 
					     IrtRType CosPhi,
					     IrtRType SinTheta, 
					     IrtRType CosTheta,
					     IrtRType SinSigma, 
					     IrtRType CosSigma,
					     IrtRType SinTau, 
					     IrtRType CosTau);
static void TorusShapeFuncAux(IrtPtType CurPoint,
			      IrtRType ModelParams[],
			      IrtRType* YPointer,
			      IrtRType YdParams[]);
static void InternalToExternalTorusParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[]);

IRIT_STATIC_DATA const GMFitFittingShapeStruct TorusFittingStruct = {
    NUM_OF_REQUIRED_POINTS_FOR_TORUS,
    8,
    NUM_OF_TORUS_INTERNAL_PARAMS,
    FALSE,
    NULL,
    TorusShapeFuncAux,
    TorusInitialEstimateAux,
    NULL,
    InternalToExternalTorusParamsAux,
    CalcTorusFittingErrorAux,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL /* real error should be added */
};


/* Cone functions. */
static int ConeInitialEstimateAux(IrtRType **PointList,
				  unsigned int NumberOfPointsInList,
				  IrtRType ModelInitialParams[]);
static void ConeShapeFuncAux(IrtPtType CurPoint,
			     IrtRType ModelParams[],
			     IrtRType* YPointer,
			     IrtRType YdParams[]);
static void InternalToExternalConeParamsAux(IrtRType InternalModelParams[],
					    IrtRType ExternalModelParams[]);
static IrtRType CalcApproxConeFittingErrorAux(IrtRType *PointData,
					      IrtRType InternalModelParams[]);
static void ConeResultPrintfAux(FILE *OutputFile,
				IrtRType ExternalModelParams[]);
static IrtRType CalcConeFittingErrorAux(IrtRType *PointData,
					IrtRType ExternalModelParams[]);

IRIT_STATIC_DATA const GMFitFittingShapeStruct ConeFittingStruct = {
    NUM_OF_REQUIRED_POINTS_FOR_CONE,
    7,
    9,
    FALSE,
    NULL,
    ConeShapeFuncAux,
    ConeInitialEstimateAux,
    NULL,
    InternalToExternalConeParamsAux,
    CalcApproxConeFittingErrorAux,
    ConeResultPrintfAux,
    NULL,
    NULL,
    NULL,
    CalcConeFittingErrorAux /* Real Error should be added */
};

/* Cylinder functions. */
static int CylinderInitialEstimateAux(IrtRType **PointList,
				      unsigned int NumberOfPointsInList,
				      IrtRType ModelInitialParams[]);
static void CylinderShapeFuncAux(IrtPtType CurPoint,
				 IrtRType ModelParams[],
				 IrtRType* YPointer,
				 IrtRType YdParams[]);
static void InternalToExternalCylinderParamsAux(IrtRType InternalModelParams[],
					       IrtRType ExternalModelParams[]);
static IrtRType CalcApproxCylinderFittingErrorAux(IrtRType *PointData,
					       IrtRType InternalModelParams[]);
static IrtRType CalcCylinderFittingErrorAux(IrtRType *PointData,
					    IrtRType ExternalModelParams[]);
static void CylinderResultPrintfAux(FILE *OutputFile,
				    IrtRType ExternalModelParams[]);
IRIT_STATIC_DATA const GMFitFittingShapeStruct CylinderFittingStruct = {
    NUM_OF_REQUIRED_POINTS_FOR_CYLINDER,
    7,
    8,
    FALSE,
    NULL,
    CylinderShapeFuncAux,
    CylinderInitialEstimateAux,
    NULL,
    InternalToExternalCylinderParamsAux,
    CalcApproxCylinderFittingErrorAux,
    CylinderResultPrintfAux,
    NULL,
    NULL,
    NULL,
    CalcCylinderFittingErrorAux
};

/* Sphere functions. */
static int SphereInitialEstimateAux (IrtRType **PointList,
				     unsigned int NumberOfPointsInList,
				     IrtRType ModelInitialParams[]);
static void SphereShapeFuncAux (IrtPtType CurPoint,
				IrtRType ModelParams[],
				IrtRType* YPointer,
				IrtRType YdParams[]);
static void ExternalToIntrnalSphereParamsAux(IrtRType ExternalModelParams[],
					     IrtRType InternalModelParams[]);
static void IntrnalToExternalSphereParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[]);
static IrtRType CalcApproxSphereFittingErrorAux(IrtRType *PointData, 
					  IrtRType InternalModelParams[]);
static IrtRType CalcSphereFittingErrorAux(IrtRType *PointData, 
					  IrtRType ExternalModelParams[]);
static void SphereResultPrintfAux(FILE* OutputFile,
				  IrtRType ExternalModelParams[]);
IRIT_STATIC_DATA const GMFitFittingShapeStruct SphereFittingStruct = {
    NUM_OF_REQUIRED_POINTS_FOR_SPHERE,
    4,
    4,
    FALSE,
    NULL,
    SphereShapeFuncAux,
    SphereInitialEstimateAux,
    ExternalToIntrnalSphereParamsAux,
    IntrnalToExternalSphereParamsAux,
    CalcApproxSphereFittingErrorAux,
    SphereResultPrintfAux,
    NULL,
    NULL,
    NULL,
    CalcSphereFittingErrorAux
};

/* Circle functions. */
static int CircleInitialEstimateAux(IrtRType **PointList,
				    unsigned int NumberOfPointsInList,
				    IrtRType ModelInitialParams[]);
static void CircleShapeFuncAux(IrtPtType CurPoint,
			       IrtRType ModelParams[],
			       IrtRType* YPointer,
			       IrtRType YdParams[]);
static void ExternalToIntrnalCircleParamsAux(IrtRType ExternalModelParams[], 
					     IrtRType InternalModelParams[]);
static void IntrnalToExternalCircleParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[]);
static IrtRType CalcCircleFittingErrorAux(IrtRType *PointData,
					  IrtRType ExternalModelParams[]);
static IrtRType CalcApproxCircleFittingErrorAux(IrtRType *PointData,
					      IrtRType InternalModelParams[]);
static void CircleResultPrintfAux(FILE* OutputFile,
				  IrtRType ExternalModelParams[]);
static void CircleNumericalProtectionAux(IrtRType InternalModelParams[]);
static int CircleModelValidatorAux(IrtRType InternalModelParams[]);
IRIT_STATIC_DATA const GMFitFittingShapeStruct CircleFittingStruct = {
    NUM_OF_REQUIRED_POINTS_FOR_CIRCLE,
    3,
    5,
    FALSE,
    NULL,
    CircleShapeFuncAux,
    CircleInitialEstimateAux,
    ExternalToIntrnalCircleParamsAux,
    IntrnalToExternalCircleParamsAux,
    CalcApproxCircleFittingErrorAux,
    CircleResultPrintfAux,
    NULL,
    CircleModelValidatorAux,
    CircleNumericalProtectionAux,
    CalcCircleFittingErrorAux
};

/* Plane functions - three possible (orthogonal) normalization factors. */
IRIT_STATIC_DATA IrtRType
    PlaneNormalizationConstraints[3][4] = { {  0.5,  0.5, -1.0, 0.0 },
					    {  1.5,  0.5,  1.0, 0.0 },
					    { -1.0,  2.0,  0.5, 0.0 } };

static void PlaneBaseShapeFuncAux(IrtPtType CurPoint, IrtRType BaseFuncs[]);
static IrtRType CalcPlaneFittingErrorAux(IrtRType *PointData,
					 IrtRType InternalModelParams[]);
static void PlaneResultPrintfAux(FILE* OutputFile,
				 IrtRType ExternalModelParams[]);
static void PlaneAdditionalConstraintAux(IrtRType Combination[],
					 IrtRType *ExpectedResult,
					 int Trial);
IRIT_STATIC_DATA const GMFitFittingShapeStruct PlaneFittingStruct = {
    4,
    4,
    4,
    TRUE,
    PlaneBaseShapeFuncAux,
    NULL,
    NULL,
    NULL,
    NULL,
    CalcPlaneFittingErrorAux,
    PlaneResultPrintfAux,
    PlaneAdditionalConstraintAux,
    NULL,
    NULL,
    CalcPlaneFittingErrorAux
};

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function returns the matching GMFitFittingShapeStruct to the given  M
* enum, or NULL if none exist. THIS POINTER MUST NOT BE FREED!!!             M
*                                                                            *
* PARAMETERS:                                                                M
*   FittingModel: The enum of the needed fitting model.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   const GMFitFittingShapeStruct *: A pointer to a staticly allocated       M
*			             fitting struct.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   FitData, FitDataWithOutliers                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   _GMFitGetFittingModel                                                    M
*****************************************************************************/
const GMFitFittingShapeStruct *_GMFitGetFittingModel(GMFittingModelType
						                 FittingModel)
{
    switch (FittingModel) {
	case GM_FIT_PLANE:
            return &PlaneFittingStruct;
	case GM_FIT_SPHERE:
	    return &SphereFittingStruct;
	case GM_FIT_CYLINDER:
	    return &CylinderFittingStruct;
        case GM_FIT_CIRCLE:
	    return &CircleFittingStruct;
	case GM_FIT_CONE:
	    return &ConeFittingStruct;
	case GM_FIT_TORUS:
	    return &TorusFittingStruct;
	default:
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attepts to guess an initial estimate for the Cylinder      *
* external model params.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointList:          A list of PointData (X, Y, Z, K1, K2, K1CurvX,       *
*                       K1CurvY, K1CurvZ, K2CurvX, K2CurvY, K2CurvZ,         *
*                       NormX, NormY, NormZ)                                 *
*   NumberOfPointsInList: The number of points in PointList.                 *
*   ModelInitialParams: The result vector of the initial params.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE on success.                                                    *
*****************************************************************************/
static int CylinderInitialEstimateAux(IrtRType **PointList,
				      unsigned int NumberOfPointsInList,
				      IrtRType ModelInitialParams[])
{
    unsigned int i;
    IrtRType MeanCurv;
    IrtVecType CylDirection, EstimatedNormal;
    
    if (NumberOfPointsInList < NUM_OF_REQUIRED_POINTS_FOR_CYLINDER) {
	IRIT_WARNING_MSG("not enough points to calculate estimation.");
	return FALSE;
    }

    EstimatedNormal[0] = PointList[0][11];
    EstimatedNormal[1] = PointList[0][12];
    EstimatedNormal[2] = PointList[0][13];
    ModelInitialParams[0] = 0.0;
    ModelInitialParams[1] = atan2(EstimatedNormal[0], EstimatedNormal[1]);
    ModelInitialParams[2] = 
      atan2(IRIT_VEC2D_LENGTH(EstimatedNormal), EstimatedNormal[2]);

    MeanCurv = 0.0;
    IRIT_VEC_RESET(CylDirection);

    /* Find the Cylinder direction */

    /* Both methods work, the first is faster. */
#ifdef ESTIMATE_USING_PRINCIPAL_DIR
    for (i = 0; i < NumberOfPointsInList; ++i) {
        unsigned int VecOffset,j;
	IrtRType MaxElem, AbsMaxElem;

	if (IRIT_FABS(PointList[i][3]) < IRIT_FABS(PointList[i][4])) {
	    VecOffset = 5;
	    MeanCurv += IRIT_FABS(PointList[i][4]);
	}
	else {
	    VecOffset = 8;
	    MeanCurv += IRIT_FABS(PointList[i][3]);
	}
	MaxElem = PointList[i][VecOffset];
	AbsMaxElem = IRIT_FABS(MaxElem);
	for (j = 1; j < 3; ++j) {
	    if (IRIT_FABS(PointList[i][VecOffset+j]) > AbsMaxElem){
	        MaxElem = PointList[i][VecOffset+j];
	        AbsMaxElem = IRIT_FABS(MaxElem);			
	    }
	}
	if (MaxElem < 0.0)
	    IRIT_VEC_SCALE(&PointList[i][VecOffset], -1.0);
	IRIT_VEC_ADD(CylDirection, &PointList[i][VecOffset], CylDirection);
    }
    IRIT_VEC_SAFE_NORMALIZE(CylDirection);
#else
    {
	IrtPtType *PointsOnObject, PointOnAxis;
	IrtVecType *Normals;
	
	PointsOnObject = 
	    (IrtPtType*) IritMalloc(sizeof(IrtPtType) * NumberOfPointsInList);
	Normals = 
	   (IrtVecType*) IritMalloc(sizeof(IrtVecType) * NumberOfPointsInList);
	if ((PointsOnObject == NULL) || (Normals == NULL)) {
	    IRIT_FATAL_ERROR("Unable to allocate memory.");
	    return FALSE;
	}
	for (i = 0; i < NumberOfPointsInList; ++i){
	    IRIT_PT_COPY(PointsOnObject[i], PointList);
	    IRIT_VEC_COPY(Normals[i], &(PointList[i][11]));
	    if (IRIT_FABS(PointList[i][3]) < IRIT_FABS(PointList[i][4])) 
	        MeanCurv += IRIT_FABS(PointList[i][4]);
	    else
	        MeanCurv += IRIT_FABS(PointList[i][3]);

	}
	GMFitEstimateRotationAxis(PointsOnObject, Normals, 
				  NumberOfPointsInList, PointOnAxis, 
				  CylDirection);
	IritFree(PointsOnObject);
	IritFree(Normals);
    }
#endif /* ESTIMATE_USING_PRINCIPAL_DIR */

    ModelInitialParams[3] = asin(IRIT_DOT_PROD(CylDirection, EstimatedNormal));
    ModelInitialParams[4] = MeanCurv / NumberOfPointsInList;

    ModelInitialParams[5] = PointList[0][0];
    ModelInitialParams[6] = PointList[0][1];
    ModelInitialParams[7] = PointList[0][2];

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivitives of the    *
* current point and the specified cylinder.                                  *
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
static void CylinderShapeFuncAux(IrtPtType CurPoint,
				 IrtRType ModelParams[],
				 IrtRType *YPointer,
				 IrtRType YdParams[])
{
    IrtPtType P, DirectionToCenter, CylinderDirection, NTheta, NPhi, NPhiHat,
	PointOnCylinder, PHat, PHatACrossProd, NThetaPhi, NPhiPhiHat,
	DADPhi, AHat;
    IrtRType PNDotProd,PADotProd, PNPhiDotProd, PNThetaDotProd,

    SinTheta = sin(ModelParams[2]),
    CosTheta = cos(ModelParams[2]),
    SinPhi = sin(ModelParams[1]),
    CosPhi = cos(ModelParams[1]),
    SinAlpha = sin(ModelParams[3]),
    CosAlpha = cos(ModelParams[3]);
    
    DirectionToCenter[0] = CosPhi * SinTheta;
    DirectionToCenter[1] = SinPhi * SinTheta;
    DirectionToCenter[2] = CosTheta;

    NPhi[0] = -DirectionToCenter[1];
    NPhi[1] = DirectionToCenter[0];
    NPhi[2] = 0.0;
    NPhiHat[0] = -SinPhi;
    NPhiHat[1] = CosPhi;
    NPhiHat[2] = 0.0;
    NTheta[0] = CosTheta * CosPhi;
    NTheta[1] = SinPhi * CosTheta;
    NTheta[2] = -SinTheta;

    CylinderDirection[0] = NTheta[0] * CosAlpha + NPhiHat[0] * SinAlpha;
    CylinderDirection[1] = NTheta[1] * CosAlpha + NPhiHat[1] * SinAlpha;
    CylinderDirection[2] = NTheta[2] * CosAlpha + NPhiHat[2] * SinAlpha;
	
    P[0] = CurPoint[0] - ModelParams[5];
    P[1] = CurPoint[1] - ModelParams[6];
    P[2] = CurPoint[2] - ModelParams[7];

    PNDotProd = IRIT_DOT_PROD(P, DirectionToCenter);
    PNPhiDotProd = IRIT_DOT_PROD(P, NPhi);
    PNThetaDotProd = IRIT_DOT_PROD(P, NTheta);
    PADotProd = IRIT_DOT_PROD(P, CylinderDirection);
    IRIT_PT_SCALE2(PointOnCylinder, DirectionToCenter, ModelParams[0]);
    IRIT_PT_SUB(PHat, P, PointOnCylinder);
    IRIT_CROSS_PROD(PHatACrossProd, PHat, CylinderDirection);
    NThetaPhi[0] = -SinPhi * CosTheta;
    NThetaPhi[1] = CosPhi * CosTheta;
    NThetaPhi[2] = 0.0;
    NPhiPhiHat[0] = -CosPhi;
    NPhiPhiHat[1] = SinPhi;
    NPhiPhiHat[2] = 0.0;
    DADPhi[0] = NThetaPhi[0] * CosAlpha + NPhiPhiHat[0] * SinAlpha;
    DADPhi[1] = NThetaPhi[1]*CosAlpha+NPhiPhiHat[1]*SinAlpha;
    DADPhi[2] = 0.0;
    AHat[0] = NTheta[0] * SinAlpha +NPhiHat[0] * CosAlpha;
    AHat[1] = NTheta[1] * SinAlpha +NPhiHat[1] * CosAlpha;
    AHat[2] = NTheta[2] * SinAlpha +NPhiHat[2] * CosAlpha;

    if (IRIT_FABS(ModelParams[4]) < IRIT_EPS)
        *YPointer = IRIT_INFNTY;
    else	
        *YPointer = (ModelParams[4] * 0.5) * IRIT_PT_SQR_LENGTH(PHatACrossProd) -
					     IRIT_DOT_PROD(PHat, DirectionToCenter);
    YdParams[0] = ModelParams[4] * (ModelParams[0] - PNDotProd) + 1;
    YdParams[1] = (-ModelParams[4]) * 
        (ModelParams[0] * PNPhiDotProd + PADotProd * IRIT_DOT_PROD(P, DADPhi))
	- PNPhiDotProd;
    YdParams[2] = ModelParams[4] * 
        (PADotProd * PNDotProd * CosAlpha - ModelParams[0] * PNThetaDotProd)
        - PNThetaDotProd;
    YdParams[3] = ModelParams[4] * PADotProd * (IRIT_DOT_PROD(P, AHat));
    YdParams[4] = 0.5 * IRIT_PT_SQR_LENGTH(PHatACrossProd);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from parametrisation to internal external         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: (Length, Theta, Phi, 1/Radius, Xtrans,              *
* Ytrans, Ztrans).                                                           *
*   ExternalModelParams: (Xcenter, Ycenter, Zcenter, Xdir, Ydir, Zdir,       *
* Radius).                                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InternalToExternalCylinderParamsAux(IrtRType InternalModelParams[],
						IrtRType ExternalModelParams[])
{
    IrtRType SinTheta, CosTheta, SinPhi, CosPhi, 
	SinAlpha, CosAlpha, ScalingFactor, InvRad;
    IrtVecType VecToCenter;
    
    SinPhi = sin(InternalModelParams[1]);
    CosPhi = cos(InternalModelParams[1]);
    SinTheta = sin(InternalModelParams[2]);
    CosTheta = cos(InternalModelParams[2]);
    SinAlpha = sin(InternalModelParams[3]);
    CosAlpha = cos(InternalModelParams[3]);
    VecToCenter[0] = CosPhi * SinTheta;
    VecToCenter[1] = SinPhi * SinTheta;
    VecToCenter[2] = CosTheta;
    InvRad = 1.0 / InternalModelParams[4];
    ExternalModelParams[6] = IRIT_FABS(InvRad);
    ScalingFactor = InternalModelParams[0] + InvRad;
    ExternalModelParams[0] = 
	ScalingFactor * VecToCenter[0] + InternalModelParams[5];
    ExternalModelParams[1] = 
	ScalingFactor * VecToCenter[1] + InternalModelParams[6];
    ExternalModelParams[2] = 
	ScalingFactor * VecToCenter[2] + InternalModelParams[7];
    ExternalModelParams[3] = CosTheta * CosPhi * CosAlpha - SinPhi * SinAlpha;
    ExternalModelParams[4] = CosTheta * SinPhi * CosAlpha + CosPhi * SinAlpha;
    ExternalModelParams[5] = -SinTheta * CosAlpha;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function writes to the Output file a formated representation of     *
* the Cylinder.                                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   OutputFile:          The file to write to.                               *
*   ExternalModelParams: The fitted sphere parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CylinderResultPrintfAux(FILE* OutputFile, 
				    IrtRType ExternalModelParams[])
{
    fprintf(OutputFile,
	    "Center:( %lf , %lf , %lf )\tAxis:( %lf , %lf , %lf )\tR = %lf",
	    ExternalModelParams[0], ExternalModelParams[1],
	    ExternalModelParams[2], ExternalModelParams[3],
	    ExternalModelParams[4], ExternalModelParams[5],
	    ExternalModelParams[6]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This fucntion returns the distance of the point from the Cylinder.       *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   ExternalModelParams: The cylinder (external representation).             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: the distance of the point from the Cylinder.                   *
*****************************************************************************/
static IrtRType CalcCylinderFittingErrorAux(IrtRType *PointData,
					    IrtRType ExternalModelParams[])
{
	return IRIT_FABS(ExternalModelParams[6] - 
				GMDistPointLine(PointData,ExternalModelParams, 
						&ExternalModelParams[3]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This fucntion returns the distance of the point from the Cylinder.       *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   InternalModelParams: The cylinder (internal representation).             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: approximaion of the distance of the point from the Cylinder.   *
*****************************************************************************/
static IrtRType CalcApproxCylinderFittingErrorAux(IrtRType *PointData,
					        IrtRType InternalModelParams[])
{
    IrtPtType DirectionToCenter, CylinderDirection, NTheta, NPhiHat, P,
	PointOnCylinder, PHat, PHatACrossProd;
    IrtRType 
	SinTheta = sin(InternalModelParams[2]),
	CosTheta = cos(InternalModelParams[2]),
        SinPhi = sin(InternalModelParams[1]),
        CosPhi = cos(InternalModelParams[1]),
        SinAlpha = sin(InternalModelParams[3]),
        CosAlpha = cos(InternalModelParams[3]);

    if (IRIT_FABS(InternalModelParams[4]) < IRIT_EPS)
        return IRIT_INFNTY;

    P[0] = PointData[0] - InternalModelParams[5];
    P[1] = PointData[1] - InternalModelParams[6];
    P[2] = PointData[2] - InternalModelParams[7];

    DirectionToCenter[0] = CosPhi * SinTheta;
    DirectionToCenter[1] = SinPhi * SinTheta;
    DirectionToCenter[2] = CosTheta;
    NPhiHat[0] = -SinPhi;
    NPhiHat[1] = CosPhi;
    NPhiHat[2] = 0.0;
    NTheta[0] = CosTheta * CosPhi;
    NTheta[1] = SinPhi * CosTheta;
    NTheta[2] = -SinTheta;
    CylinderDirection[0] = NTheta[0] * CosAlpha + NPhiHat[0] * SinAlpha;
    CylinderDirection[1] = NTheta[1] * CosAlpha + NPhiHat[1] * SinAlpha;
    CylinderDirection[2] = NTheta[2] * CosAlpha + NPhiHat[2] * SinAlpha;
    IRIT_PT_SCALE2(PointOnCylinder, DirectionToCenter, InternalModelParams[0]);
    IRIT_PT_SUB(PHat, P, PointOnCylinder);
    IRIT_CROSS_PROD(PHatACrossProd, PHat, CylinderDirection);
    return IRIT_FABS(((InternalModelParams[4] * 0.5) *
		 (IRIT_PT_SQR_LENGTH(PHatACrossProd))) -
					IRIT_DOT_PROD(PHat, DirectionToCenter));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attepts to guess an initial estimate for the Sphere        *
* external model params.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointList:          A list of PointData (X, Y, Z, K1, K2, K1CurvX,       *
*                       K1CurvY, K1CurvZ, K2CurvX, K2CurvY, K2CurvZ,         *
*                       NormX, NormY, NormZ)                                 *
*   NumberOfPointsInList: The number of points in PointList.                 *
*   ModelInitialParams:   The result vector of the initial params.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE on success.                                                    *
*****************************************************************************/
static int SphereInitialEstimateAux(IrtRType **PointList,
				    unsigned int NumberOfPointsInList,
				    IrtRType ModelInitialParams[])
{
    unsigned int i;
    IrtRType
	Curvature = 0.0;
    IrtPtType CenterPoint;

    IRIT_PT_RESET(CenterPoint);
    
    if (NumberOfPointsInList < NUM_OF_REQUIRED_POINTS_FOR_SPHERE)
        return FALSE;
    for (i = 0;i < NumberOfPointsInList; ++i)
        IRIT_PT_ADD(CenterPoint, PointList[i], CenterPoint); 
    
    IRIT_PT_SCALE (CenterPoint, 1.0 / (IrtRType)NumberOfPointsInList);
    for (i = 0; i < NumberOfPointsInList; ++i)
        Curvature += IRIT_FABS(PointList[i][4]) + IRIT_FABS(PointList[i][3]);
    Curvature /= (IrtRType)(2 * NumberOfPointsInList);
    IRIT_PT_COPY(ModelInitialParams, CenterPoint);
    if (Curvature < IRIT_EPS)
	/* While the correct answer seems to be IRIT_INFNTY, this value    */
	/* would allow the fitting to start from a position it can change. */
        ModelInitialParams[3] = 1.0;
    else
        ModelInitialParams[3] = 1.0 / Curvature;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivitives of the    *
* current point and the specified sphere.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:    The current point to be calculated.                         *
*   ModelParams: The current model params (internal representation).         *
*   YPointer:    The distance of the point from the sphere.                  *
*   YdParams:    The derivitives of the distance in the model params.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SphereShapeFuncAux(IrtPtType CurPoint,
			       IrtRType ModelParams[],
			       IrtRType *YPointer,
			       IrtRType YdParams[])
{
    IrtPtType DirectionToCenter, PointOnSphere, Phat, NTheta, NPhi;
    IrtRType PNDotProd,
	SinTheta = sin(ModelParams[2]),
	CosTheta = cos(ModelParams[2]),
	SinPhi = sin(ModelParams[1]),
	CosPhi = cos(ModelParams[1]);

    DirectionToCenter[0] = CosPhi * SinTheta;
    DirectionToCenter[1] = SinPhi * SinTheta;
    DirectionToCenter[2] = CosTheta;

    NPhi[0] = -DirectionToCenter[1];
    NPhi[1] = DirectionToCenter[0];
    NPhi[2] = 0.0;
    NTheta[0] = CosTheta * CosPhi;
    NTheta[1] = SinPhi * CosTheta;
    NTheta[2] = -SinTheta;
    IRIT_PT_SCALE2(PointOnSphere, DirectionToCenter, ModelParams[0]);
    IRIT_PT_SUB(Phat, CurPoint, PointOnSphere);

    if (IRIT_FABS(ModelParams[3]) < IRIT_EPS)
        *YPointer = IRIT_INFNTY;
    else
        *YPointer = ((ModelParams[3] * 0.5) * (IRIT_PT_SQR_LENGTH(Phat))) -
					    IRIT_DOT_PROD(Phat, DirectionToCenter);
    PNDotProd = IRIT_DOT_PROD(CurPoint, DirectionToCenter);
    YdParams[0] = ModelParams[3] * (ModelParams[0] - PNDotProd) + 1;
    YdParams[1] = (-ModelParams[3] * ModelParams[0] - 1) *
        IRIT_DOT_PROD(CurPoint, NPhi);
    YdParams[2] = (-ModelParams[3] * ModelParams[0] - 1) *
        IRIT_DOT_PROD(CurPoint, NTheta);
    YdParams[3] = 0.5 * (IRIT_PT_SQR_LENGTH(CurPoint) - 
			 2 * ModelParams[0] * IRIT_DOT_PROD(CurPoint,
						       DirectionToCenter) + 
			 IRIT_SQR(ModelParams[0]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from external parametrisation to internal         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   ExternalModelParams: (Xcenter, Ycenter, Zcenter, Radius).                *
*   InternalModelParams: (Length, Theta, Phi, 1/Radius).                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ExternalToIntrnalSphereParamsAux(IrtRType ExternalModelParams[],
					     IrtRType InternalModelParams[])
{
    IrtRType DistFromOrigin;
    IrtVecType VecToCenter;
    
    IRIT_PT_COPY(VecToCenter, ExternalModelParams);
    IRIT_VEC_SAFE_NORMALIZE(VecToCenter);
    DistFromOrigin = IRIT_PT_LENGTH(ExternalModelParams);

    InternalModelParams[0] = DistFromOrigin - ExternalModelParams[3];
    InternalModelParams[1] = atan2(VecToCenter[1], VecToCenter[0]);
    InternalModelParams[2] = atan2(IRIT_VEC2D_LENGTH(VecToCenter), VecToCenter[2]);
    InternalModelParams[3] = 1.0 / ExternalModelParams[3];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from parametrisation to internal external         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: (Length, Theta, Phi, 1/Radius).                     *
*   ExternalModelParams: (Xcenter, Ycenter, Zcenter, Radius).                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntrnalToExternalSphereParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[])
{
    IrtRType SinTheta, ScalingFactor;
    IrtVecType VecToCenter;
    
    SinTheta = sin(InternalModelParams[2]);
    VecToCenter[0] = cos(InternalModelParams[1]) * SinTheta;
    VecToCenter[1] = sin(InternalModelParams[1]) * SinTheta;
    VecToCenter[2] = cos(InternalModelParams[2]);
    ExternalModelParams[3] = IRIT_FABS(1.0 / InternalModelParams[3]);
    ScalingFactor = InternalModelParams[0] + ExternalModelParams[3];
    ExternalModelParams[0] = ScalingFactor * VecToCenter[0];
    ExternalModelParams[1] = ScalingFactor * VecToCenter[1];
    ExternalModelParams[2] = ScalingFactor * VecToCenter[2];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function writes to the Outputfile a formated representation of      *
* the Sphere.                                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   OutputFile:          The file to write to.                               *
*   ExternalModelParams: The fitted sphere parameters                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
 ****************************************************************************/
static void SphereResultPrintfAux(FILE* OutputFile,
				  IrtRType ExternalModelParams[])
{
    fprintf(OutputFile,
	    "X = %lf\tY = %lf\tZ = %lf\tR = %lf",
	    ExternalModelParams[0], ExternalModelParams[1], 
	    ExternalModelParams[2], ExternalModelParams[3]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function returns the distance of the point from the sphere.         *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   ExternalModelParams: The sphere (external representation).               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:      Computed  error.                                          *
*****************************************************************************/
static IrtRType CalcSphereFittingErrorAux(IrtRType *PointData,
					  IrtRType ExternalModelParams[])
{
    return IRIT_FABS(IRIT_PT_PT_DIST(ExternalModelParams, PointData) - 
		ExternalModelParams[3]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function returns the distance of the point from the sphere.         *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   InternalModelParams: The sphere (internal representation).               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:      Computed approximated error.                              *
*****************************************************************************/
static IrtRType CalcApproxSphereFittingErrorAux(IrtRType *PointData,
					  IrtRType InternalModelParams[])
{
    IrtRType 
	SinTheta = sin(InternalModelParams[2]),
        CosTheta = cos(InternalModelParams[2]),
        SinPhi = sin(InternalModelParams[1]),
        CosPhi = cos(InternalModelParams[1]);
    IrtPtType DirectionToCenter, PointOnSphere, Phat;
    
    if (IRIT_FABS(InternalModelParams[3]) < IRIT_EPS)
        return IRIT_INFNTY;

    DirectionToCenter[0] = CosPhi * SinTheta;
    DirectionToCenter[1] = SinPhi * SinTheta;
    DirectionToCenter[2] = CosTheta;

    IRIT_PT_SCALE2(PointOnSphere, DirectionToCenter, InternalModelParams[0]);
    IRIT_PT_SUB(Phat,PointData, PointOnSphere);

    return IRIT_FABS(((InternalModelParams[3] * 0.5) * 
			 (IRIT_PT_SQR_LENGTH(Phat))) -
				      IRIT_DOT_PROD(Phat, DirectionToCenter));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attepts to guess an initial estimate for the circle        *
* external model params.                                                     *
*   Based on an article by N. Chernov and C.Lesort: "Fitting circles and     *
* lines by least squares: theory and experiment", www.math.uab.edu/cl/cl1/.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PointList:            A list of PointData (X, Y, curvature).             *
*   NumberOfPointsInList: The number of points in PointList.                 *
*   ModelInitialParams:   The result vector of the initial params.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE on success.                                                    *
*****************************************************************************/
static int CircleInitialEstimateAux(IrtRType **PointList,
				    unsigned int NumberOfPointsInList,
				    IrtRType ModelInitialParams[])
{
    unsigned int i, j,
	BestI = -1,
	BestJ = -1,
	BestK = -1;
    IrtRType NewSqrDist, ConstDetFactor,
	MaxSqrDist = 0.0,
	MaxArea = 0.0;
    IrtPtType CenterPoint, Pt0, Pt1, Pt2;

    if (NumberOfPointsInList < NUM_OF_REQUIRED_POINTS_FOR_CIRCLE)
        return FALSE;
	
    if (NumberOfPointsInList > 3) {
        for (i = 0; i < NumberOfPointsInList - 1; ++i)
	    for (j = i + 1; j < NumberOfPointsInList; ++j) {
	        NewSqrDist = IRIT_PT2D_DIST_SQR(PointList[i], PointList[j]);
		if (NewSqrDist > MaxSqrDist) {
		    BestI = i;
		    BestJ = j;
		    MaxSqrDist = NewSqrDist;
		}
	    }

	if (IRIT_APX_EQ(MaxSqrDist, 0.0))
	    return FALSE;
		
	IRIT_PT2D_COPY(Pt0, PointList[BestI]);
	Pt0[2] = 0;
	IRIT_PT2D_COPY(Pt1, PointList[BestJ]);
	Pt1[2] = 0;
	ConstDetFactor = Pt0[0] * Pt1[1] - Pt0[1] * Pt1[0];	
	for (i = 0; i < NumberOfPointsInList; ++i) {
	    IrtRType 
	        CurArea = IRIT_FABS(Pt1[0] * PointList[i][1] + 
			       PointList[i][0] * Pt0[1] -
			       PointList[i][0] * Pt1[1] - 
			       Pt0[0] * PointList[i][1] + 
			       ConstDetFactor);

	    if (CurArea > MaxArea) {
	        MaxArea = CurArea;
		BestK = i;
	    }
	}
    }
    else {
        IRIT_PT2D_COPY(Pt0, PointList[0]);
	Pt0[2] = 0;
	IRIT_PT2D_COPY(Pt1, PointList[1]);
	Pt1[2] = 0;
	BestK = 2;
	MaxArea = Pt1[0] * PointList[2][1] + PointList[2][0] * Pt0[1] -
	          PointList[2][0] * Pt1[1] - Pt0[0] * PointList[2][1] + 
		  Pt0[0] * Pt1[1] - Pt0[1] * Pt1[0];	
    }

    if (IRIT_APX_EQ_EPS(MaxArea, 0.0, IRIT_UEPS))
        return FALSE;

    IRIT_PT2D_COPY(Pt2, PointList[BestK]);
    Pt2[2] = 0;
    GMCircleFrom3Points(CenterPoint, &ModelInitialParams[2], Pt0, Pt1, Pt2);
    IRIT_PT2D_COPY(ModelInitialParams, CenterPoint);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function validated that the current model is a valis model of       *
* some circle.                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: The current model params (internal representation). *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE iff valid, otherwise FALSE.                                    *
*****************************************************************************/
static int CircleModelValidatorAux(IrtRType InternalModelParams[])
{
    if ((1 + 4 * InternalModelParams[0] * InternalModelParams[1]) >= 0.0)
        return TRUE;
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivitives of the    *
* current point and the specified circle.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:    The current point to be calculated.                         *
*   ModelParams: The current model params (internal representation).         *
*   YPointer:    The distance of the point from the circle.                  *
*   YdParams:    The derivitives of the distance in the model params.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CircleShapeFuncAux(IrtPtType CurPoint,
			       IrtRType ModelParams[],
			       IrtRType* YPointer,
			       IrtRType YdParams[])
{
    IrtRType x, y, z, E, u, R, Q, InvQ, P, d, CosTheta, SinTheta;
    
    x = CurPoint[0] + ModelParams[3];
    y = CurPoint[1] + ModelParams[4];
    z = IRIT_SQR(x) + IRIT_SQR(y);

    CosTheta = cos(ModelParams[2]);
    SinTheta = sin(ModelParams[2]);

    u = x * CosTheta + y * SinTheta;
    E = (IrtRType)sqrt(1 + 4 * ModelParams[0] * ModelParams[1]);
    P = ModelParams[0] * z + E * u + ModelParams[1];
    Q = (IrtRType)sqrt(1 + 4 * ModelParams[0] * P);
    InvQ = 1.0 / Q;
    d = 2 * P / (1 + Q);
    *YPointer = d;
    R = 2 * (1 - ModelParams[0] * d * InvQ) / (1 + Q);

    YdParams[0] = (z + 2 * ModelParams[1] * u / E) * R - IRIT_SQR(d) * InvQ;
    YdParams[1] = (2 * ModelParams[0] * u / E + 1) * R; 
    YdParams[2] = (y * CosTheta - x * SinTheta) * E * R;
    YdParams[3] = 0;
    YdParams[4] = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from external parametrisation to internal         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   ExternalModelParams: (Xcenter, Ycenter, Radius).                         *
*   InternalModelParams: (A,D,Theta,XOffset,YOffset).                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ExternalToIntrnalCircleParamsAux(IrtRType ExternalModelParams[],
					     IrtRType InternalModelParams[])
{
    IrtRType B, C;
    
    InternalModelParams[0] = 0.5 / ExternalModelParams[2];
    B = -2.0 * InternalModelParams[0] * ExternalModelParams[0];
    C = -2.0 * InternalModelParams[0] * ExternalModelParams[1];
    InternalModelParams[1] = (IRIT_SQR(B) + IRIT_SQR(C) - 1) /
					         (4 * InternalModelParams[0]);
    InternalModelParams[2] = atan2(ExternalModelParams[0],
				   ExternalModelParams[1]);
    InternalModelParams[3] = 0.0;
    InternalModelParams[4] = 0.0;
    CircleNumericalProtectionAux(InternalModelParams);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from parametrisation to internal external         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: (A,D,Theta,XOffset,YOffset).                        *
*   ExternalModelParams: (Xcenter, Ycenter, Radius).                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntrnalToExternalCircleParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[])
{
    IrtRType B, C,
        E = (IrtRType) sqrt(1 + 4 * InternalModelParams[0]
			          * InternalModelParams[1]);

    B = E * cos(InternalModelParams[2]);
    C = E * sin(InternalModelParams[2]);
    ExternalModelParams[0] = -0.5 * B / InternalModelParams[0]
						      - InternalModelParams[3];
    ExternalModelParams[1] = -0.5 * C / InternalModelParams[0]
						      - InternalModelParams[4];
    ExternalModelParams[2] = 0.5 / IRIT_FABS(InternalModelParams[0]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function writes to the Output file a formated representation of     *
* the circle.                                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   OutputFile:          The file to write to.                               *
*   ExternalModelParams: The fitted circle parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CircleResultPrintfAux(FILE* OutputFile,
				  IrtRType ExternalModelParams[])
{
    fprintf(OutputFile,
	    "Center:( %lf , %lf)\tR = %lf",
	    ExternalModelParams[0],
	    ExternalModelParams[1],
	    ExternalModelParams[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This fucntion returns approximation of the distance of the point         *
*   from the circle.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   InternalModelParams: The sphere (internal representation).               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: An approximation of the fitting error.                         *
*****************************************************************************/
static IrtRType CalcApproxCircleFittingErrorAux(IrtRType *PointData,
					  IrtRType InternalModelParams[])
{
    IrtRType x, y, z, E, u, Q, P, CosTheta, SinTheta;
    
    x = PointData[0] + InternalModelParams[3];
    y = PointData[1] + InternalModelParams[4];
    z = IRIT_SQR(x) + IRIT_SQR(y);

    CosTheta = cos(InternalModelParams[2]);
    SinTheta = sin(InternalModelParams[2]);

    u = x * CosTheta + y * SinTheta;
    E = (IrtRType)sqrt(1 + 4 * InternalModelParams[0] * 
					   InternalModelParams[1]);
    P = InternalModelParams[0] * z + E * u + InternalModelParams[1];
    Q = (IrtRType)sqrt(1 + 4 * InternalModelParams[0] * P);

    return IRIT_FABS(2 * P / (1 + Q));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This fucntion returns the distance of the point from the circle.         *
*   from the circle.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   ExternalModelParams: The sphere (internal representation).               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The fittingError                                               *
*****************************************************************************/
static IrtRType CalcCircleFittingErrorAux(IrtRType *PointData,
					  IrtRType ExternalModelParams[])
{
    IrtRType 
	XDiff = ExternalModelParams[0] - PointData[0],
	YDiff = ExternalModelParams[1] - PointData[1];

    return IRIT_FABS(sqrt(IRIT_SQR(XDiff) + IRIT_SQR(YDiff)) -
						      ExternalModelParams[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function prevents numerical problems by updating the offset.        *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: In/Out internal model params.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CircleNumericalProtectionAux(IrtRType InternalModelParams[])
{
    IrtRType 
        SqrE = 1 + 4 * InternalModelParams[0] * InternalModelParams[1];

    if (IRIT_FABS(SqrE) < IRIT_EPS) {
	InternalModelParams[3] -= 1.0;
	InternalModelParams[4] -= 1.0;
	InternalModelParams[1] = (8.0 * IRIT_SQR(InternalModelParams[0]) - 1.0)
	  / (4.0 * InternalModelParams[0]);
	InternalModelParams[2] = IRIT_DEG2RAD(45.0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This functions calculates the element of matrix of A * x = 0.            *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:  The point for which A is calculates.                          *
*   BaseFuncs: The result.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PlaneBaseShapeFuncAux(IrtRType CurPoint[], IrtRType BaseFuncs[])
{
    IRIT_GEN_COPY(BaseFuncs, CurPoint, sizeof(IrtRType) * 3);
    BaseFuncs[3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the error for a given point from the fitted     *
* plane.                                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The point.                                          *
*   InternalModelParams: The fitted plane.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The Euclidian distance of the point from the plane.            *
*****************************************************************************/
static IrtRType CalcPlaneFittingErrorAux(IrtRType *PointData,
					 IrtRType InternalModelParams[])
{
    return IRIT_FABS(GMDistPointPlane(PointData, InternalModelParams));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function writes to the Outputfile a formated representation of      *
* the plane.                                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   OutputFile:          The file to write to.                               *
*   ExternalModelParams: The fitted plane parameters.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PlaneResultPrintfAux(FILE *OutputFile,
				 IrtRType ExternalModelParams[])
{
    fprintf(OutputFile,
	    "A = %lf\tB = %lf\tC = %lf\tD = %lf",
	    ExternalModelParams[0], ExternalModelParams[1],
	    ExternalModelParams[2], ExternalModelParams[3]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function returns the plane normalization constraint.                *
* Because this normalization constraint could be coplanar with the plane     *
* itself, we try it 3 times with 3 orthogonal normalization vectors.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Combination:    The combination of A,B,C,D.                              *
*   ExpectedResult: The expected result.                                     *
*   Trial:          This trial attempt number.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PlaneAdditionalConstraintAux(IrtRType Combination[],
					 IrtRType *ExpectedResult,
					 int Trial)
{
    *ExpectedResult = 1.0;

    IRIT_GEN_COPY(Combination, PlaneNormalizationConstraints[Trial % 3],
	          sizeof(IrtRType) * 4);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from external parametrisation to internal         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: (Length, Phi, Theta, 1/Radius, Sigma,               *
*                         Tau, XOff, YOff, ZOff)                             *
*   ExternalModelParams: (ApexX, ApexY, ApexZ, Apex Semi angle               *
*                         ConeDirX, ConeDirY, ConeDirZ)                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InternalToExternalConeParamsAux(IrtRType InternalModelParams[],
					    IrtRType ExternalModelParams[])
{
    IrtRType ApexAng, DefPlaneNormLength,
        SinTheta = sin(InternalModelParams[2]),
	CosTheta = cos(InternalModelParams[2]),
        SinPhi = sin(InternalModelParams[1]),
        CosPhi = cos(InternalModelParams[1]),
        SinSigma = sin(InternalModelParams[4]),
        CosSigma = cos(InternalModelParams[4]),
        SinTau = sin(InternalModelParams[5]),
        CosTau = cos(InternalModelParams[5]);
    IrtVecType DirectionToCone, AxisDirection, DefPlaneNorm;
    
    DirectionToCone[0] = CosPhi * SinTheta;
    DirectionToCone[1] = SinPhi * SinTheta;
    DirectionToCone[2] = CosTheta;

    AxisDirection[0] = CosSigma * SinTau;
    AxisDirection[1] = SinSigma * SinTau;
    AxisDirection[2] = CosTau;
 
    IRIT_CROSS_PROD(DefPlaneNorm, DirectionToCone, AxisDirection);
    DefPlaneNormLength = IRIT_VEC_LENGTH(DefPlaneNorm);
    ApexAng = acos(DefPlaneNormLength);
    ExternalModelParams[3] = ApexAng;
    ExternalModelParams[4] = AxisDirection[0];
    ExternalModelParams[5] = AxisDirection[1];
    ExternalModelParams[6] = AxisDirection[2];
    if ((IRIT_FABS(ApexAng) > IRIT_EPS) && 
	(IRIT_FABS(InternalModelParams[3]) > IRIT_EPS)) {
	IrtRType TanAng, Radius;
	IrtPtType PointOnCone, PointOnAxis, ApexPoint;
	IrtVecType TangDir, VecToAxis, VecToApex;

	IRIT_VEC_SAFE_NORMALIZE(DefPlaneNorm);
	TanAng = IRIT_DOT_PROD(DirectionToCone, AxisDirection) / DefPlaneNormLength;
	Radius = DefPlaneNormLength / InternalModelParams[3];
	IRIT_CROSS_PROD(TangDir, AxisDirection, DefPlaneNorm);
	
	PointOnCone[0] = 
	  DirectionToCone[0] * InternalModelParams[0] + InternalModelParams[6];
	PointOnCone[1] = 
	  DirectionToCone[1] * InternalModelParams[0] + InternalModelParams[7];
	PointOnCone[2] = 
	  DirectionToCone[2] * InternalModelParams[0] + InternalModelParams[8];
	IRIT_VEC_SCALE2(VecToAxis, TangDir, Radius);
	IRIT_VEC_SCALE2(VecToApex, AxisDirection,  - TanAng * Radius);
	IRIT_PT_ADD(PointOnAxis, PointOnCone, VecToAxis);
	IRIT_PT_ADD(ApexPoint, PointOnAxis, VecToApex);
	ExternalModelParams[0] = ApexPoint[0];
	ExternalModelParams[1] = ApexPoint[1];
	ExternalModelParams[2] = ApexPoint[2];
    }
    else
        IRIT_WARNING_MSG("Internal representation doesn't match a valid cone.");
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   This fucntion returns the distance of the point from the Cylinder.       *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The current point coordinates.                      *
*   ExternalModelParams: (ApexX, ApexY, ApexZ, Apex Semi angle               *
*                         ConeDirX, ConeDirY, ConeDirZ)                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: the distance of the point from the cone.                       *
*****************************************************************************/
static IrtRType CalcConeFittingErrorAux(IrtRType *PointData,
					IrtRType ExternalModelParams[])
{
    IrtRType Dist1, Dist2;
    IrtPtType TransformedDataPoint, TransformedApexPoint;
    IrtVecType VecToPoint, ConeDir, PlaneNormal, TestDir1, TestDir2;
    IrtHmgnMatType RotationMat, InvRotMat;

    IRIT_VEC_COPY(ConeDir, &ExternalModelParams[4]);
    IRIT_PT_SUB(VecToPoint, ExternalModelParams, PointData);
    IRIT_CROSS_PROD(PlaneNormal, VecToPoint, ConeDir);
    IRIT_VEC_SAFE_NORMALIZE(PlaneNormal)
    GMGenMatrixZ2Dir(InvRotMat, PlaneNormal);
    MatTranspMatrix(InvRotMat, RotationMat);       /* Compute the inverse. */
    MatMultPtby4by4(TransformedDataPoint, PointData, RotationMat);
    MatMultPtby4by4(TransformedApexPoint, ExternalModelParams, RotationMat);
    TransformedApexPoint[2] = 0.0;
    TransformedDataPoint[2] = 0.0;
    IRIT_VEC_SET(TestDir1, cos(ExternalModelParams[3]), 
	    sin(ExternalModelParams[3]), 0.0);
    IRIT_VEC_SET(TestDir2, TestDir1[0], -TestDir1[1], 0.0);
    Dist1 = GMDistPointLine(TransformedDataPoint,
			    TransformedApexPoint, TestDir1);
    Dist2 = GMDistPointLine(TransformedDataPoint,
			    TransformedApexPoint, TestDir2);
    return Dist1 < Dist2 ? Dist1 : Dist2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attepts to guess an initial estimate for the Cone          *
* internal model params.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointList:          A list of PointData (X, Y, Z, K1, K2, K1CurvX,       *
*                       K1CurvY, K1CurvZ, K2CurvX, K2CurvY, K2CurvZ,         *
*                       NormX, NormY, NormZ)                                 *
*   NumberOfPointsInList:   The number of data points.                       *
*   ModelInitialParams: The output initial estimate of the internal params.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE on success.                                                    *
*****************************************************************************/
static int ConeInitialEstimateAux(IrtRType **PointList,
				  unsigned int NumberOfPointsInList,
				  IrtRType ModelInitialParams[])
{
    unsigned int i;
    IrtVecType EstimatedRotationAxisDirection, EstimatedNormal, *Normals;
    IrtPtType PointOnEstimatedRotationAxis, *PointsOnObject;

    if (NumberOfPointsInList < NUM_OF_REQUIRED_POINTS_FOR_CONE)
        return FALSE;
    
    PointsOnObject = 
        (IrtPtType*) IritMalloc(sizeof(IrtPtType) * NumberOfPointsInList);
    Normals = 
        (IrtVecType*) IritMalloc(sizeof(IrtVecType) * NumberOfPointsInList);
    if ((PointsOnObject == NULL) || (Normals == NULL)) {
	IRIT_FATAL_ERROR("Unable to allocate memory.");
	return FALSE;
    }
	
    EstimatedNormal[0] = PointList[0][11];
    EstimatedNormal[1] = PointList[0][12];
    EstimatedNormal[2] = PointList[0][13];
    ModelInitialParams[0] = 0.0;
    ModelInitialParams[1] = atan2(EstimatedNormal[0], EstimatedNormal[1]);
    ModelInitialParams[2] = 
        atan2(IRIT_VEC2D_LENGTH(EstimatedNormal), EstimatedNormal[2]);
    for (i = 0; i < NumberOfPointsInList; ++i) {
	IRIT_PT_COPY(PointsOnObject[i], PointList[i]);
	IRIT_VEC_COPY(Normals[i], &(PointList[i][11]));
    }
    GMFitEstimateRotationAxis(PointsOnObject, Normals, NumberOfPointsInList,
			      PointOnEstimatedRotationAxis,
			      EstimatedRotationAxisDirection);
    IritFree(PointsOnObject);
    IritFree(Normals);
	
    ModelInitialParams[4] = atan2(EstimatedRotationAxisDirection[0],
				  EstimatedRotationAxisDirection[1]);
    ModelInitialParams[5] = 
        atan2(IRIT_VEC2D_LENGTH(EstimatedRotationAxisDirection), 
	      EstimatedRotationAxisDirection[2]);
    if (IRIT_FABS(PointList[0][3]) < IRIT_FABS(PointList[0][4]))
        ModelInitialParams[3] = PointList[0][4];
    else
        ModelInitialParams[3] = PointList[0][3];
    ModelInitialParams[6] = PointList[0][0];
    ModelInitialParams[7] = PointList[0][1];
    ModelInitialParams[8] = PointList[0][2];
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the error for a given point from the fitted     *
* cone.                                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The point.                                          *
*   InternalModelParams: The fitted cone.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The Euclidian distance of the point from the cone.             *
*****************************************************************************/
static IrtRType CalcApproxConeFittingErrorAux(IrtRType *PointData,
					      IrtRType InternalModelParams[])
{
    IrtRType NACrossProdSqrLen, PHatADotProd, Nominator,
        SinTheta = sin(InternalModelParams[2]),
        CosTheta = cos(InternalModelParams[2]),
        SinPhi = sin(InternalModelParams[1]),
        CosPhi = cos(InternalModelParams[1]),
        SinSigma = sin(InternalModelParams[4]),
        CosSigma = cos(InternalModelParams[4]),
        SinTau = sin(InternalModelParams[5]),
        CosTau = cos(InternalModelParams[5]);
    IrtVecType N, A, NACrossProd;
    IrtPtType PHat;

    if (IRIT_FABS(InternalModelParams[3]) < IRIT_EPS)
        return IRIT_INFNTY;
    
    N[0] = CosPhi * SinTheta;
    N[1] = SinPhi * SinTheta;
    N[2] = CosTheta;

    A[0] = CosSigma * SinTau;
    A[1] = SinSigma * SinTau;
    A[2] = CosTau;
    
    IRIT_CROSS_PROD(NACrossProd, N, A);
    NACrossProdSqrLen = IRIT_VEC_SQR_LENGTH(NACrossProd);
    
    PHat[0] = 
      PointData[0] - (InternalModelParams[0] * N[0] + InternalModelParams[6]);
    PHat[1] = 
      PointData[1] - (InternalModelParams[0] * N[1] + InternalModelParams[7]);
    PHat[2] = 
      PointData[2] - (InternalModelParams[0] * N[2] + InternalModelParams[8]);
    
    PHatADotProd = IRIT_DOT_PROD(PHat, A);
    
    Nominator = 0.5 * InternalModelParams[3] *
      (NACrossProdSqrLen * IRIT_PT_SQR_LENGTH(PHat) - IRIT_SQR(PHatADotProd)) - 
	    (IRIT_DOT_PROD(PHat, N) * NACrossProdSqrLen);
    
    return IRIT_FABS(Nominator / 
		(InternalModelParams[3] * PHatADotProd * IRIT_DOT_PROD(N, A) + 
		 NACrossProdSqrLen));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivitives of the    *
* current point and the specified cone.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:    The current point to be calculated.                         *
*   ModelParams: The current model params (internal representation).         *
*   YPointer:    The distance of the point from the cone.                    *
*   YdParams:    The derivitives of the distance in the model params.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ConeShapeFuncAux(IrtPtType CurPoint,
			     IrtRType ModelParams[],
			     IrtRType* YPointer,
			     IrtRType YdParams[])
{
    IrtRType Mu, PHatADotProd, Nominator, Lambda, Epsilon, Eta, DerivDenom,
        TPointADotProd, PHatSqrLen, NADotProd, PHatNDotProd, k, SqrK, 
	NPhiADotProd, NThetaADotProd, NASigmaDotProd, NATauDotProd, 
        PHatASigmaDotProd, PHatATauDotProd, LambdaDRho, EpsilonDRho, MuDRho,
	LambdaDPhi, EpsilonDPhi, MuDPhi, EtaDPhi, LambdaDTheta, EpsilonDTheta,
        MuDTheta, EtaDTheta, LambdaDSigma, EpsilonDSigma, MuDSigma, EtaDSigma,
        LambdaDTau, EpsilonDTau, MuDTau, EtaDTau,  
	SinTheta = sin(ModelParams[2]),
        CosTheta = cos(ModelParams[2]),
        SinPhi = sin(ModelParams[1]),
        CosPhi = cos(ModelParams[1]),
        SinSigma = sin(ModelParams[4]),
        CosSigma = cos(ModelParams[4]),
        SinTau = sin(ModelParams[5]),
        CosTau = cos(ModelParams[5]);
    IrtVecType N, A, NACrossProd, NPhi, NTheta, ASigma, ATau;
    IrtPtType PHat, P, TPoint;
    
    N[0] = CosPhi * SinTheta;
    N[1] = SinPhi * SinTheta;
    N[2] = CosTheta;
    NPhi[0] = -N[1];
    NPhi[1] = N[0];
    NPhi[2] = 0.0;
    NTheta[0] =  CosTheta * CosPhi;
    NTheta[1] = SinPhi * CosTheta;
    NTheta[2] = -SinTheta;
    
    A[0] = CosSigma * SinTau;
    A[1] = SinSigma * SinTau;
    A[2] = CosTau;
    ASigma[0] = -A[1];
    ASigma[1] = A[0];
    ASigma[2] = 0.0;
    ATau[0] = CosSigma * CosTau;
    ATau[1] = SinSigma * CosTau;
    ATau[2] = -SinTau;
    
    IRIT_CROSS_PROD(NACrossProd, N, A);
    Eta = IRIT_VEC_SQR_LENGTH(NACrossProd);

    /* Compensate for the initial translation. */ 
    P[0] = CurPoint[0] - ModelParams[6];
    P[1] = CurPoint[1] - ModelParams[7];
    P[2] = CurPoint[2] - ModelParams[8];

    PHat[0] = P[0] - ModelParams[0] * N[0];
    PHat[1] = P[1] - ModelParams[0] * N[1];
    PHat[2] = P[2] - ModelParams[0] * N[2];
    
    TPoint[0] = P[0] - 2.0 * ModelParams[0] * N[0];
    TPoint[1] = P[1] - 2.0 * ModelParams[0] * N[1];
    TPoint[2] = P[2] - 2.0 * ModelParams[0] * N[2];
    
    k = ModelParams[3];
    SqrK = IRIT_SQR(k);

    PHatSqrLen = IRIT_PT_SQR_LENGTH(PHat);
    PHatADotProd = IRIT_DOT_PROD(PHat, A);
    NADotProd = IRIT_DOT_PROD(N, A);
    PHatNDotProd = IRIT_DOT_PROD(PHat, N);
    NPhiADotProd = IRIT_DOT_PROD(NPhi, A);
    NThetaADotProd = IRIT_DOT_PROD(NTheta, A);
    TPointADotProd = IRIT_DOT_PROD(TPoint, A);
    NASigmaDotProd = IRIT_DOT_PROD(N, ASigma);
    NATauDotProd = IRIT_DOT_PROD(N, ATau);
    PHatASigmaDotProd = IRIT_DOT_PROD(PHat, ASigma);
    PHatATauDotProd = IRIT_DOT_PROD(PHat, ATau);

    Mu = PHatADotProd * NADotProd;
    Epsilon = -PHatNDotProd * Eta;
    Lambda = 0.5 * (Eta * PHatSqrLen - IRIT_SQR(PHatADotProd));

    LambdaDRho = ModelParams[0] * (Eta - 2.0 * IRIT_SQR(NADotProd)) +
	         2.0 * IRIT_DOT_PROD(P, A) * NADotProd - IRIT_DOT_PROD(P, N) * Eta;
    EpsilonDRho = 2.0 * Eta;
    MuDRho = -Eta;
    
    LambdaDPhi = 2.0 * ModelParams[0] *PHatADotProd * NPhiADotProd -
                 NPhiADotProd * NADotProd * PHatSqrLen -
	         ModelParams[0] *  Eta * IRIT_DOT_PROD(NPhi, P);  
    EpsilonDPhi = 2.0 * NPhiADotProd * NADotProd - IRIT_DOT_PROD(PHat, NPhi) * Eta;
    MuDPhi = NPhiADotProd * TPointADotProd;
    EtaDPhi = -2.0 * NPhiADotProd * NADotProd;
    
    LambdaDTheta = 2.0 * ModelParams[0] *PHatADotProd * NThetaADotProd -
                   NThetaADotProd * NADotProd * PHatSqrLen -
	           ModelParams[0] *  Eta * IRIT_DOT_PROD(NTheta ,P); 
    EpsilonDTheta = 2.0 * NThetaADotProd * NADotProd - 
                                                 IRIT_DOT_PROD(PHat, NTheta) * Eta;
    MuDTheta =  NThetaADotProd * TPointADotProd;
    EtaDTheta = -2.0 * NThetaADotProd * NADotProd;
    
    LambdaDSigma = - (NASigmaDotProd * NADotProd * PHatSqrLen +
		      PHatADotProd * PHatASigmaDotProd);
    EpsilonDSigma = PHatNDotProd * NASigmaDotProd * NADotProd;
    MuDSigma = PHatASigmaDotProd * NADotProd + PHatADotProd * NASigmaDotProd;
    EtaDSigma = -2.0 * NASigmaDotProd * NADotProd;

    LambdaDTau =  - (NATauDotProd * NADotProd * PHatSqrLen +
		     PHatADotProd * PHatATauDotProd);
    EpsilonDTau = PHatNDotProd * NATauDotProd * NADotProd;
    MuDTau = PHatATauDotProd * NADotProd + PHatADotProd * NATauDotProd;
    EtaDTau = -2.0 * NATauDotProd * NADotProd;

    Nominator = 0.5 * k * (Eta * PHatSqrLen - IRIT_SQR(PHatADotProd)) - 
	(PHatNDotProd * Eta);
    DerivDenom = 1.0 / IRIT_SQR(Mu * k + Eta);

    if (IRIT_FABS(k) < IRIT_EPS)
        *YPointer = IRIT_INFNTY;
    else
        *YPointer = Nominator / (k * PHatADotProd * NADotProd + Eta);

    YdParams[0] = 
	((LambdaDRho * Mu - Lambda * MuDRho) * SqrK + 
	 (LambdaDRho * Eta + EpsilonDRho * Mu - Epsilon * MuDRho) * k +
	 EpsilonDRho * Eta)  *DerivDenom;
    YdParams[1] = 
	((LambdaDPhi * Mu - Lambda * MuDPhi) * SqrK +  
	 (LambdaDPhi * Eta + EpsilonDPhi * Mu - Lambda * EtaDPhi -
	  Epsilon * MuDPhi) * k + EpsilonDPhi * Eta - 
	 Epsilon * EtaDPhi) * DerivDenom;
    YdParams[2] = 
	((LambdaDTheta * Mu - Lambda * MuDTheta) * SqrK +  
	 (LambdaDTheta * Eta + EpsilonDTheta * Mu - Lambda * EtaDTheta -
	  Epsilon * MuDTheta) * k + EpsilonDTheta * Eta - 
	 Epsilon * EtaDTheta) * DerivDenom;
    YdParams[3] = (Lambda * Eta - Mu * Epsilon) * DerivDenom;
    YdParams[4] = 
	((LambdaDSigma * Mu - Lambda * MuDSigma) * SqrK +  
	 (LambdaDSigma * Eta + EpsilonDSigma * Mu - Lambda * EtaDSigma -
	  Epsilon * MuDSigma) * k + EpsilonDSigma * Eta - 
	 Epsilon * EtaDSigma) * DerivDenom;
    YdParams[5] = 
	((LambdaDTau * Mu - Lambda * MuDTau) * SqrK +  
	 (LambdaDTau * Eta + EpsilonDTau * Mu - Lambda * EtaDTau -
	  Epsilon * MuDTau) * k + EpsilonDTau * Eta - 
	 Epsilon * EtaDTau) * DerivDenom;
    YdParams[6] = 0.0;
    YdParams[7] = 0.0;
    YdParams[8] = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function writes to the Output file a formated representation of     *
* the Cone.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   OutputFile:          The file to write to.                               *
*   ExternalModelParams: The fitted cone parameters.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ConeResultPrintfAux(FILE *OutputFile,
				IrtRType ExternalModelParams[])
{
    fprintf (OutputFile,
	     "Point:( %lf, %lf, %lf ) Direction:( %lf, %lf, %lf ) R = %lf",
	     ExternalModelParams[0], ExternalModelParams[1],
	     ExternalModelParams[2], ExternalModelParams[4],
	     ExternalModelParams[5], ExternalModelParams[6],
	     ExternalModelParams[3]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function converts from external parametrisation to internal         *
* parametrisation.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   InternalModelParams: (Length, Phi, Theta, 1/DiscRadius, 1/ExtRadius,     *
*                         Sigma, Tau, XOff, YOff, ZOff)                      *
*   ExternalModelParams: (PointOnTorusX, PointOnTorusY, PointOnTorusZ,       *
*                         DiscRadius, ExtRadius, TorDirX, TorDirY, TorDirZ)  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InternalToExternalTorusParamsAux(IrtRType InternalModelParams[],
					     IrtRType ExternalModelParams[])
{
    IrtRType 
        SinTheta = sin(InternalModelParams[2]),
        CosTheta = cos(InternalModelParams[2]),
        SinPhi = sin(InternalModelParams[1]),
        CosPhi = cos(InternalModelParams[1]),
        SinSigma = sin(InternalModelParams[5]),
        CosSigma = cos(InternalModelParams[5]),
        SinTau = sin(InternalModelParams[6]),
        CosTau = cos(InternalModelParams[6]);
    IrtVecType DirectionToTorus;
    
    DirectionToTorus[0] = CosPhi * SinTheta;
    DirectionToTorus[1] = SinPhi * SinTheta;
    DirectionToTorus[2] = CosTheta;
    
    ExternalModelParams[0] = 
        DirectionToTorus[0] * InternalModelParams[0] + InternalModelParams[7];
    ExternalModelParams[1] = 
        DirectionToTorus[1] * InternalModelParams[0] + InternalModelParams[8];
    ExternalModelParams[2] = 
        DirectionToTorus[2] * InternalModelParams[0] + InternalModelParams[9];

    if (IRIT_FABS(InternalModelParams[3]) > IRIT_EPS) 
        ExternalModelParams[3] = 1 / IRIT_FABS(InternalModelParams[3]);
    else
        ExternalModelParams[3] = IRIT_INFNTY;

    if (IRIT_FABS(InternalModelParams[4]) > IRIT_EPS) 
        ExternalModelParams[4] = 1 / IRIT_FABS(InternalModelParams[4]);
    else
        ExternalModelParams[4] = IRIT_INFNTY;

    ExternalModelParams[5] = CosSigma * SinTau;
    ExternalModelParams[6] = SinSigma * SinTau;
    ExternalModelParams[7] = CosTau;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attepts to guess an initial estimate for the torus         *
* internal model params.                                                     *
* The code assumes that the external radius is larger then the disc radius   *
* for other cases the code should be modified.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   PointList:          A list of PointData (X, Y, Z, K1, K2, K1CurvX,       *
*                       K1CurvY, K1CurvZ, K2CurvX, K2CurvY, K2CurvZ,         *
*                       NormX, NormY, NormZ)                                 *
*   NumberOfPointsInList:   The number of data points.                       *
*   ModelInitialParams: The output initial estimate of the internal params.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE on success.                                                    *
*****************************************************************************/
static int TorusInitialEstimateAux(IrtRType **PointList,
				   unsigned int NumberOfPointsInList,
				   IrtRType ModelInitialParams[])
{
    unsigned int i;
    IrtVecType EstimatedRotationAxisDirection, EstimatedNormal, *Normals;
    IrtPtType PointOnEstimatedRotationAxis, *PointsOnObject;

    if (NumberOfPointsInList < NUM_OF_REQUIRED_POINTS_FOR_TORUS)
        return FALSE;

    PointsOnObject = 
        (IrtPtType*) IritMalloc(sizeof(IrtPtType) * NumberOfPointsInList);
    Normals = 
        (IrtVecType*) IritMalloc(sizeof(IrtVecType) * NumberOfPointsInList);
    if ((PointsOnObject == NULL) || (Normals == NULL)) {
	IRIT_FATAL_ERROR("Unable to allocate memory.");
	return FALSE;
    }

    EstimatedNormal[0] = PointList[0][11];
    EstimatedNormal[1] = PointList[0][12];
    EstimatedNormal[2] = PointList[0][13];
    ModelInitialParams[0] = 0.0;
    ModelInitialParams[1] = atan2(EstimatedNormal[0], EstimatedNormal[1]);
    ModelInitialParams[2] = 
        atan2(IRIT_VEC2D_LENGTH(EstimatedNormal), EstimatedNormal[2]);

    for (i = 0; i < NumberOfPointsInList; ++i) {
	IRIT_PT_COPY(PointsOnObject[i], PointList[i]);
	IRIT_VEC_COPY(Normals[i], &(PointList[i][11]));
    }
    GMFitEstimateRotationAxis(PointsOnObject, Normals, NumberOfPointsInList,
			      PointOnEstimatedRotationAxis,
			      EstimatedRotationAxisDirection);
    IritFree(PointsOnObject);
    IritFree(Normals);

    ModelInitialParams[5] = atan2(EstimatedRotationAxisDirection[0],
				  EstimatedRotationAxisDirection[1]);
    ModelInitialParams[6] = 
        atan2(IRIT_VEC2D_LENGTH(EstimatedRotationAxisDirection), 
	      EstimatedRotationAxisDirection[2]);
    ModelInitialParams[7] = PointList[0][0];
    ModelInitialParams[8] = PointList[0][1];
    ModelInitialParams[9] = PointList[0][2];

    /* Appropriate to ExtRad > DiscRad - otherwise should be modified. */
    if (IRIT_FABS(PointList[0][3]) < IRIT_FABS(PointList[0][4])){
	ModelInitialParams[4] = PointList[0][3];
	ModelInitialParams[3] = PointList[0][4];
    }
    else {
	ModelInitialParams[3] = PointList[0][3];
	ModelInitialParams[4] = PointList[0][4];
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the error for a given point from the fitted     *
* torus.                                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The point.                                          *
*   InternalModelParams: The fitted torus.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The Euclidian distance of the point from the torus.            *
*****************************************************************************/
static IrtRType CalcTorusFittingErrorAux(IrtRType *PointData,
					 IrtRType InternalModelParams[])
{
    IrtRType 
	SinTheta = sin(InternalModelParams[2]),
	CosTheta = cos(InternalModelParams[2]),
	SinPhi = sin(InternalModelParams[1]),
	CosPhi = cos(InternalModelParams[1]),
	SinSigma = sin(InternalModelParams[5]),
	CosSigma = cos(InternalModelParams[5]),
	SinTau = sin(InternalModelParams[6]),
	CosTau = cos(InternalModelParams[6]);

    if ((IRIT_FABS(InternalModelParams[3]) < IRIT_EPS)||
	(IRIT_FABS(InternalModelParams[4]) < IRIT_EPS))
        return IRIT_INFNTY;
    
    
    if (IRIT_FABS(InternalModelParams[3]) <
	IRIT_FABS(InternalModelParams[4])) {
	IrtRType AppleRes, LemonRes, Err;
	AppleRes = 
	    CalcTorusFittingErrorWEpsAux(PointData, InternalModelParams, 1.0,
					 SinPhi, CosPhi, SinTheta, CosTheta,
					 SinSigma, CosSigma, SinTau, CosTau);
	LemonRes = 
	    CalcTorusFittingErrorWEpsAux(PointData, InternalModelParams, -1.0,
					 SinPhi, CosPhi, SinTheta, CosTheta,
					 SinSigma, CosSigma, SinTau, CosTau);
	Err = IRIT_MIN(AppleRes, LemonRes);
	return IRIT_FABS(Err);
    }
    else
        return IRIT_FABS(CalcTorusFittingErrorWEpsAux(PointData,
						 InternalModelParams, 1.0,
						 SinPhi, CosPhi, SinTheta,
						 CosTheta, SinSigma, CosSigma,
						 SinTau, CosTau));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the error for a given point from the fitted     *
* torus, for a specified eps.                                                *
* Esp is 1 for the external portion of the torus and -1 for the internal.    *
* Trigonometric values are also portion of the input to accelarate.          *
*                                                                            *
* PARAMETERS:                                                                *
*   PointData:           The point.                                          *
*   InternalModelParams: The fitted torus.                                   *
*   Eps:      1.0 for the external portion, -1.0 for the internal portion.   *
*   SinPhi:   sin(phi).                                                      *
*   CosPhi:   cos(phi).                                                      *
*   SinTheta: sin(theta).                                                    *
*   CosTheta: cos(theta).                                                    *
*   SinSigma: sin(sigma).                                                    *
*   CosSigma: cos(sigma).                                                    *
*   SinTau:   sin(tau).                                                      *
*   CosTau:   cos(tau).                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The euclidean fitting error.                                   *
*****************************************************************************/
static IrtRType CalcTorusFittingErrorWEpsAux(IrtRType *PointData,
					     IrtRType InternalModelParams[],
					     IrtRType Eps, 
					     IrtRType SinPhi, 
					     IrtRType CosPhi,
					     IrtRType SinTheta, 
					     IrtRType CosTheta,
					     IrtRType SinSigma, 
					     IrtRType CosSigma,
					     IrtRType SinTau, 
					     IrtRType CosTau)
{
    IrtRType D0, DelEps, SubExpSign, InvS, NACrossProdLen,
        S = InternalModelParams[4],
        K = InternalModelParams[3];
    IrtVecType N, A, NACrossProd, ScaledN, TransPHatACrossProd;
    IrtPtType PHat, TransPHat;

    if ((IRIT_FABS(InternalModelParams[3]) < IRIT_EPS)||
	(IRIT_FABS(InternalModelParams[4]) < IRIT_EPS))
        return IRIT_INFNTY;
    
    InvS = 1.0 / S; 
    
    N[0] = CosPhi * SinTheta;
    N[1] = SinPhi * SinTheta;
    N[2] = CosTheta;
    
    A[0] = CosSigma * SinTau;
    A[1] = SinSigma * SinTau;
    A[2] = CosTau;
    
    IRIT_CROSS_PROD(NACrossProd, N, A);
    NACrossProdLen = IRIT_VEC_LENGTH(NACrossProd);
    
    PHat[0] = 
      PointData[0] - (InternalModelParams[0] * N[0] + InternalModelParams[6]);
    PHat[1] = 
      PointData[1] - (InternalModelParams[0] * N[1] + InternalModelParams[7]);
    PHat[2] = 
      PointData[2] - (InternalModelParams[0] * N[2] + InternalModelParams[8]);
    
    SubExpSign = IRIT_SIGN((IRIT_SQR(K) * InvS) - K);
    IRIT_VEC_SCALE2(ScaledN, N, InvS);
    IRIT_PT_SUB(TransPHat, PHat, ScaledN);
    IRIT_CROSS_PROD(TransPHatACrossProd, TransPHat, A);
    
    D0 = 0.5 * K *IRIT_PT_SQR_LENGTH(PHat) - IRIT_DOT_PROD(PHat, N);
    DelEps = 
      (Eps * SubExpSign * IRIT_PT_LENGTH(TransPHatACrossProd) * NACrossProdLen +
       IRIT_DOT_PROD(TransPHatACrossProd, NACrossProd)) * (K * InvS - 1.0);
    return D0 - DelEps;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the distance and distance derivitives of the    *
* current point and the specified torus.                                     *
*                                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   CurPoint:    The current point to be calculated.                         *
*   ModelParams: The current model params (internal representation).         *
*   YPointer:    The distance of the point from the torus.                   *
*   YdParams:    The derivitives of the distance in the model params.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TorusShapeFuncAux(IrtPtType CurPoint,
			      IrtRType ModelParams[],
			      IrtRType *YPointer,
			      IrtRType YdParams[])
{
    unsigned int i;
    IrtRType Eps, SubExpSign, InvS, NACrossProdLen,
        KDSM1, VUDotProd, TransPHatACrossProdLen, PNDotProd,
        D0DParams[NUM_OF_TORUS_INTERNAL_PARAMS],
	DelEpsDParams[NUM_OF_TORUS_INTERNAL_PARAMS],
        SinTheta = sin(ModelParams[2]),
        CosTheta = cos(ModelParams[2]),
        SinPhi = sin(ModelParams[1]),
        CosPhi = cos(ModelParams[1]),
        SinSigma = sin(ModelParams[5]),
        CosSigma = cos(ModelParams[5]),
        SinTau = sin(ModelParams[6]),
        CosTau = cos(ModelParams[6]),
        S = ModelParams[4],
        K = ModelParams[3];
    IrtVecType N, A, NACrossProd, ScaledN, TransPHatACrossProd, V, U,
        NPhi, NTheta, NPhiACrossProd, NThetaACrossProd, ATau, ASigma,
        TransPHatATauCrossProd, TransPHatASigmaCrossProd,
        NATauCrossProd, NASigmaCrossProd;
    IrtPtType P, PHat, TransPHat;

    if ((IRIT_FABS(ModelParams[3]) < IRIT_EPS)||
	(IRIT_FABS(ModelParams[4]) < IRIT_EPS)) {
	*YPointer = IRIT_INFNTY;
	Eps = 1.0; 
    }
    else {
	if (IRIT_FABS(ModelParams[3]) < IRIT_FABS(ModelParams[4])) {
	    IrtRType AppleRes, LemonRes;
	    AppleRes = 
	      CalcTorusFittingErrorWEpsAux(CurPoint, ModelParams, 1.0,
					   SinPhi, CosPhi, SinTheta, CosTheta,
					   SinSigma, CosSigma, SinTau, CosTau);
	    LemonRes = 
	      CalcTorusFittingErrorWEpsAux(CurPoint, ModelParams, 
					   -1.0, SinPhi, CosPhi, SinTheta, 
					   CosTheta, SinSigma, CosSigma, 
					   SinTau, CosTau);
	    if (IRIT_FABS(AppleRes) < IRIT_FABS(LemonRes)) {
		Eps = 1.0;
		*YPointer = AppleRes;
	    }
	    else {
		Eps = -1.0;
		*YPointer = LemonRes;
	    }
	}
	else {
	    Eps = 1.0;
	    *YPointer = 
	        CalcTorusFittingErrorWEpsAux(CurPoint, ModelParams, 
					     1.0, SinPhi, CosPhi, SinTheta,
					     CosTheta, SinSigma, CosSigma, 
					     SinTau, CosTau);
	}
    }

    N[0] = CosPhi * SinTheta;
    N[1] = SinPhi * SinTheta;
    N[2] = CosTheta;
	
    A[0] = CosSigma * SinTau;
    A[1] = SinSigma * SinTau;
    A[2] = CosTau;
    
    ASigma[0] = -A[1];
    ASigma[1] = A[0];
    ASigma[2] = 0.0;
    ATau[0] = CosSigma * CosTau;
    ATau[1] = SinSigma * CosTau;
    ATau[2] = -SinTau;

    IRIT_CROSS_PROD(NACrossProd, N, A);
    NACrossProdLen = IRIT_VEC_LENGTH(NACrossProd);

    P[0] = CurPoint[0] - ModelParams[6];
    P[1] = CurPoint[1] - ModelParams[7];
    P[2] = CurPoint[2] - ModelParams[8];

    PHat[0] = P[0] - ModelParams[0] * N[0];
    PHat[1] = P[1] - ModelParams[0] * N[1];
    PHat[2] = P[2] - ModelParams[0] * N[2];
    
    NPhi[0] = -N[1];
    NPhi[1] = N[0];
    NPhi[2] = 0.0;
    NTheta[0] = CosTheta * CosPhi;
    NTheta[1] = SinPhi * CosTheta;
    NTheta[2] = -SinTheta;

    InvS = 1.0 / S;
    SubExpSign = IRIT_SIGN((IRIT_SQR(K) * InvS) - K);
    IRIT_VEC_SCALE2(ScaledN, N, InvS);
    IRIT_PT_SUB(TransPHat, PHat, ScaledN);
    IRIT_CROSS_PROD(TransPHatACrossProd, TransPHat, A);
    IRIT_CROSS_PROD(TransPHatATauCrossProd, TransPHat, ATau);
    IRIT_CROSS_PROD(TransPHatASigmaCrossProd, TransPHat, ASigma);
    TransPHatACrossProdLen = IRIT_VEC_LENGTH(TransPHatACrossProd);
    PNDotProd = IRIT_DOT_PROD(P, N);
    IRIT_VEC_COPY(U, TransPHatACrossProd);
    IRIT_VEC_COPY(V, NACrossProd);
    IRIT_VEC_SAFE_NORMALIZE(V);
    IRIT_VEC_SAFE_NORMALIZE(U);
    KDSM1 = K / S - 1.0;
    VUDotProd = IRIT_DOT_PROD(V, U);
    IRIT_CROSS_PROD(NPhiACrossProd, NPhi, A); 
    IRIT_CROSS_PROD(NThetaACrossProd, NTheta, A);
    IRIT_CROSS_PROD(NATauCrossProd, N, ATau);
    IRIT_CROSS_PROD(NASigmaCrossProd, N, ASigma);

    IRIT_ZAP_MEM(D0DParams, sizeof(IrtRType) * NUM_OF_TORUS_INTERNAL_PARAMS);
    IRIT_ZAP_MEM(DelEpsDParams,
		 sizeof(IrtRType) * NUM_OF_TORUS_INTERNAL_PARAMS);
    D0DParams[0] = K * (ModelParams[0] - PNDotProd) + 1;
    D0DParams[1] = (-K * ModelParams[0] - 1) * IRIT_DOT_PROD(P, NPhi);
    D0DParams[2] = (-K * ModelParams[0] - 1) * IRIT_DOT_PROD(P, NTheta);
    D0DParams[3] = 0.5 * (IRIT_PT_SQR_LENGTH(CurPoint) - 
			 2.0 * ModelParams[0] * IRIT_DOT_PROD(P, N) + 
			 IRIT_SQR(ModelParams[0]));
    DelEpsDParams[0] = -IRIT_SQR(NACrossProdLen) * KDSM1 *
	(Eps * SubExpSign * VUDotProd + 1.0);
    DelEpsDParams[1] = -(ModelParams[0] + 1.0 / S) * KDSM1 *
	(Eps * SubExpSign * TransPHatACrossProdLen + NACrossProdLen) * 
        (Eps * SubExpSign * IRIT_DOT_PROD(NPhiACrossProd, V) + 
	 IRIT_DOT_PROD(NPhiACrossProd, U));
    DelEpsDParams[2] = -(ModelParams[0] + 1.0 / S) * KDSM1 *
	(Eps * SubExpSign * TransPHatACrossProdLen + NACrossProdLen) * 
	(Eps * SubExpSign * IRIT_DOT_PROD(NThetaACrossProd, V) + 
	 IRIT_DOT_PROD(NThetaACrossProd, U));
    DelEpsDParams[3] = TransPHatACrossProdLen * NACrossProdLen *
	(Eps * SubExpSign + VUDotProd) / S;
    DelEpsDParams[4] = NACrossProdLen * 
	(Eps * SubExpSign + VUDotProd) *
	(Eps * IRIT_SIGN(K) * IRIT_FABS(KDSM1) * NACrossProdLen - 
	 K * TransPHatACrossProdLen) / IRIT_SQR(S);
    DelEpsDParams[5] = KDSM1 *
	(Eps * SubExpSign * IRIT_DOT_PROD(TransPHatASigmaCrossProd, V) * 
        NACrossProdLen  + 
        IRIT_DOT_PROD(NASigmaCrossProd, U) * TransPHatACrossProdLen +
        IRIT_DOT_PROD(TransPHatASigmaCrossProd, U) * NACrossProdLen +
        IRIT_DOT_PROD(NASigmaCrossProd, V) * TransPHatACrossProdLen);
    DelEpsDParams[6] = KDSM1 *
	(Eps * SubExpSign * IRIT_DOT_PROD(TransPHatATauCrossProd, V) * 
        NACrossProdLen  + 
        IRIT_DOT_PROD(NATauCrossProd, U) * TransPHatACrossProdLen +
        IRIT_DOT_PROD(TransPHatATauCrossProd, U) * NACrossProdLen +
        IRIT_DOT_PROD(NATauCrossProd, V) * TransPHatACrossProdLen);
    for (i = 0; i < NUM_OF_TORUS_INTERNAL_PARAMS; ++i) 
        YdParams[i] = D0DParams[i] - DelEpsDParams[i];
}
