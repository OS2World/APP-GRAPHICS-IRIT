/*****************************************************************************
* Generic parser for the "Irit" solid modeller.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Sep. 1991   *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "geom_lib.h"
#include "allocate.h"
#include "attribut.h"

#ifdef __WINNT__
#include <fcntl.h>
#include <io.h>
#define popen _popen
#endif /* __WINNT__ */

#ifdef __UNIX__
#include <unistd.h>
#endif /* __UNIX__ */

#if defined(AMIGA) && defined(__SASC)
#include "popen.h"
#endif

/* #define EMPTY_OBJECT_IS_ERROR     If empty data should return with error. */

#define IP_ATTR_HIERARCHY	1000

IRIT_STATIC_DATA int
    GlblPropagateAttrs = TRUE,   /* If TRUE, attributes are propagated down. */
    GlblFlattenObjects = TRUE,	   /* If input list hierarchy is to be kept. */
    GlblFlattenInvisibObjects = TRUE;   /* Should we flatten invisible objs. */

IRIT_STATIC_DATA IPObjectStruct
    *GlblResolveInstObjects = NULL;

IRIT_GLOBAL_DATA IPStreamInfoStruct
    _IPStream[IP_MAX_NUM_OF_STREAMS];
IRIT_GLOBAL_DATA int
    _IPReadOneObject = FALSE,		/* If only one object is to be read. */
    _IPMaxActiveStream = 0;
IRIT_GLOBAL_DATA IPProcessLeafObjType
    _IPGlblProcessLeafFunc = NULL;

static IPObjectStruct *EliminateDegenLists(IPObjectStruct *PObj);
static int InputGetC(int Handler);
static int InputEOF(int Handler);
static int GetStringToken(int Handler, char *StringToken, int *Quoted);
static void GetVertexAttributes(IPVertexStruct *PVertex, int Handler);
static void GetPolygonAttributes(IPPolygonStruct *PPolygon,
				 int Handler);
static void GetObjectAttributes(IPObjectStruct *PObject, int Handler);
static void GetGenericAttribute(IPAttributeStruct **Attr,
				int Handler,
				char *Name);
static void GetPointData(int Handler,
			 IPPolygonStruct *PPolygon,
			 int IsPolygon);
static void IPGetAllObjects(int Handler,
			    IPObjectStruct *PObjParent,
			    int Level);
static void GetNumericToken(int Handler, IrtRType *r);
static void IPGetAuxObject(int Handler, IPObjectStruct *PObj);
static int FindFileHandler(void);
static IPObjectStruct *IPResolveInstancesAux(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Open a data file for read/write.					     M
*   Data file can be either Ascii IRIT data file or binary IRIT data file.   M
*   A binary data file must have a ".ibd" (for Irit Binary Data) file type.  M
*   Under unix, file names with the postfix ".Z" are assumed compressed and  M
* treated accordingly.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   To try and open.                                             M
*   Read:       If TRUE assume a read operation, otheriwse write.            M
*   Messages:   Do we want error/warning message?	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        A handler to the open file, -1 if error.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetObjects, IPSetPolyListCirc, IPSetFlattenObjects,		     M
* IPSetReadOneObject						  	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenDataFile, files, parser                                            M
*****************************************************************************/
int IPOpenDataFile(const char *FileName, int Read, int Messages)
{
    FILE *f;
    int ReadWriteBinary = IPSenseBinaryFile(FileName),
        IsCompressed = IPSenseCompressedFile(FileName),
	IsPipe = FALSE;
    char *p, Cmd[IRIT_LINE_LEN_VLONG];

    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

    if (Read) {
	if (strcmp(FileName, "-") == 0) {
	    f = stdin;
	}
	else if ((p = strrchr(FileName, '.')) != NULL &&
		 (stricmp(p, ".Z") == 0 || stricmp(p, ".gz") == 0)) {
	    sprintf(Cmd, "gzip -cd %s", FileName);/* "zcat %s" is an option. */
#	    if defined(__WINNT__) || defined(__OS2GCC__)
	        f = popen(Cmd, ReadWriteBinary ? "rb" : "r");
#	    else
	        f = popen(Cmd, "r");
#	    endif /* __WINNT__ || __OS2GCC__ */
	    IsPipe = TRUE;
	}
	else {
	    if ((p = strrchr(FileName, '.')) != NULL &&
		(stricmp(p + 1, IRIT_TEXT_DATA_FILE) == 0 ||
		 stricmp(p + 1, IRIT_BINARY_DATA_FILE) == 0 ||
		 stricmp(p + 1, IRIT_MATRIX_DATA_FILE) == 0)) {
	        /* The file name has a file type. */
	        f = fopen(FileName, "r");
	    }
	    else if (IsCompressed) {
	        /* The file is compressed data file, open it as compressed. */
	        f = _IPC_OPEN_FILE(FileName, IP_READ_BIN_MODE);
	    }
	    else {
	        /* The file name has no file type - try openning it with no  */
	        /* file type and if fails, append text file type and retry.  */
		if ((f = fopen(FileName, "r")) == NULL) {
		    char FileNameType[IRIT_LINE_LEN_VLONG];

		    sprintf(FileNameType, "%s.%s", FileName,
			    IRIT_TEXT_DATA_FILE);
		    f = fopen(FileNameType, "r");
		}
	    }
	}
    }
    else { /* Write */
	if (strcmp(FileName, "-") == 0) {
	    f = stdout;
	}
	else if ((p = strrchr(FileName, '.')) != NULL &&
		 stricmp(p, ".Z") == 0) {
	    sprintf(Cmd, "compress > %s", FileName);
#	    if defined(__WINNT__) || defined(__OS2GCC__)
	        f = popen(Cmd, ReadWriteBinary ? "wb" : "w");
#	    else
	        f = popen(Cmd, "w");
#	    endif /* __WINNT__ || __OS2GCC__ */
	    IsPipe = TRUE;
	}
	else if ((p = strrchr(FileName, '.')) != NULL &&
		 stricmp(p, ".gz") == 0) {
	    sprintf(Cmd, "gzip > %s", FileName);
#	    if defined(__WINNT__) || defined(__OS2GCC__)
	        f = popen(Cmd, ReadWriteBinary ? "wb" : "w");
#	    else
	        f = popen(Cmd, "w");
#	    endif /* __WINNT__ || __OS2GCC__ */
	    IsPipe = TRUE;
	}
	else {
	    if ((p = strrchr(FileName, '.')) != NULL &&
		(stricmp(p + 1, IRIT_TEXT_DATA_FILE) == 0 ||
		 stricmp(p + 1, IRIT_BINARY_DATA_FILE) == 0 ||
		 stricmp(p + 1, IRIT_MATRIX_DATA_FILE) == 0)) {
	        /* The file name has a file type. */
	        f = fopen(FileName, "w");
	    } else if (IsCompressed) {
	        /* The file is compressed. */
	        f = _IPC_OPEN_FILE(FileName,  IP_WRITE_BIN_MODE);
            }
	    else {
	        /* The file name has no file type - try openning it with no  */
	        /* file type and if fails, append text file type and retry.  */
		if ((f = fopen(FileName, "r")) == NULL) {
		    char FileNameType[IRIT_LINE_LEN_VLONG];

		    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

		    sprintf(FileNameType, "%s.%s", FileName,
			    IRIT_TEXT_DATA_FILE);
		    f = fopen(FileNameType, "w");
		}
	    }
	}
    }

    if (f == NULL) {
	if (Messages)
	    IRIT_WARNING_MSG_PRINTF("Can't open data file %s.\n", FileName);
	return -1;
    }
    else {
	int Handle = IPOpenStreamFromFile2(f, Read,
					   IPSenseFileType(FileName),
					   ReadWriteBinary, 
                                           IsCompressed,
                                           IsPipe);

	strncpy(_IPStream[Handle].FileName, FileName, IRIT_LINE_LEN_VLONG);

	return Handle;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to detect the type of the file from its name.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:    The files' name to sense file type from.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPStreamFormatType:  Type of file detected.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSenseBinaryFile                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSenseFileType                                                          M
*****************************************************************************/
IPStreamFormatType IPSenseFileType(const char *FileName)
{
    char Type[IRIT_LINE_LEN_VLONG], *p;

    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

    strncpy(Type, FileName, IRIT_LINE_LEN_VLONG - 5);
    p = strrchr(Type, '.');

    /* Make sure we skip the compression suffix, if any. */
    if ((p = strrchr(Type, '.')) != NULL &&
	(stricmp(p, ".Z") == 0 || stricmp(p, ".gz") == 0)) {
        *p = 0;
	p = strrchr(Type, '.');
    }

    if (p == NULL)
	return IP_IDAT_FILE;

    if (stricmp(p + 1, IRIT_TEXT_DATA_FILE) == 0 ||
        stricmp(p + 1, IRIT_COMPRESSED_DATA_FILE) == 0 ||
	stricmp(p + 1, IRIT_BINARY_DATA_FILE) == 0)
	return IP_IDAT_FILE;
    else if (stricmp(p, ".wrl") == 0 || stricmp(p, ".vrl") == 0)
	return IP_VRML_FILE;
    else if (stricmp(p, ".hgl") == 0 || stricmp(p, ".hpgl") == 0)
	return IP_HPGL_FILE;
    else if (stricmp(p, ".ps") == 0 || stricmp(p, ".eps") == 0)
	return IP_PS_FILE;
    else if (stricmp(p, ".igs") == 0 || stricmp(p, ".iges") == 0)
	return IP_IGS_FILE;
    else if (stricmp(p, ".cnc") == 0 ||
	     stricmp(p, ".nc") == 0 ||
	     stricmp(p, ".gcode") == 0)
	return IP_GCODE_FILE;
    else if (stricmp(p, ".stl") == 0 ||
	     stricmp(p, ".bstl") == 0)
	return IP_STL_FILE;
    else if (stricmp(p, ".nff") == 0)
	return IP_NFF_FILE;
    else if (stricmp(p, ".obj") == 0)
	return IP_OBJ_FILE;
    else if (stricmp(p, ".off") == 0)
	return IP_OFF_FILE;
    else if (stricmp(p, ".plg") == 0)
	return IP_PLG_FILE;
    else if (stricmp(p, ".pov") == 0)
	return IP_POV_FILE;
    else if (stricmp(p, ".ray") == 0)
	return IP_RAY_FILE;
    else if (stricmp(p, ".scn") == 0)
	return IP_SCN_FILE;
    else if (stricmp(p, ".xfg") == 0)
	return IP_XFG_FILE;
    else
	return IP_IDAT_FILE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Senses if a given file (name) is a binary or a text file.                M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:  File to sense.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if binary, FALSE if text.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSenseFileType                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSenseBinaryFile                                                        M
*****************************************************************************/
int IPSenseBinaryFile(const char *FileName)
{
    char Str[IRIT_LINE_LEN_VLONG], *p;

    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

    strncpy(Str, FileName, IRIT_LINE_LEN_VLONG - 5);
    p = strrchr(Str, '.');

    if ((p = strrchr(Str, '.')) != NULL &&
	(stricmp(p, ".Z") == 0 || stricmp(p, ".gz") == 0)) {
        *p = 0;
	p = strrchr(Str, '.');
    }

    return p == NULL ? FALSE
		     : stricmp(p + 1, IRIT_BINARY_DATA_FILE) == 0 ||
                       stricmp(p + 1, STL_BINARY_DATA_FILE) == 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Senses if a given file (name) is a compressed file.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:  File to sense.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if compressed, FALSE else.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSenseFileType                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSenseCompressedFile                                                    M
*****************************************************************************/
int IPSenseCompressedFile(const char *FileName)
{
#ifdef IPC_BIN_COMPRESSION
    char Str[IRIT_LINE_LEN_VLONG], *p;

    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

    strncpy(Str, FileName, IRIT_LINE_LEN_VLONG - 5);
    p = strrchr(Str, '.');

    if ((p = strrchr(Str, '.')) != NULL &&
	(stricmp(p, ".Z") == 0 || stricmp(p, ".gz") == 0)) {
        *p = 0;
	p = strrchr(Str, '.');
    }

    return p == NULL ? FALSE : stricmp(p + 1, IRIT_COMPRESSED_DATA_FILE) == 0;
#else
    return FALSE;
#endif /* IPC_BIN_COMPRESSION */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Close a data file for read/write.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:     A handler to the open stream.				     M
*   Free:	 If TRUE, release content.      			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCloseStream, files, stream, parser                                     M
*****************************************************************************/
void IPCloseStream(int Handler, int Free)
{
    if (Handler >= 0 && Handler < IP_MAX_NUM_OF_STREAMS) {
	if (Free) {
	    if (_IPStream[Handler].f != NULL) {
#ifdef __UNIX__
		if (_IPStream[Handler].IsPipe)
		    pclose(_IPStream[Handler].f);
		else
#endif /* __UNIX__ */
		if (_IPStream[Handler].f != stdin &&
		    _IPStream[Handler].f != stdout &&
                    _IPStream[Handler].f != stderr) {
                        if (_IPStream[Handler].FileType == IP_FILE_COMPRESSED) {
			    _IPC_CLOSE_ALL(Handler);
                        }
                        else
		            fclose(_IPStream[Handler].f);
		    }
            }

	    if (_IPStream[Handler].Soc) {
#		ifdef __UNIX__
		    close(_IPStream[Handler].Soc);
#		endif /* __UNIX__ */
#		ifdef __WINNT__
		    closesocket(_IPStream[Handler].Soc);
#		endif /* __WINNT__ */
	    }
	}

	_IPStream[Handler].InUse = FALSE;

	while (_IPMaxActiveStream > 0 &&
	       !_IPStream[_IPMaxActiveStream - 1].InUse)
	    _IPMaxActiveStream--;
    }
    else
	IP_FATAL_ERROR(IP_ERR_INVALID_STREAM_HNDL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Open a stream using direct call back function(s) to read/write data.     M
*                                                                            *
* PARAMETERS:                                                                M
*   ReadFunc:      A call back function to read a character (non blocking).  M
*                  will be ignored if Read == FALSE.			     M
*   WriteFunc:     A call back function to write a block of data.            M
*                  will be ignored if Read == TRUE.			     M
*   Read:          TRUE for reading from f, FALSE for writing to f.          M
*   IsBinary:      Is it a binary file?                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       A handle on the constructed stream.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOpenStreamFromFile, IPProcessFreeForm2                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenStreamFromCallBackIO                                               M
*****************************************************************************/
int IPOpenStreamFromCallBackIO(IPStreamReadCharFuncType ReadFunc,
			       IPStreamWriteBlockFuncType WriteFunc,
			       int Read,
			       int IsBinary)
{
    int Handler = FindFileHandler();

    if (Handler >= 0) {
	_IPStream[Handler].f = NULL;
	_IPStream[Handler].ReadCharFunc = Read ? ReadFunc : NULL;
	_IPStream[Handler].WriteBlockFunc = Read ? WriteFunc : NULL;
	_IPStream[Handler].Read = Read;
	_IPStream[Handler].Format = IP_IDAT_FILE;
        _IPStream[Handler].FileType = IsBinary ? IP_FILE_BINARY : IP_FILE_TEXT;
        _IPStream[Handler].QntError = IPC_QUANTIZATION_DEFAULT;
        _IPStream[Handler].IsPipe = FALSE;
	_IPStream[Handler].FileName[0] = 0;
    }

    return Handler;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts an open file into a stream.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   f:            A handle to the open file.                                 M
*   Read:         TRUE for reading from f, FALSE for writing to f.           M
*   IsBinary:     Is it a binary file?                                       M
*   IsCompressed: Is it compressed file?                                     M
*   IsPipe:       Is it a pipe?                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       A handle on the constructed stream.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOpenStreamFromCallBackIO, IPProcessFreeForm2                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenStreamFromFile                                                     M
*****************************************************************************/
int IPOpenStreamFromFile(FILE *f, 
                         int Read, 
                         int IsBinary, 
                         int IsCompressed, 
                         int IsPipe)
{
    return IPOpenStreamFromFile2(f, Read, IP_IDAT_FILE, 
                                 IsBinary, IsCompressed, IsPipe);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts an open file into a stream.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   f:            A handle to the open file.                                 M
*   Read:         TRUE for reading from f, FALSE for writing to f.           M
*   Format:       IRIT Dat, VRML, etc.					     M
*   IsBinary:     Is it a binary file?                                       M
*   IsCompressed: Is it compressed file?                                     M
*   IsPipe:       Is it a pipe?                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       A handle on the constructed stream.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOpenStreamFromFile, IPOpenStreamFromCallBackIO                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenStreamFromFile2, IPOpenStreamFromFile                              M
*****************************************************************************/
int IPOpenStreamFromFile2(FILE *f, 
			  int Read, 
			  IPStreamFormatType Format,
			  int IsBinary, 
                          int IsCompressed,
			  int IsPipe)
{
    int Handler = FindFileHandler();
    IPFileType FileType;
    
    if (IsCompressed)
         FileType = IP_FILE_COMPRESSED;
    else if (IsBinary) 
         FileType = IP_FILE_BINARY;
    else
         FileType = IP_FILE_TEXT;

#if defined(__OS2GCC__)
    if (IsBinary || IsCompressed)
	setmode(fileno(f), O_BINARY);    /* Make sure it is in binary mode. */
#endif /* __OS2GCC__ */
#if defined(__WINNT__)
    if (IsBinary || IsCompressed)
	_setmode(_fileno(f), _O_BINARY); /* Make sure it is in binary mode. */
#endif /* __WINNT__ */

    if (Handler >= 0) {
        _IPStream[Handler].f = f;
	_IPStream[Handler].ReadCharFunc = NULL;
	_IPStream[Handler].WriteBlockFunc = NULL;
	_IPStream[Handler].Read = Read;
        _IPStream[Handler].Format = Format;
	_IPStream[Handler].FileType = FileType;
        _IPStream[Handler].QntError = IPC_QUANTIZATION_DEFAULT;        
	_IPStream[Handler].IsPipe = IsPipe;
	_IPStream[Handler].FileName[0] = 0;
    }

    return Handler;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts an open socket into a stream.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Soc:       A handle to the open socket.                                  M
*   IsBinary:  Is it a binary file?                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       A handle on the constructed stream.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenStreamFromSocket                                                   M
*****************************************************************************/
int IPOpenStreamFromSocket(int Soc, int IsBinary)
{
    int Handler = FindFileHandler();

    if (Handler >= 0) {
	_IPStream[Handler].Soc = Soc;
	_IPStream[Handler].ReadCharFunc = IPSocReadCharNonBlock;
	_IPStream[Handler].WriteBlockFunc = IPSocWriteBlock;
	_IPStream[Handler].Format = IP_IDAT_FILE;
	_IPStream[Handler].FileType = IsBinary? IP_FILE_BINARY : IP_FILE_TEXT;
        _IPStream[Handler].QntError = IPC_QUANTIZATION_DEFAULT;
	_IPStream[Handler].f = NULL;
	_IPStream[Handler].FileName[0] = 0;
    }

    return Handler;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Searches and returns a free Open File handler.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     Free handler, or -1 if none found.                              *
*****************************************************************************/
static int FindFileHandler(void)
{
    int i,
	Handler = -1;

    for (i = 0; i < IP_MAX_NUM_OF_STREAMS; i++) {
	if (!_IPStream[i].InUse) {
	    _IPStream[i].InUse = TRUE;
	    _IPStream[i].TokenStackPtr = 0;
	    _IPStream[i].LineNum = 0;
	    _IPStream[i].UnGetChar = -1;
	    _IPStream[i].BufferSize = 0;
	    _IPStream[i].BufferPtr = 0;
	    _IPStream[i].Soc = -1;
	    _IPStream[i].f = NULL;
	    _IPStream[i].FileName[0] = 0;
	    Handler = i;
	    break;
	}
    }

    if (i < 0)
	IP_FATAL_ERROR(IP_ERR_STREAM_TBL_FULL);

    if (_IPMaxActiveStream <= i)
	_IPMaxActiveStream = i + 1;

    return Handler;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads data from a set of files specified by file names.		     M
*    Messages and MoreMessages controls the level of printout. 		     M
*    Freeform geometry read in is handed out to a call back function named   M
* IPProcessFreeForm before it is returned from this routine. This is done    M
* so applications that do not want to deal with freeform shapes will be      M
* able to provide a call back that processes the freeform shapes into        M
* other geometry such as polygons.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   DataFileNames:    Array of strings (file names) to process.              M
*   NumOfDataFiles:   Number of elements in DataFileNames.                   M
*   Messages:         Do we want error messages?                             M
*   MoreMessages:     Do we want informative messages?                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Objects read from all files, NULL if error.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPProcessFreeForm                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetDataFiles, files, parser                                            M
*****************************************************************************/
IPObjectStruct *IPGetDataFiles(char const * const *DataFileNames,
			       int NumOfDataFiles,
			       int Messages,
			       int MoreMessages)
{
    int	i, Handler;
    const char *ErrorMsg;
    IPObjectStruct
	*PObjHead = NULL;

    for	(i = 0; i < NumOfDataFiles; i++) {
	if (MoreMessages)
	    IRIT_INFO_MSG_PRINTF("Reading data file %s\n", *DataFileNames);

	if ((Handler = IPOpenDataFile(*DataFileNames, TRUE, Messages)) < 0)
	    continue;

	PObjHead = IPAppendObjLists(IPGetObjects(Handler), PObjHead);

	if (Messages && IPHasError(&ErrorMsg))
	    IRIT_WARNING_MSG_PRINTF("File %s, %s\n", *DataFileNames, ErrorMsg);

	IPCloseStream(Handler, TRUE);

	DataFileNames++;			  /* Skip to next file name. */
    }

    if (PObjHead == NULL) {
	if (Messages) {
	    IPHasError(&ErrorMsg);
	    IRIT_WARNING_MSG_PRINTF("No data found. %s\n",
				    ErrorMsg == NULL ? "" : ErrorMsg);
	}
	return NULL;
    }

    return PObjHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convenience function for reading input from file handles instead         M
* of indirectly from filenames; caller must supply an array of               M
* filename extensions (as char * strings) for ascertaining the input         M
* files' formats.                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Files:            Array of file handles to process.                      M
*   NumOfFiles:       Number of elements in Files.                           M
*   Extensions:       Array of file name extensions for the files;           M
*                     used to determine file formats.                        M
*   Messages:         Do we want error messages?                             M
*   MoreMessages:     Do we want informative messages?                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Objects read from all filehandles.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetDataFiles, IPGetDataFromFilehandles2                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetDataFromFilehandles, filehandles, parser                            M
*****************************************************************************/
IPObjectStruct *IPGetDataFromFilehandles(FILE **Files,
					 int NumOfFiles,
					 char **Extensions,
					 int Messages,
					 int MoreMessages)
{
    int i,
	*IsBinaryIndicators = IritMalloc(NumOfFiles * sizeof(int));
    IPStreamFormatType
	*Formats = IritMalloc(NumOfFiles * sizeof(IPStreamFormatType));
    IPObjectStruct *RetVal;

    for (i = 0; i < NumOfFiles; i++) {
	Formats[i] = IPSenseFileType(Extensions[i]);
	IsBinaryIndicators[i] = IPSenseBinaryFile(Extensions[i]);
    }

    RetVal = IPGetDataFromFilehandles2(Files, NumOfFiles, Formats,
				       IsBinaryIndicators, 
				       Messages, MoreMessages);

    IritFree(Formats);
    IritFree(IsBinaryIndicators);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convenience function for reading input from file handles instead         M
*   of indirectly from filenames; caller must supply an array of             M
*   file formats and 'IsBinary' indicators.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Files:               Array of file handles to process.                   M
*   NumOfFiles:          Number of elements in Files.                        M
*   Formats:             Array of file formats for the files                 M
*   IsBinaryIndicators:  Array of 'IsBinary' indications for the files       M
*   Messages:            Do we want error messages?                          M
*   MoreMessages:        Do we want informative messages?                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Objects read from all filehandles.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetDataFiles, IPGetDataFromFilehandles                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetDataFromFilehandles2, filehandles, parser                           M
*****************************************************************************/
IPObjectStruct *IPGetDataFromFilehandles2(FILE **Files,
					  int NumOfFiles,
					  IPStreamFormatType *Formats,
					  int *IsBinaryIndicators,
					  int Messages,
					  int MoreMessages)
{
    int	i, Handler;
    const char *ErrorMsg;
    IPObjectStruct
	*PObjHead = NULL;

    for	(i = 0; i < NumOfFiles; i++) {
	if (MoreMessages)
	    IRIT_INFO_MSG_PRINTF("Reading data file handle #%d\n", i);
		
	if ((Handler = IPOpenStreamFromFile2(Files[i], TRUE, Formats[i],
					     IsBinaryIndicators[i],
                                             FALSE,
					     FALSE)) < 0)
	    continue;

	PObjHead = IPAppendObjLists(IPGetObjects(Handler), PObjHead);

	if (Messages && IPHasError(&ErrorMsg))
	    IRIT_WARNING_MSG_PRINTF("File handle #%d, %s\n", i, ErrorMsg);
    }

    if (PObjHead == NULL) {
	if (Messages) {
	    IPHasError(&ErrorMsg);
	    IRIT_WARNING_MSG_PRINTF("No data found. %s\n", ErrorMsg);
	}
	return NULL;
    }

    return PObjHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resolves, in place, all instances in the objects into their proper       M
* geometry.  This functions hence eliminates all instances' objects while    M
* increasing the size of the data, and do so in place.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:      To eliminate instances from, in place.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Same geometry as in PObjects but without instances. M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPResolveInstances                                                       M
*****************************************************************************/
IPObjectStruct *IPResolveInstances(IPObjectStruct *PObjects)
{
    IPObjectStruct *PObj, *Prev;

    GlblResolveInstObjects = PObjects;

    /* Map the instances to their geometry, in line. */
    for (PObj = PObjects, Prev = NULL; PObj != NULL; ) {
	if (PObj == PObjects) {
	    GlblResolveInstObjects =
	        PObjects = IPResolveInstancesAux(PObj);
	    Prev = PObjects;
	}
	else if (Prev != NULL) {
	    Prev -> Pnext = IPResolveInstancesAux(PObj);
	    Prev = Prev -> Pnext;
	}

	if (Prev != NULL)
	    PObj = Prev -> Pnext;
	else
	    break; /* Failed to resolve instances... */
    }

    GlblResolveInstObjects = NULL;

    return PObjects;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IPResolveInstances, in place                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      To eliminate instances from, in place.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Same geometry but instances free.                     *
*****************************************************************************/
static IPObjectStruct *IPResolveInstancesAux(IPObjectStruct *PObj)
{
    if (IP_IS_INSTNC_OBJ(PObj)) {
	IPObjectStruct
	    *PObjInst = IPGetObjectByName(PObj -> U.Instance -> Name,
					  GlblResolveInstObjects, FALSE);

	if (PObjInst != NULL) {
	    IPObjectStruct
	        *PTmp = GMTransformObject(IPResolveInstancesAux(PObjInst),
					  PObj -> U.Instance -> Mat);

	    PTmp -> Pnext = PObj -> Pnext;
	    IPFreeObject(PObj);
	    AttrSetObjectIntAttrib(PTmp, "WasInstance", TRUE);
	    AttrFreeObjectAttribute(PTmp, "invisible");
	    IP_CAT_OBJ_NAME(PTmp, "_INST");

	    return PTmp;
	}
	else {
	    IRIT_WARNING_MSG_PRINTF("Failed to locate base geometry \"%s\" of instance \"%s\"\n",
				    PObj -> U.Instance -> Name,
				    IP_GET_OBJ_NAME(PObj));
	    return NULL;
	}
    }
    else if (IP_IS_OLST_OBJ(PObj)) {
	int i;
	IPObjectStruct *PTmp;

	for (i = 0; (PTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
	    IPObjectStruct
		*PTmpNew = IPResolveInstancesAux(PTmp);

	    if (PTmpNew != PTmp)
	        IPListObjectInsert(PObj, i, PTmpNew);
	}
	return PObj;
    }
    else
	return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to read the data from	a given	file.				     M
*   Returns NULL if EOF was reached or error occured.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:     A handler to the open stream.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Read object, or NULL if failed.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*    IPSetPolyListCirc, IPSetFlattenObjects,  IPSetReadOneObject	     M
*  IPTransformInstances.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetObjects, files, parser	                                             M
*****************************************************************************/
IPObjectStruct *IPGetObjects(int Handler)
{
    IPObjectStruct
	*PObj = NULL;

    _IPParseResetError();

    /* If the following gain control and is non zero - its from error! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
        _IPLongJumpActive = FALSE;
	return NULL;
    }
    _IPLongJumpActive = TRUE;

    switch (_IPStream[Handler].Format) {
	case IP_IGS_FILE:
            if (!IRT_STR_ZERO_LEN(_IPStream[Handler].FileName))
	        PObj = IPIgesLoadFile(_IPStream[Handler].FileName, FALSE,
				      FALSE, FALSE, FALSE);
            break;
	case IP_STL_FILE:
            if (!IRT_STR_ZERO_LEN(_IPStream[Handler].FileName))
	        PObj = IPSTLLoadFile(_IPStream[Handler].FileName,
				     _IPStream[Handler].FileType
							    == IP_FILE_BINARY,
				     FALSE, FALSE, FALSE);
            break;
	case IP_OBJ_FILE:
            if (!IRT_STR_ZERO_LEN(_IPStream[Handler].FileName))
	        PObj = IPOBJLoadFile(_IPStream[Handler].FileName,
				     FALSE, TRUE, TRUE, TRUE);
            break;
	case IP_GCODE_FILE:
            if (!IRT_STR_ZERO_LEN(_IPStream[Handler].FileName))
	        PObj = IPNCGCodeLoadFile(_IPStream[Handler].FileName,
					 FALSE, FALSE);
            break;
	default:  /* Try IRIT data files. */
	    if (_IPStream[Handler].FileType == IP_FILE_BINARY) {
	        PObj = IPGetBinObject(Handler);
	    }
#ifdef IPC_BIN_COMPRESSION
            else if (_IPStream[Handler].FileType == IP_FILE_COMPRESSED) {
                PObj = IpcDecompressObj(Handler);
            }
#endif /* IPC_BIN_COMPRESSION */
	    else {  /* IP_FILE_TEXT */ 
	        PObj = IPAllocObject("", IP_OBJ_UNDEF, NULL);

		IPGetAllObjects(Handler, PObj, 0);
	    }
	    break;
    }

    if (PObj == NULL || IP_IS_UNDEF_OBJ(PObj)) {
        if (PObj != NULL && IP_IS_UNDEF_OBJ(PObj))
	    IPFreeObject(PObj);

	_IPLongJumpActive = FALSE;

	return NULL;
    }

    PObj = IPProcessReadObject(PObj);

    _IPLongJumpActive = FALSE;

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Filters out degenetared list objects with zero or one elements.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Read object.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Same object as Pobj but if PObj is a degenerated     *
*                       of one element or even zero element, it is fixed up. *
*****************************************************************************/
static IPObjectStruct *EliminateDegenLists(IPObjectStruct *PObj)
{
    if (PObj == NULL)
	return NULL;

    if (IP_IS_OLST_OBJ(PObj)) {
        if (IPListObjectGet(PObj, 0) == NULL) {
	    /* Nothing read in. */
	    IPFreeObject(PObj);
	    PObj = NULL;

	    IP_FATAL_ERROR_EX(IP_ERR_FILE_EMPTY, IP_ERR_NO_LINE_NUM, "");
	}
	else if (IPListObjectGet(PObj, 1) == NULL) {
	    IPObjectStruct
		*PTmp = IPListObjectGet(PObj, 0);

	    /* Only one object in list - return the object instead. */
	    IPListObjectInsert(PObj, 0, NULL);
	    IPFreeObject(PObj);
	    PObj = PTmp;
	}
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Process a read object, in place, before returning it to the caller.	     M
*   List objects of zero or one elements are eliminated.                     M
*   Attributes are propagated throughout the hierarchy.			     M
*   If FlattenTree mode (see IPSetFlattenObjects) hierarchy is               M
* flattened out.			       				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to process.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Processed object, in place.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessReadObject, files, parser                                       M
*****************************************************************************/
IPObjectStruct *IPProcessReadObject(IPObjectStruct *PObj)
{
    if (PObj == NULL ||	(PObj = EliminateDegenLists(PObj)) == NULL)
	return NULL;

    if (GlblPropagateAttrs)
	AttrPropagateAttr(PObj, NULL);

    if (GlblFlattenObjects && PObj != NULL)
	PObj = IPFlattenTree(PObj);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls the hierarchy flattening of a read object.       		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Flatten:  If TRUE, list objects will be flattened out to a long linear   M
*             list. If FALSE, read object will be unchanged.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of flatten state.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetFlattenObjects, files, parser                                       M
*****************************************************************************/
int IPSetFlattenObjects(int Flatten)
{
    int OldFlatten = GlblFlattenObjects;

    GlblFlattenObjects = Flatten;

    return OldFlatten;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls the propagation of attributes from internal nodes to the leaves.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Propagate:  If TRUE, attributes will be propagated from internal nodes   M
*               to the leaves.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of propagation state.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetPropagateAttrs, files, parser                                       M
*****************************************************************************/
int IPSetPropagateAttrs(int Propagate)
{
    int OldPropagate = GlblPropagateAttrs;

    GlblPropagateAttrs = Propagate;

    return OldPropagate;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls the hierarchy flattening of a read object.       		     M
*                                                                            *
* PARAMETERS:                                                                M
*   FlattenInvisib:  If TRUE, list objects will be flattened out to a long   M
*                    linear list. If FALSE, read object will be unchanged.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of flatten state.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFlattenInvisibleObjects, files, parser                                 M
*****************************************************************************/
int IPFlattenInvisibleObjects(int FlattenInvisib)
{
    int OldFlattenInvisib = GlblFlattenInvisibObjects;

    GlblFlattenInvisibObjects = FlattenInvisib;

    return OldFlattenInvisib;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls the way the Ascii/bin parser handle multiple objects in a file.   M
*                                                                            *
* PARAMETERS:                                                                M
*   OneObject: If TRUE, only next object will be read by IPGetObjectst.	     M
*              If FALSE, objects will be read until EOF is detected and      M
*	       placed in a linked list.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of read one object.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetObjects                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetReadOneObject, files, parser                                        M
*****************************************************************************/
int IPSetReadOneObject(int OneObject)
{
    int OldReadOneObject = _IPReadOneObject;

    _IPReadOneObject = OneObject;

    return OldReadOneObject;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function on every leaf object read in.                  M
* PARAMETERS:                                                                M
*   ProcessLeafFunc:  A pointer to a call back function to be invoked on     M
*	      every leaf object read in.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPProcessLeafObjType:      Old call back pointer value.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetObjects                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetProcessLeafFunc, files, parser                                      M
*****************************************************************************/
IPProcessLeafObjType IPSetProcessLeafFunc(IPProcessLeafObjType ProcessLeafFunc)
{
    IPProcessLeafObjType
	OldProcessLeafFunc = _IPGlblProcessLeafFunc;

    _IPGlblProcessLeafFunc = ProcessLeafFunc;

    return OldProcessLeafFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Flattens out a tree hierarchy of objects into a linear list, in place. As  M
* a side effect freeform entities are processed by IPProcessFreeForm.        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object(s) to flatten out, in place.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Flattened hierarchy.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPProcessFreeForm, IPEvalFreeForms                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFlattenTree                                                            M
*****************************************************************************/
IPObjectStruct *IPFlattenTree(IPObjectStruct *PObj)
{
    IPFreeFormStruct IPFreeForm;

    IPFreeForm.CrvObjs = NULL;
    IPFreeForm.SrfObjs = NULL;
    IPFreeForm.TrimSrfObjs = NULL;
    IPFreeForm.TrivarObjs = NULL;
    IPFreeForm.TriSrfObjs = NULL;
    IPFreeForm.ModelObjs = NULL;
    IPFreeForm.MultiVarObjs = NULL;

    if (ATTR_OBJ_IS_INVISIBLE(PObj) && !GlblFlattenInvisibObjects)
       return NULL;

    if (PObj -> Pnext != NULL)
	return PObj;		    /* Can only flatten a single hierarchy. */

    if (IP_IS_OLST_OBJ(PObj)) {
	int i;
	IPObjectStruct *PTmp, *PTmp2,
	    *RetListTail = NULL,
	    *RetList = NULL;

	for (i = 0; (PTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
#ifdef IRIT_PRSR_ZERO_NAMES
	    /* Zero name of object if it is interior to list. */
	    if (strnicmp(PTmp -> Name, "VIEW_MAT", 8) != 0 &&
		strnicmp(PTmp -> Name, "PRSP_MAT", 8) != 0)
		PTmp -> Name[0] = 0;
#endif /* IRIT_PRSR_ZERO_NAMES */

	    if ((PTmp2 = IPFlattenTree(PTmp)) != NULL) {
		if (RetList != NULL)
		    RetListTail -> Pnext = PTmp2;
		else
		    RetList = PTmp2;
		RetListTail = IPGetLastObj(PTmp2);
	    }
	}

	IPListObjectInsert(PObj, 0, NULL);
	IPFreeObject(PObj);

	return RetList;
    }
    else if (IP_IS_CRV_OBJ(PObj)) {
	IPFreeForm.CrvObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
	IPFreeForm.SrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRIMSRF_OBJ(PObj)) {
	IPFreeForm.TrimSrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRIVAR_OBJ(PObj)) {
	IPFreeForm.TrivarObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRISRF_OBJ(PObj)) {
	IPFreeForm.TriSrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_MODEL_OBJ(PObj)) {
	IPFreeForm.ModelObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_MVAR_OBJ(PObj)) {
	IPFreeForm.MultiVarObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else {
	return PObj;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Process a model freeform into trimmed surfaces freeform, in place.       M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:  Freeform model to process into trimmed srfs, in place.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if models were found and processed, FALSE otherwise.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessModel2TrimSrfs, conversion                                      M
*****************************************************************************/
int IPProcessModel2TrimSrfs(IPFreeFormStruct *FreeForms)
{
    IPObjectStruct *PObj;

    if (FreeForms -> ModelObjs == NULL)
        return FALSE;

    for (PObj = FreeForms -> ModelObjs; PObj != NULL; PObj = PObj -> Pnext) {
        TrimSrfStruct
	    *TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

	FreeForms -> TrimSrfObjs = IPGenTrimSrfObject(NULL, TrimSrfs,
						    FreeForms -> TrimSrfObjs);
	FreeForms -> TrimSrfObjs -> Attr = PObj -> Attr;

	MdlModelFreeList(PObj -> U.Mdls);
	PObj -> U.Mdls = NULL;
	PObj -> Attr = NULL;
    }

    IPFreeObjectList(FreeForms -> ModelObjs);
    FreeForms -> ModelObjs = NULL;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the freeforms in the given hierarchy - usually to convert into   M
* a polygonal approximation.  This function invokes IPProcessFreeForm        M
* for the evaluation of the individual freeform entities.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object(s) to freeform evaluate, in place.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Evaluated hierarchy, in place.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPProcessFreeForm                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPEvalFreeForms                                                          M
*****************************************************************************/
IPObjectStruct *IPEvalFreeForms(IPObjectStruct *PObj)
{
    IPFreeFormStruct IPFreeForm;

    IPFreeForm.CrvObjs = NULL;
    IPFreeForm.SrfObjs = NULL;
    IPFreeForm.TrimSrfObjs = NULL;
    IPFreeForm.TrivarObjs = NULL;
    IPFreeForm.TriSrfObjs = NULL;
    IPFreeForm.ModelObjs = NULL;
    IPFreeForm.MultiVarObjs = NULL;

    if (IP_IS_OLST_OBJ(PObj)) {
	int i;
	IPObjectStruct *PTmp;

	for (i = 0; (PTmp = IPListObjectGet(PObj, i)) != NULL; i++)
	    IPListObjectInsert(PObj, i, IPEvalFreeForms(PTmp));

	return PObj;
    }
    else if (IP_IS_CRV_OBJ(PObj)) {
	IPFreeForm.CrvObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
	IPFreeForm.SrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRIMSRF_OBJ(PObj)) {
	IPFreeForm.TrimSrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRIVAR_OBJ(PObj)) {
	IPFreeForm.TrivarObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_TRISRF_OBJ(PObj)) {
	IPFreeForm.TriSrfObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_MODEL_OBJ(PObj)) {
	IPFreeForm.ModelObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else if (IP_IS_MVAR_OBJ(PObj)) {
	IPFreeForm.MultiVarObjs = PObj;
	return IPProcessFreeForm(&IPFreeForm);
    }
    else {
	return PObj;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Flattens out a list of trees' hierarchy (a forrest) of objects into a      M
* linear list, in place.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:    List of object(s) to flatten out.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Flattened hierarchy.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFlattenForrest                                                         M
*****************************************************************************/
IPObjectStruct *IPFlattenForrest(IPObjectStruct *PObjList)
{
    IPObjectStruct *PObj,
	*PNewList = NULL,
	*PNewLast = NULL;

    for (PObj = PObjList; PObj != NULL; ) {
	IPObjectStruct *PFlat,
	    *Pnext = PObj -> Pnext;

	PObj -> Pnext = NULL;
	if ((PFlat = IPFlattenTree(PObj)) != NULL) {
	    if (PNewList == NULL) {
		PNewList = PFlat;
		PNewLast = IPGetLastObj(PFlat);
	    }
	    else {
		PNewLast -> Pnext = PFlat;
		PNewLast = IPGetLastObj(PFlat);
	    }
	}

	PObj = Pnext;
    }

    return PNewList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read the geometry data from a given file. Reads "[OBJECT ..."   *
* prefixes only and invoke the auxiliary routine.			     *
*   Objects may be recursively defined.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:     A handler to the open stream.				     *
*   PObjParent:  One list object, this read object should be hooked as an    *
*                element.						     *
*   Level:       Of recursion.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPGetAllObjects(int Handler, IPObjectStruct *PObjParent, int Level)
{
    char StringToken[IRIT_LINE_LEN_LONG];
    IPTokenType Token;
    int	WasObjectToken = FALSE,
	ObjCount = 0,
	Quit = FALSE;
    IPObjectStruct *PObj;

    while (!Quit) {
    	while ((Token = _IPGetToken(Handler, StringToken)) !=
							IP_TOKEN_OPEN_PAREN &&
	       Token != IP_TOKEN_CLOSE_PAREN &&
	       Token != IP_TOKEN_EOF);

	if (Token == IP_TOKEN_CLOSE_PAREN || Token == IP_TOKEN_EOF) {
	    if (Token == IP_TOKEN_CLOSE_PAREN)
		_IPUnGetToken(Handler, StringToken);
	    Quit = TRUE;
	    break;
	}

	switch (_IPGetToken(Handler, StringToken)) {
	    case IP_TOKEN_OBJECT:
		WasObjectToken = TRUE;

	        IPReallocNewTypeObject(PObjParent, IP_OBJ_LIST_OBJ);
		PObj = IPAllocObject("", IP_OBJ_UNDEF, NULL);

		/* The following handles optional attributes in record. */
		if (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN)
		    GetObjectAttributes(PObj, Handler);
		else {
		    _IPUnGetToken(Handler, StringToken);
		}

		if (_IPGetToken(Handler, StringToken) == IP_TOKEN_OTHER &&
		    stricmp(StringToken, "NONE") != 0) {
		    IP_SET_OBJ_NAME2(PObj, StringToken);
		    IritStrUpper(PObj -> ObjName);
		}

		IPGetAllObjects(Handler, PObj, Level + 1);

		_IPGetCloseParenToken(Handler);

#		ifdef EMPTY_OBJECT_IS_ERROR
		    if (IP_IS_UNDEF_OBJ(PObj))
		        IP_FATAL_ERROR_EX(IP_ERR_OBJECT_EMPTY,
					  _IPStream[Handler].LineNum,
					  "");
		    else
		        IPListObjectInsert(PObjParent, ObjCount++, PObj);
#		else
		    if (!IP_IS_UNDEF_OBJ(PObj))
		        IPListObjectInsert(PObjParent, ObjCount++, PObj);
#		endif /* EMPTY_OBJECT_IS_ERROR */
		break;
	    default:
		if (WasObjectToken) {
		    IP_FATAL_ERROR_EX(IP_ERR_OBJECT_EXPECTED,
				      _IPStream[Handler].LineNum,
				      StringToken);
		}
		_IPUnGetToken(Handler, StringToken);
		_IPUnGetToken(Handler, "[");
		IPGetAuxObject(Handler, PObjParent);
		if (IP_IS_POLY_OBJ(PObjParent)) {
		    PObjParent -> U.Pl =
		        IPReversePlList(PObjParent -> U.Pl);
		}

		Quit = TRUE;

		if (Level < IP_ATTR_HIERARCHY &&
		    _IPGlblProcessLeafFunc != NULL)
		    _IPGlblProcessLeafFunc(PObjParent);
		break;
	}

	if (Level == 0 && WasObjectToken && _IPReadOneObject)
	    Quit = TRUE;
    }

    if (IP_IS_OLST_OBJ(PObjParent)) {
	IPListObjectInsert(PObjParent, ObjCount++, NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get close paren token from FILE f.				     M
*   This function invokes the parser's abort routine, if no close paren.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:     A handler to the open stream.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPGetCloseParenToken						     M
*****************************************************************************/
void _IPGetCloseParenToken(int Handler)
{
    char StringToken[IRIT_LINE_LEN_LONG];

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN)
	IP_FATAL_ERROR_EX(IP_ERR_CLOSE_PAREN_EXPECTED,
			  _IPStream[Handler].LineNum, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to skip to the next closed parenthesis.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:     A handler to the open stream.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE, if found close paren.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPSkipToCloseParenToken						     M
*****************************************************************************/
int _IPSkipToCloseParenToken(int Handler)
{
    char StringToken[IRIT_LINE_LEN_LONG];
    IPTokenType
	Token = IP_TOKEN_EOF;

    while (!InputEOF(Handler) &&
	  (Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN);

    return Token == IP_TOKEN_CLOSE_PAREN;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get one numeric token into r.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:     A handler to the open stream.				     *
*   r:           Where numeric data should go to.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetNumericToken(int Handler, IrtRType *r)
{
    char StringToken[IRIT_LINE_LEN_LONG];

    _IPGetToken(Handler, StringToken);
    if (sscanf(StringToken, IP_IRIT_FLOAT_READ, r) != 1)
        IP_FATAL_ERROR_EX(IP_ERR_NUMBER_EXPECTED,
			  _IPStream[Handler].LineNum, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read the content of a single object.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:     A handler to the open stream.				     *
*   PObj:        Where to place the read object.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPGetAuxObject(int Handler, IPObjectStruct *PObj)
{
    int	i, j, ErrLine;
    IPTokenType
	Token = IP_TOKEN_NONE;
    char *ErrStr, StringToken[IRIT_LINE_LEN_XLONG];
    CagdRType *Coords;
    IPPolygonStruct *PPolygon;
    CagdCrvStruct *PCurve, *Crv;
    CagdSrfStruct *PSurface, *Srf;
    TrimSrfStruct *PTrimSrf, *TSrf;
    TrivTVStruct *PTrivar, *Trivar;
    TrngTriangSrfStruct *PTriSrf, *TriSrf;
    MdlModelStruct *PModel, *Mdl;
    MvarMVStruct *PMultiVar, *MultiVar;

    IPReallocNewTypeObject(PObj, IP_OBJ_UNDEF);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	switch (Token = _IPGetToken(Handler, StringToken)) {
	    case IP_TOKEN_POLYGON:
	    case IP_TOKEN_POLYLINE:
	    case IP_TOKEN_POINTLIST:
	    case IP_TOKEN_POLYSTRIP:
		IPReallocNewTypeObject(PObj, IP_OBJ_POLY);
		PPolygon = IPAllocPolygon(0, NULL, NULL);
		switch (Token) {
		    case IP_TOKEN_POLYGON:
			IP_SET_POLYGON_OBJ(PObj);
			break;
		    case IP_TOKEN_POLYLINE:
			IP_SET_POLYLINE_OBJ(PObj);
			break;
		    case IP_TOKEN_POINTLIST:
			IP_SET_POINTLIST_OBJ(PObj);
			break;
		    case IP_TOKEN_POLYSTRIP:
			IP_SET_POLYSTRIP_OBJ(PObj);
			break;
		    default:
			IP_FATAL_ERROR_EX(IP_ERR_UNDEF_EXPR_HEADER,
					  _IPStream[Handler].LineNum,
					  StringToken);
			break;
		}

		/* The following handle the optional attributes in struct.   */
		if (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN)
		    GetPolygonAttributes(PPolygon, Handler);
		else
		    _IPUnGetToken(Handler, StringToken);

		/* The following handles reading the vertices. */
		GetPointData(Handler, PPolygon, IP_IS_POLYGON_OBJ(PObj));

		if (IP_IS_POLYGON_OBJ(PObj)) {
		    if (!IP_HAS_PLANE_POLY(PPolygon))
			IPUpdatePolyPlane(PPolygon);

		    IPUpdateVrtxNrml(PPolygon, PPolygon -> Plane);
		}

		PPolygon -> Pnext = PObj -> U.Pl;
		PObj -> U.Pl = PPolygon;

		if (IP_IS_POLYSTRIP_OBJ(PObj)) {
		    for (PPolygon = PObj -> U.Pl;
			 PPolygon != NULL;
			 PPolygon = PPolygon -> Pnext) {
		        IP_SET_STRIP_POLY(PPolygon);
		    }
		}
		break;
	    case IP_TOKEN_SURFACE:
		IPReallocNewTypeObject(PObj, IP_OBJ_SURFACE);
		ErrLine = _IPStream[Handler].LineNum;
		PSurface = CagdSrfReadFromFile2(Handler, &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PSurface != NULL) {
		    if (PObj -> U.Srfs == NULL)
		        PObj -> U.Srfs = PSurface;
		    else {
		        for (Srf = PObj -> U.Srfs;
			     Srf -> Pnext != NULL;
			     Srf = Srf -> Pnext);
			Srf -> Pnext = PSurface;
		    }
		}
		break;
	    case IP_TOKEN_CURVE:
		IPReallocNewTypeObject(PObj, IP_OBJ_CURVE);
		ErrLine = _IPStream[Handler].LineNum;
		PCurve = CagdCrvReadFromFile2(Handler, &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PCurve != NULL) {
		    if (PObj -> U.Crvs == NULL)
		        PObj -> U.Crvs = PCurve;
		    else {
		        for (Crv = PObj -> U.Crvs;
			     Crv -> Pnext != NULL;
			     Crv = Crv -> Pnext);
			Crv -> Pnext = PCurve;
		    }
		}
		break;
	    case IP_TOKEN_TRIMSRF:
		IPReallocNewTypeObject(PObj, IP_OBJ_TRIMSRF);
		ErrLine = _IPStream[Handler].LineNum;
		PTrimSrf = TrimReadTrimmedSrfFromFile2(Handler, TRUE,
						       &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_TRIM_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PTrimSrf != NULL) {
		    if (PObj -> U.TrimSrfs == NULL)
		        PObj -> U.TrimSrfs = PTrimSrf;
		    else {
		        for (TSrf = PObj -> U.TrimSrfs;
			     TSrf -> Pnext != NULL;
			     TSrf = TSrf -> Pnext);
			TSrf -> Pnext = PTrimSrf;
		    }
		}
		break;
	    case IP_TOKEN_TRIVAR:
		IPReallocNewTypeObject(PObj, IP_OBJ_TRIVAR);
		ErrLine = _IPStream[Handler].LineNum;
		PTrivar = TrivTVReadFromFile2(Handler, &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_TRIV_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PTrivar != NULL) {
		    if (PObj -> U.Trivars == NULL)
		        PObj -> U.Trivars = PTrivar;
		    else {
		        for (Trivar = PObj -> U.Trivars;
			     Trivar -> Pnext != NULL;
			     Trivar = Trivar -> Pnext);
			Trivar -> Pnext = PTrivar;
		    }
		}
		break;
	    case IP_TOKEN_TRISRF:
		IPReallocNewTypeObject(PObj, IP_OBJ_TRISRF);
		ErrLine = _IPStream[Handler].LineNum;
		PTriSrf = TrngTriSrfReadFromFile2(Handler, &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PTriSrf != NULL) {
		    if (PObj -> U.TriSrfs == NULL)
		        PObj -> U.TriSrfs = PTriSrf;
		    else {
		        for (TriSrf = PObj -> U.TriSrfs;
			     TriSrf -> Pnext != NULL;
			     TriSrf = TriSrf -> Pnext);
			TriSrf -> Pnext = PTriSrf;
		    }
		}
		break;
	    case IP_TOKEN_MODEL:
		IPReallocNewTypeObject(PObj, IP_OBJ_MODEL);
		ErrLine = _IPStream[Handler].LineNum;
		PModel = MdlReadModelFromFile2(Handler, TRUE,
					       &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PModel != NULL) {
		    if (PObj -> U.Mdls == NULL)
		        PObj -> U.Mdls = PModel;
		    else {
		        for (Mdl = PObj -> U.Mdls;
			     Mdl -> Pnext != NULL;
			     Mdl = Mdl -> Pnext);
			Mdl -> Pnext = PModel;
		    }
		}
		break;
	    case IP_TOKEN_MULTIVAR:
		IPReallocNewTypeObject(PObj, IP_OBJ_MULTIVAR);
		ErrLine = _IPStream[Handler].LineNum;
		PMultiVar = MvarMVReadFromFile2(Handler, &ErrStr, &ErrLine);
		_IPStream[Handler].LineNum = ErrLine;

		if (ErrStr != NULL) {
		    IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, ErrLine, ErrStr);
		    break;
		}

		if (PMultiVar != NULL) {
		    if (PObj -> U.MultiVars == NULL)
		        PObj -> U.MultiVars = PMultiVar;
		    else {
		        for (MultiVar = PObj -> U.MultiVars;
			     MultiVar -> Pnext != NULL;
			     MultiVar = MultiVar -> Pnext);
			MultiVar -> Pnext = PMultiVar;
		    }
		}
		break;
	    case IP_TOKEN_NUMBER:
		IPReallocNewTypeObject(PObj, IP_OBJ_NUMERIC);
		GetNumericToken(Handler, &PObj -> U.R);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_STRING:
		IPReallocNewTypeObject(PObj, IP_OBJ_STRING);
		_IPGetToken(Handler, StringToken);
		PObj -> U.Str = IritStrdup(StringToken);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_POINT:
		IPReallocNewTypeObject(PObj, IP_OBJ_POINT);
		for (i = 0; i < 3; i++)
		    GetNumericToken(Handler, &PObj -> U.Pt[i]);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_VECTOR:
		IPReallocNewTypeObject(PObj, IP_OBJ_VECTOR);
		for (i = 0; i < 3; i++)
		    GetNumericToken(Handler, &PObj -> U.Vec[i]);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_PLANE:
		IPReallocNewTypeObject(PObj, IP_OBJ_PLANE);
		for (i = 0; i < 4; i++)
		    GetNumericToken(Handler, &PObj -> U.Plane[i]);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_MATRIX:
		IPReallocNewTypeObject(PObj, IP_OBJ_MATRIX);
		for (i = 0; i < 4; i++)
		    for (j = 0; j < 4; j++)
			GetNumericToken(Handler, &(*PObj -> U.Mat)[i][j]);
		_IPGetCloseParenToken(Handler);

		if (strnicmp(IP_GET_OBJ_NAME(PObj), "VIEW_MAT", 8) == 0) {
		    IPWasViewMat = TRUE;
		    IRIT_HMGN_MAT_COPY(IPViewMat, PObj -> U.Mat);
		}
		else if (strnicmp(IP_GET_OBJ_NAME(PObj), "PRSP_MAT", 8) == 0) {
		    IPWasPrspMat = TRUE;
		    IRIT_HMGN_MAT_COPY(IPPrspMat, PObj -> U.Mat);
		}
		break;
	    case IP_TOKEN_INSTANCE:
		IPReallocNewTypeObject(PObj, IP_OBJ_INSTANCE);
		_IPGetToken(Handler, StringToken);
		PObj -> U.Instance -> Name = IritStrdup(StringToken);
		for (i = 0; i < 4; i++)
		    for (j = 0; j < 4; j++)
			GetNumericToken(Handler,
					&PObj -> U.Instance -> Mat[i][j]);
		_IPGetCloseParenToken(Handler);
		break;
	    case IP_TOKEN_CTLPT:
		IPReallocNewTypeObject(PObj, IP_OBJ_CTLPT);
		_IPGetToken(Handler, StringToken);

		i = atoi(&StringToken[1]);
		if ((StringToken[0] == 'P' || StringToken[0] == 'E' ) &&
		    i > 0 && i < 6) {
		   j = StringToken[0] == 'E';
		   PObj -> U.CtlPt.PtType = CAGD_MAKE_PT_TYPE(!j, i);
		}
		else {
		    IP_FATAL_ERROR_EX(IP_ERR_PT_TYPE_EXPECTED,
				      _IPStream[Handler].LineNum,
				      StringToken);
		    i = j = 0;
		    break;
		}

		Coords = PObj -> U.CtlPt.Coords;
		for ( i += 1 - j; i > 0; i--)
		    GetNumericToken(Handler, &Coords[j++]);
		_IPGetCloseParenToken(Handler);
		break;
	    default:
	      IP_FATAL_ERROR_EX(IP_ERR_UNDEF_EXPR_HEADER,
				_IPStream[Handler].LineNum,
				StringToken);
		break;
	} /* Of switch. */
    } /* Of while. */

    _IPUnGetToken(Handler, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to unget one token (on stack of UNGET_STACK_SIZE levels!)	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:       A handler to the open stream.			     M
*   StringToken:   Token to unget                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPUnGetToken							     M
*****************************************************************************/
void _IPUnGetToken(int Handler, char *StringToken)
{
    if (_IPStream[Handler].TokenStackPtr >= UNGET_STACK_SIZE)
        IP_FATAL_ERROR_EX(IP_ERR_STACK_OVERFLOW, _IPStream[Handler].LineNum, "");

    strcpy(_IPStream[Handler].TokenStack[_IPStream[Handler].TokenStackPtr++],
	   StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to unget a single character from input stream.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:   A handler to the open stream.				     M
*   c:         Character to unget.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPInputUnGetC, files, parser                                             M
*****************************************************************************/
void IPInputUnGetC(int Handler, char c)
{
    _IPStream[Handler].UnGetChar = c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get a single character from input stream.			     *
*   If input returns EOF block until new input arrives (can happen if        *
* reading from a non io blocked socket).				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       Read character.                                               *
*****************************************************************************/
static int InputGetC(int Handler)
{
    int c;

    if (_IPStream[Handler].UnGetChar >= 0) {
	c = _IPStream[Handler].UnGetChar;

	_IPStream[Handler].UnGetChar = -1;
    }
    else if (_IPStream[Handler].f != NULL) {
	c = getc(_IPStream[Handler].f);
#	ifdef __UNIX__
	    if (c == 0x0d)   /* Skip Ascii CR that comes for Win based OSs. */
	        return InputGetC(Handler);
#	endif /* __UNIX__ */
    }
    else {
        assert(_IPStream[Handler].ReadCharFunc != NULL);
	while ((c = _IPStream[Handler].ReadCharFunc(Handler)) == EOF)
	    IritSleep(10);
    }

    if (c < ' ' && c > 0 && c != '\n' && c != '\r' && c != '\t')
        IP_FATAL_ERROR_EX(IP_ERR_BIN_IN_TEXT,
			  _IPStream[Handler].LineNum,
			  IRIT_EXP_STR("Is it a binary file!?"));

    return c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test for EOF condition in input stream.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:   A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if EOF detected.                                          *
*****************************************************************************/
static int InputEOF(int Handler)
{
    if (_IPStream[Handler].f != NULL)
	return feof(_IPStream[Handler].f);
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get the next token out of the input file f.		     *
*   Returns TRUE if !InputEOF and the next token found in StringToken.	     *
*   StringToken must be allocated before calling this routine!		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:      A handler to the open stream.				     *
*   StringToken:  String token will be placed herein.                        *
*   Quoted:       If we detected a quoated string: "xxx yyy".                *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if successful.                                        *
*****************************************************************************/
static int GetStringToken(int Handler, char *StringToken, int *Quoted)
{
    int	Len;
    char *LocalStringToken,
	c = EOF;

    *Quoted = FALSE;

    if (_IPStream[Handler].TokenStackPtr) { /*	get first the unget token */
	strcpy(StringToken, _IPStream[Handler].TokenStack[--_IPStream[Handler].
							       TokenStackPtr]);
	return TRUE;
    }
    /* skip white spaces: */
    while ((!InputEOF(Handler)) &&
	   (((c = InputGetC(Handler)) == ' ') || (c == '\t') || (c == '\n')) &&
	   (c != (char) EOF))
	if (c == '\n')
	    _IPStream[Handler].LineNum++;		 /* Count the lines. */

    LocalStringToken = StringToken;
    Len = 2;          /* Take two characters as spare (for eos and luck...). */

    if (c == '[') {		      /* Its a token by	itself so return it. */
	*LocalStringToken++ = c;	      /* Copy the token	into string. */
	Len++;
    }
    else {
	if (!InputEOF(Handler) && (c != (char) EOF)) {
	    if (c == '"') {
		*Quoted = TRUE;
		while ((!InputEOF(Handler)) &&
		       ((c = InputGetC(Handler)) != '"') &&
		       (c != '\n') &&
		       (c != (char) EOF)) {
		    *LocalStringToken++ = c;      /* Copy the quoted string. */
		    if (Len++ >= IRIT_LINE_LEN_XLONG) {
			StringToken[IRIT_LINE_LEN_XLONG - 1] = 0;
			IP_FATAL_ERROR_EX(IP_ERR_STR_TOO_LONG,
					  _IPStream[Handler].LineNum,
					  StringToken);
		    }			
		    if (c == '\\') {
			/* Next character is quoted - copy verbatim. */
			*--LocalStringToken = c = InputGetC(Handler);
			LocalStringToken++;
		    }
		}
	    }
	    else {
		do {
		    *LocalStringToken++ = c;  /* Copy the token into string. */
		    if (Len++ >= IRIT_LINE_LEN_XLONG) {
			StringToken[IRIT_LINE_LEN_XLONG - 1] = 0;
			IP_FATAL_ERROR_EX(IP_ERR_STR_TOO_LONG,
					  _IPStream[Handler].LineNum,
					  StringToken);
		    }			
		}
		while ((!InputEOF(Handler)) &&
		       ((c = InputGetC(Handler)) != ' ') &&
		       (c != '\t') &&
		       (c != '\n') &&
		       (c != (char) EOF));
	    }
	    if (!InputEOF(Handler) && c == '\n')
	        IPInputUnGetC(Handler, c);            /* Save for next time. */
	}
    }
    *LocalStringToken =	0;					 /* Put	eos. */

    /* The following handles the spacial case were we have XXXX] - we must   */
    /* split it	into two token XXXX and	], _IPUnGetToken(']') & return XXXX: */
    if (!*Quoted &&
	(StringToken[Len = (int) strlen(StringToken) - 1] == ']') &&
	(Len > 0)) {
	/* Return CloseParan */
	_IPUnGetToken(Handler, &StringToken[Len]);	 /* Save next token. */
	StringToken[Len] = 0;			/* Set end of string on	"]". */
    }

    return !InputEOF(Handler) && (c != (char) EOF);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to get the next token out of the input file f as token number.     M
*   StringToken must be allocated before calling this routine!		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:       A handler to the open stream.			     M
*   StringToken:   String token will be placed herein.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPTokenType:   Token as a numeral.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPGetToken						       		     M
*****************************************************************************/
IPTokenType _IPGetToken(int Handler, char *StringToken)
{
    IRIT_STATIC_DATA IPTokenType IntTokens[] = {
	IP_TOKEN_OPEN_PAREN,
	IP_TOKEN_CLOSE_PAREN,
	IP_TOKEN_E1,
	IP_TOKEN_P1,
	IP_TOKEN_E2,
	IP_TOKEN_P2,
	IP_TOKEN_E3,
	IP_TOKEN_P3,
	IP_TOKEN_E4,
	IP_TOKEN_P4,
	IP_TOKEN_E5,
	IP_TOKEN_P5,
	IP_TOKEN_E6,
	IP_TOKEN_P6,
	IP_TOKEN_E7,
	IP_TOKEN_P7,
	IP_TOKEN_E8,
	IP_TOKEN_P8,
	IP_TOKEN_E9,
	IP_TOKEN_P9,
	IP_TOKEN_NUMBER,
	IP_TOKEN_STRING,
	IP_TOKEN_POINT,
	IP_TOKEN_VECTOR,
	IP_TOKEN_MATRIX,
	IP_TOKEN_CTLPT,
	IP_TOKEN_VERTEX,
	IP_TOKEN_POLYGON,
	IP_TOKEN_POLYLINE,
	IP_TOKEN_POINTLIST,
	IP_TOKEN_POLYSTRIP,
	IP_TOKEN_OBJECT,
	IP_TOKEN_COLOR,
	IP_TOKEN_RGB,
	IP_TOKEN_INTERNAL,
	IP_TOKEN_NORMAL,
	IP_TOKEN_PLANE,
	IP_TOKEN_CURVE,
	IP_TOKEN_SURFACE,
	IP_TOKEN_BEZIER,
	IP_TOKEN_BSPLINE,
	IP_TOKEN_GREGORY,
	IP_TOKEN_POWER,
	IP_TOKEN_TRIVAR,
	IP_TOKEN_PTYPE,
	IP_TOKEN_NUM_PTS,
	IP_TOKEN_ORDER,
	IP_TOKEN_KV,
	IP_TOKEN_KVP,
	IP_TOKEN_TRIMMDL,
	IP_TOKEN_TRIMSRF,
	IP_TOKEN_TRIMCRV,
	IP_TOKEN_TRIMCRVSEG,
	IP_TOKEN_INSTANCE,
	IP_TOKEN_TRISRF,
	IP_TOKEN_MODEL,
	IP_TOKEN_MDLTSEG,
	IP_TOKEN_MDLTSRF,
	IP_TOKEN_MDLLOOP,
	IP_TOKEN_MULTIVAR,

	IP_TOKEN_NONE
    };
    IRIT_STATIC_DATA char *StrTokens[] = {
	"[",
	"]",
	"E1",
	"P1",
	"E2",
	"P2",
	"E3",
	"P3",
	"E4",
	"P4",
	"E5",
	"P5",
	"E6",
	"P6",
	"E7",
	"P7",
	"E8",
	"P8",
	"E9",
	"P9",
	"NUMBER",
	"STRING",
	"POINT",
	"VECTOR",
	"MATRIX",
	"CTLPT",
	"VERTEX",
	"POLYGON",
	"POLYLINE",
	"POINTLIST",
	"POLYSTRIP",
	"OBJECT",
	"COLOR",
	"RGB",
	"INTERNAL",
	"NORMAL",
	"PLANE",
	"CURVE",
	"SURFACE",
	"BEZIER",
	"BSPLINE",
	"GREGORY",
	"POWER",
	"TRIVAR",
	"PTYPE",
	"NUMPTS",
	"ORDER",
	"KV",
	"KVP",
	"TRIMMODEL",
	"TRIMSRF",
	"TRIMCRV",
	"TRIMCRVSEG",
	"INSTANCE",
	"TRISRF",
	"MODEL",
	"MDLTSEG",
	"MDLTSRF",
	"MDLLOOP",
	"MULTIVAR",

	NULL
    };
    int i, Quoted;

    if (!GetStringToken(Handler, StringToken, &Quoted))
	return IP_TOKEN_EOF;

    if (Quoted)
	return IP_TOKEN_QUOTED;

    for (i = 0; StrTokens[i] != NULL; i++)
	if (stricmp(StringToken, StrTokens[i]) == 0)
	    return IntTokens[i];

    return IP_TOKEN_OTHER;			  /* Must be number or name. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read from input file f the	following [ATTR ...] [ATTR ...].     *
*   The first '[' was already read.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVertex:    Where attributes should go to.                               *
*   Handler:    A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetVertexAttributes(IPVertexStruct *PVertex, int Handler)
{
    int i;
    IrtRType Len;
    char StringToken[IRIT_LINE_LEN_LONG];

    do {
	switch (_IPGetToken(Handler, StringToken)) {
	    case IP_TOKEN_INTERNAL:
		_IPGetCloseParenToken(Handler);
		IP_SET_INTERNAL_VRTX(PVertex);
		break;
	    case IP_TOKEN_NORMAL:
		/* The following handles reading 3 coord. of vertex normal. */
		for (i = 0; i < 3; i++)
		    GetNumericToken(Handler, &PVertex -> Normal[i]);

		/* Make sure it is normalized. */
		Len = IRIT_PT_LENGTH(PVertex -> Normal);
		if (Len > 0) {
		    for (i = 0; i < 3; i++)
			PVertex -> Normal[i] /= Len;
		    IP_SET_NORMAL_VRTX(PVertex);
		}
		_IPGetCloseParenToken(Handler);
		break;
	    default:
		GetGenericAttribute(&PVertex -> Attr, Handler, StringToken);
		break;
	}
    }
    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN);

    PVertex -> Attr = AttrReverseAttributes(PVertex -> Attr);

    _IPUnGetToken(Handler, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read from input file f the	following [ATTR ...] [ATTR ...].     *
*   The first '[' was already read.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPolygon:   Where attributes should go to.                               *
*   Handler:    A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetPolygonAttributes(IPPolygonStruct *PPolygon, int Handler)
{
    int i;
    IrtRType Len;
    char StringToken[IRIT_LINE_LEN_LONG];

    do {
	switch (_IPGetToken(Handler, StringToken)) {
	    case IP_TOKEN_PLANE:
		/* The following handles reading of 4 coord. of plane eqn.. */
		for (i = 0; i < 4; i++)
		    GetNumericToken(Handler, &PPolygon -> Plane[i]);

		/* Make sure it is normalized. */
		Len = IRIT_PT_LENGTH(PPolygon -> Plane);
		if (Len > 0)
		    for (i = 0; i < 4; i++)
			PPolygon -> Plane[i] /= Len;
		else
		    IP_FATAL_ERROR_EX(IP_ERR_DEGEN_NORMAL,
				      _IPStream[Handler].LineNum, "");

		_IPGetCloseParenToken(Handler);
		IP_SET_PLANE_POLY(PPolygon);
		break;
	    default:
		GetGenericAttribute(&PPolygon -> Attr, Handler, StringToken);
		break;
	}
    }
    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN);

    PPolygon -> Attr = AttrReverseAttributes(PPolygon -> Attr);

    _IPUnGetToken(Handler, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read from input file f the	following [ATTR ...] [ATTR ...].     *
*   The first '[' was already read.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObject:    Where attributes should go to.                               *
*   Handler:    A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetObjectAttributes(IPObjectStruct *PObject, int Handler)
{
    int	i;
    char StringToken[IRIT_LINE_LEN_LONG];

    do {
	switch (_IPGetToken(Handler, StringToken)) {
	    case IP_TOKEN_COLOR:
		_IPGetToken(Handler, StringToken);
		if (sscanf(StringToken, "%d", &i) != 1)
		    IP_FATAL_ERROR_EX(IP_ERR_NUMBER_EXPECTED,
				      _IPStream[Handler].LineNum,
				      StringToken);
		_IPGetCloseParenToken(Handler);
		AttrSetObjectColor(PObject, i);
		break;
	    default:
		GetGenericAttribute(&PObject -> Attr, Handler, StringToken);
		break;
	}
    }
    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN);

    PObject -> Attr = AttrReverseAttributes(PObject -> Attr);

    _IPUnGetToken(Handler, StringToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read one generic attribute.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Attr:     Where to place the read attribute.                             *
*   Handler:  A handler to the open stream.				     *
*   Name:     Name of attribute.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetGenericAttribute(IPAttributeStruct **Attr,
				int Handler,
				char *Name)
{
    int Token;
    char StringToken[IRIT_LINE_LEN_XLONG];

    if ((Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_CLOSE_PAREN) {
	AttrSetStrAttrib(Attr, Name, "");
    }
    else if (Token == IP_TOKEN_QUOTED) {
	AttrSetStrAttrib(Attr, Name, StringToken);

	_IPSkipToCloseParenToken(Handler);
    }
    else if (Token == IP_TOKEN_OPEN_PAREN) {
	IPObjectStruct
	    *PObj = IPAllocObject("", IP_OBJ_UNDEF, NULL);

	_IPUnGetToken(Handler, StringToken);
	IPGetAllObjects(Handler, PObj, IP_ATTR_HIERARCHY);
	if (PObj -> ObjType != IP_OBJ_UNDEF) {
	    PObj = EliminateDegenLists(PObj);
	    AttrSetObjAttrib(Attr, Name, PObj, FALSE);
	}
	else
	    IPFreeObject(PObj);

	_IPSkipToCloseParenToken(Handler);
    }
    else {
	int i;
	IrtRType d;

	for (i = (int) strlen(StringToken) - 1; i >= 0; i--) {
	    if (!(isdigit(StringToken[i]) ||
		  StringToken[i] == 'e' ||
		  StringToken[i] == 'E' ||
		  StringToken[i] == '.' ||
		  StringToken[i] == '+' ||
		  StringToken[i] == '-'))
		break;
	}
	if (i < 0 && sscanf(StringToken, IP_IRIT_FLOAT_READ, &d) == 1) {
	    if (d == (int) d)
		AttrSetIntAttrib(Attr, Name, (int) d);
	    else
		AttrSetRealAttrib(Attr, Name, d);
	}
	else
	    AttrSetStrAttrib(Attr, Name, StringToken);

	_IPSkipToCloseParenToken(Handler);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read poly vertex information.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:     A handler to the open stream.				     *
*   PPolygon:    Where vertices are to be placed.                            *
*   IsPolygon:   Should we expect a polygon or a polyline? a pointlist?      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetPointData(int Handler, IPPolygonStruct *PPolygon, int IsPolygon)
{
    int i, j, Length;
    char StringToken[IRIT_LINE_LEN_LONG];
    IPVertexStruct *V,
	*VTail = NULL;

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &Length) != 1)
	IP_FATAL_ERROR_EX(IP_ERR_NUMBER_EXPECTED,
			  _IPStream[Handler].LineNum,
			  StringToken);

    for (i = 0; i < Length; i++) {
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN)
	    IP_FATAL_ERROR_EX(IP_ERR_OPEN_PAREN_EXPECTED,
			      _IPStream[Handler].LineNum,
			      StringToken);

	V = IPAllocVertex2(NULL);

	/* The following handle the optional attributes in struct. */
	if (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN)
	    GetVertexAttributes(V, Handler);
	else
	    _IPUnGetToken(Handler, StringToken);

	for (j = 0; j < 3; j++)				/* Read coordinates. */
	    GetNumericToken(Handler, &V -> Coord[j]);

	_IPGetCloseParenToken(Handler);

	if (VTail == NULL)
	    PPolygon -> PVertex = VTail = V;
	else {
	    VTail -> Pnext = V;
	    VTail = V;
	}
    }

    if (_IPPolyListCirc && IsPolygon)
	VTail -> Pnext = PPolygon -> PVertex;

    _IPGetCloseParenToken(Handler);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Concatenate all freeform objects in FreeForms into a single list.          M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:  Freeform geometry to process.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   concatenated linked list.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPConcatFreeForm, conversion                                             M
*****************************************************************************/
IPObjectStruct *IPConcatFreeForm(IPFreeFormStruct *FreeForms)
{
    IPObjectStruct *ObjLast,
	*Objs = NULL,
	*CrvObjs = FreeForms -> CrvObjs,
	*SrfObjs = FreeForms -> SrfObjs,
	*TrimSrfObjs = FreeForms -> TrimSrfObjs,
	*TrivarObjs = FreeForms -> TrivarObjs,
	*TriSrfObjs = FreeForms -> TriSrfObjs,
	*ModelObjs = FreeForms -> ModelObjs,
	*MultiVarObjs = FreeForms -> MultiVarObjs;

    if (CrvObjs != NULL) {
	ObjLast = IPGetLastObj(CrvObjs);
	ObjLast -> Pnext = Objs;
	Objs = CrvObjs;
    }
    if (SrfObjs != NULL) {
	ObjLast = IPGetLastObj(SrfObjs);
	ObjLast -> Pnext = Objs;
	Objs = SrfObjs;
    }
    if (TrimSrfObjs != NULL) {
	ObjLast = IPGetLastObj(TrimSrfObjs);
	ObjLast -> Pnext = Objs;
	Objs = TrimSrfObjs;
    }
    if (TrivarObjs != NULL) {
	ObjLast = IPGetLastObj(TrivarObjs);
	ObjLast -> Pnext = Objs;
	Objs = TrivarObjs;
    }
    if (TriSrfObjs != NULL) {
	ObjLast = IPGetLastObj(TriSrfObjs);
	ObjLast -> Pnext = Objs;
	Objs = TriSrfObjs;
    }
    if (ModelObjs != NULL) {
	ObjLast = IPGetLastObj(ModelObjs);
	ObjLast -> Pnext = Objs;
	Objs = ModelObjs;
    }
    if (MultiVarObjs != NULL) {
	ObjLast = IPGetLastObj(MultiVarObjs);
	ObjLast -> Pnext = Objs;
	Objs = MultiVarObjs;
    }

    return Objs;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets an IRIT matrix file.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   File:       File to read the matrix file from.                           M
*   ViewMat, ProjMat:    Matrices to get.                                    M
*   HasProjMat: TRUE if has a perspective matrix, FALSE otherwise.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if (at least) VIEW_MAT was found, FALSE otherwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPPutMatrixFile                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetMatrixFile                                                          M
*****************************************************************************/
int IPGetMatrixFile(const char *File,
		    IrtHmgnMatType ViewMat,
		    IrtHmgnMatType ProjMat,
		    int *HasProjMat)
{
    IPObjectStruct *MatObj, *View, *Proj;

    IPSetFlattenObjects(FALSE);
    if ((MatObj = IPGetDataFiles(&File, 1, TRUE, TRUE)) == NULL) {
	return FALSE;
    }
    
    if ((View = IPGetObjectByName("VIEW_MAT", MatObj, FALSE)) != NULL) {
	IRIT_GEN_COPY(ViewMat, View -> U.Mat, sizeof(IrtHmgnMatType));
	
	if ((Proj = IPGetObjectByName("PRSP_MAT", MatObj, FALSE)) != NULL) {
	    IRIT_GEN_COPY(ProjMat, Proj -> U.Mat, sizeof(IrtHmgnMatType));
	    *HasProjMat = TRUE;
	}
	else {
	    *HasProjMat = FALSE;
	}
	return TRUE;
    }

    return FALSE;
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
*****************************************************************************/
void IPDbg(void)
{
    CagdDbg(NULL);
    TrimDbg(NULL);
    TrivDbg(NULL);
    TrngDbg(NULL);
    MvarDbg(NULL);
    MdlDbg(NULL);

#ifdef DEBUG
    CagdDbgV(NULL);
    MdlDbgMC(NULL, FALSE);
    MdlDbgSC(NULL, FALSE);
#endif /* DEBUG */

    IPStderrObject(NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dumps a list of vertices.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IPVertexDbg(IPVertexStruct *V)
{
    IPPolygonStruct
	*Pl = IPAllocPolygon(0, V, NULL);
    IPObjectStruct
        *PObj = IPGenPOLYLINEObject(Pl);

    IP_SET_POLYLINE_OBJ(PObj);

    IPStderrObject(PObj);

    PObj -> U.Pl -> PVertex = NULL;
    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dumps a polygon to stdout.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IPPolygonDbg(IPPolygonStruct *Pl)
{
    IPObjectStruct
        *PObj = IPGenPOLYLINEObject(Pl);

    IP_SET_POLYLINE_OBJ(PObj);

    IPStderrObject(PObj);

    PObj -> U.Pl = NULL;
    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Draws an object on the display device via a socket connection.           *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to display, NULL to clear the screen.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   IPDbgDisplayObject                                                       *
*****************************************************************************/
void IPDbgDisplayObject(IPObjectStruct *PObj)
{
    static int
	PrgmIO = -1;
    static char
	*Program = NULL;

    if (Program == NULL) {
	if ((Program = getenv("IRIT_DISPLAY")) == NULL) {
#	    ifdef __WINNT__
		Program = "wntgdrvs -s-";
#	    endif /* __WINNT__ */
#	    ifdef __UNIX__
		Program = "xogldrvs -s-";
#	    endif /* __UNIX__ */
	}
    }

    if (PrgmIO < 0) {
        IPSocSrvrInit();        /* Initialize the listen socket for clients. */
        PrgmIO = IPSocExecAndConnect(Program,
				     getenv("IRIT_BIN_IPC") != NULL);
    }

    if (PObj == NULL) {
	IPObjectStruct
	    *PClrObj = IPGenStrObject("command_", "clear", NULL);

	IPSocWriteOneObject(PrgmIO, PClrObj);
	IPFreeObject(PClrObj);
    }
    else
        IPSocWriteOneObject(PrgmIO, PObj);
}

#endif /* DEBUG */
