/******************************************************************************
* Orthotom.c - computation of k-orthotomics of curves and surfaces.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, February 97					      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"

#define CONTOUR_EPS   1.5902720e-8  /* Level above zero to actually contour. */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the K-orthotomic of a curve with respect to point P:            M
*       P + K < (C(t) - P), N(t) > N(t)                                      V
*   See "Fundamentals of Computer Aided Geometric Design", by J. Hoschek and M
* and D. Lasser.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     To compute its K-orthotomic                                     M
*   P:	     The points to which the K-orthotomic is computed for Crv for.   M
*   K:       The magnitude of the orthotomic function.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The K-orthotomic                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfOrthotomic                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvOrthotomic                                                        M
*****************************************************************************/
CagdCrvStruct *SymbCrvOrthotomic(const CagdCrvStruct *Crv,
				 const CagdPType P,
				 CagdRType K)
{
    CagdPType PTmp;
    CagdCrvStruct *NCrv, *TCrv1, *TCrv2, *TCrv3, *Orthotomic;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) != 2) {
        SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D);
	return NULL;
    }

    NCrv = SymbCrv2DUnnormNormal(Crv);

    IRIT_PT_COPY(PTmp, P);
    IRIT_PT_SCALE(PTmp, -1.0);

    TCrv2 = CagdCrvCopy(Crv);
    CagdCrvTransform(TCrv2, PTmp, 1.0);
    TCrv1 = SymbCrvDotProd(TCrv2, NCrv);
    CagdCrvFree(TCrv2);

    TCrv3 = SymbCrvMultScalar(NCrv, TCrv1);
    CagdCrvFree(TCrv1);
    
    TCrv1 = SymbCrvDotProd(NCrv, NCrv);
    CagdCrvFree(NCrv);

    if (CAGD_IS_RATIONAL_CRV(TCrv1)) {
	TCrv2 = SymbCrvInvert(TCrv1);

	Orthotomic = SymbCrvMultScalar(TCrv3, TCrv2);
	CagdCrvFree(TCrv2);
    }
    else {
	CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ;

	SymbCrvSplitScalar(TCrv3, &CrvW, &CrvX, &CrvY, &CrvZ);

	CagdMakeCrvsCompatible(&TCrv1, &CrvX, TRUE, TRUE);
	CagdMakeCrvsCompatible(&TCrv1, &CrvY, TRUE, TRUE);
	CagdMakeCrvsCompatible(&CrvX,  &CrvY, TRUE, TRUE);

	Orthotomic = SymbCrvMergeScalar(TCrv1, CrvY, CrvX, NULL);
	CagdCrvFree(CrvX);
	CagdCrvFree(CrvY);
	if (CrvZ != NULL)
	    CagdCrvFree(CrvZ);
    }
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv3);

    CagdCrvTransform(Orthotomic, NULL, K);
    CagdCrvTransform(Orthotomic, P, 1.0);

    return Orthotomic;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the K-orthotomic of a surface with respect to point P:          M
*       P + K < (S(u,v) - P), N(u,v) > N(u,v)                                V
*   See "Fundamentals of Computer Aided Geometric Design, by J. Hoschek and  M
* and D. Lasser.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To compute its K-orthotomic                                     M
*   P:	     The points to which the K-orthotomic is computed for Srf for.   M
*   K:       The magnitude of the orthotomic function.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The K-orthotomic                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOrthotomic, SymbSrfSilhouette                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfOrthotomic                                                        M
*****************************************************************************/
CagdSrfStruct *SymbSrfOrthotomic(const CagdSrfStruct *Srf,
				 const CagdPType P,
				 CagdRType K)
{
    CagdPType PTmp;
    CagdSrfStruct *NSrf, *TSrf1, *TSrf2, *TSrf3, *Orthotomic;

    NSrf = SymbSrfNormalSrf(Srf);

    IRIT_PT_COPY(PTmp, P);
    IRIT_PT_SCALE(PTmp, -1.0);

    TSrf2 = CagdSrfCopy(Srf);
    CagdSrfTransform(TSrf2, PTmp, 1.0);
    TSrf1 = SymbSrfDotProd(TSrf2, NSrf);
    CagdSrfFree(TSrf2);

    TSrf3 = SymbSrfMultScalar(NSrf, TSrf1);
    CagdSrfFree(TSrf1);
    
    TSrf1 = SymbSrfDotProd(NSrf, NSrf);
    CagdSrfFree(NSrf);

    if (CAGD_IS_RATIONAL_SRF(TSrf1)) {
	TSrf2 = SymbSrfInvert(TSrf1);

	Orthotomic = SymbSrfMultScalar(TSrf3, TSrf2);
	CagdSrfFree(TSrf2);
    }
    else {
	CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ;

	SymbSrfSplitScalar(TSrf3, &SrfW, &SrfX, &SrfY, &SrfZ);

	CagdMakeSrfsCompatible(&TSrf1, &SrfX, TRUE, TRUE, TRUE, TRUE);
	CagdMakeSrfsCompatible(&TSrf1, &SrfY, TRUE, TRUE, TRUE, TRUE);
	CagdMakeSrfsCompatible(&SrfX,  &SrfY, TRUE, TRUE, TRUE, TRUE);

	Orthotomic = SymbSrfMergeScalar(TSrf1, SrfY, SrfX, SrfZ);
	CagdSrfFree(SrfX);
	CagdSrfFree(SrfY);
	if (SrfZ != NULL)
	    CagdSrfFree(SrfZ);
    }
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf3);

    CagdSrfTransform(Orthotomic, NULL, K);
    CagdSrfTransform(Orthotomic, P, 1.0);

    return Orthotomic;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the silhouette edges of the given surfaces, orthographically    M
* seen from the given view direction VDir.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute its silhouette edges.                           M
*   VDir:         View direction vector (a unit vector).                     M
*   Tolerance:    Accuracy of computation.				     M
*   Euclidean:    If TRUE, returns the silhouettes in Euclidean space.       M
*		  Otherwise, the silhouette edges are returned in the        M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The silhouettes as piecewise linear edges.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfOrthotomic, SymbSrfIsocline, SymbSrfPolarSilhouette               M
*   MvarSrfSilhouette						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSilhouette                                                        M
*****************************************************************************/
IPPolygonStruct *SymbSrfSilhouette(const CagdSrfStruct *Srf,
				   const CagdVType VDir,
				   CagdRType Tolerance,
				   CagdBType Euclidean)
{
    IRIT_STATIC_DATA const IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };    /* A scalar srf - only X. */
    CagdRType UMin, UMax, VMin, VMax;
    IPPolygonStruct *Cntrs, *Cntr;
    CagdSrfStruct
	*NrmlSrf = SymbSrfNormalSrf(Srf),
	*ProjNrmlSrf = SymbSrfVecDotProd(NrmlSrf, VDir);

    CagdSrfFree(NrmlSrf);

    Cntrs = UserCntrSrfWithPlane(ProjNrmlSrf, Plane, Tolerance);

    CagdSrfFree(ProjNrmlSrf);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;

	/* Maps the UV data as required. */
	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    if (Euclidean) {
	        CagdRType
		  *R = CagdSrfEval(Srf, IRIT_BOUND(V -> Coord[1], UMin, UMax),
				        IRIT_BOUND(V -> Coord[2], VMin, VMax));

		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	    else {
	        V -> Coord[0] = V -> Coord[1];
		V -> Coord[1] = V -> Coord[2];
		V -> Coord[2] = 0.0;
	    }
	}
    }

    return Cntrs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the polar silhouette edges of the given surfaces, along axis    M
* VDir.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute its polar silhouette edges.                     M
*   VDir:         Axis of polar silhouette.		                     M
*   Tolerance:    Accuracy of computation.				     M
*   Euclidean:    If TRUE, returns the silhouettes in Euclidean space.       M
*		  Otherwise, the silhouette edges are returned in the        M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The silhouettes as piecewise linear edges.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfOrthotomic, SymbSrfSilhouette, SymbSrfIsocline                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfPolarSilhouette                                                   M
*****************************************************************************/
IPPolygonStruct *SymbSrfPolarSilhouette(const CagdSrfStruct *Srf,
					const CagdVType VDir,
					CagdRType Tolerance,
					CagdBType Euclidean)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };    /* A scalar srf - only X. */
    IPPolygonStruct *Cntrs, *Cntr;
    CagdSrfStruct
	*NrmlSrf = SymbSrfNormalSrf(Srf),
	*PlrNrmlSrf = SymbSrfCrossProd(Srf, NrmlSrf),
	*ProjPlrNrmlSrf = SymbSrfVecDotProd(PlrNrmlSrf, VDir);

    CagdSrfFree(NrmlSrf);
    CagdSrfFree(PlrNrmlSrf);

    Cntrs = UserCntrSrfWithPlane(ProjPlrNrmlSrf, Plane, Tolerance);

    CagdSrfFree(ProjPlrNrmlSrf);

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;

	/* Maps the UV data as required. */
	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    if (Euclidean) {
	        CagdRType
		    *R = CagdSrfEval(Srf, V -> Coord[1], V -> Coord[2]);

		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	    else {
	        V -> Coord[0] = V -> Coord[1];
		V -> Coord[1] = V -> Coord[2];
		V -> Coord[2] = 0.0;
	    }
	}
    }

    return Cntrs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the isocline edges of the given surfaces, orthographically      M
* seen from the given view direction VDir, at an inclination angle of Theta  M
* degrees.								     M
*   The isocline is a curve with a fixed angle between the surface normal    M
* and the viewing direction.  An angle of 90 degrees yields the silhouettes. M
*   Computed as the zero set of:					     M
*				 (<N, V>)^2 - (Cos(Theta))^2 <N, N>          V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute its isocline edges.                             M
*   VDir:         View direction vector (a unit vector).                     M
*   Theta:        The fixed angle between the viewing direction and the      M
*		  surface normal, in degrees.				     M
*		  An angle of 90 degrees yields the silhouettes.	     M
*   Tolerance:    Accuracy of computation.				     M
*   Euclidean:    If TRUE, returns the isoclines in Euclidean space.         M
*		  Otherwise, the isocline edges are returned in the          M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The isoclines as piecewise linear edges.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfOrthotomic, SymbSrfSilhouette, UserMoldReliefAngle2Srf            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfIsocline                                                          M
*****************************************************************************/
IPPolygonStruct *SymbSrfIsocline(const CagdSrfStruct *Srf,
				 const CagdVType VDir,
				 CagdRType Theta,
				 CagdRType Tolerance,
				 CagdBType Euclidean)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };    /* A scalar srf - only X. */
    IPPolygonStruct *Cntrs, *Cntr;
    CagdRType
	CosThetaSqr = IRIT_SQR(cos(IRIT_DEG2RAD(Theta)));
    CagdVType VDirNormalized;
    CagdSrfStruct *NrmlSrf, *ProjNrmlSrf, *NrmlSqrSizeSrf, *TSrf1, *TSrf2,
	*TSrf3;

    IRIT_VEC_COPY(VDirNormalized, VDir);
    IRIT_VEC_NORMALIZE(VDirNormalized);

    if (IRIT_APX_EQ(Theta, 90.0))
        return SymbSrfSilhouette(Srf, VDirNormalized, Tolerance, Euclidean);

    NrmlSrf = SymbSrfNormalSrf(Srf);
    ProjNrmlSrf = SymbSrfVecDotProd(NrmlSrf, VDirNormalized);
    NrmlSqrSizeSrf = SymbSrfDotProd(NrmlSrf, NrmlSrf);
    TSrf1 = SymbSrfMult(ProjNrmlSrf, ProjNrmlSrf);
    TSrf2 = SymbSrfScalarScale(NrmlSqrSizeSrf, CosThetaSqr);
    TSrf3 = SymbSrfSub(TSrf1, TSrf2);

    CagdSrfFree(NrmlSrf);
    CagdSrfFree(ProjNrmlSrf);
    CagdSrfFree(NrmlSqrSizeSrf);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    Cntrs = UserCntrSrfWithPlane(TSrf3, Plane, Tolerance);

    CagdSrfFree(TSrf3);

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;

	/* Maps the UV data as required. */
	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    if (Euclidean) {
	        CagdRType
		    *R = CagdSrfEval(Srf, V -> Coord[1], V -> Coord[2]);

		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	    else {
	        V -> Coord[0] = V -> Coord[1];
		V -> Coord[1] = V -> Coord[2];
		V -> Coord[2] = 0.0;
	    }
	}
    }

    return Cntrs;
}
