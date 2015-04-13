/*****************************************************************************
* Find neighbors of a given vertex in a model.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan 2003    *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "grap_lib.h"
#include "misc_lib.h"

#define DRAW_PRIN_DIR(Str, Clr, Width) if (Str != NULL) { \
	    IPObjectStruct *PObjCrv; \
	    sscanf(Str, "%lg, %lg, %lg", &Pt2.Pt[0], &Pt2.Pt[1], &Pt2.Pt[2]); \
	    IRIT_PT_ADD(Pt2.Pt, Pt2.Pt, Pt1.Pt); \
	    PObjCrv = IPGenCRVObject(CagdMergePtPt(&Pt1, &Pt2)); \
	    AttrSetObjectColor(PObjCrv, Clr); \
	    AttrSetObjectRealAttrib(PObjCrv, "dwidth", Width); \
	    IPSocWriteOneObject(PrgmIO, PObjCrv); \
	    IPFreeObject(PObjCrv); }

static char *CtrlStr =
    "PlNghbr r%-#Rings!d R%-PtRad!F h%- DFiles!*s";

void main(int argc, char **argv)
{
    int i, j, OldRes, NumFiles, Error, PrgmIO, *Nbrs,
	NumRings = 1,
	RingsFlag = FALSE,
	RadiusFlag = FALSE,
	HelpFlag = FALSE;
    char **FileNames,
	*Program = getenv("IRIT_DISPLAY");
    double
	SphereRadius = 0.02;
    IPObjectStruct *PObj, *PTmp;
    IPPolyVrtxIdxStruct *PVIdx;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &RingsFlag, &NumRings,
			   &RadiusFlag, &SphereRadius,
			   &HelpFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	exit(1);
    }

    if (HelpFlag) {
	fprintf(stderr, "This is poly neighborhood testing...\n");
	GAPrintHowTo(CtrlStr);
	exit(0);
    }

#ifdef __WINNT__
    if (Program == NULL)
	Program = "wntgdrvs -s-";
#endif /* __WINNT__ */
#ifdef __UNIX__
    if (Program == NULL)
	Program = "x11drvs -s-";
#endif /* __UNIX__ */

    /* Get the data files: */
    if ((PObj = IPGetDataFiles(FileNames, NumFiles, TRUE, FALSE)) == NULL ||
	!IP_IS_POLY_OBJ(PObj) ||
	!IP_IS_POLYGON_OBJ(PObj)) {
	fprintf(stderr, "Expecting one polygonal model...\n");
	exit(1);
    }

    /* Compute curvature estimations per vertex. */
    fprintf(stderr, "Starting to compute curvature info...");
    GMPlCrvtrSetCurvatureAttr(PObj -> U.Pl, 1, TRUE);
    fprintf(stderr, "Done.\n");

    /* Build alternative vertex-list and poly-pointers to these polygons. */
    fprintf(stderr, "Starting to build vertex Idx data structure...");
    PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, TRUE, 0);
    fprintf(stderr, "Done.\n");

    OldRes = PrimSetResolution(4);

    IPSocSrvrInit();            /* Initialize the listen socket for clients. */

    if ((PrgmIO = IPSocExecAndConnect(Program,
				      getenv("IRIT_BIN_IPC") != NULL)) >= 0) {
	char Line[IRIT_LINE_LEN];
	IPObjectStruct
	    *PClrObj = IPGenStrObject("command_", "clear", NULL);

	for (i = 0; i < PVIdx -> NumVrtcs; i++) {
	    const char *Str;
	    CagdPtStruct Pt1, Pt2;

	    /* Clear old data and display our curve and data. */
	    IPSocWriteOneObject(PrgmIO, PClrObj);

	    IPSocWriteOneObject(PrgmIO, PObj);

	    /* Draw the center vertex. */
	    PTmp = PrimGenSPHEREObject(PVIdx -> Vertices[i] -> Coord,
				       SphereRadius);
	    AttrSetObjectColor(PTmp, IG_IRIT_GREEN);
	    IPSocWriteOneObject(PrgmIO, PTmp);
	    IPFreeObject(PTmp);

	    /* Draw the neighbors. */
	    Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, i, NumRings);
	    for (j = 0; Nbrs[j] >= 0; j++) {
	        PTmp = PrimGenSPHEREObject(PVIdx -> Vertices[Nbrs[j]] -> Coord,
					   SphereRadius);
		AttrSetObjectColor(PTmp, IG_IRIT_RED);
		IPSocWriteOneObject(PrgmIO, PTmp);
		IPFreeObject(PTmp);
	    }

	    /* Process curvature info. */
	    IRIT_PT_COPY(Pt1.Pt, PVIdx -> Vertices[i] -> Coord);

	    printf("\nK = %8.5g, H = %8.5g\n",
		   AttrGetRealAttrib(PVIdx -> Vertices[i] -> Attr, "KCurv"),
		   AttrGetRealAttrib(PVIdx -> Vertices[i] -> Attr, "HCurv"));

	    printf("k1 = %8.5g, D1 = %s\n",
		   AttrGetRealAttrib(PVIdx -> Vertices[i] -> Attr, "K1Curv"),
		   Str = AttrGetStrAttrib(PVIdx -> Vertices[i] -> Attr, "D1"));
	    DRAW_PRIN_DIR(Str, IG_IRIT_MAGENTA, 4);

	    printf("k2 = %8.5g, D2 = %s\n",
		   AttrGetRealAttrib(PVIdx -> Vertices[i] -> Attr, "K2Curv"),
		   Str = AttrGetStrAttrib(PVIdx -> Vertices[i] -> Attr, "D2"));
	    DRAW_PRIN_DIR(Str, IG_IRIT_CYAN, 2);

	    gets(Line);
	    if (Line[0] == 'q' || Line[0] == 'Q')
	        break;
	}

	IPSocDisConnectAndKill(TRUE, PrgmIO);
	IPFreeObject(PClrObj);
    }

    /* Going over all vertices for neighbors. */
    fprintf(stderr, "Starting traversal over all neighbors...");
    for (i = 0; i < PVIdx -> NumVrtcs; i++) {
        Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, i, NumRings);
    }
    fprintf(stderr, "Done with %d vertices\n", i);

    IPPolyVrtxIdxFree(PVIdx);

    IPFreeObject(PObj);

    PrimSetResolution(OldRes);

    exit(0);
}
