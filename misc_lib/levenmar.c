/******************************************************************************
* LevenMar.c - Levenberg-marquardt and Gauss-Jordan elimination methods.      *
* This imlemetation is based on the algorithm described in "Numerical         *
* recipies" Cambridge University Press, 1986, Section 2.1 and Section 14.4.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Ronen Lev, November 2002                                         *
******************************************************************************/

#include "irit_sm.h"
#include "misc_loc.h"

#define IRIT_GAUSS_JORDAN_SWAP_INC(type, x, y) \
			{ type temp = (x); x++ = (y); y++ = temp; }

static unsigned int
    MaxLevenbergIterations = 1000;
static IrtRType
    MaxLevenbergLambda = IRIT_INFNTY;

static void CalcAlphaBetaSig1Aux(IrtRType **X,
				 IrtRType Y[],
				 unsigned NumberOfDataElements,
				 IrtRType ModelParams[],
				 IritLevenEvalFuncType *ShapeFunc,
				 IrtRType *Alpha,
				 IrtRType Beta[],
				 unsigned NumberOfModelParams,
				 IrtRType *ChiSqr,
				 IrtRType *AuxMem);
static void CalcAlphaBetaAux(IrtRType **X,
			     IrtRType Y[],
			     IrtRType Sigma[],
			     unsigned int NumberOfDataElements,
			     IrtRType ModelParams[],
			     IritLevenEvalFuncType *ShapeFunc,
			     IrtRType Alpha[],
			     IrtRType Beta[],
			     unsigned int NumberOfModelParams,
			     IrtRType *ChiSqr,
			     IrtRType *AuxMem);
static int LevenMinProcessAux(IrtRType **X,
			      IrtRType Y[],
			      IrtRType Sigma[],
			      unsigned NumberOfDataElements,
			      IrtRType ModelParams[],
			      IritLevenEvalFuncType *ShapeFunc,
			      IritLevenIsModelValidFuncType 
			          *ModelValidatorFunc,
			      IrtRType Alpha[],
			      IrtRType Beta[],
			      unsigned NumberOfModelParams,
			      IrtRType Lambda,
			      IrtRType *CurChiSqr,
			      IrtRType *AuxMem);
static int LevenMinProcessSig1Aux(IrtRType **X,
				  IrtRType Y[],
				  unsigned NumberOfDataElements,
				  IrtRType ModelParams[],
				  IritLevenEvalFuncType *ShapeFunc,
				  IritLevenIsModelValidFuncType 
				      *ModelValidatorFunc,
				  IrtRType Alpha[],
				  IrtRType Beta[],
				  unsigned NumberOfModelParams,
				  IrtRType Lambda,
				  IrtRType *CurChiSqr,
				  IrtRType *AuxMem);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function modifies the maximum number of iteration when calculating  M
* Levenberg-Marquardt.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   NewVal: The new maximum number of iterations.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   unsigned: The old maximum number of iterations.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritLevenMarMinSig1, IritLevenMarMin                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritLevenMarSetMaxIterations                                             M
*****************************************************************************/
unsigned IritLevenMarSetMaxIterations(unsigned NewVal)
{
    unsigned 
	  OldVal = MaxLevenbergIterations;

    MaxLevenbergIterations = NewVal;
    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the Alpha, Beta and ChiSqr of the data.         *
* This function is same as CalcAlphaBetaAux except it assumes the            *
* sigma is 1 for all data points.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   X:                    Pointer to a list of data.                         *
*   Y:                    Pointer to the list of expected results.           *
*   NumberOfDataElements: The number of data elements in X & Y.              *
*   ModelParams:          The model params to be checked.                    *
*   ShapeFunc:            The shape function.                                *
*   Alpha:                The result Alpha matrix.                           *
*   Beta:                 The result Beta Vector.                            *
*   NumberOfModelParams:  The number of model params.                        *
*   ChiSqr:               The sum of the squared distance.                   *
*   AuxMem:               An auxiliry allocated vercor of IrtRType of size   *
*                         NumberOfModelParams.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CalcAlphaBetaSig1Aux(IrtRType **X,
				 IrtRType Y[],
				 unsigned NumberOfDataElements,
				 IrtRType ModelParams[],
				 IritLevenEvalFuncType *ShapeFunc,
				 IrtRType *Alpha,
				 IrtRType Beta[],
				 unsigned NumberOfModelParams,
				 IrtRType *ChiSqr,
				 IrtRType *AuxMem)
{
    unsigned int i, j, DataNum;
    IrtRType 
        *DyDParam = AuxMem;
    
    /* Initilize Alpha and Beta. */
    IRIT_ZAP_MEM(Beta, NumberOfModelParams * sizeof(IrtRType));
    IRIT_ZAP_MEM(Alpha,
	    NumberOfModelParams * NumberOfModelParams * sizeof(IrtRType));

    *ChiSqr = 0;
    for (DataNum = 0; DataNum < NumberOfDataElements; ++DataNum) {
	unsigned int CurModelParam;
	IrtRType ResultingY, Dy;
	
	ShapeFunc(X[DataNum], ModelParams, &ResultingY, DyDParam);
	Dy = Y[DataNum] - ResultingY;
	for (CurModelParam = 0;
	     CurModelParam < NumberOfModelParams;
	     ++CurModelParam) {
	    for (i = 0; i <= CurModelParam; ++i)
	        Alpha[CurModelParam * NumberOfModelParams + i]
		   += DyDParam[CurModelParam] * DyDParam[i];
	    Beta[CurModelParam] += DyDParam[CurModelParam] * Dy;
	}
	*ChiSqr += IRIT_SQR(Dy);
    }
    
    /* Fill the symmetric portion. */
    for (i = 1; i < NumberOfModelParams; ++i)
        for (j = 0; j < i; ++j)
	    Alpha[j * NumberOfModelParams + i] = 
	        Alpha[i * NumberOfModelParams + j];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function calculates the Alpha, Beta and ChiSqr of the data.         *
*                                                                            *
* PARAMETERS:                                                                *
*   X:                   Pointer to a list of data.                          *
*   Y:                   Pointer to the list of expected results.            *
*   Sigma:               Pointer to the expected variance vector.            *
*   NumberOfDataElements: The number of data elements in X, Y & Sigma.       *
*   ModelParams:         The model params to be checked.                     *
*   ShapeFunc:           The shape function.                                 *
*   Alpha:               The result Alpha matrix.                            *
*   Beta:                The result Beta Vector.                             *
*   NumberOfModelParams: The number of model params.                         *
*   ChiSqr:              The sum of the squared distance.                    *
*   AuxMem:              An auxiliry allocated vercor of IrtRType of size    *
*                        NumberOfModelParams.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CalcAlphaBetaAux(IrtRType **X,
			     IrtRType Y[],
			     IrtRType Sigma[],
			     unsigned int NumberOfDataElements,
			     IrtRType ModelParams[],
			     IritLevenEvalFuncType *ShapeFunc,
			     IrtRType Alpha[],
			     IrtRType Beta[],
			     unsigned int NumberOfModelParams,
			     IrtRType *ChiSqr,
			     IrtRType *AuxMem) 
{
    unsigned int i, j, DataNum;
    IrtRType 
        *DyDParam = AuxMem;
    
    /* Initilize Alpha and Beta. */
    IRIT_ZAP_MEM(Beta, NumberOfModelParams * sizeof(IrtRType));
    IRIT_ZAP_MEM(Alpha,
	    NumberOfModelParams * NumberOfModelParams * sizeof(IrtRType));

    *ChiSqr = 0;
    for (DataNum = 0; DataNum < NumberOfDataElements; ++DataNum) {
	unsigned int CurModelParam;
	IrtRType ResultingY, Dy, DSig2;
	
	ShapeFunc(X[DataNum], ModelParams, &ResultingY, DyDParam);
	Dy = Y[DataNum] - ResultingY;
	DSig2 = 1.0 / IRIT_SQR(Sigma[DataNum]);
	for (CurModelParam = 0;
	     CurModelParam < NumberOfModelParams;
	     ++CurModelParam) {
	    IrtRType 
	        WeightedDiff = DyDParam[CurModelParam] / DSig2;
	    
	    for (i = 0; i <= CurModelParam; ++i)
	        Alpha[CurModelParam * NumberOfModelParams + i] +=
		    WeightedDiff * DyDParam[i];
	    Beta[CurModelParam] += WeightedDiff * Dy;
	}
	*ChiSqr += (IrtRType) (IRIT_SQR(Dy) * DSig2);
    }
    
    /* Fill the symmetric portion. */
    for (i = 1; i < NumberOfModelParams; ++i)
        for (j = 0; j < i; ++j)
	    Alpha[j * NumberOfModelParams + i] = 
	        Alpha[i * NumberOfModelParams + j];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attempts to improve the current Model Params.              *
* If the attempt is successful it also updates Alpha and Beta.               *
*                                                                            *
* PARAMETERS:                                                                *
*   X:                    Pointer to the list of the data.                   *
*   Y:                    Pointer to the list of expected results.           *
*   Sigma:                Pointer to the expected variance vector.           *
*   NumberOfDataElements: The Number of data elements in X, Y & Sigma.       *
*   ModelParams:       Pointer to the current model params (may be updated). *
*   ShapeFunc:            The shape function.                                *
*   ModelValidatorFunc:   The model validator.                               *
*   Alpha:                The result alpha matrix (may be updated).          *
*   Beta:                 The results Beta vector (may be updated).          *
*   NumberOfModelParams:  The number of model params.                        *
*   Lambda:               Current Lambda.                                    *
*   CurChiSqr:            Current ChiSqr (may be updated).                   *
*   AuxMem:               An auxiliry allocated vector of IrtRType of size   *
*                         NumberOfModelParams * (NumberOfModelParams + 3).   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE if a better model has been found, FALSE otherwise.             *
*****************************************************************************/
static int LevenMinProcessAux(IrtRType **X,
			       IrtRType Y[],
			       IrtRType Sigma[],
			       unsigned NumberOfDataElements,
			       IrtRType ModelParams[],
			       IritLevenEvalFuncType *ShapeFunc,
			       IritLevenIsModelValidFuncType 
						          *ModelValidatorFunc,
			       IrtRType Alpha[],
			       IrtRType Beta[],
			       unsigned NumberOfModelParams,
			       IrtRType Lambda,
			       IrtRType *CurChiSqr,
			       IrtRType *AuxMem)
{
    unsigned int i,
        Result = FALSE;
    IrtRType Multiplier, NewChiSqr, *NewAlpha, *NewBeta, *NewModelParams;
    
    Multiplier = 1.0f + Lambda;
    NewAlpha = AuxMem;
    IRIT_GEN_COPY(NewAlpha, Alpha,
	     sizeof(IrtRType) * NumberOfModelParams * NumberOfModelParams);
    NewBeta = &AuxMem[NumberOfModelParams * NumberOfModelParams];
    IRIT_GEN_COPY(NewBeta, Beta, sizeof(IrtRType) * NumberOfModelParams);
    NewModelParams = &AuxMem[NumberOfModelParams * NumberOfModelParams
			                            + NumberOfModelParams];
    
    for (i = 0; i < NumberOfModelParams; ++i)
        NewAlpha[i + NumberOfModelParams * i] *= Multiplier;
    IritGaussJordan(NewAlpha, NewBeta, NumberOfModelParams, 1);
    for (i = 0; i < NumberOfModelParams; ++i)
        NewModelParams[i] = ModelParams[i] + NewBeta[i];
    
    CalcAlphaBetaAux(X, Y, Sigma, NumberOfDataElements, NewModelParams,
		     ShapeFunc, NewAlpha, NewBeta, NumberOfModelParams,
		     &NewChiSqr,
		     &AuxMem[NumberOfModelParams * NumberOfModelParams
			                   + 2 * NumberOfModelParams]);

    if ((NewChiSqr < *CurChiSqr) && 
	((ModelValidatorFunc == NULL) || 
	 ((ModelValidatorFunc != NULL) && 
	  (ModelValidatorFunc(NewModelParams) == TRUE)))) {
	IRIT_GEN_COPY(Alpha, NewAlpha, 
		 sizeof(IrtRType) * NumberOfModelParams * NumberOfModelParams);
	IRIT_GEN_COPY(Beta, NewBeta, sizeof(IrtRType) * NumberOfModelParams);
	IRIT_GEN_COPY(ModelParams, NewModelParams,
		 sizeof(IrtRType) * NumberOfModelParams);
	*CurChiSqr = NewChiSqr;
	Result = TRUE;
    }

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function attempts to improve the current Model Params.              *
* If the attempt is successful it also updates Alpha and Beta.               *
* This function is similar to LevenMinProcessAux except that sigma=1.       *
*                                                                            *
* PARAMETERS:                                                                *
*   X:                    Pointer to the list of the data.                   *
*   Y:                    Pointer to the list of expected results.           *
*   NumberOfDataElements: The Number of data elements in X, Y & Sigma.       *
*   ModelParams:       Pointer to the current model params (may be updated). *
*   ShapeFunc:            The shape function.                                *
*   ModelValidatorFunc:   The model validator.                               *
*   Alpha:                The result alpha matrix (may be updated).          *
*   Beta:                 The results Beta vector (may be updated).          *
*   NumberOfModelParams:  The number of model params.                        *
*   Lambda:               Current Lambda.                                    *
*   CurChiSqr:            Current ChiSqr (may be updated).                   *
*   AuxMem:               An auxiliry allocated vector of IrtRType of size.  *
*                         NumberOfModelParams * (NumberOfModelParams + 3).   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE if a better model has been found, FALSE otherwise.             *
*****************************************************************************/
static int LevenMinProcessSig1Aux(IrtRType **X,
				  IrtRType Y[],
				  unsigned NumberOfDataElements,
				  IrtRType ModelParams[],
				  IritLevenEvalFuncType *ShapeFunc,
				  IritLevenIsModelValidFuncType 
				      *ModelValidatorFunc,
				  IrtRType Alpha[],
				  IrtRType Beta[],
				  unsigned NumberOfModelParams,
				  IrtRType Lambda,
				  IrtRType *CurChiSqr,
				  IrtRType *AuxMem)
{
    unsigned int i;
    int Result = FALSE;
    IrtRType Multiplier, NewChiSqr, *NewAlpha, *NewBeta, *NewModelParams;
    
    Multiplier = 1.0f + Lambda;
    NewAlpha = AuxMem;
    IRIT_GEN_COPY(NewAlpha, Alpha, 
	     sizeof(IrtRType) * NumberOfModelParams * NumberOfModelParams);
    NewBeta = &AuxMem[NumberOfModelParams * NumberOfModelParams];
    IRIT_GEN_COPY(NewBeta, Beta, sizeof(IrtRType) * NumberOfModelParams);
    NewModelParams = &AuxMem[NumberOfModelParams * NumberOfModelParams
			                              + NumberOfModelParams];

    for (i = 0; i < NumberOfModelParams; ++i)
        NewAlpha[i + NumberOfModelParams * i] *= Multiplier;
    IritGaussJordan(NewAlpha, NewBeta, NumberOfModelParams, 1);
    for (i = 0; i < NumberOfModelParams; ++i)
        NewModelParams[i] = ModelParams[i] + NewBeta[i];

    CalcAlphaBetaSig1Aux(X, Y, NumberOfDataElements, NewModelParams,
			 ShapeFunc, NewAlpha, NewBeta, NumberOfModelParams,
			 &NewChiSqr,
			 &AuxMem[NumberOfModelParams * NumberOfModelParams
				 + 2 * NumberOfModelParams]);

    if ((NewChiSqr < *CurChiSqr) && 
	((ModelValidatorFunc == NULL) || 
	 ((ModelValidatorFunc != NULL) && 
	  (ModelValidatorFunc(NewModelParams) == TRUE)))) {
	IRIT_GEN_COPY(Alpha, NewAlpha, 
		 sizeof(IrtRType) * NumberOfModelParams * NumberOfModelParams);
	IRIT_GEN_COPY(Beta, NewBeta, sizeof(IrtRType) * NumberOfModelParams);
	IRIT_GEN_COPY(ModelParams, NewModelParams,
		 sizeof(IrtRType) * NumberOfModelParams);
	*CurChiSqr = NewChiSqr;
	Result = TRUE;
    }

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function calculates the levenberg-marquardt minimization of the     M
* Specified function.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   X:                   Pointer to a list of data.                          M
*   Y:                   Pointer to the list of expected results.            M
*   Sigma:               Pointer to the expected variance vector.            M
*   NumberOfDataElements: The number of data elements in X, Y & Sigma.       M
*   ModelParams:         The model params to be checked.                     M
*   ShapeFunc:           The shape function.                                 M
*   ProtectionFunc:      The numerical protection function.                  M
*   ModelValidatorFunc:  The model validator.                                M
*   NumberOfModelParams: The number of model params.                         M
*   Tolerance:           If the error is smaller then Tolerance return.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The squared sum of the error.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritLevenMarMinSig1, IritLevenMarSetMaxIterations                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritLevenMarMin                                                          M
*****************************************************************************/
IrtRType IritLevenMarMin(IrtRType **X,
			 IrtRType Y[],
			 IrtRType Sigma[],
			 unsigned NumberOfDataElements,
			 IrtRType ModelParams[],
			 IritLevenEvalFuncType *ShapeFunc,
			 IritLevenNumerProtectionFuncType *ProtectionFunc,
			 IritLevenIsModelValidFuncType *ModelValidatorFunc,
			 unsigned NumberOfModelParams,
			 IrtRType Tolerance)
{
    unsigned int 
        IterNum = 0;
    IrtRType *Alpha, *Beta, *AuxMem, Lambda, CurChiSqr;

    Alpha = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams
				                       * NumberOfModelParams);
    Beta = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams);
    AuxMem = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams
				                 * (3 + NumberOfModelParams));
    
    if ((Alpha == NULL) || (Beta == NULL) || (AuxMem == NULL)) {
	IRIT_FATAL_ERROR("Unable to allocate memory.");
    }
    CalcAlphaBetaAux(X, Y, Sigma, NumberOfDataElements, ModelParams,
		     ShapeFunc, Alpha, Beta, NumberOfModelParams, &CurChiSqr,
		     AuxMem);
    Lambda = 0.001;

    while ((IterNum < MaxLevenbergIterations) && 
	   (CurChiSqr > Tolerance) && 
	   (Lambda < MaxLevenbergLambda)) {
        int TryResult = LevenMinProcessAux(X, Y, Sigma, NumberOfDataElements,
					   ModelParams, ShapeFunc,
					   ModelValidatorFunc, Alpha,
					   Beta, NumberOfModelParams, Lambda,
					   &CurChiSqr, AuxMem);
	if (TryResult)
	    Lambda *= 0.1;
	else
	    Lambda *= 10.0;
	if (ProtectionFunc != NULL)
	    ProtectionFunc(ModelParams);
    }

    IritFree(Alpha);
    IritFree(Beta);
    IritFree(AuxMem);

    return CurChiSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function calculates the levenberg-marquardt minimization of the     M
* Specified function.                                                        M
*   This function is similar to LevenMin except that sigma is allways 1.     M
*                                                                            *
* PARAMETERS:                                                                M
*   X:                   Pointer to a list of data.                          M
*   Y:                   Pointer to the list of expected results.            M
*   NumberOfDataElements: The number of data elements in X, Y & Sigma.       M
*   ModelParams:         The model params to be checked.                     M
*   ShapeFunc:           The shape function.                                 M
*   ProtectionFunc:      The numerical protection function.                  M
*   ModelValidatorFunc:  The model validator.                                M
*   NumberOfModelParams: The number of model params.                         M
*   Tolerance:           If the error is smaller then Tolerance return.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The squared sum of the error.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritLevenMarMin, IritLevenMarSetMaxIterations                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritLevenMarMinSig1                                                      M
*****************************************************************************/
IrtRType IritLevenMarMinSig1(IrtRType **X,
			     IrtRType Y[],
			     unsigned NumberOfDataElements,
			     IrtRType ModelParams[],
			     IritLevenEvalFuncType *ShapeFunc,
			     IritLevenNumerProtectionFuncType *ProtectionFunc,
			     IritLevenIsModelValidFuncType *ModelValidatorFunc,
			     unsigned NumberOfModelParams,
			     IrtRType Tolerance)
{
    unsigned int 
        IterNum = 0;
    IrtRType *Alpha, *Beta, *AuxMem, Lambda, CurChiSqr;

    Alpha = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams
				                       * NumberOfModelParams);
    Beta = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams);
    AuxMem = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumberOfModelParams
				                * ( 3 + NumberOfModelParams));
    
    if ((Alpha == NULL) || (Beta == NULL) || (AuxMem == NULL)) {
	IRIT_FATAL_ERROR("Unable to allocate memory.");
    }
    CalcAlphaBetaSig1Aux(X, Y, NumberOfDataElements, ModelParams, ShapeFunc,
			 Alpha, Beta, NumberOfModelParams, &CurChiSqr, AuxMem);
    Lambda = 0.001;

    while ((IterNum < MaxLevenbergIterations) &&
	   (CurChiSqr > Tolerance) &&
	   (Lambda < MaxLevenbergLambda)){
	int TryResult = LevenMinProcessSig1Aux(X, Y, NumberOfDataElements,
					       ModelParams, ShapeFunc, 
					       ModelValidatorFunc,
					       Alpha, Beta,
					       NumberOfModelParams, Lambda,
					       &CurChiSqr, AuxMem);
	++IterNum;
	if (TryResult)
	    Lambda *= 0.1;
	else
	    Lambda *= 10.0;

	if (ProtectionFunc != NULL)
	    ProtectionFunc(ModelParams);
    }

    IritFree(Alpha);
    IritFree(Beta);
    IritFree(AuxMem);

    return CurChiSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This functions solves the linear equation Ax=B, using the Gauss-Jordan   M
* elimination algorithm.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   A: A matrix of N*N elements, is invalid on exit.                         M
*   B: A matrix of N*M elements, contains the result on exit.                M
*   N: Described above.                                                      M
*   M: Described above.                                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE on success, FALSE if singular.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritGaussJordan, Gauss-Jordan elimination                                M
*****************************************************************************/
int IritGaussJordan(IrtRType *A, IrtRType *B, unsigned N, unsigned M)
{
    unsigned int i, j, k, CurPivot, CurRow;
    unsigned char *SolvedColumnVec, *c;
    IrtRType *p, *q;

    SolvedColumnVec = (unsigned char *) IritMalloc(sizeof(unsigned char) * N);
    IRIT_ZAP_MEM(SolvedColumnVec, sizeof(unsigned char) * N);

    for (i = 0; i < N; ++i) {
        IrtRType Divisor,
	Biggest = 0.0;
	
	/* An unreachable value to allow checking if CurPivot has been set. */
	CurPivot = N; 
	CurRow = 0;      /* To prevent a warning. */
	for (j = 0; j < N; ++j) {
	    if (SolvedColumnVec[j] != 1) {
	        c = SolvedColumnVec;
		p = &A[j * N];

	        for (k = 0; k < N; ++k, p++) {
		    if (*c++ == 0 && IRIT_FABS(*p) > Biggest) {
		        CurPivot = k;
			CurRow = j;
			Biggest = IRIT_FABS(*p);
		    }
		}
	    }
	}

	if (CurPivot == N) {
	    IritFree(SolvedColumnVec);
	    return FALSE;
	}

	++SolvedColumnVec[CurPivot];
	if (CurPivot != CurRow) {
	    p = &A[CurPivot * N];
	    q = &A[CurRow * N];
	    for (j = 0; j++ < N; )
	        IRIT_GAUSS_JORDAN_SWAP_INC(IrtRType, *p, *q);

	    p = &B[CurPivot * M];
	    q = &B[CurRow * M];
	    for (j = 0; j++ < M; )
	        IRIT_GAUSS_JORDAN_SWAP_INC(IrtRType, *p, *q);
	}
	
	if (IRIT_APX_EQ(A[CurPivot + CurPivot * N], 0.0)) {
	    IritFree(SolvedColumnVec);
	    return FALSE;
	}
	
	Divisor = 1.0 / A[CurPivot + CurPivot * N];
	A[CurPivot + CurPivot * N] = 1.0;
	for (j = 0; j < N; ++j)
	    if (j != CurPivot)
	        A[j + CurPivot * N] *= Divisor;
	for (j = 0; j < M; ++j)
	    B[j + CurPivot * M] *= Divisor;

	for (j = 0; j < N; ++j)
	    if (j != CurPivot) {
	        IrtRType
		    LineMultiplier = A[CurPivot + j * N];

		p = &A[j * N];
		q = &A[CurPivot * N];
		for (k = 0; k++ < N; )
		    *p++ -= LineMultiplier * *q++;
		
		p = &B[j * M];
		q = &B[CurPivot * M];
		for (k = 0; k++ < M; )
		    *p++ -= LineMultiplier * *q++;
	    }
    }
    
    IritFree(SolvedColumnVec);

    return TRUE;  
}
