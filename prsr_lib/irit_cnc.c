/******************************************************************************
* Irit_Cnc.c - Module to process Code NC files.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by: Gershon Elber				Ver 1.0, Oct 2006.    *
******************************************************************************/

#include <ctype.h>

#include "irit_sm.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "prsr_loc.h"

#define IP_NC_GC_INIT_ARRAY_SIZE	100
#define IP_NC_HELIX_APPROX		25
#define IP_NC_MAX_CRV_LENGTH		100

#define IP_NC_IS_GCODE_LINEAR_MOTION(GC) \
       ((GC) -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST || \
	(GC) -> GCodeType == IP_NC_GCODE_LINE_MOTION_G1LINEAR)

#define IP_NC_IS_GCODE_ARC_MOTION(GC) \
       ((GC) -> GCodeType == IP_NC_GCODE_LINE_MOTION_G2CW || \
	(GC) -> GCodeType == IP_NC_GCODE_LINE_MOTION_G3CCW)

#define IP_NC_INIT_VRTX_FROM_GC(V, Pl, GC) \
	V = IPAllocVertex2(Pl -> PVertex); \
	Pl -> PVertex = V; \
	IRIT_PT_COPY(V -> Coord, GC -> XYZ); \
	IRIT_VEC_COPY(V -> Normal, GC -> IJK); \
	AttrSetRealAttrib(&V -> Attr, "ToolNum", GC -> ToolNumber); \
	AttrSetRealAttrib(&V -> Attr, "FeedRate", GC -> FeedRate); \
	AttrSetRealAttrib(&V -> Attr, "SpindleSpeed", GC -> SpindleSpeed); \
	IP_SET_NORMAL_VRTX(V); 

#define IP_NC_BBOX_UPDATE_FROM_GC(BBox, GC) { \
    int _i; \
	for (_i = 0; _i < 3; _i++) { \
	    if (BBox.Min[_i] > GC -> XYZ[_i]) \
		BBox.Min[_i] = GC -> XYZ[_i]; \
	    if (BBox.Max[_i] < GC -> XYZ[_i] && GC -> XYZ[_i] != IRIT_INFNTY) \
		BBox.Max[_i] = GC -> XYZ[_i]; \
	} \
    }

typedef struct IPNCGCodeAccumArcLenStruct {
    IrtRType TriggerArcLen;  /* Accumulate at least this amount of arc len. */
    IrtRType StartArcLen;     /* Accumulated arc length from starting step. */
    int StartGCodeIndex;	/* Starting GCode from which we accumulate. */
} IPNCGCodeAccumArcLenStruct;

typedef struct IPNCGCodeStreamStruct {
    IPNCGCodeLineStruct CrntState;  /* Currrent state of stream processing. */
    IPNCGCodeLineStruct **GCodes;              /* All the processed stream. */
    int NumOfAllocGCodes;             /* Size of allocated array of GCodes. */
    int NumOfGCodes;                            /* Number of actual GCodes. */
    int CrntStep;			/* Current step of processed GCode. */
    int ArcCentersRelative;/* TRUE if arc centers relative to arc start pt. */
    int ReverseZDir;   /* TRUE to flip Z (when +Z is down - away from tool. */
    /* The following are used in toolpath animation along the path.         */
    IrtRType FastSpeedUpFactor;     /* Speed multiplier for fast G0 motion. */
    IrtRType SlowestFeedRate;         /* Slowest feed rate found in stream. */
    IrtRType CrntTime;            /* Current time of animation, from start. */
    IrtRType CrntArcLen;     /* Current arc length of animation from start. */
    int CrntGCodeIndex; 	     /* Index of currently processed GCode. */
    IrtRType CrntGCodeParam;  /* In [0, 1], tool pos. inside current GCode. */
    IPNCGCodeParserErrorFuncType ErrorFunc;    /* Error reporting function. */
    int TraversalStringIndex; /* Used internally to traverse GCodes' lines. */
    IPNCGCodeAccumArcLenStruct AAL;
} IPNCGCodeStreamStruct;

typedef struct IPNCGCodeLineInfoStruct {
    int HasN;
    int N;

    int HasG;
    int G;

    int HasX, HasY, HasZ;
    IrtRType X, Y, Z;

    int HasI, HasJ, HasK;
    IrtRType I, J, K;

    int HasS, HasF;
    IrtRType S, F;

    int HasR;
    IrtRType R;

    int HasT;
    int T;
} IPNCGCodeLineInfoStruct;

static int IPNCGcodeUpdateLineInfo(char *Line,
				   IPNCGCodeLineInfoStruct *LineInfo);
static IrtRType IPNCArcComputeArcLength(IrtRType *Start,
					IrtRType *Center,
					IrtRType *End,
					int CW);
static CagdCrvStruct *IPNCMakeArc2Helix(CagdCrvStruct *Arc,
					CagdRType Z1,
					CagdRType Z2,
					int Fineness);
static CagdCrvStruct *IPNCCreateArcCCW(CagdPtStruct *PtStart,
				       CagdPtStruct *PtCenter,
				       CagdPtStruct *PtEnd);
static CagdCrvStruct *IPNCCreateArcCW(CagdPtStruct *PtStart,
				      CagdPtStruct *PtCenter,
				      CagdPtStruct *PtEnd);
static void IPNCGSubstituteSubString(char **Line,
				     char GType,
				     const char *NewStr);
static void IPNCGUpdateLines(IPNCGCodeStreamStruct *GCS);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads in a G-code CN file and return it as Irit geometry.                M
*                                                                            *
* PARAMETERS:                                                                M
*   NCGCODEFileName:  G-Code file to read in.				     M
*   ArcCentersRelative:  TRUE for arc center in relative coordinates with    M
*		      respect to arc starting location, FALSE if in          M
*		      absolute coordinates.				     M
*   Messages:         TRUE, for more messages.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Read data or NULL if error.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeLoadFile                                                        M
*****************************************************************************/
IPObjectStruct *IPNCGCodeLoadFile(const char *NCGCODEFileName,
				  int ArcCentersRelative,
				  int Messages)
{
    FILE *f;
    VoidPtr
	GStream = IPNCGCodeParserInit(ArcCentersRelative, 1.0, 1000.0, 1,
				      FALSE, NULL);

    if (GStream != NULL &&
	(f = fopen(NCGCODEFileName, "r")) != NULL) {
        int LineNum = 1;
        char Line[IRIT_LINE_LEN_LONG];
	IPObjectStruct *PObj;

        /* Read the G-Code and process it one line at a time. */
	while (fgets(Line, IRIT_LINE_LEN_VLONG, f) != NULL)
	    IPNCGCodeParserParseLine(GStream, Line, LineNum++);

	if (!feof(f) || !IPNCGCodeParserDone(GStream)) {
	    IPNCGCodeParserFree(GStream);
	    fclose(f);
	    return NULL;
	}
	fclose(f);

	PObj = IPNCGCode2Geometry(GStream);

	IPNCGCodeParserFree(GStream);

	return PObj;
    }

    if (GStream != NULL) {
	IPNCGCodeParserFree(GStream);
	IP_FATAL_ERROR_EX(IP_ERR_FILE_NOT_FOUND, IP_ERR_NO_LINE_NUM,
			  NCGCODEFileName);
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize a new stream of G code to process.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ArcCentersRelative:  TRUE if arc centers (G2/G3) are relative to arc     M
*			 starting location, FALSE if centers are absolute.   M
*   DefFeedRate:         Default feed rate to use if none found.  In units   M
*		         per second.					     M
*   DefSpindleSpeed:     Default spindle speed if none found, in RPM.	     M
*   DefToolNumber:       Default tool number to use if none found.	     M
*   ReverseZDir:         TRUE to reverse Z values.  If FALSE, +Z points up   M
*			 and toward the amchining tool.			     M
*   ErrorFunc:           Call back function in case of errors.		     M
*			 Can be NULL to fully ignore errors.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:           A handle on the G code stream to process.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserDone, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserFree, IPNCGCodeParserSetStep,			     M
*   IPNCGCodeParserGetNext, IPNCGCodeParserNumSteps			     M
*   IPNCGCodeParserGetPrev						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserInit                                                      M
*****************************************************************************/
VoidPtr IPNCGCodeParserInit(int ArcCentersRelative,
			    IrtRType DefFeedRate,
			    IrtRType DefSpindleSpeed,
			    int DefToolNumber,
			    int ReverseZDir,
			    IPNCGCodeParserErrorFuncType ErrorFunc)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *)
				 IritMalloc(sizeof(IPNCGCodeStreamStruct));
    IPNCGCodeLineStruct
        *GC = &GCS -> CrntState;

    /* Reset the current state. */
    GC -> StreamLineNumber = -1;
    GC -> GCodeLineNumber = -1;
    GC -> Line = NULL;
    GC -> GCodeType = IP_NC_GCODE_LINE_NON_MOTION;
    IRIT_PT_RESET(GC -> XYZ);
    GC -> XYZ[2] = IRIT_INFNTY;
    IRIT_VEC_RESET(GC -> IJK);

    GCS -> SlowestFeedRate = IRIT_FABS(DefFeedRate);

    /* Keep the default values negative: */
    GC -> FeedRate = -IRIT_FABS(DefFeedRate);
    GC -> SpindleSpeed = -IRIT_FABS(DefSpindleSpeed);
    GC -> ToolNumber = -IRIT_ABS(DefToolNumber);

    /* Build the array (list) of gcodes to save. */
    GCS -> NumOfAllocGCodes = IP_NC_GC_INIT_ARRAY_SIZE;
    GCS -> NumOfGCodes = 0;
    GCS -> GCodes = (IPNCGCodeLineStruct **)
	IritMalloc(sizeof(IPNCGCodeLineStruct *) * GCS -> NumOfAllocGCodes);
    GCS -> ErrorFunc = ErrorFunc;

    /* Update state. */
    GCS -> ArcCentersRelative = ArcCentersRelative;
    GCS -> ReverseZDir = ReverseZDir;

    return GCS;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Parses the given line and locate all NC GCodes key-chars.                *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:       Line to parse of GCode key characters.                       *
*   LineInfo:   LineInfo structure to update with what we found.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if found valid data in this line, FALSE otherwise (comment!?).*
*****************************************************************************/
static int IPNCGcodeUpdateLineInfo(char *Line,
				   IPNCGCodeLineInfoStruct *LineInfo)
{
    int i, k,
        HasData = FALSE,
	Quit = FALSE;

    IRIT_ZAP_MEM(LineInfo, sizeof(IPNCGCodeLineInfoStruct));

    for (i = 0; Line[i] != 0 && !Quit; i++) {
	switch (Line[i]) {
	    case 'n':
	    case 'N':
	        LineInfo -> HasN = HasData = 
		    sscanf(&Line[i + 1], "%d", &LineInfo -> N) == 1;
		break;
	    case 'g':
	    case 'G':
		if (sscanf(&Line[i + 1], "%d", &k) == 1 &&
		    k >= 0 && k <= 3) { /* G0 - G3 only */
	            LineInfo -> HasG = HasData = TRUE;
		    LineInfo -> G = k;
		}
		break;

	    case 'x':
	    case 'X':
	        LineInfo -> HasX = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> X) == 1;
		break;
	    case 'y':
	    case 'Y':
	        LineInfo -> HasY = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> Y) == 1;
		break;
	    case 'z':
	    case 'Z':
	        LineInfo -> HasZ = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> Z) == 1;
		break;

	    case 'i':
	    case 'I':
	        LineInfo -> HasI = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> I) == 1;
		break;
	    case 'j':
	    case 'J':
	        LineInfo -> HasJ = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> J) == 1;
		break;
	    case 'k':
	    case 'K':
	        LineInfo -> HasK = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> K) == 1;
		break;

	    case 's':
	    case 'S':
	        LineInfo -> HasS = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> S) == 1;
		break;
	    case 'f':
	    case 'F':
	        LineInfo -> HasF = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> F) == 1;
		break;
	    case 't':
	    case 'T':
	        LineInfo -> HasT = HasData =
		    sscanf(&Line[i + 1], "%d", &LineInfo -> T) == 1;
		break;

	    case 'r':
	    case 'R':
	        LineInfo -> HasR = HasData =
		    sscanf(&Line[i + 1], "%lf", &LineInfo -> R) == 1;
		break;

	    case ';': /* Comment chars. */
	    case '(':
	    case '%':
	    case '!':
	    case '#':
	        Quit = TRUE;
		break;

	}
    }

    return HasData;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Process one GCode line as specified by NextLine.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:    Current GCodes' stream.                                   M
*   NextLine:      Next GCode line to process.                               M
*   LineNum:       Line number in the file this line was read from.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  A handle on the G code stream to process                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserDone,				     M
*   IPNCGCodeParserFree, IPNCGCodeParserNumSteps,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserGetPrev						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserParseLine                                                 M
*****************************************************************************/
VoidPtr IPNCGCodeParserParseLine(VoidPtr IPNCGCodes,
				 const char *NextLine,
				 int LineNum)
{
    char *Line;
    int n, i;
    IPNCGCodeLineInfoStruct LineInfo;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct *GC,
        *GCSCrntState = &GCS -> CrntState;

    if (GCS -> NumOfGCodes >= GCS -> NumOfAllocGCodes) {
        /* Reallocated the array of GCodes. */
        n = sizeof(IPNCGCodeLineStruct *) * GCS -> NumOfAllocGCodes;
        GCS -> GCodes = (IPNCGCodeLineStruct **)
					 IritRealloc(GCS -> GCodes, n, n * 2);
	GCS -> NumOfAllocGCodes *= 2;
    }

    GC = GCS -> GCodes[GCS -> NumOfGCodes++] = (IPNCGCodeLineStruct *)
				    IritMalloc(sizeof(IPNCGCodeLineStruct));

    /* Use the current state as a default for this state. */
    IRIT_GEN_COPY(GC, GCSCrntState, sizeof(IPNCGCodeLineStruct));
    GC -> StreamLineNumber = LineNum;

    /* Time to parse this next line. */
    Line = GC -> Line = IritStrdup(NextLine);
    for (i = ((int) strlen(Line)) - 1; i >= 0; i--) {
        if (Line[i] == '\n' || Line[i] == '\r')
	    Line[i] = 0;
	else
	    break;
    }

    GC -> Comment = !IPNCGcodeUpdateLineInfo(Line, &LineInfo);

    /* Do we have a Nxxxx line number? */
    if (LineInfo.HasN)
	GC -> GCodeLineNumber = LineInfo.N;
    else
	GC -> GCodeLineNumber = LineNum;

    /* Do we have a motion Gxxxx line number? Note a line can hold more than */
    /* one G code so we have to examine them all.			     */
    GC -> GCodeType = IP_NC_GCODE_LINE_NONE;
    if (LineInfo.HasG) {
	switch (LineInfo.G) {
	    case 0:
	        GC -> GCodeType = IP_NC_GCODE_LINE_MOTION_G0FAST;
		break;
	    case 1:
	        GC -> GCodeType = IP_NC_GCODE_LINE_MOTION_G1LINEAR;
		break;
	    case 2:
	        GC -> GCodeType = IP_NC_GCODE_LINE_MOTION_G2CW;
		break;
	    case 3:
	        GC -> GCodeType = IP_NC_GCODE_LINE_MOTION_G3CCW;
		break;
        }
    }
    /* If we found no motion G code in this line - use the previous state.  */
    if (GC -> GCodeType == IP_NC_GCODE_LINE_NONE && GCS -> NumOfGCodes > 1)
        GC -> GCodeType = GCSCrntState -> GCodeType;

    if (GCS -> ArcCentersRelative)
        IRIT_PT_RESET(GC -> IJK);	    /* IJK are zero if not defined. */

    /* Do we have a X? */
    if (LineInfo.HasX)
	GC -> XYZ[0] = LineInfo.X;

    /* Do we have a Y? */
    if (LineInfo.HasY)
	GC -> XYZ[1] = LineInfo.Y;

    /* Do we have a Z? */
    if (LineInfo.HasZ)
        GC -> XYZ[2] = GCS -> ReverseZDir ? -LineInfo.Z : LineInfo.Z;

    /* Do we have a I? */
    if (LineInfo.HasI)
	GC -> IJK[0] = LineInfo.I;

    /* Do we have a J? */
    if (LineInfo.HasJ)
	GC -> IJK[1] = LineInfo.J;

    /* Do we have a K? */
    if (LineInfo.HasK)
	GC -> IJK[2] = GCS -> ReverseZDir ? -LineInfo.K : LineInfo.Y;

    /* Do we have a S? (spindle speed). */
    if (LineInfo.HasS) {
        if (LineInfo.S == 0.0) {
	    if (GCS -> ErrorFunc != NULL) {
	        char Line[IRIT_LINE_LEN_LONG];

	        sprintf(Line,
			IRIT_EXP_STR("Zero spindle speed detected in line %d, using 5000 instead"),
			LineNum);
	        GCS -> ErrorFunc(Line);
	    }
	}
	else
	    GC -> SpindleSpeed = LineInfo.S;

	/* Do we have some initial G Codes with no real spindle speed?  If */
	/* so, use this spindle speed to initialize those as well.         */
	if (GCS -> GCodes[0] -> SpindleSpeed < 0.0) {
	    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	        if (GCS -> GCodes[i] -> SpindleSpeed < 0.0) 
		    GCS -> GCodes[0] -> SpindleSpeed = LineInfo.S;
		else
		    break;
	    }
	}
    }

    /* Do we have a R? (arc radius). */
    if (LineInfo.HasR) {
	IrtPtType Inter1Pt, Inter2Pt;

        /* Convert the arc radius to center. */
        if (GCS -> NumOfGCodes >= 2 &&
	    (GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G2CW ||
	     GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G3CCW) &&
	    GM2PointsFromCircCirc(GCSCrntState -> XYZ, LineInfo.R,
				  GC -> XYZ, LineInfo.R,
				  Inter1Pt, Inter2Pt)) {
	    /* The following assumes the arcs are less than 180 degrees. */
	    if (GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G2CW)
	        IRIT_VEC2D_COPY(GC -> IJK, Inter2Pt);
	    else
	        IRIT_VEC2D_COPY(GC -> IJK, Inter1Pt);
	    GC -> IJK[2] = (GCSCrntState -> XYZ[2] + GC -> XYZ[2]) * 0.5;

	    /* Make center relative to start position if so set. */
	    if (GCS -> ArcCentersRelative)
	        IRIT_VEC_SUB(GC -> IJK, GC -> IJK, GCSCrntState -> XYZ);
	}
	else {
	    IP_FATAL_ERROR(IP_ERR_NC_ARC_INVALID_RAD);
	}
    }

    /* Do we have a F? (feed rate). */
    if (LineInfo.HasF) {
        if ((GC -> FeedRate = LineInfo.F) == 0.0) {
	    if (GCS -> ErrorFunc != NULL) {
	        char Line[IRIT_LINE_LEN_LONG];

	        sprintf(Line,
			IRIT_EXP_STR("Zero feed rate detected in line %d, using %f instead"),
			LineNum, GCS -> SlowestFeedRate);
	        GCS -> ErrorFunc(Line);
	    }

	    /* Do not allow zero feed rate - change to slowest so far. */
	    GC -> FeedRate = GCS -> SlowestFeedRate;
	}
	else {
	    GCS -> SlowestFeedRate = IRIT_MIN(GCS -> SlowestFeedRate,
					      LineInfo.F);
	}

	/* Do we have some initial G Codes with no real feedrate?  If so,  */
	/* use this feed rate to initialize those as well.                 */
	if (GCS -> GCodes[0] -> FeedRate < 0.0) {
	    if (GCS -> ErrorFunc != NULL)
	        GCS -> ErrorFunc(IRIT_EXP_STR("No feed rate detected in NC file"));

	    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	        if (GCS -> GCodes[i] -> FeedRate < 0.0) 
		    GCS -> GCodes[i] -> FeedRate = LineInfo.F;
		else
		    break;
	    }
	}
    }

    /* Do we have a T? (tool number). */
    if (LineInfo.HasT) {
	GC -> ToolNumber = n = LineInfo.T;

	/* Do we have some initial G Codes with no real tool number?  If   */
	/* so, use this tool number to initialize those as well.           */
	if (GCS -> GCodes[0] -> ToolNumber < 0) {
	    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	        if (GCS -> GCodes[i] -> ToolNumber < 0) 
		    GCS -> GCodes[0] -> ToolNumber = n;
		else
		    break;
	    }
	}
    }

    GC -> IsVerticalUpMotion =
        IRIT_APX_EQ(GCSCrntState -> XYZ[0], GC -> XYZ[0]) &&
        IRIT_APX_EQ(GCSCrntState -> XYZ[1], GC -> XYZ[1]) &&
        GCSCrntState -> XYZ[2] < GC -> XYZ[2];

    GC -> HasMotion = LineInfo.HasX || LineInfo.HasY || LineInfo.HasZ ||
                      LineInfo.HasI || LineInfo.HasJ || LineInfo.HasK;

    IRIT_GEN_COPY(GCSCrntState, GC, sizeof(IPNCGCodeLineStruct));
    GCSCrntState -> Line = NULL;

    GC -> Len = GC -> LenStart = 0.0;
    GC -> Crv = NULL;

    return GCS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Complete the reading of a new stream of G code.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserFree, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserInit, IPNCGCodeParserSetStep,			     M
*   IPNCGCodeParserGetNext, IPNCGCodeParserNumSteps,			     M
*   IPNCGCodeParserGetPrev						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserDone                                                      M
*****************************************************************************/
int IPNCGCodeParserDone(VoidPtr IPNCGCodes)
{
    char Line[IRIT_LINE_LEN_LONG];
    int i, j;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct
	**GC = GCS -> GCodes;

    if (GCS -> NumOfGCodes < 2)
        return FALSE;

    /* If we have only comments - failed. */
    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	if (!GC[i] -> Comment)
	    break;
    }
    if (i >= GCS -> NumOfGCodes)
        return FALSE;

    /* Use default values of feed rate, spindle speed, and tool number if  */
    /* none were found in the NC file itself.  Negative values signals     */
    /* that no real value was found in the entire read NC stream.	   */
    if (GC[0] -> FeedRate < 0.0) {
        if (GCS -> ErrorFunc != NULL) {
	    sprintf(Line,
		    IRIT_EXP_STR("No feedrate detected in NC file - using default %f"),
		    IRIT_FABS(GC[0] -> FeedRate));
	    GCS -> ErrorFunc(Line);
	}

	for (i = 0; i < GCS -> NumOfGCodes; i++)
	    GC[i] -> FeedRate = IRIT_FABS(GC[i] -> FeedRate);
    }
    if (GC[0] -> SpindleSpeed < 0.0) {
        if (GCS -> ErrorFunc != NULL) {
	    sprintf(Line,
		    IRIT_EXP_STR("No spindle speed detected in NC file - using default %f"),
		    IRIT_FABS(GC[0] -> SpindleSpeed));
	    GCS -> ErrorFunc(Line);
	}

	for (i = 0; i < GCS -> NumOfGCodes; i++)
	    GC[i] -> SpindleSpeed = IRIT_FABS(GC[i] -> SpindleSpeed);
    }
    if (GC[0] -> ToolNumber < 0) {
        if (GCS -> ErrorFunc != NULL) {
	    sprintf(Line,
		    IRIT_EXP_STR("No tool index detected in NC file - using default %d"),
		    IRIT_ABS(GC[0] -> ToolNumber));
	    GCS -> ErrorFunc(Line);
	}
	for (i = 0; i < GCS -> NumOfGCodes; i++)
	    GC[i] -> ToolNumber = IRIT_ABS(GC[i] -> ToolNumber);
    }

    if (GC[0] -> XYZ[2] == IRIT_INFNTY) {
	for (i = 0;
	     i < GCS -> NumOfGCodes && GC[i] -> XYZ[2] == IRIT_INFNTY;
	     i++);

	if (i >= GCS -> NumOfGCodes) {
	    if (GCS -> ErrorFunc != NULL)
	        GCS -> ErrorFunc(IRIT_EXP_STR("No Z values detected in NC file - using 0.0 instead"));

	    i = GCS -> NumOfGCodes - 1;
	    GC[i] -> XYZ[2] = 0.0;
	}

	/* Reset all initial unknown Z values to first detected Z value. */
	for (j = 0; j < i; j++)
	    GC[j] -> XYZ[2] = GC[i] -> XYZ[2];
    }

    if (GC[GCS -> NumOfGCodes - 1] -> GCodeType == IP_NC_GCODE_LINE_NONE ||
	GC[GCS -> NumOfGCodes - 1] -> GCodeType == IP_NC_GCODE_LINE_NON_MOTION) {
	if (GCS -> ErrorFunc != NULL)
	    GCS -> ErrorFunc(IRIT_EXP_STR("No G command detected in file, assume G1 throughout"));

	for (i = 0; i < GCS -> NumOfGCodes; i++)
	    GC[i] -> GCodeType = IP_NC_GCODE_LINE_MOTION_G1LINEAR;
    }
    else if (GC[0] -> GCodeType == IP_NC_GCODE_LINE_NONE ||
	     GC[0] -> GCodeType == IP_NC_GCODE_LINE_NON_MOTION) {
	if (GCS -> ErrorFunc != NULL)
	    GCS -> ErrorFunc(IRIT_EXP_STR("No G command detected in the beginning of file"));
    }

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
        GC[i] -> UpdatedFeedRate = GC[i] -> FeedRate;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the number of GCode steps in the parsed stream IPNCGCodes.       M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Number of GCode steps in IPNCGCodes.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserFree, IPNCGCodeParserGetPrev				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserNumSteps                                                  M
*****************************************************************************/
int IPNCGCodeParserNumSteps(VoidPtr IPNCGCodes)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    return GCS -> NumOfGCodes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the current GCode step in the parsed stream IPNCGCodes.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*   NewStep:	 The new current step to set to.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPNCGCodeLineStruct *:  This step's GCode parsed state in the stream.    M
*		NULL if outside the range of stream.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserFree, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserNumSteps, IPNCGCodeParserGetPrev			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserSetStep                                                   M
*****************************************************************************/
IPNCGCodeLineStruct *IPNCGCodeParserSetStep(VoidPtr IPNCGCodes,
					    int NewStep)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    if (NewStep >= GCS -> NumOfGCodes || NewStep < 0)
        return NULL;

    GCS -> CrntStep = NewStep;

    return GCS -> GCodes[GCS -> CrntStep];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the next GCode line in the parsed stream IPNCGCodes.             M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPNCGCodeLineStruct *:  The next GCode parsed state in the stream.       M
*		NULL if end of stream.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserFree,			     M
*   IPNCGCodeParserNumSteps, IPNCGCodeParserGetPrev			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserGetNext                                                   M
*****************************************************************************/
IPNCGCodeLineStruct *IPNCGCodeParserGetNext(VoidPtr IPNCGCodes)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    if (GCS -> NumOfGCodes > GCS -> CrntStep)
	return GCS -> GCodes[GCS -> CrntStep++];
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the previous GCode line in the parsed stream IPNCGCodes.         M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPNCGCodeLineStruct *:  The previous GCode parsed state in the stream.   M
*		NULL if beginning of stream.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserNumSteps, IPNCGCodeParserFree			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserGetPrev                                                   M
*****************************************************************************/
IPNCGCodeLineStruct *IPNCGCodeParserGetPrev(VoidPtr IPNCGCodes)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    if (GCS -> CrntStep > 0)
	return GCS -> GCodes[--GCS -> CrntStep];
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free a processed stream of G codes.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:   G codes' stream to free.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserNumSteps, IPNCGCodeParserGetPrev			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeParserFree                                                      M
*****************************************************************************/
void IPNCGCodeParserFree(VoidPtr IPNCGCodes)
{
    int i;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
        IritFree(GCS -> GCodes[i] -> Line);
        CagdCrvFree(GCS -> GCodes[i] -> Crv);
	IritFree(GCS -> GCodes[i]);
    }
    IritFree(GCS -> GCodes);
    IritFree(GCS);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the expected arc length of the arc from Start to End with       *
* center point Center.  If Start == End, a full circle is assumed.           *
*                                                                            *
* PARAMETERS:                                                                *
*   Start:   Location of arc.				                     *
*   Center:  Location of arc.				                     *
*   End:     Location of arc.				                     *
*   CCW:      TRUE for a CCW arc, FALSE for a CW arc.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Computed arc length.                                         *
*****************************************************************************/
static IrtRType IPNCArcComputeArcLength(IrtRType *Start,
					IrtRType *Center,
					IrtRType *End,
					int CW)
{
    CagdRType
	Rad = IRIT_PT_PT_DIST(Start, Center),
	StartAngle = atan2(Start[1] - Center[1], Start[0] - Center[0]),
	EndAngle = atan2(End[1] - Center[1], End[0] - Center[0]);

    if (IRIT_APX_EQ_EPS(EndAngle, StartAngle, IRIT_UEPS))
        return 2.0 * M_PI * Rad;

    if (EndAngle < StartAngle)
        EndAngle += M_PI * 2.0;

    if (CW) {
	return (2.0 * M_PI - (EndAngle - StartAngle)) * Rad;
    }
    else {
	return (EndAngle - StartAngle) * Rad;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Approximates a given arc into a helix.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Arc:         2D Arc to approximate into a 3D helix.                      *
*   Z1, Z2:      Initial and final Z levels of the created helix.            *
*   Fineness:    A hint how much to refine the arc.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:    Approximated Helix.                                  *
*****************************************************************************/
static CagdCrvStruct *IPNCMakeArc2Helix(CagdCrvStruct *Arc,
					CagdRType Z1,
					CagdRType Z2,
					int Fineness)
{
    int i, Len;
    CagdRType TMin, TMax, Dt, t, *ZPts, *WPts,
        *Params = (CagdRType *) IritMalloc(sizeof(CagdRType) * Fineness);
    CagdCrvStruct *Helix;

    if (CAGD_IS_BEZIER_CRV(Arc))
	Arc = CagdCnvrtBzr2BspCrv(Arc);
    else
        Arc = CagdCrvCopy(Arc);

    CagdCrvDomain(Arc, &TMin, &TMax);
    Dt = (TMax - TMin) / (Fineness + 1);
    for (t = TMin + Dt, i = 0; i < Fineness; i++, t += Dt)
	Params[i] = t;

    Helix = CagdCrvRefineAtParams(Arc, FALSE, Params, Fineness);

    ZPts = Helix -> Points[3];
    WPts = Helix -> Points[0];
    Len = Helix -> Length;

    IritFree(Params);
    Params = CagdCrvNodes(Helix);

    for (i = 0; i < Len; i++)
        *ZPts++ = *WPts++ * (Z1 + (Z2 - Z1) * Params[i]);

    IritFree(Params);
    CagdCrvFree(Arc);

    return Helix;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmpute a CCW arc from PtStart to PtEnd, cented at PtCenter.  The arc    *
* can have different Z values, in which case a helix is approximated.        *
*                                                                            *
* PARAMETERS:                                                                *
*   PtStart:  First end point of arc.                                        *
*   PtCenter: Center of arc.		                                     *
*   PtEnd:    Last end point of arc.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Created CCW arc.                                       *
*****************************************************************************/
static CagdCrvStruct *IPNCCreateArcCCW(CagdPtStruct *PtStart,
				       CagdPtStruct *PtCenter,
				       CagdPtStruct *PtEnd)
{
    if (IRIT_APX_EQ(PtStart -> Pt[2], PtEnd -> Pt[2]))
        return CagdCrvCreateArcCCW(PtStart, PtCenter, PtEnd);
    else {
        CagdCrvStruct *Arc, *Helix;
	CagdRType
	    ZStart = PtStart -> Pt[2],
	    ZCntr = PtCenter -> Pt[2],
	    ZEnd = PtEnd -> Pt[2];

	PtStart -> Pt[2] = PtCenter -> Pt[2] = PtEnd -> Pt[2] = 0.0;
	Arc = CagdCrvCreateArcCCW(PtStart, PtCenter, PtEnd);
	
	PtStart -> Pt[2] = ZStart;
	PtCenter -> Pt[2] = ZCntr;
	PtEnd -> Pt[2] = ZEnd;

	Helix = IPNCMakeArc2Helix(Arc, ZStart, ZEnd, IP_NC_HELIX_APPROX);
	CagdCrvFree(Arc);
	return Helix;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   COmpute a CW arc from PtStart to PtEnd, cented at PtCenter.  The arc     *
* can have different Z values, in which case a helix is approximated.        *
*                                                                            *
* PARAMETERS:                                                                *
*   PtStart:  First end point of arc.                                        *
*   PtCenter: Center of arc.		                                     *
*   PtEnd:    Last end point of arc.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Created CW arc.                                        *
*****************************************************************************/
static CagdCrvStruct *IPNCCreateArcCW(CagdPtStruct *PtStart,
				      CagdPtStruct *PtCenter,
				      CagdPtStruct *PtEnd)
{
    if (IRIT_APX_EQ(PtStart -> Pt[2], PtEnd -> Pt[2]))
        return CagdCrvCreateArcCW(PtStart, PtCenter, PtEnd);
    else {
        CagdCrvStruct *Arc, *Helix;
	CagdRType
	    ZStart = PtStart -> Pt[2],
	    ZCntr = PtCenter -> Pt[2],
	    ZEnd = PtEnd -> Pt[2];

	PtStart -> Pt[2] = PtCenter -> Pt[2] = PtEnd -> Pt[2] = 0.0;
	Arc = CagdCrvCreateArcCW(PtStart, PtCenter, PtEnd);
	
	PtStart -> Pt[2] = ZStart;
	PtCenter -> Pt[2] = ZCntr;
	PtEnd -> Pt[2] = ZEnd;

	Helix = IPNCMakeArc2Helix(Arc, ZStart, ZEnd, IP_NC_HELIX_APPROX);
	CagdCrvFree(Arc);
	return Helix;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert the given stream of G Code toolpath to IRIT geometry.	     M
*   Every linear motion is considered one polyline as long as it is either   M
* G0 or G1.  Free-air motion (G0) is marked with attribute "freemotion".     M
* "ToolNum", "SpindleSpeed" and "SpindleSpeed" attributes are saved on every M
* vertex.								     M
*   As a side effect, computes the length (Len slot) of each G Code motion,  M
* and a curve representation (Crv slot).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:  Current GCodes' stream.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The toolpath in IRIT form.  			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeParserNumSteps, IPNCGCodeParserFree,			     M
*   IPNCGCodeLength, IPNCGCodeBBox					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCode2Geometry                                                       M
*****************************************************************************/
IPObjectStruct *IPNCGCode2Geometry(VoidPtr IPNCGCodes)
{
    int i;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct
	*LastGC = NULL;
    IPNCGCodeLineType
	GCodeType = GCS -> GCodes[0] -> GCodeType;
    IPObjectStruct *PObj,
	*PGeometry = IPGenLISTObject(NULL);
    IPPolygonStruct *Pl2,
	*Pl = IPAllocPolygon(0, NULL, NULL);
    IPVertexStruct *V;
    CagdCrvStruct
	*LastCrv = NULL;

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	IPNCGCodeLineStruct
	    *GC = GCS -> GCodes[i];

	if (GC -> Comment || !GC -> HasMotion)
	    continue;

	if (GCodeType != GC -> GCodeType &&
	    (GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST ||
	     GCodeType == IP_NC_GCODE_LINE_MOTION_G1LINEAR)) {
	    assert(Pl != NULL && Pl -> PVertex != NULL);
	    /* Copy last vertex (note last vertex is pushed first). */
	    Pl2 = IPAllocPolygon(0, IPCopyVertex(Pl -> PVertex), NULL);

	    /* Save the polyline so far in the output geometry. Call twice  */
	    /* as we might delete duplicates in first round and have only   */
	    /* one vertex left for the second round...                      */
	    if ((Pl = GMCleanUpPolylineList(&Pl, IRIT_UEPS)) != NULL &&
		(Pl = GMCleanUpPolylineList(&Pl, IRIT_UEPS)) != NULL) {
	        Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);
		PObj = IPGenPOLYLINEObject(Pl);
		if (GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST) {
		    AttrSetObjectIntAttrib(PObj, "FreeMotion", TRUE);
		    AttrSetObjectRGBColor(PObj, 0, 255, 0);
		}
		else {
		    AttrSetObjectRGBColor(PObj, 255, 0, 0);
		}
		IPListObjectAppend(PGeometry, PObj);
	    }

	    Pl = Pl2;
	}

        switch (GC -> GCodeType) {
	    case IP_NC_GCODE_LINE_MOTION_G0FAST:
	    case IP_NC_GCODE_LINE_MOTION_G1LINEAR:
	        if (LastCrv != NULL) {  /* Save arcs we generated until now. */
		    PObj = IPGenCRVObject(LastCrv);
		    AttrSetObjectRGBColor(PObj, 255, 0, 0);
		    IPListObjectAppend(PGeometry, PObj);
		    LastCrv = NULL;

		    /* Update the position in the first and only vertex of   */
		    /* Pl, if any, with the end position of the last arc.    */
		    assert(Pl != NULL);
		    assert(IP_NC_IS_GCODE_ARC_MOTION(LastGC));
		    if (Pl -> PVertex != NULL) {
		        /* If has one, should have only one. */
		        assert(Pl -> PVertex -> Pnext == NULL);
			IPFreeVertex(Pl -> PVertex);
			Pl -> PVertex = NULL;
		    }
		    IP_NC_INIT_VRTX_FROM_GC(V, Pl, LastGC);
		}
		IP_NC_INIT_VRTX_FROM_GC(V, Pl, GC);
	        if (LastGC != NULL &&
		    !IRIT_PT_APX_EQ_EPS(LastGC -> XYZ, GC -> XYZ, IRIT_UEPS)) {
		    CagdPtStruct PtStart, PtEnd;

		    IRIT_PT_COPY(PtStart.Pt, LastGC -> XYZ);
		    IRIT_PT_COPY(PtEnd.Pt, GC -> XYZ);
		    GC -> Crv = CagdMergePtPt(&PtStart, &PtEnd);
		    GC -> Len = IRIT_PT_PT_DIST(GC -> XYZ, LastGC -> XYZ);
		}
		break;
	    case IP_NC_GCODE_LINE_MOTION_G2CW:
	    case IP_NC_GCODE_LINE_MOTION_G3CCW:
	        if (LastGC != NULL) {
		    CagdCrvStruct *Arc, *TCrv;
		    CagdPtStruct PtStart, PtCenter, PtEnd;

		    /* Create the arc, assuming it is less than 360 degrees. */
		    IRIT_PT_COPY(PtStart.Pt, LastGC -> XYZ);
		    IRIT_PT_COPY(PtEnd.Pt, GC -> XYZ);
		    IRIT_PT_COPY(PtCenter.Pt, GC -> IJK);
		    if (GCS -> ArcCentersRelative) {
			IRIT_PT_ADD(PtCenter.Pt, PtCenter.Pt, PtStart.Pt);
		    }
		    PtCenter.Pt[2] = PtStart.Pt[2];
		    if (GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G2CW)
		        Arc = IPNCCreateArcCW(&PtStart, &PtCenter, &PtEnd);
		    else
		        Arc = IPNCCreateArcCCW(&PtStart, &PtCenter, &PtEnd);
		    if (Arc != NULL) {
			if (CAGD_IS_BSPLINE_CRV(Arc)) {
			    BspKnotAffineTransOrder2(Arc -> KnotVector,
						     Arc -> Order,
						     Arc -> Order + Arc -> Length,
						     0.0, 1.0);
			}
			GC -> Crv = CagdCrvCopy(Arc);
			GC -> Len =
			    IPNCArcComputeArcLength(PtStart.Pt, PtCenter.Pt,
					PtEnd.Pt,
					GC -> GCodeType == 
					   IP_NC_GCODE_LINE_MOTION_G2CW);

			/* Hook the new arc to the accumulated curve so far. */
			if (LastCrv == NULL)
			    LastCrv = Arc;
			else {
			    TCrv = CagdMergeCrvCrv(LastCrv, Arc, FALSE);
			    CagdCrvFree(LastCrv);
			    CagdCrvFree(Arc);
			    LastCrv = TCrv;
			    if (LastCrv -> Length > IP_NC_MAX_CRV_LENGTH) {
				PObj = IPGenCRVObject(LastCrv);
				AttrSetObjectRGBColor(PObj, 255, 0, 0);
				IPListObjectAppend(PGeometry, PObj);
				LastCrv = NULL;
			    }
			}
		    }
		}
		break;
	    default:
		break;
        }

	GCodeType = GC -> GCodeType;
	LastGC = GC;
    }

    if (Pl != NULL &&				    /* Save last polyline. */
	(Pl = GMCleanUpPolylineList(&Pl, IRIT_UEPS)) != NULL) {
	Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);
        PObj = IPGenPOLYLINEObject(Pl);
	if (GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST) {
	    AttrSetObjectIntAttrib(PObj, "FreeMotion", TRUE);
	    AttrSetObjectRGBColor(PObj, 0, 255, 0);
	}
	else {
	    AttrSetObjectRGBColor(PObj, 255, 0, 0);
	}
	IPListObjectAppend(PGeometry, PObj);
    }

    if (LastCrv != NULL) {			        /* Save last arcs. */
	PObj = IPGenCRVObject(LastCrv);
	AttrSetObjectRGBColor(PObj, 255, 0, 0);
	IPListObjectAppend(PGeometry, PObj);
    }

    return IPReverseObjList(PGeometry);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the accumulated arc length of the given stream of G Code        M
* toolpath, in cutting speed motion.					     M
*   Assumes IPNCGCode2Geometry was invoked on this stream to compute each    M
* G code individual arc length.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:    Current GCodes' stream.                                   M
*   FastLength:    Accumulate fast motion length here, if non NULL.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Computed arc length.		  			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeBBox, IPNCGCode2Geometry, IPNCGCodeParserNumSteps,		     M
*   IPNCGCodeParserFree							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeLength                                                          M
*****************************************************************************/
IrtRType IPNCGCodeLength(VoidPtr IPNCGCodes, IrtRType *FastLength)
{
    int i;
    IrtRType
	Len = 0.0;
    IPNCGCodeLineStruct
        *LastGC = NULL;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    if (FastLength != NULL)
        *FastLength = 0.0;

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	IPNCGCodeLineStruct
	    *GC = GCS -> GCodes[i];

	if (GC -> Comment)
	    continue;

	GC -> LenStart = Len;

        switch (GC -> GCodeType) {
	    case IP_NC_GCODE_LINE_MOTION_G0FAST:
	        if (FastLength != NULL && LastGC != NULL)
		    *FastLength += GC -> Len;
		break;
	    case IP_NC_GCODE_LINE_MOTION_G1LINEAR:
	    case IP_NC_GCODE_LINE_MOTION_G2CW:
	    case IP_NC_GCODE_LINE_MOTION_G3CCW:
	        Len += GC -> Len;
		break;
	    case IP_NC_GCODE_LINE_NONE:
		break;
	    default:
	        assert(0);;
        }
	LastGC = GC;
    }

    return Len;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a bounding box over the coordinates found in the G Code stream. M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:    Current GCodes' stream.                                   M
*   IgnoreG0Fast:    If TRUE, ignore motions with G0's in bbox computaton.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:  The computed bbox returned in a static memory area.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeLength,  IPNCGCodeParserNumSteps, IPNCGCodeParserFree           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeBBox                                                            M
*****************************************************************************/
GMBBBboxStruct *IPNCGCodeBBox(VoidPtr IPNCGCodes, int IgnoreG0Fast)
{
    IRIT_STATIC_DATA GMBBBboxStruct BBox;
    int i;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct
        *LastGC = NULL;
    BBox.Min[0] = BBox.Min[1] = BBox.Min[2] = IRIT_INFNTY;
    BBox.Max[0] = BBox.Max[1] = BBox.Max[2] = -IRIT_INFNTY;

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	IPNCGCodeLineStruct
	    *GC = GCS -> GCodes[i];

	if (GC -> Comment)
	    continue;

        switch (GC -> GCodeType) {
	    case IP_NC_GCODE_LINE_MOTION_G0FAST:
	        if (LastGC != NULL &&
		    LastGC -> GCodeType != IP_NC_GCODE_LINE_NON_MOTION &&
		    LastGC -> GCodeType != IP_NC_GCODE_LINE_MOTION_G0FAST)
		    IP_NC_BBOX_UPDATE_FROM_GC(BBox, LastGC);
	        if (IgnoreG0Fast)
		    break;
	    case IP_NC_GCODE_LINE_MOTION_G1LINEAR:
	        if (LastGC != NULL &&
		    LastGC -> GCodeType != IP_NC_GCODE_LINE_NON_MOTION)
		    IP_NC_BBOX_UPDATE_FROM_GC(BBox, LastGC);
		IP_NC_BBOX_UPDATE_FROM_GC(BBox, GC);
		break;
	    case IP_NC_GCODE_LINE_MOTION_G2CW:
	    case IP_NC_GCODE_LINE_MOTION_G3CCW:
	        /* We only consider the end points of the arc. */
		if (LastGC != NULL)
		    IP_NC_BBOX_UPDATE_FROM_GC(BBox, LastGC);
		IP_NC_BBOX_UPDATE_FROM_GC(BBox, GC);
		break;
	    default:
	        assert(0);;
        }
	LastGC = GC;
    }

    for (i = 0; i < 3; i++) {
        if (BBox.Min[i] == IRIT_INFNTY)
	    BBox.Min[i] = 0.0;
        if (BBox.Max[i] == -IRIT_INFNTY)
	    BBox.Max[i] = 0.0;
    }

    return &BBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Init a request to traverse this sequence of G Code.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:         G Code sequence to traverse.                         M
*   InitTime:           Initial time of animation.			     M
*   FastSpeedUpFactor:  Speedup multiplier for fast G0 motion.		     M
*   TriggerArcLen:      A minimal arc length to accumulate to create         M
*                       triggers every such arc length,			     M
*                       or non-positive value to ignore.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Total arc-length of cutting speed motion                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeLength, IPNCGCodeTraverseTime, IPNCGCodeTraverseTriggerAAL      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeTraverseInit                                                    M
*****************************************************************************/
IrtRType IPNCGCodeTraverseInit(VoidPtr IPNCGCodes,
			       IrtRType InitTime,
			       IrtRType FastSpeedUpFactor,
			       IrtRType TriggerArcLen)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IrtRType FastLen,
        Len = IPNCGCodeLength(IPNCGCodes, &FastLen);

    GCS -> FastSpeedUpFactor = FastSpeedUpFactor;
    GCS -> CrntTime = 0.0;
    GCS -> CrntArcLen = 0.0;
    GCS -> CrntGCodeIndex = 0;
    GCS -> CrntGCodeParam = 0.0;

    GCS -> AAL.StartGCodeIndex = 0;
    GCS -> AAL.StartArcLen = 0.0;
    GCS -> AAL.TriggerArcLen = TriggerArcLen;

    return Len + FastLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle accumulations of arc-length (AAL). Tests and generates triggers   M
* every prescribed arc length.				                     M
*   If a trigger is generated and EvalMRR is not NULL, this function is      M
* invoked to evaluate the material removal rate in this arc length interval. M
* EvalMRR should return a 1.0 if the feed rate is appropriate and return a   M
* multiplicative factor to modify the feed rate if not.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:   G Code sequence to traverse.                               M
*   EvalMRR:      Function to invoke to compute the material removal rate in M
*                 this arclen interval, or NULL to ignore.  This function    M
*                 should return 1.0 if feedrate is fine or a multiplicative  M
*                 factor to modify the feedrate otherwise.                   M
*   MRRData:      A pointer to pass to EWvalMRR as its single parameter.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if generated a trigger, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeLength, IPNCGCodeTraverseInit, IPNCGCodeTraverseStep            M
*   IPNCGCodeTraverseTime						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeTraverseTriggerAAL                                              M
*****************************************************************************/
int IPNCGCodeTraverseTriggerAAL(VoidPtr IPNCGCodes,
				IPNCGCodeEvalMRRFuncType EvalMRR,
				VoidPtr MRRData)
{
    IrtRType RelFeed;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    /* No handling to accumulation of arc-length. */
    if (GCS -> AAL.TriggerArcLen <= 0.0)
        return FALSE;

    /* Did not accumulate enough arc-length. */
    if (GCS -> AAL.TriggerArcLen < IRIT_FABS(GCS -> AAL.StartArcLen - 
					     GCS -> CrntArcLen))
        return FALSE;

    /* If we are asked to evaluate material removal rates and update the   */
    /* feedrates, we should do it now.					   */
    if (EvalMRR != NULL && (RelFeed = EvalMRR(MRRData)) != 1.0) {
        int i,
	    MinIndex = IRIT_MIN(GCS -> AAL.StartGCodeIndex,
				GCS -> CrntGCodeIndex),
	    MaxIndex = IRIT_MAX(GCS -> AAL.StartGCodeIndex,
				GCS -> CrntGCodeIndex);

        for (i = MinIndex; i <= MaxIndex; i++) {
	    IPNCGCodeLineStruct
	        *GC = GCS -> GCodes[i];

	    GC -> UpdatedFeedRate =
	        IRIT_MIN(GC -> UpdatedFeedRate, GC -> FeedRate * RelFeed);
	}
    }

    GCS -> AAL.StartArcLen = GCS -> CrntArcLen;
    GCS -> AAL.StartGCodeIndex = GCS -> CrntGCodeIndex;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the index of the current G code to a new non comment, valid line.*
*                                                                            *
* PARAMETERS:                                                                *
*   GCS:        G Code sequence we traverse.                                 *
*   NewIndex:   New index to assign.                                         *
*   Forward:    TRUE to shift forward until a non-comment, false backward.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    New assigned index.                                              *
*****************************************************************************/
static int IPNCGCodeUpdateGCodeIndex(IPNCGCodeStreamStruct *GCS,
				     int NewIndex,
				     int Forward)
{
    GCS -> CrntGCodeIndex = NewIndex;

    if (Forward) {        /* Skip forward until a non comment is detected. */
        while (GCS -> CrntGCodeIndex < GCS -> NumOfGCodes - 1 &&
	       GCS -> GCodes[GCS -> CrntGCodeIndex] -> Comment)
	    GCS -> CrntGCodeIndex++;
    }
    else {               /* Skip backward until a non comment is detected. */
        while (GCS -> CrntGCodeIndex > 0 &&
	       GCS -> GCodes[GCS -> CrntGCodeIndex] -> Comment)
	    GCS -> CrntGCodeIndex--;
    }

    return GCS -> CrntGCodeIndex;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Advance the animated tool motion along this sequence of G Code to Time.  M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:       G Code sequence to traverse.                           M
*   Dt:               Delta time step to add/subtract from current time.     M
*                     Can be negative to subtract and go backward.	     M
*   NewRealTime:      New real time of new position is returned here with    M
*		      respect to starting time (that is 0.0).		     M
*   NewToolPosition:  The traversed tool position is saved here.	     M
*   NewGC:            GCode info at NewToolPosition.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   New arc-length of cutting speed motion so far.		     M
*		-1.0 is returned if we completed the traversal,		     M
*		-2.0 is returned if we when back to the start point.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeLength, IPNCGCodeTraverseInit, IPNCGCodeTraverseStep            M
*   IPNCGCodeTraverseTriggerAAL						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeTraverseTime                                                    M
*****************************************************************************/
IrtRType IPNCGCodeTraverseTime(VoidPtr IPNCGCodes,
			       IrtRType Dt,
			       IrtRType *NewRealTime,
			       IrtPtType NewToolPosition,
			       IPNCGCodeLineStruct **NewGC)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct *GC;

    if (GCS -> CrntGCodeIndex >= GCS -> NumOfGCodes) {
        if (Dt >= 0.0) {
	    *NewGC = GCS -> GCodes[GCS -> NumOfGCodes - 1];
	    IRIT_PT_COPY(NewToolPosition, (*NewGC) -> XYZ);
	    *NewRealTime = GCS -> CrntTime;
	    return -1.0;
	}
	else {
	    IPNCGCodeUpdateGCodeIndex(GCS, GCS -> NumOfGCodes - 1, FALSE);
	    GCS -> CrntGCodeParam = 1.0;
	}
    }
    else if (GCS -> CrntGCodeIndex < 0) {
	if (Dt <= 0.0) {
	    *NewGC = GCS -> GCodes[0];
	    IRIT_PT_COPY(NewToolPosition, (*NewGC) -> XYZ);
	    *NewRealTime = 0.0;
	    return -2.0;
	}
	else {
	    IPNCGCodeUpdateGCodeIndex(GCS, 0, TRUE);
	    GCS -> CrntGCodeParam = 0.0;
	}
    }

    GC = GCS -> GCodes[GCS -> CrntGCodeIndex];

    if (Dt >= 0.0) {
        do {
	    /* Compute time left needed to completely traverse this GC. */
	    IrtRType *R,
	        FeedRate =
	            GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST ?
			GCS -> FastSpeedUpFactor * GC -> UpdatedFeedRate :
		        GC -> UpdatedFeedRate,
	        LeftLen = (1.0 - GCS -> CrntGCodeParam) * GC -> Len,
	        LeftTime = LeftLen / FeedRate;

	    if (LeftTime >= Dt) {   /* For Dt time, we stay inside this GC. */
	        GCS -> CrntGCodeParam += (1.0 - GCS -> CrntGCodeParam)
							      * Dt / LeftTime;
		assert(IRIT_FABS(Dt) < 10000);
		GCS -> CrntTime += Dt;
		GCS -> CrntArcLen += Dt * FeedRate;

		Dt = 0.0;
		R = CagdCrvEval(GC -> Crv, GCS -> CrntGCodeParam);
		CagdCoerceToE3(NewToolPosition, &R, -1, GC -> Crv -> PType);
	    }
	    else {	         /* For Dt time, we must go beyond this GC. */
	        if (IPNCGCodeUpdateGCodeIndex(GCS, GCS -> CrntGCodeIndex + 1,
					      TRUE) >= GCS -> NumOfGCodes) {
		    /* We are at the end of the tool path! */
		    IRIT_PT_COPY(NewToolPosition, GC -> XYZ);
		    *NewRealTime = GCS -> CrntTime;
		    *NewGC = GCS -> GCodes[GCS -> NumOfGCodes - 1];
		    return -1.0;
		}
		assert(IRIT_FABS(LeftTime) < 10000);
		GCS -> CrntTime += LeftTime;
		GCS -> CrntArcLen += LeftLen;
		Dt -= LeftTime;
		GCS -> CrntGCodeParam = 0.0;
		GC = GCS -> GCodes[GCS -> CrntGCodeIndex];
	    }
	}
	while (Dt > 0.0);
    }
    else {			 /* Moving backward in time, negative Dt... */
        do {
	    /* Compute time left needed to completely traverse this GC. */
	    IrtRType *R,
	        FeedRate =
	            GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST ?
			GCS -> FastSpeedUpFactor * GC -> UpdatedFeedRate :
		        GC -> UpdatedFeedRate,
	        LeftLen = GCS -> CrntGCodeParam * GC -> Len,
	        LeftTime = LeftLen / FeedRate;

	    if (GC -> Len > 0.0 && LeftTime >= -Dt) {
	        /* For -Dt time, we stay inside this GC. */
	        GCS -> CrntGCodeParam -= GCS -> CrntGCodeParam
							     * -Dt / LeftTime;
		assert(IRIT_FABS(Dt) < 10000);
		GCS -> CrntTime += Dt;
		GCS -> CrntArcLen += Dt * FeedRate;

		Dt = 0.0;
		R = CagdCrvEval(GC -> Crv, GCS -> CrntGCodeParam);
		CagdCoerceToE3(NewToolPosition, &R, -1, GC -> Crv -> PType);
	    }
	    else {	         /* For -Dt time, we must go below this GC. */
	        if (IPNCGCodeUpdateGCodeIndex(GCS, GCS -> CrntGCodeIndex - 1,
					      FALSE) < 0) {
		    /* We are at the beginning of the tool path! */
		    IRIT_PT_COPY(NewToolPosition, GC -> XYZ);
		    *NewRealTime = 0.0;
		    *NewGC = GCS -> GCodes[0];
		    return -2.0;
		}
		assert(IRIT_FABS(LeftTime) < 10000);
		GCS -> CrntTime -= LeftTime;
		GCS -> CrntArcLen -= LeftLen;
		Dt += LeftTime;
		GCS -> CrntGCodeParam = 1.0;
		GC = GCS -> GCodes[GCS -> CrntGCodeIndex];
	    }
	}
	while (Dt < 0.0);
    }

    *NewRealTime = GCS -> CrntTime;
    *NewGC = GC;
    return GCS -> CrntArcLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Advance the animated tool motion along this sequence of G Code by Step.  M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:       G Code sequence to traverse.                           M
*   Step:             Delta position step to add/subtract from current pos.  M
*                     Can be negative to subtract and go backward.	     M
*   NewRealTime:      New real time of new position is returned here with    M
*		      respect to starting time (that is 0.0).		     M
*   NewToolPosition:  The traversed tool position is saved here.	     M
*   NewGC:            GCode info at NewToolPosition.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   New arc-length of cutting speed motion so far.		     M
*		-1.0 is returned if we completed the traversal,		     M
*		-2.0 is returned if we when back to the start point.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeLength, IPNCGCodeTraverseInit, IPNCGCodeTraverseTime            M
*   IPNCGCodeTraverseTriggerAAL						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeTraverseStep                                                    M
*****************************************************************************/
IrtRType IPNCGCodeTraverseStep(VoidPtr IPNCGCodes,
			       IrtRType Step,
			       IrtRType *NewRealTime,
			       IrtPtType NewToolPosition,
			       IPNCGCodeLineStruct **NewGC)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    IPNCGCodeLineStruct *GC;

    if (GCS -> CrntGCodeIndex >= GCS -> NumOfGCodes) {
	if (Step >= 0.0) {
	    *NewGC = GCS -> GCodes[GCS -> NumOfGCodes - 1];
	    IRIT_PT_COPY(NewToolPosition, (*NewGC) -> XYZ);
	    *NewRealTime = GCS -> CrntTime;
	    return -1.0;
	}
	else {
	    IPNCGCodeUpdateGCodeIndex(GCS, GCS -> NumOfGCodes - 1, FALSE);
	    GCS -> CrntGCodeParam = 1.0;
	}
    }
    else if (GCS -> CrntGCodeIndex < 0) {
        if (Step <= 0.0) {
	    *NewGC = GCS -> GCodes[0];
	    IRIT_PT_COPY(NewToolPosition, (*NewGC) -> XYZ);
	    *NewRealTime = 0.0;
	    return -2.0;
	}
	else {
	    IPNCGCodeUpdateGCodeIndex(GCS, 0, TRUE);
	    GCS -> CrntGCodeParam = 0.0;
	}
    }

    GC = GCS -> GCodes[GCS -> CrntGCodeIndex];
    if (GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST)
	Step *= GCS -> FastSpeedUpFactor;

    if (Step >= 0.0) {
        do {
	    /* Compute time left needed to completely traverse this GC. */
	    IrtRType *R, Dt,
	        FeedRate =
	            GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST ?
		        GCS -> FastSpeedUpFactor * GC -> UpdatedFeedRate :
		        GC -> UpdatedFeedRate,
	        LeftLen = (1.0 - GCS -> CrntGCodeParam) * GC -> Len,
	        LeftTime = LeftLen / FeedRate;

	    if (GC -> Len > 0.0 && LeftLen >= Step) {
	        /* For Step, we stay inside this GC. */
	        Dt = Step / FeedRate;
	        GCS -> CrntGCodeParam += (1.0 - GCS -> CrntGCodeParam)
							      * Step / LeftLen;
		GCS -> CrntTime += Dt;
		GCS -> CrntArcLen += Step;

		Step = 0.0;
		R = CagdCrvEval(GC -> Crv, GCS -> CrntGCodeParam);
		CagdCoerceToE3(NewToolPosition, &R, -1, GC -> Crv -> PType);
	    }
	    else {	            /* For Step, we must go beyond this GC. */
	        if (IPNCGCodeUpdateGCodeIndex(GCS, GCS -> CrntGCodeIndex + 1,
					      TRUE) >= GCS -> NumOfGCodes) {
		    /* We are at the end of the tool path! */
		    IRIT_PT_COPY(NewToolPosition, GC -> XYZ);
		    *NewRealTime = GCS -> CrntTime;
		    *NewGC = GCS -> GCodes[GCS -> NumOfGCodes - 1];
		    return -1.0;
		}
		GCS -> CrntTime += LeftTime;
		GCS -> CrntArcLen += LeftLen;
		Step -= LeftLen;
		GCS -> CrntGCodeParam = 0.0;
		GC = GCS -> GCodes[GCS -> CrntGCodeIndex];
	    }
	}
	while (Step > 0.0);
    }
    else {			 /* Moving backward in time, negative Dt... */
        do {
	    /* Compute time left needed to completely traverse this GC. */
	    IrtRType *R, Dt,
	        FeedRate =
	            GC -> GCodeType == IP_NC_GCODE_LINE_MOTION_G0FAST ?
			GCS -> FastSpeedUpFactor * GC -> UpdatedFeedRate :
	                GC -> UpdatedFeedRate,
	        LeftLen = GCS -> CrntGCodeParam * GC -> Len,
	        LeftTime = LeftLen / FeedRate;

	    if (LeftLen >= -Step) {   /* For -Step, we stay inside this GC. */
	        Dt = Step / FeedRate;
	        GCS -> CrntGCodeParam -= GCS -> CrntGCodeParam
							     * -Step / LeftLen;
		GCS -> CrntTime += Dt;
		GCS -> CrntArcLen += Step;

		Step = 0.0;
		R = CagdCrvEval(GC -> Crv, GCS -> CrntGCodeParam);
		CagdCoerceToE3(NewToolPosition, &R, -1, GC -> Crv -> PType);
	    }
	    else {	         /* For -Dt time, we must go below this GC. */
	        if (IPNCGCodeUpdateGCodeIndex(GCS, GCS -> CrntGCodeIndex - 1,
					      FALSE) < 0) {
		    /* We are at the beginning of the tool path! */
		    IRIT_PT_COPY(NewToolPosition, GC -> XYZ);
		    *NewRealTime = 0.0;
		    *NewGC = GCS -> GCodes[0];
		    return -2.0;
		}
		GCS -> CrntTime -= LeftTime;
		GCS -> CrntArcLen -= LeftLen;
		Step += LeftLen;
		GCS -> CrntGCodeParam = 1.0;
		GC = GCS -> GCodes[GCS -> CrntGCodeIndex];
	    }
	}
	while (Step < 0.0);
    }

    *NewRealTime = GCS -> CrntTime;
    *NewGC = GC;
    return GCS -> CrntArcLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   create tool geometry from tool's parameters, assuming not a general tool M
* in which case this function returns NULL.		                     M
*   Tool is created at a canonical position (origin) and orientation (+Z),   M
*                                                                            *
* PARAMETERS:                                                                M
*   ToolType:    One of flat, ball, or Torus end.                            M
*   Diameter:    Of tool main cylinder.                                      M
*   Height:      Of constructed tool entire geometry. Height must be larger  M
*		 than diameter.			                             M
*   TorusRadius: Only of a Torus end tool - minor radius of torus rounding.  M
*		 TorusRadius must be smaller than Diameter/2.		     M
*   ToolProfile: A 2D profile cross section curve of the constructed tool in M
*		 the XZ plane (+Z only).  This profile is symmetric with     M
*                respect to the Z axis, spanning both -X and +X sides and    M
*		 only holds the bottom visible part of the tool (to be used  M
*		 in Z-buffer further processing).			     M
*   ToolBottom:  The bottom part of the tool that will cut material, in the  M
*		 same canonical position.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Geometry of the constructed tool, NULL if failed.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeTraverseStep                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeGenToolGeom                                                     M
*****************************************************************************/
CagdSrfStruct *IPNCGCodeGenToolGeom(IPNCGCToolType ToolType,
				    IrtRType Diameter,
				    IrtRType Height,
				    IrtRType TorusRadius,
				    CagdCrvStruct **ToolProfile,
				    CagdSrfStruct **ToolBottom)
{
    IRIT_STATIC_DATA IrtRType
	ScaleX[3] = { -1.0, 1.0, 1.0 };
    IrtRType
	Radius = Diameter / 2.0;
    CagdPtStruct Pt1, Pt2, Pt3;
    CagdCrvStruct *Crv1, *Crv2, *Crv;
    CagdSrfStruct *Srf;

    if (ToolType == IP_NC_GCODE_TOOL_TORUS_END) {
        if (TorusRadius >= Radius)
	    ToolType = IP_NC_GCODE_TOOL_BALL_END;
	else if (TorusRadius < IRIT_EPS)
	    ToolType = IP_NC_GCODE_TOOL_FLAT_END;
    }

    if (Height <= Diameter)
	Height = Diameter + IRIT_EPS;
					       
    switch (ToolType) {
	case IP_NC_GCODE_TOOL_BALL_END:
	    IRIT_PT_RESET(Pt1.Pt);
	    Pt1.Pt[0] = Pt1.Pt[2] = Radius;
	    IRIT_PT_RESET(Pt2.Pt);
	    Pt2.Pt[2] = Radius;
	    IRIT_PT_RESET(Pt3.Pt);
	    if ((Crv2 = BzrCrvCreateArc(&Pt1, &Pt2, &Pt3)) == NULL)
		return NULL;
	    break;
	case IP_NC_GCODE_TOOL_TORUS_END:
	    Pt1.Pt[0] = Radius;
	    Pt1.Pt[1] = 0.0;
	    Pt1.Pt[2] = TorusRadius;
	    Pt2.Pt[0] = Radius - TorusRadius;
	    Pt2.Pt[1] = 0.0;
	    Pt2.Pt[2] = TorusRadius;
	    IRIT_PT_RESET(Pt3.Pt);
	    Pt3.Pt[0] = Radius - TorusRadius;
	    if ((Crv1 = BzrCrvCreateArc(&Pt1, &Pt2, &Pt3)) == NULL)
		return NULL;
	    IRIT_PT_RESET(Pt1.Pt);
	    if ((Crv2 = CagdMergeCrvPt(Crv1, &Pt1)) == NULL) {
		CagdCrvFree(Crv1);
		return NULL;
	    }
	    CagdCrvFree(Crv1);
	    break;
	case IP_NC_GCODE_TOOL_FLAT_END:
	    IRIT_PT_RESET(Pt1.Pt);
	    IRIT_PT_RESET(Pt2.Pt);
	    Pt1.Pt[0] = Radius;
	    if ((Crv2 = CagdMergePtPt(&Pt1, &Pt2)) == NULL)
		return NULL;
	    break;
	case IP_NC_GCODE_TOOL_GENERAL:
	default:
	    *ToolProfile = NULL;
	    *ToolBottom = NULL;
	    return NULL;
    }

    IRIT_PT_RESET(Pt1.Pt);
    IRIT_PT_RESET(Pt2.Pt);
    Pt1.Pt[2] = Pt2.Pt[2] = Height;
    Pt2.Pt[0] = Radius;
    if ((Crv1 = CagdMergePtPt(&Pt1, &Pt2)) == NULL) {
	CagdCrvFree(Crv2);
	return NULL;
    }
    if ((Crv = CagdMergeCrvCrv(Crv1, Crv2, TRUE)) == NULL) {
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	return NULL;
    }

    CagdCrvFree(Crv1);
    Srf = CagdSurfaceRev(Crv);
    CagdCrvFree(Crv);

    /* Create the curves' profile. */
    *ToolBottom = CagdSurfaceRev(Crv2);
    Crv1 = CagdCrvCopy(Crv2);
    CagdCrvScale(Crv1, ScaleX);
    Crv = CagdCrvReverse(Crv1);
    CagdCrvFree(Crv1);
    *ToolProfile = CagdMergeCrvCrv(Crv2, Crv, TRUE);
    CagdCrvFree(Crv);
    CagdCrvFree(Crv2);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Substitute G code of type GType in Line, in place.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:    GCode string to substitute one value.                           *
*   GType:   G type to substitute.                                           *
*   NewStr:  New value to substitute in.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPNCGSubstituteSubString(char **Line,
				     char GType,
				     const char *NewStr)
{
    char *p, *q, *r, NewLine[IRIT_LINE_LEN_LONG];

    if ((p = strchr(*Line, GType)) == NULL) {
        /* This G type is not there - simply append it in the end. */
	strcpy(NewLine, *Line);
	sprintf(&NewLine[strlen(NewLine)], " %c%s", GType, NewStr);
    }
    else {
        for (q = *Line, r = NewLine; q != p; )    /* Copy up to the G type. */
	    *r++ = *q++;
	*r = 0;

	sprintf(&NewLine[strlen(NewLine)], "%c%s", GType, NewStr);

	while (!isspace(*p))      /* Skip the property in old and copy new. */
	    p++;
	strcat(NewLine, p);                 /* Copy the rest of the string. */
    }

    /* Replace the string in place. */
    IritFree(*Line);
    *Line = IritStrdup(NewLine);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates the current string representation of this G code sequence.       *
*                                                                            *
* PARAMETERS:                                                                *
*   GCS:     G Code sequence to update as a string representation.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void					                             *
*****************************************************************************/
static void IPNCGUpdateLines(IPNCGCodeStreamStruct *GCS)
{
    int i,
	WasFeedRateChange = FALSE;
    IPNCGCodeLineStruct CrntState;

    IRIT_GEN_COPY(&CrntState, GCS -> GCodes[0], sizeof(IPNCGCodeLineStruct));

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
        char NewStr[IRIT_LINE_LEN_LONG];
        IPNCGCodeLineStruct
            *GC = GCS -> GCodes[i];
	IPNCGCodeLineInfoStruct LineInfo;

	IPNCGcodeUpdateLineInfo(GC -> Line, &LineInfo);

	/* If data is different than string representation, update string. */
	if (GC -> FeedRate > GC -> UpdatedFeedRate || WasFeedRateChange) {
	    sprintf(NewStr, _IPNCGcodeFloatFormat, GC -> UpdatedFeedRate);
	    IPNCGSubstituteSubString(&GC -> Line, 'f', NewStr);

	    LineInfo.HasF = TRUE;
	    LineInfo.F = GC -> UpdatedFeedRate;
	    WasFeedRateChange = GC -> FeedRate > GC -> UpdatedFeedRate;
	}

	/* Update current state. */
	if (LineInfo.HasN)
	    CrntState.GCodeLineNumber = LineInfo.N;

	if (LineInfo.HasG) {
	    switch (LineInfo.G) {
	        case 0:
		    CrntState.GCodeType = IP_NC_GCODE_LINE_MOTION_G0FAST;
		    break;
	        case 1:
		    CrntState.GCodeType = IP_NC_GCODE_LINE_MOTION_G1LINEAR;
		    break;
	        case 2:
		    CrntState.GCodeType = IP_NC_GCODE_LINE_MOTION_G2CW;
		    break;
	        case 3:
		    CrntState.GCodeType = IP_NC_GCODE_LINE_MOTION_G3CCW;
		    break;
	    }
	}

	if (LineInfo.HasX)
	    CrntState.XYZ[0] = LineInfo.X;
	if (LineInfo.HasY)
	    CrntState.XYZ[1] = LineInfo.Y;
	if (LineInfo.HasZ)
	    CrntState.XYZ[2] = LineInfo.Z;

	if (LineInfo.HasI)
	    CrntState.IJK[0] = LineInfo.I;
	if (LineInfo.HasJ)
	    CrntState.IJK[1] = LineInfo.J;
	if (LineInfo.HasK)
	    CrntState.IJK[2] = LineInfo.K;

	if (LineInfo.HasT)
	    CrntState.ToolNumber = LineInfo.T;

	if (LineInfo.HasS)
	    CrntState.SpindleSpeed = LineInfo.S;
	if (LineInfo.HasF)
	    CrntState.FeedRate = LineInfo.F;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Save the given G code sequence into a file.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:    Current GCodes' stream.                                   M
*   FName:         File name to save the G Codes into, "-" for stdout.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, false otherwise.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeLength,  IPNCGCodeParserNumSteps, IPNCGCodeParserFree           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeSave2File                                                       M
*****************************************************************************/
int IPNCGCodeSave2File(VoidPtr IPNCGCodes, const char *FName)
{
    int i;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;
    FILE *f;

    IPNCGUpdateLines(IPNCGCodes);

    if (strcmp(FName, "-") == 0)
        f = stdout;
    else {
        if ((f = fopen(FName, "w")) == NULL)
	    return FALSE;
    }

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
	IPNCGCodeLineStruct
	    *GC = GCS -> GCodes[i];

	fprintf(f, "%s\n", GC -> Line);
    }

    fclose(f);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Traverses the given G code sequence as strings.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:    Current GCodes' stream.                                   M
*   Restart:       TRUE to init the traversal process.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:  Next line, NULL if done.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPNCGCodeParserInit, IPNCGCodeParserParseLine,			     M
*   IPNCGCodeParserSetStep, IPNCGCodeParserGetNext,			     M
*   IPNCGCodeLength,  IPNCGCodeParserNumSteps, IPNCGCodeParserFree           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeTraverseLines                                                   M
*****************************************************************************/
const char *IPNCGCodeTraverseLines(VoidPtr IPNCGCodes, int Restart)
{
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    if (Restart)
        GCS -> TraversalStringIndex = 0;
    else if (GCS -> TraversalStringIndex >= GCS -> NumOfGCodes)
        return NULL;

    return GCS -> GCodes[GCS -> TraversalStringIndex++] -> Line;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset the feedrates to original inputfile values.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   IPNCGCodes:       G Code sequence to traverse.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPNCGCodeResetFeedRates                                                  M
*****************************************************************************/
void IPNCGCodeResetFeedRates(VoidPtr IPNCGCodes)
{
    int i;
    IPNCGCodeStreamStruct
	*GCS = (IPNCGCodeStreamStruct *) IPNCGCodes;

    for (i = 0; i < GCS -> NumOfGCodes; i++) {
        IPNCGCodeLineStruct
            *GC = GCS -> GCodes[i];

	GC -> UpdatedFeedRate = GC -> FeedRate;
    }
}
