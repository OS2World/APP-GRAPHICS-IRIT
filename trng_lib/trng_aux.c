/******************************************************************************
* Trng_aux.c - auxiliary routine to interface to tringular surface rep.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aux. 96.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a triangular surface, returns its parametric domain.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:       Triangular surface function to consider.                   M
*   UMin, UMax:   U Domain of TriSrf will be placed herein.                  M
*   VMin, VMax:   V Domain of TriSrf will be placed herein.                  M
*   WMin, WMax:   W Domain of TriSrf will be placed herein.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfDomain, triangular surfaces                                    M
*****************************************************************************/
void TrngTriSrfDomain(const TrngTriangSrfStruct *TriSrf,
		      CagdRType *UMin,
		      CagdRType *UMax,
		      CagdRType *VMin,
		      CagdRType *VMax,
		      CagdRType *WMin,
		      CagdRType *WMax)
{
    int Order, Len;

    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	case TRNG_TRISRF_GREGORY_TYPE:
	    *UMin = 0.0;
	    *UMax = 1.0;
	    *VMin = 0.0;
	    *VMax = 1.0;
	    *WMin = 0.0;
	    *WMax = 1.0;
	    break;
	case TRNG_TRISRF_BSPLINE_TYPE:
	    Order = TriSrf -> Order;
	    Len = TriSrf -> Length;

	    *UMin = TriSrf -> KnotVector[Order - 1];
	    *UMax = TriSrf -> KnotVector[Len];
	    *VMin = TriSrf -> KnotVector[Order - 1];
	    *VMax = TriSrf -> KnotVector[Len];
	    *WMin = TriSrf -> KnotVector[Order - 1];
	    *WMax = TriSrf -> KnotVector[Len];
	    break;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a triangular surface and a domain - validate it.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To make sure t is in its Dir domain.	                     M
*   t:        Parameter value to verify.                                     M
*   Dir:      Direction. Either U or V or W.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if in domain, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngParamInDomain, triangular surfaces                                   M
*****************************************************************************/
CagdBType TrngParamInDomain(TrngTriangSrfStruct *TriSrf,
			    CagdRType t,
			    TrngTriSrfDirType Dir)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;

    TrngTriSrfDomain(TriSrf, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    switch (Dir) {
	case TRNG_CONST_U_DIR:
	    return t >= UMin && t <= UMax;
	case TRNG_CONST_V_DIR:
	    return t >= VMin && t <= VMax;
	case TRNG_CONST_W_DIR:
	    return t >= WMin && t <= WMax;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_WRONG_DOMAIN);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a triangular surface and a domain - validate it.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To make sure (u, v, w) is in its domain.	                     M
*   u, v, w:  To verify if it is in TriSrf's parametric domain.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if in domain, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngParamsInDomain, triangular surfaces                                  M
*****************************************************************************/
CagdBType TrngParamsInDomain(const TrngTriangSrfStruct *TriSrf,
			     CagdRType u,
			     CagdRType v,
			     CagdRType w)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;

    TrngTriSrfDomain(TriSrf, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    return u >= UMin && u <= UMax &&
           v >= VMin && v <= VMax &&
	   w >= WMin && w <= WMax;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a triangular surfaces.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfBBox, bbox, bounding box                                       M
*****************************************************************************/
void TrngTriSrfBBox(const TrngTriangSrfStruct *TriSrf, CagdBBoxStruct *BBox)
{
    CagdPointsBBox(TriSrf -> Points, TRNG_TRISRF_MESH_SIZE(TriSrf),
		   3, BBox -> Min, BBox -> Max);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of triangular surfaces.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:  To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfListBBox, bbox, bounding box                                   M
*****************************************************************************/
void TrngTriSrfListBBox(const TrngTriangSrfStruct *TriSrfs, CagdBBoxStruct *BBox)
{
    CAGD_RESET_BBOX(BBox);

    for ( ; TriSrfs != NULL; TriSrfs = TriSrfs -> Pnext) {
	CagdBBoxStruct TmpBBox;

	TrngTriSrfBBox(TriSrfs, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given triangular Bspline surface has open end	     M
* coditions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To check for open end conditions.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if given Bspline triangular surface has open end	     M
*		 conditions, FALSE otherwise.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfHasOpenEC, open end conditions                              M
*****************************************************************************/
CagdBType TrngBspTriSrfHasOpenEC(const TrngTriangSrfStruct *TriSrf)
{
    return BspKnotHasOpenEC(TriSrf -> KnotVector, TriSrf -> Length,
			    TriSrf -> Order);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given triangular Bspline surface has open end	     M
* coditions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To check for open end conditions.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: A triangular Bspline surface with open end cond.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfOpenEnd, open end conditions                                M
*****************************************************************************/
TrngTriangSrfStruct *TrngBspTriSrfOpenEnd(const TrngTriangSrfStruct *TriSrf)
{
    TRNG_FATAL_ERROR(TRNG_ERR_BSPLINE_NO_SUPPORT);
    return NULL;
}
