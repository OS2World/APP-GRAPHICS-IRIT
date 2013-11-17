/******************************************************************************
* SBzrEval.c - Bezier surfaces handling routines - evaluation routines.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given tensor product Bezier surface at a given point, by     M
* extracting an isoparamteric curve along u from the surface and evaluating  M
* the curve at parameter v.                                                  M
*									     M
*		u -->							     V
*     +----------------------+						     V
*     |P0		 Pi-1|						     V
*   V |Pi		P2i-1|	Parametric space orientation - control mesh. V
*    ||			     |						     V
*    v|Pn-i		 Pn-1|						     V
*     +----------------------+						     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to evaluate at the given (u, v) location.             M
*   u, v:      Location where to evaluate the surface.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfEval, BspSrfEvalAtParam, BspSrfEvalAtParam2, TrimSrfEval          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfEvalAtParam, evaluation, Bezier                                    M
*****************************************************************************/
CagdRType *BzrSrfEvalAtParam(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v)
{
    CagdRType *Pt;
    CagdCrvStruct
	*IsoCrv = BzrSrfCrvFromSrf(Srf, u, CAGD_CONST_U_DIR);

    Pt = BzrCrvEvalAtParam(IsoCrv, v);

    CagdCrvFree(IsoCrv);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts an isoparametric curve out of the given tensor product Bezier     M
* surface in direction Dir at the parameter value of t.                      M
*   Operations should prefer the CONST_U_DIR, in which the extraction is     M
* somewhat faster if that is possible.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract an isoparametric ocurve from.                      M
*   t:         Parameter value of extracted isoparametric curve.             M
*   Dir:       Direction of the isocurve on the surface. Either U or V.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  An isoparametric curve of Srf. This curve inherits the M
*		      order and continuity of surface Srf in direction Dir.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, BspSrfCrvFromSrf, CagdCrvFromMesh, BzrSrfCrvFromMesh,    M
*   BspSrfCrvFromMesh					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfCrvFromSrf, isoparametric curves, curve from surface               M
*****************************************************************************/
CagdCrvStruct *BzrSrfCrvFromSrf(const CagdSrfStruct *Srf,
				CagdRType t,
				CagdSrfDirType Dir)
{
    CagdCrvStruct
	*Crv = NULL;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, CrvOrder, VecLen,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CrvP, *SrfP;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Crv = BzrCrvNew(CrvOrder = Srf -> VLength, Srf -> PType);
	    VecLen = Srf -> ULength;

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i];
		for (j = 0; j < CrvOrder; j++) {
		    *CrvP++ = BzrCrvEvalVecAtParam(SrfP, CAGD_NEXT_U(Srf),
								VecLen, t);
		    SrfP += CAGD_NEXT_V(Srf);
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    Crv = BzrCrvNew(CrvOrder = Srf -> ULength, Srf -> PType);
	    VecLen = Srf -> VLength;

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i];
		for (j = 0; j < CrvOrder; j++) {
		    *CrvP++ = BzrCrvEvalVecAtParam(SrfP, CAGD_NEXT_V(Srf),
								VecLen, t);
		    SrfP += CAGD_NEXT_U(Srf);
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a curve from the mesh of a tensor product Bezier surface Srf in   M
* direction Dir at index Index.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract a curve from.  		                     M
*   Index:     Index along the mesh of Srf to extract the curve from.        M
*   Dir:       Direction of extracted curve. Either U or V.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve from Srf. This curve inherit the order and    M
*                      continuity of surface Srf in direction Dir. However,  M
*                      thiscurve is not on surface Srf, in general.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, BzrSrfCrvFromSrf, BspSrfCrvFromSrf,                      M
*   CagdCrvFromMesh, BspSrfCrvFromMesh                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfCrvFromMesh, isoparametric curves, curve from mesh                 M
*****************************************************************************/
CagdCrvStruct *BzrSrfCrvFromMesh(const CagdSrfStruct *Srf,
				 int Index,
				 CagdSrfDirType Dir)
{
    CagdCrvStruct
	*Crv = NULL;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, CrvOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CrvP, *SrfP;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    if (Index + 1 > Srf -> ULength)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    Crv = BzrCrvNew(CrvOrder = Srf -> VLength, Srf -> PType);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_U(Srf);
		for (j = 0; j < CrvOrder; j++) {
		    *CrvP++ = *SrfP;
		    SrfP += CAGD_NEXT_V(Srf);
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    if (Index + 1 > Srf -> VLength)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    Crv = BzrCrvNew(CrvOrder = Srf -> ULength, Srf -> PType);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_V(Srf);
		for (j = 0; j < CrvOrder; j++) {
		    *CrvP++ = *SrfP;
		    SrfP += CAGD_NEXT_U(Srf);
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
    return Crv;
}

