/******************************************************************************
* Triv_gen.c - General routines used by all modules of TRIV_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "triv_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

#ifdef DEBUG
#undef TrivTVFree
#undef TrivTVFreeList
#undef TrivTriangleFree
#undef TrivTriangleFreeList
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new trivariate.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   GType:      Type of geometry the curve should be - Bspline, Bezier etc.  M
*   PType:      Type of control points (E2, P3, etc.).                       M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*   WLength:    Number of control points in the W direction.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    An uninitialized freeform trivariate.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVNew, TrivBspTVNew, TrivPwrTVNew                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVNew, trivariates, allocation                                       M
*****************************************************************************/
TrivTVStruct *TrivTVNew(TrivGeomType GType,
			CagdPointType PType,
			int ULength,
			int VLength,
			int WLength)
{
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(PType);
    TrivTVStruct
	*NewTV = (TrivTVStruct *) IritMalloc(sizeof(TrivTVStruct));

    NewTV -> GType = GType;
    NewTV -> PType = PType;
    NewTV -> ULength = ULength;
    NewTV -> VLength = VLength;
    NewTV -> WLength = WLength;
    NewTV -> UVPlane = ULength * VLength;
    NewTV -> UOrder = 0;
    NewTV -> VOrder = 0;
    NewTV -> WOrder = 0;
    NewTV -> UKnotVector = NULL;
    NewTV -> VKnotVector = NULL;
    NewTV -> WKnotVector = NULL;
    NewTV -> UPeriodic = FALSE;
    NewTV -> VPeriodic = FALSE;
    NewTV -> WPeriodic = FALSE;
    NewTV -> Pnext = NULL;
    NewTV -> Attr = NULL;
    NewTV -> Points[0] = NULL;			    /* The rational element. */

    for (i = !CAGD_IS_RATIONAL_PT(PType); i <= MaxAxis; i++)
	NewTV -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) *
						ULength * VLength * WLength);

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewTV -> Points[i] = NULL;

    return NewTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bspline trivariate.                M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*   WLength:    Number of control points in the W direction.                 M
*   UOrder:     Order of trivariate in the U direction.			     M
*   VOrder:     Order of trivariate in the V direction.			     M
*   WOrder:     Order of trivariate in the W direction.			     M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    An uninitialized freeform trivariate Bspline.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVNew, TrivTVNew, TrivPwrTVNew                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVNew, trivariates, allocation                                    M
*****************************************************************************/
TrivTVStruct *TrivBspTVNew(int ULength, 
			   int VLength,
			   int WLength,
			   int UOrder,
			   int VOrder,
			   int WOrder,
			   CagdPointType PType)
{
    TrivTVStruct *TV;

    if (ULength < UOrder || VLength < VOrder || WLength < WOrder) {
	TRIV_FATAL_ERROR(TRIV_ERR_WRONG_ORDER);
	return NULL;
    }

    TV = TrivTVNew(TRIV_TVBSPLINE_TYPE, PType, ULength, VLength, WLength);

    TV -> UKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
                                                           (UOrder + ULength));
    TV -> VKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
                                                           (VOrder + VLength));
    TV -> WKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
                                                           (WOrder + WLength));

    TV -> UOrder = UOrder;
    TV -> VOrder = VOrder;
    TV -> WOrder = WOrder;

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bezier trivariate.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*   WLength:    Number of control points in the W direction.                 M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    An uninitialized freeform trivariate Bezier.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVNew, TrivBspTVNew, TrivPwrTVNew                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVNew, trivariates, allocation                                    M
*****************************************************************************/
TrivTVStruct *TrivBzrTVNew(int ULength,
			   int VLength,
			   int WLength,
			   CagdPointType PType)
{
    TrivTVStruct
	*TV = TrivTVNew(TRIV_TVBEZIER_TYPE, PType, ULength, VLength, WLength);

    TV -> UOrder = TV -> ULength = ULength;
    TV -> VOrder = TV -> VLength = VLength;
    TV -> WOrder = TV -> WLength = WLength;

    TV -> UKnotVector = TV -> VKnotVector = TV -> WKnotVector = NULL;

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new power basis trivariate.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:    Number of control points in the U direction.                 M
*   VLength:    Number of control points in the V direction.                 M
*   WLength:    Number of control points in the W direction.                 M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    An uninitialized freeform trivariate power basis.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVNew, TrivBspTVNew, TrivTVNew                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivPwrTVNew, trivariates, allocation                                    M
*****************************************************************************/
TrivTVStruct *TrivPwrTVNew(int ULength,
			   int VLength,
			   int WLength,
			   CagdPointType PType)
{
    TrivTVStruct
	*TV = TrivTVNew(TRIV_TVPOWER_TYPE, PType, ULength, VLength, WLength);

    TV -> UOrder = TV -> ULength = ULength;
    TV -> VOrder = TV -> VLength = VLength;
    TV -> WOrder = TV -> WLength = WLength;

    TV -> UKnotVector = TV -> VKnotVector = TV -> WKnotVector = NULL;

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a trivariate structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        Trivariate to duplicate                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    Duplicated trivariate.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVCopy, trivariates                                                  M
*****************************************************************************/
TrivTVStruct *TrivTVCopy(const TrivTVStruct *TV)
{
    int i, Len,
	MaxAxis = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
	*NewTV = (TrivTVStruct *) IritMalloc(sizeof(TrivTVStruct));

    NewTV -> GType = TV -> GType;
    NewTV -> PType = TV -> PType;
    NewTV -> ULength = TV -> ULength;
    NewTV -> VLength = TV -> VLength;
    NewTV -> WLength = TV -> WLength;
    NewTV -> UVPlane = NewTV -> ULength * NewTV -> VLength;
    NewTV -> UOrder = TV -> UOrder;
    NewTV -> VOrder = TV -> VOrder;
    NewTV -> WOrder = TV -> WOrder;
    NewTV -> UPeriodic = TV -> UPeriodic;
    NewTV -> VPeriodic = TV -> VPeriodic;
    NewTV -> WPeriodic = TV -> WPeriodic;
    if (TV -> UKnotVector != NULL)
	NewTV -> UKnotVector = BspKnotCopy(NULL, TV -> UKnotVector,
				       TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder);
    else
	NewTV -> UKnotVector = NULL;
    if (TV -> VKnotVector != NULL)
	NewTV -> VKnotVector = BspKnotCopy(NULL, TV -> VKnotVector,
				       TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder);
    else
	NewTV -> VKnotVector = NULL;
    if (TV -> WKnotVector != NULL)
	NewTV -> WKnotVector = BspKnotCopy(NULL, TV -> WKnotVector,
				       TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder);
    else
	NewTV -> WKnotVector = NULL;
    NewTV -> Pnext = NULL;
    NewTV -> Attr = NULL;
    NewTV -> Points[0] = NULL;			    /* The rational element. */

    Len = TV -> ULength * TV -> VLength * TV -> WLength;
    for (i = !TRIV_IS_RATIONAL_TV(TV); i <= MaxAxis; i++) {
	NewTV -> Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);
	CAGD_GEN_COPY(NewTV -> Points[i], TV -> Points[i],
		      sizeof(CagdRType) * Len);
    }

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewTV -> Points[i] = NULL;

    return NewTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of trivariate structures.			 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TVList:    List of trivariates to duplicate.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  Duplicated list of trivariates.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVCopyList, trivariates                                              M
*****************************************************************************/
TrivTVStruct *TrivTVCopyList(const TrivTVStruct *TVList)
{
    TrivTVStruct *TVTemp, *NewTVList;

    if (TVList == NULL)
	return NULL;
    TVTemp = NewTVList = TrivTVCopy(TVList);
    TVList = TVList -> Pnext;
    while (TVList) {
	TVTemp -> Pnext = TrivTVCopy(TVList);
	TVTemp = TVTemp -> Pnext;
	TVList = TVList -> Pnext;
    }
    return NewTVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a trivariate structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:         Trivariate to free.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVFree, trivariates                                                  M
*****************************************************************************/
void TrivTVFree(TrivTVStruct *TV)
{
    int i, MaxAxis;

    if (TV == NULL)
	return;

    MaxAxis = CAGD_NUM_OF_PT_COORD(TV -> PType);

    for (i = !TRIV_IS_RATIONAL_TV(TV); i <= MaxAxis; i++)
	IritFree(TV -> Points[i]);

    if (TV -> UKnotVector != NULL)
	IritFree(TV -> UKnotVector);
    if (TV -> VKnotVector != NULL)
	IritFree(TV -> VKnotVector);
    if (TV -> WKnotVector != NULL)
	IritFree(TV -> WKnotVector);

    IP_ATTR_FREE_ATTRS(TV -> Attr);
    IritFree(TV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of trivariate structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TVList:    Trivariate list to free.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVFreeList, trivariates                                              M
*****************************************************************************/
void TrivTVFreeList(TrivTVStruct *TVList)
{
    TrivTVStruct *TVTemp;

    while (TVList) {
	TVTemp = TVList -> Pnext;
	TrivTVFree(TVList);
	TVList = TVTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new triangle.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTriangleStruct *:  An uninitialized triangle.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTriangleNew, allocation                                              M
*****************************************************************************/
TrivTriangleStruct *TrivTriangleNew(void)
{
    TrivTriangleStruct
	*NewTri = (TrivTriangleStruct *)
				IritMalloc(sizeof(TrivTriangleStruct));

    NewTri -> Pnext = NULL;
    NewTri -> Attr = NULL;

    return NewTri;    
}
/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and duplicates all slots of a triangle structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Triangle:    Triangle to duplicate.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTriangleStruct *:  Duplicated triangle.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTriangleCopy                                                         M
*****************************************************************************/
TrivTriangleStruct *TrivTriangleCopy(const TrivTriangleStruct *Triangle)
{
    TrivTriangleStruct
	*NewTri = (TrivTriangleStruct *)
				IritMalloc(sizeof(TrivTriangleStruct));

    IRIT_GEN_COPY(NewTri, Triangle, sizeof(TrivTriangleStruct));

    NewTri -> Pnext = NULL;
    NewTri -> Attr = NULL;

    return NewTri;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a list of triangle structures.			 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriangleList:    List of triangle to duplicate.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTriangleStruct *:  Duplicated list of triangle.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTriangleCopyList	                                             M
*****************************************************************************/
TrivTriangleStruct *TrivTriangleCopyList(const TrivTriangleStruct
					                        *TriangleList)
{
    TrivTriangleStruct *TriangleTemp, *NewTriangleList;

    if (TriangleList == NULL)
	return NULL;
    TriangleTemp = NewTriangleList = TrivTriangleCopy(TriangleList);
    TriangleList = TriangleList -> Pnext;
    while (TriangleList) {
	TriangleTemp -> Pnext = TrivTriangleCopy(TriangleList);
	TriangleTemp = TriangleTemp -> Pnext;
	TriangleList = TriangleList -> Pnext;
    }
    return NewTriangleList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a triangle structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Triangle:   Triangle to free.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTriangleFree                                                         M
*****************************************************************************/
void TrivTriangleFree(TrivTriangleStruct *Triangle)
{
    if (Triangle)
        return;

    IP_ATTR_FREE_ATTRS(Triangle -> Attr);
    IritFree(Triangle);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a list of triangle structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   TriangleList:    Triangle list to free.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTriangleFreeList		                                     M
*****************************************************************************/
void TrivTriangleFreeList(TrivTriangleStruct *TriangleList)
{
    TrivTriangleStruct *TriangleTemp;

    while (TriangleList) {
	TriangleTemp = TriangleList -> Pnext;
	TrivTriangleFree(TriangleList);
	TriangleList = TriangleTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Linearly transforms, in place, given TV as specified by Translate and      M
* Scale.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:            Trivariate to transform.                                  M
*   Translate:     Translation factor. Can be NULL for non.                  M
*   Scale:         Scaling factor.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVTransform, trivariates                                             M
*****************************************************************************/
void TrivTVTransform(TrivTVStruct *TV, CagdRType *Translate, CagdRType Scale)
{
    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	    CagdTransform(TV -> Points,
	    		  TV -> ULength * TV -> VLength * TV -> WLength,
	                  CAGD_NUM_OF_PT_COORD(TV -> PType),
			  !TRIV_IS_RATIONAL_TV(TV),
		          Translate,
        	          Scale);
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms, in place, the given TV as specified by homogeneous matrix Mat. M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:            Trivariate to transform.                                  M
*   Mat:           Homogeneous transformation to apply to TV.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVMatTransform, trivariates                                          M
*****************************************************************************/
void TrivTVMatTransform(TrivTVStruct *TV, CagdMType Mat)
{
    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	    CagdMatTransform(TV -> Points,
    			     TV -> ULength * TV -> VLength * TV -> WLength,
        	             CAGD_NUM_OF_PT_COORD(TV -> PType),
			     !TRIV_IS_RATIONAL_TV(TV),
		             Mat);
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier trivariate into a Bspline trivariate by adding two open  M
* end uniform knot vectors to it.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        A Bezier trivariate to convert to a Bspline TV.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A Bspline trivariate representing the same geometry as  M
*                    the given Bezier TV.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivCnvrtBsp2BzrTV				                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCnvrtBzr2BspTV, conversion, trivariate		                     M
*****************************************************************************/
TrivTVStruct *TrivCnvrtBzr2BspTV(const TrivTVStruct *TV)
{
    TrivTVStruct *BspTV;

    if (TV -> GType != TRIV_TVBEZIER_TYPE) {
        TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
        return NULL;
    }

    BspTV = TrivTVCopy(TV);

    BspTV -> UOrder = BspTV -> ULength;
    BspTV -> VOrder = BspTV -> VLength;
    BspTV -> WOrder = BspTV -> WLength;
    BspTV -> UKnotVector = BspKnotUniformOpen(BspTV -> ULength,
					      BspTV -> UOrder, NULL);
    BspTV -> VKnotVector = BspKnotUniformOpen(BspTV -> VLength,
					      BspTV -> VOrder, NULL);
    BspTV -> WKnotVector = BspKnotUniformOpen(BspTV -> WLength,
					      BspTV -> WOrder, NULL);
    BspTV -> GType = TRIV_TVBSPLINE_TYPE;
    return BspTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Convert a Bspline trivar into a set of Bezier trivars by subdiving the     M
* Bspline trivar at all its internal knots.				     M
*   Returned is a list of Bezier trivars.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Bspline trivar to convert to a Bezier trivar.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A list of Bezier trivars representing same geometry     M
*                    as TV.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivCnvrtBzr2BspeTV	                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCnvrtBsp2BzrTV, conversion	                                     M
*****************************************************************************/
TrivTVStruct *TrivCnvrtBsp2BzrTV(const TrivTVStruct *TV)
{
    int i, Orders[3], Lengths[3];
    CagdRType *KnotVectors[3];
    TrivTVStruct *BezTV;

    if (TV -> GType != TRIV_TVBSPLINE_TYPE) {
	TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	return NULL;
    }

    Orders[0] = TV -> UOrder;
    Orders[1] = TV -> VOrder;
    Orders[2] = TV -> WOrder;

    Lengths[0] = TV -> ULength;
    Lengths[1] = TV -> VLength;
    Lengths[2] = TV -> WLength;

    KnotVectors[0] = TV -> UKnotVector;
    KnotVectors[1] = TV -> VKnotVector;
    KnotVectors[2] = TV -> WKnotVector;

    for (i = 0; i < 3; i++) {
	if (!BspKnotHasBezierKV(KnotVectors[i],	Lengths[i], Orders[i])) {
	    CagdRType
		t = KnotVectors[i][(Lengths[i] + Orders[i]) >> 1];
	    TrivTVDirType Dir = i == 0 ? TRIV_CONST_U_DIR
				       : (i == 1 ? TRIV_CONST_V_DIR
				                 : TRIV_CONST_W_DIR);
	    TrivTVStruct *TV1Bzrs, *TV2Bzrs,
		*TV1 = TrivTVSubdivAtParam(TV, t, Dir),
		*TV2 = TV1 -> Pnext;

	    TV1 -> Pnext = NULL;

	    TV1Bzrs = TrivCnvrtBsp2BzrTV(TV1);
	    TV2Bzrs = TrivCnvrtBsp2BzrTV(TV2);

	    TrivTVFree(TV1);
	    TrivTVFree(TV2);

	    return CagdListAppend(TV1Bzrs, TV2Bzrs);
	}
    }

    /* It is a Bezier trivariate! */
    BezTV = TrivTVCopy(TV);

    BezTV -> GType = TRIV_TVBEZIER_TYPE;
    IritFree(BezTV -> UKnotVector);
    IritFree(BezTV -> VKnotVector);
    IritFree(BezTV -> WKnotVector);
    BezTV -> UKnotVector = NULL;
    BezTV -> VKnotVector = NULL;
    BezTV -> WKnotVector = NULL;

    return BezTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline trivariate into a Bspline trivariate with floating end  M
* conditions.                                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Bspline trivariate to convert to floating end conditions.      M
*             Assume TV is either periodic or has floating end condition.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A Bspline trivariate with floating end conditions,      M
*                    representing the same geometry as TV.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivCnvrtFloat2OpenTV						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCnvrtPeriodic2FloatTV, conversion                                    M
*****************************************************************************/
TrivTVStruct *TrivCnvrtPeriodic2FloatTV(const TrivTVStruct *TV)
{
  int i, j, k, l,
	UOrder = TV -> UOrder,
	VOrder = TV -> VOrder,
	WOrder = TV -> VOrder,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	MaxAxis = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct *NewTV;

    if (!TRIV_IS_BSPLINE_TV(TV)) {
	TRIV_FATAL_ERROR(TRIV_ERR_BSP_TV_EXPECT);
	return NULL;
    }

    if (!TRIV_IS_UPERIODIC_TV(TV) &&
	!TRIV_IS_VPERIODIC_TV(TV) &&
	!TRIV_IS_WPERIODIC_TV(TV)) {
	TRIV_FATAL_ERROR(TRIV_ERR_PERIODIC_EXPECTED);
	return NULL;
    }

    NewTV = TrivBspTVNew(TRIV_TV_UPT_LST_LEN(TV), TRIV_TV_VPT_LST_LEN(TV),
			 TRIV_TV_WPT_LST_LEN(TV), UOrder, VOrder, WOrder,
			 TV -> PType);

    CAGD_GEN_COPY(NewTV -> UKnotVector, TV -> UKnotVector,
		  sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) + UOrder));
    CAGD_GEN_COPY(NewTV -> VKnotVector, TV -> VKnotVector,
		  sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) + VOrder));
    CAGD_GEN_COPY(NewTV -> WKnotVector, TV -> WKnotVector,
		  sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) + WOrder));

    for (i = 0; i < TRIV_TV_UPT_LST_LEN(TV); i++) {
        for (j = 0; j < TRIV_TV_VPT_LST_LEN(TV); j++) {
	    for (k = 0; k < TRIV_TV_WPT_LST_LEN(TV); k++) {
	        int NewIdx = TRIV_MESH_UVW(NewTV, i, j, k),
		    Idx = TRIV_MESH_UVW(NewTV,
					i % ULength, j % VLength, k % WLength);

		for (l = !CAGD_IS_RATIONAL_PT(TV -> PType);
		     l <= MaxAxis;
		     l++)
		    NewTV -> Points[l][NewIdx] = TV -> Points[l][Idx];
	    }
	}
    }

    for (i = MaxAxis + 1; i <= CAGD_MAX_PT_COORD; i++)
	NewTV -> Points[i] = NULL;

    CAGD_PROPAGATE_ATTR(NewTV, TV);

    return NewTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a float Bspline trivariate to a Bspline trivariate with open end  M
* conditions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Bspline trivariate to convert to open end conditions.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A Bspline trivariate with open end conditions,	     M
*                    representing the same geometry as TV.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivCnvrtPeriodic2FloatTV, TrivTVOpenEnd                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCnvrtFloat2OpenTV, conversion                                        M
*****************************************************************************/
TrivTVStruct *TrivCnvrtFloat2OpenTV(const TrivTVStruct *TV)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;
    TrivTVStruct *TV1, *TV2;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    TV1 = TrivTVRegionFromTV(TV, UMin, UMax, TRIV_CONST_U_DIR);
    TV2 = TrivTVRegionFromTV(TV1, VMin, VMax, TRIV_CONST_V_DIR);
    TrivTVFree(TV1);
    TV1 = TrivTVRegionFromTV(TV2, WMin, WMax, TRIV_CONST_W_DIR);
    TrivTVFree(TV2);

    TRIV_PROPAGATE_ATTR(TV1, TV);

    return TV1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts an arbitrary Bspline trivariate to a Bspline trivariate with open M
* end conditions.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Bspline trivariate to convert to open end conditions.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A Bspline trivariate with open end conditions,	     M
*                     representing the same geometry as TV.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivCnvrtPeriodic2FloatTV, TrivCnvrtFloat2OpenTV                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVOpenEnd, conversion                                                M
*****************************************************************************/
TrivTVStruct *TrivTVOpenEnd(const TrivTVStruct *TV)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;
    TrivTVStruct *TV1, *TV2;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    if (TRIV_IS_PERIODIC_TV(TV)) {
        TV2 = TrivCnvrtPeriodic2FloatTV(TV);
	TV1 = TrivTVRegionFromTV(TV2, UMin, UMax, TRIV_CONST_U_DIR);
	TrivTVFree(TV2);
    }
    else
        TV1 = TrivTVRegionFromTV(TV, UMin, UMax, TRIV_CONST_U_DIR);

    TV2 = TrivTVRegionFromTV(TV1, VMin, VMax, TRIV_CONST_V_DIR);
    TrivTVFree(TV1);
    TV1 = TrivTVRegionFromTV(TV2, WMin, WMax, TRIV_CONST_W_DIR);
    TrivTVFree(TV2);

    TRIV_PROPAGATE_ATTR(TV1, TV);

    return TV1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given trivar has no interior knot open end KVs.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To check for KVs that mimics Bezier polynomial surface.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if same as Bezier trivar, FALSE otherwise.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVHasBezierKVs, conversion                                        M
*****************************************************************************/
CagdBType TrivBspTVHasBezierKVs(const TrivTVStruct *TV)
{
    return
	BspKnotHasBezierKV(TV -> UKnotVector, TV -> ULength, TV -> UOrder) &&
	BspKnotHasBezierKV(TV -> VKnotVector, TV -> VLength, TV -> VOrder) &&
	BspKnotHasBezierKV(TV -> WKnotVector, TV -> WLength, TV -> WOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline trivar has open end coditions.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:      To check for open end conditions.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if trivar has open end conditions, FALSE otherwise.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVHasOpenEC, open end conditions                                  M
*****************************************************************************/
CagdBType TrivBspTVHasOpenEC(const TrivTVStruct *TV)
{
    return
	BspKnotHasOpenEC(TV -> UKnotVector, TV -> ULength, TV -> UOrder) &&
	BspKnotHasOpenEC(TV -> VKnotVector, TV -> VLength, TV -> VOrder) &&
	BspKnotHasOpenEC(TV -> WKnotVector, TV -> WLength, TV -> WOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two trivariates for similarity.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Tv1, Tv2:     The two trivariates to compare.                            M
*   Eps:          Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if trivariates are the same, FALSE otehrwise.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfsSame, CagdCrvsSame, MvarMVsSame			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVsSame                                                              M
*****************************************************************************/
CagdBType TrivTVsSame(const TrivTVStruct *Tv1,
		      const TrivTVStruct *Tv2,
		      CagdRType Eps)
{
    do {
        if (Tv1 -> PType != Tv2 -> PType ||
	    Tv1 -> GType != Tv2 -> GType ||
	    Tv1 -> UOrder != Tv2 -> UOrder ||
	    Tv1 -> VOrder != Tv2 -> VOrder ||
	    Tv1 -> WOrder != Tv2 -> WOrder ||
	    Tv1 -> ULength != Tv2 -> ULength ||
	    Tv1 -> VLength != Tv2 -> VLength ||
	    Tv1 -> WLength != Tv2 -> WLength ||
	    Tv1 -> UPeriodic != Tv2 -> UPeriodic ||
	    Tv1 -> VPeriodic != Tv2 -> VPeriodic ||
	    Tv1 -> WPeriodic != Tv2 -> WPeriodic)
	    return FALSE;

	if (!CagdCtlMeshsSame(Tv1 -> Points, Tv2 -> Points,
			      Tv1 -> ULength * Tv1 -> VLength * Tv1 -> WLength,
			      Eps))
            return FALSE;

	if ((Tv1 -> UKnotVector != NULL && Tv2 -> UKnotVector != NULL &&
	     !BspKnotVectorsSame(Tv1 -> UKnotVector, Tv2 -> UKnotVector,
				 Tv1 -> ULength + Tv1 -> UOrder, Eps)) ||
	    (Tv1 -> VKnotVector != NULL && Tv2 -> VKnotVector != NULL &&
	     !BspKnotVectorsSame(Tv1 -> VKnotVector, Tv2 -> VKnotVector,
				 Tv1 -> VLength + Tv1 -> VOrder, Eps)) ||
	    (Tv1 -> WKnotVector != NULL && Tv2 -> WKnotVector != NULL &&
	     !BspKnotVectorsSame(Tv1 -> WKnotVector, Tv2 -> WKnotVector,
				 Tv1 -> WLength + Tv1 -> WOrder, Eps)))
	    return FALSE;

	Tv1 = Tv1 -> Pnext;
	Tv2 = Tv2 -> Pnext;
    }
    while (Tv1 != NULL && Tv2 != NULL);

    return Tv1 == NULL && Tv2 == NULL;
}
