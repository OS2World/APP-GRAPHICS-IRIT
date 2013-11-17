/*****************************************************************************
* Filter to convert IRIT data files to a PostScript file.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

#define DEFAULT_LINE_WIDTH		0.004
#define DEFAULT_SIZE			7.0
#define DEFAULT_DEPTH_CUE_CRV_SIZE	1.0
#define DEFAULT_FONT_SCALE		0.1 
#define MAX_NUM_OF_PTS			100
#define MIN_PS_LINE_WIDTH		0.002
#define BBOX_BNDRY_OFFSET		4   /* Add 4 pixel boundary to bbox. */

#define WIDEN_END_START			1
#define WIDEN_END_END			2
#define WIDEN_LN_SUBDIV_FACTOR		0.1

#define DIST_PT_PT_2D(Pt1, Pt2) sqrt(IRIT_SQR(Pt1[0] - Pt2[0]) + \
				     IRIT_SQR(Pt1[1] - Pt2[1]))

typedef enum {
    DRAW_POINT_CROSS,
    DRAW_POINT_FULL_CIRC,
    DRAW_POINT_HOLLOW_CIRC
} DrawIrtPtType;

#ifdef NO_CONCAT_STR 
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2PS		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2PS		" IRIT_VERSION
	",	Gershon Elber,	" __DATE__ ",   " __TIME__ "\n"
	IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2ps l%- 4%- s%-Size!F I%-#UIso[:#VIso[:#WIso]]!s F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!F M%- G%- P%- W%-LineWidth!F w%-WidenLen|WidenWidth!F!F b%-R|G|B!d!d!d B%-X1|Y1|X2|Y2!F!F!F!F c%- C%- T%- t%-AnimTime!F N%-FontName!s i%- o%-OutName!s d%-[Zmin|Zmax]%F%F D%-[Zmin|Zmax]%F%F p%-PtType|PtSize!s!F u%- z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *GlblFontName = "Times-Roman",
    *GlblPSTitle = NULL,
    *OutFileName = "irit2ps.ps";

IRIT_STATIC_DATA DrawIrtPtType
    GlblDrawPoint = DRAW_POINT_FULL_CIRC;

IRIT_STATIC_DATA int
    GlblColorPS = FALSE,
    GlblBBoxClip = FALSE,
    GlblWidenPolyEnds = FALSE,
    GlblDepthCue = FALSE,
    GlblDepthCueGray = FALSE,
    GlblBackGround = FALSE,
    GlblBackGroundColor[3] = { 0, 0, 0 };

IRIT_STATIC_DATA IrtRType
    GlblLineWidth = DEFAULT_LINE_WIDTH,
    GlblDepthCueZ[2] = {IRIT_INFNTY, -IRIT_INFNTY},
    GlblBBoxClipDomain[4],
    GlblPointScale = 0.01,
    GlblWidenEndLength = 0.1,
    GlblWidenEndWidthScale = 2.0,
    GlblPrintSize = DEFAULT_SIZE;

IRIT_STATIC_DATA GMBBBboxStruct GlblBbox;

IRIT_STATIC_DATA FILE
    *OutputFile = NULL;

IRIT_STATIC_DATA IrtHmgnMatType
    GlblCrntViewMat;

static IrtRType RGBtoGray(int R, int G, int B);
static void DumpDataForPostScript(const char *FileName, 
				  IPObjectStruct *PObjects,
				  IrtHmgnMatType CrntViewMat);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void DumpAttrState(FILE *f,
			  IrtRType Width,
			  IrtRType Gray,
			  IrtRType *RGBColor,
			  const char *Dash);
static void DumpOneObject(FILE *f, IPObjectStruct *PObject);
static void DumpOnePoly(FILE *f,
			IPPolygonStruct *PPolygon,
			int IsPolygon,
			int Fill,
			int IgnoreDepthCue,
			IrtRType Gray,
			IrtRType LineWidth,
			IrtRType *RGBColor);
static IrtRType PolylineLength2D(IPVertexStruct *V);
static IrtRType EdgeLength2D(IPVertexStruct *V);
static void SubdivAndWidenEdge(IPVertexStruct *V,
			       IrtRType Scale1,
			       IrtRType Scale2,
			       IrtRType LineWidth,
			       IrtRType EdgePortion);
static void DumpOneCubicCurve(FILE *f,
			      CagdCrvStruct *Crv,
			      int Fill,
			      IrtRType Gray,
			      IrtRType LineWidth,
			      IrtRType *RGBColor);
static void DumpOnePoint(FILE *f,
			 IrtRType *Pt,
			 int IsVector,
			 IrtRType *RGBColor,
			 IrtRType Gray,
			 IrtRType Width);
static void DumpOneString(FILE *f,
			IPObjectStruct *StringObj,
			IrtRType *RGBColor,
			IrtRType Gray);
static void Irit2PsExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2ps - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int Error,
	HasTime = FALSE,
	DrawFFAsPolylines = TRUE,
	DrawFFAsPolygons = FALSE,
	PrintSizeFlag = FALSE,
	NumOfIsolinesFlag = FALSE,
	SrfFineNessFlag = FALSE,
	CrvOptiPolylineFlag = FALSE,
        LineWidthFlag = FALSE,
	VerFlag = FALSE,
	PointFlag = FALSE,
	FontNameFlag = FALSE,
	ForceUnitMat = FALSE,
	OutFileFlag = FALSE,
	NumFiles = 0;
    char *DrawPoint,
        *StrNumOfIsolines = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects, *PObj;
    GMBBBboxStruct
	*PGlblBbox = NULL;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &PrintSizeFlag,
			   &GlblPrintSize, &NumOfIsolinesFlag,
			   &StrNumOfIsolines, &SrfFineNessFlag,
			   &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &CrvOptiPolylineFlag,
			   &IPFFCState.CrvApproxMethod,
			   &IPFFCState.CrvApproxTolSamples,
			   &IPFFCState.DrawFFMesh, &DrawFFAsPolylines,
			   &DrawFFAsPolygons, &LineWidthFlag,
			   &GlblLineWidth, &GlblWidenPolyEnds,
			   &GlblWidenEndLength,
			   &GlblWidenEndWidthScale, &GlblBackGround,
			   &GlblBackGroundColor[0],
			   &GlblBackGroundColor[1],
			   &GlblBackGroundColor[2], &GlblBBoxClip,
			   &GlblBBoxClipDomain[0], &GlblBBoxClipDomain[1],
			   &GlblBBoxClipDomain[2], &GlblBBoxClipDomain[3],
			   &GlblColorPS, &IPFFCState.CubicCrvsAprox,
			   &IPFFCState.Talkative,
			   &HasTime, &CurrentTime,
			   &FontNameFlag, &GlblFontName,
			   &IPFFCState.ShowInternal, &OutFileFlag,
			   &OutFileName, &GlblDepthCue,
			   &GlblDepthCueZ[0], &GlblDepthCueZ[1],
			   &GlblDepthCueGray, &GlblDepthCueZ[0],
			   &GlblDepthCueZ[1], &PointFlag, &DrawPoint,
			   &GlblPointScale, &ForceUnitMat, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2PsExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2PsExit(0);
    }

    if (DrawFFAsPolylines && DrawFFAsPolygons) {
	IRIT_WARNING_MSG("Cannot used both -G and -P.\n");
	Irit2PsExit(1);
    }
    if (DrawFFAsPolylines) {
	IPFFCState.DrawFFGeom = TRUE;
	IPFFCState.DumpObjsAsPolylines = TRUE;
    }
    else if (DrawFFAsPolygons) {
	IPFFCState.DrawFFGeom = TRUE;
	IPFFCState.DumpObjsAsPolylines = FALSE;
    }
    else {
	IPFFCState.DrawFFGeom = FALSE;
    }

    IPFFCState.ComputeNrml = FALSE;      /* No need for normals of vertices. */

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2PsExit(1);
    }
    GlblPSTitle = FileNames[0];

    if (GlblDepthCueGray) {
	if (GlblDepthCue) {
	    IRIT_WARNING_MSG("Only one of '-d' or '-D' at once please, exit.\n");
	    GAPrintHowTo(CtrlStr);
	    Irit2PsExit(1);
	}
	GlblDepthCue = TRUE;
    }
    if (GlblWidenPolyEnds && !GlblDepthCue) {
	IRIT_WARNING_MSG("If '-w' is set, either '-d' or -'D' must be set as well.\n");
	Irit2PsExit(1);
    }

    if (PointFlag) {
	switch (DrawPoint[0]) {
	    case 'h':
	    case 'H':
	        GlblDrawPoint = DRAW_POINT_HOLLOW_CIRC;
		break;
	    case 'f':
	    case 'F':
	        GlblDrawPoint = DRAW_POINT_FULL_CIRC;
		break;
	    case 'c':
	    case 'C':
	        GlblDrawPoint = DRAW_POINT_CROSS;
		break;
	    default:
		IRIT_WARNING_MSG("Point type drawing supported: H)ollowed circle, F)ull circle, C)ross.\n");
		GAPrintHowTo(CtrlStr);
		Irit2PsExit(1);
	}
    }

    if (GlblPointScale < 0.0) {
	GlblPointScale = -GlblPointScale;
    }

    if (NumOfIsolinesFlag && StrNumOfIsolines != NULL) {
	if (sscanf(StrNumOfIsolines, "%d:%d:%d",
		   &IPFFCState.NumOfIsolines[0],
		   &IPFFCState.NumOfIsolines[1],
		   &IPFFCState.NumOfIsolines[2]) != 3) {
	    if (sscanf(StrNumOfIsolines, "%d:%d",
		       &IPFFCState.NumOfIsolines[0],
		       &IPFFCState.NumOfIsolines[1]) != 2) {
		if (sscanf(StrNumOfIsolines, "%d",
			   &IPFFCState.NumOfIsolines[1]) != 1) {
		    IRIT_WARNING_MSG(
			    "Number(s) of isolines (-I) cannot be parsed.\n");
		    GAPrintHowTo(CtrlStr);
		    Irit2PsExit(1);
		}
		else {
		    IPFFCState.NumOfIsolines[2] =
			IPFFCState.NumOfIsolines[1] =
			    IPFFCState.NumOfIsolines[0];
		}
	    }
	    else {
		IPFFCState.NumOfIsolines[2] = IPFFCState.NumOfIsolines[0];
	    }

	}
    }

    IPFFCState.BBoxGrid = TRUE;

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2PsExit(1);
    PObjects = IPResolveInstances(PObjects);

    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext)
	IPPropagateObjectName(PObj, NULL);

    if (HasTime)
	GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    if (ForceUnitMat) {
	MatGenUnitMat(GlblCrntViewMat);
    }
    else {
	if (IPWasPrspMat)
	    MatMultTwo4by4(GlblCrntViewMat, IPViewMat, IPPrspMat);
	else
	    IRIT_GEN_COPY(GlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    }

    IPTraverseObjListHierarchy(PObjects, GlblCrntViewMat, IPMapObjectInPlace);

    if (!IPFFCState.DrawFFMesh)
	CagdTightBBox(TRUE);
    PGlblBbox = GMBBComputeBboxObjectList(PObjects);
    IRIT_GEN_COPY(&GlblBbox, PGlblBbox, sizeof(GMBBBboxStruct));

    if (GlblDepthCue) {
	if (GlblDepthCueZ[0] == IRIT_INFNTY ||
	    GlblDepthCueZ[1] == -IRIT_INFNTY ||
	    IRIT_APX_EQ(GlblDepthCueZ[0], GlblDepthCueZ[1])) {

	    GlblDepthCueZ[0] = GlblBbox.Min[2];
	    GlblDepthCueZ[1] = GlblBbox.Max[2];
	}
	IRIT_INFO_MSG_PRINTF("Z Depth Cue Bounding Box is [%f...%f]\n",
		             GlblDepthCueZ[0], GlblDepthCueZ[1]);
    }

    DumpDataForPostScript(OutFileFlag ? OutFileName : NULL,
			  PObjects, GlblCrntViewMat);

    Irit2PsExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts RGB to a gray level value.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   R, G, B:  Color coefficients to convert to a gray level.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Gray level, normalized to be between zero and one.           *
*****************************************************************************/
static IrtRType RGBtoGray(int R, int G, int B)
{
    return (R * 0.3 + G * 0.59 + B * 0.11) / 255.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for Postscript into FileName (stdout in NULL).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:     Where output should go to.                                 *
*   PObjects:     To dump into file.                                         *
*   CrntViewMat:  Viewing matrix.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForPostScript(const char *FileName,
				  IPObjectStruct *PObjects,
				  IrtHmgnMatType CrntViewMat)
{
    IRIT_STATIC_DATA char *PSHeader[] = {
	"%!PS-Adobe-2.0 EPSF-1.2",
	"BBOX",
	"TITLE",
	"%%Creator: irit2ps",
	"DATE",
	"%%EndComments",
	"",
	"% This file was automatically created from IRIT solid modeller data",
	"% using Irit2ps - IRIT to PostScript filter.",
	"",
	"gsave",
	"",
	"72 72 scale",	/* First scale to inches, se we can speak in inches. */
	"TRANSFORM",
	"",
	"/pl {",
	"    newpath",
	"    moveto",
        "    {counttomark 0 ne {lineto} {exit} ifelse} loop",
	"    pop % the mark",
	"    stroke",
	"} def",
	"",
	"/pf {",
	"    newpath",
	"    moveto",
        "    {counttomark 0 ne {lineto} {exit} ifelse} loop",
	"    pop % the mark",
	"    fill",
	"} def",
	"",
	"/cl {",
	"    newpath",
	"    moveto",
        "    curveto",
	"    stroke",
	"} def",
	"",
	"/cf {",
	"    newpath",
	"    moveto",
        "    curveto",
	"    fill",
	"} def",
	"",
	"/ln { % Line",
	"    newpath",
	"    moveto",
	"    lineto",
	"    stroke",
	"} def",
	"",
	"/lw { % Line with Width control",
	"    newpath",
	"    setlinewidth",
	"    moveto",
	"    lineto",
	"    stroke",
	"} def",
	"",
	"/ldg { % Line with Depth control - Gray",
	"    newpath",
	"    setgray",
	"    moveto",
	"    lineto",
	"    stroke",
	"} def",
	"",
	"/ldc { % Line with Depth control - Color",
	"    newpath",
	"    setrgbcolor",
	"    moveto",
	"    lineto",
	"    stroke",
	"} def",
	"",
	"/cw { % Curve with Width control",
	"    newpath",
	"    setlinewidth",
	"    moveto",
	"    curveto",
	"    stroke",
	"} def",
	"",
	"/cdg { % Curve with Width control - Gray",
	"    newpath",
	"    setgray",
	"    moveto",
	"    curveto",
	"    stroke",
	"} def",
	"",
	"/cdc { % Curve with Width control - Color",
	"    newpath",
	"    setrgbcolor",
	"    moveto",
	"    curveto",
	"    stroke",
	"} def",
	"",
	"1 setlinecap",
	"1 setlinejoin",
	"WIDTH",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *PSTrailer[] = {
	"",
	"showpage",
	"grestore",
	NULL
    };
    IrtRType
	PrintWidth = GlblPrintSize,
	PrintHeight = GlblPrintSize;
    int i, IPrintLeft, IPrintRight, IPrintBot, IPrintTop,
	IPrintCntrX = (int) (72.0 * 4.0),
	IPrintCntrY = (int) (72.0 * (1.0 + PrintHeight / 2.0)),
	IPrintWidth2 = (int) (72.0 * PrintWidth / 2),
	IPrintHeight2 = (int) (72.0 * PrintHeight / 2);

    if (GlblBBoxClip) {
        IPrintLeft = IPrintCntrX + ((int) (IPrintWidth2 * GlblBBoxClipDomain[0]));
	IPrintRight = IPrintCntrX + ((int) (IPrintWidth2 * GlblBBoxClipDomain[2]));
	IPrintBot = IPrintCntrY + ((int) (IPrintHeight2 * GlblBBoxClipDomain[1]));
	IPrintTop = IPrintCntrY + ((int) (IPrintHeight2 * GlblBBoxClipDomain[3]));
    }
    else {
        IPrintLeft = IPrintCntrX + ((int) (IPrintWidth2 * GlblBbox.Min[0]));
	IPrintRight = IPrintCntrX + ((int) (IPrintWidth2 * GlblBbox.Max[0]));
	IPrintBot = IPrintCntrY + ((int) (IPrintHeight2 * GlblBbox.Min[1]));
	IPrintTop = IPrintCntrY + ((int) (IPrintHeight2 * GlblBbox.Max[1]));
    }

    if (FileName != NULL) {
	if ((OutputFile = fopen(FileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", FileName);
	    Irit2PsExit(2);
	}
    }
    else
	OutputFile = stdout;

    for (i = 0; PSHeader[i] != NULL; i++) {
	if (strcmp(PSHeader[i], "BBOX") == 0) {
	    fprintf(OutputFile, "%%%%BoundingBox: %d %d %d %d\n",
		    IPrintLeft - BBOX_BNDRY_OFFSET,
		    IPrintBot - BBOX_BNDRY_OFFSET,
		    IPrintRight + BBOX_BNDRY_OFFSET,
		    IPrintTop + BBOX_BNDRY_OFFSET);
	}
	else if (strcmp(PSHeader[i], "DATE") == 0) {
	    fprintf(OutputFile, "%%%%CreationDate: %s\n", IritRealTimeDate());
	}
	else if (strcmp(PSHeader[i], "TITLE") == 0)
	    fprintf(OutputFile, "%%%%Title: %s\n", GlblPSTitle);
	else if (strcmp(PSHeader[i], "TRANSFORM") == 0) {
	    fprintf(OutputFile, "4.0 %f translate\t\t%% center image\n",
		    1.0 + PrintHeight / 2.0);
	    fprintf(OutputFile, "%f %f scale\t\t%% maps -1..1 to this domain\n",
		    PrintWidth / 2.0, PrintHeight / 2.0);
	}
	else if (strcmp(PSHeader[i], "WIDTH") == 0)
	    fprintf(OutputFile, "%f setlinewidth\n",
		    GlblLineWidth > 0.0 ? GlblLineWidth
					: -GlblLineWidth * DEFAULT_LINE_WIDTH);
	else
	    fprintf(OutputFile, "%s\n", PSHeader[i]);
    }

    if (GlblBackGround) {
	if (GlblColorPS)
	    fprintf(OutputFile, "newpath\n%f %f %f setrgbcolor\n",
		    GlblBackGroundColor[0] / 255.0,
		    GlblBackGroundColor[1] / 255.0,
		    GlblBackGroundColor[2] / 255.0);
	else
	    fprintf(OutputFile, "newpath\n%f setgray\n",
		    RGBtoGray(GlblBackGroundColor[0],
			      GlblBackGroundColor[1],
			      GlblBackGroundColor[2]));
	fprintf(OutputFile,
		"-2 -2 moveto\n 2 -2 lineto\n 2  2 lineto\n-2  2 lineto\n-2 -2 lineto\nfill\n");
    }

    if (GlblBBoxClip) {
	fprintf(OutputFile,
		"newpath\n%f %f moveto\n%f %f lineto\n%f %f lineto\n%f %f lineto\n%f %f lineto\nclip\n",
		GlblBBoxClipDomain[0], GlblBBoxClipDomain[1],
		GlblBBoxClipDomain[2], GlblBBoxClipDomain[1],
		GlblBBoxClipDomain[2], GlblBBoxClipDomain[3],
		GlblBBoxClipDomain[0], GlblBBoxClipDomain[3],
		GlblBBoxClipDomain[0], GlblBBoxClipDomain[1]);
    }

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, DumpOneTraversedObject);

    for (i = 0; PSTrailer[i] != NULL; i++)
	fprintf(OutputFile, "%s\n", PSTrailer[i]);

    fclose(OutputFile);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IPObjectStruct *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj)) {
        int Override, OldDumpObjsAsPolylines;

        /* Objects with "fill" attribute should always be tesselated into   */
        /* polygons, so override the default whatever it is.		    */
        if ((Override = (AttrFindAttribute(PObj -> Attr, "fill") != NULL))
	                                                          != FALSE) {
	    OldDumpObjsAsPolylines = IPFFCState.DumpObjsAsPolylines;
	    IPFFCState.DumpObjsAsPolylines = FALSE;
	}

        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */

	if (Override) {
	    IPFFCState.DumpObjsAsPolylines = OldDumpObjsAsPolylines;
	}
    }
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
	DumpOneObject(OutputFile, PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps attribute's state.	                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   f:          File to dumps to.                                            *
*   Width:      Width PS attribute.                                          *
*   Gray:       Gray level PS attribute.                                     *
*   RGBColor:   RGB color PS attribute.                                      *
*   Dash:       Dash/dotted lines PS attribute.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpAttrState(FILE *f,
			  IrtRType Width,
			  IrtRType Gray,
			  IrtRType *RGBColor,
			  const char *Dash)
{
    fprintf(f, "%f setlinewidth\n", Width);

    if (Dash != NULL)
	fprintf(f, "%s setdash\n", Dash);
    else
	fprintf(f, "[] 0 setdash\n");

    if (GlblColorPS)
	fprintf(f, "%f %f %f setrgbcolor\n",
		RGBColor[0] / 255.0, RGBColor[1] / 255.0, RGBColor[2] / 255.0);
    else
	fprintf(f, "%f setgray\n", Gray);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to file f.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneObject(FILE *f, IPObjectStruct *PObject)
{
    IRIT_STATIC_DATA IrtRType
	RGBWhite[3] = {255.0, 255.0, 255.0};
    int i, Red, Green, Blue, IgnoreDepthCue,
	HasColor = FALSE;
    const char *Dash;
    IrtRType *R, RGBColor[3], Fill, Gray, OrigWidth, Width, TubeRatio;
    IrtPtType Pt;
    IPPolygonStruct *PList;
    CagdCrvStruct *CList;

    if (GlblBBoxClip) {
        GMBBBboxStruct
	    *BBox = GMBBComputeBboxObject(PObject);

	if (BBox -> Max[0] < GlblBBoxClipDomain[0] ||
	    BBox -> Max[1] < GlblBBoxClipDomain[1] ||
	    BBox -> Min[0] > GlblBBoxClipDomain[2] ||
	    BBox -> Min[1] > GlblBBoxClipDomain[3]) {
	    return;  /* Outside the clipping zone. */
	}        
    }

    OrigWidth = Width = AttrGetObjectRealAttrib(PObject, "width");
    if (IP_ATTR_IS_BAD_REAL(Width)) {
        if (GlblLineWidth > 0.0)
	    Width = GlblLineWidth;
	else
	    Width = -GlblLineWidth * DEFAULT_LINE_WIDTH;
    }
    else {
	if (GlblLineWidth < 0.0)
	    Width *= -GlblLineWidth;
    }

    IgnoreDepthCue = AttrGetObjectIntAttrib(PObject, "DepthCue") == FALSE;

    Dash = AttrGetObjectStrAttrib(PObject, "dash");

    Fill = AttrGetObjectRealAttrib(PObject, "fill");
    if (IP_ATTR_IS_BAD_REAL(Fill))
	Fill = -1.0;

    Gray = AttrGetObjectRealAttrib(PObject, "gray");
    if (IP_ATTR_IS_BAD_REAL(Gray))
	Gray = 0.0;

    TubeRatio = AttrGetObjectRealAttrib(PObject, "tubular");
    if (IP_ATTR_IS_BAD_REAL(TubeRatio))
	TubeRatio = 0.0;

    /* Do not attempt to make tubes on such skiny lines. */
    if (Width < MIN_PS_LINE_WIDTH)
	TubeRatio = 0.0;

    if (GlblColorPS) {
	if (AttrGetObjectRGBColor2(PObject, NULL, &Red, &Green, &Blue)) {
	    HasColor = TRUE;
	    RGBColor[0] = Red;
	    RGBColor[1] = Green;
	    RGBColor[2] = Blue;
	}

	if (!HasColor)
	    for (i = 0; i < 3; i++)
		RGBColor[i] = 0.0;
    }

    fprintf(f, "\n%%\n%% Object: %s\n%%\n",
	    IP_VALID_OBJ_NAME(PObject) ? IP_GET_OBJ_NAME(PObject) : "Noname");
    DumpAttrState(f, Width, Gray, RGBColor, Dash);

    switch (PObject -> ObjType) {
	case IP_OBJ_POLY:
	    PList = PObject -> U.Pl;

	    while (PList) {
		if (IP_IS_POINTLIST_OBJ(PObject)) {
		    IPVertexStruct *V;

		    for (V = PList -> PVertex; V != NULL; V = V -> Pnext) {
			DumpOnePoint(f, V -> Coord, FALSE, RGBColor,
				     Gray, OrigWidth);
			if (V -> Pnext != NULL)
			    DumpAttrState(f, Width, Gray, RGBColor, Dash);
		    }
		}
		else {
		    if (TubeRatio > 0.0)
			DumpAttrState(f, Width, Gray, RGBColor, Dash);

		    DumpOnePoly(f, PList, IP_IS_POLYGON_OBJ(PObject),
				Fill >= 0.0, IgnoreDepthCue, Gray,
				Width, RGBColor);

		    if (TubeRatio > 0.0) {
			DumpAttrState(f, Width * TubeRatio, 1.0,
				      RGBWhite, Dash);

			DumpOnePoly(f, PList, IP_IS_POLYGON_OBJ(PObject),
				    Fill >= 0.0, IgnoreDepthCue, Gray,
				    Width * TubeRatio, RGBColor);
		    }
		}

		PList = PList -> Pnext;
	    }
	    break;
	case IP_OBJ_CURVE:
	    if (IPFFCState.CubicCrvsAprox) {
		if (IPFFCState.DrawFFGeom) {
		    CList = PObject -> U.Crvs;

		    while (CList) {
			if (TubeRatio > 0.0)
			    DumpAttrState(f, Width, Gray, RGBColor, Dash);

			DumpOneCubicCurve(f, CList, Fill >= 0.0, Gray, Width,
					  RGBColor);

			if (TubeRatio > 0.0) {
			    DumpAttrState(f, Width * TubeRatio, 1.0,
					  RGBWhite, Dash);

			    DumpOneCubicCurve(f, CList, Fill >= 0.0, Gray,
					      Width * TubeRatio, RGBColor);
			}
			CList = CList -> Pnext;
		    }
	
		}
		break;
	    }
	    break;
	case IP_OBJ_VECTOR:
	    DumpOnePoint(f, PObject -> U.Vec, TRUE, RGBColor, Gray, OrigWidth);
	    break;
	case IP_OBJ_CTLPT:
	    R = PObject -> U.CtlPt.Coords;
	    CagdCoercePointTo(Pt, CAGD_PT_E3_TYPE,
			      &R, -1, PObject -> U.CtlPt.PtType);
	    DumpOnePoint(f, Pt, FALSE, RGBColor, Gray, OrigWidth);
	    break;
	case IP_OBJ_POINT:
	    DumpOnePoint(f, PObject -> U.Vec, FALSE, RGBColor,
			 Gray, OrigWidth);
	    break;
	case IP_OBJ_STRING:
	    DumpOneString(f, PObject, RGBColor, Gray);
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon/line.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   f:              File to dump object to.                                  *
*   PPolygon:       Poly to dump to file f.                                  *
*   IsPolygon:      TRUE for polygon, FALSE otherwise.                       *
*   Fill:           Fill PS attribute.                                       *
*   IgnoreDepthCue: If TRUE, no depth cue for this one.                      *
*   Gray:           Gray level PS attribute.                                 *
*   LineWidth:      Line width PS attribute.                                 *
*   RGBColor:       RGB color PS attribute.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOnePoly(FILE *f,
			IPPolygonStruct *PPolygon,
			int IsPolygon,
			int Fill,
			int IgnoreDepthCue,
			IrtRType Gray,
			IrtRType LineWidth,
			IrtRType *RGBColor)
{
    int Length, WidenEnd;
    IPVertexStruct
	*VList = PPolygon -> PVertex;

    if (GlblBBoxClip) {
        GMBBBboxStruct
	    *BBox = GMBBComputeOnePolyBbox(PPolygon);

	if (BBox -> Max[0] < GlblBBoxClipDomain[0] ||
	    BBox -> Max[1] < GlblBBoxClipDomain[1] ||
	    BBox -> Min[0] > GlblBBoxClipDomain[2] ||
	    BBox -> Min[1] > GlblBBoxClipDomain[3]) {
	    return;  /* Outside the clipping zone. */
	}        
    }

    if (!IsPolygon && GlblWidenPolyEnds) {
	WidenEnd = AttrGetIntAttrib(PPolygon -> Attr, "widenend");
	if (!IP_ATTR_IS_BAD_INT(WidenEnd)) {
	    IrtRType
		PolyLength = PolylineLength2D(PPolygon -> PVertex),
		WidenEndLen = GlblWidenEndLength,
		EndScale = GlblWidenEndWidthScale;

	    if (WidenEndLen > PolyLength / 2.0) {
		EndScale *= (PolyLength / 2.0) / WidenEndLen;
		if (EndScale < 1.0)
		    EndScale = 1.0;
		WidenEndLen = PolyLength / 2.0;
	    }

	    if (WidenEnd & WIDEN_END_START) {
		IrtRType
		    Scale2 = EndScale,
		    WidenEndLenLeft = WidenEndLen;
		IPVertexStruct
		    *V = VList;

		while (WidenEndLenLeft > 0.0) {
		    IrtRType
			EdgeLength = EdgeLength2D(V),
			ProcessedLength = IRIT_MIN(EdgeLength, WidenEndLenLeft),
			Scale1 = Scale2;

		    Scale2 = Scale2 * (WidenEndLenLeft - ProcessedLength) /
							    WidenEndLenLeft;
		    if (Scale2 < 1.0)
			Scale2 = 1.0;

		    SubdivAndWidenEdge(V, Scale1, Scale2, LineWidth,
				       WidenEndLenLeft > EdgeLength ?
				            1.0 :
				            WidenEndLenLeft / EdgeLength);
		    WidenEndLenLeft -= ProcessedLength;

		    V = V -> Pnext;
		}
	    }
	    if (WidenEnd & WIDEN_END_END) {
		IrtRType
		    Scale2 = EndScale,
		    WidenEndLenLeft = WidenEndLen;
		IPVertexStruct
		    *V = IPGetPrevVrtx(VList, IPGetLastVrtx(VList));

		while (WidenEndLenLeft > 0.0) {
		    IrtRType
			EdgeLength = EdgeLength2D(V),
			ProcessedLength = IRIT_MIN(EdgeLength, WidenEndLenLeft),
			Scale1 = Scale2;

		    Scale2 = Scale2 * (WidenEndLenLeft - ProcessedLength) /
							    WidenEndLenLeft;
		    if (Scale2 < 1.0)
			Scale2 = 1.0;

		    SubdivAndWidenEdge(V, Scale2, Scale1, LineWidth,
				       WidenEndLenLeft > EdgeLength ?
				            1.0 :
				            WidenEndLenLeft / EdgeLength);
		    WidenEndLenLeft -= ProcessedLength;

		    V = IPGetPrevVrtx(VList, V);
		}
	    }
	}
	else
	    WidenEnd = 0;
    }

    if (GlblDepthCue && !IgnoreDepthCue && !Fill) {
	if (VList != NULL && VList -> Pnext != NULL) {
	    IrtRType Ztemp, Scale;

	    do {
		if (IPFFCState.ShowInternal || !IP_IS_INTERNAL_VRTX(VList)) {
		    Scale = AttrGetRealAttrib(VList -> Attr, "widthscale");
		    if (IP_ATTR_IS_BAD_REAL(Scale))
			Scale = 1.0;
		    /* Disable rounding of ends. */
		    if (Scale != 1.0)
			fprintf(f, "0 setlinecap\n");

		    fprintf(f, "%6.4f %6.4f",
			    VList -> Coord[0], VList -> Coord[1]);

		    Ztemp = (VList -> Pnext -> Coord[2] - GlblDepthCueZ[0]) /
			    (GlblDepthCueZ[1] - GlblDepthCueZ[0]);
		    Ztemp = Scale * IRIT_MAX(Ztemp, IRIT_EPS);
		    if (GlblDepthCueGray) {
			if (GlblColorPS)
			    fprintf(f, " %6.4f %6.4f %6.4f %6.4f %6.4f ldc\n",
				    VList -> Pnext -> Coord[0],
				    VList -> Pnext -> Coord[1],
				    RGBColor[0] * Ztemp / 255.0 + 1.0 - Ztemp,
				    RGBColor[1] * Ztemp / 255.0 + 1.0 - Ztemp,
				    RGBColor[2] * Ztemp / 255.0 + 1.0 - Ztemp);
			else
			    fprintf(f, " %6.4f %6.4f %6.4f ldg\n",
				    VList -> Pnext -> Coord[0],
				    VList -> Pnext -> Coord[1],
				    (1.0 - Gray) * (1.0 - Ztemp) );
		    }
		    else
			fprintf(f, " %6.4f %6.4f %6.4f lw\n",
				VList -> Pnext -> Coord[0],
				VList -> Pnext -> Coord[1],
				LineWidth * Ztemp);

		    /* Enable rounding of ends. */
		    if (Scale != 1.0)
			fprintf(f, "1 setlinecap\n");
		}

		VList = VList -> Pnext;
	    }
	    while (VList -> Pnext != NULL &&
		   VList != PPolygon -> PVertex);

	    if (IsPolygon) {
		Scale = AttrGetRealAttrib(PPolygon -> PVertex -> Attr,
					  "widthscale");
		if (IP_ATTR_IS_BAD_REAL(Scale))
			Scale = 1.0;

		fprintf(f, "%6.4f %6.4f",
			VList -> Coord[0], VList -> Coord[1]);

		Ztemp = (PPolygon -> PVertex -> Coord[2] - GlblDepthCueZ[0]) /
			(GlblDepthCueZ[1] - GlblDepthCueZ[0]);
		Ztemp = Scale * IRIT_MAX(Ztemp, IRIT_EPS);
		if (GlblDepthCueGray) {
		    if (GlblColorPS)
			fprintf(f, " %6.4f %6.4f %6.4f %6.4f %6.4f ldc\n",
				PPolygon -> PVertex -> Coord[0],
				PPolygon -> PVertex -> Coord[1],
				RGBColor[0] * Ztemp / 255.0 + 1.0 - Ztemp,
				RGBColor[1] * Ztemp / 255.0 + 1.0 - Ztemp,
				RGBColor[2] * Ztemp / 255.0 + 1.0 - Ztemp);
		    else
			fprintf(f, " %6.4f %6.4f %6.4f ldg\n",
				PPolygon -> PVertex -> Coord[0],
				PPolygon -> PVertex -> Coord[1],
				(1.0 - Gray) * (1.0 - Ztemp));
		}
		else
		    fprintf(f, " %6.4f %6.4f %6.4f lw\n",
			    PPolygon -> PVertex -> Coord[0],
			    PPolygon -> PVertex -> Coord[1],
			    LineWidth * Ztemp);
	    }
	}
    }
    else {
	Length = 0;
	do {
	    if (!Fill &&
		((!IPFFCState.ShowInternal && IP_IS_INTERNAL_VRTX(VList)) ||
		 Length > MAX_NUM_OF_PTS)) {
		if (Length > 0) {
		    fprintf(f, "%6.4f %6.4f\n",
			    VList -> Coord[0], VList -> Coord[1]);
		    fprintf(f, Fill ? "pf\n" : "pl\n");
		    Length = 0;
		}
		if (!IPFFCState.ShowInternal && IP_IS_INTERNAL_VRTX(VList))
		    VList = VList -> Pnext;
	    }
	    else {
		if (Length == 0)
		    fprintf(f, "mark\n");
		fprintf(f, "%6.4f %6.4f\n",
			VList -> Coord[0], VList -> Coord[1]);
		Length++;
		VList = VList -> Pnext;
	    }
	}
	while (VList != NULL && VList != PPolygon -> PVertex);

	if (IsPolygon && Length > 0) {
	    fprintf(f, "%6.4f %6.4f\n",
		    PPolygon -> PVertex -> Coord[0],
		    PPolygon -> PVertex -> Coord[1]);
	}
	if (Length > 0)
	    fprintf(f, Fill ? "pf\n" : "pl\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the length of a polyline in the XY plane.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:        First vertex in polyline.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Length of polyline.                                          *
*****************************************************************************/
static IrtRType PolylineLength2D(IPVertexStruct *V)
{
    IrtRType Len = 0.0;

    for ( ; V -> Pnext != NULL; V = V -> Pnext)
	Len += EdgeLength2D(V);

    return Len;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the length of an edge in the XY plane.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         First vertex of edge.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    Distance from V to V -> Pnext.                              *
*****************************************************************************/
static IrtRType EdgeLength2D(IPVertexStruct *V)
{
    return sqrt(IRIT_SQR(V -> Pnext -> Coord[0] - V -> Coord[0]) +
		IRIT_SQR(V -> Pnext -> Coord[1] - V -> Coord[1]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Subdivides the EdgePortion from V to V -> Pnext into small segments and    *
* sets their width attribute to be from Scale1 to Scale2 of LineWidth.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:               First vertex of edge.                                   *
*   Scale1, Scale2:  Scaling of width from V to V -> Pnext.                  *
*   LineWidth:       Line width expected.                                    *
*   EdgePortion:     1.0 for all the edge, <1.0 for less than full edge.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SubdivAndWidenEdge(IPVertexStruct *V,
			       IrtRType Scale1,
			       IrtRType Scale2,
			       IrtRType LineWidth,
			       IrtRType EdgePortion)
{
    IPVertexStruct *Vtmp;
    IrtRType Len,
	SubdivFactor = GlblWidenEndLength * WIDEN_LN_SUBDIV_FACTOR;

    AttrSetRealAttrib(&V -> Attr, "widthscale", Scale1);

    if (EdgePortion < 1.0) {
	/* Clip edge to make a full edge that needs to be subdivided. */
	Vtmp = IPAllocVertex2(V -> Pnext);

	if (Scale1 > Scale2) {
	    /* It is the beginning of the edge. */
	    IRIT_PT_BLEND(Vtmp -> Coord, V -> Pnext -> Coord, V -> Coord,
		     EdgePortion);
	    V -> Pnext = Vtmp;
	}
	else {
	    /* It is the end of the edge. */
	    IRIT_PT_BLEND(Vtmp -> Coord, V -> Coord, V -> Pnext -> Coord,
		     EdgePortion);
	    V -> Pnext = Vtmp;
	    V = Vtmp;
	}
    }

    while ((Len = EdgeLength2D(V)) > SubdivFactor) {
	EdgePortion = SubdivFactor / Len;
	Vtmp = IPAllocVertex2(V -> Pnext);
	IRIT_PT_BLEND(Vtmp -> Coord, V -> Pnext -> Coord, V -> Coord, EdgePortion);
	V -> Pnext = Vtmp;
 	V = Vtmp;
	Scale1 = Scale1 * (1.0 - EdgePortion) + Scale2 * EdgePortion;
	AttrSetRealAttrib(&Vtmp -> Attr, "widthscale", Scale1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one curve.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   Crv:	  Curve to dump to file f.                                   *
*   Fill:         Fill PS attribute.                                         *
*   Gray:         Gray level PS attribute.                                   *
*   LineWidth:    Line width PS attribute.                                   *
*   RGBColor:     RGB color PS attribute.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneCubicCurve(FILE *f,
			      CagdCrvStruct *Crv,
			      int Fill,
			      IrtRType Gray,
			      IrtRType LineWidth,
			      IrtRType *RGBColor)
{
    CagdBType
	NewCrv = FALSE;
    int j;
    CagdRType **Points;

    if (GlblBBoxClip) {
        CagdBBoxStruct BBox;

	CagdCrvBBox(Crv, &BBox);

	if (BBox.Max[0] < GlblBBoxClipDomain[0] ||
	    BBox.Max[1] < GlblBBoxClipDomain[1] ||
	    BBox.Min[0] > GlblBBoxClipDomain[2] ||
	    BBox.Min[1] > GlblBBoxClipDomain[3]) {
	    return;  /* Outside the clipping zone. */
	}        
    }

    if (Crv ->  PType != CAGD_PT_E3_TYPE) {
	Crv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);
	NewCrv = TRUE;
    }
    Points = Crv -> Points;

    if (GlblDepthCue && !Fill) {
	IrtRType Z, Ztemp;

	for (j = 1; j < 4; j++)
	    fprintf(f, "%6.4f %6.4f ", Points[1][j], Points[2][j]);
	fprintf(f, "%6.4f %6.4f ", Points[1][0], Points[2][0]);
	Z = Points[2][0];

	Ztemp = (Z - GlblDepthCueZ[0]) /
		(GlblDepthCueZ[1] - GlblDepthCueZ[0]);
	Ztemp = IRIT_MAX((1.0 - Ztemp), IRIT_EPS);

	if (GlblDepthCueGray) {
	    if (GlblColorPS)
		fprintf(f, "%6.4f %6.4f %6.4f cdc\n",
			RGBColor[0] * Ztemp / 255.0 + 1.0 - Ztemp,
			RGBColor[1] * Ztemp / 255.0 + 1.0 - Ztemp,
			RGBColor[2] * Ztemp / 255.0 + 1.0 - Ztemp);
	    else
		fprintf(f, "%6.4f cdg\n", (1.0 - Gray) * (1.0 - Ztemp));
	}
	else
	    fprintf(f, "%6.4f cw\n", LineWidth * Ztemp);
    }
    else {
	for (j = 1; j < 4; j++)
	    fprintf(f, "%6.4f %6.4f ", Points[1][j], Points[2][j]);
	fprintf(f, "%6.4f %6.4f ", Points[1][0], Points[2][0]);
	fprintf(f, Fill ? "cf\n" : "cl\n");
    }

    if (NewCrv)
	CagdCrvFree(Crv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one point.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   Pt:	  	  Point to dump to file f.                                   *
*   IsVector:	  If TRUE, dumps a line to origin.                           *
*   RGBColor:     RGB color PS attribute.                                    *
*   Gray:         Gray level PS attribute.                                   *
*   Width:        Width attribute found on object, if any.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOnePoint(FILE *f,
			 IrtRType *Pt,
			 int IsVector,
			 IrtRType *RGBColor,
			 IrtRType Gray,
			 IrtRType Width)
{
    IrtRType
	Z = Pt[2],
	Ztemp = (Z - GlblDepthCueZ[0]) / (GlblDepthCueZ[1] - GlblDepthCueZ[0]),
	Size = GlblDepthCue ? GlblPointScale * IRIT_MAX(Ztemp, IRIT_EPS)
			    : GlblPointScale;

    /* If we are width attribute, use it as raadius. */
    if (!IP_ATTR_IS_BAD_REAL(Width))
	Size = Width;

    if (GlblDepthCueGray) {
	if (GlblColorPS)
	    fprintf(f, "%f %f %f setrgbcolor\n",
		    RGBColor[0] * Ztemp / 255.0 + 1.0 - Ztemp,
		    RGBColor[1] * Ztemp / 255.0 + 1.0 - Ztemp,
		    RGBColor[2] * Ztemp / 255.0 + 1.0 - Ztemp);
	else
	    fprintf(f, "%f setgray\n", (1.0 - Gray) * (1.0 - Ztemp));
    }

    if (IsVector) {
	fprintf(f, "0 0 %6.4f %6.4f 0 lw\n", Pt[0], Pt[1]);
    }

    switch (GlblDrawPoint) {
	case DRAW_POINT_FULL_CIRC:
	    /* Dump a point as a full circle. */
	    fprintf(f, "newpath %6.4f %6.4f %6.4f 0 360 arc fill\n",
		    Pt[0], Pt[1], GlblDepthCueGray ? GlblPointScale : Size); 
	    break;
	case DRAW_POINT_HOLLOW_CIRC:
	    /* Dump a point as a hollowed circle. */
	    fprintf(f, "newpath %6.4f %6.4f %6.4f 0 360 arc fill\n",
		    Pt[0], Pt[1], GlblDepthCueGray ? GlblPointScale : Size); 
	    if (GlblColorPS)
		fprintf(f, "1.0 1.0 1.0 setrgbcolor\n");
	    else
		fprintf(f, "1.0 setgray\n");
	    fprintf(f, "newpath %6.4f %6.4f %6.4f 0 360 arc fill\n",
		    Pt[0], Pt[1], GlblDepthCueGray ? GlblPointScale * 0.7
						   : Size * 0.7); 
	    break;
	case DRAW_POINT_CROSS:
	    /* Dump a point as a cross. */
	    fprintf(f, "%6.4f %6.4f %6.4f %6.4f  %6.4f lw\n",
		    Pt[0] - Size, Pt[1],
		    Pt[0] + Size, Pt[1],
		    GlblDepthCueGray ? GlblPointScale * 0.2 : Size * 0.2);
	    fprintf(f, "%6.4f %6.4f %6.4f %6.4f  %6.4f lw\n",
		    Pt[0], Pt[1] - Size,
		    Pt[0], Pt[1] + Size,
		    GlblDepthCueGray ? GlblPointScale * 0.2 : Size * 0.2);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one string.                                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PObj:         String to dump to file f.                                  *
*   RGBColor:     RGB color PS attribute.                                    *
*   Gray:         Gray level PS attribute.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneString(FILE *f,
			  IPObjectStruct *PObj,
			  IrtRType *RGBColor,
			  IrtRType Gray)
{
    IRIT_STATIC_DATA IrtRType
	LastScale = 0.0;
    IRIT_STATIC_DATA IrtVecType
	Origin = { 0.0, 0.0, 0.0 };
    IrtRType
        Scale = AttrGetObjectRealAttrib(PObj, "StrScale");
    IrtVecType *Pos3D, Pos;
    IPObjectStruct
        *Pos3DObj = AttrGetObjectObjAttrib(PObj, "StrPos");

    if (Pos3DObj == NULL || !IP_IS_VEC_OBJ(Pos3DObj))
        Pos3D = &Origin;
    else
        Pos3D = &Pos3DObj -> U.Vec;
    MatMultPtby4by4(Pos, *Pos3D, GlblCrntViewMat);

    if (Scale == IP_ATTR_BAD_REAL)
        Scale = DEFAULT_FONT_SCALE;
    Scale *= MatScaleFactorMatrix(GlblCrntViewMat);

    if (GlblDepthCueGray) {
        IrtRType
	    Z = (*Pos3D)[2],
	    Ztemp = (Z - GlblDepthCueZ[0]) /
					 (GlblDepthCueZ[1] - GlblDepthCueZ[0]);

	if (GlblColorPS)
	    fprintf(f, "%f %f %f setrgbcolor\n",
		    RGBColor[0] * Ztemp / 255.0 + 1.0 - Ztemp,
		    RGBColor[1] * Ztemp / 255.0 + 1.0 - Ztemp,
		    RGBColor[2] * Ztemp / 255.0 + 1.0 - Ztemp);
	else
	    fprintf(f, "%f setgray\n", (1.0 - Gray) * (1.0 - Ztemp));
    }

    /* Dumps selected font and create text at proper location. */
    if (!IRIT_APX_EQ(LastScale, Scale)) {
        fprintf(f,
		"/currentfont /%s findfont %f scalefont def\ncurrentfont setfont\n",
		GlblFontName, Scale);

	LastScale = Scale;
    }

    fprintf(f, "%f %f moveto (%s) show\n", Pos[0], Pos[1], PObj -> U.Str);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Ps exit routine.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2PsExit(int ExitCode)
{
    exit(ExitCode);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dummy function to link at debugging time.                               *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*****************************************************************************/
void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
