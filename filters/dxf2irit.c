/*****************************************************************************
* Filter to convert AutoCad DXF data files to IRIT .irt files.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
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

#ifdef IRIT_DOUBLE
#    define SCANF_FLOAT_FORMAT "%lf"
#else
#    define SCANF_FLOAT_FORMAT "%f"
#endif /* IRIT_DOUBLE */

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "DXF2Irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "dxf2irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "DXF2Irit m%- f%- o%-OutName!s b%- z%- DXFFile!*s";

IRIT_STATIC_DATA char
    GlblUnGetStr[IRIT_LINE_LEN];

IRIT_STATIC_DATA int
    GlblBinaryOutput = FALSE,
    GlblUngetCode = -1,
    GlblLineCount = 0,
    GlblFloatEndCondition = FALSE,
    GlblMoreFlag = FALSE;

IRIT_STATIC_DATA IPObjectStruct
    *GlblBlockInstances = NULL;

static int GetNextEntity(FILE *DXFFile, char **Str);
static void UnGetEntity(int Code, char *Str);
static void GetInstancesForIrit(FILE *DXFFile);
static void DumpDataForIrit(FILE *DXFFile, FILE *DATFile);
static IPObjectStruct *GetBLOCK(FILE *DXFFile);
static IPObjectStruct *GetINSERT(FILE *DXFFile);
static IPObjectStruct *GetPOINT(FILE *DXFFile);
static IPObjectStruct *Get3DLINE(FILE *DXFFile);
static IPObjectStruct *GetARC(FILE *DXFFile);
static IPObjectStruct *GetCIRCLE(FILE *DXFFile);
static IPObjectStruct *Get3DFACE(FILE *DXFFile);
static IPObjectStruct *GetPOLYLINE(FILE *DXFFile);
static void DumpOneObject(FILE *DATFile, IPObjectStruct *PObject, int FreeObj);
static void ListObjectInsertDXF(IPObjectStruct *PObjBlock,
				int *Index,
				IPObjectStruct *NewObj);

void Dxf2IritExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of dat2irit - Read command line and do what is needed...	     M
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
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	NumFiles = 0;
    char *Line,
	*OutFileName = NULL,
	**FileNames = NULL;
    FILE *DXFFile, *DATFile;
    IPObjectStruct *PObj;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GlblMoreFlag,
			   &GlblFloatEndCondition,
			   &OutFileFlag, &OutFileName,
			   &GlblBinaryOutput, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Dxf2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Dxf2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Dxf2IritExit(1);
    }
    else if (NumFiles > 1) {
	IRIT_WARNING_MSG("Cannot handle more than one DXF file at a time, exit.\n");
	GAPrintHowTo(CtrlStr);
	Dxf2IritExit(1);
    }

    if (strcmp(FileNames[0], "-") == 0)
	DXFFile = stdin;
    else if ((DXFFile = fopen(FileNames[0], "r")) == NULL) {
	IRIT_WARNING_MSG_PRINTF("Cannot open DXF file \"%s\", exit.\n", FileNames[0]);
	Dxf2IritExit(1);
    }

    if (OutFileName != NULL) {
	if ((DATFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Dxf2IritExit(2);
	}
    }
    else
	DATFile = stdout;

    /* Skip the DXF file until "SECTION BLOCKS/ENTITIES" is found. */
    while (GetNextEntity(DXFFile, &Line) >= 0) {
	if (strncmp(Line, "SECTION", 7) == 0 &&
	    GetNextEntity(DXFFile, &Line) == 2 &&
	    (strncmp(Line, "BLOCKS", 6) == 0 ||
	     strncmp(Line, "ENTITIES", 8) == 0))
	      break;
    }

    if (strncmp(Line, "BLOCKS", 6) == 0) {
	GetInstancesForIrit(DXFFile);

	if (GlblBlockInstances != NULL) {
	    if (GlblMoreFlag)
	        IRIT_INFO_MSG("Detected BLOCKS:\n");
	    for (PObj = GlblBlockInstances;
		 PObj != NULL;
		 PObj = PObj -> Pnext) {
		if (GlblMoreFlag)
		    IRIT_INFO_MSG_PRINTF("%s\n", IP_GET_OBJ_NAME(PObj));
		DumpOneObject(DATFile, PObj, FALSE);
	    }
	}

	/* Skip the DXF file until "SECTION ENTITIES" is found. */
	while (GetNextEntity(DXFFile, &Line) >= 0) {
	    if (strncmp(Line, "SECTION", 7) == 0 &&
		GetNextEntity(DXFFile, &Line) == 2 &&
		strncmp(Line, "ENTITIES", 8) == 0)
	      break;
	}
    }

    if (feof(DXFFile)) {
	IRIT_WARNING_MSG_PRINTF("Illegal DXF file \"%s\", exit.\n", FileNames[0]);
	Dxf2IritExit(1);
    }

    DumpDataForIrit(DXFFile, DATFile);
    DumpOneObject(DATFile, NULL, TRUE);
    fclose(DXFFile);
    fclose(DATFile);

    Dxf2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   One level of unget entity.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Code:      of entity to unget.                                           *
*   Str:       String representation of entity to unget.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void UnGetEntity(int Code, char *Str)
{
    GlblUngetCode = Code;
    strcpy(GlblUnGetStr, Str);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads the next entity from the DXF file.  Entity consists of a code in   *
* one line (integer) and a string of the following line.		     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFile:      The DXF file to read from.                                  *
*   Str:         The string of the following line, in a statically allocated *
*		 space.							     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:         The DXF code of the entity, -1 for EOF, -2 for error.       *
*****************************************************************************/
static int GetNextEntity(FILE *DXFFile, char **Str)
{
    IRIT_STATIC_DATA char Line[IRIT_LINE_LEN_VLONG];
    int i, Code;

    if (GlblUngetCode >= 0) { /* Reget last unget entity. */
        *Str = GlblUnGetStr;
	Code = GlblUngetCode;
        GlblUngetCode = -1;
        return Code;
    }

    if (fgets(Line, IRIT_LINE_LEN_VLONG - 1, DXFFile) == NULL)
	return -1;
    if (sscanf(Line, "%d", &Code) != 1) {
	IRIT_WARNING_MSG_PRINTF(
		"Entity integer code expected in line %d, found \"%s\"\n",
		GlblLineCount, Line);
	return -2;
    }

    if (fgets(Line, IRIT_LINE_LEN_VLONG - 1, DXFFile) == NULL)
	return -1;
    for (i = strlen(Line) - 1; i >= 0; i--) {         /* Filters out /n/r. */
	if (Line[i] < ' ')
            Line[i] = 0;
	else
	    break;
    }

    GlblLineCount += 2;

    *Str = Line;

    return Code;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets BLOCKS section from DXF file f as intances into GlblBlockInstances.   *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:        File where DXF input comes from.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetInstancesForIrit(FILE *DXFFile)
{
    int PObjBlockIndex;
    char *Line;
    IPObjectStruct
	*PObjBlock = NULL;

    while (GetNextEntity(DXFFile, &Line) >= 0) {
	if (strncmp(Line, "BLOCK", 5) == 0) {
	    if (PObjBlock != NULL) {
		IRIT_WARNING_MSG_PRINTF("A BLOCK without an ENDBLK detected, line %d\n",
			                GlblLineCount);
	    }
	    PObjBlockIndex = 0;
	    PObjBlock = GetBLOCK(DXFFile);
	}
	else if (strncmp(Line, "INSERT", 6) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, GetINSERT(DXFFile));
	}
	else if (strncmp(Line, "POINT", 5) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, GetPOINT(DXFFile));
	}
	else if (strncmp(Line, "LINE", 4) == 0 ||
	    strncmp(Line, "3DLINE", 6) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, Get3DLINE(DXFFile));
	}
	else if (strncmp(Line, "CIRCLE", 6) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, GetCIRCLE(DXFFile));
	}
	else if (strncmp(Line, "ARC", 3) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, GetARC(DXFFile));
	}
	else if (strncmp(Line, "3DFACE", 6) == 0 ||
		 strncmp(Line, "SOLID", 5) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, Get3DFACE(DXFFile));
	}
	else if (strncmp(Line, "POLYLINE", 8) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, GetPOLYLINE(DXFFile));
	}
	else if (strncmp(Line, "ENDBLK", 6) == 0) {
	    ListObjectInsertDXF(PObjBlock, &PObjBlockIndex, NULL);
	    if (PObjBlock -> U.Lst.PObjList[0] != NULL) {
		PObjBlock -> Pnext = GlblBlockInstances;
		GlblBlockInstances = PObjBlock;
	    }
	    else {
		IRIT_WARNING_MSG_PRINTF("Empty block \"%s\" ignored, line %d\n",
			                IP_GET_OBJ_NAME(PObjBlock),
					GlblLineCount);
	    }
	    PObjBlock = NULL;
	}
	else if (strncmp(Line, "ENDSEC", 6) == 0) {
	    break;
	}
    }

    if (GlblMoreFlag)
        IRIT_INFO_MSG("Done instances (Blocks)\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data from DXF file f into stdout.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:   File where DXF input comes from.                              *
*   DATFile:   File where IRIT data output goes to.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForIrit(FILE *DXFFile, FILE *DATFile)
{
    char *Line;

    while (GetNextEntity(DXFFile, &Line) >= 0) {
	if (strncmp(Line, "INSERT", 6) == 0) {
	    DumpOneObject(DATFile, GetINSERT(DXFFile), TRUE);
	}
	else if (strncmp(Line, "POINT", 5) == 0) {
	    DumpOneObject(DATFile, GetPOINT(DXFFile), TRUE);
	}
	else if (strncmp(Line, "LINE", 4) == 0 ||
	    strncmp(Line, "3DLINE", 6) == 0) {
	    DumpOneObject(DATFile, Get3DLINE(DXFFile), TRUE);
	}
	else if (strncmp(Line, "CIRCLE", 6) == 0) {
	    DumpOneObject(DATFile, GetCIRCLE(DXFFile), TRUE);
	}
	else if (strncmp(Line, "ARC", 3) == 0) {
	    DumpOneObject(DATFile, GetARC(DXFFile), TRUE);
	}
	else if (strncmp(Line, "3DFACE", 6) == 0 ||
		 strncmp(Line, "TRACE", 5) == 0 ||
		 strncmp(Line, "SOLID", 5) == 0) {
	    DumpOneObject(DATFile, Get3DFACE(DXFFile), TRUE);
	}
	else if (strncmp(Line, "POLYLINE", 8) == 0) {
	    DumpOneObject(DATFile, GetPOLYLINE(DXFFile), TRUE);
	}
	else if (strncmp(Line, "ENDSEC", 6) == 0) {
	    break;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one BLOCK entity out of DXF file f (BLOCK line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:    File to read DXF from.                                       *
*   PObjBlock:  Object to update its name etc.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  An IRIT object representing the block's geometry.     *
*****************************************************************************/
static IPObjectStruct *GetBLOCK(FILE *DXFFile)
{
    int Code,
	Done = FALSE;
    char *Line;
    IrtPtType Pt;
    IPObjectStruct
	*PObjBlock = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

    IRIT_PT_RESET(Pt);

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 2:
	    case 3:
		IP_SET_OBJ_NAME2(PObjBlock, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	        {
		    int CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Pt[CoordIndex]);
		}
		break;
	    default:
		break;
	}
    }

    AttrSetObjectObjAttrib(PObjBlock, "Position",
			   IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]), FALSE);
    return PObjBlock;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one INSERT entity out of DXF file f (INSERT line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  An IRIT object representing the insert's geometry.    *
*****************************************************************************/
static IPObjectStruct *GetINSERT(FILE *DXFFile)
{
    int i, Code,
	Done = FALSE,
	HasName = FALSE;
    char *Line, Name[IRIT_LINE_LEN];
    IrtRType RotAngle = 0.0;
    IrtPtType Pt;
    IrtVecType Scale, ExtrudeDir;
    IrtHmgnMatType Mat;
    IPObjectStruct
	*PObj = IPAllocObject("", IP_OBJ_INSTANCE, NULL);

    IRIT_PT_RESET(Pt);
    IRIT_PT_RESET(ExtrudeDir);
    ExtrudeDir[2] = 1.0;
    Scale[0] = Scale[1] = Scale[2] = 1.0;
    strcpy(Name, "Dxf");

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
    	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 2:
		for (i = strlen(Line) - 1; i >= 0; i--) {
		    if (Line[i] < ' ')
		        Line[i] = 0;
		    else
		        break;
		}
		PObj -> U.Instance -> Name = IritStrdup(Line);
		HasName = TRUE;
		break;
	    case 8:
		strcat(Name, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	        {
		    int CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Pt[CoordIndex]);
		}
		break;
	    case 41:
	    case 42:
	    case 43:
	        {
		    int CoordIndex = (Code % 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Scale[CoordIndex]);
		}
		break;
	    case 44:
	    case 45:
	    case 70:
	    case 71:
		IRIT_WARNING_MSG_PRINTF("Unsupported code %d in INSERT, line %d\n",
			                Code, GlblLineCount);
		break;
	    case 50:
		sscanf(Line, SCANF_FLOAT_FORMAT, &RotAngle);
		break;
	    case 210:
	    case 220:
	    case 230:
	        {
		    int CoordIndex = ((Code - 200) / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &ExtrudeDir[CoordIndex]);
		}
		break;
	    default:
		break;
	}
    }

    if (!HasName) {
	IRIT_INFO_MSG_PRINTF("Name of INSERT was not found, line %d\n",
		             GlblLineCount);
    }

    MatGenUnitMat(PObj -> U.Instance -> Mat);

    MatGenMatRotZ1(IRIT_DEG2RAD(RotAngle), Mat);
    MatMultTwo4by4(PObj -> U.Instance -> Mat, PObj -> U.Instance -> Mat, Mat);

    MatGenMatScale(Scale[0], Scale[1], Scale[2], Mat);
    MatMultTwo4by4(PObj -> U.Instance -> Mat, PObj -> U.Instance -> Mat, Mat);
    MatGenMatTrans(Pt[0], Pt[1], Pt[2], Mat);
    MatMultTwo4by4(PObj -> U.Instance -> Mat, PObj -> U.Instance -> Mat, Mat);

    IP_SET_OBJ_NAME2(PObj, Name);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets POINT entities out of DXF file f (POINT line read).		     *
* If several POINTs are together, they are all read and processed.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    An IRIT object representing the POINT.	             *
*****************************************************************************/
static IPObjectStruct *GetPOINT(FILE *DXFFile)
{
    int Code,
	Done = FALSE;
    char *Line, Name[IRIT_LINE_LEN];
    IPVertexStruct
        *V = IPAllocVertex2(NULL),
        *VList = NULL;
    IPObjectStruct *PObj;
    
    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
        switch (Code) {
	    case 0:
		if (strncmp(Line, "POINT", 5) == 0) {       /* More points. */
		    IRIT_LIST_PUSH(V, VList);
		    V = IPAllocVertex2(NULL);
		}
		else {
		    UnGetEntity(Code, Line);
		    Done = TRUE;
		}
		break;
	    case 8:
	        strncpy(Name, Line, IRIT_LINE_LEN);
		break;
	    case 10:
	    case 20:
	    case 30:
	        {
		    int CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &V -> Coord[CoordIndex]);
		}
		break;
	    default:
	        break;
	}
    }

    if (V != NULL && VList != NULL) {
        /* More than one point here. */
        IRIT_LIST_PUSH(V, VList);
    }
    
    if (VList != NULL) {
        PObj = IPGenPointListObject(Name, IPAllocPolygon(0, VList, NULL),
				    NULL);
    }
    
    else {
        PObj = IPGenPtObject(Name,
			     &V -> Coord[0], &V -> Coord[1], &V -> Coord[2],
			     NULL);
	IPFreeVertex(V);
    }
    
    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one 3DLINE entity out of DXF file f (3DLINE line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    An IRIT polyline representing the 3DLINE.           *
*****************************************************************************/
static IPObjectStruct *Get3DLINE(FILE *DXFFile)
{
    int i, Code,
	Done = FALSE;
    char *Line, Name[IRIT_LINE_LEN];
    IrtRType
	Elevation = 0.0,
	Thickness = 0.0;
    IrtPtType Pts[2];
    IrtVecType ExtrudeDir;
    IPVertexStruct *V1, *V2;
    IPPolygonStruct *P;
    IPObjectStruct *PObj;

    IRIT_PT_RESET(Pts[0]);
    IRIT_PT_RESET(Pts[1]);
    IRIT_PT_RESET(ExtrudeDir);
    ExtrudeDir[2] = 1.0;
    strcpy(Name, "Dxf");

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
    	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 8:
		strcat(Name, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	    case 11:
	    case 21:
	    case 31:
	        {
		    int PtIndex = (Code % 10),
		        CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Pts[PtIndex][CoordIndex]);
		}
		break;
	    case 38:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Elevation);
		break;
	    case 39:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Thickness);
		break;
	    case 210:
	    case 220:
	    case 230:
	        {
		    int CoordIndex = ((Code - 200) / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &ExtrudeDir[CoordIndex]);
		}
		break;
	    default:
		break;
	}
    }

    V2 = IPAllocVertex2(NULL);
    V1 = IPAllocVertex2(V2);
    P = IPAllocPolygon(0, V1, NULL);

    for (i = 0; i < 3; i++) {
	V1 -> Coord[i] = Pts[0][i];
	V2 -> Coord[i] = Pts[1][i];
    }
    V1 -> Coord[2] += Elevation;
    V2 -> Coord[2] += Elevation;

    PObj = IPGenPolyObject(Name, P, NULL);
    IP_SET_POLYLINE_OBJ(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one ARC entity out of DXF file f (ARC line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    An IRIT curve representing the ARC.	             *
*****************************************************************************/
static IPObjectStruct *GetARC(FILE *DXFFile)
{
    int Code,
	Done = FALSE;
    char *Line, Name[IRIT_LINE_LEN];
    IrtRType
	Elevation = 0.0,
	Thickness = 0.0,
	Radius = 1.0,
	StartAngle = 0.0,
	EndAngle = 360;
    CagdPtStruct Center;
    IrtVecType ExtrudeDir;
    CagdCrvStruct *Crv;
    IPObjectStruct *PObj;

    IRIT_PT_RESET(Center.Pt);
    IRIT_PT_RESET(ExtrudeDir);
    ExtrudeDir[2] = 1.0;
    strcpy(Name, "Dxf");

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
    	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 8:
		strcat(Name, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	        {
		    int CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Center.Pt[CoordIndex]);
		}
		break;
	    case 38:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Elevation);
		break;
	    case 39:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Thickness);
		break;
	    case 40:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Radius);
		break;
	    case 50:
		sscanf(Line, SCANF_FLOAT_FORMAT, &StartAngle);
		break;
	    case 51:
		sscanf(Line, SCANF_FLOAT_FORMAT, &EndAngle);
		break;
	    case 210:
	    case 220:
	    case 230:
	        {
		    int CoordIndex = ((Code - 200) / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &ExtrudeDir[CoordIndex]);
		}
		break;
	    default:
		break;
	}
    }

    Center.Pt[2] += Elevation;

    Crv = CagdCrvCreateArc(&Center, Radius, StartAngle, EndAngle);

    PObj = IPGenCrvObject(Name, Crv, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one CIRCLE entity out of DXF file f (CIRCLE line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    An IRIT curve representing the CIRCLE.	             *
*****************************************************************************/
static IPObjectStruct *GetCIRCLE(FILE *DXFFile)
{
    int Code,
	Done = FALSE;
    char *Line, Name[IRIT_LINE_LEN];
    IrtRType
	Elevation = 0.0,
	Thickness = 0.0,
	Radius = 1.0;
    CagdPtStruct Center;
    CagdVecStruct ExtrudeDir;
    CagdCrvStruct *Crv;
    IPObjectStruct *PObj;

    IRIT_PT_RESET(Center.Pt);
    IRIT_PT_RESET(ExtrudeDir.Vec);
    ExtrudeDir.Vec[2] = 1.0;
    strcpy(Name, "Dxf");

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
    	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 8:
		strcat(Name, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	        {
		    int CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Center.Pt[CoordIndex]);
		}
		break;
	    case 38:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Elevation);
		break;
	    case 39:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Thickness);
		break;
	    case 40:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Radius);
		break;
	    case 210:
	    case 220:
	    case 230:
	        {
		    int CoordIndex = ((Code - 200) / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT,
			   &ExtrudeDir.Vec[CoordIndex]);
		}
		break;
	    default:
		break;
	}
    }

    Center.Pt[2] += Elevation;

    Crv = BspCrvCreateCircle(&Center, Radius);

    if (!IRIT_APX_EQ(Thickness, 0.0)) {
	CagdSrfStruct *Srf;

	IRIT_PT_SCALE(ExtrudeDir.Vec, Thickness);
	Srf = CagdExtrudeSrf(Crv, &ExtrudeDir);

	CagdCrvFree(Crv);
	PObj = IPGenSrfObject(Name, Srf, NULL);
    }
    else
        PObj = IPGenCrvObject(Name, Crv, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one 3DFACE entity out of DXF file f (3DFACE line read).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:       File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    An IRIT poly representing the 3DFACE.               *
*****************************************************************************/
static IPObjectStruct *Get3DFACE(FILE *DXFFile)
{
    int	i, Code,
	Done = FALSE,
    	HiddenEdges = 0;
    char *Line, Name[IRIT_LINE_LEN];
    IrtRType
	Elevation = 0.0;
    IrtPtType Pts[4];
    IPVertexStruct *V1, *V2, *V3, *V4;
    IPPolygonStruct *P;
    IPObjectStruct *PObj;

    IRIT_PT_RESET(Pts[0]);
    IRIT_PT_RESET(Pts[1]);
    IRIT_PT_RESET(Pts[2]);
    IRIT_PT_RESET(Pts[3]);
    strcpy(Name, "Dxf");

    while (!Done && (Code = GetNextEntity(DXFFile, &Line)) >= 0) {
	switch (Code) {
	    case 0:
		UnGetEntity(Code, Line);
	        Done = TRUE;
		break;
	    case 8:
		strcat(Name, Line);
		break;
	    case 10:
	    case 20:
	    case 30:
	    case 11:
	    case 21:
	    case 31:
	    case 12:
	    case 22:
	    case 32:
	    case 13:
	    case 23:
	    case 33:
	        {
		    int PtIndex = (Code % 10) - 0,
		        CoordIndex = (Code / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &Pts[PtIndex][CoordIndex]);
		}
		break;
	    case 38:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Elevation);
		break;
	    case 70:
		sscanf(Line, "%d", &HiddenEdges);
		break;
	    default:
		break;
	}
    }

    V3 = IPAllocVertex2(NULL);
    V2 = IPAllocVertex2(V3);
    V1 = IPAllocVertex2(V2);
    P = IPAllocPolygon(0, V1, NULL);

    if (IRIT_PT_APX_EQ(Pts[2], Pts[3])) {
    	V4 = NULL;
    }
    else {
	V4 = IPAllocVertex2(NULL);
	V3 -> Pnext = V4;
    }

    /* Set hidden attributes. */
    if (HiddenEdges & 0x01)
	IP_SET_INTERNAL_VRTX(V1);
    if (HiddenEdges & 0x02)
	IP_SET_INTERNAL_VRTX(V2);
    if (HiddenEdges & 0x04)
	IP_SET_INTERNAL_VRTX(V3);
    if (V4 != NULL && HiddenEdges & 0x08)
	IP_SET_INTERNAL_VRTX(V4);

    P = IPAllocPolygon(0, V1, NULL);

    /* From my little experience with DXF files it looks like the order of   */
    /* the vertices is consistent. To get normals to point inside as IRIT    */
    /* needs, we switch the order. Hopefully this is really consistent.      */
    for (i = 0; i < 3; i++) {
	V1 -> Coord[i] = Pts[0][i];
	V2 -> Coord[i] = Pts[1][i];
	V3 -> Coord[i] = Pts[2][i];
	if (V4 != NULL)			      /* We have 4 vertices polygon. */
	    V4 -> Coord[i] = Pts[3][i];
    }
    V1 -> Coord[2] += Elevation;
    V2 -> Coord[2] += Elevation;
    V3 -> Coord[2] += Elevation;
    if (V4 != NULL)
        V4 -> Coord[2] += Elevation;

    PObj = IPGenPolyObject(Name, P, NULL);
    IP_SET_POLYGON_OBJ(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Gets one POLYLINE entity out of DXF file f (POLYLINEline read). 	     *
*   The entity can also be a surface mesh, so it is supported as well.       *
*    Returns TRUE if polyline, FALSE if a surface.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   DXFFile:	   File to read DXF from.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  IRIT object representing the POLY geometry. Can be    *
*		       a polyline, polygon or a surface.                     *
*****************************************************************************/
static IPObjectStruct *GetPOLYLINE(FILE *DXFFile)
{
    char *Line, Name[IRIT_LINE_LEN];
    int Code = 0,
	PolyFlags = 0,
        NumVertices = 0,
	ULength = 0,
	VLength = 0,
	SmoothULength = 0,
	SmoothVLength = 0,
	SurfaceType = 0, /* No smooth surface fitted. */
	IsSurface = FALSE,
	IsClosedM = FALSE,
	IsClosedN = FALSE,
	IsPolyfaceMesh = FALSE,
	SkipRead = FALSE,
	Done = FALSE,
	VertexArrayIndex = 0,
	VertexArraySize = 10;
    IrtRType
	Elevation = 0.0,
	Thickness = 0.0;
    IrtVecType ExtrudeDir;
    CagdSrfStruct *PSrf;
    IPVertexStruct *V,
	**VertexArray,
	*VLast = NULL;
    IPPolygonStruct
	*PFace = NULL,
	*P = IPAllocPolygon(0, NULL, NULL);
    IPObjectStruct *PObj;

    VertexArray = (IPVertexStruct **)
	IritMalloc(sizeof(IPVertexStruct *) * VertexArraySize);

    strcpy(Name, "Dxf");
    IRIT_PT_RESET(ExtrudeDir);
    ExtrudeDir[2] = 1.0;

    while (!Done && (SkipRead || (Code = GetNextEntity(DXFFile, &Line)) >= 0)) {
	SkipRead = FALSE;

    	switch (Code) {
	    case 8:
		strcat(Name, Line);
		break;
	    case 70:
    	        sscanf(Line, "%d", &PolyFlags);
		IsClosedM = (PolyFlags & 1) != 0;
		IsSurface = (PolyFlags & 16) != 0;
		IsClosedN = (PolyFlags & 32) != 0;
		IsPolyfaceMesh = (PolyFlags & 64) != 0;
		break;
	    case 71:
		sscanf(Line, "%d", &VLength);
		break;
	    case 72:
		sscanf(Line, "%d", &ULength);
		break;
	    case 73:
		sscanf(Line, "%d", &SmoothVLength);
		break;
	    case 74:
		sscanf(Line, "%d", &SmoothULength);
		break;
	    case 75:
		sscanf(Line, "%d", &SurfaceType);
		break;
	    case 0:
		if (strncmp(Line, "VERTEX", 6) == 0) {
		    int VCode = -1,
			VDone = FALSE,
			VIndex = 0,
		        VFlags = 0,
		        VIndices[4];

		    NumVertices++;

		    V = IPAllocVertex2(NULL);
		    IRIT_PT_RESET(V -> Coord);

		    while (!VDone && (VCode = GetNextEntity(DXFFile,
							    &Line)) >= 0) {
			switch (VCode) {
			    case 10:
			    case 20:
			    case 30:
			        {
				    int Index = (VCode / 10) - 1;

				    sscanf(Line, SCANF_FLOAT_FORMAT,
					   &V -> Coord[Index]);
				}
				break;
			    case 70:
				sscanf(Line, "%d", &VFlags);
				break;
			    case 71:
			    case 72:
			    case 73:
			    case 74:
				if (VIndex >= 4) {
				    IRIT_WARNING_MSG_PRINTF(
					    "More than four vertices in a pface, line %d\n",
					    GlblLineCount);
				    break;
				}
				sscanf(Line, "%d", &VIndices[VIndex]);
				if (VIndices[VIndex] > 0)
				    VIndices[VIndex++]--;   /* Array from 0. */
				else
				    VIndices[VIndex++]++;
				break;
			    case 0:
				VDone = TRUE;
				break;
			    default:
				break;
			}

		    }

		    if ((VFlags & 128) && !(VFlags & 64)) {
			/* This vertex actually defines a face in pface... */
			if (VIndex < 3) {
			    IRIT_WARNING_MSG_PRINTF(
				    "Less than three vertices in a pface, line %d\n",
				    GlblLineCount);
			}
			else {
			    int i;

			    PFace = IPAllocPolygon(0, V, PFace);
			    if (VIndices[0] < 0) {
				IP_SET_INTERNAL_VRTX(V);
				VIndices[0] = -VIndices[0];
			    }
			    IRIT_PT_COPY(V -> Coord,
				    VertexArray[VIndices[0]] -> Coord);

			    for (i = 1; i < VIndex; i++) {
				V -> Pnext = IPAllocVertex2(NULL);
				V = V -> Pnext;
				if (VIndices[i] < 0) {
				    IP_SET_INTERNAL_VRTX(V);
				    VIndices[i] = -VIndices[i];
				}
				IRIT_PT_COPY(V -> Coord,
					VertexArray[VIndices[i]] -> Coord);
			    }
			}
		    }
		    else {
			if (VLast == NULL) {
			    VLast = P -> PVertex = V;
			}
			else {
			    VLast -> Pnext = V;
			    VLast = V;
			}

			if (VertexArraySize - 2 < VertexArrayIndex) {
			    VertexArray = (IPVertexStruct **)
			        IritRealloc(VertexArray,
					    sizeof(IPVertexStruct *) *
					        VertexArraySize,
					    sizeof(IPVertexStruct *) *
					        VertexArraySize * 2);
			    VertexArraySize *= 2;
			}
			VertexArray[VertexArrayIndex++] = V;
		    }

		    SkipRead = TRUE;
		}
		else if (strncmp(Line, "SEQEND", 6) == 0)
		    Done = TRUE;
		break;
	    case 38:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Elevation);
		break;
	    case 39:
		sscanf(Line, SCANF_FLOAT_FORMAT, &Thickness);
		break;
	    case 210:
	    case 220:
	    case 230:
	        {
		    int CoordIndex = ((Code - 200) / 10) - 1;

		    sscanf(Line, SCANF_FLOAT_FORMAT, &ExtrudeDir[CoordIndex]);
		}
		break;
	    case 5:
	    case 6:
	    case 10:
	    case 20:
	    case 30:
	    case 40:
	    case 41:
	    case 62:
	    case 66:
	    case 67:
		break;
	    default:
		IRIT_WARNING_MSG_PRINTF("Unknown code %d in POLYLINE, line %d\n",
			                Code, GlblLineCount);
		break;
	}
    }

    /* Update elevation information, if any. */
    for (V = P -> PVertex; V != NULL; V = V -> Pnext)
	 V -> Coord[2] += Elevation;

    if (IsSurface) {                 /* Convert the polyline into a surface. */
	int i, j;
    	CagdRType **Points;

	V = P -> PVertex;

	if (ULength * VLength > NumVertices) {
	    IRIT_WARNING_MSG_PRINTF(
		    "Polyline has %d vertices, should have %d vertices.\n",
		    NumVertices, ULength * VLength);
	    IRIT_WARNING_MSG_PRINTF("\tVLength shortened from %d ", VLength);
	    while (ULength * VLength > NumVertices)
		VLength--;
	    IRIT_WARNING_MSG_PRINTF("to %d\n", VLength);
	}

	if (IsClosedN)
	    ULength++;
	if (IsClosedM)
	    VLength++;

	switch (SurfaceType) {
	    default:
	    	IRIT_WARNING_MSG_PRINTF(
		    "Surface type %d is not support, type 0 selected, line %d.\n",
	    	    SurfaceType, GlblLineCount);
	    case 0:
		/* Implement as a piecewise linear Bspline surface. */
		PSrf = BspSrfNew(ULength, VLength, 2, 2, CAGD_PT_E3_TYPE);
		if (GlblFloatEndCondition) {
		    BspKnotUniformFloat(ULength, 2, PSrf -> UKnotVector);
		    BspKnotUniformFloat(VLength, 2, PSrf -> VKnotVector);
		}
		else {
		    BspKnotUniformOpen(ULength, 2, PSrf -> UKnotVector);
		    BspKnotUniformOpen(VLength, 2, PSrf -> VKnotVector);
		}
		break;
	    case 5:
		ULength = SmoothULength;
		VLength = SmoothVLength;
		PSrf = BspSrfNew(ULength, VLength, 3, 3, CAGD_PT_E3_TYPE);
		if (GlblFloatEndCondition) {
		    BspKnotUniformFloat(ULength, 3, PSrf -> UKnotVector);
		    BspKnotUniformFloat(VLength, 3, PSrf -> VKnotVector);
		}
		else {
		    BspKnotUniformOpen(ULength, 3, PSrf -> UKnotVector);
		    BspKnotUniformOpen(VLength, 3, PSrf -> VKnotVector);
		}
		break;
	    case 6:
		ULength = SmoothULength;
		VLength = SmoothVLength;
		PSrf = BspSrfNew(ULength, VLength, 4, 4, CAGD_PT_E3_TYPE);
		if (GlblFloatEndCondition) {
		    BspKnotUniformFloat(ULength, 4, PSrf -> UKnotVector);
		    BspKnotUniformFloat(VLength, 4, PSrf -> VKnotVector);
		}
		else {
		    BspKnotUniformOpen(ULength, 4, PSrf -> UKnotVector);
		    BspKnotUniformOpen(VLength, 4, PSrf -> VKnotVector);
		}
		break;
	    case 8:
		ULength = SmoothULength;
		VLength = SmoothVLength;
		PSrf = BzrSrfNew(ULength, VLength, CAGD_PT_E3_TYPE);
		break;
	}

	/* Copy the vertices into the control mesh. */
	Points = PSrf -> Points;
	for (j = 0; j < VLength; j++) {
	    if (j == VLength - 1 && IsClosedM) {
		/* Duplicate the first row as last and quit. */
		IRIT_GEN_COPY(&Points[1][j * ULength], &Points[1][0],
			 sizeof(CagdRType) * ULength);
		IRIT_GEN_COPY(&Points[2][j * ULength], &Points[2][0],
			 sizeof(CagdRType) * ULength);
		IRIT_GEN_COPY(&Points[3][j * ULength], &Points[3][0],
			 sizeof(CagdRType) * ULength);
		break;
	    }
	      
	    for (i = 0; i < ULength; i++) {
		int Index = j * ULength + i;

		if (i == ULength - 1 && IsClosedN) {
		    /* Duplicate the first point as past. */
		    Points[1][Index] = Points[1][j * ULength];
		    Points[2][Index] = Points[2][j * ULength];
		    Points[3][Index] = Points[3][j * ULength];
		}
		else {
		    Points[1][Index] = V -> Coord[0];
		    Points[2][Index] = V -> Coord[1];
		    Points[3][Index] = V -> Coord[2];

		    V = V -> Pnext;
		}
	    }
	}

	IPFreePolygonList(P);

	PObj = IPGenSRFObject(PSrf);
    }
    else if (IsPolyfaceMesh) {
	IPFreePolygon(P);

	PObj = IPGenPOLYObject(PFace);
	IP_SET_POLYGON_OBJ(PObj);

	if (!IRIT_APX_EQ(Thickness, 0.0)) {
	    IPObjectStruct *PObjExt;

	    IRIT_PT_SCALE(ExtrudeDir, Thickness);
	    PObjExt = PrimGenEXTRUDEObject(PObj, ExtrudeDir, 3);

	    IPFreeObject(PObj);
	    PObj = PObjExt;
	}
    }
    else {
	if (IsClosedM) {
	    VLast -> Pnext = IPAllocVertex2(NULL);
	    IRIT_PT_COPY(VLast -> Pnext -> Coord, P -> PVertex -> Coord);
	}	

	PObj = IPGenPOLYObject(P);
	IP_SET_POLYLINE_OBJ(PObj);

	if (!IRIT_APX_EQ(Thickness, 0.0)) {
	    IPObjectStruct *PObjExt;

	    IRIT_PT_SCALE(ExtrudeDir, Thickness);
	    PObjExt = PrimGenEXTRUDEObject(PObj, ExtrudeDir, 0);

	    IPFreeObject(PObj);
	    PObj = PObjExt;
	}
    }

    IP_SET_OBJ_NAME2(PObj, Name);

    IritFree(VertexArray);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to files.  Queues one previous dumped object in   *
* an attempt to merge similar types.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   DATFile:      IRIT data file to dump object to.                          *
*   PObject:      Object to dump to file f, NULL for last time.              *
*   FreeObj:      If TRUE, free (and merge) the PObject, otherwise dump      *
*		  PObject immediately with no merge attemps.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneObject(FILE *DATFile, IPObjectStruct *PObject, int FreeObj)
{
    IRIT_STATIC_DATA int
	FirstTime = TRUE;
    IRIT_STATIC_DATA char *Header[] = {
	"Creator: by DXF2IRIT - Autocad DXF to IRIT filer.",
	"",
    	"[OBJECT MATRICES",
    	"    [OBJECT VIEW_MAT",
    	"\t[MATRIX",
    	"\t    1 0 0 0",
    	"\t    0 1 0 0",
    	"\t    0 0 1 0",
    	"\t    0 0 0 1",
    	"\t]",
    	"    ]",
    	"]",
    	"",
    	NULL
    };
    IRIT_STATIC_DATA IPObjectStruct
	*PrevObject = NULL;

    if (FirstTime) {
	int i;

	for (i = 0; Header[i] != NULL; i++)
	    fprintf(DATFile, "%s\n", Header[i]);
	FirstTime = FALSE;
    }

    if (!FreeObj)                  /* No merging/freeing - dump immediately. */
	IPPutObjectToFile(DATFile, PObject, GlblBinaryOutput);
    else if (PObject == NULL) {    /* Last time - clear the internal buffer. */
        if (PrevObject) {      /* We actually had something in the dxf file. */
	    IPPutObjectToFile(DATFile, PrevObject, GlblBinaryOutput);
	    IPFreeObject(PrevObject);
	    PrevObject = NULL;
	}
    }
    else if (PrevObject == NULL)         /* First time - no previous object. */
	PrevObject = PObject;
    else if (PrevObject -> ObjType == PObject -> ObjType) {
	if (IP_IS_POLY_OBJ(PrevObject) &&
	    ((IP_IS_POLYGON_OBJ(PrevObject) && IP_IS_POLYGON_OBJ(PObject)) ||
	     (IP_IS_POLYLINE_OBJ(PrevObject) && IP_IS_POLYLINE_OBJ(PObject)))) {
	    IPPolygonStruct
		*PLast = IPGetLastPoly(PrevObject -> U.Pl);

	    PLast -> Pnext = PObject -> U.Pl;
	    PObject -> U.Pl = NULL;
	    IPFreeObject(PObject);
        }
	else {
	    IPPutObjectToFile(DATFile, PrevObject, GlblBinaryOutput);
	    IPFreeObject(PrevObject);
	    PrevObject = PObject;
	}
    }
    else {
	IPPutObjectToFile(DATFile, PrevObject, GlblBinaryOutput);
	IPFreeObject(PrevObject);
	PrevObject = PObject;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Inserts one new object into PObjBlock. Queues one previous inserted object *
* in an attempt to merge similar types.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjBlock:   List object to insert NewObj into.                          *
*   Index:       In list, where to place NewObj.                             *
*   NewObj:      Object to add to list object PObjBlock, NULL for last time. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ListObjectInsertDXF(IPObjectStruct *PObjBlock,
				int *Index,
				IPObjectStruct *NewObj)
{
    IRIT_STATIC_DATA IPObjectStruct
	*PrevObject = NULL;

    if (NewObj == NULL) {         /* Last time - clear the internal buffer. */
	IPListObjectInsert(PObjBlock, (*Index)++, PrevObject);
	if (PrevObject != NULL) {
	    IPListObjectInsert(PObjBlock, (*Index)++, NULL);
	    PrevObject = NULL;
	}
    }
    else if (PrevObject == NULL)        /* First time - no previous object. */
	PrevObject = NewObj;
    else if (PrevObject -> ObjType == NewObj -> ObjType) {
	if (IP_IS_POLY_OBJ(PrevObject) &&
	    ((IP_IS_POLYGON_OBJ(PrevObject) && IP_IS_POLYGON_OBJ(NewObj)) ||
	     (IP_IS_POLYLINE_OBJ(PrevObject) && IP_IS_POLYLINE_OBJ(NewObj)))) {
	    IPPolygonStruct
		*PLast = IPGetLastPoly(PrevObject -> U.Pl);

	    PLast -> Pnext = NewObj -> U.Pl;
	    NewObj -> U.Pl = NULL;
	    IPFreeObject(NewObj);
        }
	else {
	    IPListObjectInsert(PObjBlock, (*Index)++, PrevObject);
	    PrevObject = NewObj;
	}
    }
    else {
	IPListObjectInsert(PObjBlock, (*Index)++, PrevObject);
	PrevObject = NewObj;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dxf2Irit Irit2Ray exit routine.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void Dxf2IritExit(int ExitCode)
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
