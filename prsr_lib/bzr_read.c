/******************************************************************************
* Bzr_Read.c - Bezier handling routines - read from file.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bezier curve(s).					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the Bezier curve from.                             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvReadFromFile, files, read                                          M
*****************************************************************************/
CagdCrvStruct *BzrCrvReadFromFile(const char *FileName,
				  char **ErrStr,
				  int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    CagdCrvStruct *Crv,
	*CrvTail = NULL,
	*CrvList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
        Crv = BzrCrvReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (CrvList == NULL)
	    CrvList = CrvTail = Crv;
	else {
	    CrvTail -> Pnext = Crv;
	    CrvTail = Crv;
	}
    }

    IPCloseStream(Handler, TRUE);

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bezier curve.					     M
*   If NameWasRead is TRUE, it is assumed prefix "[CURVE BEZIER" has         M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller.          M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of curve. 								     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  TRUE if "[CURVE BEZIER" has been read, FALSE otherwise.    M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvReadFromFile2, files, read, stream                                 M
*****************************************************************************/
CagdCrvStruct *BzrCrvReadFromFile2(int Handler,
				   CagdBType NameWasRead,
				   char **ErrStr,
				   int *ErrLine)
{
    CagdBType
	IsBezier = TRUE; 
    CagdPointType PType;
    IPTokenType Token;
    int i, j, Length, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    CagdCrvStruct *NewCrv;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CURVE ||
	    ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_BEZIER &&
	     Token != IP_TOKEN_POWER)) {
            *ErrStr = IRIT_EXP_STR("CURVE BEZIER/POWER key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
	else if (Token == IP_TOKEN_POWER)
	    IsBezier = FALSE;
    }

    if ((Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_OPEN_PAREN) {
	if ((*ErrStr = _IPGetCurveAttributes(Handler)) != NULL) {
            *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    else
	_IPUnGetToken(Handler, StringToken);

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &Length) != 1) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER curve - number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
	strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER curve - point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    PType = CAGD_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewCrv = BzrCrvNew(Length, PType);
    if (!IsBezier)
        NewCrv -> GType = CAGD_CPOWER_TYPE;

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < Length; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) !=
							IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    CagdCrvFree(NewCrv);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) !=
							      IP_TOKEN_OTHER ||
		sscanf(StringToken, IP_IRIT_FLOAT_READ,
					       &NewCrv -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		CagdCrvFree(NewCrv);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) !=
							      IP_TOKEN_OTHER ||
		sscanf(StringToken, IP_IRIT_FLOAT_READ,
					       &NewCrv -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		CagdCrvFree(NewCrv);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    CagdCrvFree(NewCrv);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	CagdCrvFree(NewCrv);
	return NULL;
    }

    *ErrLine = _IPStream[Handler].LineNum;
    *ErrStr = NULL;

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bezier surface(s).					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the Bezier surface from.                           M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfReadFromFile, files, read                                          M
*****************************************************************************/
CagdSrfStruct *BzrSrfReadFromFile(const char *FileName,
				  char **ErrStr,
				  int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    CagdSrfStruct *Srf,
	*SrfTail = NULL,
	*SrfList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	Srf = BzrSrfReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (SrfList == NULL)
	    SrfList = SrfTail = Srf;
	else {
	    SrfTail -> Pnext = Srf;
	    SrfTail = Srf;
	}
    }

    IPCloseStream(Handler, TRUE);

    return SrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bezier surface.					     M
*   If NameWasRead is TRUE, it is assumed prefix "[SURFACE BEZIER" has       M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller.          M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of surface. 								     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead: TRUE if "[SURFACE BEZIER" hasbeen read, FALSE otherwise.    M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfReadFromFile2, files, read, stream                                 M
*****************************************************************************/
CagdSrfStruct *BzrSrfReadFromFile2(int Handler,
				   CagdBType NameWasRead,
				   char **ErrStr,
				   int *ErrLine)
{
    CagdBType
	IsBezier = TRUE; 
    CagdPointType PType;
    IPTokenType Token;
    int i, j, ULength, VLength, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    CagdSrfStruct *NewSrf;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_SURFACE ||
	    ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_BEZIER &&
	     Token != IP_TOKEN_POWER)) {
	    *ErrStr = IRIT_EXP_STR("SURFACE BEZIER or SURFACE POWER key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
	else if (Token == IP_TOKEN_POWER)
	    IsBezier = FALSE;
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
        sscanf(StringToken, "%d", &ULength) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &VLength) != 1) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER surface - number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
        strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER surface - point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    PType = CAGD_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewSrf = BzrSrfNew(ULength, VLength, PType);
    if (!IsBezier)
        NewSrf -> GType = CAGD_SPOWER_TYPE;

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < ULength * VLength; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) !=
							IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    CagdSrfFree(NewSrf);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) !=
							     IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					&NewSrf -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		CagdSrfFree(NewSrf);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) !=
							      IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					       &NewSrf -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		CagdSrfFree(NewSrf);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    CagdSrfFree(NewSrf);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	CagdSrfFree(NewSrf);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewSrf;
}
