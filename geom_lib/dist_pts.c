/******************************************************************************
* Distribute N points in a one/two dimensional domain, so as to minimize      *
* their energy according to a given energy function.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*				      Written by Gershon Elber, February 1994 *
******************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Distributes N points with a given energy in the region in the X line that  M
* is bounded by XMin, XMax.						     M
*   Energy is specified via the EnergyFunc that recieves the X location.     M
*   Resolution * N specifies how many samples to take from EnergyFunc.	     M
*   Returns an array of N distributed points.				     M
*   The solution to the distribution is analythic provided EnergyFunc can be M
* integrated. Herein, this integral is computed nomerically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   N:           Number of points to distribute,                             M
*   XMin:        Minimum of domain to distribute points.                     M
*   XMax:        Minimum of domain to distribute points.                     M
*   Resolution:  Fineness of integral calculation.                           M
*   EnergyFunc:  Energy function to use.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType *:  A vector of N points distributed as requested.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDistPoint1DWithEnergy, point distribution                              M
*****************************************************************************/
IrtRType *GMDistPoint1DWithEnergy(int N,
				  IrtRType XMin,
				  IrtRType XMax,
				  int Resolution,
				  GMDistEnergy1DFuncType EnergyFunc)
{
    int i, j, 
	IntegN = N * Resolution;
    IrtRType x, *R, IntegMaxVal, IntegDelta, IntegVal, Dx, DxN, *Integ, *Dist;

    if (N < 2)
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_TWO_PTS);

    N = N < 2 ? 2 : N,
    IntegN = N * Resolution;
    Dx = XMax - XMin;
    DxN = Dx / IntegN;
    Integ = (IrtRType *) IritMalloc((IntegN + 2) * sizeof(IrtRType));
    Dist = (IrtRType *) IritMalloc(N * sizeof(IrtRType));

    /* Compute first order approximation of the integral of energy function. */
    Integ[0] = 0.0;
    for (i = 1, R = &Integ[1], x = XMin + DxN * 0.5;
	 i < IntegN + 2;
	 i++, R++, x += DxN) {
	IrtRType
	    E = EnergyFunc(x);

	*R = R[-1] + IRIT_MAX(E, IRIT_EPS);/* Do not allow negative energy...*/
    }

    if ((IntegMaxVal = Integ[IntegN]) < IRIT_EPS) {
	/* No real energy in the data. Apply uniform distribution. */
	for (i = 1, R = &Integ[1]; i < IntegN + 2; i++)
	    *R++ = i;
	IntegMaxVal = Integ[IntegN];
    }

    IntegDelta = (IntegMaxVal - IRIT_EPS) / (N - 1);
    IntegVal = 0.0;

    /* Copy the points to the returned data structure. */
    for (i = 0, j = 0; i < N; i++) {
	while (IntegVal >= Integ[j])
	    j++;
	x = j - 1 + (IntegVal - Integ[j - 1]) / (Integ[j] - Integ[j - 1]);
	IntegVal += IntegDelta;

	Dist[i] = XMin + Dx * x / IntegN;
    }

    IritFree(Integ);

    return Dist;
}
