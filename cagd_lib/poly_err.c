/*****************************************************************************
*   Verification of polygonal approximation of freeform surfaces.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, April 1995. *
*****************************************************************************/

#include "irit_sm.h"
#include "cagd_loc.h"
#include "geom_lib.h"

IRIT_STATIC_DATA int
    GlblErrNumOfSamples = 10;

IRIT_STATIC_DATA CagdPolyErrEstimateType
    GlblErrEstimateMethod = CAGD_POLY_APPROX_ERR_CENTER;

static CagdRType ComputeErr(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    const CagdRType *Plane);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the methods of sampling the error of a polygonal approximation.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Method:     1. Samples one distance at the center of each polygon.       M
*		2. Samples Samples samples uniformly distributed in the      M
*		   parametric area of each polygon and selects the maximum.  M
*		3. Samples Samples samples uniformly distributed in the      M
*		   parametric area of each polygon and selects the average.  M
*   Samples:    Number of samples to sample in the parametric domain of each M
*		polygon.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	Old sampling method + Old sampling rate << 8                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolyApproxErrEstimate, polygonization, error estimate                M
*****************************************************************************/
int CagdPolyApproxErrEstimate(CagdPolyErrEstimateType Method, int Samples)
{
    int OrigMethod = GlblErrEstimateMethod,
	OrigSamples = GlblErrNumOfSamples;

    switch (Method) {
	case CAGD_POLY_APPROX_ERR_CENTER:
	case CAGD_POLY_APPROX_ERR_SAMPLES_MAX:
	case CAGD_POLY_APPROX_ERR_SAMPLES_AVG:
	    GlblErrEstimateMethod = Method;
	    break;
	default:
	    GlblErrEstimateMethod = CAGD_POLY_APPROX_ERR_CENTER;
	    break;
    }

    GlblErrNumOfSamples = Samples;

    return OrigMethod + (OrigSamples << 8);
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the maximal error between the surface and its polygonal approx.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Approximated surface.                                         M
*   Polys:     The given polygonal approximation.                            M
*	       Assumes UV slots are computed and updated in Polys.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Maximal error between surface and polygonal approximation   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolyApproxMaxErr, polygonization, error estimate                     M
*****************************************************************************/
CagdRType CagdPolyApproxMaxErr(const CagdSrfStruct *Srf,
			       const CagdPolygonStruct *Polys)
{
    int i,
	Len = CagdListLength(Polys);
    CagdRType
	*Errs = CagdPolyApproxErrs(Srf, Polys),
	MaxErr = 0;

    for (i = 0; i < Len; i++)
        if (MaxErr < Errs[i])
	    MaxErr = Errs[i];

    IritFree(Errs);

    return MaxErr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the errors between the surface and its polygonal approx.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Approximated surface.                                         M
*   Polys:     The given polygonal approximation.                            M
*	       Assumes UV slots are computed and updated in Polys.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Errors between surface and its polygons. A vector of size M
*		(number of polygons + 1) holding the maximal error of each   M
*		polygon. The last element of the vector will be negative.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolyApproxErrs, polygonization, error estimate                       M
*****************************************************************************/
CagdRType *CagdPolyApproxErrs(const CagdSrfStruct *Srf,
			      const CagdPolygonStruct *Polys)
{
    int i,
	NumOfPolys = CagdListLength(Polys);
    CagdRType E, u, v, w1, w2, w3, w,
	*Errs = IritMalloc((1 + NumOfPolys) * sizeof(CagdRType));

    for (i = 0; i < NumOfPolys; i++, Polys = Polys -> Pnext) {
	int j;
	IrtPlnType Plane;

	if (Polys -> PolyType == CAGD_POLYGON_TYPE_POLYSTRIP) {
	    CAGD_FATAL_ERROR(CAGD_ERR_POLYGON_EXPECTED);
	    continue;
	}

	GMPlaneFrom3Points(Plane,
			   Polys -> U.Polygon[0].Pt,
			   Polys -> U.Polygon[1].Pt,
			   Polys -> U.Polygon[2].Pt);

	switch (GlblErrEstimateMethod) {
	    case CAGD_POLY_APPROX_ERR_CENTER:
	        u = (Polys -> U.Polygon[0].UV[0] +
		     Polys -> U.Polygon[1].UV[0] +
		     Polys -> U.Polygon[2].UV[0]) / 3.0;
	        v = (Polys -> U.Polygon[0].UV[1] +
		     Polys -> U.Polygon[1].UV[1] +
		     Polys -> U.Polygon[2].UV[1]) / 3.0;
		Errs[i] = ComputeErr(Srf, u, v, Plane);
		break;
	    default:
	    case CAGD_POLY_APPROX_ERR_SAMPLES_MAX:
		Errs[i] = 0.0;
		for (j = 0; j < GlblErrNumOfSamples; j++) {
		    w1 = IritRandom(0, 1);
		    w2 = IritRandom(0, 1);
		    w3 = IritRandom(0, 1);
		    w = w1 + w2 + w3;
		    u = (w1 * Polys -> U.Polygon[0].UV[0] +
		         w2 * Polys -> U.Polygon[1].UV[0] +
			 w3 * Polys -> U.Polygon[2].UV[0]) / w;
		    v = (w1 * Polys -> U.Polygon[0].UV[1] +
			 w2 * Polys -> U.Polygon[1].UV[1] +
			 w3 * Polys -> U.Polygon[2].UV[1]) / w;
		    E = ComputeErr(Srf, u, v, Plane);
		    if (Errs[i] < E)
		        Errs[i] = E;
		}
		break;
	    case CAGD_POLY_APPROX_ERR_SAMPLES_AVG:
		Errs[i] = 0.0;
		for (j = 0; j < GlblErrNumOfSamples; j++) {
		    w1 = IritRandom(0, 1);
		    w2 = IritRandom(0, 1);
		    w3 = IritRandom(0, 1);
		    w = w1 + w2 + w3;
		    u = (w1 * Polys -> U.Polygon[0].UV[0] +
		         w2 * Polys -> U.Polygon[1].UV[0] +
			 w3 * Polys -> U.Polygon[2].UV[0]) / w;
		    v = (w1 * Polys -> U.Polygon[0].UV[1] +
			 w2 * Polys -> U.Polygon[1].UV[1] +
			 w3 * Polys -> U.Polygon[2].UV[1]) / w;
		    Errs[i] += ComputeErr(Srf, u, v, Plane);
		}
		Errs[i] /= GlblErrNumOfSamples;
		break;
	}
    }

    Errs[i] = -1.0; /* Signals end of list. */

    return Errs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the distance between the location (u, v) on surface Srf and the *
* plane of polygon Poly.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        To evaluate at the given u, v location against the polygon.  *
*   u, v:       The location on Srf to test for distance from given polygon. *
*   Plane:      Of polygon that approximate the region of Srf(u, v) is in.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  Computed distance between Srf(u, v) and plane of Poly.       *
*****************************************************************************/
static CagdRType ComputeErr(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    const CagdRType *Plane)
{
    CagdPType E3Pt;
    CagdRType
	*Pt = CagdSrfEval(Srf, u, v);

    CagdCoerceToE3(E3Pt, &Pt, -1, Srf -> PType);

    return IRIT_FABS(GMDistPointPlane(E3Pt, Plane));
}
