/*****************************************************************************
* IritWCnc.c - Module to save IRIT data into a G Code NC files.	       	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, Mar 2007   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>

#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

/* Accuracy of curve approximation into piecewise linear motion. */
#define IP_NC_CRV_TOLERANCE         0.025
#define IP_NC_DEF_RETRACT_LEVEL	    (Units == 1 ? 25.0 : 1.0)
#define IP_NC_DEF_MAX_XYBRIDGE_GAP  (Units == 1 ? 1.0 : 0.04)
#define IP_NC_DEF_MAX_ZBRIDGE_GAP   (Units == 1 ? 2.0 : 0.08)
#define IP_NC_ADVANCE_LINE(n) \
	    if (n >= 9990) \
	        n = n % 10 + 1; \
	    else \
	        n += 10 \

IRIT_GLOBAL_DATA char
    *_IPNCGcodeFloatFormat = "%-.2f";

IRIT_STATIC_DATA const char
    *GlblNCGCodeComment = NULL;
IRIT_STATIC_DATA int
    GlblNCGcodeReverseZ = FALSE,
    GlblNCFirstObject = TRUE,
    GlblNCUnits = 1,
    GlblNCMessages = FALSE,
    GlblNCLineNum = 10,
    GlblNCGcodeUpRetractFast = FALSE;
IRIT_STATIC_DATA FILE
    *GlblNCFile = NULL;
IRIT_STATIC_DATA IrtRType
    GlblGCodeCrvApproxTolSamples = IP_NC_CRV_TOLERANCE,
    GlblNCGCodeFeedRate = 10.0,
    GlblNCGCodeBridgeRelFeedRate = 1.0,
    GlblNCGcodeRetractZLevel = 10.0,
    GlblNCGcodeMaxXYBridgeGap = 0.0,
    GlblNCGcodeMaxZBridgeGap = 0.0,
    GlblNCGcodeNCDownPlungeFast = IRIT_INFNTY;

IRIT_STATIC_DATA IrtPtType
    GlblNCGcodePos = { IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY };

static void DumpVertexForNCGCode(IPVertexStruct *V, int n1, int n2);
static void DumpDataForNCGCode(IPObjectStruct *PObj, IrtHmgnMatType DummyMat);

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Sets the tolerance to use when approximating curves as polylines for    M
* CNC path.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Tol:   New tolerance to use.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Old tolerance.                                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeSaveFile                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeSaveFileSetTol                                                  M
*****************************************************************************/
IrtRType IPNCGCodeSaveFileSetTol(IrtRType Tol)
{
    IrtRType
        OldVal = GlblGCodeCrvApproxTolSamples;

    GlblGCodeCrvApproxTolSamples = Tol;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dumps IRIT object as an NCGcode file.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:          IritObject to dump as NCGCODE file.                       M
*   CrntViewMat:   The current viewing matrix to apply to the object.        M
*   NCGCODEFileName:   Name of NCGCODE file, "-" or NULL for stdout.	     M
*   Messages:      TRUE for warning messages.				     M
*   Units:         0 for inches, 1 for Milimeters.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeSaveFileSetTol                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeSaveFile                                                        M
*****************************************************************************/
int IPNCGCodeSaveFile(const IPObjectStruct *PObj,
		      IrtHmgnMatType CrntViewMat,
		      const char *NCGCODEFileName,
		      int Messages,
		      int Units)
{
    int i,
	OldRefCountState = IPSetCopyObjectReferenceCount(FALSE);
    IPObjectStruct
        *PTmp = IPCopyObject(NULL, PObj, TRUE);
    GMBBBboxStruct *BBox;

    PTmp -> Pnext = NULL;
    BBox = GMBBComputeBboxObject(PTmp);

    IPSetCopyObjectReferenceCount(OldRefCountState);

    if (NCGCODEFileName != NULL && strncmp(NCGCODEFileName, "-", 1) != 0) {
        if ((GlblNCFile = fopen(NCGCODEFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n",
				    NCGCODEFileName);
	    return FALSE;
	}
    }
    else
        GlblNCFile = stdout;

    /* Feedrate parameters. */
    GlblNCGCodeFeedRate = AttrGetObjectRealAttrib(PTmp, "NCFeedRate");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGCodeFeedRate)) {
        GlblNCGCodeFeedRate = 10.0;
        IRIT_WARNING_MSG_PRINTF("No \"NCFeedRate\" attribute detected, using %f instead\n",
				GlblNCGCodeFeedRate);
    }

    GlblNCGCodeBridgeRelFeedRate =
	AttrGetObjectRealAttrib(PTmp, "NCBridgeRelFeedRate");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGCodeBridgeRelFeedRate)) {
        GlblNCGCodeBridgeRelFeedRate = 1.0;
        IRIT_WARNING_MSG_PRINTF("No \"NCBridgeRelFeedRate\" attribute detected, using %f instead\n",
				GlblNCGCodeBridgeRelFeedRate);
    }

    /* Reverse Z parameter. */
    GlblNCGcodeReverseZ = AttrGetObjectIntAttrib(PTmp, "NCReverseZ");
    if (IP_ATTR_IS_BAD_INT(GlblNCGcodeReverseZ))
	GlblNCGcodeReverseZ = FALSE;
    IRIT_INFO_MSG_PRINTF("\"NCReverseZ\" attributed detected = %d\n",
			 GlblNCGcodeReverseZ);

    /* Z retraction level parameter. */
    GlblNCGcodeRetractZLevel = AttrGetObjectRealAttrib(PTmp, "NCRetractZLevel");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGcodeRetractZLevel)) {
	if (GlblNCGcodeReverseZ)
	    GlblNCGcodeRetractZLevel = BBox -> Min[2] - IP_NC_DEF_RETRACT_LEVEL;
	else
	    GlblNCGcodeRetractZLevel = BBox -> Max[2] + IP_NC_DEF_RETRACT_LEVEL;

        IRIT_WARNING_MSG_PRINTF("No \"NCRetractZLevel\" attribute detected, using %f instead\n",
				GlblNCGcodeRetractZLevel);
    }
    IRIT_INFO_MSG_PRINTF("\"NCRetractZLevel\" set at %f\n",
			 GlblNCGcodeRetractZLevel);

    /* Up/Down motion parameters. */
    GlblNCGcodeUpRetractFast = AttrGetObjectIntAttrib(PTmp, "NCUpRetractFast");
    if (IP_ATTR_IS_BAD_INT(GlblNCGcodeUpRetractFast))
        GlblNCGcodeUpRetractFast = FALSE;
    IRIT_INFO_MSG_PRINTF("\"NCUpRetractFast\" set to %d\n",
			 GlblNCGcodeUpRetractFast);
    GlblNCGcodeNCDownPlungeFast = AttrGetObjectRealAttrib(PTmp, "NCDownPlungeFast");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGcodeNCDownPlungeFast))
        GlblNCGcodeNCDownPlungeFast = IRIT_INFNTY;
    IRIT_INFO_MSG_PRINTF("\"NCDownPlungeFast\" set to %f\n",
			 GlblNCGcodeNCDownPlungeFast);

    /* Maximum XY and Z distances to bridge parameters. */
    GlblNCGcodeMaxXYBridgeGap = AttrGetObjectRealAttrib(PTmp, "NCMaxXYBridgeGap");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGcodeMaxXYBridgeGap)) {
	GlblNCGcodeMaxXYBridgeGap = IP_NC_DEF_MAX_XYBRIDGE_GAP;

        IRIT_WARNING_MSG_PRINTF("No \"NCMaxXYBridgeGap\" attribute detected, using %f instead\n",
				GlblNCGcodeMaxXYBridgeGap);
    }

    GlblNCGcodeMaxZBridgeGap = AttrGetObjectRealAttrib(PTmp, "NCMaxZBridgeGap");
    if (IP_ATTR_IS_BAD_REAL(GlblNCGcodeMaxZBridgeGap)) {
	GlblNCGcodeMaxZBridgeGap = IP_NC_DEF_MAX_ZBRIDGE_GAP;

        IRIT_WARNING_MSG_PRINTF("No \"NCMaxZBridgeGap\" attribute detected, using %f instead\n",
				GlblNCGcodeMaxZBridgeGap);
    }

    GlblNCGcodePos[0] = GlblNCGcodePos[1] = GlblNCGcodePos[2] = IRIT_INFNTY;

    IPFFCState.CrvApproxTolSamples = GlblGCodeCrvApproxTolSamples;
    IPFFCState.CrvApproxMethod = SYMB_CRV_APPROX_TOLERANCE;

    _IPNCGcodeFloatFormat = Units == 1 ? "%-.2f" : "%.3f";
    GlblNCUnits = Units;
    GlblNCMessages = Messages;
    GlblNCLineNum = 10;

    /* handle comment header. */
    GlblNCGCodeComment = AttrGetObjectStrAttrib(PTmp, "NCCommentChar");
    if (GlblNCGCodeComment == NULL || GlblNCGCodeComment[0] == 0) {
        IRIT_WARNING_MSG_PRINTF("No \"NCCommentChar\" attribute detected, dumping no comment\n");
    }
    else {
        fprintf(GlblNCFile, "%c\n", GlblNCGCodeComment[0]);
        fprintf(GlblNCFile, "%c NC file \"%s\"\n",
		GlblNCGCodeComment[0],
		GlblNCFile == stdout ? "stdout" : NCGCODEFileName);
        fprintf(GlblNCFile,
		IRIT_EXP_STR("%c The IRIT solid modeler version %s%s\n"),
		GlblNCGCodeComment[0], IRIT_VERSION,
#ifdef DEBUG
		"");        /* Save no date in debug so we can diff files. */
#else
		", " __DATE__);
#endif /* DEBUG */
        fprintf(GlblNCFile, "%c PARAMETERS:\n", GlblNCGCodeComment[0]);
        fprintf(GlblNCFile, "%c Units: %s. %sReversed Z\n",
		GlblNCGCodeComment[0], 
		GlblNCUnits == 0 ? "inches" : "mm",
		GlblNCGcodeReverseZ ? "" : "No ");
        fprintf(GlblNCFile, "%c Feedrate %.6lg\n",
		GlblNCGCodeComment[0], GlblNCGCodeFeedRate);
	fprintf(GlblNCFile, "%c Retract level %.6lg\n",
		GlblNCGCodeComment[0], GlblNCGcodeRetractZLevel);
	fprintf(GlblNCFile, "%c XY bridge distance %.6lg, Z bridge distance %.6lg\n",
		GlblNCGCodeComment[0], 
		GlblNCGcodeMaxXYBridgeGap, GlblNCGcodeMaxZBridgeGap);
	fprintf(GlblNCFile, "%c BBox: [%.6lg, %.6lg, %.6lg] :: [%.6lg, %.6lg, %.6lg]\n",
		GlblNCGCodeComment[0], 
		BBox -> Min[0], BBox -> Min[1], BBox -> Min[2],
		BBox -> Max[0], BBox -> Max[1], BBox -> Max[2]);
        fprintf(GlblNCFile, "%c\n", GlblNCGCodeComment[0]);
    }

    /* Dump a prelude. */
    fprintf(GlblNCFile, "n%04d g90\n", GlblNCLineNum = 10);  /* Abs. coords. */
    IP_NC_ADVANCE_LINE(GlblNCLineNum);
    fprintf(GlblNCFile, "n%04d g%d t1 m6\n",
	    GlblNCLineNum, GlblNCUnits == 0 ? 70 : 71);
    IP_NC_ADVANCE_LINE(GlblNCLineNum);

    GlblNCGcodePos[0] = BBox -> Min[0];
    GlblNCGcodePos[1] = BBox -> Min[1];
    GlblNCGcodePos[2] = GlblNCGcodeRetractZLevel;
    fprintf(GlblNCFile, "n%04d g0", GlblNCLineNum );
    for (i = 0; i < 3; i++) {
        fprintf(GlblNCFile, " %c", 'x' + i);
	fprintf(GlblNCFile, _IPNCGcodeFloatFormat, GlblNCGcodePos[i]);
    }
    IP_NC_ADVANCE_LINE(GlblNCLineNum);
    fprintf(GlblNCFile, "\n");
    GlblNCFirstObject = TRUE;

    IPTraverseObjHierarchy(PTmp, NULL, IPMapObjectInPlace, CrntViewMat, FALSE);
    PTmp = IPResolveInstances(PTmp);

    IPTraverseObjHierarchy(PTmp, NULL, DumpDataForNCGCode, CrntViewMat, FALSE);

    IPFreeObject(PTmp);

    /* Dump a postlude. */
    fprintf(GlblNCFile, "n%04d z", GlblNCLineNum);
    IP_NC_ADVANCE_LINE(GlblNCLineNum);
    fprintf(GlblNCFile, _IPNCGcodeFloatFormat,
	    GlblNCGcodePos[2] = GlblNCGcodeRetractZLevel);
    fprintf(GlblNCFile, "\n");
    fprintf(GlblNCFile, "n%04d m30", GlblNCLineNum);
    IP_NC_ADVANCE_LINE(GlblNCLineNum);

    if (GlblNCFile != stdout)
        fclose(GlblNCFile);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Dumps teh XYZ coordinates of the given vertex to GCode.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   V:        Vertex to dump its XYZ coordinates.                            *
*   n1, n2:   Coordinates to dump: 0-1 for XY, 2-2 for Z, etc.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpVertexForNCGCode(IPVertexStruct *V, int n1, int n2)
{
    int i;

    for (i = n1; i <= n2; i++) {
        if (!IRIT_APX_EQ(V -> Coord[i], GlblNCGcodePos[i])) {
	    fprintf(GlblNCFile, " %c", 'x' + i);
	    fprintf(GlblNCFile, _IPNCGcodeFloatFormat, V -> Coord[i]);
	    GlblNCGcodePos[i] = V -> Coord[i];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for NCGCode to OutFileName.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         To dump into file.                                         *
*   DummyMat:     Matrix to transform object.  Ignored.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForNCGCode(IPObjectStruct *PObj, IrtHmgnMatType DummyMat)
{
    char
	*Name = IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "irit2GCode";
    int i;
    IPPolygonStruct *Pl,
	*Pllns = NULL;
    IPObjectStruct *PTmp,
        *PPObj = NULL;

    /* Process freeform univariate geometry into polylines. */
    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    if (IP_IS_POLYLINE_OBJ(PObj))
	        Pllns = PObj -> U.Pl;
	    break;
	case IP_OBJ_CURVE:
	    PPObj = IPCopyObject(NULL, PObj, FALSE);
	    IPConvertFreeForm(PPObj, &IPFFCState);     /* Convert in place. */
	    Pllns = IPReversePlList(PPObj -> U.Pl);
	    break;
	case IP_OBJ_LIST_OBJ:
	    i = 0;

	    /* Search in its list. */
	    while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	        DumpDataForNCGCode(PTmp, DummyMat);
	    return;
        default:
	    return;
    }

    if (GlblNCMessages)	
        IRIT_INFO_MSG_PRINTF("Processing object \"%s\".\n", Name);

    if (!GlblNCFirstObject) {
        IrtRType
	    *R = Pllns -> PVertex -> Coord,
	    DXY = IRIT_PT2D_DIST(GlblNCGcodePos, R),
	    DZ = GlblNCGcodePos[2] - R[2];

	if (DXY > GlblNCGcodeMaxXYBridgeGap ||
	    DZ > GlblNCGcodeMaxZBridgeGap) {   /* Retract safely to new pos. */
	    IRIT_INFO_MSG_PRINTF("Retracting before approaching object \"%s\"; DXY = %6f, DZ = %6f.\n",
				 Name, DXY, DZ);

	    if (GlblNCGcodeUpRetractFast)
	        fprintf(GlblNCFile, "n%04d g0 z", GlblNCLineNum);
	    else
	        fprintf(GlblNCFile, "n%04d z", GlblNCLineNum);
	    IP_NC_ADVANCE_LINE(GlblNCLineNum);
	    fprintf(GlblNCFile, _IPNCGcodeFloatFormat,
		    GlblNCGcodePos[2] = GlblNCGcodeRetractZLevel);
	    fprintf(GlblNCFile, "\n");
	}
    }
    GlblNCFirstObject = FALSE;

    /* Dump the polyline geometry out. */
    for (Pl = Pllns; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct *V;
	int RecoverFeedrate = FALSE;

	if (Pl -> PVertex == NULL) {
	    IRIT_INFO_MSG_PRINTF("Zero length polyline detected.  Aborted\n");
	    break;
	}
	  
	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    if (V == Pl -> PVertex) {	      /* First location in polyline. */
	        IrtRType Dz;

	        if ((GlblNCGcodeReverseZ &&
		     GlblNCGcodePos[2] <= GlblNCGcodeRetractZLevel) ||
		    (!GlblNCGcodeReverseZ &&
		     GlblNCGcodePos[2] >= GlblNCGcodeRetractZLevel)) {
		    /* Move at retraction level just above the new location. */
		    if (!IRIT_APX_EQ(GlblNCGcodePos[2], GlblNCGcodeRetractZLevel))
			IRIT_INFO_MSG_PRINTF(
			    "Motion to \"%s\" (z %6f) is above retraction level %6f; using G0\n",
			    Name, GlblNCGcodePos[2], GlblNCGcodeRetractZLevel);

		    fprintf(GlblNCFile, "n%04d g0", GlblNCLineNum);
		    IP_NC_ADVANCE_LINE(GlblNCLineNum);

		    DumpVertexForNCGCode(V, 0, 1);

		    fprintf(GlblNCFile, "\n");
		}

		RecoverFeedrate = GlblNCGCodeBridgeRelFeedRate != 1.0;

		Dz = GlblNCGcodePos[2] - V -> Coord[2];
		if (IRIT_ABS(Dz) > GlblNCGcodeNCDownPlungeFast) {
		    IPVertexStruct VDup;
		    /* Plunge in g0 first. */
		    fprintf(GlblNCFile, "n%04d g0", GlblNCLineNum);
		    IP_NC_ADVANCE_LINE(GlblNCLineNum);

		    VDup = *V;

		    VDup.Coord[2] += Dz > 0 ? GlblNCGcodeNCDownPlungeFast
			  	            : -GlblNCGcodeNCDownPlungeFast;
		    DumpVertexForNCGCode(&VDup, 2, 2);

		    fprintf(GlblNCFile, "\n");
		}

		fprintf(GlblNCFile, "n%04d g1 f", GlblNCLineNum);
		fprintf(GlblNCFile, _IPNCGcodeFloatFormat,
			GlblNCGCodeFeedRate * GlblNCGCodeBridgeRelFeedRate);
	    }
	    else {
		if (RecoverFeedrate) {
		    fprintf(GlblNCFile, "n%04d f", GlblNCLineNum);
		    fprintf(GlblNCFile, _IPNCGcodeFloatFormat,
			    GlblNCGCodeFeedRate);
		    RecoverFeedrate = FALSE;
		}
		else
	            fprintf(GlblNCFile, "n%04d", GlblNCLineNum);
	    }

	    IP_NC_ADVANCE_LINE(GlblNCLineNum);

	    DumpVertexForNCGCode(V, 0, 2);
	    fprintf(GlblNCFile, "\n");
	}

	if (Pl -> Pnext != NULL && Pl -> Pnext -> PVertex != NULL) {
	    IrtRType
	        *R = Pl -> Pnext -> PVertex -> Coord,
	        DXY = IRIT_PT2D_DIST(GlblNCGcodePos, R),
	        DZ = IRIT_FABS(GlblNCGcodePos[2] - R[2]);

	    if (DXY > GlblNCGcodeMaxXYBridgeGap ||
		DZ > GlblNCGcodeMaxZBridgeGap) {/* Retract safely to new pos.*/
	        if (GlblNCGcodeUpRetractFast)
		    fprintf(GlblNCFile, "n%04d g0 z", GlblNCLineNum);
		else
		    fprintf(GlblNCFile, "n%04d z", GlblNCLineNum);
		IP_NC_ADVANCE_LINE(GlblNCLineNum);
		fprintf(GlblNCFile, _IPNCGcodeFloatFormat,
			GlblNCGcodePos[2] = GlblNCGcodeRetractZLevel);
		fprintf(GlblNCFile, "\n");
	    }
	}
    }

    if (PPObj != NULL)			        /* Input object was a Curve. */
        IPFreeObject(PPObj);
}
