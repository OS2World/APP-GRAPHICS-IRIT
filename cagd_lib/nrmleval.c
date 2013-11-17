/******************************************************************************
* NrmlEval.c - Normal evaluation for freeform surface.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 97.					      *
******************************************************************************/

#include "cagd_loc.h"

IRIT_STATIC_DATA CagdSrfStruct
    *GlblDuSrf = NULL,
    *GlblDvSrf = NULL,
    *GlblSrf = NULL;
IRIT_STATIC_DATA CagdCrvStruct
    *GlblDuSrfUIso = NULL,
    *GlblDvSrfUIso = NULL,
    *GlblSrfUIso = NULL;
IRIT_STATIC_DATA CagdRType
     GlblUValue = -IRIT_INFNTY;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Do the necessary preprocessing so we can efficiently evaluate normal on  M
* Srf.  For best efficiency normals with same U values should be evaluated   M
* in a sequence, before moving to the next U.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Do preprocess for fast normal evaluations.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, CagdSrfEffiNrmlEval, CagdSrfEffiNrmlPostlude              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfEffiNrmlPrelude                                                   M
*****************************************************************************/
void CagdSrfEffiNrmlPrelude(const CagdSrfStruct *Srf)
{
    GlblDuSrf = CagdSrfDeriveScalar(Srf, CAGD_CONST_U_DIR);
    GlblDvSrf = CagdSrfDeriveScalar(Srf, CAGD_CONST_V_DIR);
    if (CAGD_IS_RATIONAL_SRF(Srf))
        GlblSrf = CagdSrfCopy(Srf);
    else
      	GlblSrf = NULL;

    GlblDuSrfUIso = NULL;
    GlblDvSrfUIso = NULL;
    GlblSrfUIso = NULL;

    GlblUValue = -IRIT_INFNTY;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evalue the surface normal at the given (u, v) surface location.  The     M
* normal is normalized if Normalize is TRUE.  For best performance normal    M
* locations with the same U values should be invoking this function in a     M
* sequence before moving on to a different U value.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   u, v:      Parameter values of the location on the surface to compute    M
*	       the normal for.  For efficiency, no test is made as for the   M
*	       validity of the (u, v) position.				     M
*   Normalize: if TRUE, the normal is normalized into a unit length.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:   A pointer to a statically allocated normal vector.    M
*                      A all zero vector is returned if failed to compute.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, CagdSrfEffiNrmlPrelude, CagdSrfEffiNrmlPostlude           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfEffiNrmlEval                                                      M
*****************************************************************************/
CagdVecStruct *CagdSrfEffiNrmlEval(CagdRType u,
				   CagdRType v,
				   CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct RetVec;
    CagdRType V1[4], V2[4], P1[4], P2[4], *R;

    if (!IRIT_APX_EQ_EPS(u, GlblUValue, IRIT_UEPS)) {
        /* Extract the necessary isoparametric curves out of the surface. */
        if (GlblDuSrfUIso != NULL)
	    CagdCrvFree(GlblDuSrfUIso);
        if (GlblDvSrfUIso != NULL)
	    CagdCrvFree(GlblDvSrfUIso);
        if (GlblSrfUIso != NULL)
	    CagdCrvFree(GlblSrfUIso);

	GlblDuSrfUIso = CagdCrvFromSrf(GlblDuSrf, u, CAGD_CONST_U_DIR);
	GlblDvSrfUIso = CagdCrvFromSrf(GlblDvSrf, u, CAGD_CONST_U_DIR);
	if (GlblSrf)
	    GlblSrfUIso = CagdCrvFromSrf(GlblSrf, u, CAGD_CONST_U_DIR);

	GlblUValue = u;
    }

    if (GlblSrf != NULL) {
        CagdRType
	    *V11 = &V1[1],
	    *V21 = &V2[1],
	    *P11 = &P1[1],
	    *P21 = &P2[1];

        /* This is a rational surface. */
        R = CagdCrvEval(GlblDuSrfUIso, v);
	CagdCoerceToP3(V1, &R, -1, GlblDuSrfUIso -> PType);
        R = CagdCrvEval(GlblDvSrfUIso, v);
	CagdCoerceToP3(V2, &R, -1, GlblDvSrfUIso -> PType);
        R = CagdCrvEval(GlblSrfUIso, v);
	CagdCoerceToP3(P1, &R, -1, GlblSrfUIso -> PType);
	CAGD_GEN_COPY(P2, P1, sizeof(CagdRType) * 4);

	IRIT_PT_SCALE(V11, P1[0]); /* Numerator of quotient rule X'W - XW'. */
	IRIT_PT_SCALE(P11, V1[0]);
	IRIT_PT_SUB(V11, V11, P11);
	IRIT_PT_SCALE(V21, P2[0]);
	IRIT_PT_SCALE(P21, V2[0]);
	IRIT_PT_SUB(V21, V21, P21);
	IRIT_CROSS_PROD(RetVec.Vec, V21, V11);
    }
    else {
        /* This is an integral surface. */
        R = CagdCrvEval(GlblDuSrfUIso, v);
	CagdCoerceToE3(V1, &R, -1, GlblDuSrfUIso -> PType);
        R = CagdCrvEval(GlblDvSrfUIso, v);
	CagdCoerceToE3(V2, &R, -1, GlblDvSrfUIso -> PType);
	IRIT_CROSS_PROD(RetVec.Vec, V2, V1);
    }

    if (Normalize && !IRIT_PT_APX_EQ_ZERO_EPS(RetVec.Vec, IRIT_EPS))
        IRIT_PT_NORMALIZE(RetVec.Vec);

    return &RetVec;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Released all data structures allocated by this efficient normal          M
* evalation routines.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, CagdSrfEffiNrmlEval, CagdSrfEffiNrmlPrelude               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfEffiNrmlPostlude                                                  M
*****************************************************************************/
void CagdSrfEffiNrmlPostlude(void)
{
    CagdSrfFree(GlblDuSrf);
    GlblDuSrf = NULL;
    
    CagdSrfFree(GlblDvSrf);
    GlblDvSrf = NULL;

    CagdSrfFree(GlblSrf);
    GlblSrf = NULL;

    
    CagdCrvFree(GlblDuSrfUIso);
    GlblDuSrfUIso = NULL;

    CagdCrvFree(GlblDvSrfUIso);
    GlblDvSrfUIso = NULL;

    CagdCrvFree(GlblSrfUIso);
    GlblSrfUIso = NULL;

    GlblUValue = -IRIT_INFNTY;
}

