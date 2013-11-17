/*****************************************************************************
* Writew one image out.							     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	May. 1999    *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"

#ifdef __WINNT__
    #include <io.h>
    #include <fcntl.h>
#endif /* __WINNT__ */

#ifdef IRIT_HAVE_URT_RLE
#define NO_DECLARE_MALLOC /* For rle.h */
#include <rle.h>
#include <rle_raw.h>
IRIT_STATIC_DATA rle_hdr *Header;
IRIT_STATIC_DATA rle_pixel **Rows;
#endif /* IRIT_HAVE_URT_RLE */

#ifdef IRIT_HAVE_PNG_LIB
#include "png.h"
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif
IRIT_STATIC_DATA png_structp GlblPngPtr;
IRIT_STATIC_DATA png_infop GlblPngInfoPtr;
IRIT_STATIC_DATA png_bytep GlblPngRows;
IRIT_STATIC_DATA FILE
    *GlblPNGFile = NULL;
#endif /* IRIT_HAVE_PNG_LIB */

IRIT_STATIC_DATA IrtImgImageType
    GlblImgType = IRIT_IMAGE_PPM6_TYPE;
IRIT_STATIC_DATA FILE
    *GlblPPMFile = NULL;
IRIT_STATIC_DATA int GlblImgWidth, GlblImgHeight, GlblImgLine, GlblImgHasAlpha;
IRIT_STATIC_DATA IrtImgPixelStruct **PixelData;

static int PPMOpenFile(const char **argv,
		       const char *FName,
		       int XSize,
		       int YSize);
static void PPMPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels);
static void PPMCloseFile(void);

static int PNGOpenFile(const char **argv,
		       const char *FName,
		       int Alpha,
		       int XSize,
		       int YSize);
static void PNGPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels);
static void PNGCloseFile(void);

static int RLEOpenFile(const char **argv,
		       const char *FName,
		       int Alpha,
		       int XSize,
		       int YSize);
static void RLEPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels);
static void RLECloseFile(void);

#ifdef IRIT_HAVE_URT_RLE
static void RLEDummyLink(void);
#endif /* IRIT_HAVE_URT_RLE */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets image type.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   ImageType:  A string describing the image type.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgImageType:  Returns the detected type.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgWriteSetType                                                       M
*****************************************************************************/
IrtImgImageType IrtImgWriteSetType(const char *ImageType)
{
    if (IRT_STR_NULL_ZERO_LEN(ImageType)) {
	IRIT_FATAL_ERROR("Empty image file name to write to.");
	return GlblImgType;
    }

    if (stricmp(ImageType, "rle") == 0)
        GlblImgType = IRIT_IMAGE_RLE_TYPE;
    else if (stricmp(ImageType, "png") == 0)
        GlblImgType = IRIT_IMAGE_PNG_TYPE;
    else if (strnicmp(ImageType, "ppm", 3) == 0) {
	switch (ImageType[3]) {
	    case '3':
		GlblImgType = IRIT_IMAGE_PPM3_TYPE;
	        break;
	    default:
	    case '6':
		GlblImgType = IRIT_IMAGE_PPM6_TYPE;
	        break;
	}
    }
    else
        GlblImgType = IRIT_IMAGE_UNKNOWN_TYPE;

    return GlblImgType;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Opens output image file.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   argv:     Pointer to the name of this program.                           M
*   FName:    Filename to open, or NULL for stdout.			     M
*   Alpha:    Do we have aan alpha channel.				     M
*   XSize:    X dimension of the image.                                      M
*   YSize:    Y dimension of the image.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgWriteOpenFile, output image                                        M
*****************************************************************************/
int IrtImgWriteOpenFile(const char **argv,
			const char *FName,
			int Alpha,
			int XSize,
			int YSize)
{
    GlblImgHasAlpha = Alpha;
    GlblImgWidth = XSize;
    GlblImgHeight = YSize;
    GlblImgLine = 0;

    switch (GlblImgType) {
	default:
	case IRIT_IMAGE_RLE_TYPE:
	    return RLEOpenFile(argv, FName, Alpha, XSize, YSize);
	    break;
	case IRIT_IMAGE_PNG_TYPE:
	    return PNGOpenFile(argv, FName, Alpha, XSize, YSize);
	    break;
	case IRIT_IMAGE_PPM3_TYPE:
	case IRIT_IMAGE_PPM6_TYPE:
	    return PPMOpenFile(argv, FName, XSize, YSize);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Outputs given line of color pixels and alpha correction values to the    M
*   ouput image file.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Alpha:  array of alpha values.                                           M
*   Pixels: array of color pixels.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgWritePutLine, output image                                         M
*****************************************************************************/
void IrtImgWritePutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels)
{
    switch (GlblImgType) {
	default:
	case IRIT_IMAGE_RLE_TYPE:
	    RLEPutLine(Alpha, Pixels);
	    break;
	case IRIT_IMAGE_PNG_TYPE:
	    PNGPutLine(Alpha, Pixels);
	    break;
	case IRIT_IMAGE_PPM3_TYPE:
	case IRIT_IMAGE_PPM6_TYPE:
	    PPMPutLine(Alpha, Pixels);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Closes output image file.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtImgWriteCloseFile, output image                                       M
*****************************************************************************/
void IrtImgWriteCloseFile(void)
{
    switch (GlblImgType) {
	default:
	case IRIT_IMAGE_RLE_TYPE:
	    RLECloseFile();
	    break;
	case IRIT_IMAGE_PNG_TYPE:
	    PNGCloseFile();
	    break;
	case IRIT_IMAGE_PPM3_TYPE:
	case IRIT_IMAGE_PPM6_TYPE:
	    PPMCloseFile();
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Opens output image file in PPM format.                                   *
*   Creates file header. Note we allocate full image size since ppm expects  *
* lines top to bottom and we scan convert bottom to top.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   argv:     Pointer to the name of this program.                           *
*   FName:    Filename to open, or NULL for stdout.			     *
*   XSize:    X dimension of the image.                                      *
*   YSize:    Y dimension of the image.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE otherwise.                               *
*****************************************************************************/
static int PPMOpenFile(const char **argv,
		       const char *FName,
		       int XSize,
		       int YSize)
{
    int i;

    if (GlblPPMFile != stdout && GlblPPMFile != NULL)
        fclose(GlblPPMFile);

    if (FName == NULL)
        GlblPPMFile = stdout;
    else {
        if ((GlblPPMFile = fopen(FName, "w")) == NULL)
	    return FALSE;
    }

    PixelData = (IrtImgPixelStruct **) IritMalloc(GlblImgHeight *
						  sizeof(IrtImgPixelStruct *));

    for (i = 0; i < GlblImgHeight; i++) {
	PixelData[i] =
	    (IrtImgPixelStruct *) IritMalloc((GlblImgWidth + 4) *
					     sizeof(IrtImgPixelStruct));
    }

    fprintf(GlblPPMFile,
#ifdef IRIT_QUIET_STRINGS
	    "%s\n# image file 24 bit rgb\n%d %d\n%d\n",
#else
	    "%s\n# IRIT Irender image file 24 bit rgb\n%d %d\n%d\n",
#endif /* IRIT_QUIET_STRINGS */
	    GlblImgType == IRIT_IMAGE_PPM3_TYPE ? "P3" : "P6",
	    GlblImgWidth, GlblImgHeight, 255);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Outputs given line of color pixels and alpha correction values to the    *
*   ouput image file.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Alpha:  Array of alpha values.                                           *
*   Pixels: Array of color pixels.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PPMPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels)
{
    if (GlblImgLine < GlblImgHeight)
	IRIT_GEN_COPY(PixelData[GlblImgLine++], Pixels,
		 GlblImgWidth * sizeof(IrtImgPixelStruct));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Closes output image file.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PPMCloseFile(void)
{
    int i;

    for (i = 0; i < GlblImgHeight; i++) {
	if (GlblImgType == IRIT_IMAGE_PPM3_TYPE) {
	    int j;
	    IrtImgPixelStruct
		*Pixels = PixelData[GlblImgHeight - i - 1];

	    for (j = 0; j < GlblImgWidth; j++, Pixels++) {
		fprintf(GlblPPMFile, j % 6 == 0 && j > 0 ? "\n%3d %3d %3d "
						         : "%3d %3d %3d ",
			Pixels -> r, Pixels -> g, Pixels -> b);
	    }
	    if (j % 6 != 0)
		fprintf(GlblPPMFile, "\n");
	}
	else {
#ifdef __WINNT__
	    if (i == 0)
		_setmode(_fileno(GlblPPMFile), O_BINARY);
#endif /* __WINNT__ */
	    fwrite(PixelData[GlblImgHeight - i - 1],
		   GlblImgWidth * sizeof(IrtImgPixelStruct), 1, GlblPPMFile);
	}

	IritFree(PixelData[GlblImgHeight - i - 1]);
    }
    IritFree(PixelData);

    if (GlblPPMFile != NULL)
	fclose(GlblPPMFile);

    GlblPPMFile = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Opens output image file in PNG format.                                   *
*   Creates file header. Note we allocate full image size since ppm expects  *
* lines top to bottom and we scan convert bottom to top.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   argv:     Pointer to the name of this program.                           *
*   FName:    Filename to open, or NULL for stdout.			     *
*   Alpha:    Do we have Alpha channel.					     *
*   XSize:    X dimension of the image.                                      *
*   YSize:    Y dimension of the image.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE otherwise.                               *
*****************************************************************************/
static int PNGOpenFile(const char **argv,
		       const char *FName,
		       int Alpha,
		       int XSize,
		       int YSize)
{
#ifdef IRIT_HAVE_PNG_LIB
    /* Open the file. */
    if (FName == NULL) {
        GlblPNGFile = stdout;
#       if defined(__WINNT__) || defined(OS2GCC)
	    if (_fileno(stdout) >= 0)
	        setmode(_fileno(stdout), O_BINARY);
#	endif /* __WINNT__ || OS2GCC */
    }
    else {
#       if defined(__WINNT__) || defined(OS2GCC)
	    if ((GlblPNGFile = fopen(FName, "wb")) == NULL)
#	else
	    if ((GlblPNGFile = fopen(FName, "w")) == NULL)
#	endif /* __WINNT__ || OS2GCC */
		return FALSE;
    }

    if ((GlblPngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
					     (png_voidp) NULL,
					     NULL, NULL)) == NULL ||
	(GlblPngInfoPtr = png_create_info_struct(GlblPngPtr)) == NULL ||
	setjmp(png_jmpbuf(GlblPngPtr))) {
        fclose(GlblPNGFile);
	return FALSE;
    }

    /* set up the output control if you are using standard C streams */
    png_init_io(GlblPngPtr, GlblPNGFile);
    png_set_compression_level(GlblPngPtr, Z_BEST_COMPRESSION);
    png_set_IHDR(GlblPngPtr, GlblPngInfoPtr,
		 XSize,			/* Width. */
		 YSize,			/* Height. */
		 8,			/* Bit depth. */
		 Alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
		 PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT,
		 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(GlblPngPtr, GlblPngInfoPtr);

    GlblPngRows = (png_bytep) IritMalloc(sizeof(png_byte) * 4
					 * GlblImgWidth * GlblImgHeight);

    return TRUE;
#else
    IRIT_WARNING_MSG("PNG image format is not supported\n");
    return FALSE;
#endif /* IRIT_HAVE_PNG_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Outputs given line of color pixels and alpha correction values to the    *
*   ouput image file.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Alpha:  Array of alpha values.                                           *
*   Pixels: Array of color pixels.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PNGPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels)
{
#ifdef IRIT_HAVE_PNG_LIB
    int x,
	BytesPerLine = (GlblImgHasAlpha ? 4 : 3) * GlblImgWidth;
    png_byte
	*p = &GlblPngRows[BytesPerLine * GlblImgLine++];

    if (GlblImgLine > GlblImgHeight)
	return; /* Play it safe. */

    for (x = 0; x < GlblImgWidth; x++, Pixels++) {
	*p++ = Pixels -> r;
	*p++ = Pixels -> g;
	*p++ = Pixels -> b;
	if (Alpha != NULL)
	    *p++ = *Alpha++;
    }
#endif /* IRIT_HAVE_PNG_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Closes output image file.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PNGCloseFile(void)
{
#ifdef IRIT_HAVE_PNG_LIB
    int y,
	BytesPerLine = (GlblImgHasAlpha ? 4 : 3) * GlblImgWidth;

    for (y = GlblImgHeight - 1; y >= 0; y--)
        png_write_row(GlblPngPtr,
		      &GlblPngRows[BytesPerLine * y]);
    IritFree(GlblPngRows);

    png_write_end(GlblPngPtr, GlblPngInfoPtr);
    png_destroy_write_struct(&GlblPngPtr, &GlblPngInfoPtr);
    fclose(GlblPNGFile);
#endif /* IRIT_HAVE_PNG_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Opens output image file in RLE format.                                   *
*   Creates file header.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   argv:      Pointer to the command line parameters.                       *
*   FName:     Filename to open, or NULL for stdout.			     *
*   Alpha:     Do we have Alpha channel.				     *
*   XSize:     X dimension of the image.                                     *
*   YSize:     Y dimension of the image.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE otherwise.                               *
*****************************************************************************/
static int RLEOpenFile(const char **argv,
		       const char *FName,
		       int Alpha,
		       int XSize,
		       int YSize)
{
#ifdef IRIT_HAVE_URT_RLE
#ifdef URT_OLD_COMPAT
    Header = &rle_dflt_hdr;
    Header -> rle_file = rle_open_f("", FName, "w");
#else
    Header = rle_hdr_init((rle_hdr *) NULL);
    if (argv != NULL)
      rle_names(Header, (char *) *argv, NULL, 0);
    Header -> rle_file = rle_open_f(Header -> cmd, (char *) FName, "w");
#endif /* URT_OLD_COMAP */

    if (Header -> rle_file == NULL)
        return FALSE;

    if (argv != NULL)
	rle_addhist((char **) argv, (rle_hdr *) NULL, Header);
    if (Alpha) {
        RLE_SET_BIT(*Header, RLE_ALPHA);
	Header -> alpha = 1;
    }
    Header -> xmax = --XSize;
    Header -> ymax = --YSize;
    rle_put_setup(Header);
    rle_row_alloc(Header, &Rows);

    return TRUE;
#else
    IRIT_WARNING_MSG("Utah raster tool kit is not supported\n");
    return FALSE;
#endif /* IRIT_HAVE_URT_RLE */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Outputs given line of color pixels and alpha correction values to the    *
*   ouput image file.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Alpha:  Array of alpha values.                                           *
*   Pixels: Array of color pixels.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLEPutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels)
{
#ifdef IRIT_HAVE_URT_RLE
    int x;

    for (x = 0; x <= Header -> xmax; x++) {
        IrtImgPixelStruct
	    *p = &Pixels[x];

	if (Alpha != NULL)
	    Rows[RLE_ALPHA][x] = Alpha[x];
        Rows[RLE_RED][x] = p -> r;
        Rows[RLE_GREEN][x] = p -> g;
        Rows[RLE_BLUE][x] = p -> b;
    }
    rle_putrow(Rows, Header -> xmax + 1, Header);
#endif /* IRIT_HAVE_URT_RLE */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Closes output image file.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLECloseFile(void)
{
#ifdef IRIT_HAVE_URT_RLE
    rle_puteof(Header);
    rle_close_f(Header -> rle_file);
#endif /* IRIT_HAVE_URT_RLE */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Force the link of necessary rle functions so readimag.c in misc_lib      *
* would function, even if shared library.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef IRIT_HAVE_URT_RLE
static void RLEDummyLink(void)
{
    rle_open_f_noexit(NULL, NULL, NULL);
    rle_get_setup(NULL);
    rle_row_alloc(NULL, NULL);
    rle_getrow(NULL, NULL);
}
#endif /* IRIT_HAVE_URT_RLE */
