/******************************************************************************
* NrmlCone.c - normal cone bounds for normal fields.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, October 95.					      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"

#define Determinant3x3(a1,a2,a3,b1,b2,b3,c1,c2,c3) \
                (a1)*(b2)*(c3) + (b1)*(c2)*(a3) + (c1)*(a2)*(b3)  \
              -((c1)*(b2)*(a3) + (b1)*(a2)*(c3) + (a1)*(c2)*(b3));

IRIT_STATIC_DATA int
   GlblSymbNormalConeForSrfOptimal = FALSE;

static CagdRType SymbNormalConeMaxAngle(const CagdSrfStruct *NrmlSrf,
					const CagdVType Axis);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a tangent cone for a given curve, by examine the control polygon  M
* of the curve and deriving its angular span.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:    To compute a tangent cone for.                                   M
*   Planar: If TRUE, only the X and Y coefficients are considered.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const SymbNormalConeStruct *:    The computed tangent cone, statically   M
*                                    allocated.	                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeOverlap, SymbNormalConeForSrf                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTangentConeForCrv, tangents, tangent bound                           M
*****************************************************************************/
const SymbNormalConeStruct *SymbTangentConeForCrv(const CagdCrvStruct *Crv,
					          int Planar)
{
    IRIT_STATIC_DATA SymbNormalConeStruct
	TangentCone;
    int i, j, Length;
    CagdRType * const *Points, t, ConeAngle;
    CagdVType ConeAxis, Tan;

    Crv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);

    Points = Crv -> Points;
    Length = Crv -> Length;
    
    IRIT_PT_RESET(ConeAxis);

    /* Make sure coefficients of nrmlCrv are all unit length normals.       */
    /* Also compute the average vector at the same time.		    */
    for (i = 1; i < Length; i++) {
	if (Planar) {
	    for (j = 0; j < 2; j++)
	        Tan[j] = Points[j + 1][i] - Points[j + 1][i - 1];
	    Tan[2] = 0.0;
	}
	else {
	    for (j = 0; j < 3; j++)
	        Tan[j] = Points[j + 1][i] - Points[j + 1][i - 1];
	}
	if ((t = IRIT_PT_LENGTH(Tan)) > IRIT_EPS) {
	    t = 1.0 / t;
	    IRIT_PT_SCALE(Tan, t);
	}

	IRIT_PT_ADD(ConeAxis, ConeAxis, Tan);
    }
    if ((t = IRIT_VEC_LENGTH(ConeAxis)) < IRIT_UEPS) {
	return NULL;
    }
    else {
        t = 1.0 / t;
        IRIT_PT_SCALE(ConeAxis, t);
    }

    /* Find the maximal angle between ConeAxis and the vector in mesh. */
    ConeAngle = 1.0;
    for (i = 1; i < Length; i++) {
        CagdRType InnerProd;

	if (Planar) {
	    for (j = 0; j < 2; j++)
	        Tan[j] = Points[j + 1][i] - Points[j + 1][i - 1];
	    Tan[2] = 0.0;
	}
	else {
	    for (j = 0; j < 3; j++)
	        Tan[j] = Points[j + 1][i] - Points[j + 1][i - 1];
	}
	if ((t = IRIT_VEC_LENGTH(Tan)) > IRIT_UEPS) {
	    t = 1.0 / t;
	    IRIT_VEC_SCALE(Tan, t);

	    InnerProd = IRIT_DOT_PROD(ConeAxis, Tan);

	    if (ConeAngle > InnerProd)
	        ConeAngle = InnerProd;
	}
    }
    	
    IRIT_PT_COPY(TangentCone.ConeAxis, ConeAxis);
    TangentCone.ConeAngle = acos(ConeAngle);
    return &TangentCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the maximal angle of the vectors in NrmlSrf mesh, from Axis.    *
* Assumes vectors in NrmlSrf mesh are E3 unit length (already normalized).   *
*                                                                            *
* PARAMETERS:                                                                *
*   NrmlSrf:   Normal vector field srf with all control vectors normalized.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  The maximal deviate angle in radians.                        *
*****************************************************************************/
static CagdRType SymbNormalConeMaxAngle(const CagdSrfStruct *NrmlSrf,
					const CagdVType Axis)
{
    int i,
	MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;
    CagdRType
        * const *Points = NrmlSrf -> Points;
    CagdRType const
        *XPts = Points[1],
        *YPts = Points[2],
        *ZPts = Points[3];
    CagdRType
        Angle = 1.0;

    assert(NrmlSrf -> PType == CAGD_PT_E3_TYPE);
    assert(IRIT_APX_EQ(IRIT_VEC_LENGTH(Axis), 1.0));

    for (i = 0; i < MeshSize; i++) {
    	CagdRType
    	    InnerProd = Axis[0] * XPts[i] +
	                Axis[1] * YPts[i] +
			Axis[2] * ZPts[i];

	assert(IRIT_APX_EQ(IRIT_SQR(XPts[i]) +
			   IRIT_SQR(YPts[i]) + 
			   IRIT_SQR(ZPts[i]), 1.0));

	if (Angle > InnerProd)
	    Angle = InnerProd;
    }

    return Angle > -1.0 ? acos(Angle) : M_PI;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets whether to use an optimal (but slower) algorithm to compue bounding M
* cones for normals or use a simple vector averaging that is faster.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Optimal:   New setting.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Previous setting.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrfAvg, SymbNormalConeForSrfOpt                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeForSrfDoOptimal                                            M
*****************************************************************************/
int SymbNormalConeForSrfDoOptimal(int Optimal)
{
    int OldVal = GlblSymbNormalConeForSrfOptimal;

    GlblSymbNormalConeForSrfOptimal = Optimal;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a normal cone for a given surface, by computing the normal field  M
* of the surface and deriving the angular span of this normal field by       M
* testing the angular span of all control vector in the normal field.        M
*   A normal field is searched for as "_NormalSrf" attribute in Srf or       M
* computed locally of no such attribute is found.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    To compute a normal cone for.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   const SymbNormalConeStruct *:   The computed normal cone, statically     M
*                                   allocated, or NULL if failed.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrfOpt, SymbNormalConeForSrfAvg, SymbTangentConeForCrv, M
*   SymbNormalConeForSrfDoOptimal					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeForSrf, normals, normal bound                              M
*****************************************************************************/
const SymbNormalConeStruct *SymbNormalConeForSrf(const CagdSrfStruct *Srf)
{
    if (GlblSymbNormalConeForSrfOptimal)
	return SymbNormalConeForSrfOpt(Srf);
    else
        return SymbNormalConeForSrfAvg(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a normal cone for a given surface, by computing the normal field  M
* of the surface and deriving the angular span of this normal field by       M
* testing the angular span of all control vector in the normal field.        M
*   A normal field is searched for as "_NormalSrf" attribute in Srf or       M
* computed locally of no such attribute is found.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    To compute a normal cone for.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   const SymbNormalConeStruct *:   The computed normal cone, statically     M
*                                   allocated, or NULL if failed.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrfOpt, SymbNormalConeOverlap, SymbTangentConeForCrv,   M
*   SymbNormalConeForSrfDoOptimal, SymbNormalConeForSrf			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeForSrfAvg, normals, normal bound                           M
*****************************************************************************/
const SymbNormalConeStruct *SymbNormalConeForSrfAvg(const CagdSrfStruct *Srf)
{
    IRIT_STATIC_DATA SymbNormalConeStruct
	NormalCone;
    CagdBType LocalNrmlSrf;
    int i, MeshSize;
    CagdSrfStruct
	*NrmlSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						      "_NormalSrf");
    CagdRType **Points, *XPts, *YPts, *ZPts, ConeAngle;
    CagdVType ConeAxis;

    if (NrmlSrf == NULL) {
	NrmlSrf = SymbSrfNormalSrf(Srf);
	LocalNrmlSrf = TRUE;
    }
    else
    	LocalNrmlSrf = FALSE;

    if (NrmlSrf -> PType != CAGD_PT_E3_TYPE) {
	CagdSrfStruct
	    *TSrf = CagdCoerceSrfTo(NrmlSrf, CAGD_PT_E3_TYPE, FALSE);

	if (LocalNrmlSrf)
	    CagdSrfFree(NrmlSrf);
	NrmlSrf = TSrf;
	LocalNrmlSrf = TRUE;
    }

    Points = NrmlSrf -> Points;
    XPts = Points[1];
    YPts = Points[2];
    ZPts = Points[3];

    MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;
    
    IRIT_PT_RESET(ConeAxis);

    /* Make sure coefficients of nrmlSrf are all unit length normals.       */
    /* Also compute the average vector at the same time.		    */
    for (i = 0; i < MeshSize; i++) {
	CagdRType
	    Len = sqrt(IRIT_SQR(XPts[i]) +
		       IRIT_SQR(YPts[i]) +
		       IRIT_SQR(ZPts[i]));

	if (Len != 0.0) {
	    XPts[i] /= Len;
	    YPts[i] /= Len;
	    ZPts[i] /= Len;
	}

	ConeAxis[0] += XPts[i];
	ConeAxis[1] += YPts[i];
	ConeAxis[2] += ZPts[i];
    }
    IRIT_PT_SCALE(ConeAxis, (1.0 / MeshSize));

    if (IRIT_VEC_SQR_LENGTH(ConeAxis) < IRIT_UEPS) {
	return NULL;                   /* Failed to derive the normal cone. */
    }
    IRIT_VEC_NORMALIZE(ConeAxis);

    /* Find the maximal angle between ConeAxis and the vector in mesh. */
    ConeAngle = SymbNormalConeMaxAngle(NrmlSrf, ConeAxis);

    if (LocalNrmlSrf)
	CagdSrfFree(NrmlSrf);
    	
    IRIT_PT_COPY(NormalCone.ConeAxis, ConeAxis);
    NormalCone.ConeAngle = ConeAngle;
    return &NormalCone;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the optimal normal cone for a given surface, using linear prog,   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    To compute a normal cone for.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   const SymbNormalConeStruct *:   The computed normal cone, statically     M
*                                   allocated, or NULL if failed.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrfAvg, GMMinSpanCone, SymbNormalConeForSrfDoOptimal    M
*   SymbNormalConeForSrf						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeForSrfOpt, normals, normal bound                           M
*****************************************************************************/
const SymbNormalConeStruct *SymbNormalConeForSrfOpt(const CagdSrfStruct *Srf)
{
    IRIT_STATIC_DATA SymbNormalConeStruct
	NormalCone;
    CagdBType LocalNrmlSrf;
    int i, MeshSize;
    CagdSrfStruct
	*NrmlSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						      "_NormalSrf");
    CagdRType **Points, *XPts, *YPts, *ZPts;
    IrtVecType *DTVecs;

    if (NrmlSrf == NULL) {
	NrmlSrf = SymbSrfNormalSrf(Srf);
	LocalNrmlSrf = TRUE;
    }
    else
    	LocalNrmlSrf = FALSE;
    MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;
    
    if (NrmlSrf -> PType != CAGD_PT_E3_TYPE) {
	CagdSrfStruct
	    *TSrf = CagdCoerceSrfTo(NrmlSrf, CAGD_PT_E3_TYPE, FALSE);

	if (LocalNrmlSrf)
	    CagdSrfFree(NrmlSrf);
	NrmlSrf = TSrf;
	LocalNrmlSrf = TRUE;
    }

    Points = NrmlSrf -> Points;
    XPts = Points[1];
    YPts = Points[2];
    ZPts = Points[3];

    /* Mape all vectors to a linear array. */
    DTVecs = (IrtVecType *) IritMalloc(sizeof(IrtVecType) * MeshSize);
    for (i = 0; i < MeshSize; i++) {
        DTVecs[i][0] = XPts[i];
        DTVecs[i][1] = YPts[i];
        DTVecs[i][2] = ZPts[i];
    }

    if (!GMMinSpanCone(DTVecs, FALSE, MeshSize, NormalCone.ConeAxis,
		       &NormalCone.ConeAngle)) {
        /* Failed to compute the optimal cone. */
        IritFree(DTVecs);
	return NULL;
    }

    IritFree(DTVecs);

    return &NormalCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as SymbNormalConeForSrf but also estimates a main axis (principal     M
* component) for the cone 						     M
*   A normal field is searched for as "_NormalSrf" attribute in Srf or       M
* computed locally of no such attribute is found.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute normal cone and main axis of normal cone for.     M
*   MainAxis:   Main axis (principal component) of the normal cone's         M
*               vectors distribution. 					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   const SymbNormalConeStruct *:   The computed normal cone, statically     M
*                                   allocated, or NULL if failed.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrf, SymbNormalConeOverlap, SymbTangentConeForCrv       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeForSrfMainAxis, normals, normal bound                      M
*****************************************************************************/
const SymbNormalConeStruct *SymbNormalConeForSrfMainAxis(
						  const CagdSrfStruct *Srf,
						  CagdVType MainAxis)
{
    int i, MeshSize;
    CagdRType **Points, Res;
    CagdPType LinePos;
    CagdPtStruct *PtList;
    const SymbNormalConeStruct *Cone;
    CagdSrfStruct *TSrf,
	*NrmlSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						      "_NormalSrf");

    if (NrmlSrf == NULL) {
	NrmlSrf = SymbSrfNormalSrf(Srf);

	if (NrmlSrf -> PType != CAGD_PT_E3_TYPE) {
	    TSrf = CagdCoerceSrfTo(NrmlSrf, CAGD_PT_E3_TYPE, FALSE);
	    CagdSrfFree(NrmlSrf);
	    NrmlSrf = TSrf;
	}

	/* Force a new "_NormalSrf" attr on a const object... */
	AttrSetPtrAttrib(&((CagdSrfStruct *) Srf) -> Attr, "_NormalSrf",
			 NrmlSrf);
    }

    if ((Cone = SymbNormalConeForSrf(Srf)) == NULL) {/* Compute normal cone. */
        return NULL;
    }

    /* Fit a line to all vectors in NrmlSrf. */
    Points = NrmlSrf -> Points;
    MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;
    for (i = 0, PtList = NULL; i < MeshSize; i++) {
        CagdPtStruct
	    *Pt = CagdPtNew();

	Pt -> Pt[0] = Points[1][i];
	Pt -> Pt[1] = Points[2][i];
	Pt -> Pt[2] = Points[3][i];
	IRIT_LIST_PUSH(Pt, PtList);
    }

    Res = CagdLineFitToPts(PtList, MainAxis, LinePos);
    CagdPtFreeList(PtList);

    if (Res == IRIT_INFNTY) {

        return NULL;
    }
    else
        return Cone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a 2cones bound to the normal field of surface Srf.  The 2cones    M
* bound the normal field in the common intersection space.		     M
*  The 2cones are computed using the regular normal cone by expanding in the M
* direction orthogonal to the cone axis and its main principal component.    M
*  The expansion is done an amount that is equal to regular cone radius      M
* times ExpandingFactor.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             To compute the normal 2cones for.	                     M
*   ExpandingFactor: Factor to expand placement of 2cones axes locations.    M
*   Cone1, Cone2:    The two cones to compute or ConeAngle == M_PI if error. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrf, SymbNormalConeOverlap, SymbTangentConeForCrv       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormal2ConesForSrf, normals, normal bound	                     M
*****************************************************************************/
int SymbNormal2ConesForSrf(const CagdSrfStruct *Srf,
			   CagdRType ExpandingFactor,
			   SymbNormalConeStruct *Cone1,
			   SymbNormalConeStruct *Cone2)
{
    int i, MeshSize;
    CagdRType **Points;
    CagdVType MainAxis, Dir, V;
    CagdSrfStruct *NrmlSrf;
    SymbNormalConeStruct NormalCone1, NormalCone2;
    const SymbNormalConeStruct
        *Cone = SymbNormalConeForSrfMainAxis(Srf, MainAxis);

    if (Cone == NULL || Cone -> ConeAngle >= M_PI / 2.0) {
        Cone1 -> ConeAngle = Cone2 -> ConeAngle = M_PI;
	return FALSE;
    }

    IRIT_CROSS_PROD(Dir, Cone -> ConeAxis, MainAxis);
    IRIT_VEC_NORMALIZE(Dir);
    ExpandingFactor *= tan(Cone -> ConeAngle);
    IRIT_VEC_SCALE2(V, Dir, ExpandingFactor);

    IRIT_PT_ADD(NormalCone1.ConeAxis, Cone -> ConeAxis, V);
    IRIT_PT_SUB(NormalCone2.ConeAxis, Cone -> ConeAxis, V);
    IRIT_VEC_NORMALIZE(NormalCone1.ConeAxis);
    IRIT_VEC_NORMALIZE(NormalCone2.ConeAxis);

    /* Now that the 2cones axes are set, figure out the needed radii by    */
    /* finding the maximal angle between ConeAxis and the vector in mesh.  */
    NrmlSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr, "_NormalSrf");
    assert(NrmlSrf != NULL);

    /* Make sure NrmlSrf is an E3 srf of unit vectors only. */
    assert(NrmlSrf -> PType == CAGD_PT_E3_TYPE);
    MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;
    Points = NrmlSrf -> Points;
    for (i = 0; i < MeshSize; i++) {
	CagdRType
	    Len = sqrt(IRIT_SQR(Points[1][i]) +
		       IRIT_SQR(Points[2][i]) +
		       IRIT_SQR(Points[3][i]));

	if (Len != 0.0) {
	    Points[1][i] /= Len;
	    Points[2][i] /= Len;
	    Points[3][i] /= Len;
	}
    }

    NormalCone1.ConeAngle = SymbNormalConeMaxAngle(NrmlSrf,
						   NormalCone1.ConeAxis);
    NormalCone2.ConeAngle = SymbNormalConeMaxAngle(NrmlSrf,
						   NormalCone2.ConeAxis);

    *Cone1 = NormalCone1;
    *Cone2 = NormalCone2;

    return TRUE;
}
 
/*****************************************************************************
* DESCRIPTION:                                                               M
* Tests if the given two normal cones overlap or not.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalCone1, NormalCone2:  The two normal cones to test for angular      M
		overlap.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if overlap, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeOverlap                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConeOverlap, normals, normal bound                             M
*****************************************************************************/
CagdBType SymbNormalConeOverlap(const SymbNormalConeStruct *NormalCone1,
				const SymbNormalConeStruct *NormalCone2)
{
    CagdRType
    	Angle = acos(IRIT_DOT_PROD(NormalCone1 -> ConeAxis,
			           NormalCone2 -> ConeAxis));

    return Angle < NormalCone1 -> ConeAngle + NormalCone2 -> ConeAngle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the convex conical hull of the normal field of surface Srf.     M
*   The result is returned as an array of subset of vectors from the field.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:	To compute the conical hull for.	                     M
*   CH:		The vectors of the convex conical hull to compute.           M
*               CH[i][j] is the i'th coordinate of the j'th vector.          M
*   NPts:       Contain the number of vectors in CH.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbNormalConeStruct *:       A circular normal cone as a by product.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConvexHullConeOverlap                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConvexHullConeForSrf                                           M
*****************************************************************************/
SymbNormalConeStruct *SymbNormalConvexHullConeForSrf(const CagdSrfStruct *Srf,
						     CagdRType ***CH,
						     int *NPts)
{
    CagdBType LocalNrmlSrf;
    int i, MeshSize;
    CagdSrfStruct
        *NrmlSrf = (CagdSrfStruct *) AttrGetPtrAttrib(Srf -> Attr,
						      "_NormalSrf");
    CagdRType **Points, *XPts, *YPts, *ZPts, ConeAngle, Ax1[3], Ax2[3],
        Xdir[3] = { 1, 0, 0 };
    CagdVType ConeAxis;
    SymbNormalConeStruct
        *NormalCone = (SymbNormalConeStruct *)
                                   IritMalloc(sizeof(SymbNormalConeStruct));
    IrtE2PtStruct *DTPts;

    if (NrmlSrf == NULL) {
        NrmlSrf = SymbSrfNormalSrf(Srf);
        LocalNrmlSrf = TRUE;
    }
    else
        LocalNrmlSrf = FALSE;
    if (NrmlSrf -> PType != CAGD_PT_E3_TYPE) {
        CagdSrfStruct
            *TSrf = CagdCoerceSrfTo(NrmlSrf, CAGD_PT_E3_TYPE, FALSE);

        if (LocalNrmlSrf)
            CagdSrfFree(NrmlSrf);
        NrmlSrf = TSrf;
        LocalNrmlSrf = TRUE;
    }

    Points = NrmlSrf -> Points;
    XPts = Points[1];
    YPts = Points[2];
    ZPts = Points[3];

    MeshSize = NrmlSrf -> ULength * NrmlSrf -> VLength;

    IRIT_PT_RESET(ConeAxis);

    /* Make sure coefficients of nrmlSrf are all unit length normals.       */
    /* Also compute the average vector at the same time.	            */
    for (i = 0; i < MeshSize; i++) {
        CagdRType
            Len = sqrt(IRIT_SQR(XPts[i]) +
            IRIT_SQR(YPts[i]) +
            IRIT_SQR(ZPts[i]));

        if (Len != 0.0) {
            XPts[i] /= Len;
            YPts[i] /= Len;
            ZPts[i] /= Len;
        }

        ConeAxis[0] += XPts[i];
        ConeAxis[1] += YPts[i];
        ConeAxis[2] += ZPts[i];
    }
    IRIT_PT_SCALE(ConeAxis, (1.0 / MeshSize));

    /* Find the maximal angle between ConeAxis and the vector in mesh. */
    ConeAngle = 1.0;
    for (i = 0; i < MeshSize; i++) {
        CagdRType
            InnerProd = ConeAxis[0] * XPts[i] +
            ConeAxis[1] * YPts[i] +
            ConeAxis[2] * ZPts[i];

        if (ConeAngle > InnerProd)
            ConeAngle = InnerProd;
    }


    IRIT_PT_NORMALIZE(ConeAxis);
    IRIT_PT_COPY(NormalCone -> ConeAxis, ConeAxis);
    NormalCone -> ConeAngle = acos(ConeAngle);

    /* project normals to plane orthogonal to ConeAxis: */
    IRIT_CROSS_PROD(Ax1, ConeAxis, Xdir);
    IRIT_PT_NORMALIZE(Ax1);
    IRIT_CROSS_PROD(Ax2, ConeAxis, Ax1);
    IRIT_PT_NORMALIZE(Ax2);
    DTPts = (IrtE2PtStruct *) IritMalloc(MeshSize * sizeof(IrtE2PtStruct));
    for (i = 0; i < MeshSize; i++) {
        DTPts[i].Pt[0] = Ax1[0] * XPts[i] + Ax1[1] * YPts[i] +
	                                                    Ax1[2] * ZPts[i];
        DTPts[i].Pt[1] = Ax2[0] * XPts[i] + Ax2[1] * YPts[i] +
	                                                    Ax2[2] * ZPts[i];
    }
    if (LocalNrmlSrf)
        CagdSrfFree(NrmlSrf);
    *NPts = MeshSize;

    GMConvexHull(DTPts, NPts);

    /* Unproject to points back to the sphere. */
    Points = (CagdRType **) IritMalloc(3 * sizeof(CagdRType *));
    XPts = (CagdRType *) IritMalloc(*NPts * sizeof(CagdRType));
    Points[0] = XPts;
    YPts = (CagdRType *) IritMalloc(*NPts * sizeof(CagdRType));
    Points[1] = YPts;
    ZPts = (CagdRType *) IritMalloc(*NPts * sizeof(CagdRType));
    Points[2] = ZPts;

    for (i = 0; i < *NPts; i++) {
        CagdRType
	    Pt3 = sqrt(1 - IRIT_SQR(DTPts[i].Pt[0]) -
		           IRIT_SQR(DTPts[i].Pt[1]));

        XPts[i] = DTPts[i].Pt[0] * Ax1[0] + DTPts[i].Pt[1] * Ax2[0] +
	                                                   Pt3 * ConeAxis[0];
        YPts[i] = DTPts[i].Pt[0] * Ax1[1] + DTPts[i].Pt[1] * Ax2[1] +
	                                                   Pt3 * ConeAxis[1];
        ZPts[i] = DTPts[i].Pt[0] * Ax1[2] + DTPts[i].Pt[1] * Ax2[2] +
	                                                   Pt3 * ConeAxis[2];
    }

    *CH = Points;
    IritFree(DTPts);

    return NormalCone;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
* Tests if the given two convex conical hulls overlap or not.	   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   NormalCone1: The normal cone of the first surface.                       M
*   CH1:         The convex conical hull of the first surface.               M
*   NPts1:       Number of points in the convex conical of the first surface.M
*   NormalCone2: The normal cone of the second surface.                      M
*   CH2:	 The convexc onical hull of the second surface.              M
*   NPts2:	 Number of point in the convex conical of the second surface.M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if overlap, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConvexHullConeForSrf                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbNormalConvexHullConeOverlap                                          M
*****************************************************************************/
CagdBType SymbNormalConvexHullConeOverlap(const SymbNormalConeStruct
					                        *NormalCone1,
					  const CagdRType **CH1,
					  int NPts1,
					  const SymbNormalConeStruct
					                        *NormalCone2,
					  const CagdRType **CH2,
					  int NPts2)
{
    int i, j, Ip1, Jp1;
    CagdRType
        Angle = acos(IRIT_DOT_PROD(NormalCone1 -> ConeAxis,
				   NormalCone2 -> ConeAxis));
    CagdRType Det, DetX;

    if (Angle > NormalCone1 -> ConeAngle + NormalCone2 -> ConeAngle)
        return TRUE;

    for (i = 0; i < NPts1; i++) {
        if (i < NPts1 - 1) 
            Ip1 = i + 1;
        else 
            Ip1 = 0;

        for(j = 0; j < NPts2; j++) {
            if (j < NPts2 - 1) 
                Jp1 = j + 1;
            else 
                Jp1 = 0;

            /* Det of {CH2[][j], CH2[][j+1], -CH1[][ip1]}. */
            Det = Determinant3x3( CH2[0][j],    CH2[1][j],    CH2[2][j],
			          CH2[0][Jp1],  CH2[1][Jp1],  CH2[2][Jp1],
			         -CH1[0][Ip1], -CH1[1][Ip1], -CH2[2][Ip1]);

            /* Det of {CH1[][i], CH2[][j+1], -CH1[][Ip1]}. */
            DetX = Determinant3x3( CH1[0][i],    CH1[1][i],    CH1[2][i],
			           CH2[0][Jp1],  CH2[1][Jp1],  CH2[2][Jp1],
			          -CH1[0][Ip1], -CH1[1][Ip1], -CH2[2][Ip1]);

            if (IRIT_SIGN(Det) != IRIT_SIGN(DetX))
                continue;

            /* Det of {CH2[][j], CH1[][i], -CH1[][Ip1]}. */
            DetX = Determinant3x3( CH2[0][j],    CH2[1][j],    CH2[2][j],
			           CH1[0][i],    CH1[1][i],    CH1[2][i],
			          -CH1[0][Ip1], -CH1[1][Ip1], -CH2[2][Ip1]);

            if (IRIT_SIGN(Det) != IRIT_SIGN(DetX))
                continue;

            /* Det of {CH2[][j], CH2[][j+1i], CH1[][i]}. */
            Det = Determinant3x3(CH2[0][j],   CH2[1][j],   CH2[2][j],
			         CH2[0][Jp1], CH2[1][Jp1], CH2[2][Jp1],
			         CH1[0][i],   CH1[1][i],   CH1[2][i]);
            if (IRIT_SIGN(Det) != IRIT_SIGN(DetX))
                continue;

            return TRUE;
        }
    }

    /* No intersection. Check if one one cone is contained in the other. */
    for (i = 0; i < NPts1; i++) {
        if (i < NPts1 - 1) 
	    Ip1 = i + 1;
	else 
	    Ip1 = 0;
	Det = Determinant3x3(CH2[0][0],     CH2[1][0],    CH2[2][0],
			     CH1[0][i],     CH1[1][i],    CH1[2][i],
			     CH1[0][Ip1],   CH1[1][Ip1],  CH1[2][Ip1]);
	if (Det > 0)
	    break;
    }
	
    if (i == NPts1)
        return TRUE;

    for (i = 0; i < NPts2; i++) {
        if (i < NPts1 - 1) 
	    Ip1 = i + 1;
	else 
	    Ip1 = 0;
	Det = Determinant3x3(CH1[0][0],    CH1[1][0],    CH1[2][0],
			     CH2[0][i],    CH2[1][i],    CH2[2][i],
			     CH2[0][Ip1],  CH2[1][Ip1],  CH2[2][Ip1]);
	if (Det > 0)
	    break;
    }
    
    if (i == NPts1)
        return TRUE;
	
    return FALSE;
}
