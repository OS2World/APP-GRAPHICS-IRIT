/******************************************************************************
* Bsp_Read.c - Bspline handling routines - read from file.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bspline curve(s).					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the Bspline curve from.                            M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvReadFromFile, files, read                                          M
*****************************************************************************/
CagdCrvStruct *BspCrvReadFromFile(const char *FileName,
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
    	Crv = BspCrvReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

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
* Reads from a file a Bspline curve.					     M
*   If NameWasRead is TRUE, it is assumed prefix "[CURVE BSPLINE" has        M
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
*   NameWasRead: TRUE if "[CURVE BSPLINE" has been read, FALSE otherwise.    M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The read curve, or NULL if an error occured.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvReadFromFile2, files, read, stream                                 M
*****************************************************************************/
CagdCrvStruct *BspCrvReadFromFile2(int Handler,
				   CagdBType NameWasRead,
				   char **ErrStr,
				   int *ErrLine)
{
    CagdPointType PType;
    IPTokenType Token;
    int i, j, Length, Order, MaxCoord, KVPeriodic;
    char StringToken[IRIT_LINE_LEN];
    CagdCrvStruct *NewCrv;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CURVE ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BSPLINE) {
            *ErrStr = IRIT_EXP_STR("CURVE BSPLINE key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
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

    /* Read the knot vector first: */
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_KV) {
	if (Token == IP_TOKEN_KVP) {
	    KVPeriodic = TRUE;
	}
	else {
	    *ErrStr = IRIT_EXP_STR("KV/KVP expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    else
	KVPeriodic = FALSE;

    NewCrv = KVPeriodic ? BspPeriodicCrvNew(Length, Order, TRUE, PType)
			: BspCrvNew(Length, Order, PType);

    for (i = 0; i < Order + Length + (KVPeriodic ? Order - 1 : 0); i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	    sscanf(StringToken, IP_IRIT_FLOAT_READ,
					     &NewCrv -> KnotVector[i]) != 1) {
	    *ErrStr = IRIT_EXP_STR("Numeric data expected");
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
* Reads from a file Bspline surface(s).					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the Bspline surface from.                          M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfReadFromFile, files, read                                          M
*****************************************************************************/
CagdSrfStruct *BspSrfReadFromFile(const char *FileName,
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
    	Srf = BspSrfReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

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
* Reads from a file a Bspline surface.					     M
*   If NameWasRead is TRUE, it is assumed prefix "[SURFACE BSPLINE" has      M
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
*   NameWasRead:  TRUE if "[SURFACE BSPLINE" hasbeen read, FALSE otherwise.  M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The read surface, or NULL if an error occured.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfReadFromFile2, files, read, stream                                 M
*****************************************************************************/
CagdSrfStruct *BspSrfReadFromFile2(int Handler,
				   CagdBType NameWasRead,
				   char **ErrStr,
				   int *ErrLine)
{
    int i, j, k, KVLen, ULength, VLength, UOrder, VOrder, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    CagdRType *KnotVector;
    CagdPointType PType;
    IPTokenType Token;
    CagdSrfStruct *NewSrf;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_SURFACE ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BSPLINE) {
	    *ErrStr = IRIT_EXP_STR("SURFACE BSPLINE key words expected");
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
	sscanf(StringToken, "%d", &VLength) != 1) {
	*ErrStr = IRIT_EXP_STR("BSPLINE Number of points expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &UOrder) != 1 ||
	(Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &VOrder) != 1) {
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

    NewSrf = BspSrfNew(ULength, VLength, UOrder, VOrder, PType);

    /* Read the knot vectors first: */
    for (k = 0; k < 2; k++) {
	int KVPeriodic;

    	if ((Token = _IPGetToken(Handler, StringToken)) !=
							IP_TOKEN_OPEN_PAREN) {
    	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    CagdSrfFree(NewSrf);
	    return NULL;
    	}
    	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_KV) {
	    if (Token == IP_TOKEN_KVP) {
		KVPeriodic = TRUE;
	    }
	    else {
		*ErrStr = IRIT_EXP_STR("KV/KVP expected");
		*ErrLine = _IPStream[Handler].LineNum;
		CagdSrfFree(NewSrf);
		return NULL;
	    }
	}
	else
	    KVPeriodic = FALSE;

	if (k == 0) {
	    NewSrf -> UPeriodic = KVPeriodic;
	    if (KVPeriodic) {
		/* Make Knot vector longer. */
		IritFree(NewSrf -> UKnotVector);
		NewSrf -> UKnotVector =
		    (CagdRType *) IritMalloc(sizeof(CagdRType) *
					     (NewSrf -> UOrder +
					      NewSrf -> ULength +
					      NewSrf -> UOrder - 1));
	    }
	    KnotVector = NewSrf -> UKnotVector;
	    KVLen = NewSrf -> UOrder + NewSrf -> ULength +
				(KVPeriodic ? NewSrf -> UOrder - 1 : 0);
	}
	else {
	    NewSrf -> VPeriodic = KVPeriodic;
	    if (KVPeriodic) {
		/* Make Knot vector longer. */
		IritFree(NewSrf -> VKnotVector);
		NewSrf -> VKnotVector =
		    (CagdRType *) IritMalloc(sizeof(CagdRType) *
					     (NewSrf -> VOrder +
					      NewSrf -> VLength +
					      NewSrf -> VOrder - 1));
	    }
	    KnotVector = NewSrf -> VKnotVector;
	    KVLen = NewSrf -> VOrder + NewSrf -> VLength +
				(KVPeriodic ? NewSrf -> VOrder - 1 : 0);
	}

	for (i = 0; i < KVLen; i++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) !=
							     IP_TOKEN_OTHER ||
    		sscanf(StringToken, IP_IRIT_FLOAT_READ, &KnotVector[i]) != 1) {
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
