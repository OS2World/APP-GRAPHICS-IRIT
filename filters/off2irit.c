/*****************************************************************************
* Filter to convert GeomView's OFF data files to IRIT .irt files.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1998    *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_lib.h"
#include "misc_lib.h"

#define SAME_COLOR_EPS	1e-2

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "off2irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char 
    *VersionStr = "off2irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "off2irit o%-OutName!s z%- OFFFile!*s";

IRIT_STATIC_DATA int
    GlblLineCount = 0;

static char *GetOFFLine(FILE *OFFFile);
static void Off2IritExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of off2irit - Read command line and do what is needed...	     M
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
    int i, Error, Color[3],
	DATHandle = -1,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
        NumVertices = 0,
        NumPolys = 0,
	NumEdges = 0,
	NumFiles = 0;
    char Name[IRIT_LINE_LEN],
	*Line,
	*OutFileName = NULL,
	**FileNames = NULL;
    FILE *OFFFile;
    IrtVecType
	*Vertices = NULL;
    IPPolygonStruct
        *PHead = NULL;
    IPObjectStruct *PObj;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &OutFileFlag, &OutFileName, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Off2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Off2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Off2IritExit(1);
    }
    else if (NumFiles > 1) {
	IRIT_WARNING_MSG("Cannot handle more than one OFF file at a time, exit.\n");
	GAPrintHowTo(CtrlStr);
	Off2IritExit(1);
    }

    if (strcmp(FileNames[0], "-") == 0) {
        strcpy(Name, "NONE");
	OFFFile = stdin;
    }
    else if ((OFFFile = fopen(FileNames[0], "r")) == NULL) {
	IRIT_WARNING_MSG_PRINTF("Cannot open OFF file \"%s\", exit.\n", FileNames[0]);
	Off2IritExit(1);
    }
    else {
	char *p;

	if ((p = strrchr(FileNames[0], '\\')) == NULL &&
	    (p = strrchr(FileNames[0], '/')) == NULL)
	    p = FileNames[0];
	strcpy(Name, p);
	if ((p = strchr(Name, '.')) != NULL)
	    *p = 0;
    }

    if (OutFileName != NULL) {
	if ((DATHandle = IPOpenDataFile(OutFileName, FALSE, TRUE)) < 0) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Off2IritExit(2);
	}
    }
    else
        DATHandle = IPOpenStreamFromFile(stdout, FALSE, FALSE, FALSE, FALSE);

    Line = GetOFFLine(OFFFile);
    if (strncmp(Line, "OFF", 3) != 0) {
	IRIT_WARNING_MSG("Expecting 'OFF' at first line\n");
	Off2IritExit(3);
    }

    if (sscanf(Line, "OFF %d %d %d",
	       &NumVertices, &NumPolys, &NumEdges) != 3) {
	Line = GetOFFLine(OFFFile);
	if (sscanf(Line, "%d %d %d",
		   &NumVertices, &NumPolys, &NumEdges) != 3) {
	    IRIT_WARNING_MSG("Expecting number of vertices and number of polygons to begin with\n");
	    Off2IritExit(3);
	}
    }

    /* Get the vertices. */
    Vertices = (IrtVecType *) IritMalloc(sizeof(IrtVecType) * NumVertices);
    for (i =  0; i < NumVertices; i++) {
	Line = GetOFFLine(OFFFile);

#ifdef IRIT_DOUBLE
	if (sscanf(Line, "%lf %lf %lf",
#else
	if (sscanf(Line, "%f %f %f",
#endif /* IRIT_DOUBLE */
		   &Vertices[i][0], &Vertices[i][1], &Vertices[i][2]) != 3) {
	    IRIT_WARNING_MSG_PRINTF("Expecting 3 vertex coordinates at line %d\n",
		                    GlblLineCount);
	    Off2IritExit(3);
	}
    }

    /* Get the polygons. */
    for (i = 0; i < NumPolys; i++) {
	int j, n;
	char *p;

	Line = GetOFFLine(OFFFile);
	PHead = IPAllocPolygon(0, NULL, PHead);

	p = strtok(Line, " \t\n\r");
	if (p == NULL || sscanf(p, "%d", &n) != 1) {
	    IRIT_WARNING_MSG_PRINTF(
		    "Expecting number of polygon's vertices at line %d\n",
		    GlblLineCount);
	    Off2IritExit(4);
	}
	if (n < 3) {
	    IRIT_WARNING_MSG_PRINTF(
		    "Number of polygon's vertices less than 3 ignored (found %d), line %d\n",
		    GlblLineCount);
	    continue;
	}

	for (j = 0; j < n; j++) {
	    int VIndex;

	    p = strtok(NULL, " \t\n\r");
	    if (p == NULL || sscanf(p, "%d", &VIndex) != 1) {
		IRIT_WARNING_MSG_PRINTF("Expecting vertex index at line %d\n",
			                GlblLineCount);
		Off2IritExit(5);
	    }

	    PHead -> PVertex = IPAllocVertex2(PHead -> PVertex);
	    IRIT_PT_COPY(PHead -> PVertex -> Coord, Vertices[VIndex]);
	}

	assert(PHead -> PVertex != NULL &&
	       PHead -> PVertex -> Pnext != NULL &&
	       PHead -> PVertex -> Pnext -> Pnext != NULL);

	if (i == 0) {
	    /* Pick a color. */
	    for (j = 0; j < 3; j++) {
		float f;

		p = strtok(NULL, " \t\n\r");
		if (p == NULL || sscanf(p, "%f", &f) != 1) {
		    Color[0] = Color[1] = Color[2] = 255;
		    break;
		}
		Color[j] = (int) (255.0 * f);
	    }
	}
    }

    IRIT_INFO_MSG_PRINTF("%d polygons allocated\n", IPPolyListLen(PHead));

    PObj = IPGenPolyObject(Name, PHead, NULL);
    IP_SET_POLYGON_OBJ(PObj);
    AttrSetObjectRGBColor(PObj, Color[0], Color[1], Color[2]);
    IPSetFilterDegen(FALSE);
    IPPutObjectToHandler(DATHandle, PObj);
    IPFreeObject(PObj);
    IPCloseStream(DATHandle, TRUE);
    fclose(OFFFile);

    IritFree(Vertices);

    Off2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one line from the OFF file.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   OFFFile:   File to read from.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:    Read line, or NUL if EOF.                                     *
*****************************************************************************/
static char *GetOFFLine(FILE *OFFFile)
{
    IRIT_STATIC_DATA char Line[IRIT_LINE_LEN_VLONG];

    while (fgets(Line, IRIT_LINE_LEN_VLONG - 1, OFFFile)) {
	GlblLineCount++;

	if (Line[0] != '#' && Line[0] != 0x0a && Line[0] != 0x0d)
	    return Line;
    }

    IRIT_WARNING_MSG_PRINTF("Premature termination of OFF file at line %d\n",
	                    GlblLineCount);
    Off2IritExit(4);
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Off2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Off2IritExit(int ExitCode)
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
