/******************************************************************************
* MvarTopo.c - Handle silhouttes (and related curves) of freeforms.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 09.					      *
******************************************************************************/

#include "allocate.h"
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the silhouette edges of the given surfaces, orthographically    M
* seen from the given view direction VDir.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute its silhouette edges.                           M
*   VDir:         View direction vector (a unit vector).                     M
*   Step:	  Step size for curve tracing.				     M
*   SubdivTol:	  The subdivision tolerance to use.			     M
*   NumericTol:	  The numerical tolerance to use.			     M
*   Euclidean:    If TRUE, returns the silhouettes in Euclidean space.       M
*		  Otherwise, the silhouette edges are returned in the        M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The silhouettes as piecewise linear edges.            M
*                      Can include two object, 1st with points.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfOrthotomic, SymbSrfSilhouette			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSilhouette                                                        M
*****************************************************************************/
IPObjectStruct *MvarSrfSilhouette(const CagdSrfStruct *Srf,
				  const CagdVType VDir,
				  CagdRType Step,
				  CagdRType SubdivTol,
				  CagdRType NumericTol,
				  CagdBType Euclidean)
{
    CagdSrfStruct
	*NrmlSrf = SymbSrfNormalSrf(Srf),
	*ProjNrmlSrf = SymbSrfVecDotProd(NrmlSrf, VDir);
    MvarMVStruct *MV;
    MvarPolyStruct *Pls;
    IPObjectStruct *PSils;

    if (CAGD_IS_RATIONAL_SRF(Srf)) {
        CagdSrfStruct
	    *TSrf = CagdCoerceSrfsTo(ProjNrmlSrf, CAGD_PT_E1_TYPE, FALSE);

        MV = MvarSrfToMV(TSrf);
	CagdSrfFree(TSrf);
    }
    else
        MV = MvarSrfToMV(ProjNrmlSrf);

    CagdSrfFree(NrmlSrf);
    CagdSrfFree(ProjNrmlSrf);

    Pls = MvarMVUnivarInter(&MV, Step, SubdivTol, NumericTol);

    MvarMVFree(MV);

    if (Pls == NULL)
	return NULL;

    PSils = MvarCnvrtMVPolysToIritPolys2(Pls, TRUE);
    MvarPolyFreeList(Pls);

    if (Euclidean) {
	IPPolygonStruct *Pl;

        assert(IP_IS_POLY_OBJ(PSils));

	for (Pl = PSils -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *V;
	    CagdRType *R;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	        R = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);
		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	}
    }

    return PSils;
}
