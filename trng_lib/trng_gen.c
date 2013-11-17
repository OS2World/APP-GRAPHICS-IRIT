/******************************************************************************
* Trng_gen.c - General routines used by all modules of trng_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "trng_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new triangular surface.                M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the curve should be - Bspline, Bezier etc.  M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   Length:     Number of control points along the edge of the triangle.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:    An uninitialized freeform triangular surface.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfNew, triangular surfaces, allocation                           M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfNew(TrngGeomType GType,
				   CagdPointType PType,
				   int Length)
{
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);
    TrngTriangSrfStruct
	*NewTriSrf =
	    (TrngTriangSrfStruct *) IritMalloc(sizeof(TrngTriangSrfStruct));

    NewTriSrf -> GType = GType;
    NewTriSrf -> PType = PType;
    NewTriSrf -> Length = Length;
    NewTriSrf -> KnotVector = NULL;
    NewTriSrf -> Pnext = NULL;
    NewTriSrf -> Attr = NULL;
    NewTriSrf -> Points[0] = NULL;		    /* The rational element. */

    for (i = !CAGD_IS_RATIONAL_PT(PType); i <= MaxAxis; i++)
	NewTriSrf -> Points[i] = (CagdRType *)
	    IritMalloc(sizeof(CagdRType) * TRNG_TRISRF_MESH_SIZE(NewTriSrf));

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewTriSrf -> Points[i] = NULL;

    return NewTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bspline triangular surface.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points along the edge of the triangle.     M
*   Order:      Order of triangular surface in all U,V,W directions.         M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:    An uninitialized freeform triangular surface   M
*			   Bspline.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfNew, triangular surfaces, allocation                        M
*****************************************************************************/
TrngTriangSrfStruct *TrngBspTriSrfNew(int Length, 
				      int Order,
				      CagdPointType PType)
{
    TrngTriangSrfStruct *TriSrf;

    if (Length < Order) {
	TRNG_FATAL_ERROR(TRNG_ERR_WRONG_ORDER);
	return NULL;
    }

    TriSrf = TrngTriSrfNew(TRNG_TRISRF_BSPLINE_TYPE, PType, Length);

    TriSrf -> KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
                                                           (Order + Length));
    TriSrf -> Order = Order;

    return TriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bezier triangular surface.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points along the edge of the triangle.     M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:    An uninitialized freeform triangular surface   M
*			   Bezier.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfNew, triangular surfaces, allocation                        M
*****************************************************************************/
TrngTriangSrfStruct *TrngBzrTriSrfNew(int Length, CagdPointType PType)
{
    TrngTriangSrfStruct
	*TriSrf = TrngTriSrfNew(TRNG_TRISRF_BEZIER_TYPE, PType, Length);

    TriSrf -> Order = TriSrf -> Length = Length;

    TriSrf -> KnotVector = NULL;

    return TriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Gregory triangular surface.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points along the edge of the triangle.     M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:    An uninitialized freeform triangular surface   M
*			   Gregory.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGrgTriSrfNew, triangular surfaces, allocation                        M
*****************************************************************************/
TrngTriangSrfStruct *TrngGrgTriSrfNew(int Length, CagdPointType PType)
{
    TrngTriangSrfStruct
	*TriSrf = TrngTriSrfNew(TRNG_TRISRF_GREGORY_TYPE, PType, Length);

    TriSrf -> Order = TriSrf -> Length = Length;

    TriSrf -> KnotVector = NULL;

    return TriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a triangular surface structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:        triangular surface to duplicate                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:    Duplicated triangular surface.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfCopy, triangular surfaces                                      M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfCopy(const TrngTriangSrfStruct *TriSrf)
{
    int i, Len,
	MaxAxis = CAGD_NUM_OF_PT_COORD(TriSrf -> PType);
    TrngTriangSrfStruct
	*NewTriSrf =
	    (TrngTriangSrfStruct *) IritMalloc(sizeof(TrngTriangSrfStruct));

    NewTriSrf -> GType = TriSrf -> GType;
    NewTriSrf -> PType = TriSrf -> PType;
    NewTriSrf -> Length = TriSrf -> Length;
    NewTriSrf -> Order = TriSrf -> Order;
    if (TriSrf -> KnotVector != NULL)
	NewTriSrf -> KnotVector = BspKnotCopy(NULL, TriSrf -> KnotVector,
					   TriSrf -> Length + TriSrf -> Order);
    else
	NewTriSrf -> KnotVector = NULL;
    NewTriSrf -> Pnext = NULL;
    NewTriSrf -> Attr = NULL;
    NewTriSrf -> Points[0] = NULL;		    /* The rational element. */

    Len = TRNG_TRISRF_MESH_SIZE(TriSrf);
    for (i = !TRNG_IS_RATIONAL_TRISRF(TriSrf); i <= MaxAxis; i++) {
	NewTriSrf -> Points[i] =
	    (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);
	CAGD_GEN_COPY(NewTriSrf -> Points[i], TriSrf -> Points[i],
		      sizeof(CagdRType) * Len);
    }

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewTriSrf -> Points[i] = NULL;

    return NewTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of triangular surface structures.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfList:    List of triangular surfaces to duplicate.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:  Duplicated list of triangular surfaces.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfCopyList, triangular surfaces                                  M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfCopyList(const TrngTriangSrfStruct *TriSrfList)
{
    TrngTriangSrfStruct *TriSrfTemp, *NewTriSrfList;

    if (TriSrfList == NULL)
	return NULL;
    TriSrfTemp = NewTriSrfList = TrngTriSrfCopy(TriSrfList);
    TriSrfList = TriSrfList -> Pnext;
    while (TriSrfList) {
	TriSrfTemp -> Pnext = TrngTriSrfCopy(TriSrfList);
	TriSrfTemp = TriSrfTemp -> Pnext;
	TriSrfList = TriSrfList -> Pnext;
    }
    return NewTriSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a triangular surface structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:     triangular surface to free.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfFree, triangular surfaces                                      M
*****************************************************************************/
void TrngTriSrfFree(TrngTriangSrfStruct *TriSrf)
{
    int i, MaxAxis;

    if (TriSrf == NULL)
	return;

    MaxAxis = CAGD_NUM_OF_PT_COORD(TriSrf -> PType);

    for (i = !TRNG_IS_RATIONAL_TRISRF(TriSrf); i <= MaxAxis; i++)
	IritFree(TriSrf -> Points[i]);

    if (TriSrf -> KnotVector != NULL)
	IritFree(TriSrf -> KnotVector);

    IP_ATTR_FREE_ATTRS(TriSrf -> Attr);
    IritFree(TriSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of triangular surface structures.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfList:    triangular surface list to free.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfFreeList, triangular surfaces                                  M
*****************************************************************************/
void TrngTriSrfFreeList(TrngTriangSrfStruct *TriSrfList)
{
    TrngTriangSrfStruct *TriSrfTemp;

    while (TriSrfList) {
	TriSrfTemp = TriSrfList -> Pnext;
	TrngTriSrfFree(TriSrfList);
	TriSrfList = TriSrfTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Linearly transforms, in place, given TriSrf as specified by Translate and  M
* Scale.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:        Triangular surface to transform.                          M
*   Translate:     Translation factor.                                       M
*   Scale:         Scaling factor.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfTransform, triangular surfaces                                 M
*****************************************************************************/
void TrngTriSrfTransform(TrngTriangSrfStruct *TriSrf,
			 CagdRType *Translate,
			 CagdRType Scale)
{
    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	case TRNG_TRISRF_BSPLINE_TYPE:
	case TRNG_TRISRF_GREGORY_TYPE:
	    CagdTransform(TriSrf -> Points,
	    		  TRNG_TRISRF_MESH_SIZE(TriSrf),
	                  CAGD_NUM_OF_PT_COORD(TriSrf -> PType),
			  !TRNG_IS_RATIONAL_TRISRF(TriSrf),
		          Translate,
        	          Scale);
	    break;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms, in place, the given TV as specified by homogeneous matrix Mat. M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:        Triangular surface to transform.                          M
*   Mat:           Homogeneous transformation to apply to TriSrf.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfMatTransform, triangular surfaces                              M
*****************************************************************************/
void TrngTriSrfMatTransform(TrngTriangSrfStruct *TriSrf, CagdMType Mat)
{
    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	case TRNG_TRISRF_BSPLINE_TYPE:
	case TRNG_TRISRF_GREGORY_TYPE:
	    CagdMatTransform(TriSrf -> Points,
    			     TRNG_TRISRF_MESH_SIZE(TriSrf),
        	             CAGD_NUM_OF_PT_COORD(TriSrf -> PType),
			     !TRNG_IS_RATIONAL_TRISRF(TriSrf),
		             Mat);
	    break;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier triangular surface into a Bspline triangular surface by  M
* adding open end uniform knot vector to it.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   A Bezier triangular surface to convert to a Bspline TriSrf.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:  A Bspline triangular surface representing the    M
*			 same geometry as the given Bezier TriSrf.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngCnvrtBzr2BspTriSrf, conversion, triangular surface                   M
*****************************************************************************/
TrngTriangSrfStruct *TrngCnvrtBzr2BspTriSrf(const TrngTriangSrfStruct *TriSrf)
{
    TrngTriangSrfStruct *BspTriSrf;

    if (TriSrf -> GType != TRNG_TRISRF_BEZIER_TYPE) {
        TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
        return NULL;
    }

    BspTriSrf = TrngTriSrfCopy(TriSrf);

    BspTriSrf -> Order = BspTriSrf -> Length;
    BspTriSrf -> KnotVector = BspKnotUniformOpen(BspTriSrf -> Length,
					         BspTriSrf -> Order, NULL);
    BspTriSrf -> GType = TRNG_TRISRF_BSPLINE_TYPE;
    return BspTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two surfaces for similarity.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:     The two surfaces to compare.                             M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if surfaces are the same, FALSE otehrwise.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfsSame				                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfsSame                                                          M
*****************************************************************************/
CagdBType TrngTriSrfsSame(const TrngTriangSrfStruct *Srf1,
			  const TrngTriangSrfStruct *Srf2,
			  CagdRType Eps)
{
    do {
        if (Srf1 -> PType != Srf2 -> PType ||
	    Srf1 -> GType != Srf2 -> GType ||
	    Srf1 -> Order != Srf2 -> Order ||
	    Srf1 -> Length != Srf2 -> Length)
	    return FALSE;

	if (!CagdCtlMeshsSame(Srf1 -> Points, Srf2 -> Points,
			      TRNG_TRISRF_MESH_SIZE(Srf1), Eps))
	    return FALSE;

	if (Srf1 -> KnotVector != NULL && Srf2 -> KnotVector != NULL &&
	    !BspKnotVectorsSame(Srf1 -> KnotVector, Srf2 -> KnotVector,
				Srf1 -> Length + Srf1 -> Order, Eps))
	    return FALSE;

	Srf1 = Srf1 -> Pnext;
	Srf2 = Srf2 -> Pnext;
    }
    while (Srf1 != NULL && Srf2 != NULL);

    return Srf1 == NULL && Srf2 == NULL;
}
