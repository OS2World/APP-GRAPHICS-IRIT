/******************************************************************************
* TrngRead.c - Triangular surface(s) reading from files.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "prsr_loc.h"
#include "ctype.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file triangular surfaces.				     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the triangular surface from. 		             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		  occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfReadFromFile, files, read, triangular surfaces                 M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfReadFromFile(const char *FileName,
					    char **ErrStr,
					    int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	   Token != IP_TOKEN_OPEN_PAREN);

    /* We found beginning of definition - read one: */
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRISRF ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("TRISRF key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    IPCloseStream(Handler, TRUE);

    switch (Token) {
	case IP_TOKEN_GREGORY:
	    return TrngGrgTriSrfReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BEZIER:
	    return TrngBzrTriSrfReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return TrngBspTriSrfReadFromFile(FileName, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or GREGORY Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a triangular surface.				     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of triangular surface.						     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		 occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfReadFromFile2, files, read, triangular surfaces                M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfReadFromFile2(int Handler,
					     char **ErrStr,
					     int *ErrLine)
{
    char StringToken[IRIT_LINE_LEN];

    switch (_IPGetToken(Handler, StringToken)) {
	case IP_TOKEN_GREGORY:
	    return TrngGrgTriSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BEZIER:
	    return TrngBzrTriSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return TrngBspTriSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or GREGORY Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bezier triangular surfaces.				     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the triangular surface from. 	                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfReadFromFile, files, read, triangular surfaces              M
*****************************************************************************/
TrngTriangSrfStruct *TrngBzrTriSrfReadFromFile(const char *FileName,
					       char **ErrStr,
					       int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    TrngTriangSrfStruct *TriSrf,
	*TriSrfTail = NULL,
	*TriSrfList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	TriSrf = TrngBzrTriSrfReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (TriSrfList == NULL)
	    TriSrfList = TriSrfTail = TriSrf;
	else {
	    TriSrfTail -> Pnext = TriSrf;
	    TriSrfTail = TriSrf;
	}
    }

    IPCloseStream(Handler, TRUE);

    return TriSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bezier triangular surface.			     M
*   If NameWasRead is TRUE, it is assumed prefix "[TRISRF BEZIER" has        M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of triangular surface.						     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRISRF BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
* 		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfReadFromFile2, files, read, triangular surfaces             M
*****************************************************************************/
TrngTriangSrfStruct *TrngBzrTriSrfReadFromFile2(int Handler,
						CagdBType NameWasRead,
						char **ErrStr,
						int *ErrLine)
{
    CagdPointType PType;
    IPTokenType Token;
    int i, j, Length, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    TrngTriangSrfStruct *NewTriSrf;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRISRF ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BEZIER) {
	    *ErrStr = IRIT_EXP_STR("TRISRF BEZIER key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
    }

    if ((Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_OPEN_PAREN) {
	if ((*ErrStr = _IPGetSurfaceAttributes(Handler)) != NULL) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    else
	_IPUnGetToken(Handler, StringToken);

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
        sscanf(StringToken, "%d", &Length) != 1) {
	*ErrStr = IRIT_EXP_STR("BEZIER Number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
        strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("BEZIER Point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    PType = CAGD_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewTriSrf = TrngBzrTriSrfNew(Length, PType);

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < TRNG_TRISRF_MESH_SIZE(NewTriSrf); i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					    &NewTriSrf -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					    &NewTriSrf -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bspline triangular surfaces.			     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the triangular surface from. 	                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfReadFromFile, files, read, triangular surfaces              M
*****************************************************************************/
TrngTriangSrfStruct *TrngBspTriSrfReadFromFile(const char *FileName,
					       char **ErrStr,
					       int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    TrngTriangSrfStruct *TriSrf,
	*TriSrfTail = NULL,
	*TriSrfList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	TriSrf = TrngBspTriSrfReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (TriSrfList == NULL)
	    TriSrfList = TriSrfTail = TriSrf;
	else {
	    TriSrfTail -> Pnext = TriSrf;
	    TriSrfTail = TriSrf;
	}
    }

    IPCloseStream(Handler, TRUE);

    return TriSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bspline triangular surface.			     M
*   If NameWasRead is TRUE, it is assumed prefix "[TRISRF BSPLINE" has       M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of triangular surface.						     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRISRF BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfReadFromFile2, files, read, triangular surfaces             M
*****************************************************************************/
TrngTriangSrfStruct *TrngBspTriSrfReadFromFile2(int Handler,
						CagdBType NameWasRead,
						char **ErrStr,
						int *ErrLine)
{
    int i, j, Len, Length, Order, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    CagdRType *KnotVector;
    CagdPointType PType;
    IPTokenType Token;
    TrngTriangSrfStruct *NewTriSrf;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRISRF ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BSPLINE) {
	    *ErrStr = IRIT_EXP_STR("TRISRF BSPLINE key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
    }

    if ((Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_OPEN_PAREN) {
	if ((*ErrStr = _IPGetSurfaceAttributes(Handler)) != NULL) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    else
	_IPUnGetToken(Handler, StringToken);

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
        sscanf(StringToken, "%d", &Length) != 1) {
	*ErrStr = IRIT_EXP_STR("BSPLINE Number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &Order) != 1) {
	*ErrStr = IRIT_EXP_STR("BSPLINE Order expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
        strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("BSPLINE Point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    PType = CAGD_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewTriSrf = TrngBspTriSrfNew(Length, Order, PType);

    /* Read the knot vector first: */
    KnotVector = NewTriSrf -> KnotVector;
    Len = NewTriSrf -> Order + NewTriSrf -> Length;

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_KV) {
	*ErrStr = IRIT_EXP_STR("KV expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }

    for (i = 0; i < Len; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	    sscanf(StringToken, IP_IRIT_FLOAT_READ, &KnotVector[i]) != 1) {
	    *ErrStr = IRIT_EXP_STR("Numeric data expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
    }

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < TRNG_TRISRF_MESH_SIZE(NewTriSrf); i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					    &NewTriSrf -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					    &NewTriSrf -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Gregory triangular surfaces.			     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the triangular surface from. 	                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
*		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGrgTriSrfReadFromFile, files, read, triangular surfaces              M
*****************************************************************************/
TrngTriangSrfStruct *TrngGrgTriSrfReadFromFile(const char *FileName,
					       char **ErrStr,
					       int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    TrngTriangSrfStruct *TriSrf,
	*TriSrfTail = NULL,
	*TriSrfList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	TriSrf = TrngGrgTriSrfReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (TriSrfList == NULL)
	    TriSrfList = TriSrfTail = TriSrf;
	else {
	    TriSrfTail -> Pnext = TriSrf;
	    TriSrfTail = TriSrf;
	}
    }

    IPCloseStream(Handler, TRUE);

    return TriSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Gregory triangular surface.			     M
*   If NameWasRead is TRUE, it is assumed prefix "[TRISRF GREGORY" has        M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of triangular surface.						     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRISRF GREGORY prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *: The read triangular surface, or NULL if an error  M
* 		occured.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGrgTriSrfReadFromFile2, files, read, triangular surfaces             M
*****************************************************************************/
TrngTriangSrfStruct *TrngGrgTriSrfReadFromFile2(int Handler,
						CagdBType NameWasRead,
						char **ErrStr,
						int *ErrLine)
{
    CagdPointType PType;
    IPTokenType Token;
    int i, j, Length, MaxCoord;
	 int CurIndex;
    char StringToken[IRIT_LINE_LEN];
    TrngTriangSrfStruct *NewTriSrf;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRISRF ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_GREGORY) {
	    *ErrStr = IRIT_EXP_STR("TRISRF GREGORY key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
    }

    if ((Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_OPEN_PAREN) {
	if ((*ErrStr = _IPGetSurfaceAttributes(Handler)) != NULL) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    else
	_IPUnGetToken(Handler, StringToken);

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
        sscanf(StringToken, "%d", &Length) != 1) {
	*ErrStr = IRIT_EXP_STR("GREGORY Number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
        strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("GREGORY Point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    PType = CAGD_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewTriSrf = TrngGrgTriSrfNew(Length, PType);

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < TRNG_LENGTH_MESH_SIZE(NewTriSrf -> Length); i++) {
	if ((Token = _IPGetToken(Handler,
				 StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
	/* Check for double control points. */
	if (IP_GREGORY_DBL_POINT(i, Length)) {
	    if ((Token = _IPGetToken(Handler,
				     StringToken)) != IP_TOKEN_OPEN_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"[\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
		       &NewTriSrf -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}
	
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
		       &NewTriSrf -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}

	/* Check for double control points. */
	if (IP_GREGORY_DBL_POINT(i, Length)) {
	    /* Compute the place in the global table where the second       */
	    /* control points in double control point case, must be         */
	    CurIndex = IP_GREGORY_DBL_PT_IDX(i, Length);

	    if ((Token = _IPGetToken(Handler,
				     StringToken)) != IP_TOKEN_CLOSE_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"]\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	    if ((Token = _IPGetToken(Handler,
				     StringToken)) != IP_TOKEN_OPEN_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"[\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }

	    if (CAGD_IS_RATIONAL_PT(PType)) {
		if ((Token = _IPGetToken(Handler,
					 StringToken)) != IP_TOKEN_OTHER ||
		    sscanf(StringToken, IP_IRIT_FLOAT_READ,
			   &NewTriSrf -> Points[0][CurIndex]) != 1) {
		    *ErrStr = IRIT_EXP_STR("Numeric data expected");
		    *ErrLine = _IPStream[Handler].LineNum;
		    TrngTriSrfFree(NewTriSrf);
		    return NULL;
		}
	    }

	    for (j = 1; j <= MaxCoord; j++) {
		if ((Token = _IPGetToken(Handler,
					 StringToken)) != IP_TOKEN_OTHER ||
		    sscanf(StringToken, IP_IRIT_FLOAT_READ,
			   &NewTriSrf -> Points[j][CurIndex]) != 1) {
		    *ErrStr = IRIT_EXP_STR("Numeric data expected");
		    *ErrLine = _IPStream[Handler].LineNum;
		    TrngTriSrfFree(NewTriSrf);
		    return NULL;
		}
	    }
	    if ((Token = _IPGetToken(Handler,
				     StringToken)) != IP_TOKEN_CLOSE_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"]\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrngTriSrfFree(NewTriSrf);
		return NULL;
	    }
	}

	if ((Token = _IPGetToken(Handler,
				 StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrngTriSrfFree(NewTriSrf);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrngTriSrfFree(NewTriSrf);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTriSrf;
}
