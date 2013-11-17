/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to provide the required interfact for the cagd library for the    *
* free form surfaces and curves.					     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "ext_lib.h"
#include "freeform.h"

static User3DSpreadType UserCnvrtInt2BlobSpreadMethod(int i);
static UserImgShd3dBlobColorType UserCnvrtInt2BlobColorMethod(int i);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts an integral integer value to a blob spread method.              *
*                                                                            *
* PARAMETERS:                                                                *
*   i:   Integral integer input.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   User3DSpreadType:    Converted data.                                     *
*****************************************************************************/
static User3DSpreadType UserCnvrtInt2BlobSpreadMethod(int i)
{
    switch (i) {
        default:
        case 1:
	    return USER_3D_SPREAD_RANDOM;
        case 2:
	    return USER_3D_SPREAD_DIAG_PLANE;
	case 3:
	    return USER_3D_SPREAD_DIAG_PLANE2;
        case 4:
	    return USER_3D_SPREAD_ANTI_DIAG_PLANE;
	case 5:
	    return USER_3D_SPREAD_ANTI_DIAG_PLANE2;
	case 6:
	    return USER_3D_SPREAD_ANTI_DIAG_PLANE3;
	case 7:
	    return USER_3D_SPREAD_DISCONT2PLANE;
        case 8:
	    return USER_3D_SPREAD_DISCONT4PLANE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts an integral integer value to a blob color method.               *
*                                                                            *
* PARAMETERS:                                                                *
*   i:   Integral integer input.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserImgShd3dBlobColorType:    Converted data.                            *
*****************************************************************************/
static UserImgShd3dBlobColorType UserCnvrtInt2BlobColorMethod(int i)
{
    switch (i) {
        default:
        case 1:
	    return USER_IMG_SHD_3D_BLOB_NO_COLOR;
        case 2:
	    return USER_IMG_SHD_3D_BLOB_GRAY_LEVEL;    
        case 3:
	    return USER_IMG_SHD_3D_BLOB_COLOR;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Resolution^2 blobs that looks like the 1st image (gray level)    M
* from the XZ plane and like the 2nd image from the YZ plane.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:    Name of 1st image to load.                                M
*   Image2Name:    Name of 2nd image to load.                                M
*   RDoTexture:    TRUE to add 'uvvals' attributes, FALSE to add actual      M
*                  color to the vertices of the objels.                      M
*   Blob:          If specified used as the blob.  If NULL, a cross is used. M
*                  Blob must be of size one in each axis, probably centered  M
*                  around the origin.  Must be a list of 3 objects for blob  M
*                  coloring methods other that "No color".		     M
*   RBlobSpread:   Method of spreading the blobs.                            M
*   RBlobColor:    Method of coloring each blob.                             M
*   RResolution:   Resolution of created objects (n^2 objects are created).  M
*   RNegative:     Default (FALSE) is white blobs over dark background. If   M
*                  TRUE, assume dark blobs over white background.            M
*   Intensity:     The gray level affect on the blobs' scale.                M
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blobl  M
*                  completely in one direction which will render it          M
*                  impossible to actually manufacture it.		     M
*   RMergePolys:   TRUE to merge all objects' polygons into one object.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list (poly if MergePolys) object of Resolution^2    M
*		       spherical blobs.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMake3DStatueFrom2Images                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   Model3DFrom2Images                                                       M
*****************************************************************************/
IPObjectStruct *Model3DFrom2Images(const char *Image1Name,
				   const char *Image2Name,
				   IrtRType *RDoTexture,
				   const IPObjectStruct *Blob,
				   IrtRType *RBlobSpread,
				   IrtRType *RBlobColor,
				   IrtRType *RResolution,
				   IrtRType *RNegative,
				   IrtRType *Intensity,
				   IrtRType *MinIntensity,
				   IrtRType *RMergePolys)
{
    User3DSpreadType
        BlobSpreadMethod = UserCnvrtInt2BlobSpreadMethod(
					 IRIT_REAL_PTR_TO_INT(RBlobSpread));
    UserImgShd3dBlobColorType
        BlobColorMethod = UserCnvrtInt2BlobColorMethod(
					 IRIT_REAL_PTR_TO_INT(RBlobColor));
	       
    if (!IP_IS_POLY_OBJ(Blob) && !IP_IS_OLST_OBJ(Blob))
        Blob = NULL;

    return UserMake3DStatueFrom2Images(Image1Name, Image2Name,
				       IRIT_REAL_PTR_TO_INT(RDoTexture), Blob,
				       BlobSpreadMethod, BlobColorMethod,
				       IRIT_REAL_PTR_TO_INT(RResolution),
				       IRIT_REAL_PTR_TO_INT(RNegative),
				       *Intensity, *MinIntensity,
				       IRIT_REAL_PTR_TO_INT(RMergePolys));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Resolution^2 3D cross blobs that looks like the 1st image        M
* (gray level) from the XZ plane, like the 2nd image from the YZ plane, and  M
* like the 3rd image from the XY plane.					     M
*   A 3D cross is used as a blob.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:    Name of 1st image to load.                                M
*   Image2Name:    Name of 2nd image to load.                                M
*   Image3Name:    Name of 3rd image to load.                                M
*   RDoTexture:    TRUE to add 'uvvals' attributes, FALSE to add actual      M
*                  color to the vertices of the objels.                      M
*   Blob:          If specified used as the blob. If NULL, a 3-cross is used.M
*                  Blob must be of size one in each axis, probably centered  M
*                  around the origin.  Should hold 3 polygons for "No color" M
*		   blob color method and should hold a list of 3 objects for M
*                  the other methods. 					     M
*   RBlobSpread:   Method of spreading the blobs.                            M
*   RBlobColor:    Method of coloring each blob.                             M
*   RResolution:   Resolution of created objects (n^2 objects are created).  M
*   RNegative:     Default (FALSE) is white blobs over dark background. If   M
*                  TRUE, assume dark blobs over white background.            M
*   Intensity:     The gray level affect on the blobs' scale.                M
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blobl  M
*                  completely in one direction which will render it          M
*                  impossible to actually manufacture it.		     M
*   RMergePolys:   TRUE to merge all objects' polygons into one object.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list (poly if MergePolys) object of Resolution^2    M
*		       spherical blobs.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserMake3DStatueFrom3Images                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   Model3DFrom3Images                                                       M
*****************************************************************************/
IPObjectStruct *Model3DFrom3Images(const char *Image1Name,
				   const char *Image2Name,
				   const char *Image3Name,
				   IrtRType *RDoTexture,
				   const IPObjectStruct *Blob,
				   IrtRType *RBlobSpread,
				   IrtRType *RBlobColor,
				   IrtRType *RResolution,
				   IrtRType *RNegative,
				   IrtRType *Intensity,
				   IrtRType *MinIntensity,
				   IrtRType *RMergePolys)
{
    User3DSpreadType
        BlobSpreadMethod = UserCnvrtInt2BlobSpreadMethod(
					 IRIT_REAL_PTR_TO_INT(RBlobSpread));
    UserImgShd3dBlobColorType
        BlobColorMethod = UserCnvrtInt2BlobColorMethod(
					 IRIT_REAL_PTR_TO_INT(RBlobColor));

    if (!IP_IS_POLY_OBJ(Blob) && !IP_IS_OLST_OBJ(Blob))
        Blob = NULL;

    return UserMake3DStatueFrom3Images(Image1Name, Image2Name, Image3Name,
				       IRIT_REAL_PTR_TO_INT(RDoTexture),
				       Blob, BlobSpreadMethod, BlobColorMethod,
				       IRIT_REAL_PTR_TO_INT(RResolution),
				       IRIT_REAL_PTR_TO_INT(RNegative),
				       *Intensity, *MinIntensity,
				       IRIT_REAL_PTR_TO_INT(RMergePolys));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Resolution^2 blobs that looks like the 1st image (gray level)    M
* from the XZ plane and like the 2nd image from the YZ plane.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:  Name of 1st image to load.                                  M
*   Image2Name:  Name of 2nd image to load.                                  M
*   RDitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.             M
*   RMatchWidth:  Width to allow matching in a row:			     M
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. M
*   RNegate:     TRUE to negate the images before use.			     M
*   RAugmentContrast:  Number of iterations to add micro-pixels, to augment  M
*                the contrast, behind existing pixels.  Zero to disable.     M
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random M
*                spread to use.						     M
*   SphereRad:   Radius of construct spherical blob, zero to return points.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Micro blobs if SphereRad > 0, center points, if = 0.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserDither3D2Images                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MicroBlobsDither3DFrom2Images                                            M
*****************************************************************************/
IPObjectStruct *MicroBlobsDither3DFrom2Images(const char *Image1Name,
					      const char *Image2Name,
					      IrtRType *RDitherSize,
					      IrtRType *RMatchWidth,
					      IrtRType *RNegate,
					      IrtRType *RAugmentContrast,
					      IrtRType *RSpreadMethod,
					      IrtRType *SphereRad)
{
    IrtRType AccumPenalty;
    User3DSpreadType
        SpreadMethod = UserCnvrtInt2BlobSpreadMethod(
					 IRIT_REAL_PTR_TO_INT(RSpreadMethod));
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    PObj = User3DDither2Images(Image1Name, Image2Name,
			       IRIT_REAL_PTR_TO_INT(RDitherSize),
			       IRIT_REAL_PTR_TO_INT(RMatchWidth),
			       IRIT_REAL_PTR_TO_INT(RNegate),
			       IRIT_REAL_PTR_TO_INT(RAugmentContrast),
			       SpreadMethod, *SphereRad, &AccumPenalty);

    if (PObj == NULL)
	IRIT_NON_FATAL_ERROR("BFrom2Img: invalid input (images of different sizes!?)");
    
    IRIT_WNDW_FPRINTF2("BFrom2Img: total computed penalty = %f", AccumPenalty);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Resolution^2 blobs that looks like the 1st image (gray level)    M
* from the XZ plane, like the 2nd image from the YZ plane, and like the 3nd  M
* image from the XY plane.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:  Name of 1st image to load.                                  M
*   Image2Name:  Name of 2nd image to load.                                  M
*   Image3Name:  Name of 3rd image to load.                                  M
*   RDitherSize:  2, 3, or 4 for (2x2x2), (3x3x3), or (3x3x3) dithering.     M
*   RMatchWidth:  Width to allow matching in a row:			     M
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. M
*   RNegate:     TRUE to negate the images before use.			     M
*   RAugmentContrast:  Number of iterations to add micro-pixels, to augment  M
*                the contrast, behind existing pixels.  Zero to disable.     M
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random M
*                spread to use.						     M
*   SphereRad:   Radius of construct spherical blob, zero to return points.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Micro blobs if SphereRad > 0, center points, if = 0.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserDither3D2Images                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MicroBlobsDither3DFrom2Images                                            M
*****************************************************************************/
IPObjectStruct *MicroBlobsDither3DFrom3Images(const char *Image1Name,
					      const char *Image2Name,
					      const char *Image3Name,
					      IrtRType *RDitherSize,
					      IrtRType *RMatchWidth,
					      IrtRType *RNegate,
					      IrtRType *RAugmentContrast,
					      IrtRType *RSpreadMethod,
					      IrtRType *SphereRad)
{
    IrtRType AccumPenalty;
    User3DSpreadType
        SpreadMethod = UserCnvrtInt2BlobSpreadMethod(
					 IRIT_REAL_PTR_TO_INT(RSpreadMethod));
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    PObj = User3DDither3Images(Image1Name, Image2Name, Image3Name,
			       IRIT_REAL_PTR_TO_INT(RDitherSize),
			       IRIT_REAL_PTR_TO_INT(RMatchWidth),
			       IRIT_REAL_PTR_TO_INT(RNegate),
			       IRIT_REAL_PTR_TO_INT(RAugmentContrast),
			       SpreadMethod, *SphereRad, &AccumPenalty);

    if (PObj == NULL)
	IRIT_NON_FATAL_ERROR("BFrom3Img: invalid input (images of different sizes!?)");
    else
        IRIT_WNDW_FPRINTF2("BFrom3Img: total computed penalty = %f",
			   AccumPenalty);

    AccumPenalty = 0;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fit a ruled surface to the given general surface Srf.  The best fit is   M
* found using a dynmaic programming search over all possibly rulings, while  M
* each ruling line's distance is measured agains the surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     To fit a ruled surface through.                              M
*   RDir:       Either the U or the V direction.  This is used only to       M
*               sample Srf and construct the possibly ruling lines.          M
*   ExtndDmn:   Amount to extended the selected sampled boundary curves.     M
*               Zero will not extend and match the ruling from the original  M
*               boundary to its maximum.				     M
*   RSamples:   Number of samples to compute the dynamic programming with.   M
*		Typically in the hundreds.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The fitted ruled surface, or NULL if error.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRuledSrfFit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   RuledSrfFit                                                              M
*****************************************************************************/
IPObjectStruct *RuledSrfFit(IPObjectStruct *SrfObj,
			    IrtRType *RDir,
			    IrtRType *ExtndDmn,
			    IrtRType *RSamples)
{
    CagdRType Error, MaxError;
    CagdSrfStruct
        *RuledSrf = UserRuledSrfFit(SrfObj -> U.Srfs,
				    (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(RDir),
				    *ExtndDmn,
				    IRIT_REAL_PTR_TO_INT(RSamples),
				    &Error,
				    &MaxError);

    if (RuledSrf == NULL)
        return NULL;
    else {
        IPObjectStruct
	    *PObj = IPGenSRFObject(RuledSrf);

	AttrSetObjectRealAttrib(PObj, "Error", Error);
	AttrSetObjectRealAttrib(PObj, "MaxError", MaxError);
	return PObj;
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the orthogonal projection of the given curve on given surface.  M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:       The curve to project on SrfObj, orthogonally.		     M
*   SrfObj:       The surface to project CrvObj on.			     M
*   Tol:          Tolerance of the computation.			             M
*   Euclidean:    TRUE to return the curves in Euclidean space, FALSE in the M
*		  surface parametric domain.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectSTruct *:    Projected curves.  Can be several!                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVOrthoCrvProjOnSrf                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvOrthoProjOnSrf                                                        M
*****************************************************************************/
IPObjectStruct *CrvOrthoProjOnSrf(IPObjectStruct *CrvObj,
				  IPObjectStruct *SrfObj,
				  IrtRType *Tol,
				  IrtRType *Euclidean)
{
    IPPolygonStruct *Pl;
    const CagdSrfStruct
        *Srf = SrfObj -> U.Srfs;
    MvarPolyStruct
        *Plls = MvarMVOrthoCrvProjOnSrf(CrvObj -> U.Crvs, Srf, *Tol);
    IPObjectStruct
        *PllsObj = MvarCnvrtMVPolysToIritPolys2(Plls, TRUE);
    CagdCrvStruct *Crvs;

    MvarPolyFreeList(Plls);

    if (!IRIT_APX_EQ(*Euclidean, 0.0)) {         /* Evaluate to E3 points. */
        for (Pl = PllsObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *V;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	        CagdRType
		    *R = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);

		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	}
    }

    Crvs = UserPolylines2LinBsplineCrvs(PllsObj -> U.Pl, TRUE);
    IPFreeObject(PllsObj);

    return IPLnkListToListObject(Crvs, IP_OBJ_CURVE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tiling with rectangular regions to the domain bound by the      M
* given set of curves.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:            List of curves that bound the domain.                M
*   RAngularDeviations: Maximal angular deviations, in degrees, on the       M
*                       boundary before forcing a split.		     M
*   RCurveOutputType:   1 for Polygonal rectangles,                          M
*                       2 for curves lists,				     M
*                       3 for surfaces.					     M
*   RSizeRectangle:     Approximated edge size of expected rectangles.       M
*   RNumSmoothingSteps: Low pass (smoothing) filtering to apply to the       M
*                       vertices, with respect to neighboring vertices.      M
*                       Number of smoothing iteration - zero to disable.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object that holds polygons, or curves, or      M
*                      surfaces, depending on RCurveOutputType.		     M
*                      NULL is returned in case of error (and ErrorMsg set). M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtExtC2SGeneral                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   Crvs2RectRegionGeneral                                                   M
*****************************************************************************/
IPObjectStruct *Crvs2RectRegionGeneral(IPObjectStruct *CrvList,
				       IrtRType *RAngularDeviations,
				       IrtRType *RCurveOutputType,
				       IrtRType *RSizeRectangle,
				       IrtRType *RNumSmoothingSteps)
{
    int i = 0;
    const char *ErrMsg;
    CagdCrvStruct
        *Crvs = NULL;
    IPObjectStruct *PCrv, *PObj;

    while ((PCrv = IPListObjectGet(CrvList, i++)) != NULL) {
        if (IP_IS_CRV_OBJ(PCrv)) {
	    Crvs = CagdListAppend(Crvs, CagdCrvCopyList(PCrv -> U.Crvs));
        }
    }
    if (Crvs == NULL) {
        IRIT_NON_FATAL_ERROR("C2RectRgn: No curves were found in the input");
	return NULL;
    }

    PObj = IrtExtC2SGeneral(&Crvs, IRIT_REAL_PTR_TO_INT(RAngularDeviations),
			    *RSizeRectangle,
			    IRIT_REAL_PTR_TO_INT(RCurveOutputType),
			    *RSizeRectangle,
			    IRIT_REAL_PTR_TO_INT(RNumSmoothingSteps),
			    &ErrMsg, "C2RR");
    CagdCrvFreeList(Crvs);

    if (ErrMsg != NULL)
        IRIT_NON_FATAL_ERROR(ErrMsg);

    return PObj;
}
