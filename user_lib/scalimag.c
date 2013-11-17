/*****************************************************************************
* Scale an image by converting it to a bivariate spline.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	Jul. 2006    *
*****************************************************************************/

#include <math.h>
#include <assert.h>
#include "irit_sm.h"
#include "user_loc.h"

#define MAX_SCALE_ORDER	10

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scale an image by mapping it to a bivariate spline and resampling.       M
*                                                                            *
* PARAMETERS:                                                                M
*   InImage:    A vector of RGBRGB... of size (MaxX+1) * (MaxY+1) * 3 or     M
*		NULL if failed.						     M
*		  If however, Alpha is available we have RGBARGBA and        M
*	        InImage is actually IrtImgRGBAPxlStruct.		     M
*   InMaxX:     Maximum X of input image.			             M
*   InMaxY:     Maximum Y of input image.		                     M
*   InAlpha:	If TRUE, we have alpha as well and InImage is actually	     M
*		IrtImgRGBAPxlStruct.					     M
*   OutMaxX:    Maximum X of output image.			             M
*   OutMaxY:    Maximum Y of output image.		                     M
*   Order:      Of the spline filer. 2 for a bilinear and the higher the     M
*		Order is the smoother the result will be.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *: The scaled image as vector of RGBRGB (or RGBARGBA). M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgReadImage			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgScaleImage                                                         M
*****************************************************************************/
IrtImgPixelStruct *IrtImgScaleImage(IrtImgPixelStruct *InImage,
				    int InMaxX,
				    int InMaxY,
				    int InAlpha,
				    int OutMaxX,
				    int OutMaxY,
				    int Order)
{
    int i, j, l;
    IrtRType **Pts, *R, u, v, Du, Dv;
    IrtImgPixelStruct *Img, *OutImage;
    IrtImgRGBAPxlStruct *AImg;
    CagdSrfStruct *Srf;

    Order = IRIT_MIN(IRIT_MAX(Order, 2), MAX_SCALE_ORDER);

    assert(InMaxX > 1);
    assert(InMaxY > 1);
    assert(OutMaxX > 1);
    assert(OutMaxY > 1);

    /* Construct the spline surface from the image. */
    Srf = BspSrfNew(InMaxX + 1, InMaxY + 1, Order, Order,
		    InAlpha ? CAGD_PT_E4_TYPE : CAGD_PT_E3_TYPE);
    BspKnotUniformOpen(InMaxX + 1, Order, Srf -> UKnotVector);
    BspKnotUniformOpen(InMaxY + 1, Order, Srf -> VKnotVector);

    Pts = Srf -> Points;

    if (InAlpha) {
        AImg = (IrtImgRGBAPxlStruct *) InImage;

        for (i = l = 0; i <= InMaxX; i++) {
	    for (j = 0; j <= InMaxY; j++, l++, AImg++) {
	        Pts[0][l] = AImg -> a;
	        Pts[1][l] = AImg -> r;
	        Pts[2][l] = AImg -> g;
	        Pts[3][l] = AImg -> b;
	    }
	}
    }
    else {
        Img = InImage;

        for (i = l = 0; i <= InMaxX; i++) {
	    for (j = 0; j <= InMaxY; j++, l++, Img++) {
	        Pts[1][l] = Img -> r;
	        Pts[2][l] = Img -> g;
	        Pts[3][l] = Img -> b;
	    }
	}
    }

    /* Allocate the output image and resample Srf. */
    Du = 1.0 / (OutMaxX + 1);
    Dv = 1.0 / (OutMaxY + 1);
    u = v = 0.0;
    if (InAlpha) {
        AImg = (IrtImgRGBAPxlStruct *) IritMalloc(sizeof(IrtImgRGBAPxlStruct) *
						(OutMaxX + 1) * (OutMaxY + 1));
	OutImage = (IrtImgPixelStruct *) AImg;

        for (v = Dv / 2.0; v < 1.0; v += Dv) {
	    for (u = Du / 2.0; u < 1.0; u += Du, AImg++) {
	        R = CagdSrfEval(Srf, u, v);
	        AImg -> a = (int) R[0];
	        AImg -> r = (int) R[1];
	        AImg -> g = (int) R[2];
	        AImg -> b = (int) R[3];
	    }
	}
    }
    else {
        OutImage = Img = (IrtImgPixelStruct *)
	    IritMalloc(sizeof(IrtImgPixelStruct) *
					        (OutMaxX + 1) * (OutMaxY + 1));

        for (v = Dv / 2.0; v < 1.0; v += Dv) {
	    for (u = Du / 2.0; u < 1.0; u += Du, Img++) {
	        R = CagdSrfEval(Srf, u, v);
	        Img -> r = (int) R[1];
	        Img -> g = (int) R[2];
	        Img -> b = (int) R[3];
	    }
	}
    }

    CagdSrfFree(Srf);

    return OutImage;
}

