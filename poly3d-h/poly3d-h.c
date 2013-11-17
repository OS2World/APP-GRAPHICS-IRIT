/*****************************************************************************
*   Program to draw 3D object as wireframe after removing the hidden lines.  *
* This porgram works in object space, and if redirect stdout to a file, dump *
* the visible polylines into it instead of drawing them on current device.   *
* This may be used to display the results on any device (a plotter !?) later.*
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 3.0, Aug. 1990   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "misc_lib.h"
#include "ip_cnvrt.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Poly3D-H	Version 11,	Gershon Elber,\n\
	(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Poly3D-H	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",  " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "poly3d-h b%- m%- i%- e%-#Edges!d H%- 4%- W%-Width!F F%-PolyOpti|FineNess!d!F q%- o%-OutName!s t%-AnimTime!F c%- z%- DFiles!*s";

IRIT_GLOBAL_DATA int
    NumOfPolygons = 0;		      /* Total number of polygons to handle. */
IRIT_GLOBAL_DATA IrtHmgnMatType GlblViewMat;		  /* Current view of object. */

/* Data structures used by the hidden line modules: */
IRIT_GLOBAL_DATA int
    EdgeCount = 0;
IRIT_GLOBAL_DATA EdgeStruct *EdgeHashTable[EDGE_HASH_TABLE_SIZE];
IRIT_GLOBAL_DATA IPPolygonStruct *PolyHashTable[POLY_HASH_TABLE_SIZE];

/* The following are setable variables (via configuration file poly3d-h.cfg).*/
IRIT_GLOBAL_DATA int
    GlblMore = FALSE,
    GlblClipScreen = TRUE,
    GlblFourPerFlat = FALSE,
    GlblOptimalPolyApprox = FALSE,
    GlblQuiet = FALSE,
    GlblOutputHasRGB = FALSE,
    GlblOutputRGB[3] = { 255, 255, 255 },
    GlblOutputColor = VISIBLE_COLOR,
    GlblNumEdge = 0,
    GlblBackFacing = FALSE,
    GlblInternal = FALSE,
    GlblOutputHiddenData = FALSE;

IRIT_GLOBAL_DATA IrtRType
    GlblFineNess = DEFAULT_FINENESS,
    GlblOutputWidth = VISIBLE_WIDTH;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
    { "Internal",    "-i", (VoidPtr) &GlblInternal,	IC_BOOLEAN_TYPE },
    { "BackFacing",  "-b", (VoidPtr) &GlblBackFacing,	IC_BOOLEAN_TYPE },
    { "More",	     "-m", (VoidPtr) &GlblMore,		IC_BOOLEAN_TYPE },
    { "ClipScreen",  "-c", (VoidPtr) &GlblClipScreen,	IC_BOOLEAN_TYPE },
    { "Quiet",	     "-q", (VoidPtr) &GlblQuiet,	IC_BOOLEAN_TYPE },
    { "FourPerFlat", "-4", (VoidPtr) &GlblFourPerFlat,	IC_BOOLEAN_TYPE },
    { "DumpHidden",  "-H", (VoidPtr) &GlblOutputHiddenData,IC_BOOLEAN_TYPE },
    { "PolyOpti",    "-F", (VoidPtr) &GlblOptimalPolyApprox,IC_INTEGER_TYPE },
    { "NumOfEdges",  "-e", (VoidPtr) &GlblNumEdge,	IC_INTEGER_TYPE },
    { "LineWidth",   "-W", (VoidPtr) &GlblOutputWidth,	IC_REAL_TYPE },
    { "FineNess",    "-F", (VoidPtr) &GlblFineNess,     IC_REAL_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of poly3d-h - Read command line and do what is needed...	     M
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
int main(int argc, char **argv)
{
    int Error,
	EdgesFlag = FALSE,
	VerFlag = FALSE,
	OutFlag = FALSE,
	WidthFlag = FALSE,
	NumFiles = FALSE,
        TimeFlag = FALSE,
	OptPolyApproxFlag = FALSE;
    char
	*OutFileName = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    FILE *OutFile;
    IPObjectStruct *PObjects, *PTmpObj;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IritConfig("poly3d-h", SetUp, NUM_SET_UP);/* Read config. file if exists.*/

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &GlblBackFacing, &GlblMore, &GlblInternal,
			   &EdgesFlag, &GlblNumEdge, &GlblOutputHiddenData,
			   &GlblFourPerFlat, &WidthFlag,
			   &GlblOutputWidth, &OptPolyApproxFlag,
			   &GlblOptimalPolyApprox, &GlblFineNess,
			   &GlblQuiet, &OutFlag, &OutFileName,
			   &TimeFlag, &CurrentTime,
			   &GlblClipScreen, &VerFlag, &NumFiles,
			   &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Poly3dhExit(1);
    }

    if (VerFlag) {
	fprintf(stderr, "\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	IritConfigPrint(SetUp, NUM_SET_UP);
	Poly3dhExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit\n");
	GAPrintHowTo(CtrlStr);
	Poly3dhExit(1);
    }

    if (GlblQuiet)
	GlblMore = FALSE;

    if (GlblOutputHiddenData && GlblBackFacing) {
	IRIT_WARNING_MSG("Warning: You have activated both output of hidden data (-H) and back facing\n");
	IRIT_WARNING_MSG("\t elimination (-b). Chances are you did not want to do that.\n");
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames, NumFiles,
				   TRUE, GlblMore)) == NULL)
	Poly3dhExit(1);
    PObjects = IPResolveInstances(PObjects);

    /* Do we have animation time flag? */
    if (TimeFlag)
        GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    IPFlattenInvisibleObjects(FALSE);
    PObjects = IPFlattenForrest(PObjects);

    /* If has color/width attribute in geometries - uses them. Assume */
    /* all objects share the same attribute (should be fixed).        */
    for (PTmpObj = PObjects; PTmpObj != NULL; PTmpObj = PTmpObj -> Pnext) {
	if (IP_IS_GEOM_OBJ(PTmpObj)) {
	    int Color;
	    IrtRType
		Width = AttrGetObjectRealAttrib(PTmpObj, "width");

	    if (!IP_ATTR_IS_BAD_REAL(Width))
		GlblOutputWidth = Width;

	    if (!GlblOutputHasRGB)
		GlblOutputHasRGB = AttrGetObjectRGBColor(PTmpObj,
							 &GlblOutputRGB[0],
							 &GlblOutputRGB[1],
							 &GlblOutputRGB[2]);
	    if ((Color = AttrGetObjectColor(PTmpObj)) != IP_ATTR_NO_COLOR)
		GlblOutputColor = Color;
	}
    }

    /* And update the global viewing matrix: */
    if (IPWasPrspMat)
	MatMultTwo4by4(GlblViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(GlblViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    /* Prepare data structures to be able to decide on visibility: */
    PrepareViewData(PObjects);

    if (OutFlag) {
	if ((OutFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Poly3dhExit(2);
	}
    }
    else
	OutFile = stdout;

    OutVisibleEdges(OutFile);	       /* Scan all sub-edges output visible. */

    if (OutFile != stdout)
	fclose(OutFile);

    Poly3dhExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert all surfaces/curves into polylines as follows:	     M
*   Curve is converted to a single polyline with SamplesPerCurve samples.    M
*   Surface is converted into GlblNumOfIsolines curves in each axes, each    M
* handled as Curve above. The original curves and surfaces are then deleted. M
*   This function is a call back function of the irit parser.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:  Crvs/Srfs/Trimmed Srfs/Trivariates/TriSrf read from a file   M
*               by the irit parser.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Processed freeform geometry. This function simply    M
*                       returns what it gots.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessFreeForm, conversion                                            M
*****************************************************************************/
IPObjectStruct *IPProcessFreeForm(IPFreeFormStruct *FreeForms)
{
    IPObjectStruct *Object;
    IPPolygonStruct *Poly;

    /* Make sure requested format is something reasonable. */
    if (GlblOptimalPolyApprox == 0 && GlblFineNess < 2) {
	GlblFineNess = 2;
	if (GlblMore)
	    IRIT_WARNING_MSG(
		    "FineNess is less than two, two picked instead.\n");
    }

    for (Object = FreeForms -> CrvObjs;
	 Object != NULL;
	 Object = Object -> Pnext) {
	CagdCrvStruct *Curves, *Curve;
	IrtRType
	    RelativeFineNess = AttrGetObjectRealAttrib(Object,
						       "crv_resolution");

	if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	    RelativeFineNess = 1.0;

        Curves = Object -> U.Crvs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYLINE_OBJ(Object);
	for (Curve = Curves; Curve != NULL; Curve = Curve -> Pnext) {
	    Poly = IPCurve2Polylines(Curve,
				     (int) (GlblFineNess * RelativeFineNess),
				     SYMB_CRV_APPROX_UNIFORM);

	    Object -> U.Pl = IPAppendPolyLists(Poly, Object -> U.Pl);
	}
	CagdCrvFreeList(Curves);
    }

    for (Object = FreeForms -> SrfObjs;
	 Object != NULL;
	 Object = Object -> Pnext) {
	CagdSrfStruct *Surfaces, *Surface;
	IrtRType
	    RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");

	if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	    RelativeFineNess = 1.0;

        Surfaces = Object -> U.Srfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (Surface = Surfaces; Surface != NULL; Surface = Surface -> Pnext) {
	    IrtRType t;

	    t = AttrGetObjectRealAttrib(Object, "u_resolution");
	    if (!IP_ATTR_IS_BAD_REAL(t))
	        AttrSetRealAttrib(&Surface -> Attr, "u_resolution", t);
	    t = AttrGetObjectRealAttrib(Object, "u_resolution");
	    if (!IP_ATTR_IS_BAD_REAL(t))
	        AttrSetRealAttrib(&Surface -> Attr, "v_resolution", t);

	    Poly = IPSurface2Polygons(Surface, GlblFourPerFlat,
				      GlblFineNess * RelativeFineNess,
				      FALSE, FALSE, GlblOptimalPolyApprox);
	    if (Poly != NULL) {
		IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
		Object -> U.Pl = Poly;
	    }
	}
	CagdSrfFreeList(Surfaces);
    }

    /* Converts models, if any, to freeform trimmed surfaces, in place. */
    IPProcessModel2TrimSrfs(FreeForms);

    for (Object = FreeForms -> TrimSrfObjs;
	 Object != NULL;
	 Object = Object -> Pnext) {
	IrtRType RelativeFineNess;
	TrimSrfStruct *TrimSrfs, *TrimSrf;

	RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
	if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	    RelativeFineNess = 1.0;

        TrimSrfs = Object -> U.TrimSrfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (TrimSrf = TrimSrfs; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
            Poly = IPTrimSrf2Polygons(TrimSrf, GlblFourPerFlat,
				      GlblFineNess * RelativeFineNess,
				      FALSE, FALSE, GlblOptimalPolyApprox);
	    if (Poly != NULL) {
		IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
		Object -> U.Pl = Poly;
	    }
        }
        TrimSrfFreeList(TrimSrfs);
    }

    for (Object = FreeForms -> TrivarObjs;
	 Object != NULL;
	 Object = Object -> Pnext) {
	IrtRType RelativeFineNess;
	TrivTVStruct *Trivars, *Trivar;

	RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
	if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	    RelativeFineNess = 1.0;

        Trivars = Object -> U.Trivars;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (Trivar = Trivars; Trivar != NULL; Trivar = Trivar -> Pnext) {
            Poly = IPTrivar2Polygons(Trivar, GlblFourPerFlat,
				     GlblFineNess * RelativeFineNess,
				     FALSE, FALSE, GlblOptimalPolyApprox);
	    if (Poly != NULL) {
		IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
		Object -> U.Pl = Poly;
	    }
        }
        TrivTVFreeList(Trivars);
    }

    for (Object = FreeForms -> TriSrfObjs;
	 Object != NULL;
	 Object = Object -> Pnext) {
	IrtRType RelativeFineNess;
	TrngTriangSrfStruct *TriSrfs, *TriSrf;

	RelativeFineNess = AttrGetObjectRealAttrib(Object, "resolution");
	if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	    RelativeFineNess = 1.0;

        TriSrfs = Object -> U.TriSrfs;
        Object -> U.Pl = NULL;
        Object -> ObjType = IP_OBJ_POLY;
        IP_SET_POLYGON_OBJ(Object);
        for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
            Poly = IPTriSrf2Polygons(TriSrf,
				     GlblFineNess * RelativeFineNess,
				     FALSE, FALSE, GlblOptimalPolyApprox);
	    if (Poly != NULL) {
		IPGetLastPoly(Poly) -> Pnext = Object -> U.Pl;
		Object -> U.Pl = Poly;
	    }
        }
	TrngTriSrfFreeList(TriSrfs);
    }

    return IPConcatFreeForm(FreeForms);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Poly3d-h Exit routine.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ExitCode:    To notify O.S. with result of program.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Poly3dhExit                                                              M
*****************************************************************************/
void Poly3dhExit(int ExitCode)
{
    fprintf(stderr, "\n");

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
