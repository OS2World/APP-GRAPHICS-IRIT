/******************************************************************************
* Trng2Ply.c - Converts a triangular surface surface into polygons.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single triangular surface to a set of triangles       M
* approximating it. 							     M
*   FineNess is a fineness control on result and the larger it is more       M
* triangles may result.							     M
*   A value of 10 is a good starting value.				     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:           To approximate into triangles.                         M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrf2Polygons, polygonization, surface approximation               M
*****************************************************************************/
CagdPolygonStruct *TrngTriSrf2Polygons(const TrngTriangSrfStruct *TriSrf,
				       int FineNess,
				       CagdBType ComputeNormals,
				       CagdBType ComputeUV)
{
    int i, j;
    CagdPtStruct *Pts;
    CagdVecStruct *Nrml;
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax, Du, Dv, u, v;
    CagdPolygonStruct
	*CagdPlList = NULL;

    TrngTriSrfDomain(TriSrf, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);
    Du = (UMax - UMin - IRIT_UEPS) / FineNess;
    Dv = (VMax - VMin - IRIT_UEPS) / FineNess;

    Pts = (CagdPtStruct *) IritMalloc(sizeof(CagdPtStruct) * (FineNess + 1));
    Nrml = (CagdVecStruct *) IritMalloc(sizeof(CagdVecStruct) * (FineNess + 1));

    for (v = VMin, j = 0; j <= FineNess; v += Dv, j++) { /* Eval first row. */
	CagdRType
	    *R = TrngTriSrfEval2(TriSrf, UMin, v);

	CagdCoerceToE3(Pts[j].Pt, &R, -1, TriSrf -> PType);

	if (ComputeNormals) {
	    Nrml[j] = *TrngTriSrfNrml(TriSrf, UMin, v);
	}
    }

    for (u = UMin + Du, i = 1; i <= FineNess; u += Du, i++) {
	CagdPType LastPt;
	CagdVType LastNrml;

	for (v = VMin, j = 0; j + i <= FineNess; v += Dv, j++) {
	    CagdRType
	        *R = TrngTriSrfEval2(TriSrf, u, v);
	    CagdPType NewPt;
	    CagdVType NewNrml;
	    CagdPolygonStruct
	        *CagdPl = CagdPolygonNew(3);

	    CagdCoerceToE3(NewPt, &R, -1, TriSrf -> PType);

	    if (ComputeNormals) {
		CagdVecStruct
		    *N = TrngTriSrfNrml(TriSrf, u, v);

		IRIT_PT_COPY(NewNrml, N -> Vec);
	    }

	    if (j > 0) {
		IRIT_PT_COPY(CagdPl -> U.Polygon[0].Pt, LastPt);
		IRIT_PT_COPY(CagdPl -> U.Polygon[1].Pt, NewPt);
		IRIT_PT_COPY(CagdPl -> U.Polygon[2].Pt, Pts[j].Pt);
		if (ComputeNormals) {
		    IRIT_PT_COPY(CagdPl -> U.Polygon[0].Nrml, LastNrml);
		    IRIT_PT_COPY(CagdPl -> U.Polygon[1].Nrml, NewNrml);
		    IRIT_PT_COPY(CagdPl -> U.Polygon[2].Nrml, Nrml[j].Vec);
		}
		if (ComputeUV) {
		    CagdPl -> U.Polygon[0].UV[0] = u;
		    CagdPl -> U.Polygon[0].UV[1] = v - Dv;
		    CagdPl -> U.Polygon[1].UV[0] = u;
		    CagdPl -> U.Polygon[1].UV[1] = v;
		    CagdPl -> U.Polygon[2].UV[0] = u - Du;
		    CagdPl -> U.Polygon[2].UV[1] = v - Dv;
		}
		IRIT_LIST_PUSH(CagdPl, CagdPlList);

	        CagdPl = CagdPolygonNew(3);
	    }

	    IRIT_PT_COPY(CagdPl -> U.Polygon[0].Pt, Pts[j + 1].Pt);
	    IRIT_PT_COPY(CagdPl -> U.Polygon[1].Pt, Pts[j].Pt);
	    IRIT_PT_COPY(CagdPl -> U.Polygon[2].Pt, NewPt);
	    if (ComputeNormals) {
		IRIT_PT_COPY(CagdPl -> U.Polygon[0].Nrml, Nrml[j + 1].Vec);
		IRIT_PT_COPY(CagdPl -> U.Polygon[1].Nrml, Nrml[j].Vec);
		IRIT_PT_COPY(CagdPl -> U.Polygon[2].Nrml, NewNrml);
	    }
	    if (ComputeUV) {
		CagdPl -> U.Polygon[0].UV[0] = u - Du;
		CagdPl -> U.Polygon[0].UV[1] = v;
		CagdPl -> U.Polygon[1].UV[0] = u - Du;
		CagdPl -> U.Polygon[1].UV[1] = v - Dv;
		CagdPl -> U.Polygon[2].UV[0] = u;
		CagdPl -> U.Polygon[2].UV[1] = v;
	    }
	    IRIT_LIST_PUSH(CagdPl, CagdPlList);

	    IRIT_PT_COPY(LastPt, NewPt);
	    IRIT_PT_COPY(Pts[j].Pt, LastPt);
	    if (ComputeNormals) {
		IRIT_PT_COPY(LastNrml, NewNrml);
		IRIT_PT_COPY(Nrml[j].Vec, LastNrml);
	    }

	}
    }

    IritFree(Pts);
    IritFree(Nrml);

    return CagdPlList;
}
