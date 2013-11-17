/******************************************************************************
* CagdRead.c - Generic Curve/Surface reading from files.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file curve(s).						     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the curve from. 		                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvReadFromFile, files, read                                         M
*****************************************************************************/
CagdCrvStruct *CagdCrvReadFromFile(const char *FileName,
				   char **ErrStr,
				   int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    CagdCrvStruct *Crv;

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
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CURVE ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("CURVE key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    IPCloseStream(Handler, TRUE);

    switch (Token) {
	case IP_TOKEN_POWER:
	    Crv = BzrCrvReadFromFile(FileName, ErrStr, ErrLine);
	    Crv -> GType = CAGD_CPOWER_TYPE;
	    return Crv;
	case IP_TOKEN_BEZIER:
	    return BzrCrvReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return BspCrvReadFromFile(FileName, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE/BEZIER/POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file surface(s).						     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the surface from.    		                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfReadFromFile, files, read                                         M
*****************************************************************************/
CagdSrfStruct *CagdSrfReadFromFile(const char *FileName,
				   char **ErrStr,
				   int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    CagdSrfStruct *Srf;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	   Token != IP_TOKEN_OPEN_PAREN);

    /* We found beginning of definition - read one: */
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_SURFACE ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("SURFACE key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    IPCloseStream(Handler, TRUE);

    switch (Token) {
	case IP_TOKEN_POWER:
	    Srf = BzrSrfReadFromFile(FileName, ErrStr, ErrLine);
	    Srf -> GType = CAGD_SPOWER_TYPE;
	    return Srf;
	case IP_TOKEN_BEZIER:
	    return BzrSrfReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return BspSrfReadFromFile(FileName, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE/BEZIER/POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a stream a curve.						     M
*    It is assumed prefix "[CURVE" has already been read. This is useful for M
* a global parser which invokes this routine to read from a stream several   M
* times as a parent controller. 				             M
*   For exactly this reason, the given stream descriptor is NOT closed in    M
* the end.								     M
*   If error is found in reading the stream, ErrStr is set to a string       M
* describing it and ErrLine to line it occured in stream relative to         M
* begining of curve.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in strea of the error, if occured.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvReadFromFile2, files, read, stream                                M
*****************************************************************************/
CagdCrvStruct *CagdCrvReadFromFile2(int Handler, char **ErrStr, int *ErrLine)
{
    char StringToken[IRIT_LINE_LEN];
    CagdCrvStruct *Crv;

    switch (_IPGetToken(Handler, StringToken)) {
	case IP_TOKEN_POWER:
	    Crv = BzrCrvReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	    Crv -> GType = CAGD_CPOWER_TYPE;
	    return Crv;
	case IP_TOKEN_BEZIER:
	    return BzrCrvReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return BspCrvReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE/BEZIER/POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a stream a surface.					     M
*    It is assumed prefix "[SURFACE" has already been read. This is useful   M
* for a global parser which invokes this routine to read from a stream       M
* several times as a parent controller. 			             M
*   For exactly this reason, the given stream descriptor is NOT closed in    M
* the end.								     M
*   If error is found in reading the stream, ErrStr is set to a string       M
* describing it and ErrLine to line it occured in stream relative to         M
* begining of surface.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in stream of the error, if occured.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfReadFromFile2, files, read, stream                                M
*****************************************************************************/
CagdSrfStruct *CagdSrfReadFromFile2(int Handler, char **ErrStr, int *ErrLine)
{
    char StringToken[IRIT_LINE_LEN];
    CagdSrfStruct *Srf;

    switch (_IPGetToken(Handler, StringToken)) {
	case IP_TOKEN_POWER:
	    Srf = BzrSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	    Srf -> GType = CAGD_SPOWER_TYPE;
	    return Srf;
	case IP_TOKEN_BEZIER:
	    return BzrSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return BspSrfReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE/BEZIER/POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read from input stream the	following [ATTR ...] [ATTR ...].     *
* Note the '[' was already read.					     *
* Current supported attributes: None.					     *
* Returns NULL if O.k., otherwise string describing the error.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:      A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:       Always NULL                                                *
*****************************************************************************/
char *_IPGetCurveAttributes(int Handler)
{
    IPTokenType i;
    char StringToken[IRIT_LINE_LEN];

    do {
	switch (_IPGetToken(Handler, StringToken)) {
	    default:
		while ((i = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_CLOSE_PAREN &&
		       i != IP_TOKEN_EOF);
		if (i == IP_TOKEN_EOF)
		    return "EOF detected in middle of attribute.";
		break;
	}
    }
    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN);

    _IPUnGetToken(Handler, StringToken);

    return NULL;
}
/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to read from input stream the	following [ATTR ...] [ATTR ...].     *
* Note the '[' was already read.					     *
* Current supported attributes: None.					     *
* Returns NULL if O.k., otherwise string describing the error.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:      A handler to the open stream.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:       Always NULL                                                *
*****************************************************************************/
char *_IPGetSurfaceAttributes(int Handler)
{
    IPTokenType i;
    char StringToken[IRIT_LINE_LEN];

    do {
	switch (_IPGetToken(Handler, StringToken)) {
	    default:
		while ((i = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_CLOSE_PAREN &&
		       i != IP_TOKEN_EOF);
		if (i == IP_TOKEN_EOF)
		    return "EOF detected in middle of attribute.";
		break;
	}
    }
    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN);

    _IPUnGetToken(Handler, StringToken);

    return NULL;
}
