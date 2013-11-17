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
#include "allocate.h"
#include "attribut.h"

#ifdef __WINNT__
#include <fcntl.h>
#include <io.h>
#endif /* __WINNT__ */

#define ZERO_NUM_IRIT_EPS	1e-13

IRIT_STATIC_DATA IPPrintFuncType
    IPPrintFunc = NULL;

IRIT_GLOBAL_DATA jmp_buf
    _IPLongJumpBuffer;				   /* Used in error traping. */
IRIT_GLOBAL_DATA int
    _IPLongJumpActive = FALSE,
    _IPFilterDegeneracies = TRUE;
IRIT_GLOBAL_DATA char
    *_IPGlblFloatFormat = "%-16.14lg";

IRIT_STATIC_DATA IrtRType
    IPZeroNumIritEps = ZERO_NUM_IRIT_EPS;

static void IPPutAttributes(int Handler,
			    const IPAttributeStruct *Attr,
			    int Indent);
static void IPPutAllObjects(const IPObjectStruct *PObj,
			    int Handler,
			    int Indent);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the data from given object into stdout.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To be put out to stdout.                                      M
*   IsBinary: Is this a binary file we should dump?			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPStdoutObject, files                                                    M
*****************************************************************************/
void IPStdoutObject(const IPObjectStruct *PObj, int IsBinary)
{
    IPPutObjectToFile(stdout, PObj, IsBinary);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the data from given object into stderr.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To be put out to stderr.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPStderrObject, files                                                    M
*****************************************************************************/
void IPStderrObject(const IPObjectStruct *PObj)
{
    IPPutObjectToFile(stderr, PObj, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the data from given object into given file handle.	     M
*   See function IPSetPrintFunc, IPSetFloatFormat. 		   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   f:        Output stream file handle.                                     M
*   PObj:     Object to put on output stream.                                M
*   IsBinary: Is this a binary file we should dump?			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutObjectToFile, files                                                 M
*****************************************************************************/
void IPPutObjectToFile(FILE *f, const IPObjectStruct *PObj, int IsBinary)
{
    int Handler = -1;

    /* If the following gain control and is non zero - its from error! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
	_IPLongJumpActive = FALSE;
	IPCloseStream(Handler, FALSE);
	return;
    }
    _IPLongJumpActive = TRUE;

    Handler = IPOpenStreamFromFile(f, FALSE, IsBinary, FALSE, FALSE);

    if (_IPStream[Handler].FileType == IP_FILE_BINARY)
        IPPutBinObject(Handler, PObj);
#ifdef IPC_BIN_COMPRESSION
    else if (_IPStream[Handler].FileType == IP_FILE_COMPRESSED)
        IpcCompressObj(Handler, PObj);
#endif /* IPC_BIN_COMPRESSION */
    else
	IPPutAllObjects(PObj, Handler, 0);

    IPCloseStream(Handler, FALSE);

    _IPLongJumpActive = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the data from given object into given file FileName.	     M
*   If FileName is NULL or empty, print using IPPrintFunc.		     M
*   See function IPSetPrintFunc, IPSetFloatFormat.		    	     M
*                                                                            *
* PARAMETERS:                                                                M
*   f:        Output stream.                                                 M
*   PObj:     Object to put on output stream.                                M
*   Indent:   File indentation (always a text file).                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutObjectToFile2, files                                                M
*****************************************************************************/
void IPPutObjectToFile2(FILE *f, const IPObjectStruct *PObj, int Indent)
{
    int Handler = -1;

    /* If the following gain control and is non zero - its from error! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
	_IPLongJumpActive = FALSE;
	IPCloseStream(Handler, FALSE);
	return;
    }
    _IPLongJumpActive = TRUE;

    Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE);

    IPPutAllObjects(PObj, Handler, Indent);

    IPCloseStream(Handler, FALSE);

    _IPLongJumpActive = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the data from given object into given file designated via M
* Handler.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:  A handler to the open stream.				     M
*   PObj:     Object to put on output stream.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSetPrintFunc, IPSetFloatFormat, IPPrintFunc _IPFprintf	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutObjectToHandler, files                                              M
*****************************************************************************/
void IPPutObjectToHandler(int Handler, const IPObjectStruct *PObj)
{
    switch (_IPStream[Handler].Format) {
	case IP_VRML_FILE:
	    /* Should be const - needs to be cleaned away. */
	    IPPutVrmlObject(Handler, (IPObjectStruct *) PObj, 0);
	    break;
	case IP_IDAT_FILE:
	default:
            if (_IPStream[Handler].FileType == IP_FILE_BINARY)
		IPPutBinObject(Handler, PObj);
#ifdef IPC_BIN_COMPRESSION
            else if (_IPStream[Handler].FileType == IP_FILE_COMPRESSED)
                IpcCompressObj(Handler, PObj);
#endif /* IPC_BIN_COMPRESSION */
	    else
		IPPutAllObjects(PObj, Handler, 0);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print out the data from given object.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to put out.                                            *
*   Handler:   A handler to the open stream.				     *
*   Indent:    Indentation to put object at.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutAllObjects(const IPObjectStruct *PObj,
			    int Handler,
			    int Indent)
{
    int i, IsRational, NumCoords;
    char Str[IRIT_LINE_LEN_LONG],
	*ErrStr = NULL;
    const CagdRType *Coords;
    IPObjectStruct *PObjTmp;
    const IPPolygonStruct *PPolygon;
    const IPVertexStruct *PVertex;
    const IPAttributeStruct
	*Attr = AttrTraceAttributes(PObj -> Attr, PObj -> Attr);

    if (Attr) {
	_IPFprintf(Handler, Indent, "[OBJECT ");

	IPPutAttributes(Handler, Attr, Indent);

	_IPFprintf(Handler, 0, "%s\n",
		   IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "NONE");
    }
    else {
	_IPFprintf(Handler, Indent, "[OBJECT %s\n",
		   IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "NONE");
    }
    Indent += 4;

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    for (PPolygon = PObj -> U.Pl;
		 PPolygon != NULL;
		 PPolygon = PPolygon -> Pnext) {
		if (PPolygon -> PVertex == NULL)
		    continue;

		if (IP_IS_POLYLINE_OBJ(PObj))
		    _IPFprintf(Handler, Indent, "[POLYLINE ");
		else if (IP_IS_POINTLIST_OBJ(PObj))
		    _IPFprintf(Handler, Indent, "[POINTLIST ");
		else {
		    /* Make sure it has a valid plane normal. */
		    if (IRIT_PT_APX_EQ_ZERO_EPS(PPolygon -> Plane, IRIT_UEPS)) {
		        IPPolygonStruct
			    *Pl = IPCopyPolygon(PPolygon);

		        if (!IPUpdatePolyPlane(Pl)) {
			    if (_IPFilterDegeneracies)
			        continue;
			    else {
			        /* Default to plane Z = 0. */
			        IRIT_PLANE_RESET(PPolygon -> Plane);
				Pl -> Plane[2] = 1.0;
			    }
			}

		        _IPFprintf(Handler,
				   Indent, "[%s [PLANE %s %s %s %s] ",
				   IP_IS_POLYSTRIP_OBJ(PObj) ? "POLYSTRIP"
				                             : "POLYGON",
				   _IPReal2Str(Pl -> Plane[0]),
				   _IPReal2Str(Pl -> Plane[1]),
				   _IPReal2Str(Pl -> Plane[2]),
				   _IPReal2Str(Pl -> Plane[3]));

			IPFreePolygon(Pl);
		    }
		    else
		        _IPFprintf(Handler,
				   Indent, "[%s [PLANE %s %s %s %s] ",
				   IP_IS_POLYSTRIP_OBJ(PObj) ? "POLYSTRIP"
				                             : "POLYGON",
				   _IPReal2Str(PPolygon -> Plane[0]),
				   _IPReal2Str(PPolygon -> Plane[1]),
				   _IPReal2Str(PPolygon -> Plane[2]),
				   _IPReal2Str(PPolygon -> Plane[3]));
		}
		IPPutAttributes(Handler, PPolygon -> Attr, Indent);

		for (PVertex = PPolygon -> PVertex -> Pnext, i = 1;
		     PVertex != PPolygon -> PVertex && PVertex != NULL;
		     PVertex = PVertex -> Pnext, i++);
		_IPFprintf(Handler, Indent + 4, "%d\n", i);

		PVertex = PPolygon -> PVertex;
		do {		     /* Assume at least one edge in polygon! */
		    _IPFprintf(Handler, Indent + 4, "[");
		    
		    IPPutAttributes(Handler, PVertex -> Attr, Indent);

		    if (IP_IS_POLYLINE_OBJ(PObj) ||
			((IP_IS_POLYGON_OBJ(PObj) ||
			  IP_IS_POLYSTRIP_OBJ(PObj)) &&
			 (!IP_HAS_NORMAL_VRTX(PVertex) ||
      			  IRIT_PT_APX_EQ(PPolygon -> Plane, PVertex -> Normal) ||
			  IRIT_PT_APX_EQ_ZERO_EPS(PVertex -> Normal, IRIT_UEPS))))
			_IPFprintf(Handler, 0, "%s%s %s %s]\n",
			    IP_IS_INTERNAL_VRTX(PVertex) ? "[INTERNAL] " : "",
			    _IPReal2Str(PVertex -> Coord[0]),
			    _IPReal2Str(PVertex -> Coord[1]),
			    _IPReal2Str(PVertex -> Coord[2]));
		    else if (IP_IS_POINTLIST_OBJ(PObj))
			_IPFprintf(Handler, 0, "%s %s %s]\n",
			    _IPReal2Str(PVertex -> Coord[0]),
			    _IPReal2Str(PVertex -> Coord[1]),
			    _IPReal2Str(PVertex -> Coord[2]));
		    else
			_IPFprintf(Handler, 0,
			    "%s[NORMAL %s %s %s] %s %s %s]\n",
			    IP_IS_INTERNAL_VRTX(PVertex) ? "[INTERNAL] " : "",
			    _IPReal2Str(PVertex -> Normal[0]),
			    _IPReal2Str(PVertex -> Normal[1]),
			    _IPReal2Str(PVertex -> Normal[2]),
			    _IPReal2Str(PVertex -> Coord[0]),
			    _IPReal2Str(PVertex -> Coord[1]),
			    _IPReal2Str(PVertex -> Coord[2]));

		    PVertex = PVertex -> Pnext;
		}
		while (PVertex != PPolygon -> PVertex && PVertex != NULL);
		_IPFprintf(Handler, Indent, "]\n");    /* Close the polygon. */
	    }
	    break;
	case IP_OBJ_NUMERIC:
	    _IPFprintf(Handler, Indent, "[NUMBER %s]\n",
		       _IPReal2Str(PObj -> U.R));
	    break;
	case IP_OBJ_POINT:
	    _IPFprintf(Handler, Indent, "[POINT %s %s %s]\n",
		     _IPReal2Str(PObj -> U.Pt[0]),
		     _IPReal2Str(PObj -> U.Pt[1]),
		     _IPReal2Str(PObj -> U.Pt[2]));
	    break;
	case IP_OBJ_VECTOR:
	    _IPFprintf(Handler, Indent, "[VECTOR %s %s %s]\n",
		     _IPReal2Str(PObj -> U.Vec[0]),
		     _IPReal2Str(PObj -> U.Vec[1]),
		     _IPReal2Str(PObj -> U.Vec[2]));
	    break;
	case IP_OBJ_PLANE:
	    _IPFprintf(Handler, Indent, "[PLANE %s %s %s %s]\n",
		     _IPReal2Str(PObj -> U.Plane[0]),
		     _IPReal2Str(PObj -> U.Plane[1]),
		     _IPReal2Str(PObj -> U.Plane[2]),
		     _IPReal2Str(PObj -> U.Plane[3]));
	    break;
	case IP_OBJ_CTLPT:
	    Coords = PObj -> U.CtlPt.Coords;
	    IsRational = CAGD_IS_RATIONAL_PT(PObj -> U.CtlPt.PtType);
	    NumCoords = CAGD_NUM_OF_PT_COORD(PObj -> U.CtlPt.PtType);

	    sprintf(Str, "[CTLPT %c%d %s", IsRational ? 'P' : 'E', NumCoords,
		    IsRational ? _IPReal2Str(Coords[0]) : "");
	    
	    for (i = 1; i <= NumCoords; i++) {
		strcat(Str, " ");
	        strcat(Str, _IPReal2Str(Coords[i]));
	    }
	    strcat(Str,"]\n");
	    _IPFprintf(Handler, Indent, Str);
	    break;
	case IP_OBJ_MATRIX:
	    _IPFprintf(Handler, Indent, "[MATRIX\n");
	    for (i = 0; i < 4; i++)
		_IPFprintf(Handler, Indent + 8, "%s %s %s %s%s\n",
			 _IPReal2Str((*PObj -> U.Mat)[i][0]),
			 _IPReal2Str((*PObj -> U.Mat)[i][1]),
			 _IPReal2Str((*PObj -> U.Mat)[i][2]),
			 _IPReal2Str((*PObj -> U.Mat)[i][3]),
			 i == 3 ? "]" : "");
	    break;
	case IP_OBJ_INSTANCE:
	    _IPFprintf(Handler, Indent, "[INSTANCE %s\n",
		       PObj -> U.Instance -> Name);
	    for (i = 0; i < 4; i++)
		_IPFprintf(Handler, Indent + 8, "%s %s %s %s%s\n",
			 _IPReal2Str(PObj -> U.Instance -> Mat[i][0]),
			 _IPReal2Str(PObj -> U.Instance -> Mat[i][1]),
			 _IPReal2Str(PObj -> U.Instance -> Mat[i][2]),
			 _IPReal2Str(PObj -> U.Instance -> Mat[i][3]),
			 i == 3 ? "]" : "");
	    break;
	case IP_OBJ_STRING:
	    _IPFprintf(Handler, Indent, "[STRING \"%s\"]\n", PObj -> U.Str);
	    break;
	case IP_OBJ_LIST_OBJ:
	    /* PObj below is still const-safe... */
	    for (i = 0; (PObjTmp = IPListObjectGet((IPObjectStruct *) PObj,
						    i)) != NULL; i++) {
		if (PObjTmp == PObj)
		    IP_FATAL_ERROR(IP_ERR_LIST_CONTAIN_SELF);
		else
		    IPPutAllObjects(PObjTmp, Handler, Indent);
	    }
	    break;
	case IP_OBJ_CURVE:
    	    CagdCrvWriteToFile2(PObj -> U.Crvs, Handler,
				Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
	        IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_SURFACE:
	    CagdSrfWriteToFile2(PObj -> U.Srfs, Handler,
				Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
	        IP_FATAL_ERROR_EX(IP_ERR_CAGD_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_TRIMSRF:
	    TrimWriteTrimmedSrfToFile2(PObj -> U.TrimSrfs, Handler,
				       Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
	        IP_FATAL_ERROR_EX(IP_ERR_TRIM_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_TRIVAR:
	    TrivTVWriteToFile2(PObj -> U.Trivars, Handler,
			       Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
		IP_FATAL_ERROR_EX(IP_ERR_TRIV_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_TRISRF:
	    TrngTriSrfWriteToFile2(PObj -> U.TriSrfs, Handler,
				   Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
		IP_FATAL_ERROR_EX(IP_ERR_TRNG_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_MODEL:
	    MdlWriteModelToFile2(PObj -> U.Mdls, Handler,
				 Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
		IP_FATAL_ERROR_EX(IP_ERR_MDL_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	case IP_OBJ_MULTIVAR:
	    MvarMVWriteToFile2(PObj -> U.MultiVars, Handler,
			       Indent, NULL, &ErrStr);
	    if (ErrStr != NULL)
		IP_FATAL_ERROR_EX(IP_ERR_MVAR_LIB_ERR, IP_ERR_NO_LINE_NUM,
				  ErrStr);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNDEF_OBJECT_FOUND);
	    break;
    }

    Indent -= 4;
    _IPFprintf(Handler, Indent, "]\n");			/* Close the object. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as fprintf but with indentation.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    A handler to the open stream.				     M
*   Indent:     All printing will start at this column.                      M
*   va_alist:   Do "man stdarg"                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSetPrintFunc, IPSetFloatFormat, IPPrintFunc		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IPFprintf, files 		                                             M
*****************************************************************************/
#ifdef USE_VARARGS
void _IPFprintf(int Handler, int Indent, char *va_alist, ...)
{
    char *Format, Line[IRIT_LINE_LEN_VLONG];
    int i;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void _IPFprintf(int Handler, int Indent, char *Format, ...)
{
    char *p, Line[IRIT_LINE_LEN];
    int i;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    if (IPPrintFunc != NULL || _IPStream[Handler].f != NULL) {
	for (i = 0; Indent >= 8; i++, Indent -= 8)
	    Line[i] = '\t';
	while (Indent--)
	    Line[i++] = ' ';
	    Line[i++] = 0;

	if (IPPrintFunc != NULL)
	    IPPrintFunc(Line);
	else
	    fprintf(_IPStream[Handler].f, Line);

	IRIT_VSPRINTF(p, Format, ArgPtr);

	if (IPPrintFunc != NULL)
	    IPPrintFunc(p);
	else
	    fprintf(_IPStream[Handler].f, p);
    }
    else {  /* _IPStream[Handler].f == NULL - it is a callback/socket. */
	/* No need for indentation if writing to a callback/socket. */
        IRIT_VSPRINTF(p, Format, ArgPtr);
	_IPStream[Handler].WriteBlockFunc(Handler, p, (int) strlen(p));
    }

    va_end(ArgPtr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print the attributes of given attribute list.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:    A handler to the open stream.				     *
*   Attr:     Attributes to put out.                                         *
*   Indent:   Indentation to put attributes at.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPPutAttributes(int Handler,
			    const IPAttributeStruct *Attr,
			    int Indent)
{
    int Count = 0;

    Attr = AttrTraceAttributes(Attr, Attr);

    while (Attr) {
	if (Attr -> Type == IP_ATTR_OBJ) {
	    if (Attr -> U.PObj -> ObjType != IP_OBJ_UNDEF) {
	        _IPFprintf(Handler, 0, "\n");
		_IPFprintf(Handler, Indent + 4, "[%s\n", AttrGetAttribName(Attr));
		IPPutAllObjects(Attr -> U.PObj, Handler, Indent + 8);
		_IPFprintf(Handler, Indent + 4, "]\n");
		_IPFprintf(Handler, Indent + 4, "");
	    }
	}
	else 
	    _IPFprintf(Handler, 0, "%s ", Attr2String(Attr, TRUE));

	Attr = AttrTraceAttributes(Attr, NULL);

	if (++Count >= 2 && Attr != NULL) {
	    /* Allow two attributes at most per line. */
	    _IPFprintf(Handler, 0, "\n");
	    _IPFprintf(Handler, Indent + 4, "");
	    Count = 0;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Puts an IRIT matrix file.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   File:       File to write the matrix file to.                            M
*   ViewMat, ProjMat:    Matrices to put.                                    M
*   HasProjMat: TRUE if has a perspective matrix, FALSE otherwise.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetMatrixFile                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPutMatrixFile                                                          M
*****************************************************************************/
int IPPutMatrixFile(const char *File,
		    IrtHmgnMatType ViewMat,
		    IrtHmgnMatType ProjMat,
		    int HasProjMat)
{
    char *p, FullName[IRIT_LINE_LEN_VLONG];
    int	i, j;
    FILE *f;

    if (File == NULL)
	return FALSE;

    strcpy(FullName, File);
    if ((p = strrchr(FullName, '.')) != NULL &&
	(stricmp(p + 1, IRIT_TEXT_DATA_FILE) != 0 ||
	 stricmp(p + 1, IRIT_BINARY_DATA_FILE) != 0 ||
	 stricmp(p + 1, IRIT_MATRIX_DATA_FILE) != 0))
        *p = 0;
    strcat(FullName, ".");
    strcat(FullName, IRIT_MATRIX_DATA_FILE);

#if defined(AMIGA) && !defined(__SASC)
    if (IRT_STR_ZERO_LEN(File) || (f = fopen(File, "w")) == NULL) {
#else
    if (IRT_STR_ZERO_LEN(File) || (f = fopen(File, "wt")) == NULL) {
#endif /* defined(AMIGA) && !defined(__SASC) */
	return FALSE;
    }

    fprintf(f, "[OBJECT MATRICES\n    [OBJECT VIEW_MAT\n\t[MATRIX");
    for (i = 0; i < 4; i++) {
	fprintf(f, "\n\t   ");
	for (j = 0; j < 4; j++)
	    fprintf(f, " %s", _IPReal2Str(ViewMat[i][j]));
    }
    fprintf(f, "\n\t]\n    ]\n");

    if (HasProjMat) {
	fprintf(f, "    [OBJECT PRSP_MAT\n\t[MATRIX");
	for (i = 0; i < 4; i++) {
	    fprintf(f, "\n\t   ");
	    for (j = 0; j < 4; j++)
		fprintf(f, " %s", _IPReal2Str(ProjMat[i][j]));
	}
	fprintf(f, "\n\t]\n    ]\n");
    }

    fprintf(f, "]\n");

    fclose(f);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Convert a real number into a string.					     *
*   The routine maintains six different buffers simultanuously so up to six  *
* consecutive calls can be issued from same printf and still have valid      *
* strings.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   R:        A  real number to convert to a string.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:   A string representing R allocated statically.                  *
*****************************************************************************/
const char *_IPReal2Str(IrtRType R)
{
    IRIT_STATIC_DATA int j, k,
	i = 0;
    IRIT_STATIC_DATA char Buffer[6][IRIT_LINE_LEN];

    if (IRIT_FABS(R) < IPZeroNumIritEps)
	R = 0.0;	 		   /* Round off very small numbers. */

    sprintf(Buffer[i], _IPGlblFloatFormat, R);

    for (k = 0; !isdigit(Buffer[i][k]) && k < IRIT_LINE_LEN; k++);
    if (k >= IRIT_LINE_LEN) {
	IRIT_WARNING_MSG_PRINTF("Warning: Conversion of real number (%f) failed, zero coerced.\n", R);
	R = 0.0;
    }

    for (j = (int) strlen(Buffer[i]) - 1; Buffer[i][j] == ' ' && j > k; j--);
    if (strchr(Buffer[i], '.') != NULL &&
	strchr(Buffer[i], 'e') == NULL &&
	strchr(Buffer[i], 'E') == NULL)
	for (; Buffer[i][j] == '0' && j > k; j--);
    Buffer[i][j + 1] = 0;

    j = i;
    i = (i + 1) % 6;
    return Buffer[j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Convert a real number into a string.					     M
*   The routine maintains six different buffers simultanuously so up to six  M
* consecutive calls can be issued from same printf and still have valid      M
* strings.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   R:        A real number to convert to a string.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:   A string representing R allocated statically.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSetPrintFunc, IPSetFloatFormat, IPPrintFunc		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvReal2Str, files 	                                             M
*****************************************************************************/
const char *IPCnvReal2Str(IrtRType R)
{
    return _IPReal2Str(R);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the printing function to call if needs to redirect printing.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PrintFunc:   A function that gets a single string it should print.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPrintFuncType:   Old value of this state.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvReal2Str, IPSetFloatFormat, IPPrintFunc		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetPrintFunc, files                                                    M
*****************************************************************************/
IPPrintFuncType IPSetPrintFunc(IPPrintFuncType PrintFunc)
{
    IPPrintFuncType
	OldVal = IPPrintFunc;

    IPPrintFunc = PrintFunc;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the filtering mode of degenerated geomerty while save/load.           M
*                                                                            *
* PARAMETERS:                                                                M
*   FilterDegeneracies:   TRUE to filter, FALSE to load/dump anyway.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	Old value of this state.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetFilterDegen, files, degeneracies                                    M
*****************************************************************************/
int IPSetFilterDegen(int FilterDegeneracies)
{
    int OldVal = _IPFilterDegeneracies;

    _IPFilterDegeneracies = FilterDegeneracies;
    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the floating point printing format.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   FloatFormat:    A printf style floating point printing format string.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *: old float format.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetFloatFormat, files                                                  M
*****************************************************************************/
char *IPSetFloatFormat(const char *FloatFormat)
{
    IRIT_STATIC_DATA char Str[IRIT_LINE_LEN];
    IrtRType t;

    /* Not a full-proof test but something. */
    if (strlen(FloatFormat) >= 2 &&
	strchr(FloatFormat, '%') != NULL &&
	(strchr(FloatFormat, 'e') != NULL ||
	 strchr(FloatFormat, 'f') != NULL ||
	 strchr(FloatFormat, 'g') != NULL ||
	 strchr(FloatFormat, 'E') != NULL ||
	 strchr(FloatFormat, 'F') != NULL ||
	 strchr(FloatFormat, 'G') != NULL)) {
        strcpy(Str, _IPGlblFloatFormat);
	_IPGlblFloatFormat = IritStrdup(FloatFormat);
    }
    else {
	IP_FATAL_ERROR(IP_ERR_ILLEGAL_FLOAT_FRMT);
        strcpy(Str, _IPGlblFloatFormat);
    }

    /* Determine what is a zero to round off. */
    for (t = 1e-25; t < 1e-6; t *= 10) {
        char TStr[IRIT_LINE_LEN];
	double r;

        sprintf(TStr, _IPGlblFloatFormat, t);
	if (sscanf(TStr, "%lf", &r) == 1 && r != 0.0)
	    break;
    }
    IPZeroNumIritEps = t;

    return Str;
}
