/******************************************************************************
* MvarRead.c - Multi-variate(s) reading from files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 97.					      *
******************************************************************************/

#include "prsr_loc.h"
#include "mvar_lib.h"
#include "ctype.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file multi-variates.					     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the multi-variate from. 		             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVReadFromFile, files, read, multi-variates                          M
*****************************************************************************/
MvarMVStruct *MvarMVReadFromFile(const char *FileName,
				 char **ErrStr,
				 int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    MvarMVStruct *MV;

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
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MULTIVAR ||
	(Token = _IPGetToken(Handler, StringToken)) == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("MULTIVAR key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    IPCloseStream(Handler, TRUE);

    switch (Token) {
	case IP_TOKEN_POWER:
	    MV = MvarBzrMVReadFromFile(FileName, ErrStr, ErrLine);
            MV -> GType = MVAR_POWER_TYPE;
	    return MV;
	case IP_TOKEN_BEZIER:
	    return MvarBzrMVReadFromFile(FileName, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return MvarBspMVReadFromFile(FileName, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a multi-variate.					     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of multi-variate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVReadFromFile2, files, read, multi-variates                         M
*****************************************************************************/
MvarMVStruct *MvarMVReadFromFile2(int Handler, char **ErrStr, int *ErrLine)
{
    char StringToken[IRIT_LINE_LEN];
    MvarMVStruct *MV;

    switch (_IPGetToken(Handler, StringToken)) {
        case IP_TOKEN_POWER:
	    MV = MvarBzrMVReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
            MV -> GType = MVAR_POWER_TYPE;
	    return MV;
	case IP_TOKEN_BEZIER:
	    return MvarBzrMVReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	case IP_TOKEN_BSPLINE:
	    return MvarBspMVReadFromFile2(Handler, TRUE, ErrStr, ErrLine);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bezier multi-variates.				     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the multi-variate from. 		             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVReadFromFile, files, read, multi-variates                       M
*****************************************************************************/
MvarMVStruct *MvarBzrMVReadFromFile(const char *FileName,
				    char **ErrStr,
				    int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    MvarMVStruct *MV,
	*MVTail = NULL,
	*MVList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	MV = MvarBzrMVReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (MVList == NULL)
	    MVList = MVTail = MV;
	else {
	    MVTail -> Pnext = MV;
	    MVTail = MV;
	}
    }

    IPCloseStream(Handler, TRUE);

    return MVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bezier multi-variate.				     M
*   If NameWasRead is TRUE, it is assumed prefix "[MULTIVAR BEZIER" has      M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of multi-variate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the MULTIVAR BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVReadFromFile2, files, read, multi-variates                      M
*****************************************************************************/
MvarMVStruct *MvarBzrMVReadFromFile2(int Handler,
				     CagdBType NameWasRead,
				     char **ErrStr,
				     int *ErrLine)
{
    CagdBType
	IsBezier = TRUE; 
    MvarPointType PType;
    IPTokenType Token;
    int i, j, Dim, MaxCoord, *Lengths;
    char StringToken[IRIT_LINE_LEN];
    MvarMVStruct *NewMV;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MULTIVAR ||
	    ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_BEZIER &&
	     Token != IP_TOKEN_POWER)) {
	    *ErrStr = IRIT_EXP_STR("MVAR BEZIER/POWER key words expected");
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
        sscanf(StringToken, "%d", &Dim) != 1) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER's dimension expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Lengths = (int *) IritMalloc(Dim * sizeof(int));
    for (i = 0; i < Dim; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	    sscanf(StringToken, "%d", &Lengths[i]) != 1) {
	    *ErrStr = IRIT_EXP_STR("BEZIER/POWER's length of mesh expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }

    Token = _IPGetToken(Handler, StringToken);
    if (!IP_IS_TOKEN_POINT(Token) ||
        strlen(StringToken) != 2 ||
	(StringToken[0] != 'E' && StringToken[0] != 'P') ||
	!isdigit(StringToken[1]) ||
	atoi(&StringToken[1]) > CAGD_MAX_PT_COORD) {
	*ErrStr = IRIT_EXP_STR("BEZIER/POWER Point type expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    PType = MVAR_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));
    NewMV = MvarBzrMVNew(Dim, Lengths, PType);
    if (!IsBezier)
        NewMV -> GType = MVAR_POWER_TYPE;
    IritFree(Lengths);

    /* Read the points themselves: */
    MaxCoord = MVAR_NUM_OF_PT_COORD(PType);
    for (i = 0; i < MVAR_CTL_MESH_LENGTH(NewMV); i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
					&NewMV -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewMV -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	MvarMVFree(NewMV);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file Bspline multi-variates.				     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occured in file.		     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the multi-variate from. 		             M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVReadFromFile, files, read, multi-variates                       M
*****************************************************************************/
MvarMVStruct *MvarBspMVReadFromFile(const char *FileName,
				    char **ErrStr,
				    int *ErrLine)
{
    int Handler;
    FILE *f;
    char StringToken[IRIT_LINE_LEN];
    MvarMVStruct *MV,
	*MVTail = NULL,
	*MVList = NULL;

    if ((f = fopen(FileName, "r")) == NULL) {
	*ErrStr = IRIT_EXP_STR("File not found");
	*ErrLine = 0;
	return NULL;
    }
    Handler = IPOpenStreamFromFile(f, TRUE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    while (_IPGetToken(Handler, StringToken) == IP_TOKEN_OPEN_PAREN) {
	_IPUnGetToken(Handler, StringToken);
    	MV = MvarBspMVReadFromFile2(Handler, FALSE, ErrStr, ErrLine);

	if (MVList == NULL)
	    MVList = MVTail = MV;
	else {
	    MVTail -> Pnext = MV;
	    MVTail = MV;
	}
    }

    IPCloseStream(Handler, TRUE);

    return MVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a Bspline multi-variate.				     M
*   If NameWasRead is TRUE, it is assumed prefix "[MULTIVAR BSPLINE" has     M
* already been read. This is useful for a global parser which invokes this   M
* routine to read from a file several times as a parent controller. 	     M
*   For exactly this reason, the given file descriptor is NOT closed in the  M
* end.									     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occured in file relative to begining  M
* of multi-variate.							     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the MULTIVAR BEZIER prefix.	     M
*   ErrStr:       Will be initialized if an error has occured.               M
*   ErrLine:      Line number in file FileName of the error, if occured.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: The read multi-variate, or NULL if an error occured.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVReadFromFile2, files, read, multi-variates                      M
*****************************************************************************/
MvarMVStruct *MvarBspMVReadFromFile2(int Handler,
				     CagdBType NameWasRead,
				     char **ErrStr,
				     int *ErrLine)
{
    int i, j, k, Dim, Len, *Lengths, *Orders, MaxCoord;
    char StringToken[IRIT_LINE_LEN];
    CagdRType *KnotVector;
    MvarPointType PType;
    IPTokenType Token;
    MvarMVStruct *NewMV;

    _IPStream[Handler].LineNum = *ErrLine;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MULTIVAR ||
	    _IPGetToken(Handler, StringToken) != IP_TOKEN_BSPLINE) {
	    *ErrStr = IRIT_EXP_STR("MVAR BSPLINE key words expected");
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
        sscanf(StringToken, "%d", &Dim) != 1) {
	*ErrStr = IRIT_EXP_STR("BEZIER's dimension expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Lengths = (int *) IritMalloc(Dim * sizeof(int));
    for (i = 0; i < Dim; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	    sscanf(StringToken, "%d", &Lengths[i]) != 1) {
	    *ErrStr = IRIT_EXP_STR("BSPLINE's lengths of mesh expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
    }
    Orders = (int *) IritMalloc(Dim * sizeof(int));
    for (i = 0; i < Dim; i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	    sscanf(StringToken, "%d", &Orders[i]) != 1) {
	    *ErrStr = IRIT_EXP_STR("BSPLINE's orders expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
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
    PType = MVAR_MAKE_PT_TYPE(StringToken[0] == 'P', atoi(&StringToken[1]));

    NewMV = MvarBspMVNew(Dim, Lengths, Orders, PType);
    IritFree(Lengths);
    IritFree(Orders);

    /* Read the knot vectors first: */
    for (k = 0; k < Dim; k++) {
	KnotVector = NewMV -> KnotVectors[k];
	Len = NewMV -> Orders[k] + NewMV -> Lengths[k];

    	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
    	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
    	}
    	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_KV) {
	    if (Token == IP_TOKEN_KVP) {
	        IritFree(KnotVector);
		Len += NewMV -> Orders[k] - 1;
		KnotVector = NewMV -> KnotVectors[k] = 
		    IritMalloc(sizeof(CagdRType) * Len);
		NewMV -> Periodic[k] = TRUE;
	    }
	    else {
	        *ErrStr = IRIT_EXP_STR("KV/KVP expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}
	else {
	    NewMV -> Periodic[k] = FALSE;
	}

	for (i = 0; i < Len; i++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
    		sscanf(StringToken, IP_IRIT_FLOAT_READ, &KnotVector[i]) != 1) {
    		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}

	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
	}
    }

    /* Read the points themselves: */
    MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    for (i = 0; i < MVAR_CTL_MESH_LENGTH(NewMV); i++) {
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
	}
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewMV -> Points[0][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}
	for (j = 1; j <= MaxCoord; j++) {
	    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	        sscanf(StringToken, IP_IRIT_FLOAT_READ,
						&NewMV -> Points[j][i]) != 1) {
		*ErrStr = IRIT_EXP_STR("Numeric data expected");
		*ErrLine = _IPStream[Handler].LineNum;
		MvarMVFree(NewMV);
		return NULL;
	    }
	}
	if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"]\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    MvarMVFree(NewMV);
	    return NULL;
	}
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_CLOSE_PAREN) {
        *ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	MvarMVFree(NewMV);
	return NULL;
    }

    *ErrStr = NULL;
    *ErrLine = _IPStream[Handler].LineNum;

    return NewMV;
}
