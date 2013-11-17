/*****************************************************************************
* Manipulate images.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	Dec. 1998    *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads one image in from a file named ImageFileName.  The image is        M
* returned as a vector of RGBRGB... of size (MaxX+1) * (MaxY+1) * 3.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Img:   Image to flip its X and Y axes.                                   M
*   MaxX:  Maximum X of Img image.			                     M
*   MaxY:  Maximum Y of Img image.			                     M
*   Alpha: TRUE if this image has alpha and is actually RGBARGBA...          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *:  Flipped image.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgNegateImage, IrtImgScaleImage	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgFlipXYImage                                                        M
*****************************************************************************/
IrtImgPixelStruct *IrtImgFlipXYImage(IrtImgPixelStruct *Img,
				     int MaxX,
				     int MaxY,
				     int Alpha)
{
    int x, y,
	SizeX = MaxX + 1,
	SizeY = MaxY + 1;

    if (Alpha) {
        IrtImgRGBAPxlStruct
	  *Img2 = (IrtImgRGBAPxlStruct *) Img,
	    *FImg = (IrtImgRGBAPxlStruct *)
	        IritMalloc(sizeof(IrtImgRGBAPxlStruct) * SizeX * SizeY);

	for (y = 0; y < SizeY; y++) {
	    for (x = 0; x < SizeX; x++) {
	        FImg[x * SizeY + y] = *Img2++;
	    }
	}

	return (IrtImgPixelStruct *) FImg;
    }
    else {
        IrtImgPixelStruct
	    *FImg = (IrtImgPixelStruct *)
	        IritMalloc(sizeof(IrtImgPixelStruct) * SizeX * SizeY);

	for (y = 0; y < SizeY; y++) {
	    for (x = 0; x < SizeX; x++) {
	        FImg[x * SizeY + y] = *Img++;
	    }
	}

	return FImg;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Negate an image.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   InImage:    A vector of IrtImgPixelStruct of size (MaxX+1) * (MaxY+1).   M
*   MaxX:       Maximum X of input image.			             M
*   MaxY:       Maximum Y of input image.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *: The scaled image as vector of RGBRGB (or RGBARGBA). M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgFlipXYImage, IrtImgScaleImage	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgNegateImage                                                        M
*****************************************************************************/
IrtImgPixelStruct *IrtImgNegateImage(IrtImgPixelStruct *InImage,
				    int MaxX,
				    int MaxY)
{
    int i, j;
    IrtImgPixelStruct *OI,
        *OutImage = (IrtImgPixelStruct *)
                               IritMalloc(sizeof(IrtImgPixelStruct) *
						    (MaxX + 1) * (MaxY + 1));

    for (OI = OutImage, i = 0; i <= MaxX; i++) {
        for (j = 0; j <= MaxY; j++, OI++, InImage++) {
	    OI -> r = 255 - InImage -> r;
	    OI -> g = 255 - InImage -> g;
	    OI -> b = 255 - InImage -> b;
	}
    }

    return OutImage;
}
