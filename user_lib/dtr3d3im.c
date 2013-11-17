/******************************************************************************
* dtr3d3im.c - dither a 3d bolbs object that mimics 3 orthogonal images.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber,					 July 2010.   *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "user_loc.h"

#include "dither3d_2x2x2_3d.h"
#include "dither3d_3x3x3_3d.h"

#define USER_3DDITHER_RGB2GRAY(Pxl) \
			(Pxl -> r * 0.3 + Pxl -> g * 0.59 + Pxl -> b * 0.11)

#define USER_3DDTR_IMG1(x, y)	IC -> Img1Dtr[x + y * IC -> Size]
#define USER_3DDTR_IMG2(x, y)	IC -> Img2Dtr[x + y * IC -> Size]
#define USER_3DDTR_IMG3(x, y)	IC -> Img3Dtr[x + y * IC -> Size]
#define USER_3DDTR_IMG3SAFE(x, y)	(IC -> Img3Dtr == NULL || \
					 USER_3DDTR_IMG3(x, y))

#define USER_3DDTR_CVR1(x, y)	IC -> CoverImg1[x + y * IC -> Size]
#define USER_3DDTR_CVR2(x, y)	IC -> CoverImg2[x + y * IC -> Size]
#define USER_3DDTR_CVR3(x, y)	IC -> CoverImg3[x + y * IC -> Size]

/* How much to divide 256 (unsigned char) by, to get the 4, 9, or 16 levels. */
IRIT_STATIC_DATA const int
    DitherIntensityLevels[5] = { 0, 0, 4, 9, 16 };
IRIT_STATIC_DATA const IrtRType
    DitherIntensityDivide[5] = { 0.0, 0.0, 51.01, 25.51, 15.01 };

typedef struct User3DDtrPxlRndmSortStruct {
    short Idx;
    short Rndm;
} User3DDtrPxlRndmSortStruct;

typedef struct User3DDtrImgCoverStruct {
    IrtBType *Img1Dtr;
    IrtBType *Img2Dtr;
    IrtBType *Img3Dtr;
    int *CoverVec;
    int *CoverImg1;
    int *CoverImg2;
    int *CoverImg3;
    int Size;
} User3DDtrImgCoverStruct;

static IPVertexStruct *User3DDitherGetMatrix(int Inten1,
					     int Inten2,
					     int Inten3,
					     int DitherSize,
					     int *Error,
					     IrtPtType XYZShifts);
static int User3DDtrFetchPixelCovers(User3DDtrImgCoverStruct *IC,
				     int x,
				     int y,
				     int z);
static int User3DDtrUpdateCover(User3DDtrImgCoverStruct *IC,
				int x,
				int y,
				int z,
				int AugmentContrast,
				int MatchWidth,
				User3DSpreadType SpreadMethod,
				IPVertexStruct **Pixels);
static void User3DDtrSetPixel(User3DDtrImgCoverStruct *IC,
			      int i,
			      int x,
			      int y,
			      int z,
			      IPVertexStruct **Pixels);
static int User3DDtrGetIthCoverCoord(User3DDtrImgCoverStruct *IC,
				     int i,
				     int *x,
				     int *y,
				     int *z);
static void User3DDtrFilterCoverSpread(User3DDtrImgCoverStruct *IC,
				       int x,
				       int y,
				       int z,
				       int MatchWidth,
				       User3DSpreadType SpreadMethod);
static User3DDtrImgCoverStruct *User3DDtrAllocImgCover(IrtImgPixelStruct *Img1,
						       IrtImgPixelStruct *Img2,
						       IrtImgPixelStruct *Img3,
						       int Size,
						       int DitherSize);
static void User3DDtrFreeImgCover(User3DDtrImgCoverStruct *IC);

#if defined(ultrix) && defined(mips)
static int User3DDtrSortPixels(void *PP1, void *PP2);
#else
static int User3DDtrSortPixels(const void *PP1, const void *PP2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns the best fitted dithering matrix to the given three intensities, *
* as a list of vertices at the turned-on micro-pixels.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Inten1, Inten2, Inten3: Three intensity levels of three pixels in three  *
*                           different rows.				     *
*   DitherSize:             2 (2x2x2), or 3 (3x3x3) dithering size.          *
*   Error:                  Computed distance to nearest valid 3D matrix.    *
*   XYZShifts:              XYZ shifts of synthesized vertices in dithering  *
*                           matrix.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:    3D Matrix of size DitherSize, best fitted to given  *
*                        intensities, as vertices at on micro-pixels.	     *
*****************************************************************************/
static IPVertexStruct *User3DDitherGetMatrix(int Inten1,
					     int Inten2,
					     int Inten3,
					     int DitherSize,
					     int *Error,
					     IrtPtType XYZShifts)
{
    const char *p;
    const unsigned char *u;
    int x, y, z, n, b, I1, I2, I3;
    IPVertexStruct *V,
        *Vrtcs = NULL;

    /* Get the dithering matrix. */
    switch (DitherSize) {
        case 2:
	    assert(Inten1 >= 0 && Inten1 <= DitherIntensityLevels[DitherSize] &&
		   Inten2 >= 0 && Inten2 <= DitherIntensityLevels[DitherSize] &&
		   Inten3 >= 0 && Inten3 <= DitherIntensityLevels[DitherSize]);

	    /* Get the closest valid matrix. */
	    p = Dither3Imgs3DySize2Penalty[Inten1][Inten2][Inten3];
	    I1 = Inten1 + p[0];
	    I2 = Inten2 + p[1];
	    I3 = Inten3 + p[2];
	    assert(I1 >= 0 && I1 <= DitherIntensityLevels[DitherSize] &&
		   I2 >= 0 && I2 <= DitherIntensityLevels[DitherSize] &&
		   I3 >= 0 && I3 <= DitherIntensityLevels[DitherSize]);

	    u = Dither3Imgs3DSize2Matrices[I1][I2][I3];
	    break;
        case 3:
	    assert(Inten1 >= 0 && Inten1 <= DitherIntensityLevels[DitherSize] &&
		   Inten2 >= 0 && Inten2 <= DitherIntensityLevels[DitherSize] &&
		   Inten3 >= 0 && Inten3 <= DitherIntensityLevels[DitherSize]);

	    /* Get the closest valid matrix. */
	    p = Dither3Imgs3DySize3Penalty[Inten1][Inten2][Inten3];
	    I1 = Inten1 + p[0];
	    I2 = Inten2 + p[1];
	    I3 = Inten3 + p[2];
	    assert(I1 >= 0 && I1 <= DitherIntensityLevels[DitherSize] &&
		   I2 >= 0 && I2 <= DitherIntensityLevels[DitherSize] &&
		   I3 >= 0 && I3 <= DitherIntensityLevels[DitherSize]);

	    u = Dither3Imgs3DSize3Matrices[I1][I2][I3];
	    break;
        default:
	    assert(0);
	    return NULL;
    }

    *Error = IRIT_ABS(p[0]) + IRIT_ABS(p[1]) + IRIT_ABS(p[2]);

    /* Build the vertices. */
    for (z = n = b = 0; z < DitherSize; z++) {
        for (y = 0; y < DitherSize; y++) {
	    for (x = 0; x < DitherSize; x++, n++) {
	        if (u[n] != 0) {
		    V = IPAllocVertex2(Vrtcs);
		    Vrtcs = V;
		    V -> Coord[0] = x + XYZShifts[0];
		    V -> Coord[1] = y + XYZShifts[1];
		    V -> Coord[2] = z + XYZShifts[2];
		    b++;
		}
	    }
	}
    }

    assert((I1 == 0 && I2 == 0 && I3 == 0) || b > 0);

    return Vrtcs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Build a 3D models consisting of pixels/spherical blobs that looks like   M
* the 1st image (gray level) from the XZ plane, like the 2nd image from the  M
* YZ plane and like the 3rd image from the XY plane.			     M
*   The entire constructed geometry is confined to a cube world space of     M
* [ImageWidth]^3   (ImageWidth = ImageHeight).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:  Name of 1st image to load.                                  M
*   Image2Name:  Name of 2nd image to load.                                  M
*   Image3Name:  Name of 3rd image to load.                                  M
*   DitherSize:  2 or 3 for (2x2x2) or (3x3x3) dithering matrices.           M
*   MatchWidth:  Width to allow matching in a row:			     M
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. M
*   Negate:      TRUE to negate the images.				     M
*   AugmentContrast:  Number of iterations to add micro-pixels, to augment   M
*                the contrast, behind existing pixels.  Zero to disable.     M
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random M
*                spread to use.						     M
*   SphereRad:   Radius of construct spherical blob, zero to return points.  M
*   AccumPenalty:  Returns the accumulated error in the dithering-matching,  M
*		 where zero designates no error.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Center points.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DDither3Images                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DDither3Images2                                                     M
*****************************************************************************/
IPObjectStruct *User3DDither3Images2(const char *Image1Name,
				     const char *Image2Name,
				     const char *Image3Name,
				     int DitherSize,
				     int MatchWidth,
				     int Negate,
				     int AugmentContrast,
				     User3DSpreadType SpreadMethod,
				     IrtRType SphereRad,
				     IrtRType *AccumPenalty)
{
    int x, y, z, Img1MaxX, Img1MaxY, Img1Alpha, Img2MaxX, Img2MaxY, Img2Alpha,
        Img3MaxX, Img3MaxY, Size, Img3Alpha, *Penalties, *p, **M;
    IrtImgPixelStruct
        *Img1 = IrtImgReadImage(Image1Name, &Img1MaxX, &Img1MaxY, &Img1Alpha),
	*Img2 = IrtImgReadImage(Image2Name, &Img2MaxX, &Img2MaxY, &Img2Alpha),
	*Img3 = IrtImgReadImage(Image3Name, &Img3MaxX, &Img3MaxY, &Img3Alpha);
    IrtRType Div;
    IrtPtType XYZShifts;
    IPVertexStruct *BlobVrtcs,
        *AllVrtcs = NULL;

    IritRandomInit(301060);

    *AccumPenalty = 0;

    /* Make sure all input is valid - images are squares of same size, etc. */
    if (Img1 == NULL || Img2 == NULL || Img3 == NULL) {
	if (Img1 != NULL)
	    IritFree(Img1);
	if (Img2 != NULL)
	    IritFree(Img2);
	if (Img3 != NULL)
	    IritFree(Img3);
	return NULL;
    }
    if (DitherSize < 2 || DitherSize > 3 ||
	Img1MaxX != Img1MaxY ||
	Img1MaxX != Img2MaxX ||
	Img1MaxX != Img2MaxY ||
	Img1MaxX != Img3MaxX ||
	Img1MaxX != Img3MaxY) {
	IritFree(Img1);
	IritFree(Img2);
	IritFree(Img3);
        return NULL;
    }
    Size = Img1MaxX + 1;

    if ((M = User3DMicroBlobsCreateRandomMatrix(Size, SpreadMethod)) == NULL)
        return NULL;

    /* Build the 3D array. */
    Penalties = p = (int *) IritMalloc(sizeof(int) * IRIT_CUBE(Size));
    Div = DitherIntensityDivide[DitherSize];
    for (x = 0; x < Size; x++) {
        XYZShifts[0] = x * DitherSize;
        for (y = 0; y < Size; y++) {
	    int Inten1, Inten2, Inten3,
	        Error = 0;
	    const IrtImgPixelStruct *Pxl1, *Pxl2, *Pxl3;

	    XYZShifts[1] = y * DitherSize;

	    z = M[x][y];
	    XYZShifts[2] = z * DitherSize;

	    Pxl1 = &Img1[x + z * Size];
	    Pxl2 = &Img2[y + z * Size];
	    Pxl3 = &Img3[x + y * Size];

	    if (Negate) {
	        Inten1 = (int) ((255 - USER_3DDITHER_RGB2GRAY(Pxl1)) / Div);
		Inten2 = (int) ((255 - USER_3DDITHER_RGB2GRAY(Pxl2)) / Div);
		Inten3 = (int) ((255 - USER_3DDITHER_RGB2GRAY(Pxl3)) / Div);
	    }
	    else {
	        Inten1 = (int) (USER_3DDITHER_RGB2GRAY(Pxl1) / Div);
		Inten2 = (int) (USER_3DDITHER_RGB2GRAY(Pxl2) / Div);
		Inten3 = (int) (USER_3DDITHER_RGB2GRAY(Pxl3) / Div);
	    }

	    BlobVrtcs = User3DDitherGetMatrix(Inten1, Inten2, Inten3,
					      DitherSize, &Error,
					      XYZShifts);
	    *AccumPenalty += Error;

	    AllVrtcs = IPAppendVrtxLists(BlobVrtcs, AllVrtcs);
	}
    }

    AllVrtcs = User3DDitherSetXYTranslations(AllVrtcs);

    for (x = 0; x < Size; x++)
        IritFree(M[x]);
    IritFree(M);

    IritFree(Img1);
    IritFree(Img2);
    IritFree(Img3);

    return IPGenPOINTLISTObject(IPAllocPolygon(0, AllVrtcs, NULL));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Build a 3D models consisting of points/spherical blobs that looks like   M
* the 1st image (gray level) from the XZ plane, like the 2nd image from the  M
* YZ plane and, optionally, like the 3rd image from the XY plane.	     M
*   The entire constructed geometry is confined to a cube world space of     M
* [ImageWidth]^3   (ImageWidth = ImageHeight).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:  Name of 1st image to load.                                  M
*   Image2Name:  Name of 2nd image to load.                                  M
*   Image3Name:  Name of 3rd image to load.				     M
*		 Optional and can be NULL or a zero length string.           M
*   DitherSize:  Dithering matrix size to use: 2, 3, or 4.		     M
*   MatchWidth:  Width to allow matching in a row:			     M
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. M
*   Negate:      TRUE to negate the images.				     M
*   AugmentContrast:  Redundancy level for the micro-pixels, to augment      M
*                the contrast, behind existing pixels.  Zero to disable.     M
*   SpreadMethod: Selects initial spread to use: Random, Diagonal, etc.	     M
*   SphereRad:   Radius of construct spherical blob, zero to return points.  M
*   AccumPenalty:  Returns the accumulated error in the dithering-matching,  M
*		 where zero designates no error.  In level of achieved       M
*		 covering.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Center points, in [ImageWidth]^3 space.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   User3DDither3Images2                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DDither3Images                                                      M
*****************************************************************************/
IPObjectStruct *User3DDither3Images(const char *Image1Name,
				    const char *Image2Name,
				    const char *Image3Name,
				    int DitherSize,
				    int MatchWidth,
				    int Negate,
				    int AugmentContrast,
				    User3DSpreadType SpreadMethod,
				    IrtRType SphereRad,
				    IrtRType *AccumPenalty)
{
    int i, j, x, y, Img1MaxX, Img1MaxY, Img1Alpha, Img2MaxX, Img2MaxY, Img2Alpha,
        Img3MaxX, Img3MaxY, Img3Alpha, Size, n, m, *RandVec1, *RandVec2;
    IrtImgPixelStruct *I,
        *Img1 = IrtImgReadImage(Image1Name, &Img1MaxX, &Img1MaxY, &Img1Alpha),
	*Img2 = IrtImgReadImage(Image2Name, &Img2MaxX, &Img2MaxY, &Img2Alpha),
        *Img3 = Image3Name == NULL || strlen(Image3Name) == 0 ?
		NULL :
		IrtImgReadImage(Image3Name, &Img3MaxX, &Img3MaxY, &Img3Alpha);
    IPVertexStruct
        *AllVrtcs = NULL;
    User3DDtrImgCoverStruct *IC;

    IritRandomInit(301060);

    *AccumPenalty = 0;

    if (AugmentContrast <= 0)
	AugmentContrast = 1;			     /* At least one pixel. */

    /* Make sure all input is valid - images are squares of same size, etc. */
    if (Img1 == NULL || Img2 == NULL) {
	if (Img1 != NULL)
	    IritFree(Img1);
	if (Img2 != NULL)
	    IritFree(Img2);
	if (Img3 != NULL)
	    IritFree(Img3);
	return NULL;
    }
    if (DitherSize < 2 || DitherSize > 4 ||
	Img1MaxX != Img1MaxY ||
	Img1MaxX != Img2MaxX ||
	Img1MaxX != Img2MaxY ||
	(Img3 != NULL && Img1MaxX != Img3MaxX) ||
	(Img3 != NULL && Img1MaxX != Img3MaxY)) {
	IritFree(Img1);
	IritFree(Img2);
	if (Img3 != NULL)
	    IritFree(Img3);
        return NULL;
    }
    Size = Img1MaxX + 1;

    if (Negate) {
	I = IrtImgNegateImage(Img1, Img1MaxX, Img1MaxX);
	IritFree(Img1);
	Img1 = I;

	I = IrtImgNegateImage(Img2, Img2MaxX, Img2MaxX);
	IritFree(Img2);
	Img2 = I;

	if (Img3 != NULL) {
	    I = IrtImgNegateImage(Img3, Img3MaxX, Img3MaxX);
	    IritFree(Img3);
	    Img3 = I;
	}
    }

    /* Time to combine the three images into one 3D model. */
    IC = User3DDtrAllocImgCover(Img1, Img2, Img3, Size, DitherSize);
    IritFree(Img1);
    IritFree(Img2);
    if (Img3 != NULL)
        IritFree(Img3);

    RandVec1 = User3DMicroBlobsCreateRandomVector(IC -> Size,
						  USER_3D_SPREAD_RANDOM, TRUE);
    RandVec2 = User3DMicroBlobsCreateRandomVector(IC -> Size,
						  USER_3D_SPREAD_RANDOM, TRUE);
    for (y = 0; y < IC -> Size; y++) {
        j = RandVec1[y];
        for (x = 0; x < IC -> Size; x++) {
	    i = RandVec1[x];

	    /* Scan all pixels in all three images and create the covering. */
	    if (AugmentContrast - USER_3DDTR_CVR1(i, j) > 0 &&
	        User3DDtrFetchPixelCovers(IC, i, -1, j)) {
	        User3DDtrUpdateCover(IC, i, -1, j, AugmentContrast,
				     MatchWidth, SpreadMethod, &AllVrtcs);
	    }
	    if (AugmentContrast - USER_3DDTR_CVR2(i, j) > 0 &&
	        User3DDtrFetchPixelCovers(IC, -1, i, j)) {
	        User3DDtrUpdateCover(IC, -1, i, j, AugmentContrast,
				     MatchWidth, SpreadMethod, &AllVrtcs);
	    }
	    if (Img3 != NULL &&
		AugmentContrast - USER_3DDTR_CVR3(i, j) > 0 &&
	        User3DDtrFetchPixelCovers(IC, i, j, -1)) {
	        User3DDtrUpdateCover(IC, i, j, -1, AugmentContrast,
				     MatchWidth, SpreadMethod, &AllVrtcs);
	    }
	}
    }

    IritFree(RandVec1);
    IritFree(RandVec2);

    /* Compute the error - pixels we did not cover as desired. */
    for (j = n = m = 0; j < IC -> Size; j++) {
        for (i = 0; i < IC -> Size; i++) {
	    if (USER_3DDTR_IMG1(i, j)) {
	        m++;
		n += AugmentContrast <= USER_3DDTR_CVR1(i, j);
	    }
	    if (USER_3DDTR_IMG2(i, j)) {
	        m++;
		n += AugmentContrast <= USER_3DDTR_CVR2(i, j);
	    }
	    if (Img3 != NULL && USER_3DDTR_IMG3(i, j)) {
	        m++;
		n += AugmentContrast <= USER_3DDTR_CVR3(i, j);
	    }
	}
    }
    *AccumPenalty = ((IrtRType) n) / m;

    AllVrtcs = User3DDitherSetXYTranslations(AllVrtcs);

    User3DDtrFreeImgCover(IC);

    return IPGenPOINTLISTObject(IPAllocPolygon(0, AllVrtcs, NULL));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute all the indices in the space, along a line of sight that can be  *
* used as covering pixels for the given image coordinate.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:        The Img covering structure.                                   *
*   x, y, z:   The coordinates.  One of the coordinates must be negative,    *
*              indicating the image to use.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if ImgCover was indeed updated, FALSE otherwise.         *
*****************************************************************************/
static int User3DDtrFetchPixelCovers(User3DDtrImgCoverStruct *IC,
				     int x,
				     int y,
				     int z)
{
    if (y < 0) {					 /* Use Img1(x, z). */
        if (USER_3DDTR_IMG1(x, z) == 0)
	    return FALSE;

	/* Examine all the line at (x, *, z). */
	for (y = 0; y < IC -> Size; y++)
	    IC -> CoverVec[y] = USER_3DDTR_IMG2(y, z) &&
	                        USER_3DDTR_IMG3SAFE(x, y);
    }
    else if (x < 0) {					 /* Use Img2(y, z). */
        if (USER_3DDTR_IMG2(y, z) == 0)
	    return FALSE;

	/* Examine all the line at (*, y, z). */
	for (x = 0; x < IC -> Size; x++)
	    IC -> CoverVec[x] = USER_3DDTR_IMG1(x, z) &&
	                        USER_3DDTR_IMG3SAFE(x, y);
    }
    else {						 /* Use Img3(x, y). */
        assert(z < 0);
	if (IC -> Img3Dtr == NULL || USER_3DDTR_IMG3(x, y) == 0)
	    return FALSE;

	/* Examine all the line at (x, y, *). */
	for (z = 0; z < IC -> Size; z++)
	    IC -> CoverVec[z] = USER_3DDTR_IMG1(x, z) &&
	                        USER_3DDTR_IMG2(y, z);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two pixel entities				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PP1, PP2:  Two pointers to User3DDtrPxlRndmSortStruct structs.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two distances.           *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int User3DDtrSortPixels(void *PP1, void *PP2)
#else
static int User3DDtrSortPixels(const void *PP1, const void *PP2)
#endif /* ultrix && mips (no const support) */
{
    int Diff = ((User3DDtrPxlRndmSortStruct *) PP1) -> Rndm - 
               ((User3DDtrPxlRndmSortStruct *) PP2) -> Rndm;

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Selects, out of IC -> ImgCover vector, the actual pixels we use and      *
* update all necessary covering buffers as each pixel covers three planes.   *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:        The Img covering structure.                                   *
*   x, y, z:   The coordinates.  One of the coordinates must be negative,    *
*              indicating the image to use.                                  *
*   AugmentContrast:  Level of covering redundancy expected.                 *
*   MatchWidth:  Width to allow matching in a row:			     *
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. *
*   SpreadMethod:  Method of spreading the points in the cube: random, along *
*              the diagonal, etc.					     *
*   Pixels:    Updated with the created dithering pixels, inside the volume. *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if desired contrast is met for this one.		             *
*****************************************************************************/
static int User3DDtrUpdateCover(User3DDtrImgCoverStruct *IC,
				int x,
				int y,
				int z,
				int AugmentContrast,
				int MatchWidth,
				User3DSpreadType SpreadMethod,
				IPVertexStruct **Pixels)
{
    int i, n;

    User3DDtrFilterCoverSpread(IC, x, y, z, MatchWidth, SpreadMethod);

    if (y < 0) {					 /* Use Img1(x, z). */
        if (USER_3DDTR_CVR1(x, z) >= AugmentContrast)
	    return TRUE; /* Covered enough already. */
    }
    else if (x < 0) {					 /* Use Img2(y, z). */
        if (USER_3DDTR_CVR2(y, z) >= AugmentContrast)
	    return TRUE; /* Covered enough already. */
    }
    else {						 /* Use Img3(x, y). */
        assert(z < 0);
        if (IC -> Img3Dtr == NULL || USER_3DDTR_CVR3(x, y) >= AugmentContrast)
	    return TRUE; /* Covered enough already. */
    }

    /* Count how many 3D locations have this 2D image location set. */
    for (i = n = 0; i < IC -> Size; i++)
        n += IC -> CoverVec[i];


    if (n >= AugmentContrast) {
        int k;
	User3DDtrPxlRndmSortStruct
	    *Pxls = IritMalloc(sizeof(User3DDtrPxlRndmSortStruct) * IC -> Size);

	/* Pick the pixels' indices we can turn on. */
	for (i = k = 0; i < IC -> Size; i++) {
	    if (IC -> CoverVec[i]) {
	        Pxls[k].Idx = (short) i;
	        Pxls[k++].Rndm = (short) IritRandom(0.0, 30000.0);
	    }
	}
	/* Sort them using random keys. */
	qsort(Pxls, k, sizeof(User3DDtrPxlRndmSortStruct),
	      User3DDtrSortPixels);

	/* and use the first AugmentContrast (needed) entries. */
        for (k = 0; k < AugmentContrast; k++) {
	    User3DDtrSetPixel(IC, Pxls[k].Idx, x, y, z, Pixels);
	}

	IritFree(Pxls);

	return TRUE;
    }
    else {
        /* Use all pixels as we are in deficit. */
        for (i = n = 0; i < IC -> Size; i++) {
	    if (IC -> CoverVec[i]) {
	        User3DDtrSetPixel(IC, i, x, y, z, Pixels);
	    }
	}

	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets the volume pixel at xyz and update all covering tables.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:       The Img covering structure.                                    *
*   i:        Index into the CoverVec vector.                                *
*   x, y, z:  The coordinates of the CoverVec vector.  Updated in place to   *
*             real 3D coords.				                     *
*   Pixels:   Updated with the created dithering pixels, inside the volume.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    The CoverVec vector value at these coordinates.                  *
*****************************************************************************/
static void User3DDtrSetPixel(User3DDtrImgCoverStruct *IC,
			      int i,
			      int x,
			      int y,
			      int z,
			      IPVertexStruct **Pixels)
{
    User3DDtrGetIthCoverCoord(IC, i, &x, &y, &z);

    /* Increase the covering factors. */
    USER_3DDTR_CVR1(x, z)++;
    USER_3DDTR_CVR2(y, z)++;
    if (IC -> CoverImg3 != NULL)
        USER_3DDTR_CVR3(z, y)++;

    *Pixels = IPAllocVertex2(*Pixels);
    (*Pixels) -> Coord[0] = x;
    (*Pixels) -> Coord[1] = y;
    (*Pixels) -> Coord[2] = z;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes real XYZ coordinates, in place, for the CoverVec vector coords. *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:       The Img covering structure.                                    *
*   i:        Index into the CoverVec vector.                                *
*   x, y, z:  The coordinates of the CoverVec vector.  Updated in place to   *
*             real 3D coords.				                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    The CoverVec vector value at these coordinates.                  *
*****************************************************************************/
static int User3DDtrGetIthCoverCoord(User3DDtrImgCoverStruct *IC,
				     int i,
				     int *x,
				     int *y,
				     int *z)
{
    if (*x < 0) {
        *x = i;
    }
    else if (*y < 0) {
        *y = i;
    }
    else {
        assert(*z < 0);
	*z = i;
    }

    return IC -> CoverVec[i];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Filters and cleans all covering locations in the CoverVec that are not   *
* the the desired point spread.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:       The Img covering structure.                                    *
*   x, y, z:  The coordinates of the CoverVec vector.                        *
*   MatchWidth:   The maximal deviation, in pixels from the desired spread.  *
*   SpreadMethod: The desired pixel spread.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void User3DDtrFilterCoverSpread(User3DDtrImgCoverStruct *IC,
				       int x,
				       int y,
				       int z,
				       int MatchWidth,
				       User3DSpreadType SpreadMethod)
{
    int i, xc, yc, zc;

    for (i = 0; i < IC -> Size; i++) {
        if (IC -> CoverVec[i] > 0) {
	    IrtRType d;

	    xc = x;
	    yc = y;
	    zc = z;
	    User3DDtrGetIthCoverCoord(IC, i, &xc, &yc, &zc);

	    switch (SpreadMethod) {
	        default:
	        case USER_3D_SPREAD_DISCONT2PLANE:
	        case USER_3D_SPREAD_DISCONT4PLANE:
	        case USER_3D_SPREAD_RANDOM:
		    return;
	        case USER_3D_SPREAD_DIAG_PLANE:
		    d = xc + yc + zc;

		    /* Compute the distance to the plane X + Y + Z = Size  */
		    /* and to the plane X + Y + Z = 2 * Size.   	   */
		    if (IRIT_ABS(d - IC -> Size) > MatchWidth &&
			IRIT_ABS(d - IC -> Size * 2) > MatchWidth)
		        IC -> CoverVec[i] = 0;         /* Zero this pixel. */
		    break;
	        case USER_3D_SPREAD_DIAG_PLANE2:
		    d = xc + yc + zc;

		    /* Compute distance to plane X + Y + Z = .5 * Size     */
		    /* and to the planes X + Y + Z = 1.5 * Size.	   */
		    /*                   X + Y + Z = 2.5 * Size.	   */
		    if (IRIT_ABS(d - IC -> Size * 0.5) > MatchWidth &&
			IRIT_ABS(d - IC -> Size * 1.5) > MatchWidth &&
			IRIT_ABS(d - IC -> Size * 2.5) > MatchWidth)
		        IC -> CoverVec[i] = 0;         /* Zero this pixel. */
		    break;
	        case USER_3D_SPREAD_ANTI_DIAG_PLANE:
		    d = xc + yc - zc;

		    /* Compute the distance to the plane X + Y - Z = 0     */
		    /* and to the plane X + Y - Z = Size.		   */
		    if (IRIT_ABS(d) > MatchWidth &&
			IRIT_ABS(d - IC -> Size) > MatchWidth)
		        IC -> CoverVec[i] = 0;         /* Zero this pixel. */
		    break;
	        case USER_3D_SPREAD_ANTI_DIAG_PLANE2:
		    d = xc - yc + zc;

		    /* Compute the distance to the plane X - Y + Z = 0     */
		    /* and to the plane X - Y + Z = Size.		   */
		    if (IRIT_ABS(d) > MatchWidth &&
			IRIT_ABS(d - IC -> Size) > MatchWidth)
		        IC -> CoverVec[i] = 0;         /* Zero this pixel. */
		    break;
	        case USER_3D_SPREAD_ANTI_DIAG_PLANE3:
		    d = xc - yc - zc;

		    /* Compute the distance to the plane X - Y - Z = 0     */
		    /* and to the plane X - Y - Z = -Size.		   */
		    if (IRIT_ABS(d) > MatchWidth &&
			IRIT_ABS(d + IC -> Size) > MatchWidth)
		        IC -> CoverVec[i] = 0;         /* Zero this pixel. */
		    break;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocate the dithering cover in 3D structure.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Img1, Img2, Img3:   The three square input images of same size.          *
*   Size:               of input images.                                     *
*   DitherSize:         Dithering matrix size: 2, 3 or 4.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   User3DDtrImgCoverStruct *:                                               *
*****************************************************************************/
static User3DDtrImgCoverStruct *User3DDtrAllocImgCover(IrtImgPixelStruct *Img1,
						       IrtImgPixelStruct *Img2,
						       IrtImgPixelStruct *Img3,
						       int Size,
						       int DitherSize)
{
    User3DDtrImgCoverStruct
        *IC = (User3DDtrImgCoverStruct *)
                                 IritMalloc(sizeof(User3DDtrImgCoverStruct));


    IC -> Img1Dtr = IrtImgDitherImage(Img1, Size, Size, DitherSize, TRUE);
    IC -> Img2Dtr = IrtImgDitherImage(Img2, Size, Size, DitherSize, TRUE);
    if (Img3 != NULL)
        IC -> Img3Dtr = IrtImgDitherImage(Img3, Size, Size, DitherSize, TRUE);
    else
        IC -> Img3Dtr = NULL;

    Size *= DitherSize;
    IC -> Size = Size;

    IC -> CoverVec = IritMalloc(sizeof(int) * Size);

    IC -> CoverImg1 = IritMalloc(sizeof(int) * IRIT_SQR(Size));
    IC -> CoverImg2 = IritMalloc(sizeof(int) * IRIT_SQR(Size));
    if (Img3 != NULL)
        IC -> CoverImg3 = IritMalloc(sizeof(int) * IRIT_SQR(Size));
    else
        IC -> CoverImg3 = NULL;

    IRIT_ZAP_MEM(IC -> CoverImg1, sizeof(int) * IRIT_SQR(Size));
    IRIT_ZAP_MEM(IC -> CoverImg2, sizeof(int) * IRIT_SQR(Size));
    if (IC -> CoverImg3 != NULL)
        IRIT_ZAP_MEM(IC -> CoverImg3, sizeof(int) * IRIT_SQR(Size));

    return IC;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the dithering cover in 3D structure.	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   IC:  The structure to free.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void 		                                                     *
*****************************************************************************/
static void User3DDtrFreeImgCover(User3DDtrImgCoverStruct *IC)
{
    IritFree(IC -> CoverImg1);
    IritFree(IC -> CoverImg2);
    if (IC -> CoverImg3 != NULL)
        IritFree(IC -> CoverImg3);
    IritFree(IC -> Img1Dtr);
    IritFree(IC -> Img2Dtr);
    if (IC -> Img3Dtr != NULL)
        IritFree(IC -> Img3Dtr);
    IritFree(IC -> CoverVec);
    IritFree(IC);
}

#ifdef DEBUG_USER_DITHER_3D

main()
{
    IrtRType Penalty;
    IPObjectStruct *PObj;

    PrimSetResolution(2);

    PObj = User3DDither3Images("BenGurion150.gif",
			       "Herzel150.gif",
			       "Rabin150.gif",
			       3, 3, 0, 1, 0, USER_3D_SPREAD_RANDOM,
			       0.5, &Penalty);

    IPStdoutObject(PObj, FALSE);
}

#endif /* DEBUG_USER_DITHER_3D */
