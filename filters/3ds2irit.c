/*****************************************************************************
* Filter to convert AutoCad 3DS data files to IRIT .irt files.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
* Based on a 3ds skeleton from Martin van Velsen (vvelsen@ronix.ptf.hro.nl)  *
*       and on further information from Jeff Lewis (werewolf@worldgate.com)  *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "misc_lib.h"
#include "geom_lib.h"

#ifdef __WINNT__
#include <fcntl.h>
#include <io.h>
#endif /* __WINNT__ */

#define DEFAULT_3DS_NAME	"Obj3DS"

#define MAIN3DS       0x4D4D
#define EDIT3DS       0x3D3D      /* This is the start of the editor config. */
#define KEYF3DS       0xB000   /* This is the start of the keyframer config. */

/* Sub defines of EDIT3DS. */

#define EDIT_MATERIAL 0xAFFF
#define EDIT_CONFIG1  0x0100
#define EDIT_CONFIG2  0x3E3D
#define EDIT_VIEW_P1  0x7012
#define EDIT_VIEW_P2  0x7011
#define EDIT_VIEW_P3  0x7020
#define EDIT_VIEW1    0x7001
#define EDIT_BACKGR   0x1200
#define EDIT_AMBIENT  0x2100
#define EDIT_OBJECT   0x4000

#define EDIT_UNKNW01  0x1100
#define EDIT_UNKNW02  0x1201
#define EDIT_UNKNW03  0x1300
#define EDIT_UNKNW04  0x1400
#define EDIT_UNKNW05  0x1420
#define EDIT_UNKNW06  0x1450
#define EDIT_UNKNW07  0x1500
#define EDIT_UNKNW08  0x2200
#define EDIT_UNKNW09  0x2201
#define EDIT_UNKNW10  0x2210
#define EDIT_UNKNW11  0x2300
#define EDIT_UNKNW12  0x2302 /* New chunk type. */
#define EDIT_UNKNW13  0x3000
#define EDIT_UNKNW14  0xAFFF

/* Sub defines of EDIT_MATERIAL. */
#define MAT_NAME      0xA000   /* Includes name (see mli doc for materials). */
#define MAT_AMBIENT   0xA010
#define MAT_DIFFUSE   0xA020
#define MAT_SPECULAR  0xA030
#define MAT_TRANSPARENCY 0xA050

/* Sub defines of EDIT_OBJECT. */

#define OBJ_TRIMESH   0x4100
#define OBJ_LIGHT     0x4600
#define OBJ_CAMERA    0x4700

#define OBJ_UNKNWN01  0x4010
#define OBJ_UNKNWN02  0x4012 /* Could be shadow. */

/* Sub defines of OBJ_CAMERA. */
#define CAM_UNKNWN01  0x4710 /* New chunk type. */
#define CAM_UNKNWN02  0x4720 /* New chunk type. */

/* Sub defines of OBJ_LIGHT. */
#define LIT_OFF       0x4620
#define LIT_SPOT      0x4610
#define LIT_UNKNWN01  0x465A

/* Sub defines of OBJ_TRIMESH. */
#define TRI_VERTEXL   0x4110
#define TRI_FACEL2    0x4111 /* Unknown yet. */
#define TRI_FACEL1    0x4120
#define TRI_MATERIAL  0x4130
#define TRI_SMOOTH    0x4150
#define TRI_LOCAL     0x4160
#define TRI_VISIBLE   0x4165

/* Sub defs of KEYF3DS. */

#define KEYF_UNKNWN01 0xB009
#define KEYF_UNKNWN02 0xB00A
#define KEYF_FRAMES   0xB008
#define KEYF_OBJDES   0xB002

#define KEYF_OBJHIERARCH  0xB010
#define KEYF_OBJDUMMYNAME 0xB011
#define KEYF_OBJUNKNWN01  0xB013
#define KEYF_OBJUNKNWN02  0xB014
#define KEYF_OBJUNKNWN03  0xB015  
#define KEYF_OBJPIVOT     0xB020  
#define KEYF_OBJUNKNWN04  0xB021  
#define KEYF_OBJUNKNWN05  0xB022  

/* These define the different color chunk types. */
#define COL_RGB  	  0x0010
#define COL_TRU		  0x0011
#define COL_LINTRU	  0x0012
#define COL_LINRGB	  0x0013

/* Defines for viewport chunks. */

#define TOP           0x0001
#define BOTTOM        0x0002
#define LEFT          0x0003
#define RIGHT         0x0004
#define FRONT         0x0005
#define BACK          0x0006
#define USER          0x0007
#define CAMERA        0x0008 /* 0xFFFF is the code read from file. */
#define LIGHT         0x0009
#define DISABLED      0x0010
#define BOGUS         0x0011

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char 
    *VersionStr = "3ds2Irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "3ds2irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",  " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "3ds2Irit m%- M%- c%-ClrScale!F o%-OutName!s b%- z%- 3DSFile!*s";

IRIT_STATIC_DATA const char *ViewPorts[11] = {
    "Bogus",
    "Top",
    "Bottom",
    "Left",
    "Right",
    "Front",
    "Back",
    "User",
    "Camera",
    "Light",
    "Disabled"
};

IRIT_STATIC_DATA FILE *Bin3dsFile, *DATFile;
IRIT_STATIC_DATA char
    GlblObjName[IRIT_LINE_LEN] = DEFAULT_3DS_NAME;
IRIT_STATIC_DATA unsigned char
    GlblViewsRead = 0;
IRIT_STATIC_DATA unsigned int
    GlblBinaryOutput = FALSE,
    GlblMoreFlag = FALSE,
    GlblColorScaleFlag = FALSE,
    GlblNumbFaces = 0,
    GlblNumbVertices = 0;
IRIT_STATIC_DATA float
    TransMat[4][4],                       /* Translation matrix for objects. */
    GlblRGBVal[3] = { 1.0f, 1.0f, 1.0f },	     /* RGB Color of object. */
    *GlblVertices = NULL;		            /* Array of (x, y, z)'s. */
IRIT_STATIC_DATA IrtRType
    GlblColorScale = 1.0;

static unsigned char ReadChar(void);
static unsigned int ReadInt(void);
static unsigned long ReadLong(void);
static unsigned long ReadChunkPointer(void);
static unsigned long GetChunkPointer(void);
static void ChangeChunkPointer(unsigned long TempPointer);
static int ReadName(unsigned char *Name);
static int ReadLongName(unsigned char *Name);
static unsigned long ReadUnknownChunk(unsigned int ChunkId);
static unsigned long ReadRGBColor(void);
static unsigned long ReadTrueColor(void);
static unsigned long ReadBooleanChunk(unsigned char *Boolean);
static unsigned long ReadSpotChunk(void);
static unsigned long ReadLightChunk(void);
static unsigned long ReadCameraChunk(void);
static unsigned long ReadVerticesChunk(void);
static unsigned long ReadSmoothingChunk(void);
static unsigned long ReadFacesChunk(void);
static unsigned long ReadTranslationChunk(void);
static unsigned long ReadObjChunk(void);
static unsigned long ReadObjectChunk(void);
static unsigned long ReadBackgrChunk(void);
static unsigned long ReadAmbientChunk(void);
static unsigned long FindCameraChunk(void);
static unsigned long ReadViewPortChunk(void);
static unsigned long ReadViewChunk(void);
static unsigned long ReadMatDefChunk(void);
static unsigned long ReadMaterialChunk(void);
static unsigned long ReadEditChunk(void);
static unsigned long ReadKeyfChunk(void);
static unsigned long ReadMainChunk(void);
static int ReadPrimaryChunk(void);
static void ThreeDs2IritExit(int ExitCode);

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
	MOREFlag = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	NumFiles = 0;
    char
	*OutFileName = NULL,
	**FileNames = NULL;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GlblMoreFlag, &MOREFlag,
			   &GlblColorScaleFlag, &GlblColorScale,
			   &OutFileFlag, &OutFileName,
			   &GlblBinaryOutput, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	ThreeDs2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	ThreeDs2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	ThreeDs2IritExit(1);
    }
    else if (NumFiles > 1) {
	IRIT_WARNING_MSG("Cannot handle more than one 3DS file at a time, exit.\n");
	GAPrintHowTo(CtrlStr);
	ThreeDs2IritExit(1);
    }

    if (MOREFlag)
	GlblMoreFlag = 2;

    if (strcmp(FileNames[0], "-") == 0)
	Bin3dsFile = stdin;
    else if ((Bin3dsFile = fopen(FileNames[0], "r")) == NULL) {
	IRIT_WARNING_MSG_PRINTF("Cannot open 3DS file \"%s\", exit.\n", FileNames[0]);
	ThreeDs2IritExit(1);
    }

    /* Make sure it is in binary mode. */
#   if defined(__OS2GCC__)
        setmode(fileno(Bin3dsFile), O_BINARY);
#   endif /* __OS2GCC__ */
#   if defined(__WINNT__)
        _setmode(_fileno(Bin3dsFile), _O_BINARY);
#   endif /* __WINNT__ */

    if (OutFileName != NULL) {
	if ((DATFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    ThreeDs2IritExit(2);
	}
    }
    else
	DATFile = stdout;

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("\nLoading 3ds binary file: %s\n", FileNames[0]);
    }

    /* Get the 3ds file. */
    while (ReadPrimaryChunk() == 0);

    fclose(Bin3dsFile);
    fclose(DATFile);

    ThreeDs2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one char/integer/long from input stream.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned char/int/long/float:  Read character/int/long/float.            *
*****************************************************************************/
static unsigned char ReadChar(void)
{
    if (feof(Bin3dsFile)) {
        IRIT_WARNING_MSG("Error: premature termination of 3DS file\n");
	ThreeDs2IritExit(5);
    }

    return fgetc(Bin3dsFile);
}

static unsigned int ReadInt(void)
{
    unsigned int
	Temp = ReadChar();

    return Temp | (ReadChar() << 8);
}

static unsigned long ReadLong(void)
{
    unsigned long
	Temp = ReadInt();

    return Temp + (ReadInt() << 16);
}

static float ReadFloat(void)
{
    unsigned long
	Temp = ReadLong();
    float
        *f = (float *) &Temp;

    return *f;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a pointer                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long: pointer.                                                  *
*****************************************************************************/
static unsigned long ReadChunkPointer(void)
{
    return ReadLong();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long GetChunkPointer(void)
{
    return ftell(Bin3dsFile) - 2; /* Compensate for the already read Marker. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   long:                                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ChangeChunkPointer(unsigned long TempPointer)
{
    fseek(Bin3dsFile, (long) TempPointer, SEEK_SET);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int ReadName(unsigned char *Name)
{
    unsigned int
	Teller = 0,
	ValidName = TRUE;
    unsigned char Letter;

    if ((Letter = ReadChar()) == 0)
	return -1;		/* Dummy object. */
    Name[Teller++] = Letter;
    if (Letter < ' ' || Letter & 0x80)
	ValidName = FALSE;

    do {
	Name[Teller++] = Letter = ReadChar();
	if (Letter < ' ' || Letter & 0x80)
	    ValidName = FALSE;
    }
    while ((Letter != 0) && (Teller < 12));

    if (ValidName) {
	Name[Teller - 1] = 0;
    }
    else {
	strcpy((char *) Name, DEFAULT_3DS_NAME);
    }

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found Name: %s\n", Name);
    }
    
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int ReadLongName(unsigned char *Name)
{
    unsigned int
	Teller = 0;
    unsigned char Letter;

    strcpy((char *) Name, DEFAULT_3DS_NAME);

    if ((Letter = ReadChar()) == 0)
	return -1; /* Dummy object. */

    Name[Teller++] = Letter;

    do {
	GlblObjName[Teller++] = Letter = ReadChar();
    }
    while (Letter != 0);

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found Name: %s\n", Name);
    }

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   ChunkId:                                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadUnknownChunk(unsigned int ChunkId)
{
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();

    ChunkId = ChunkId;

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadRGBColor(void)
{
    int i;

    for (i = 0; i < 3; i++)
	GlblRGBVal[i] = ReadFloat();

    if (GlblColorScaleFlag || GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF(
		"Found Color (RGB) def of: R:%5.2f, G:%5.2f, B:%5.2f\n",
		GlblRGBVal[0], GlblRGBVal[1], GlblRGBVal[2]);
    }

    return 12L;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadTrueColor(void)
{
    int i;

    for (i = 0; i < 3; i++)
	GlblRGBVal[i] = (float) (((IrtRType) ReadChar()) / 255.0);
    
    if (GlblColorScaleFlag || GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF(
		"Found Color (RGB) def of: R:%5.2f, G:%5.2f, B:%5.2f\n",
		GlblRGBVal[0], GlblRGBVal[1], GlblRGBVal[2]);
    }

    return 3L;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Boolean:                                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long                                                            *
*****************************************************************************/
static unsigned long ReadBooleanChunk(unsigned char *Boolean)
{
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();

    *Boolean = ReadChar();

    /* Move to the new chunk position. */
    ChangeChunkPointer(CurrentPointer + TempPointer);
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadSpotChunk(void)
{
    int i;
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();
    float Target[4], HotSpot, FallOff;

    for (i = 0; i < 3; i++)
	Target[i] = ReadFloat();
    HotSpot = ReadFloat();
    FallOff = ReadFloat();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("The target of the spot is at: X:%5.2f Y:%5.2f Y:%5.2f\n",
	                     Target[0], Target[1], Target[2]);
	IRIT_INFO_MSG_PRINTF("The hotspot of this light is: %5.2f\n", HotSpot);
	IRIT_INFO_MSG_PRINTF("The falloff of this light is: %5.2f\n", FallOff);
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadLightChunk(void)
{
    int i;
    unsigned char Boolean,
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */
    float LightCoors[3];

    for (i = 0; i < 3; i++)
	LightCoors[i] = ReadFloat();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF(
		"Found light at coordinates: X: %5.2f, Y: %5.2f, Z: %5.2f\n", 
		LightCoors[0], LightCoors[1], LightCoors[2]);
    }

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch(TempInt)	{
            case LIT_UNKNWN01:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Light unknown chunk id of %0X\n", LIT_UNKNWN01);
		}
	    
		TellerTje += ReadUnknownChunk(LIT_UNKNWN01);
		break;
	    case LIT_OFF:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Light is(on/off) chunk: %0X\n", LIT_OFF);
		}
	    
		TellerTje += ReadBooleanChunk(&Boolean);
		if (GlblMoreFlag) {
		    if (Boolean == TRUE)
			IRIT_INFO_MSG("Light is on\n");
		    else
			IRIT_INFO_MSG("Light is off\n");
		}
		break;
	    case LIT_SPOT:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Light is SpotLight: %0X\n", TRI_VERTEXL);
		}
	    
		TellerTje += ReadSpotChunk();
		break;
	    case COL_RGB:
	    case COL_LINRGB:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (RGB) chunk id of %0X\n", TempInt);
		}
		
		TellerTje += ReadRGBColor();
		break;
	    case COL_TRU:
	    case COL_LINTRU:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (24bit) chunk id of %0X\n", TempInt);
		}
	    
		TellerTje += ReadTrueColor();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadCameraChunk(void)
{
    int i;
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();
    float CameraEye[3], CameraFocus[3], Rotation, Lens;

    for (i = 0; i < 3; i++)
	CameraEye[i] = ReadFloat();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found Camera viewpoint at coordinates: X: %5.2f, Y: %5.2f, Z: %5.2f\n", 
	                     CameraEye[0], CameraEye[1], CameraEye[2]);
    }

    for (i = 0; i < 3; i++)
	CameraFocus[i] = ReadFloat();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found Camera focus coors at coordinates: X: %5.2f, Y: %5.2f, Z: %5.2f\n", 
	                     CameraFocus[0], CameraFocus[1], CameraFocus[2]);
    }
    

    Rotation = ReadFloat();
    Lens = ReadFloat();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Rotation of camera is:  %5.4f\n", Rotation);
	IRIT_INFO_MSG_PRINTF("Lens in used camera is: %5.4fmm\n", Lens);
    }

    if ((TempPointer - 38) > 0) {   /* This means more chunks are to follow. */
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG("Found extra cam chunks ****\n");
	}
	
	if (ReadInt() == CAM_UNKNWN01) {
	    if (GlblMoreFlag) {
		IRIT_INFO_MSG("Found cam 1 type ch ****\n");
	    }
	    
	    ReadUnknownChunk(CAM_UNKNWN01);
	}
	if (ReadInt() == CAM_UNKNWN02) {
	    if (GlblMoreFlag) {
		IRIT_INFO_MSG("Found cam 2 type ch ****\n");
	    }
	    
	    ReadUnknownChunk(CAM_UNKNWN02);
	}
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadVerticesChunk(void)
{
    unsigned int i;
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();

    GlblNumbVertices = ReadInt();

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found %d vertices\n", GlblNumbVertices);
    }

    if (GlblVertices != NULL)
	IritFree(GlblVertices);

    GlblVertices = IritMalloc(sizeof(float) * 3 * GlblNumbVertices);
    for (i = 0; i < 3 * GlblNumbVertices; i++)
	GlblVertices[i] = ReadFloat();

    for (i = 0; i < GlblNumbVertices; i++) {
	if (GlblMoreFlag == 2) {
	    IRIT_INFO_MSG_PRINTF("Vertex nr%4d: X: %5.2f  Y: %5.2f  Z:%5.2f\n", 
		                 i,
		                 GlblVertices[i * 3],
		                 GlblVertices[i * 3 + 1],
		                 GlblVertices[i * 3 + 2]);
	}
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadSmoothingChunk(void)
{
    unsigned int i;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer();
    unsigned long Smoothing;

    for (i = 0; i < GlblNumbFaces; i++) {
	Smoothing = ReadLong();

	if (GlblMoreFlag) {
	    IRIT_INFO_MSG_PRINTF(
		    "The Smoothing group for face[%5d] is %d\n",
		    i, (int) Smoothing);
	}
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadFacesChunk(void)
{
    unsigned int i, j, RGB[3];
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer();
    unsigned int TempDiff;
    unsigned int Faces[6];	/* a, b, c, Diff(Diff =  AB: BC: CA: ). */
    IPObjectStruct *PObj;
    IPPolygonStruct
	*PlList = NULL;

    GlblNumbFaces = ReadInt();
    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Found %d Faces\n", GlblNumbFaces);
    }    

    for (i = 0; i < GlblNumbFaces; i++) {
	IPVertexStruct
	    *V = NULL;

	Faces[0] = ReadInt();
	Faces[1] = ReadInt();
	Faces[2] = ReadInt();
	TempDiff = ReadInt() & 0x000F;
	Faces[3] = (TempDiff & 0x0004) >> 2;
	Faces[4] = (TempDiff & 0x0002) >> 1;
	Faces[5] = (TempDiff & 0x0001);

	if (GlblMoreFlag == 2) {
	    IRIT_INFO_MSG_PRINTF("Face nr:%d, A: %d  B: %d  C:%d , AB:%d  BC:%d  CA:%d\n", 
		                 i,
		                 Faces[0], Faces[1], Faces[2],
		                 Faces[3], Faces[4], Faces[5]);
	}

	for (j = 0; j < 3; j++) {
	    if (Faces[j] > GlblNumbVertices) {
		IRIT_INFO_MSG("Error: Inconsistent allocated and claimed number of vertices, aborted\n");
		ThreeDs2IritExit(3);
	    }
	    V = IPAllocVertex2(V);
	    V -> Coord[0] = GlblVertices[Faces[j] * 3];
	    V -> Coord[1] = GlblVertices[Faces[j] * 3 + 1];
	    V -> Coord[2] = GlblVertices[Faces[j] * 3 + 2];
	}

	if (j == 3)
	    PlList = IPAllocPolygon(0, V, PlList);
    }

    if (ReadInt() == TRI_SMOOTH)
	ReadSmoothingChunk();
    else if (GlblMoreFlag)
	IRIT_INFO_MSG("No smoothing groups found, assuming autosmooth\n");

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    PObj = IPGenPolyObject(GlblObjName, PlList, NULL);
    for (i = 0; i < 3; i++) {
	RGB[i] = (int) (GlblColorScale * 255.0 * GlblRGBVal[i]);
	RGB[i] = IRIT_BOUND(RGB[i], 0, 255);
    }
    if (RGB[0] == 0 && RGB[1] == 0 && RGB[2] == 0)
        RGB[0] = RGB[1] = RGB[2] = 255;
    AttrSetObjectRGBColor(PObj, RGB[0], RGB[1], RGB[2]);

#   ifdef TRANS_3DS_OBJ
    {
	IPObjectStruct *PTransObj;
	IrtHmgnMatType Mat;

        for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
	        Mat[i][j] = TransMat[i][j];
        PTransObj = GMTransformObject(PObj, Mat);
        IPPutObjectToFile(DATFile, PTransObj, GlblBinaryOutput);

        IPFreeObject(PTransObj);
    }
#   else
	IPPutObjectToFile(DATFile, PObj, GlblBinaryOutput);
#   endif /* TRANS_3DS_OBJ */

    IPFreeObject(PObj);

    /* Reset all attributes. */
    strcpy(GlblObjName, DEFAULT_3DS_NAME);
    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	    TransMat[i][j] = (float) (i == j);

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadTranslationChunk(void)
{
    int i, j;
    unsigned long
	CurrentPointer = GetChunkPointer(),
	TempPointer = ReadChunkPointer();

    for (j = 0; j < 4; j++) {
	for (i = 0; i < 3; i++)
	    TransMat[j][i] = ReadFloat();
    }

    TransMat[0][3] = 0.0f;
    TransMat[1][3] = 0.0f;
    TransMat[2][3] = 0.0f;
    TransMat[3][3] = 1.0f;

    if (GlblMoreFlag) {
	IRIT_INFO_MSG("The translation matrix is:\n");
	for (i = 0; i < 4; i++)
	    IRIT_INFO_MSG_PRINTF("| %5.2f %5.2f %5.2f %5.2f |\n", 
		                 TransMat[i][0], 
		                 TransMat[i][1], 
		                 TransMat[i][2], 
		                 TransMat[i][3]);
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadObjChunk(void)
{
    unsigned char
	EndFound = FALSE,
        Boolean = TRUE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case TRI_VERTEXL:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object vertices chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadVerticesChunk();
		break;
	    case TRI_FACEL1:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object faces chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadFacesChunk();
		break;
	    case TRI_MATERIAL:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object material chunk id of %0X\n", 
			                 TempInt);
		}
	    
		break;
	    case TRI_LOCAL:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object translation chunk id of %0X\n", 
			                 TempInt);
		}
		
		TellerTje += ReadTranslationChunk();
		break;
	    case TRI_VISIBLE:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object vis/invis chunk id of %0X: ", 
			                 TempInt);
		}
		
		TellerTje += ReadBooleanChunk(&Boolean);

		if (GlblMoreFlag) {
		    if (Boolean == TRUE)
			IRIT_INFO_MSG("Object is visible\n");
		    else
			IRIT_INFO_MSG("Object is not visible\n");
		}
		
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadObjectChunk(void)
{
    unsigned char Name[IRIT_LINE_LEN],
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */
    
    if (ReadName(Name) == -1) {
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG("Dummy Object found\n");
	}
    }
    strcpy(GlblObjName, (char *) Name);

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch(TempInt)	{
            case OBJ_UNKNWN01:
	        TellerTje += ReadUnknownChunk(OBJ_UNKNWN01);
		break;
	    case OBJ_UNKNWN02:
		TellerTje += ReadUnknownChunk(OBJ_UNKNWN02);
		break;
	    case OBJ_TRIMESH:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Obj/Mesh chunk id of %0X\n", 
					 OBJ_TRIMESH);
		}

		TellerTje += ReadObjChunk();
		break;
	    case OBJ_LIGHT:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Light chunk id of %0X\n", 
			                 OBJ_LIGHT);
		}
	    
		TellerTje += ReadLightChunk();
		break;
	    case OBJ_CAMERA:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Camera chunk id of %0X\n", 
			                 OBJ_CAMERA);
		}
	    
		TellerTje += ReadCameraChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadBackgrChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch(TempInt) {
            case COL_RGB:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (RGB) chunk id of %0X\n", 
			                 TempInt);
		}

		TellerTje += ReadRGBColor();
		break;
	    case COL_TRU:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (24bit) chunk id of %0X\n", 
			                 TempInt);
		}

		TellerTje += ReadTrueColor();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadAmbientChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case COL_RGB:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (RGB) chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadRGBColor();
		break;
	    case COL_TRU:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (24bit) chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadTrueColor();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long FindCameraChunk(void)
{
    unsigned char Name[IRIT_LINE_LEN];
    int i;
    long TempPointer = 0L;

    for (i = 0; i < 12; i++)
	ReadInt();

    TempPointer = 11L;
    TempPointer = ReadName(Name);

    if (GlblMoreFlag) {
	if (TempPointer == -1)
	    IRIT_INFO_MSG("No Camera name found\n");
    }

    return TempPointer;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadViewPortChunk(void)
{
    int i;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer();
    unsigned int Port, Attribs;

    GlblViewsRead++;

    Attribs = ReadInt();
    if (Attribs == 3) {
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG("<Snap> active in ViewPort\n");
	}
	
    }

    if (Attribs == 5) {
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG("<Grid> active in ViewPort\n");
	}
    }

    for (i = 1; i < 6; i++)
	ReadInt();                    /* Read 5 ints to get to the ViewPort. */

    Port = ReadInt();
    if ((Port == 0xFFFF) || (Port == 0)) {
	FindCameraChunk();
	Port = CAMERA;
    }

    if (GlblMoreFlag) {
	IRIT_INFO_MSG_PRINTF("Reading[%s] information with id:%d\n",
		             ViewPorts[Port], Port);
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadViewChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L;

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case EDIT_VIEW_P1:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found ViewPort1 chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadViewPortChunk();
		break;
	    case EDIT_VIEW_P2:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found ViewPort2(bogus) chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadUnknownChunk(EDIT_VIEW_P2);
		break;
	    case EDIT_VIEW_P3:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found ViewPort chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadViewPortChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;

	if (GlblViewsRead > 3)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Color:                                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned:                                                                *
*****************************************************************************/
static unsigned int ReadColorChunk(float *Color)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case COL_RGB:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (RGB) chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadRGBColor();
		break;
	    case COL_TRU:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Color def (24bit) chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadTrueColor();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadMatDefChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L; /* 2 id + 4 pointer. */
    unsigned char Name[IRIT_LINE_LEN];
    int i = ReadLongName(Name);
    float AmbientColor[3], DiffuseColor[3], SpecularColor[3];

    if (GlblMoreFlag) {
	if (i == -1)
	    IRIT_INFO_MSG("No Material name found\n");
	else
	    IRIT_INFO_MSG_PRINTF("Found material \"%s\"\n", Name);
    }

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case MAT_AMBIENT:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			    "Found ambient material chunk id of %0X\n", 
			    TempInt);
		}
	    
		TellerTje += ReadColorChunk(AmbientColor);
		break;
            case MAT_DIFFUSE:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			    "Found diffuse material chunk id of %0X\n", 
			    TempInt);
		}
	    
		TellerTje += ReadColorChunk(DiffuseColor);
		break;
            case MAT_SPECULAR:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			    "Found specular material chunk id of %0X\n", 
			    TempInt);
		}
	    
		TellerTje += ReadColorChunk(SpecularColor);
		break;
	    case MAT_TRANSPARENCY:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			    "Found transparency material chunk id of %0X\n", 
			    TempInt);
		}

		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;

	if (GlblViewsRead > 3)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 
    
    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadMaterialChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L;

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case MAT_NAME:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Material def chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadMatDefChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadEditChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L;

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
 	    case EDIT_UNKNW01:
	        TellerTje += ReadUnknownChunk(EDIT_UNKNW01);
		break;
	    case EDIT_UNKNW02:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW02);
		break;
	    case EDIT_UNKNW03:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW03);
		break;
	    case EDIT_UNKNW04:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW04);
		break;
	    case EDIT_UNKNW05:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW05);
		break;
	    case EDIT_UNKNW06:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW06);
		break;
	    case EDIT_UNKNW07:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW07);
		break;
	    case EDIT_UNKNW08:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW08);
		break;
	    case EDIT_UNKNW09:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW09);
		break;
	    case EDIT_UNKNW10:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW10);
		break;
	    case EDIT_UNKNW11:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW11);
		break;
	    case EDIT_UNKNW12:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW12);
		break;
	    case EDIT_UNKNW13:
		TellerTje += ReadUnknownChunk(EDIT_UNKNW13);
		break;
	    case EDIT_MATERIAL:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Materials chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadMaterialChunk();
		break;
	    case EDIT_VIEW1:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found View main def chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadViewChunk();
		break;
	    case EDIT_BACKGR:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Backgr chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadBackgrChunk();
		break;
	    case EDIT_AMBIENT:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Ambient chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadAmbientChunk();
		break;
	    case EDIT_OBJECT:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Object chunk id of %0X\n", 
			                 TempInt);
		}
	    
		TellerTje += ReadObjectChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadKeyfChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L;

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt)	{
	    case KEYF_UNKNWN01:
	        TellerTje += ReadUnknownChunk(TempInt);
		break;
	    case KEYF_UNKNWN02:
		TellerTje += ReadUnknownChunk(TempInt);
		break;
	    case KEYF_FRAMES:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			"Found Keyframer frames chunk id of %0X\n", 
			TempInt);
		}
	    
		TellerTje += ReadUnknownChunk(TempInt);
		break;
	    case KEYF_OBJDES:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			"Found Keyframer object description chunk id of %0X\n", 
			TempInt);
		}
	    
		TellerTje += ReadUnknownChunk(TempInt);
		break;
	    case EDIT_VIEW1:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF(
			"Found View main def chunk id of %0X\n", 
			TempInt);
		}
	    
		TellerTje += ReadViewChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned long:                                                           *
*****************************************************************************/
static unsigned long ReadMainChunk(void)
{
    unsigned char
	EndFound = FALSE;
    unsigned int TempInt;
    unsigned long
	CurrentPointer = GetChunkPointer(),
        TempPointer = ReadChunkPointer(),
	TellerTje = 6L;

    while (EndFound == FALSE) {
	TempInt = ReadInt();

	switch (TempInt) {
            case KEYF3DS:
	        if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Keyframer chunk id of %0X\n", KEYF3DS);
		}
	    
		TellerTje += ReadKeyfChunk();
		break;
	    case EDIT3DS:
		if (GlblMoreFlag) {
		    IRIT_INFO_MSG_PRINTF("Found Editor chunk id of %0X\n", EDIT3DS);
		}
	    
		TellerTje += ReadEditChunk();
		break;
	    default:
		break;
	}

	TellerTje += 2;
	if (TellerTje >= TempPointer)
	    EndFound = TRUE;
    }

    ChangeChunkPointer(CurrentPointer + TempPointer); 

    /* Move to the new chunk position. */
    return TempPointer;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int ReadPrimaryChunk(void)
{
    unsigned char Version;

    if (ReadInt() == MAIN3DS) {
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG_PRINTF("Found Main chunk id of %0X\n", MAIN3DS);
	}
	
	/* Find Version number. */
	fseek(Bin3dsFile, 28L, SEEK_SET);
	Version = ReadChar();
	if (GlblMoreFlag) {
	    IRIT_INFO_MSG_PRINTF("Found 3DS file version %d\n", Version);
	}

	fseek(Bin3dsFile, 2, SEEK_SET);
	ReadMainChunk();
    }
    else
	return 1;

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* 3ds2Irit exit routine.	   				             *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ThreeDs2IritExit(int ExitCode)
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
