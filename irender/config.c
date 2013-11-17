/*****************************************************************************
* Global options configuration from config file and command line arguments.  *
* (config file name is irender.cfg, and it contains program options)         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
*****************************************************************************/

#include "rndr_lib.h"
#include "misc_lib.h"
#include "grap_lib.h"
#include "config.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "IRender	Version 11,	Gershon Elber,\n\
	(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char 
    *VersionStr = "IRender		" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",  " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char TheHelp[] =
{
    " -d\t\t\twrite Z Depth instead of color\n"
    " -l\t\t\twrite stencil buffer instead of color\n"
    " -V\t\t\twrite visibility map instead of color\n"
    " -Z Znear Zfar \t\tset near and far clipping planes \n"
    " -n\t\t\treverse vertex and plane normals direction\n"
    " -i\t\t\tsets image type to handle (rle/ppm)\n"
    " -T\t\t\thandle transparency\n"
    " -B\t\t\tapply backface culling\n"
    " -N\t\t\tClrQnt SilWidth { SilR SilG SilB } (NPR rendering)\n"
    " -v\t\t\tverbose mode\n"
    " -P Wmin [Wmax]\t\thandle polylines\n"
    " -a Ambient\t\t(% of ambient component) in (0, 1)\n"
    " -F n m\t\t\tsurface to polygons approximation optimal and fineness\n"
    " -f n m\t\t\tcurve to polylines approximation optimal and samples\n"
    " -s xSize ySize\t\tgeometry of resulting image\n"
    " -M Flat/Gouraud/Phong/None\tshade model\n"
    " -A FilterName\t\tapply filter (sinc, box, triangle etc.)\n"
    " -b R G B\t\tcolor of background\n"
    " -t\t\t\tanimation time\n"
    " -o OutName\t\tOutput filename\n"
    " -z\t\t\tthis message and more help\n"
    " file...\t\tfiles to proceed { *.(dat|mat)[.Z] }\n"
};

IRIT_STATIC_DATA const char
    *BgRGB = "0 0 0",
    *NPRSilRGB = "255 255 255",
    *Model = "Phong",
    *ImageTypeStr = "rle";

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
    { "ZDepth",      "-d", (VoidPtr) &Options.ZDepth,        IC_BOOLEAN_TYPE },
    { "Stencil",     "-l", (VoidPtr) &Options.Stencil,       IC_BOOLEAN_TYPE },
    { "VisMap",      "-V", (VoidPtr) &Options.VisMap,        IC_BOOLEAN_TYPE },
    { "Normals",     "-n", (VoidPtr) &Options.NormalReverse, IC_BOOLEAN_TYPE },
    { "PPMStyleP3",  "-i", (VoidPtr) &Options.PPMStyleP3,    IC_BOOLEAN_TYPE },
    { "ShadowCast",  "-S", (VoidPtr) &Options.Shadows,       IC_BOOLEAN_TYPE },
    { "Transparency","-T", (VoidPtr) &Options.Transp,        IC_BOOLEAN_TYPE },
    { "BackfaceCull","-B", (VoidPtr) &Options.BackFace,      IC_BOOLEAN_TYPE },
    { "Verbose",     "-v", (VoidPtr) &Options.Verbose,       IC_BOOLEAN_TYPE },
    { "PllnMinWidth","-P", (VoidPtr) &Options.PllMinW,       IC_REAL_TYPE },
    { "PllnMaxWidth","-P", (VoidPtr) &Options.PllMaxW,       IC_REAL_TYPE },
    { "Ambient",     "-a", (VoidPtr) &Options.Ambient,       IC_REAL_TYPE },
    { "SrfFineNess", "-F", (VoidPtr) &Options.Srf2PlgFineness,IC_REAL_TYPE },
    { "PolygonOpti", "-F", (VoidPtr) &Options.Srf2PlgOptimal,IC_INTEGER_TYPE },
    { "SamplesPerCurve","-f",(VoidPtr) &Options.Crv2PllSamples,IC_INTEGER_TYPE },
    { "PolylineOpti","-f", (VoidPtr) &Options.Crv2PllMethod, IC_INTEGER_TYPE },
    { "ImageWidth",  "-s", (VoidPtr) &Options.XSize,         IC_INTEGER_TYPE },
    { "ImageHeight", "-s", (VoidPtr) &Options.YSize,         IC_INTEGER_TYPE },
    { "NPRClrQnt",   "-N", (VoidPtr) &Options.NPRClrQuant,   IC_INTEGER_TYPE },
    { "ShadeModel",  "-M", (VoidPtr) &Model,                 IC_STRING_TYPE },
    { "FilterName",  "-A", (VoidPtr) &Options.FilterName,    IC_STRING_TYPE },
    { "Background",  "-b", (VoidPtr) &BgRGB,                 IC_STRING_TYPE },
    { "ImageType",   "-i", (VoidPtr) &ImageTypeStr,          IC_STRING_TYPE },
    { "NPRSilColor", "-b", (VoidPtr) &NPRSilRGB,             IC_STRING_TYPE },
    { "NPRSilWidth", "-N", (VoidPtr) &Options.NPRSilWidth,   IC_REAL_TYPE },
    { "ZClipMin",    "-Z", (VoidPtr) &Options.ZNear,         IC_REAL_TYPE },
    { "ZClipMax",    "-Z", (VoidPtr) &Options.ZFar,          IC_REAL_TYPE }

};
#define NUM_SET_UP    (sizeof(SetUp) / sizeof(IritConfigStruct))

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reports a warning message.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Txt:       IN, message text.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReportWarning(char *Txt)
{
    IRIT_WARNING_MSG_PRINTF("irender config: (Warning) %s\n", Txt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads configuration file and initialize global state of the program.     M
*   It has lowest priorety in the configuration management initializations.  M
*   Checks validity of the options and prints error messages.                M
*                                                                            *
* PARAMETERS:                                                                M
*   argv:     IN, pointer to the command line parameters of the program.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetConfig, configuration, global state                                   M
*****************************************************************************/
void GetConfig(char *argv[])
{
    IritConfig("irender", SetUp, NUM_SET_UP);

    if (BgRGB) {
        int r, g, b;

        if (sscanf(BgRGB, "%d, %d, %d", &r, &g, &b) == 3 ||
	    sscanf(BgRGB, "%d %d %d", &r, &g, &b) == 3) {
            Options.BackGround[RED_CLR] = r;
            Options.BackGround[GREEN_CLR] = g;
	    Options.BackGround[BLUE_CLR] = b;
            IRIT_PT_SCALE(Options.BackGround, 1.0 / 255.0);
        }
    }

    if (NPRSilRGB) {
        int r, g, b;

        if (sscanf(NPRSilRGB, "%d, %d, %d", &r, &g, &b) == 3 ||
	    sscanf(NPRSilRGB, "%d %d %d", &r, &g, &b) == 3) {
            Options.NPRSilColor[RED_CLR] = r;
            Options.NPRSilColor[GREEN_CLR] = g;
	    Options.NPRSilColor[BLUE_CLR] = b;
        }
    }

    if (Options.VisMap) {
        Options.ShadeModel = IRNDR_SHADING_FLAT;
        Options.Shadows = FALSE;
        Options.Transp = FALSE;
    }

    if (Options.ZDepth || Options.Stencil) {
        Options.ShadeModel = IRNDR_SHADING_FLAT;
        Options.Shadows = FALSE;
        Options.Transp = FALSE;
        Options.FilterName = NULL;
    }
    else if (Model) {
        if (!stricmp(Model, "Flat"))
            Options.ShadeModel = IRNDR_SHADING_FLAT;
        else if (!stricmp(Model, "Gouraud"))
            Options.ShadeModel = IRNDR_SHADING_GOURAUD;
        else if (!stricmp(Model, "Phong"))
            Options.ShadeModel = IRNDR_SHADING_PHONG;
        else if (!stricmp(Model, "None"))
            Options.ShadeModel = IRNDR_SHADING_NONE;
        else
            ReportWarning("Unknown shading model, Phong used\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads command line options, checks validity, and initialize global state M
*   of the program. It has the greatest priority in configuration management.M
*                                                                            *
* PARAMETERS:                                                                M
*   argc:    IN, number of command line parameters.                          M
*   argv:    IN, command line parameters.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetOptions, configuration, global state                                  M
*****************************************************************************/
void GetOptions(int argc, char *argv[])
{
    IrtRType Min,
        Max = -IRIT_INFNTY;
    int Error, Dummy, BgRed, BgGreen, BgBlue,
        OutNameFlag = FALSE,
        BgRGBFlag = FALSE,
	VerFlag = FALSE,
        Plls = FALSE,
        ImTp = FALSE;
    const char
	*Format = "v%- s%-XSize|YSize!d!d Z%-Znear|[Zfar]!F%F a%-Ambient!F b%-R|G|B!d!d!d B%- F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampPerCrv!d!d M%-Flat/Gouraud/Phong/None!s p%-PtRad!F P%-WMin|[WMax]!F%F S%- T%- t%-AnimTime!F N%-ClrQuant|SilWidth|[SilR|SilG|SilB]!d!F%F%F%F A%-FilterName!s d%- l%- V%- n%- i%-rle/ppm{3,6}!s o%-OutName!s z%- files!*s";
    char Line[IRIT_LINE_LEN],
        *Control = MALLOC(char, (int) (strlen(argv[0]) + strlen(Format)) + 2);

    Options.ZNear = Options.ZFar = -IRIT_INFNTY;
    Options.DrawPoints = TRUE;
    Options.PointDfltRadius = 0.01;

    IRIT_PT_SCALE(Options.BackGround, 0xff);
    BgRed = (int) Options.BackGround[RED_CLR];
    BgGreen = (int) Options.BackGround[GREEN_CLR];
    BgBlue = (int) Options.BackGround[BLUE_CLR];
    sprintf(Control, "%s %s", argv[0], Format);
    Error = GAGetArgs(argc, argv, Control,
		      &Options.Verbose,
                      &Dummy, &Options.XSize, &Options.YSize,
                      &Dummy, &Options.ZNear, &Options.ZFar,
                      &Dummy, &Options.Ambient,
                      &BgRGBFlag, &BgRed, &BgGreen, &BgBlue,
                      &Options.BackFace, &Dummy,
		      &Options.Srf2PlgOptimal, &Options.Srf2PlgFineness,
                      &Dummy, &Options.Crv2PllMethod, &Options.Crv2PllSamples,
                      &Dummy, &Model,
		      &Options.DrawPoints, &Options.PointDfltRadius,
                      &Plls, &Min, &Max,
                      &Options.Shadows,
                      &Options.Transp,
                      &Options.HasTime, &Options.Time,
		      &Options.NPRRendering, &Options.NPRClrQuant,
		      &Options.NPRSilWidth, &Options.NPRSilColor[RED_CLR],
		      &Options.NPRSilColor[GREEN_CLR],
		      &Options.NPRSilColor[BLUE_CLR],
                      &Dummy, &Options.FilterName,
                      &Options.ZDepth, &Options.Stencil, &Options.VisMap,
                      &Options.NormalReverse,
		      &ImTp, &ImageTypeStr,
		      &OutNameFlag, &Options.OutFileName, &VerFlag,
                      &Options.NFiles, &Options.Files);

    if (Plls) {
        Options.Polylines = Plls;
        Options.PllMinW = Min;
        Options.PllMaxW = Max > 0 ? Max : Options.PllMinW;
    }

    if (Options.VisMap) {
        Options.ShadeModel = IRNDR_SHADING_FLAT;
        Options.Shadows = FALSE;
        Options.Transp = FALSE;
    }
    else if (Options.ZDepth || Options.Stencil) {
        Options.ShadeModel =IRNDR_SHADING_FLAT;
        Options.Shadows = FALSE;
        Options.Transp = FALSE;
        Options.FilterName = NULL;
    }
    else if (Model) {
        if (!stricmp(Model, "Flat"))
            Options.ShadeModel = IRNDR_SHADING_FLAT;
        else if (!stricmp(Model, "Gouraud"))
            Options.ShadeModel = IRNDR_SHADING_GOURAUD;
        else if (!stricmp(Model, "Phong"))
            Options.ShadeModel = IRNDR_SHADING_PHONG;
        else if (!stricmp(Model, "None"))
            Options.ShadeModel = IRNDR_SHADING_NONE;
        else
            ReportWarning("Unknown shading model, Phong used\n");
    }

    if (BgRGBFlag) {
        /* Make config know about this. */
        sprintf(Line, "%d, %d, %d", BgRed, BgGreen, BgBlue);
	BgRGB = IritStrdup(Line);

	Options.BackGround[RED_CLR] = BgRed;
	Options.BackGround[GREEN_CLR] = BgGreen;
	Options.BackGround[BLUE_CLR] = BgBlue;
	IRIT_PT_SCALE(Options.BackGround, 1.0 / 255.0);
    }

    if (Options.FilterName != NULL &&
	(IRT_STR_ZERO_LEN(Options.FilterName) ||
	 stricmp(Options.FilterName, "None") == 0))
        Options.FilterName = NULL;

    Options.FileType = ImageTypeStr;

    if (!OutNameFlag)
        Options.OutFileName = NULL;

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
        GAPrintHowTo(Control);
	IritConfigPrint(SetUp, NUM_SET_UP);
        IRIT_INFO_MSG_PRINTF("%s\n", TheHelp);
        exit(0);
    }
    if (Error) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	IritConfigPrint(SetUp, NUM_SET_UP);
        GAPrintErrMsg(Error);
        GAPrintHowTo(Control);
        exit(Error);
    }
    if (!Options.NFiles) {
        IRIT_WARNING_MSG("Error in command line parsing - No files found.\n");
        GAPrintHowTo(Control);
        exit(1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize global options with default values.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InitOptions				                                     M
*****************************************************************************/
void InitOptions(void)
{
    Options.XSize = 512;
    Options.YSize = 512;
    Options.Ambient =0.2;
    Options.BackGround[RED_CLR] =
    Options.BackGround[GREEN_CLR] =
    Options.BackGround[BLUE_CLR] = 0.0;
    Options.BackFace = FALSE;
    Options.ShadeModel = IRNDR_SHADING_PHONG;
    Options.Polylines = FALSE;
    Options.Transp = FALSE;
    Options.HasTime = FALSE;
    Options.FilterName = "None";
    Options.ZDepth = FALSE;
    Options.Stencil = FALSE;
    Options.VisMap = FALSE;
    Options.NormalReverse = FALSE;
    Options.Polylines = FALSE;
    Options.PllMinW = 1.0;
    Options.PllMaxW = 1.0;
    Options.Srf2PlgOptimal = 0;
    Options.Srf2PlgFineness = IG_DEFAULT_POLYGON_FINENESS;
    Options.Crv2PllMethod = SYMB_CRV_APPROX_UNIFORM;
    Options.Crv2PllSamples = IG_DEFAULT_SAMPLES_PER_CURVE;
    Options.Verbose = FALSE;
    Options.ZFar = 0;
    Options.ZNear = 0;
    Options.NPRClrQuant = 0;
    Options.NPRSilWidth = 0.0;
    Options.NPRSilColor[0] = 255;
    Options.NPRSilColor[1] = 255;
    Options.NPRSilColor[2] = 255;
}
