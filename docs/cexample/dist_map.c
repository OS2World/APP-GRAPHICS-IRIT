/*****************************************************************************
* Computes a 2D grid of distances approximating the distance field to a crv. *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 2003   *
*****************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "misc_lib.h"

#define MAP_DIST_2_COLOR(RelDist, Pxl)  { \
	(Pxl) -> r = ((unsigned char) ((RelDist) * 255)); \
	(Pxl) -> g = ((unsigned char) (4 * (RelDist) * (1.0 - (RelDist)) * 255)); \
	(Pxl) -> b = ((unsigned char) ((1.0 - (RelDist)) * 255)); }

static char *CtrlStr =
#ifdef IRIT_DOUBLE
    "dist_map x%-XMin|XMax!F!F y%-YMin|YMax!F!F t%-Tolerance!F h%- I!-ImageName|XSize|YSize!s!d!d IritFile!*s";
#else
    "dist_map x%-XMin|XMax!f!f y%-YMin|YMax!f!f t%-Tolerance!f h%- I!-ImageName|XSize|YSize!s!d!d IritFile!*s";
#endif /* IRIT_DOUBLE */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module - Reads command line and do what is needed...		     M
* Example: "dist_map.exe -I cubic1.ppm 100 100 cubic1.itd"		     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
void main(int argc, char **argv)
{
    int NumFiles, Error,
	TolFlag = FALSE,
	XDomainFlag = FALSE,
	YDomainFlag = FALSE,
	HelpFlag = FALSE,
	ImageFlag = FALSE,
	ImageXSize = 100,
	ImageYSize = 100;
    IrtRType
	XDomain[2] = { -1.0, 1.0 },
	YDomain[2] = { -1.0, 1.0 },
	Tolerance = 0.01;
    char **FileNames,
	*ImageName = NULL;
    IPObjectStruct *PObjs;

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &XDomainFlag, &XDomain[0], &XDomain[1],
			   &YDomainFlag, &YDomain[0], &YDomain[1],
			   &TolFlag, &Tolerance,
			   &HelpFlag, &ImageFlag, &ImageName,
			   &ImageXSize, &ImageYSize,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(1);
    }

    if (ImageName == NULL || NumFiles != 1) {
	fprintf(stderr, "No image or irit data files to process.\n");
	exit(2);
    }

    if (HelpFlag) {
	GAPrintHowTo(CtrlStr);
	exit(0);
    }

    fprintf(stderr, "Processing domain [%f : %f] x [%f : %f]\n\tto image \"%s\" of size [%d x %d]\n",
	    XDomain[0], XDomain[1],
	    YDomain[0], YDomain[1],
	    ImageName, ImageXSize, ImageYSize);

    /* Get the data from all the input files. */
    IPSetFlattenObjects(TRUE);
    if ((PObjs = IPGetDataFiles(FileNames, NumFiles, TRUE, TRUE)) == NULL) {
	fprintf(stderr, "Failed to load file \"%s\"\n", FileNames[0]);
	exit(1);
    }
    fprintf(stderr, "Successfully loaded crv geometry \"%s\"\n",
	    FileNames[0]);

    /* Ray trace the glass object. */
    if (IP_IS_CRV_OBJ(PObjs)) {
	int i, j;
        IrtRType MaxDist, **Image;
        IrtImgPixelStruct
	    *ImageLine = (IrtImgPixelStruct *)
		IritMalloc(sizeof(IrtImgPixelStruct) * ImageXSize);

	Image = (IrtRType **) IritMalloc(sizeof(IrtRType *) * ImageYSize);
	for (j = 0; j < ImageYSize; j++)
	    Image[j] = (IrtRType *) IritMalloc(sizeof(IrtRType) * ImageXSize);

	MaxDist = SymbDistBuildMapToCrv(PObjs -> U.Crvs, Tolerance,
					XDomain, YDomain,
					Image, ImageXSize, ImageYSize);

	/* Dump the map as an image. */
	IrtImgWriteOpenFile(argv, ImageName, FALSE, ImageXSize, ImageYSize);

	for (j = 0; j < ImageYSize; j++) {
	    for (i = 0; i < ImageXSize; i++)
		MAP_DIST_2_COLOR(Image[i][j] / MaxDist, &ImageLine[i]);

	    IrtImgWritePutLine(NULL, ImageLine);
	}

	IrtImgWriteCloseFile();

	for (j = 0; j < ImageYSize; j++)
	    IritFree((VoidPtr) Image[j]);
	IritFree((VoidPtr) Image);
	IritFree((VoidPtr) ImageLine);


    }
    else {
        fprintf(stderr, "... but expected a curve, aborting!\n");
    }

    exit(0);
}
