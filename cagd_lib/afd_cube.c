/******************************************************************************
* AFD_Cube.c - Cubic Adaptive forward Differencing code.		      *
*									      *
* This file's code is based on the following cubic polynomial basis function  *
*									      *
*		      C2  2    C3  3					      *
* C(t) = C0 + C1 t + --- t  + --- t	,   Ci are the coefficients.	      *
*		      2	       3					      *
*									      *
* For more, see:							      *
*									      *
* 1. S. Chang, M. Shantz and R.Rocchetti. Rendering Cubic Curves and	      *
*    Surfaces with Integer Adaptive Forward Differencing. Computer Graphics,  *
*    Vol. 23, Num. 3, pp. 157-166, Siggraph Jul. 1989.			      *
* 2. M. Shantz and S. L. Lien. Shading Bicubic Patches. Computer Graphics,    *
*    Vol. 21, Num. 4, pp. 189-196, Siggraph Jul. 1987.			      *
* 3. M. Shantz and S. Chang. Rendering Trimmed NURBS with Adaptive Forward    *
*    Differencing. Computer Graphics, Vol. 22, Num. 4, pp. 189-198,	      *
*    Siggraph Aug. 1988.						      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jan. 93.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic Bezier curve, computes the four	     M
* coefficients of the cubic afd basis functions, in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:    Converts, in place, cubic Bezier Coef to AFD Coef.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyLStep, AfdApplyEStep, AfdApplyLn,	     M
*   AfdComputePolyline, AfdBzrCrvEvalToPolyline				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdCnvrtCubicBzrToAfd, forward differencing                              M
*****************************************************************************/
void AfdCnvrtCubicBzrToAfd(CagdRType Coef[4])
{
    CagdRType AfdCoef[4];

    AfdCoef[0] = Coef[0];
    AfdCoef[1] = Coef[3] - Coef[0];
    AfdCoef[2] = 6.0 * Coef[3] - 12.0 * Coef[2] + 6.0 * Coef[1];
    AfdCoef[3] = 6.0 * Coef[3] - 18.0 * Coef[2] + 18.0 * Coef[1] - 6.0 * Coef[0];
    CAGD_GEN_COPY(Coef, AfdCoef, sizeof(CagdRType) * 4);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic afd polynomial, apply the L ( half the M
* step size ) n times to them, in place.				     M
*   We basically precomputed L^n and apply it here once. Every instance of L M
* half the domain and so L^n divides the domain by 2^n.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:       Four coefficients of the AFD basis functions.                M
*   n:          How many times to compute the L transform.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyLStep, AfdApplyEStep,			     M
*   AfdCnvrtCubicBzrToAfd, AfdComputePolyline, AfdBzrCrvEvalToPolyline       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdApplyLn, forward differencing                                         M
*****************************************************************************/
void AfdApplyLn(CagdRType Coef[4], int n)
{
    CagdRType AfdCoef[4];

    switch (n) {
	case 1:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 2.0 - Coef[2] / 8.0 + Coef[3] / 16.0;
	    AfdCoef[2] = Coef[2] / 4.0 - Coef[3] / 8.0;
	    AfdCoef[3] = Coef[3];
	    break;
	case 2:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 4.0 - Coef[2] * 3.0 / 32.0 +
						Coef[3] * 7.0 / 128.0;
	    AfdCoef[2] = Coef[2] / 16.0 - Coef[3] * 3.0 / 64.0;
	    AfdCoef[3] = Coef[3] / 64.0;
	    break;
	case 3:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 8.0 - Coef[2] * 7.0 / 128.0 +
						Coef[3] * 35.0 / 1024.0;
	    AfdCoef[2] = Coef[2] / 64 - Coef[3] * 7.0 / 512.0;
	    AfdCoef[3] = Coef[3] / 512.0;
	    break;
	case 4:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 16.0 - Coef[2] * 15.0 / 512.0 +
						Coef[3] * 155.0 / 8192.0;
	    AfdCoef[2] = Coef[2] / 256 - Coef[3] * 15.0 / 4096.0;
	    AfdCoef[3] = Coef[3] / 4096.0;
	    break;
	case 5:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 32.0 - Coef[2] * 31.0 / 2048.0 +
						Coef[3] * 651.0 / 65536.0;
	    AfdCoef[2] = Coef[2] / 1024 - Coef[3] * 31.0 / 32768.0;
	    AfdCoef[3] = Coef[3] / 262144.0;
	    break;
	case 6:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 64.0 - Coef[2] * 63.0 / 8192.0 +
						Coef[3] * 2667.0 / 524288.0;
	    AfdCoef[2] = Coef[2] / 4096.0 - Coef[3] * 63.0 / 262144.0;
	    AfdCoef[3] = Coef[3] / 262144.0;
	    break;
	case 7:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 128.0 - Coef[2] * 127.0 / 32768.0 +
						Coef[3] * 10795.0 / 4194304.0;
	    AfdCoef[2] = Coef[2] / 16384.0 - Coef[3] * 127.0 / 2097152.0;
	    AfdCoef[3] = Coef[3] / 2097152.0;
	    break;
	case 8:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 256.0 - Coef[2] * 255.0 / 131072.0 +
						Coef[3] * 43435.0 / 33554432.0;
	    AfdCoef[2] = Coef[2] / 65536.0 - Coef[3] * 255.0 / 16777216.0;
	    AfdCoef[3] = Coef[3] / 16777216.0;
	    break;
	case 9:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 512.0 - Coef[2] * 511.0 / 524288.0 +
					Coef[3] * 174251.0 / 268435456.0;
	    AfdCoef[2] = Coef[2] / 262144.0 - Coef[3] * 511.0 / 134217728.0;
	    AfdCoef[3] = Coef[3] / 134217728.0;
	    break;
	case 10:
	    AfdCoef[0] = Coef[0];
	    AfdCoef[1] = Coef[1] / 1024.0 - Coef[2] * 1023.0 / 2097152.0 +
					Coef[3] * 698027.0 / 2147483648.0;
	    AfdCoef[2] = Coef[2] / 1048576.0 - Coef[3] * 1023.0 / 1073741824.0;
	    AfdCoef[3] = Coef[3] / 1073741824.0;
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_OUT_OF_RANGE);
	    return;
    }
    CAGD_GEN_COPY(Coef, AfdCoef, sizeof(CagdRType) * 4);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic afd polynomial, apply the E ( step     M
* 1 ) in place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:       Four coefficients of the AFD basis functions.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyLStep, AfdApplyLn,			     M
*   AfdCnvrtCubicBzrToAfd, AfdComputePolyline, AfdBzrCrvEvalToPolyline       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdApplyEStep, forward differencing                                      M
*****************************************************************************/
void AfdApplyEStep(CagdRType Coef[4])
{
    Coef[0] += Coef[1];
    Coef[1] += Coef[2];
    Coef[2] += Coef[3];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic afd polynomial, apply the L step,      M
* in place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:       Four coefficients of the AFD basis functions.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyEStep, AfdApplyLn,			     M
*   AfdCnvrtCubicBzrToAfd, AfdComputePolyline, AfdBzrCrvEvalToPolyline       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdApplyLStep, forward differencing                                      M
*****************************************************************************/
void AfdApplyLStep(CagdRType Coef[4])
{
    Coef[1] = Coef[1] * 0.5  - Coef[2] * 0.125 + Coef[3] * 0.0625;
    Coef[2] = Coef[2] * 0.25 - Coef[3] * 0.125;
    Coef[3] = Coef[3] * 0.125;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic afd polynomial, apply the anti L step, M
* in place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:       Four coefficients of the AFD basis functions.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AfdApplyLStep, AfdApplyEStep, AfdApplyLn,				     M
*   AfdCnvrtCubicBzrToAfd, AfdComputePolyline, AfdBzrCrvEvalToPolyline       M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdApplyAntiLStep, forward differencing                                  M
*****************************************************************************/
void AfdApplyAntiLStep(CagdRType Coef[4])
{
    Coef[1] = Coef[1] * 2.0 + Coef[2];
    Coef[2] = Coef[2] * 4.0 + Coef[3] * 4.0;
    Coef[3] = Coef[3] * 8.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four coefficents of a cubic Bezier curve, computes the four	     M
* coefficients of the cubic afd basis functions and step along them to       M
* create a piecewise polynomial approximating the curve.		     M
*   If NonAdaptive is TRUE then 2^Log2Step constant steps are taken,         M
* creating 2^Log2Step + 1 points along the curve.			     M
*   Otherwise the full blown adaptive algorithm is used.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coef:          Four coefficients of a cubic Bezier curve.                M
*   Poly:          Where to put the polyline computed.                       M
*   Log2Step:      How many steps to take (2 to the power of this).          M
*   NonAdaptive:   if TRUE, ignore the adaptive option.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyLStep, AfdApplyEStep, AfdApplyLn,	     M
*   AfdCnvrtCubicBzrToAfd, AfdBzrCrvEvalToPolyline			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdComputePolyline, forward differencing                                 M
*****************************************************************************/
void AfdComputePolyline(CagdRType Coef[4],
			CagdRType *Poly,
			int Log2Step,
			CagdBType NonAdaptive)
{
    int i;
    int n = 1 << Log2Step;

    AfdCnvrtCubicBzrToAfd(Coef);
    AfdApplyLn(Coef, Log2Step);

    if (NonAdaptive) {
	for (i = 0; i <= n; i++) {
	    Poly[i] = Coef[0];
	    AfdApplyEStep(Coef);
	}
    }
    else
    {
	CAGD_FATAL_ERROR(CAGD_ERR_AFD_NO_SUPPORT);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Samples the curves at FineNess location equally spaced in the Bezier     M
* parametric domain [0..1]. If Cache is enabled, and FineNess is power of    M
* two, upto or equal to CacheFineNess, the cache is used, otherwise the      M
* points are evaluated manually for each of the samples.		     M
*   Data is saved at the Points array of vectors (according to Curve PType), M
* each vector is assumed to be allocated for FineNess CagdRType points.	     M
* Bezier curve must be cubic.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         A cubic Bezier curve to piecewise linear sample using AFD.  M
*   FineNess:    Of samples.                                                 M
*   Points:      Where to place the piecewise linear approximation.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   AfdApplyAntiLStep, AfdApplyLStep, AfdApplyEStep, AfdApplyLn,	     M
*   AfdCnvrtCubicBzrToAfd, AfdComputePolyline			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   AfdBzrCrvEvalToPolyline, forward differencing                            M
*****************************************************************************/
void AfdBzrCrvEvalToPolyline(const CagdCrvStruct *Crv,
			     int FineNess,
			     CagdRType *Points[])
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType
	* const *CtlPoints = Crv -> Points;

    if (Crv -> Order != 4)
	CAGD_FATAL_ERROR(CAGD_ERR_CUBIC_EXPECTED);

    for (i = IsNotRational; i <= MaxCoord; i++) {
	CagdRType Coef[4];

	for (j = 0; j < 4; j++)
	    Coef[j] = CtlPoints[i][j];

	AfdComputePolyline(Coef, Points[j], FineNess, TRUE);
    }
}

#ifdef DEBUG_AFD

/* If this section is defined, this file can be compiled stand alone. */

void CagdFatalError(CagdFatalErrorType ErrID)
{
}

void main(int argc, char **argv)
{
    CagdRType
	Poly[1025],
	Coef[4] = { 0.0, 1.0, 2.0, 3.0 };
    int i,
	Log2Step = 6;

    while (argc >= 2) {
	if (argc >= 2 && strcmp( argv[1], "-s" ) == 0) {
	    Log2Step = atoi( argv[2] );
	    argc -= 2;
	    argv += 2;
	}
	else if (argc >= 2 && strcmp( argv[1], "-c" ) == 0) {
	    Coef[0] = atof(argv[2]);
	    Coef[1] = atof(argv[3]);
	    Coef[2] = atof(argv[4]);
	    Coef[3] = atof(argv[5]);
	    argc -= 5;
	    argv += 5;
	}
	else {
	    IRIT_WARNING_MSG("Wrong command line");
	    exit(1);
	}
    }

    IRIT_INFO_MSG_PRINTF("Steps = %d, Coef = %lf %lf %lf %lf\n",
	   Log2Step, Coef[0], Coef[1], Coef[2], Coef[3]);

    AfdComputePolyline(Coef, Poly, Log2Step, TRUE);
    for (i = 0; i <= (1 << Log2Step); i++)
	IRIT_INFO_MSG_PRINTF("%d = %lg\n", i, (double) Poly[i]);
}

#endif /* DEBUG_AFD */
