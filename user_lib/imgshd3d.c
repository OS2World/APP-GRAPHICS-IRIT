/******************************************************************************
* imgshd3d.c - synthesizes a 3D object from 2/3 images.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 2009.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "user_loc.h"

#define USER_MIN_IMAGE_INTESITY 0.01
#define USER_BOUND_0_255(x) \
			(IrtBType) IRIT_BOUND((x) * Intensity, 0, 255)
#define USER_SCALE_POLY_IN_PLACE(Pl, Scl) { \
		    IrtHmgnMatType SMat; \
		    IPPolygonStruct *PlTmp; \
		    MatGenMatUnifScale(Scl, SMat); \
		    PlTmp = GMTransformPolyList(Pl, SMat, TRUE); \
		    IPFreePolygon(Pl); \
		    Pl = PlTmp; \
		}


typedef struct UserRandomVecStruct {
    int i;
    IrtRType R;
} UserRandomVecStruct;

static IPObjectStruct *UserImgGetCrossBlob(int SeparatedObjs);
static IPObjectStruct *UserImgGetCross3DBlob(int SeparatedObjs);
static int UserSetBlobColorIntensity(IPObjectStruct *Blob,
				     const IrtImgPixelStruct *Image1,
				     const IrtImgPixelStruct *Image2,
				     const IrtImgPixelStruct *Image3,
				     int Img1SizeX,
				     int Img1SizeY,
				     int Img2SizeX,
				     int Img2SizeY,
				     int Img3SizeX,
				     int Img3SizeY,
				     int DoTexture,
				     UserImgShd3dBlobColorType
					                      BlobColorMethod,
				     int Negative,
				     IrtRType Intensity,
				     IrtRType MinIntensity);
static int UserSetBlobColorIntensityAux(IPObjectStruct *PObj,
					const IrtImgPixelStruct *Image,
					int ImgSizeX,
					int ImgSizeY,
					int DoTexture, 
					UserImgShd3dBlobColorType
					                    BlobColorMethod,
					int Negative,
					IrtRType Intensity,
					IrtRType MinIntensity);
static IrtRType UserGetImageIntensity(const IrtImgPixelStruct *Img,
				      int ImgSizeX,
				      int ImgSizeY,
				      IrtRType x,
				      IrtRType y,
				      int Negative,
				      IrtRType MinIntensity);
static const IrtImgPixelStruct *UserGetImageColor(const IrtImgPixelStruct *Img,
						  int ImgSizeX,
						  int ImgSizeY,
						  IrtRType x,
						  IrtRType y);

#if defined(ultrix) && defined(mips)
static int UserCompTwoReals(VoidPtr P1, VoidPtr P2);
#else
static int UserCompTwoReals(const VoidPtr P1, const VoidPtr P2);
#endif /* ultrix && mips (no const support) */

static int UserInsertNewMicroBlob(IPVertexStruct *Blob,
				  IPVertexStruct **BlobsList,
				  IrtRType MinDist);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates Resolution^2 blobs that looks like the 1st image (gray level)    M
* from the XZ plane and like the 2nd image from the YZ plane.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:    Name of 1st image to load.                                M
*   Image2Name:    Name of 2nd image to load.                                M
*   DoTexture:     TRUE to add 'uvvals' attributes, FALSE to add actual      M
*                  color to the vertices of the objels.                      M
*   Blob:          If specified used as the blob. If NULL, a 2-cross is used.M
*                  Blob must be of size one in each axis, probably centered  M
*                  around the origin.  Must be a list of 3 objects for blob  M
*		   coloring methods other than "No color".		     M
*   BlobSpreadMethod:  Method of spreading the blobs.                        M
*   BlobColorMethod:   Method of coloring each blob.                         M
*   Resolution:    Resolution of created objects (n^2 objects are created).  M
*   Negative:      Default (FALSE) is white blobs over dark background. If   M
*                  TRUE, assume dark blobs over white background.            M
*   Intensity:     The gray level affect on the blobs' scale.                M
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blobl  M
*                  completely in one direction which will render it          M
*                  impossible to actually manufacture it.		     M
*   MergePolys:    TRUE to merge all objects' polygons into one object.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list (poly if MergePolys) object of Resolution^2    M
*		       spherical blobs.			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMake3DStatueFrom2Images                                              M
*****************************************************************************/
IPObjectStruct *UserMake3DStatueFrom2Images(const char *Image1Name,
					    const char *Image2Name,
					    int DoTexture,
					    const IPObjectStruct *Blob,
					    User3DSpreadType BlobSpreadMethod,
					    UserImgShd3dBlobColorType
						             BlobColorMethod,
					    int Resolution,
					    int Negative,
					    IrtRType Intensity,
					    IrtRType MinIntensity,
					    int MergePolys)
{
    int z, Image1X, Image1Y, Image1Alpha, Image2X, Image2Y, Image2Alpha,
        n = 0;
    IrtRType AspectRatio;
    IrtImgPixelStruct
        *Image1 = IrtImgReadImage(Image1Name, &Image1X, &Image1Y, &Image1Alpha),
        *Image2 = IrtImgReadImage(Image2Name, &Image2X, &Image2Y, &Image2Alpha);
    IPObjectStruct *RetVal, *BlobObj;

    /* Do we have a valid pair of images? */
    if (Image1 == NULL || Image2 == NULL) {
        if (Image1 != NULL)
	    IritFree(Image1);
        if (Image2 != NULL)
	    IritFree(Image2);

	USER_FATAL_ERROR(USER_ERR_INVALID_IMAGE_SIZE);
        return NULL;
    }
    AspectRatio = Image1Y / ((IrtRType) Image1X);

    switch (BlobColorMethod) {
	case USER_IMG_SHD_3D_BLOB_NO_COLOR:
	    if (Blob == NULL)
	        BlobObj = UserImgGetCrossBlob(FALSE);
	    else
	        BlobObj = IPCopyObject(NULL, Blob, FALSE);
	    break;
        case USER_IMG_SHD_3D_BLOB_GRAY_LEVEL:
        case USER_IMG_SHD_3D_BLOB_COLOR:
	    if (Blob == NULL ||
		!IP_IS_OLST_OBJ(Blob) ||
		IPListObjectLength(Blob) < 2)
	        BlobObj = UserImgGetCrossBlob(TRUE);
	    else
	        BlobObj = IPCopyObject(NULL, Blob, FALSE);
	    break;
        default:
	    BlobObj = NULL;
	    assert(0);
    }

    if (MergePolys)
	RetVal = IPGenPOLYObject(NULL);
    else
	RetVal = IPGenLISTObject(NULL);

    /* Build the basic geometry at each z level: */
    for (z = 0; z < ((int) (Resolution * AspectRatio)); z++) {
        int i,
	    *Vec1 = User3DMicroBlobsCreateRandomVector(Resolution,
						       BlobSpreadMethod, TRUE),
	    *Vec2 = User3DMicroBlobsCreateRandomVector(Resolution,
						       BlobSpreadMethod, FALSE);
	IrtRType Gray1, Gray2;
        IrtHmgnMatType TrnsMat, SclMat, Mat;
	IPObjectStruct *PTmp;

	for (i = 0; i < Resolution; i++) {
	    switch (BlobColorMethod) {
	        case USER_IMG_SHD_3D_BLOB_NO_COLOR:
		    /* Build a blob at (Vec1[i], Vec2[i], ZLevel) and scaled */
		    /* according to the gray shades at this position.	     */
		    MatGenMatTrans(Vec1[i], Vec2[i] + 0.5, z + 0.5, TrnsMat);

		    Gray1 = UserGetImageIntensity(Image1,
					    Image1X + 1, Image1Y + 1,
				  	    Vec1[i] / ((IrtRType) Resolution),
					    z / ((IrtRType) Resolution),
					    Negative, MinIntensity);
		    Gray2 = UserGetImageIntensity(Image2,
					    Image2X + 1, Image2Y + 1,
				  	    Vec2[i] / ((IrtRType) Resolution),
					    z / ((IrtRType) Resolution),
					    Negative, MinIntensity);
		    MatGenMatScale(Gray1 * Intensity, Gray2 * Intensity, 1.0,
				   SclMat);

		    MatMultTwo4by4(Mat, SclMat, TrnsMat);

		    /* Normalize to [0..1]. */
		    MatGenMatUnifScale(1.0 / Resolution, SclMat);
		    MatMultTwo4by4(Mat, Mat, SclMat);

		    PTmp = GMTransformObject(BlobObj, Mat);
		    break;
		case USER_IMG_SHD_3D_BLOB_GRAY_LEVEL:
		case USER_IMG_SHD_3D_BLOB_COLOR:
		    /* Needs to paint the proper color/gray level to the    */
		    /* two polygons of BlobObj.		    	            */
		    MatGenMatTrans(Vec1[i], Vec2[i] + 0.5, z + 0.5, TrnsMat);
		    MatGenMatUnifScale(1.0 / Resolution, SclMat);
		    MatMultTwo4by4(Mat, TrnsMat, SclMat);

		    PTmp = GMTransformObject(BlobObj, Mat);

		    UserSetBlobColorIntensity(PTmp, Image1, Image2, NULL,
					      Image1X, Image1Y,
					      Image2X, Image2Y,
					      0, 0, DoTexture,
					      BlobColorMethod,
					      Negative, Intensity,
					      MinIntensity);
		    break;
		default:
		    PTmp = NULL;
		    assert(0);
	    }

	    if (MergePolys) {
	        if (IP_IS_OLST_OBJ(PTmp)) {
		    int j;
		    IPObjectStruct *PTmp2;

		    for (j = 0;
			 (PTmp2 = IPListObjectGet(PTmp, j)) != NULL;
			 j++) {
		        /* Move the attributes to the poly. */
		        PTmp2 -> U.Pl -> Attr = PTmp2 -> Attr;
			PTmp2 -> Attr = NULL;

			RetVal -> U.Pl = IPAppendPolyLists(PTmp2 -> U.Pl,
							   RetVal -> U.Pl);
			PTmp2 -> U.Pl = NULL;
		    }
		}
		else {
		    RetVal -> U.Pl = IPAppendPolyLists(PTmp -> U.Pl,
						       RetVal -> U.Pl);
		    PTmp -> U.Pl = NULL;
		}

		IPFreeObject(PTmp);
	    }
	    else
	        IPListObjectInsert(RetVal, n++, PTmp);
	}

	IritFree(Vec1);
	IritFree(Vec2);
    }

    if (!MergePolys)
        IPListObjectInsert(RetVal, n, NULL);

    IPFreeObject(BlobObj);

    IritFree(Image1);
    IritFree(Image2);

    return RetVal;
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
*   DoTexture:     TRUE to add 'uvvals' attributes, FALSE to add actual      M
*                  color to the vertices of the objels.                      M
*   Blob:          If specified used as the blob. If NULL, a 3-cross is used.M
*                  Blob must be of size one in each axis, probably centered  M
*                  around the origin.  Should hold 3 polygons for "No color" M
*		   color method and should hold a list of 3 objects for      M
*                  the other methods. 					     M
*   BlobSpreadMethod:  Method of spreading the blobs.                        M
*   BlobColorMethod:   Method of coloring each blob.                         M
*   Resolution:    Resolution of created objects (n^2 objects are created).  M
*   Negative:      Default (FALSE) is white blobs over dark background. If   M
*                  TRUE, assume dark blobs over white background.            M
*   Intensity:     The gray level affect on the blobs' scale.                M
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blobl  M
*                  completely in one direction which will render it          M
*                  impossible to actually manufacture it.		     M
*   MergePolys:    TRUE to merge all objects' polygons into one object.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list (poly if MergePolys) object of Resolution^2    M
*		       spherical blobs.			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMake3DStatueFrom3Images                                              M
*****************************************************************************/
IPObjectStruct *UserMake3DStatueFrom3Images(const char *Image1Name,
					    const char *Image2Name,
					    const char *Image3Name,
					    int DoTexture,
					    const IPObjectStruct *Blob,
					    User3DSpreadType BlobSpreadMethod,
					    UserImgShd3dBlobColorType
						             BlobColorMethod,
					    int Resolution,
					    int Negative,
					    IrtRType Intensity,
					    IrtRType MinIntensity,
					    int MergePolys)
{
    int i, j, Image1X, Image1Y, Image1Alpha, Image2X, Image2Y, Image2Alpha,
        Image3X, Image3Y, Image3Alpha, **M,
        n = 0;
    IrtImgPixelStruct
        *Image1 = IrtImgReadImage(Image1Name, &Image1X, &Image1Y, &Image1Alpha),
        *Image2 = IrtImgReadImage(Image2Name, &Image2X, &Image2Y, &Image2Alpha),
        *Image3 = Image3Name == NULL ? NULL :
                  IrtImgReadImage(Image3Name, &Image3X, &Image3Y, &Image3Alpha);
    IPObjectStruct *RetVal, *BlobObj;

    /* Do we have a valid pair (or triple) of images? */
    if (Image1 == NULL || Image2 == NULL) {
        if (Image1 != NULL)
	    IritFree(Image1);
        if (Image2 != NULL)
	    IritFree(Image2);
        if (Image3 != NULL)
	    IritFree(Image3);

	USER_FATAL_ERROR(USER_ERR_INVALID_IMAGE_SIZE);
        return NULL;
    }

    if ((Image1 != NULL && Image1X != Image1Y) ||
	(Image2 != NULL && Image2X != Image2Y) ||
	(Image3 != NULL && Image3X != Image3Y)) {
        IritFree(Image1);
	if (Image2 != NULL)
	    IritFree(Image2);
	if (Image3 != NULL)
	    IritFree(Image3);

	USER_FATAL_ERROR(USER_ERR_INVALID_IMAGE_SIZE);
        return NULL;
    }

    switch (BlobColorMethod) {
	case USER_IMG_SHD_3D_BLOB_NO_COLOR:
	    if (Blob == NULL ||
		!IP_IS_POLY_OBJ(Blob) ||
		IPPolyListLen(Blob -> U.Pl) < 3)
	        BlobObj = UserImgGetCross3DBlob(FALSE);
	    else
	        BlobObj = IPCopyObject(NULL, Blob, FALSE);
	    break;
        case USER_IMG_SHD_3D_BLOB_GRAY_LEVEL:
        case USER_IMG_SHD_3D_BLOB_COLOR:
	    if (Blob == NULL ||
		!IP_IS_OLST_OBJ(Blob) ||
		IPListObjectLength(Blob) < 3)
	        BlobObj = UserImgGetCross3DBlob(TRUE);
	    else
	        BlobObj = IPCopyObject(NULL, Blob, FALSE);
	    break;
	default:
	    BlobObj = NULL;
	    assert(0);
    }

    if (MergePolys)
	RetVal = IPGenPOLYObject(NULL);
    else
	RetVal = IPGenLISTObject(NULL);

    /* Build the basic geometry at each z level: */
    M = User3DMicroBlobsCreateRandomMatrix(Resolution, BlobSpreadMethod);
    for (i = 0; i < Resolution; i++) {
	IrtRType Gray1, Gray2, Gray3;
        IrtHmgnMatType TrnsMat, SclMat, Mat;
	IPObjectStruct *PTmp, *PTmp2;
	IPPolygonStruct *Pl1, *Pl2, *Pl3;

	for (j = 0; j < Resolution; j++) {
	    switch (BlobColorMethod) {
	        case USER_IMG_SHD_3D_BLOB_NO_COLOR:
		    /* Build a blob at (u, j, M[i][j]) and scaled	    */
		    /* according to gray shades at this position.	    */
		    MatGenMatTrans(i + 0.5, j + 0.5, M[i][j] + 0.5, TrnsMat);
		    MatGenMatUnifScale(1.0 / Resolution, SclMat);
		    MatMultTwo4by4(Mat, TrnsMat, SclMat);

		    Gray1 = UserGetImageIntensity(Image1,
					    Image1X + 1, Image1Y + 1,
					    i / ((IrtRType) Resolution),
					    M[i][j] / ((IrtRType) Resolution),
					    Negative, MinIntensity);
		    Gray2 = UserGetImageIntensity(Image2,
					    Image2X + 1, Image2Y + 1,
				  	    j / ((IrtRType) Resolution),
					    M[i][j] / ((IrtRType) Resolution),
					    Negative, MinIntensity);
		    Gray3 = UserGetImageIntensity(Image3,
					    Image3X + 1, Image3Y + 1,
					    i / ((IrtRType) Resolution),
					    j / ((IrtRType) Resolution),
					    Negative, MinIntensity);

		    /* Decompose the 3-cross into 3 individual polygons and */
		    /* scale each accordingly, only to rebuild back.	    */
		    PTmp2 = IPCopyObject(NULL, BlobObj, FALSE);
		    assert(IPPolyListLen(PTmp2 -> U.Pl) >= 3);
		    IRIT_LIST_POP(Pl1, PTmp2 -> U.Pl);
		    IRIT_LIST_POP(Pl2, PTmp2 -> U.Pl);
		    IRIT_LIST_POP(Pl3, PTmp2 -> U.Pl);
		    USER_SCALE_POLY_IN_PLACE(Pl1, sqrt(Gray1 * Intensity));
		    USER_SCALE_POLY_IN_PLACE(Pl2, sqrt(Gray2 * Intensity));
		    USER_SCALE_POLY_IN_PLACE(Pl3, sqrt(Gray3 * Intensity));
		    Pl2 -> Pnext = Pl3;
		    Pl1 -> Pnext = Pl2;
		    PTmp2 -> U.Pl = Pl1;

		    PTmp = GMTransformObject(PTmp2, Mat);
		    IPFreeObject(PTmp2);
		    break;
	        case USER_IMG_SHD_3D_BLOB_GRAY_LEVEL:
	        case USER_IMG_SHD_3D_BLOB_COLOR:
		    /* Needs to paint the proper color/gray level to the    */
		    /* two polygons of BlobObj.		    	            */
		    MatGenMatTrans(i + 0.5, j + 0.5, M[i][j] + 0.5, TrnsMat);
		    MatGenMatUnifScale(1.0 / Resolution, SclMat);
		    MatMultTwo4by4(Mat, TrnsMat, SclMat);

		    PTmp = GMTransformObject(BlobObj, Mat);

		    UserSetBlobColorIntensity(PTmp, Image1, Image2, Image3,
					      Image1X, Image1Y,
					      Image2X, Image2Y,
					      Image3X, Image3Y, DoTexture,
					      BlobColorMethod,
					      Negative, Intensity,
					      MinIntensity);
		    break;
	        default:
		    PTmp = NULL;
		    assert(0);
	    }

	    if (MergePolys) {
	        if (IP_IS_OLST_OBJ(PTmp)) {
		    int j, k;
		    IPObjectStruct *PTmp2, *PTmp3;

		    for (j = 0;
			 (PTmp2 = IPListObjectGet(PTmp, j)) != NULL;
			 j++) {
		        if (IP_IS_OLST_OBJ(PTmp2)) {
			    for (k = 0;
				 (PTmp3 = IPListObjectGet(PTmp2, k)) != NULL;
				 k++) {
			        assert(IP_IS_POLY_OBJ(PTmp3));

				/* Move the attributes to the poly. */
				PTmp3 -> U.Pl -> Attr = PTmp3 -> Attr;
				PTmp3 -> Attr = NULL;

				RetVal -> U.Pl = IPAppendPolyLists(
							       PTmp3 -> U.Pl,
							       RetVal -> U.Pl);
				PTmp3 -> U.Pl = NULL;
			    }
			}
			else {
			    assert(IP_IS_POLY_OBJ(PTmp2));

			    /* Move the attributes to the poly. */
			    PTmp2 -> U.Pl -> Attr = PTmp2 -> Attr;
			    PTmp2 -> Attr = NULL;

			    RetVal -> U.Pl = IPAppendPolyLists(PTmp2 -> U.Pl,
							       RetVal -> U.Pl);
			    PTmp2 -> U.Pl = NULL;
			}
		    }
		}
		else {
		    RetVal -> U.Pl = IPAppendPolyLists(PTmp -> U.Pl,
						       RetVal -> U.Pl);
		    PTmp -> U.Pl = NULL;
		}

		IPFreeObject(PTmp);
	    }
	    else
	        IPListObjectInsert(RetVal, n++, PTmp);
	}
    }

    for (i = 0; i < Resolution; i++)
        IritFree(M[i]);
    IritFree(M);

    if (!MergePolys)
        IPListObjectInsert(RetVal, n, NULL);

    IPFreeObject(BlobObj);

    IritFree(Image1);
    IritFree(Image2);
    if (Image3 != NULL)
        IritFree(Image3);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generate a blob in the shape of a cross using only two polygons.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   A simple blob using only two polygons as a cross.	     *
*****************************************************************************/
static IPObjectStruct *UserImgGetCrossBlob(int SeparatedObjs)
{
    IRIT_STATIC_DATA const IrtVecType
        V1a = { -0.5,  0.0, -0.5 },
        V2a = { -0.5,  0.0,  0.5 },
	V3a = {  0.5,  0.0,  0.5 },
	V4a = {  0.5,  0.0, -0.5 },
	V1b = {  0.0, -0.5, -0.5 },
	V2b = {  0.0, -0.5,  0.5 },
	V3b = {  0.0,  0.5,  0.5 },
	V4b = {  0.0,  0.5, -0.5 },
	Vin = {  0.5,  0.5,  0.0 };
    int Rvrsd;
    IPPolygonStruct
        *Pl1 = PrimGenPolygon4Vrtx(V1a, V2a, V3a, V4a, Vin, &Rvrsd, NULL),
        *Pl2 = PrimGenPolygon4Vrtx(V1b, V2b, V3b, V4b, Vin, &Rvrsd, NULL);

    if (SeparatedObjs) {
        IPObjectStruct
	    *PRet = IPGenLISTObject(IPGenPOLYObject(Pl1));

	IPListObjectAppend(PRet, IPGenPOLYObject(Pl2));
	
	return PRet;
    }
    else {
        Pl1 -> Pnext = Pl2;
        return IPGenPOLYObject(Pl1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generate a blob in the shape of a 3D cross using only three polygons.    *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   A simple blob using only two polygons as a cross.	     *
*****************************************************************************/
static IPObjectStruct *UserImgGetCross3DBlob(int SeparatedObjs)
{
    IRIT_STATIC_DATA const IrtVecType
        V1a = { -0.5,  0.0, -0.5 },
        V2a = { -0.5,  0.0,  0.5 },
	V3a = {  0.5,  0.0,  0.5 },
	V4a = {  0.5,  0.0, -0.5 },
	V1b = {  0.0, -0.5, -0.5 },
	V2b = {  0.0, -0.5,  0.5 },
	V3b = {  0.0,  0.5,  0.5 },
	V4b = {  0.0,  0.5, -0.5 },
	V1c = { -0.5, -0.5,  0.0 },
	V2c = { -0.5,  0.5,  0.0 },
	V3c = {  0.5,  0.5,  0.0 },
	V4c = {  0.5, -0.5,  0.0 },
	Vin = {  0.0,  0.0,  0.0 };
    int Rvrsd;
    IPPolygonStruct
        *Pl1 = PrimGenPolygon4Vrtx(V1a, V2a, V3a, V4a, Vin, &Rvrsd, NULL),
        *Pl2 = PrimGenPolygon4Vrtx(V1b, V2b, V3b, V4b, Vin, &Rvrsd, NULL),
        *Pl3 = PrimGenPolygon4Vrtx(V1c, V2c, V3c, V4c, Vin, &Rvrsd, NULL);

    if (SeparatedObjs) {
        IPObjectStruct
	    *PRet = IPGenLISTObject(IPGenPOLYObject(Pl1));

	IPListObjectAppend(PRet, IPGenPOLYObject(Pl2));
	IPListObjectAppend(PRet, IPGenPOLYObject(Pl3));
	
	return PRet;
    }
    else {
        Pl1 -> Pnext = Pl2;
        Pl2 -> Pnext = Pl3;
        return IPGenPOLYObject(Pl1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets the color/gray level intesity of all the polygons in the blob       *
* according the the colors in the three input images at the designated       *
* location of the blob.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Blob:                     The blob to set its polygons' colors.          *
*                             A list of three objects.  Each of the three    *
*                             objects can be a poly or a list of polys.      * 
*   Image1, Image2, Image3:   The three input images. Image3 can be NULL.    *
*   Img1SizeX, Img1SizeY:     The size of Image1.                            *
*   Img2SizeX, Img2SizeY:     The size of Image2.                            *
*   Img3SizeX, Img3SizeY:     The size of Image3.                            *
*   DoTexture:                TRUE to add 'uvvals' attributes, FALSE to add  *
*                             actual color to the vertices.                  *
*   BlobColorMethod:   Method of coloring each blob.                         *
*   Negative:      Default (FALSE) is white blobs over dark background. If   *
*                  TRUE, assume dark blobs over white background.            *
*   Intensity:     The gray level affect on the blobs' scale.                *
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blobl  *
*                  completely in one direction which will render it          *
*                  impossible to actually manufacture it.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int UserSetBlobColorIntensity(IPObjectStruct *Blob,
				     const IrtImgPixelStruct *Image1,
				     const IrtImgPixelStruct *Image2,
				     const IrtImgPixelStruct *Image3,
				     int Img1SizeX,
				     int Img1SizeY,
				     int Img2SizeX,
				     int Img2SizeY,
				     int Img3SizeX,
				     int Img3SizeY,
				     int DoTexture,
				     UserImgShd3dBlobColorType
					                      BlobColorMethod,
				     int Negative,
				     IrtRType Intensity,
				     IrtRType MinIntensity)
{
    IPObjectStruct *Obj1, *Obj2, *Obj3;

    if (IP_IS_OLST_OBJ(Blob)) {
        Obj1 = IPListObjectGet(Blob, 0);
        Obj2 = IPListObjectGet(Blob, 1);
        Obj3 = IPListObjectGet(Blob, 2);
    }
    else {
        IRIT_WARNING_MSG("Blob does not contain three lists of polygons.");
        return FALSE;
    }

    assert(Obj1 != NULL && Obj2 != NULL);

    /* Can have two or three pictures for two/three sided objels. */
    UserSetBlobColorIntensityAux(Obj1, Image1, Img1SizeX, Img1SizeY,
				 DoTexture, BlobColorMethod, Negative,
				 Intensity, MinIntensity);
    UserSetBlobColorIntensityAux(Obj2, Image2, Img2SizeX, Img2SizeY,
				 DoTexture, BlobColorMethod, Negative,
				 Intensity, MinIntensity);
    if (Image3 != NULL && Obj3 != NULL) {
        UserSetBlobColorIntensityAux(Obj3, Image3, Img3SizeX, Img3SizeY,
				     DoTexture, BlobColorMethod, Negative,
				     Intensity, MinIntensity);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxliary function of UserSetBlobColorIntensity to set the colors/gray    *
* levels for one face.                                                       *
*****************************************************************************/
static int UserSetBlobColorIntensityAux(IPObjectStruct *PObj,
					const IrtImgPixelStruct *Image,
					int ImgSizeX,
					int ImgSizeY,
					int DoTexture, 
					UserImgShd3dBlobColorType
					                    BlobColorMethod,
					int Negative,
					IrtRType Intensity,
					IrtRType MinIntensity)
{
    int NormalAxis,
	p = 0;
    IrtRType x, y, Gray;
    IrtPtType Centroid;
    IrtImgPixelStruct RGB;
    IPObjectStruct *PTmp;
    GMBBBboxStruct
        *BBox = GMBBComputeBboxObject(PObj);

    /* Find the dimension in the bbox that is minimal (shpuld be zero)     */
    /* and set the NormalAxis to it.					   */
    NormalAxis = 0;
    if (BBox -> Max[1] - BBox -> Min[1] < BBox -> Max[0] - BBox -> Min[0])
        NormalAxis = 1;
    if (BBox -> Max[2] - BBox -> Min[2] <
	                BBox -> Max[NormalAxis] - BBox -> Min[NormalAxis])
        NormalAxis = 2;

    while (TRUE) {
        if (IP_IS_POLY_OBJ(PObj)) {
	    PTmp = PObj;
	}
	else if (IP_IS_OLST_OBJ(PObj)) {
	    if ((PTmp = IPListObjectGet(PObj, p++)) == NULL)
	        return TRUE;
	}
	else {
	    IRIT_WARNING_MSG("Blob does not contain three lists of polygons.");
	    return FALSE;
	}

	if (DoTexture) {
	    IPVertexStruct
	        *V = PTmp -> U.Pl -> PVertex;

	    do {
	        switch (NormalAxis) {
                    default:
		         assert(0);
	            case 0:
		        x = V -> Coord[1];
			y = V -> Coord[2];
			break;
	            case 1:
		        x = V -> Coord[0];
			y = V -> Coord[2];
			break;
	            case 2:
		        x = V -> Coord[0];
			y = V -> Coord[1];
			break;
		}

		AttrSetUVAttrib(&V -> Attr, "uvvals",
				x / 3.0 + NormalAxis * 2.0 / 3.0, y);

		V = V -> Pnext;
	    }
	    while (V != NULL && V != PTmp -> U.Pl -> PVertex);
	}
	else {
	    GMPolyCentroid(PTmp -> U.Pl, Centroid);
	    switch (NormalAxis) {
                default:
		    assert(0);
	        case 0:
		    x = Centroid[1];
		    y = Centroid[2];
		    break;
	        case 1:
		    x = Centroid[0];
		    y = Centroid[2];
		    break;
	        case 2:
		    x = Centroid[0];
		    y = Centroid[1];
		    break;
	    }

	    switch (BlobColorMethod) {
	        default:
		    assert(0);
	        case USER_IMG_SHD_3D_BLOB_GRAY_LEVEL:
		    Gray = UserGetImageIntensity(Image,
						 ImgSizeX + 1, ImgSizeY + 1,
						 x, y,
						 Negative, MinIntensity);

		    RGB.r = (IrtBType) (Gray * 255 * Intensity);
		    RGB.g = (IrtBType) (Gray * 255 * Intensity);
		    RGB.b = (IrtBType) (Gray * 255 * Intensity);
		    break;
	        case USER_IMG_SHD_3D_BLOB_COLOR:
		   RGB = *UserGetImageColor(Image, ImgSizeX + 1, ImgSizeY + 1,
					    x, y);
		   break;
	    }

	    RGB.r = USER_BOUND_0_255(RGB.r);
	    RGB.g = USER_BOUND_0_255(RGB.g);
	    RGB.b = USER_BOUND_0_255(RGB.b);
	    AttrSetObjectRGBColor(PTmp, RGB.r, RGB.g, RGB.b);
	}

	/* If a poly object - one polygon to handle only. */
	if (IP_IS_POLY_OBJ(PObj))
	    return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sample intensity of the given image at the specified (x, y) location.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Img:      A square image of size (ImgSize x ImgSize).                    *
*   ImgSizeX: Size of image in X.					     *
*   ImgSizeY: Size of image in Y.					     *
*   x, y:     Location where to sample image intensity, between 0 and 1.0.   *
*   Negative: Default (FALSE) is white blobs over dark background.	     *
*             If TRUE, assume dark blobs over white background.              *
*   MinIntensity:  Minimum intensity allowed.  Zero will collapse the blob   *
*             completely in one direction which will render it impossible    *
*             to actually manufacture it.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Between zero and one, reflecting on the image intensity.     *
*****************************************************************************/
static IrtRType UserGetImageIntensity(const IrtImgPixelStruct *Img,
				      int ImgSizeX,
				      int ImgSizeY,
				      IrtRType x,
				      IrtRType y,
				      int Negative,
				      IrtRType MinIntensity)
{
    const IrtImgPixelStruct
        *Pxl = UserGetImageColor(Img, ImgSizeX, ImgSizeY, x, y);
    IrtRType R;

    /* Convert the RGB to gray level. */
    R = (Pxl -> r * 0.3 + Pxl -> g * 0.59 + Pxl -> b * 0.11) / 255.0;
    if (Negative)
        R = 1.0 - R;

    return IRIT_MAX(R, MinIntensity);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sample intensity of the given image at the specified (x, y) location.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Img:      A square image of size (ImgSize x ImgSize).                    *
*   ImgSize:  Size of (square) image.					     *
*   x, y:     Location where to sample image intensity, between 0.0 and 1.0. *
*   Res:      Resolution - x, y values are between 0 and Res-1.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtImgPixelStruct *:   Color of specified pixel.			     *
*****************************************************************************/
static const IrtImgPixelStruct *UserGetImageColor(const IrtImgPixelStruct *Img,
						  int ImgSizeX,
						  int ImgSizeY,
						  IrtRType x,
						  IrtRType y)
{
    int Ix = (int) (ImgSizeX * x),
        Iy = (int) (ImgSizeY * y);

    Ix = IRIT_BOUND(Ix, 0, ImgSizeX - 1);
    Iy = IRIT_BOUND(Iy, 0, ImgSizeY - 1);

    return &Img[Ix + Iy * ImgSizeX];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorting key function for qsort (see below).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:  Two point to sort out and compute their relative order.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    1, 0, -1 based on the relation between P1 and P2.                *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int UserCompTwoReals(VoidPtr P1, VoidPtr P2)
#else
static int UserCompTwoReals(const VoidPtr P1, const VoidPtr P2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
        d = ((UserRandomVecStruct *) P1) -> R - 
            ((UserRandomVecStruct *) P2) -> R;

    return IRIT_SIGN(d);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a permutation vector of numbers between 0 and Size-1, with a     M
* desired distributed, in a vector of size Size.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:              Of random vector to create to spread the blobs.       M
*   BlobSpreadMethod:  Blob spreading method desired.                        M
*   FirstVec:          TRUE for first vector, FALSE for second vector.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int *:   Created vector.                                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DMicroBlobsCreateRandomMatrix                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DMicroBlobsCreateRandomVector                                       M
*****************************************************************************/
int *User3DMicroBlobsCreateRandomVector(int Size,
					User3DSpreadType BlobSpreadMethod,
					IrtBType FirstVec)
{
    int i, d,
	*Vec = IritMalloc(Size * sizeof(int));

    switch (BlobSpreadMethod) {
        default:
	    assert(0);
	case USER_3D_SPREAD_RANDOM:
	    {
	        UserRandomVecStruct
		    *SortVec = IritMalloc(Size * sizeof(UserRandomVecStruct));

		for (i = 0; i < Size; i++) {
		    SortVec[i].i = i;
		    SortVec[i].R = IritRandom(0, 1);
		}

		qsort(SortVec, Size, sizeof(UserRandomVecStruct),
		      UserCompTwoReals);

		for (i = 0; i < Size; i++)
		    Vec[i] = SortVec[i].i;

		IritFree(SortVec);
		break;
	    }
	case USER_3D_SPREAD_DIAG_PLANE:
	    for (i = 0; i < Size; i++)
	        Vec[i] = i;
	    break;
	case USER_3D_SPREAD_ANTI_DIAG_PLANE:
	    if (FirstVec) {
		for (i = 0; i < Size; i++)
		    Vec[i] = i;
	    }
	    else {
		for (i = 0; i < Size; i++)
		    Vec[i] = Size - i - 1;
	    }
	    break;
	case USER_3D_SPREAD_DISCONT2PLANE:
	    for (i = 0; i < Size; i++)
	        Vec[i] = i;
	    if (FirstVec) {
	        int d = Size - Size / 2;

	        for (i = 0; i < Size / 2; i++)
		    IRIT_SWAP(int, Vec[i], Vec[i + d]);
	    }
	    break;
	case USER_3D_SPREAD_DISCONT4PLANE:
	    for (i = 0; i < Size; i++)
	        Vec[i] = i;
	    if (FirstVec) {
	        d = Size - Size / 4;
		for (i = 0; i < Size / 4; i++)
		    IRIT_SWAP(int, Vec[i], Vec[i + d]);

		d = 3 * Size / 4 - Size / 2;
		for (i = Size / 4; i < Size / 2; i++)
		    IRIT_SWAP(int, Vec[i], Vec[i + d]);
	    }
	    break;
    }
    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a matrix of size (Size x Size) of numbers between 0 and Size-1,  M
* randomly distributed so that no row or columns has the same number twice.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:              Of random vector to create to spread the blobs.       M
*   BlobSpreadMethod:  Blob spreading method desired.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int **:   Created matrix as vector of vectors.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DMicroBlobsCreateRandomVector                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DMicroBlobsCreateRandomMatrix                                       M
*****************************************************************************/
int **User3DMicroBlobsCreateRandomMatrix(int Size,
					 User3DSpreadType BlobSpreadMethod)
{
    int i, j, k, l, Size2, Size4, *Col,
	**Mat = IritMalloc(Size * sizeof(int *));

    switch (BlobSpreadMethod) {
        default:
	    assert(0);
	case USER_3D_SPREAD_RANDOM:
	    /* Do the first vector to serve as first row. */
	    Mat[0] = User3DMicroBlobsCreateRandomVector(Size,
							BlobSpreadMethod,
							FALSE);

	    /* Do a second vector to serve as columns.  Each i'th column  */
	    /* will be cyclic-shifted so its first entry identifies with  */
	    /* the entry of Mat[0][i], before placed as the ith column.   */
	    for (i = 1; i < Size; i++)
	        Mat[i] = IritMalloc(Size * sizeof(int));

	    Col = User3DMicroBlobsCreateRandomVector(Size,
						     BlobSpreadMethod, FALSE);

	    /* Copy the column, one at a time, shifted as necessary. */
	    for (j = 0; j < Size; j++) {
	        for (i = 0; i < Size; i++) {
		    /* Find where Mat[0][i] shows up in Col. */
		    if (Mat[0][i] == Col[j])
		        break;
		}
		assert (i < Size);

		/* Copy the column from j to Size-1 and then 0 to j-1. */
		for (k = j, l = 0; k < Size; k++, l++)
		    Mat[l][i] = Col[k];
		for (k = 0; k < j; k++, l++)
		    Mat[l][i] = Col[k];
	    }

	    IritFree(Col);

#ifdef DEBUG
	    /* Verify the solution. */
	    for (i = 0; i < Size; i++) {
	        for (j = 0; j < Size; j++) {
		    for (k = j + 1; k < Size; k++) {
		        assert(Mat[i][j] != Mat[i][k] &&
			       Mat[j][i] != Mat[k][i]);
		    }
		}
	    }
#endif /* DEBUG */
	    break;
	case USER_3D_SPREAD_DIAG_PLANE:
	    for (i = 0; i < Size; i++)
		Mat[i] = IritMalloc(Size * sizeof(int));

	    for (i = 0; i < Size; i++) {
		for (j = 0; j < Size; j++) {
		    Mat[i][j] = -1 - i - j;
		    while (Mat[i][j] < 0)
			Mat[i][j] += Size;
		}
	    }
	    break;
	case USER_3D_SPREAD_ANTI_DIAG_PLANE:
	    for (i = 0; i < Size; i++)
	        Mat[i] = IritMalloc(Size * sizeof(int));

	    for (i = 0; i < Size; i++) {
		for (j = 0; j < Size; j++) {
		    Mat[i][j] = i + j;
		    while (Mat[i][j] >= Size)
			Mat[i][j] -= Size;
		}
	    }
	    break;
	case USER_3D_SPREAD_DISCONT2PLANE:
	    for (i = 0; i < Size; i++)
		Mat[i] = IritMalloc(Size * sizeof(int));

	    Size2 = Size / 2 - 1;
	    for (i = 0; i < Size; i++) {
		for (j = 0; j < Size; j++) {
		    Mat[i][j] = Size2 - i - j;
		    while (Mat[i][j] < 0)
			Mat[i][j] += Size;
		}
	    }
	    break;
	case USER_3D_SPREAD_DISCONT4PLANE:
	    for (i = 0; i < Size; i++)
	        Mat[i] = IritMalloc(Size * sizeof(int));

	    Size4 = Size / 4 - 1;
	    for (i = 0; i < Size; i++) {
	        for (j = 0; j < Size; j++) {
		    Mat[i][j] = Size4 - i - j;
		    while (Mat[i][j] < 0)
			Mat[i][j] += Size;

		}
	    }
	    break;
    }

    return Mat;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates micro blobs for Resolution^2 ellipsoidal blobs that looks like   M
* the 1st image (gray level) from the XZ plane and like the 2nd image from   M
* the YZ plane.	 The entire constructed goemetry is confined to a world      M
* cube space of [0, CubeSize]^3.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:    Name of 1st image to load.                                M
*   Image2Name:    Name of 2nd image to load.                                M
*   Image3Name:    Name of 3rd image to load.  Optional (Can be NULL).       M
*   BlobSpreadMethod:  Method of spreading the blobs.                        M
*   Intensity:     A scale affect on the blobs' scale.	                     M
*   MicroBlobSpacing:  Spacing to use in the micro blob, in world space      M
*		   coordinates.						     M
*   RandomFactors: Maximal allowed randomization in XYZ, in world space      M
*		   coordinates.						     M
*   Resolution:    Resolution of created objects (Resolution^2 ellipsoidal   M
*		   blobs are created).					     M
*   Negative:      Default (FALSE) is white blobs over dark background. If   M
*                  TRUE, assume dark blobs over white background.            M
*   CubeSize:      Size of output.					     M
*   MergePts:      TRUE to merge all points to one list, FALSE each of the   M
8                  Resolution^2 blobs will hold its own point list.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Resolution^2 pointlists of micro blobs of Resolution^2 M
*		      spherical blobs.			                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DMicroBlobsTiling                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DMicroBlobsFrom3Images                                              M
*****************************************************************************/
IPObjectStruct *User3DMicroBlobsFrom3Images(const char *Image1Name,
					    const char *Image2Name,
					    const char *Image3Name,
					    User3DSpreadType BlobSpreadMethod,
					    IrtRType Intensity,
					    const IrtVecType MicroBlobSpacing,
					    const IrtVecType RandomFactors,
					    int Resolution,
					    int Negative,
					    IrtRType CubeSize,
					    int MergePts)
{
    int i, j, Image1X, Image1Y, Image1Alpha, Image2X, Image2Y, Image2Alpha,
        Image3X, Image3Y, Image3Alpha, **M;
    IrtRType
	Resolution1 = 1.0 / (Resolution + IRIT_UEPS);
    IrtVecType MicroBlobScaledSpacing;
    IrtImgPixelStruct
        *Image1 = IrtImgReadImage(Image1Name, &Image1X, &Image1Y, &Image1Alpha),
        *Image2 = IrtImgReadImage(Image2Name, &Image2X, &Image2Y, &Image2Alpha),
        *Image3 = Image3Name == NULL ? NULL :
                  IrtImgReadImage(Image3Name, &Image3X, &Image3Y, &Image3Alpha);
    IPObjectStruct
        *RetVal = IPGenPointListObject("MicroBlobs", NULL, NULL);

    /* Do we have a valid pair of images? */
    if (Image1 == NULL || Image2 == NULL) {
        if (Image1 != NULL)
	    IritFree(Image1);
        if (Image2 != NULL)
	    IritFree(Image2);
	if (Image3 != NULL)
	    IritFree(Image3);

	USER_FATAL_ERROR(USER_ERR_INVALID_IMAGE_SIZE);
	return NULL;
    }

    /* Are they both square? */
    if (Image1X != Image1Y ||
	Image2X != Image2Y ||
	(Image3 != NULL && Image3X != Image3Y)) {
        IritFree(Image1);
	IritFree(Image2);
	if (Image3 != NULL)
	    IritFree(Image3);

	USER_FATAL_ERROR(USER_ERR_INVALID_IMAGE_SIZE);
        return NULL;
    }

    if (Resolution <= 0)
	return NULL;

    /* Compensate the micro blob spacing for the resolution. In the         */
    /* User3DMicroBlobsTiling below, the blob spans [-1, 1]^3.              */
    /* Here, the final blob will be of size CubeSize/Resolution.	    */
    IRIT_VEC_COPY(MicroBlobScaledSpacing, MicroBlobSpacing);
    IRIT_VEC_SCALE(MicroBlobScaledSpacing, Resolution / (CubeSize * 0.5));

    if (Image3 != NULL) {
        M = User3DMicroBlobsCreateRandomMatrix(Resolution, BlobSpreadMethod);

	/* Build the basic geometry for all XY: */
	for (i = 0; i < Resolution; i++) {
	    IrtRType I1, I2, I3;
	    IrtHmgnMatType TrnsMat, SclMat, Mat;
	    IPPolygonStruct *Pts, *TransPts;

	    for (j = 0; j < Resolution; j++) {
	        I1 = Intensity * UserGetImageIntensity(Image1,
					   Image1X + 1, Image1Y + 1,
					   i / ((IrtRType) Resolution),
					   M[i][j] / ((IrtRType) Resolution),
					   Negative, USER_MIN_IMAGE_INTESITY);
		I2 = Intensity * UserGetImageIntensity(Image2,
					   Image2X + 1, Image2Y + 1,
					   j / ((IrtRType) Resolution),
					   M[i][j] / ((IrtRType) Resolution),
					   Negative, USER_MIN_IMAGE_INTESITY);
		if (Image3 != NULL)
		    I3 = Intensity * UserGetImageIntensity(Image3,
					   Image3X + 1, Image3Y + 1,
					   i / ((IrtRType) Resolution),
					   j / ((IrtRType) Resolution),
					   Negative, USER_MIN_IMAGE_INTESITY);
		else
		    I3 = -1.0;

		if ((Pts = User3DMicroBlobsTiling2(I1, I2, I3,
						   MicroBlobScaledSpacing,
						   RandomFactors)) == NULL)
		    continue;

		/* Normalize to cube size and place at right location.      */
		/* The ellipsoidal blob is at (Vec1[i], Vec2[i], ZLevel).   */
		MatGenMatTrans((i + 0.5) * Resolution1 * CubeSize,
			       (j + 0.5) * Resolution1 * CubeSize,
			       (M[i][j] + 0.5) * Resolution1 * CubeSize,
			       TrnsMat);
		MatGenMatUnifScale(Resolution1 * CubeSize * 0.5, SclMat);
		MatMultTwo4by4(Mat, SclMat, TrnsMat);

		TransPts = GMTransformPolyList(Pts, Mat, FALSE);
		IPFreePolygon(Pts);

		if (MergePts) {
		    if (RetVal -> U.Pl == NULL)
		        RetVal -> U.Pl = TransPts;	    /* First entry. */
		    else {
		        IPGetLastVrtx(TransPts -> PVertex) -> Pnext =
		                                    RetVal -> U.Pl -> PVertex;
			RetVal -> U.Pl -> PVertex = TransPts -> PVertex;
			TransPts -> PVertex = NULL;
			IPFreePolygon(TransPts);
		    }
		}
		else {
		    IRIT_LIST_PUSH(TransPts, RetVal -> U.Pl);
		}
	    }
	}

	for (i = 0; i < Resolution; i++)
	    IritFree(M[i]);
	IritFree(M);
    }
    else {
        int z;
        IrtRType I1, I2,
	    I3 = -1.0;

        /* Build the basic geometry at each z level: */
        for (z = 0; z < Resolution; z++) {
	    int i,
	        *Vec1 = User3DMicroBlobsCreateRandomVector(Resolution,
							   BlobSpreadMethod,
							   TRUE),
	        *Vec2 = User3DMicroBlobsCreateRandomVector(Resolution,
							   BlobSpreadMethod,
							   FALSE);
	    IrtHmgnMatType TrnsMat, SclMat, Mat;
	    IPPolygonStruct *Pts, *TransPts;

	    for (i = 0; i < Resolution; i++) {
	        I1 = Intensity * UserGetImageIntensity(Image1,
					   Image1X + 1, Image1Y + 1,
					   Vec1[i] / ((IrtRType) Resolution),
					   z / ((IrtRType) Resolution),
				           Negative, USER_MIN_IMAGE_INTESITY);
		I2 = Intensity * UserGetImageIntensity(Image2,
					   Image2X + 1, Image2Y + 1,
					   Vec2[i] / ((IrtRType) Resolution),
					   z / ((IrtRType) Resolution),
					   Negative, USER_MIN_IMAGE_INTESITY);

		if ((Pts = User3DMicroBlobsTiling2(I1, I2, I3,
						   MicroBlobScaledSpacing,
						   RandomFactors)) == NULL)
		    continue;

		/* Normalize to cube size and place at right location.      */
		/* The ellipsoidal blob is at (Vec1[i], Vec2[i], ZLevel).   */
		MatGenMatTrans((Vec1[i] + 0.5) * Resolution1 * CubeSize,
			       (Vec2[i] + 0.5) * Resolution1 * CubeSize,
			       (z + 0.5) * Resolution1 * CubeSize,
			       TrnsMat);
		MatGenMatUnifScale(Resolution1 * CubeSize * 0.5, SclMat);
		MatMultTwo4by4(Mat, SclMat, TrnsMat);

		TransPts = GMTransformPolyList(Pts, Mat, FALSE);
		IPFreePolygon(Pts);

		if (MergePts) {
		    if (RetVal -> U.Pl == NULL)
		        RetVal -> U.Pl = TransPts;	    /* First entry. */
		    else {
		        IPGetLastVrtx(TransPts -> PVertex) -> Pnext =
		                                    RetVal -> U.Pl -> PVertex;
			RetVal -> U.Pl -> PVertex = TransPts -> PVertex;
			TransPts -> PVertex = NULL;
			IPFreePolygon(TransPts);
		    }
		}
		else {
		    IRIT_LIST_PUSH(TransPts, RetVal -> U.Pl);
		}
	    }

	    IritFree(Vec1);
	    IritFree(Vec2);
	}
    }

    IritFree(Image1);
    IritFree(Image2);
    if (Image3 != NULL)
	IritFree(Image3);

    if (RetVal -> U.Pl != NULL)
        return RetVal;

    IPFreeObject(RetVal);
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Insert a new blobl in an existing blobl list, testing that no existing   *
* blob is close than speficied MinDist.                                      *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Blob:        New blob to insert into BlobsList.                          *
*   BlobsList:   Existing list of blobs.                                     *
*   MinDist:     Minimal distance to allow two neighboring blobs.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if Blob inserted to the BloblsList, FALSE if not.          *
*****************************************************************************/
static int UserInsertNewMicroBlob(IPVertexStruct *Blob,
				  IPVertexStruct **BlobsList,
				  IrtRType MinDist)
{
    IPVertexStruct *B;

    for (B = *BlobsList; B != NULL; B = B -> Pnext) {
        if (IRIT_PT_APX_EQ_EPS(Blob -> Coord, B -> Coord, MinDist)) {
	    IPFreeVertex(Blob);
	    return FALSE;
	}
    }

    IRIT_LIST_PUSH(Blob, *BlobsList);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tile a given blob in the shape of an ellipsoid bounded by [-1, 1]^3, by  M
* micro blobs with XYZ spacing as prescribed by ??Intensity/MicroBlobSpacing.M
*                                                                            *
* PARAMETERS:                                                                M
*   XZIntensity:       Intensity (0 to 1) of blob when viewed from XZ dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   YZIntensity:       Intensity (0 to 1) of blob when viewed from YZ dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   XYIntensity:       Intensity (0 to 1) of blob when viewed from XY dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   MicroBlobSpacing:  XYZ spacing between micro blobs.                      M
*   RandomFactors:     Maximal randomization factors to use on micro blobs.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  A pointlist of centers of the tiling micro blobs.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DMicroBlobsFrom2Images, User3DMicroBlobsTiling2                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DMicroBlobsTiling                                                   M
*****************************************************************************/
IPPolygonStruct *User3DMicroBlobsTiling(IrtRType XZIntensity,
					IrtRType YZIntensity,
					IrtRType XYIntensity,
					const IrtVecType MicroBlobSpacing,
					const IrtVecType RandomFactors)
{
    IrtRType x, y, z, A, MinDistance;
    IPVertexStruct *Cntr;
    IPPolygonStruct
        *Pts = IPAllocPolygon(0, NULL, NULL);

    IritRandomInit(301060);

    /* Estimate a reasonable minimal distance to allow between blobs. */
    MinDistance = (IRIT_MIN(MicroBlobSpacing[0],
			    IRIT_MIN(MicroBlobSpacing[1],
				     MicroBlobSpacing[2])) -
		   IRIT_MAX(RandomFactors[0],
			    IRIT_MAX(RandomFactors[1],
				     RandomFactors[2]))) * 0.9;

    /* Compute values of "A X^2 + Z^2 - 1.0" expression & seek sign changes.*/
    if (XZIntensity >= 0.0) {
        A = 1.0 / (IRIT_SQR(XZIntensity) + IRIT_EPS);
	for (x = -1.0;
	     x <= 1.0 + MicroBlobSpacing[0] * 0.5;
	     x += MicroBlobSpacing[0]) {
	    for (z = -1.0;
		 z <= 1.0 + MicroBlobSpacing[2] * 0.5;
		 z += MicroBlobSpacing[2]) {
	        if (A * IRIT_SQR(x) + IRIT_SQR(z) - 1.0 <= 0.0) {
		    /* We are in the blob - gen. a micro blob.  */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = x + IritRandom(-RandomFactors[0],
						      RandomFactors[0]);
		    Cntr -> Coord[1] = IritRandom(-RandomFactors[1],
						  RandomFactors[1]);
		    Cntr -> Coord[2] = z + IritRandom(-RandomFactors[2],
						      RandomFactors[2]);
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }

    /* Compute values of "A y^2 + Z^2 - 1.0" expression & seek sign changes.*/
    if (YZIntensity >= 0.0) {
        A = 1.0 / (IRIT_SQR(YZIntensity) + IRIT_EPS);
	for (y = -1.0;
	     y <= 1.0 + MicroBlobSpacing[1] * 0.5;
	     y += MicroBlobSpacing[1]) {
	    for (z = -1.0;
		 z <= 1.0 + MicroBlobSpacing[2] * 0.5;
		 z += MicroBlobSpacing[2]) {
	        if (A * IRIT_SQR(y) + IRIT_SQR(z) - 1.0 <= 0.0) {
		    /* We are in the blob - gen. a micro blob.  */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = IritRandom(-RandomFactors[0],
						  RandomFactors[0]);
		    Cntr -> Coord[1] = y + IritRandom(-RandomFactors[1],
						      RandomFactors[1]);
		    Cntr -> Coord[2] = z + IritRandom(-RandomFactors[2],
						      RandomFactors[2]);
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }

    /* Compute values of "A y^2 + x^2 - 1.0" expression & seek sign changes. */
    if (XYIntensity >= 0.0) {
        A = 1.0 / (IRIT_SQR(XYIntensity) + IRIT_EPS);
	for (x = -1.0;
	     x <= 1.0 + MicroBlobSpacing[0] * 0.5;
	     x += MicroBlobSpacing[0]) {
	    for (y = -1.0;
		 y <= 1.0 + MicroBlobSpacing[1] * 0.5;
		 y += MicroBlobSpacing[1]) {
	        if (A * IRIT_SQR(y) + IRIT_SQR(x) - 1.0 <= 0.0) {
		    /* We are in the blob - gen. a micro blob.  */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = x + IritRandom(-RandomFactors[0],
						      RandomFactors[0]);
		    Cntr -> Coord[1] = y + IritRandom(-RandomFactors[1],
						      RandomFactors[1]);
		    Cntr -> Coord[2] = IritRandom(-RandomFactors[2],
						  RandomFactors[2]);
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }

    if (Pts -> PVertex != NULL)
        return Pts;

    IPFreePolygon(Pts);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tile a given blob in the shape of a randomized cubed in [-1, 1]^3, by    M
* micro blobs with XYZ spacing as prescribed by ??Intensity/MicroBlobSpacing.M
*                                                                            *
* PARAMETERS:                                                                M
*   XZIntensity:       Intensity (0 to 1) of blob when viewed from XZ dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   YZIntensity:       Intensity (0 to 1) of blob when viewed from YZ dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   XYIntensity:       Intensity (0 to 1) of blob when viewed from XY dir.   M
*                      Can be invalidated and ignored if negative.	     M
*   MicroBlobSpacing:  XYZ spacing between micro blobs.                      M
*   RandomFactors:     Maximal randomization factors to use on micro blobs.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  A pointlist of centers of the tiling micro blobs.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DMicroBlobsFrom2Images, User3DMicroBlobsTiling                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DMicroBlobsTiling2                                                  M
*****************************************************************************/
IPPolygonStruct *User3DMicroBlobsTiling2(IrtRType XZIntensity,
					 IrtRType YZIntensity,
					 IrtRType XYIntensity,
					 const IrtVecType MicroBlobSpacing,
					 const IrtVecType RandomFactors)
{
    IrtRType x, y, z, MinDistance;
    IPVertexStruct *Cntr;
    IPPolygonStruct
        *Pts = IPAllocPolygon(0, NULL, NULL);

    /* Estimate a reasonable minimal distance to allow between blobs. */
    MinDistance = IRIT_MIN(MicroBlobSpacing[0],
			   IRIT_MIN(MicroBlobSpacing[1],
				    MicroBlobSpacing[2])) -
		  IRIT_MAX(RandomFactors[0],
			   IRIT_MAX(RandomFactors[1],
				    RandomFactors[2])) * 2.1;

    IritRandomInit(301060);

    /* Compute all microblobs in this position and pick them based on       */
    /* randomized intensity.						    */
    if (XZIntensity > 0.0) {
	for (x = -1.0;
	     x <= 1.0 - MicroBlobSpacing[0] * 0.5;
	     x += MicroBlobSpacing[0]) {
	    for (z = -1.0;
		 z <= 1.0 - MicroBlobSpacing[2] * 0.5;
		 z += MicroBlobSpacing[2]) {
	        if (IritRandom(0.0, 1.0) < XZIntensity) {
		    /* Generate a micro blob. */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = x + IritRandom(-RandomFactors[0],
						      RandomFactors[0]);
		    Cntr -> Coord[1] = 0.0;
		    Cntr -> Coord[2] = z + IritRandom(-RandomFactors[2],
						      RandomFactors[2]);
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }


    /* Compute all microblobs in this position and pick them based on       */
    /* randomized intensity.						    */
    if (YZIntensity > 0.0) {
	for (y = -1.0;
	     y <= 1.0 - MicroBlobSpacing[1] * 0.5;
	     y += MicroBlobSpacing[1]) {
	    for (z = -1.0;
		 z <= 1.0 - MicroBlobSpacing[2] * 0.5;
		 z += MicroBlobSpacing[2]) {
	        if (IritRandom(0.0, 1.0) < YZIntensity) {
		    /* Generate a micro blob. */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = 0.0;
		    Cntr -> Coord[1] = y + IritRandom(-RandomFactors[1],
						      RandomFactors[1]);
		    Cntr -> Coord[2] = z + IritRandom(-RandomFactors[2],
						      RandomFactors[2]);
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }

    /* Compute all microblobs in this position and pick them based on       */
    /* randomized intensity.						    */
    if (XYIntensity > 0.0) {
	for (x = -1.0;
	     x <= 1.0 - MicroBlobSpacing[0] * 0.5;
	     x += MicroBlobSpacing[0]) {
	    for (y = -1.0;
		 y <= 1.0 - MicroBlobSpacing[1] * 0.5;
		 y += MicroBlobSpacing[1]) {
	        if (IritRandom(0.0, 1.0) < XYIntensity) {
		    /* Generate a micro blob. */
		    Cntr = IPAllocVertex2(Pts -> PVertex);
		    Cntr -> Coord[0] = x + IritRandom(-RandomFactors[0],
						      RandomFactors[0]);
		    Cntr -> Coord[1] = y + IritRandom(-RandomFactors[1],
						      RandomFactors[1]);
		    Cntr -> Coord[2] = 0.0;
		    UserInsertNewMicroBlob(Cntr, &Pts -> PVertex, MinDistance);
		}
	    }
	}
    }

    if (Pts -> PVertex != NULL)
        return Pts;

    IPFreePolygon(Pts);

    return NULL;
}
