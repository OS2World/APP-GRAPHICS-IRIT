/******************************************************************************
* TrimRead.c - Trimmed surfaces reading from files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 94.					      *
******************************************************************************/

#include "prsr_loc.h"
#include "ctype.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file trimmed surfaces.					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the trimmed surface from. 		             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *: The read trimmed surface, or NULL if an error occured.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimReadTrimmedSrfFromFile, files, read, trimmed surfaces                M
*****************************************************************************/
TrimSrfStruct *TrimReadTrimmedSrfFromFile(const char *FileName,
					  char **ErrStr,
					  int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    TrimSrfStruct *TrimSrf;

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
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRIMSRF ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("TRIMSRF key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    TrimSrf = TrimReadTrimmedSrfFromFile2(Handler, TRUE, ErrStr, ErrLine);

    IPCloseStream(Handler, TRUE);

    return TrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a trimmed surface.					     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of trimmed surface.							     M
*   Assumes the [TRIMSRF prefix was read if NameWasRead.		     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the TRIMSRF BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *: The read trimmed surface, or NULL if an error occured.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimReadTrimmedSrfFromFile2, files, read, trimmed surfaces               M
*****************************************************************************/
TrimSrfStruct *TrimReadTrimmedSrfFromFile2(int Handler,
					   CagdBType NameWasRead,
					   char **ErrStr,
					   int *ErrLine)
{
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    TrimSrfStruct *NewTrimSrf;
    CagdSrfStruct *Srf;
    TrimCrvStruct
	*TrimCrvs = NULL;
    
    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_TRIMSRF) {
	    *ErrStr = IRIT_EXP_STR("TRIMSRF key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_SURFACE) {
        *ErrStr = IRIT_EXP_STR("SURFACE key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if ((Srf = CagdSrfReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
	return NULL;

    while (TRUE) {
	TrimCrvStruct *TrimCrv;
	TrimCrvSegStruct
	    *TrimCrvSegs = NULL;

	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    _IPUnGetToken(Handler, StringToken);
	    break;
	}

	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_TRIMCRV) {
	    *ErrStr = IRIT_EXP_STR("TRIMCRV key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}

	while (TRUE) {
	    CagdCrvStruct *EucCrv, *UVCrv;
	    TrimCrvSegStruct *TrimCrvSeg;

	    if ((Token = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_OPEN_PAREN) {
		_IPUnGetToken(Handler, StringToken);
		break;
	    }

	    if ((Token = _IPGetToken(Handler, StringToken)) !=
						       IP_TOKEN_TRIMCRVSEG) {
		*ErrStr = IRIT_EXP_STR("TRIMCRVSEG key words expected");
		*ErrLine = _IPStream[Handler].LineNum;
		return NULL;
	    }
	    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"[\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		return NULL;
	    }
	    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CURVE) {
		*ErrStr = IRIT_EXP_STR("CURVE key words expected");
		*ErrLine = _IPStream[Handler].LineNum;
		return NULL;
	    }

	    if ((UVCrv = CagdCrvReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
		return NULL;

	    if ((Token = _IPGetToken(Handler, StringToken)) ==
						       IP_TOKEN_OPEN_PAREN) {
		if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CURVE) {
		    *ErrStr = IRIT_EXP_STR("CURVE key words expected");
		    *ErrLine = _IPStream[Handler].LineNum;
		    return NULL;
		}

		if ((EucCrv = CagdCrvReadFromFile2(Handler, ErrStr, ErrLine))
								       == NULL)
		    return NULL;
	    }
	    else {
		_IPUnGetToken(Handler, StringToken);
	        EucCrv = NULL;
	    }

	    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
		*ErrStr = IRIT_EXP_STR("\"]\" expected");
		*ErrLine = _IPStream[Handler].LineNum;
		return NULL;
	    }

	    TrimCrvSeg = TrimCrvSegNew(UVCrv, EucCrv);

	    IRIT_LIST_PUSH(TrimCrvSeg, TrimCrvSegs);
	}

	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}

	TrimCrv = TrimCrvNew(CagdListReverse(TrimCrvSegs));
	IRIT_LIST_PUSH(TrimCrv, TrimCrvs);
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if (TrimCrvs == NULL) {
        IRIT_WARNING_MSG_PRINTF("Warning: Trimmed surface detected with no trimming curves, line %d\n",
				_IPStream[Handler].LineNum);
    }

    NewTrimSrf = TrimSrfNew(Srf, CagdListReverse(TrimCrvs), TRUE);

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewTrimSrf;
}
