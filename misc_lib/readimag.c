/*****************************************************************************
* Reads one image in.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	Dec. 1998    *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"

#ifdef IRIT_HAVE_URT_RLE
#define NO_DECLARE_MALLOC /* For rle.h */
#include <rle.h>
#include <rle_raw.h>
#endif /* IRIT_HAVE_URT_RLE */

#ifdef IRIT_HAVE_GIF_LIB
#include "gif_lib.h"
#endif /* IRIT_HAVE_GIF_LIB */

#ifdef IRIT_HAVE_PNG_LIB
#include "png.h"
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif
#endif /* IRIT_HAVE_PNG_LIB */

#ifdef __WINNT__
#include <io.h>
#include <fcntl.h>
#endif /* __WINNT__ */

typedef struct LoadedImagesStruct {
    struct LoadedImagesStruct *Pnext;
    char *FileName;
    int MaxX, MaxY, Alpha;
    IrtBType *Data;
} LoadedImagesStruct;

IRIT_STATIC_DATA unsigned int
    GlblImageXAlignment = 0xffffffff; /* No alignment by default. */

IRIT_STATIC_DATA LoadedImagesStruct
    *GlblLoadedImagesList = NULL;

static IrtBType *IrtImgVerifyAlignment(IrtBType *Data,
				       int *MaxX,
				       int *MaxY,
				       int Alpha);
static IrtBType *PPMReadImage(const char *File,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha);
static IrtBType *RLEReadImage(const char *File,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha);
static IrtBType *GIFReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha);
static IrtBType *PNGReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets an alignment of the width of the image. For example, OGL requires   M
* images to have width aligned on 4-bytes words (alignment 4).               M
*                                                                            *
* PARAMETERS:                                                                M
*   Alignment:   Word size alignment required.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     old alignment.                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgReadImage                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgReadImageXAlign                                                    M
*****************************************************************************/
int IrtImgReadImageXAlign(int Alignment)
{
    int OldVal = GlblImageXAlignment;

    GlblImageXAlignment = ~(Alignment - 1);

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads one image in from a file named ImageFileName.  The image is        M
* returned as a vector of RGBRGB... of size (MaxX+1) * (MaxY+1) * 3.         M
*                                                                            *
* PARAMETERS:                                                                M
*   ImageFileName:   Name of image to read.                                  M
*   MaxX:            Maximum X of read image is saved here.                  M
*   MaxY:            Maximum Y of read image is saved here.                  M
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  M
*		     and will return TRUE if successful in loading it.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *:  A vector of RGBRGB... of size 		     M
*		(MaxX+1) * (MaxY+1) * 3 or NULL if failed.		     M
*		  If however, Alpha is requested and found RGBARGBA... is    M
*		returned as IrtImgRGBAPxlStruct.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgReadImage2, IrtImgReadImageXAlign                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgReadImage                                                          M
*****************************************************************************/
IrtImgPixelStruct *IrtImgReadImage(const char *ImageFileName,
				   int *MaxX,
				   int *MaxY,
				   int *Alpha)
{
    const char *Type;

    if (ImageFileName == NULL) {
        IRIT_FATAL_ERROR("Empty image file name to write to.");
        return NULL;
    }

    if ((Type = strrchr(ImageFileName, '.')) == NULL)
        Type = "";

    if (stricmp(Type, ".Z") == 0) {
        Type--;
	while (Type != ImageFileName && *Type != '.')
	    Type--;
    }

    if (stricmp(Type, ".ppm") == 0) {
        return (IrtImgPixelStruct *) PPMReadImage(ImageFileName, MaxX, MaxY,
						  Alpha);
    }
    else if (stricmp(Type, ".rle") == 0 || stricmp(Type, ".rle.Z") == 0) {
	return (IrtImgPixelStruct *) RLEReadImage(ImageFileName, MaxX, MaxY,
						  Alpha);
    }
    else if (stricmp(Type, ".gif") == 0) {
	return (IrtImgPixelStruct *) GIFReadImage(ImageFileName, MaxX, MaxY,
						  Alpha);
    }
    else if (stricmp(Type, ".png") == 0) {
	return (IrtImgPixelStruct *) PNGReadImage(ImageFileName, MaxX, MaxY,
						  Alpha);
    }
    else {
	IRIT_WARNING_MSG_PRINTF(
		IRIT_EXP_STR("Texture file \"%s\" must be image of type 'rle', 'ppm', 'gif' or 'png'\n"),
		ImageFileName);
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Same as IrtImgReadImage2 but if a name of an image repeats itself, the   M
* image is read only once.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ImageFileName:   Name of image to read.                                  M
*   MaxX:            Maximum X of read image is saved here.                  M
*   MaxY:            Maximum Y of read image is saved here.                  M
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  M
*		     and will return TRUE if successful in loading it.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *:  A vector of RGBRGB... of size                      M
*		(MaxX+1) * (MaxY+1) * 3 or NULL if failed.		     M
*		  If however, Alpha is requested and found RGBARGBA... is    M
*		returned as IrtImgRGBAPxlStruct.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgReadImage, IrtImgReadClrCache                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgReadImage2                                                         M
*****************************************************************************/
IrtImgPixelStruct *IrtImgReadImage2(const char *ImageFileName,
				    int *MaxX,
				    int *MaxY,
				    int *Alpha)
{
    LoadedImagesStruct *LoadedImage;
    IrtImgPixelStruct *Data;

    /* Search if we already loaded this image. */
    for (LoadedImage = GlblLoadedImagesList;
	 LoadedImage != NULL;
	 LoadedImage = LoadedImage -> Pnext) {
	if (strcmp(ImageFileName, LoadedImage -> FileName) == 0) {
	    *MaxX = LoadedImage -> MaxX;
	    *MaxY = LoadedImage -> MaxY;
	    *Alpha = LoadedImage -> Alpha;
	    return (IrtImgPixelStruct *) LoadedImage -> Data;
	}
    }

    if ((Data = IrtImgReadImage(ImageFileName, MaxX, MaxY, Alpha)) != NULL) {
	/* Add it to global list of loaded images. */
	LoadedImage = (LoadedImagesStruct *)
				    IritMalloc(sizeof(LoadedImagesStruct));
	LoadedImage -> FileName = IritStrdup(ImageFileName);
	LoadedImage -> MaxX = *MaxX;
	LoadedImage -> MaxY = *MaxY;
	LoadedImage -> Alpha = *Alpha;
	LoadedImage -> Data = (IrtBType *) Data;
	LoadedImage -> Pnext = GlblLoadedImagesList;
	GlblLoadedImagesList = LoadedImage;
    }

    return Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the cache of read images.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IrtImgReadImage2                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgReadClrCache                                                       M
*****************************************************************************/
void IrtImgReadClrCache(void)
{
    while (GlblLoadedImagesList != NULL) {
	LoadedImagesStruct
	    *LoadedImage = GlblLoadedImagesList;

	GlblLoadedImagesList = GlblLoadedImagesList -> Pnext;
	IritFree(LoadedImage -> FileName);
	IritFree(LoadedImage -> Data);
	IritFree(LoadedImage);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifies the alignment of the image.  Returned is either the input Data  *
* if aligned, or a new aligned copy (and Data is freed).                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:    Image data to verify alignment, in place.                       *
*   MaxX:    Current dimension of image.  Might be changed after alignment.  *
*   MaxY:    Current dimension of image.				     *
*   Alpha:   Do we have Alpha in the image?				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType *: The verified image data.				     *
*****************************************************************************/
static IrtBType *IrtImgVerifyAlignment(IrtBType *Data,
				       int *MaxX,
				       int *MaxY,
				       int Alpha)
{
    int x, y,
	Width = 1 + *MaxX,
	Height = 1 + *MaxY;

    if ((Width & GlblImageXAlignment) != Width) {     /* We have alignments. */
	IrtBType *p, *q,
	    *AlignedData = IritMalloc((Alpha ? 4 : 3) * Width * Height);

	*MaxX = (Width & GlblImageXAlignment) - 1;              /* Align it. */

        for (y = 0, p = Data, q = AlignedData; y < Height; y++) {
	    for (x = 0; x < Width; x++) {
	        if (x <= *MaxX) {
		    /* Copy pixel. */
		    *q++ = *p++;
		    *q++ = *p++;
		    *q++ = *p++;
		    if (Alpha)
		        *q++ = *p++;
		}
		else
		    p += Alpha ? 4 : 3;                  /* Skip end pixels. */
	    }
	}

	IritFree(Data);
	Data = AlignedData;
    }

    return Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file in PPM format.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageFileName:   Name of PPM image to read.                              *
*   MaxX:            Maximum X of read image is saved here.                  *
*   MaxY:            Maximum Y of read image is saved here.                  *
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  *
*		     and will return TRUE if successful in loading it.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType *:  Pointer to dynamicaly created image, NULL if non.           *
*****************************************************************************/
static IrtBType *PPMReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha)
{
    int x, y, Width, Height;
    char Line[IRIT_LINE_LEN_LONG], Line2[IRIT_LINE_LEN_LONG];
    IrtBType *Data;
    FILE *PPMLoad;

    *Alpha = FALSE;				        /* No alpha in PPM. */

#if defined(__WINNT__) || defined(__WINCE__)
    if ((PPMLoad = fopen(ImageFileName, "rb")) == NULL) {
#else
    if ((PPMLoad = fopen(ImageFileName, "r")) == NULL) {
#endif /* __WINNT__ || __WINCE__ */
        IRIT_WARNING_MSG_PRINTF("Failed to read PPM file \"%s\"\n",
				ImageFileName);
        return NULL;
    }

    fgets(Line2, IRIT_LINE_LEN_LONG - 1, PPMLoad);
    if (strncmp(Line2, "P3", 2) != 0 && strncmp(Line2, "P6", 2) != 0) {
        IRIT_WARNING_MSG_PRINTF("P3 or P6 expected, found \"%s\"\n", Line2);
        return NULL;
    }

    fgets(Line, IRIT_LINE_LEN_LONG - 1, PPMLoad);
    while (Line[0] == '#')
        fgets(Line, IRIT_LINE_LEN_LONG - 1, PPMLoad);
    sscanf(Line, "%d %d", &Width, &Height);
    if (Width < 0 || Width > 100000 || Height < 0 || Height > 100000) {
        IRIT_WARNING_MSG_PRINTF("Unrealistic image size %d by %d\n",
				Width, Height);
        return NULL;
    }
    /* Get the "255" line. */
    fgets(Line, IRIT_LINE_LEN_LONG - 1, PPMLoad);

    *MaxX = Width - 1;
    *MaxY = Height - 1;

    /* Allocate the image. */
    Data = IritMalloc(3 * Width * Height);

    if (strncmp(Line2, "P6", 2) == 0) {
	int LineSize = Width * 3;

	fread(Data, 3 * Width * Height, 1, PPMLoad);

	/* Swap the lines so YMin is YMax. */
	for (y = 0; y <= (Height >> 1); y++) {
	    IrtBType
	        *p1 = &Data[(*MaxY - y) * LineSize],
	        *p2 = &Data[y * LineSize];

	    for (x = LineSize; x > 0; x--, p1++, p2++) {
	        IRIT_SWAP(IrtBType, *p1, *p2);
	    }
	}
    }
    else { /* P3 */
	int LineSize = Width * 3;

	assert(strncmp(Line2, "P3", 2) == 0);

        for (y = 0; y < Height; y++) {
	    IrtBType
	        *p = &Data[(*MaxY - y) * LineSize];

	    for (x = 0; x < Width; x++) {
	        int r, g, b;

	        fscanf(PPMLoad, "%d %d %d", &r, &g, &b);
		*p++ = (IrtBType) r;
		*p++ = (IrtBType) g;
		*p++ = (IrtBType) b;
	    }
	}
    }

    fclose(PPMLoad);

    Data = IrtImgVerifyAlignment(Data, MaxX, MaxY, *Alpha);

    return Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file in RLE format.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageFileName:   Name of RLE image to read.                              *
*   MaxX:            Maximum X of read image is saved here.                  *
*   MaxY:            Maximum Y of read image is saved here.                  *
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  *
*		     and will return TRUE if successful in loading it.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType *:  Pointer to dynamicaly created image, NULL if non.           *
*****************************************************************************/
static IrtBType *RLEReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha)
{
#ifdef IRIT_HAVE_URT_RLE
    rle_hdr Header;
    rle_pixel **Rows;
    int Error, x, y;
    IrtBType *Data, *p;

    Header = *rle_hdr_init(NULL);
    Header.rle_file = rle_open_f_noexit("RleLoadImage",
					(char *) ImageFileName, "r");
    if (!Header.rle_file) {
        IRIT_WARNING_MSG_PRINTF("Failed to read RLE file \"%s\"\n",
				ImageFileName);
        return NULL;
    }

    if (Error = rle_get_setup(&Header)) {
        rle_get_error(Error, "RleLoadImage", (char *) ImageFileName);
        return NULL;
    }
    rle_row_alloc(&Header, &Rows);
    *MaxX = Header.xmax - Header.xmin;
    *MaxY = Header.ymax - Header.ymin;

    /* Get alpha only if requested to get it. */
    if (*Alpha)
        *Alpha = Header.alpha;

    if (*Alpha)
        Data = p = IritMalloc(4 * (*MaxX + 1) * (*MaxY + 1));
    else
        Data = p = IritMalloc(3 * (*MaxX + 1) * (*MaxY + 1));

    for (y = 0; y <= *MaxY; y++) {
        rle_getrow(&Header, Rows);
        for (x = 0; x <= *MaxX; x++) {
            *p++ = Rows[RLE_RED][x];
            *p++ = Rows[RLE_GREEN][x];
            *p++ = Rows[RLE_BLUE][x];
	    if (*Alpha)
	        *p++ = Rows[RLE_ALPHA][x];
        }
    }

    rle_close_f(Header.rle_file);

    Data = IrtImgVerifyAlignment(Data, MaxX, MaxY, *Alpha);

    return Data;
#else
    IRIT_WARNING_MSG("Utah raster tool kit is not supported\n");
    return NULL;			   /* Silent the compiler's warning. */
#endif /* IRIT_HAVE_URT_RLE */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file in GIF format.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageFileName:   Name of GIF image to read.                              *
*   MaxX:            Maximum X of read image is saved here.                  *
*   MaxY:            Maximum Y of read image is saved here.                  *
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  *
*		     and will return TRUE if successful in loading it.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType *:  Pointer to dynamicaly created image, NULL if non.           *
*****************************************************************************/
static IrtBType *GIFReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha)
{
#ifdef IRIT_HAVE_GIF_LIB
    int TranspColorIndex = -1;
    GifFileType *GifFileIn;
    IrtBType *Data, *p, *Line;
    GifRecordType RecordType;

    if ((GifFileIn = DGifOpenFileName(ImageFileName)) == NULL) {
        IRIT_WARNING_MSG_PRINTF("Failed to read GIF file \"%s\"\n",
				ImageFileName);
        return NULL;
    }

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
	int i, j, l, ExtCode;
	GifByteType *Extension;
	ColorMapObject *ColorMap;
	GifColorType *ColorMapEntry;

	if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
	    return NULL;

	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
		if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
		    return NULL;

		ColorMap =
		    (GifFileIn -> Image.ColorMap ? GifFileIn -> Image.ColorMap
						 : GifFileIn -> SColorMap);

		*MaxX = GifFileIn -> Image.Width - 1;
		*MaxY = GifFileIn -> Image.Height - 1;
		
		/* Allocate the image (padding it as well). */
		if (*Alpha)
		    *Alpha = TranspColorIndex >= 0;

		if (*Alpha)
		    Data = IritMalloc(4 * (GifFileIn -> Image.Width + 3) *
				          (GifFileIn -> Image.Height + 3));
		else
		    Data = IritMalloc(3 * (GifFileIn -> Image.Width + 3) *
				          (GifFileIn -> Image.Height + 3));
		Line = IritMalloc(GifFileIn -> Image.Width + 3);

		/* Read the image itself: */

		if (GifFileIn -> Image.Interlace) {
		    /* Interlaced images - offsets and jumps. */
		    static int
			InterlacedOffset[] = { 0, 4, 2, 1 },
			InterlacedJumps[] = { 8, 8, 4, 2 };

		    /* Need to perform 4 passes on the images: */
		    for (i = 0; i < 4; i++) {
			for (l = InterlacedOffset[i];
			     l < GifFileIn -> Image.Height;
			     l += InterlacedJumps[i]) {
			    if (DGifGetLine(GifFileIn, Line,
				      GifFileIn -> Image.Width) == GIF_ERROR) {
				IritFree(Data);
				return NULL;
			    }

			    p = &Data[(GifFileIn -> Image.Height - l - 1) *
				      GifFileIn -> Image.Width *
				      ((*Alpha) ? 4 : 3)];
			    for (j = 0; j < GifFileIn -> Image.Width; j++) {
			        ColorMapEntry = &ColorMap -> Colors[Line[j]];
				*p++ = ColorMapEntry -> Red;
				*p++ = ColorMapEntry -> Green;
				*p++ = ColorMapEntry -> Blue;
				if (*Alpha)
				    *p++ = Line[j] == TranspColorIndex ? 0 : 255;
			    }
			}
		    }
		}
		else {
		    for (i = 0; i < GifFileIn -> Image.Height; i++) {
		        if (DGifGetLine(GifFileIn, Line,
				      GifFileIn -> Image.Width) == GIF_ERROR) {
			    IritFree(Data);
			    return NULL;
			}

			p = &Data[(GifFileIn -> Image.Height - i - 1) *
				  GifFileIn -> Image.Width *
				  ((*Alpha) ? 4 : 3)];
			for (j = 0; j < GifFileIn -> Image.Width; j++) {
			    ColorMapEntry = &ColorMap -> Colors[Line[j]];
			    *p++ = ColorMapEntry -> Red;
			    *p++ = ColorMapEntry -> Green;
			    *p++ = ColorMapEntry -> Blue;
			    if (*Alpha)
			        *p++ = Line[j] == TranspColorIndex ? 0 : 255;
			}
		    }
		}
		IritFree(Line);
		break;
	    case EXTENSION_RECORD_TYPE:
		/* Skip extension blocks in file: */
		DGifGetExtension(GifFileIn, &ExtCode, &Extension);
		while (Extension != NULL) {
		    if (ExtCode == 249 &&
			Extension[0] == 4 &&
			(Extension[1] & 0x01) != 0) {
		        /* We have a transp. color specification - get it. */
		        TranspColorIndex = Extension[4];
		    }

		    DGifGetExtensionNext(GifFileIn, &Extension);
		}
		break;
	    case TERMINATE_RECORD_TYPE:
		break;
	    default:		    /* Should be traps by DGifGetRecordType. */
		break;
	}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

    DGifCloseFile(GifFileIn);

    Data = IrtImgVerifyAlignment(Data, MaxX, MaxY, *Alpha);

    return Data;
#else
    IRIT_WARNING_MSG("GifLib tool kit is not supported\n");
    return NULL;			   /* Silent the compiler's warning. */
#endif /* IRIT_HAVE_GIF_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file in PNG format.  Based on the example from libpng.       *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageFileName:   Name of PNG image to read.                              *
*   MaxX:            Maximum X of read image is saved here.                  *
*   MaxY:            Maximum Y of read image is saved here.                  *
*   Alpha:	     If TRUE, will attempt to load alpha channel if has any  *
*		     and will return TRUE if successful in loading it.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType *:  Pointer to dynamicaly created image, NULL if non.           *
*****************************************************************************/
static IrtBType *PNGReadImage(const char *ImageFileName,
			      int *MaxX,
			      int *MaxY,
			      int *Alpha)
{
#ifdef IRIT_HAVE_PNG_LIB
    int y, RowSize;
    IrtBType *Data;
    png_structp PngPtr;
    png_infop InfoPtr;
    unsigned int SigRead = 0;
    FILE *fp;

    if ((fp = fopen(ImageFileName, "rb")) == NULL) {
        IRIT_WARNING_MSG_PRINTF("Failed to read PNG file \"%s\"\n",
				ImageFileName);
        return NULL;
    }

    PngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, png_voidp_NULL, 
				    png_error_ptr_NULL, png_error_ptr_NULL);
    if (PngPtr == NULL) {
        fclose(fp);
	return NULL;
    }

    /* Allocate/initialize the memory for image information.  REQUIRED. */
    InfoPtr = png_create_info_struct(PngPtr);
    if (InfoPtr == NULL) {
	fclose(fp);
	png_destroy_read_struct(&PngPtr, png_infopp_NULL, png_infopp_NULL);
	return NULL;
    }

    if (setjmp(png_jmpbuf(PngPtr))) {
        /* Free all of the memory associated with the PngPtr and InfoPtr */
        png_destroy_read_struct(&PngPtr, &InfoPtr, png_infopp_NULL);
	fclose(fp);
	/* If we get here, we had a problem reading the file */
	return NULL;
    }

    png_init_io(PngPtr, fp);

    /* If we have already read some of the signature */
    png_set_sig_bytes(PngPtr, SigRead);

    png_read_png(PngPtr, InfoPtr,
		 (*Alpha ? 0 : PNG_TRANSFORM_STRIP_ALPHA) |
		     PNG_TRANSFORM_EXPAND,
		 png_voidp_NULL);

    if (*Alpha)
        *Alpha = InfoPtr -> color_type == PNG_COLOR_TYPE_RGB_ALPHA;

    *MaxX = InfoPtr -> width - 1;
    *MaxY = InfoPtr -> height - 1;
    RowSize = InfoPtr -> rowbytes;
    Data = IritMalloc(RowSize * (*MaxY + 1));
    for (y = 0; y <= *MaxY; y++) {
        IRIT_GEN_COPY(&Data[RowSize * y], InfoPtr -> row_pointers[*MaxY - y],
		 RowSize);
    }

    /* clean up after the read, and free any memory allocated - REQUIRED */
    png_destroy_read_struct(&PngPtr, &InfoPtr, png_infopp_NULL);

    /* close the file */
    fclose(fp);

    Data = IrtImgVerifyAlignment(Data, MaxX, MaxY, *Alpha);

    /* that's it */
    return Data;
#else
    IRIT_WARNING_MSG("LibPng tool kit is not supported\n");
    return NULL;			   /* Silent the compiler's warning. */
#endif /* IRIT_HAVE_PNG_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Parses the string of the "ptexture" attribute			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PTexture:    The string of the "ptexture" attribute.                     M
*   FName:       The texture file name will be placed here.		     M
*   Scale:       The scaling vector in XYZ or just XY if Z = IRIT_INFNTY.    M
*   Flip:	 If Image flipping was requested.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE if parsed succesfully, FALSE otherwise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgParsePTextureString                                                M
*****************************************************************************/
int IrtImgParsePTextureString(const char *PTexture,
			      char *FName,
			      IrtRType *Scale,
			      int *Flip)
{
    char *p;

    Scale[0] = Scale[1] = 1.0;
    Scale[2] = IRIT_INFNTY;
    *Flip = FALSE;

    if (PTexture == NULL)
	return FALSE;

    strncpy(FName, PTexture, IRIT_LINE_LEN_LONG - 1);

    if ((p = strchr(FName, ',')) != NULL) {
	char *q;
	float Sx, Sy, Sz;

	*p++ = 0;		      /* Mark the end of the regular string. */

	if ((q = strchr(p, 'F')) != NULL)
	    *Flip = TRUE;

	if (sscanf(p, "%f, %f, %f", &Sx, &Sy, &Sz) == 3 ||
	    ((q = strchr(p, 'S')) != NULL &&
	     sscanf(q, "S %f %f %f", &Sx, &Sy, &Sz) == 3)) {
	    Scale[0] = Sx;
	    Scale[1] = Sy;
	    Scale[2] = Sz;
	}
	else if (sscanf(p, "%f, %f", &Sx, &Sy) == 2 ||
	    ((q = strchr(p, 'S')) != NULL &&
	     sscanf(q, "S %f %f", &Sx, &Sy) == 2)) {
	    Scale[0] = Sx;
	    Scale[1] = Sy;
	}
    }

    return TRUE;
}
