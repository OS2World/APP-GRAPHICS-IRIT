/*****************************************************************************
* image dithering.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	Dec. 2011    *
*****************************************************************************/

#include "irit_sm.h"
#include "misc_loc.h"

static void IrtImgDitherLine(IrtImgPixelStruct *Line,
			     int LineLen,
			     int DitherSize,
			     IrtRType *Errors,
			     IrtBType *DitheredLine,
			     IrtBType ErrorDiffusion);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to dither a given line Line of RGB pixels using dithering        *
* matrices of size DitherSize to B&W.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:         Line of length LineLen to dither into B&W                  *
*   LineLen:      Length of line Line - number of pixels.                    *
*   DitherSize:   Desired dithering size - 2, 3, or 4.                       *
*   Errors:       Vector of length LineLen that will be updated with the     *
*                 error between the input line and the dithered value (so    *
*                 error diffusion can be used on the next line).             *
*   DitheredLine: The results will be stored here.                           *
*                 Must hold DitherSize lines of pixels, each line is of      *
*		  length LineLen * Dithersize.                               *
*   ErrorDiffusion: TRUE, to also diffuse the error in the image.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IrtImgDitherLine(IrtImgPixelStruct *Line,
			     int LineLen,
			     int DitherSize,
			     IrtRType *Errors,
			     IrtBType *DitheredLines,
			     IrtBType ErrorDiffusion)
{
    /* See Foley & Van Dam pp. 597-601. */
    IRIT_STATIC_DATA const char Dither2[2][2] = {
	{ 1, 3 },
	{ 4, 2 }
    };
    IRIT_STATIC_DATA const char Dither3[3][3] = {
	{ 4, 7, 2 },
	{ 1, 9, 5 },
	{ 6, 3, 8 }
    };
    IRIT_STATIC_DATA const char Dither4[4][4] = {
	{ 1,  9,  3,  11 },
	{ 13, 5,  15, 7 },
	{ 4,  12, 2,  10 },
	{ 16, 8,  14, 6 }
    };
    int i,
        Levels = IRIT_SQR(DitherSize) + 1;
    IrtRType
        NextError = 0.0;

    /* Scan the Line and use the dither matrix to set the dithered result: */
    for (i = 0; i < LineLen; i++) {
        int x, y, XStep, XDitheredSize, Swap, SwapX, SwapY;
	IrtRType
	    Intensity = NextError + Levels * (0.30 * Line[i].r +
					      0.59 * Line[i].g +
					      0.11 * Line[i].b) / 255.0;

	XStep = i * DitherSize;
	XDitheredSize = LineLen * DitherSize;
	Swap = IritRandom(0.0, 1.0) < 0.5;
	SwapX = IritRandom(0.0, 1.0) < 0.5;
	SwapY = IritRandom(0.0, 1.0) < 0.5;

	switch (DitherSize) {
	    case 2:
	        for (y = 0; y < DitherSize; y++)
		    for (x = 0; x < DitherSize; x++) {
		        int Xc = SwapX ? 1 - x : x,
			    Yc = SwapY ? 1 - y : y;

			DitheredLines[y * XDitheredSize + XStep + x] =
			    (Swap ? Dither2[Yc][Xc] : Dither2[Xc][Yc])
			                                         <= Intensity;
		    }
		break;
	    default:
	    case 3:
	        for (y = 0; y < DitherSize; y++)
		    for (x = 0; x < DitherSize; x++) {
		        int Xc = SwapX ? 2 - x : x,
			    Yc = SwapY ? 2 - y : y;

			DitheredLines[y * XDitheredSize + XStep + x] =
			    (Swap ? Dither3[Yc][Xc] : Dither3[Xc][Yc])
			                                         <= Intensity;
		    }
		break;
	    case 4:
	        for (y = 0; y < DitherSize; y++)
		    for (x = 0; x < DitherSize; x++) {
		        int Xc = SwapX ? 3 - x : x,
			    Yc = SwapY ? 3 - y : y;

			DitheredLines[y * XDitheredSize + XStep + x] =
			    (Swap ? Dither4[Yc][Xc] : Dither4[Xc][Yc])
			                                         <= Intensity;
		    }
		break;
	}

	Errors[i] = (Intensity - ((int) Intensity)) / Levels;
	if (ErrorDiffusion) {
	    NextError = Errors[i] * 7.0 / 16.0;
	    Errors[i] += NextError;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to dither a given Image of RGB pixels using dithering matrices   M
* of size DitherSize to B&W.                                        	     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Image:          To dither.                                               M
*   XSize, YSize:   Size of the image to dither.                             M
*   DitherSize:     Dithering matrices size: 2, 3, or 4.                     M
*   ErrorDiffusion: TRUE, to also diffuse the error in the image.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtBType *:     The dithered image of size X/YSize * DitherSize while    M
*                   every pixels is either zero or one.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgDitherImage                                                        M
*****************************************************************************/
IrtBType *IrtImgDitherImage(IrtImgPixelStruct *Image,
			    int XSize,
			    int YSize,
			    int DitherSize,
			    IrtBType ErrorDiffusion)
{
    int x, y;
    IrtBType
	*DitheredImage = IritMalloc(sizeof(IrtBType) * IRIT_SQR(DitherSize) *
				                              XSize * YSize);
    IrtRType
        *Errors = IritMalloc(sizeof(IrtRType) * XSize);
    IrtImgPixelStruct
	*Line = IritMalloc(sizeof(IrtImgPixelStruct) * XSize);

    for (y = 0; y < YSize; y++) {
        IRIT_GEN_COPY(Line, &Image[y * XSize],
		      sizeof(IrtImgPixelStruct) * XSize);

	if (ErrorDiffusion && y > 0) {
	    /* Add the errors to the next line. */

	    for (x = 0; x < XSize; x++) {
	        IrtRType
		    Err = (x > 0 ? Errors[x - 1] * 3.0 / 16.0 : 0.0) +
		          Errors[x] * 5.0 / 16.0 +
		          (x < XSize - 1 ? Errors[x + 1] * 1.0 / 16.0 : 0.0);

	        Line[x].r = (IrtBType) IRIT_BOUND(Line[x].r + Err * 255.0,
						  0.0, 255.0);
	        Line[x].g = (IrtBType) IRIT_BOUND(Line[x].g + Err * 255.0,
						  0.0, 255.0);
		Line[x].b = (IrtBType) IRIT_BOUND(Line[x].b + Err * 255.0,
						  0.0, 255.0);
	    }	    
	}

	IrtImgDitherLine(Line, XSize, DitherSize, Errors,
			 &DitheredImage[y * IRIT_SQR(DitherSize) * XSize],
			 ErrorDiffusion);
    }

    IritFree(Line); 
    IritFree(Errors); 

    return DitheredImage;
}

#ifdef DEBUG_MAIN_TEST_DITHERING
/* Use "prgm image.ppm DitherSize DiffuseErrors", i.e. "prgm test.ppm 3 1" */
main(int argc, char **argv)
{
    int x, y, MaxX, MaxY, DtrXSize, DtrYSize, Alpha,
        DitherSize = atoi(argv[2]),
        DiffuseErrors = atoi(argv[3]);
    IrtImgPixelStruct *Line,
        *Img = IrtImgReadImage(argv[1], &MaxX, &MaxY, &Alpha);
    IrtBType
        *DtrImg = IrtImgDitherImage(Img, MaxX + 1, MaxY + 1,
				    DitherSize, DiffuseErrors);

    DtrXSize = (MaxX + 1) * DitherSize;
    DtrYSize = (MaxY + 1) * DitherSize;

    /* Dump the dithered image. */
    IrtImgWriteSetType("ppm");
    IrtImgWriteOpenFile(argv, "dithered.ppm", FALSE, DtrXSize, DtrYSize);
    Line = (IrtImgPixelStruct *)
                            IritMalloc(sizeof(IrtImgPixelStruct) * DtrXSize);

    for (y = 0; y < DtrYSize; y++) {
        int Ofst = y * DtrXSize;;

        for (x = 0; x < DtrXSize; x++)
	    Line[x].r = Line[x].g = Line[x].b = DtrImg[x + Ofst] ? 255 : 0;

        IrtImgWritePutLine(NULL, Line);
    }

    IrtImgWriteCloseFile();

    IritFree(Line);    
}

#endif /* DEBUG_MAIN_TEST_DITHERING */
