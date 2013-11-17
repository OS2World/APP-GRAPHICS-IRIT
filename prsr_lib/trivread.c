/******************************************************************************
* TrivRead.c - Tri-variate(s) reading from files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include "prsr_loc.h"
#include "ctype.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file trivariates.					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the trivariate from. 		                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVReadFromFile, files, read, trivariates                             M
*****************************************************************************/
TrivTVStruct *TrivTVReadFromFile(const char *FileName,
				 char **ErrStr,
				 int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = "File not found";
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	   Token != IP_TOKEN_OPEN_PAREN);

    /* We found beginning of definition - read one: */
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRIVAR ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("TRIVAR key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    IPCloseStream(Handler, TRUE);

    switch (Token) {
	case IP_TOKEN_BEZIER:
	    return TrivBzrTVReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return TrivBspTVReadFromFile(FileName, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a trivariate.					     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of trivariate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVReadFromFile2, files, read, trivariates                            M
*****************************************************************************/
TrivTVStruct *TrivTVReadFromFile2(int Handler, char **ErrStr, int *ErrLine)
{
    char StringToken[IRIT_LINE_LEN];

    switch (_IPGetToken(Handler, StringToken)) {
	case IP_TOKEN_BEZIER:
	    return TrivBzrTVReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return TrivBspTVReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bezier trivariates.					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the trivariate from. 		                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVReadFromFile, files, read, trivariates                          M
*****************************************************************************/
TrivTVStruct *TrivBzrTVReadFromFile(const char *FileName,
				    char **ErrStr,
				    int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    TrivTVStruct *TV,
	*TVTail = NULL,
	*TVList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	TV = TrivBzrTVReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (TVList == NULL)
	    TVList = TVTail = TV;
	else {
	    TVTail -> Pnext = TV;
	    TVTail = TV;
	}
    }

    IPCloseStream(Handler, TRUE);

    return TVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bezier trivariate.				     M
*   If NameWasRead is TRUE, it is assumed prefix "[TRIVAR BEZIER" has        M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of trivariate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRIVAR BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVReadFromFile2, files, read, trivariates                         M
*****************************************************************************/
TrivTVStruct *TrivBzrTVReadFromFile2(int Handler,
				     CagdBType NameWasRead,
				     char **ErrStr,
				     int *ErrLine)
{
    CagdPointType PType;
    IPTokenType Token;
    int i, j, ULength, VLength, WLength, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    TrivTVStruct *NewTV;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRIVAR ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BEZIER) {
	    *ErrStr = IRIT_EXP_STR("TRIVAR BEZIER key words expected");
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
        sscanf(StringToken, "%d", &ULength) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &VLength) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &WLength) != 1) {
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

    NewTV = TrivBzrTVNew(ULength, VLength, WLength, PType);

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < ULength * VLength * WLength; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					&NewTV -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewTV -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrivTVFree(NewTV);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bspline trivariates.				     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the trivariate from. 		                     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVReadFromFile, files, read, trivariates                          M
*****************************************************************************/
TrivTVStruct *TrivBspTVReadFromFile(const char *FileName,
				    char **ErrStr,
				    int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    TrivTVStruct *TV,
	*TVTail = NULL,
	*TVList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	TV = TrivBspTVReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (TVList == NULL)
	    TVList = TVTail = TV;
	else {
	    TVTail -> Pnext = TV;
	    TVTail = TV;
	}
    }

    IPCloseStream(Handler, TRUE);

    return TVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bspline trivariate.				     M
*   If NameWasRead is TRUE, it is assumed prefix "[TRIVAR BSPLINE" has       M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of trivariate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRIVAR BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The read trivariate, or NULL if an error occured.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVReadFromFile2, files, read, trivariates                         M
*****************************************************************************/
TrivTVStruct *TrivBspTVReadFromFile2(int Handler,
				     CagdBType NameWasRead,
				     char **ErrStr,
				     int *ErrLine)
{
    int i, j, k, Len, Order, ULength, VLength, WLength, UOrder, VOrder, WOrder,
	MaxCoord, *KVPeriodic;
    char StringToken[IRIT_LINE_LEN];
    CagdRType *KnotVector, **KnotVectorPtr;
    CagdPointType PType;
    IPTokenType Token;
    TrivTVStruct *NewTV;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRIVAR ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BSPLINE) {
	    *ErrStr = IRIT_EXP_STR("TRIVAR BSPLINE key words expected");
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
        sscanf(StringToken, "%d", &ULength) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &VLength) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &WLength) != 1) {
	*ErrStr = IRIT_EXP_STR("BSPLINE Number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &UOrder) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &VOrder) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &WOrder) != 1) {
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

    NewTV = TrivBspTVNew(ULength, VLength, WLength,
			 UOrder, VOrder, WOrder, PType);

    /* Read the knot vectors first: */
    for (k = 0; k < 3; k++) {
	switch (k) {
	    case 0:
		KnotVector = NewTV -> UKnotVector;
		KnotVectorPtr = &NewTV -> UKnotVector;
		Len = NewTV -> UOrder + NewTV -> ULength;
		Order = NewTV -> UOrder;
		KVPeriodic = &NewTV -> UPeriodic;
		break;
	    case 1:
		KnotVector = NewTV -> VKnotVector;
		KnotVectorPtr = &NewTV -> VKnotVector;
		Len = NewTV -> VOrder + NewTV -> VLength;
		Order = NewTV -> VOrder;
		KVPeriodic = &NewTV -> VPeriodic;
		break;
	    default:
	    case 2:
		KnotVector = NewTV -> WKnotVector;
		KnotVectorPtr = &NewTV -> WKnotVector;
		Len = NewTV -> WOrder + NewTV -> WLength;
		Order = NewTV -> WOrder;
		KVPeriodic = &NewTV -> WPeriodic;
		break;
	}

    	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
    	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
    	}
    	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_KV) {
	    if (Token == IP_TOKEN_KVP) {
	        IritFree(KnotVector);
		Len += Order - 1;
		*KnotVectorPtr = KnotVector = 
		    IritMalloc(sizeof(CagdRType) * Len);
	        *KVPeriodic = TRUE;
	    }
	    else {
	        *ErrStr = IRIT_EXP_STR("KV/KVP expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}

	for (i = 0; i < Len; i++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
    		sscanf(StringToken, IP_IRIT_FLOAT_READ, &KnotVector[i]) != 1) {
    		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}

	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
	}
    }

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < ULength * VLength * WLength; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewTV -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewTV -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		TrivTVFree(NewTV);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    TrivTVFree(NewTV);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	TrivTVFree(NewTV);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTV;
}
