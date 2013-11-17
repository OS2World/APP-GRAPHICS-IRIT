/******************************************************************************
* Mdl_Read.c - Model reading from files.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 96.					      *
******************************************************************************/

#include "prsr_loc.h"
#include "mdl_lib.h"
#include "ctype.h"

static MdlTrimSegStruct *MdlGetTrimmedSeg(int Handler,
					  char **ErrStr,
					  int *ErrLine);
static MdlTrimSrfStruct *MdlGetTrimmedSrf(int Handler,
					  char **ErrStr,
					  int *ErrLine);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a model.						     M
*   If error is detected in reading the file, ErrStr is set to a string      M
* describing the error and Line to the line it occurred in file.	     M
*   If no error is detected *ErrStr = NULL.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:     To read the model from. 			             M
*   ErrStr:       Will be initialized if an error has occurred.              M
*   ErrLine:      Line number in file FileName of the error, if occurred.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *: The read model surface, or NULL if an error occured.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlReadModelSrfFromFile2                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlReadModelFromFile, files, read, Mdlmed surfaces                       M
*****************************************************************************/
MdlModelStruct *MdlReadModelFromFile(const char *FileName,
				     char **ErrStr,
				     int *ErrLine)
{
    int Handler;
    FILE *f;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    MdlModelStruct *Model;

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
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_MODEL ||
	Token == IP_TOKEN_EOF) {
        *ErrStr = IRIT_EXP_STR("MODEL key word expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Model = MdlReadModelFromFile2(Handler, TRUE, ErrStr, ErrLine);

    IPCloseStream(Handler, TRUE);

    return Model;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads from a file a model.						     M
*   If error is found in reading the file, ErrStr is set to a string         M
* describing it and ErrLine to line it occurred in file relative to model.   M
*   Assumes the [MODEL prefix was read if NameWasRead.			     M
*   If no error is detected *ErrStr is set to NULL.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:      A handler to the open stream.				     M
*   NameWasRead:  If FALSE, also reads the MODEL prefix.		     M
*   ErrStr:       Will be initialized if an error has occurred.              M
*   ErrLine:      Line number in file FileName of the error, if occurred.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *: The read model, or NULL if an error occured.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlReadModelSrfFromFile                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlReadModelFromFile2, files, read, model			             M
*****************************************************************************/
MdlModelStruct *MdlReadModelFromFile2(int Handler,
				      CagdBType NameWasRead,
				      char **ErrStr,
				      int *ErrLine)
{
    int i, NumOfTrimSrfs, NumOfTrimSegs;
    IPTokenType Token;
    char StringToken[IRIT_LINE_LEN];
    MdlTrimSrfStruct
	*TrimSrfsLast = NULL,
	*TrimSrfs = NULL;
    MdlTrimSegStruct
	*TrimSegsLast = NULL,
	*TrimSegs = NULL;
    MdlModelStruct *Model;

    if (!NameWasRead) {
	while ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_EOF &&
	       Token != IP_TOKEN_OPEN_PAREN);

	/* We found beginning of definition - read one: */
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MODEL) {
	    *ErrStr = IRIT_EXP_STR("MODEL key word expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
        }
    }

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &NumOfTrimSrfs) != 1) {
	*ErrStr = IRIT_EXP_STR("MODEL Number of trimmed surfaces expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &NumOfTrimSegs) != 1) {
	*ErrStr = IRIT_EXP_STR("MODEL Number of trimmed segments expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    /* Gets all the trimming surfaces in the model. */
    for (i = 0; i < NumOfTrimSrfs; i++) {
	if (TrimSrfs == NULL)
	    TrimSrfsLast =
	        TrimSrfs = MdlGetTrimmedSrf(Handler, ErrStr, ErrLine);
	else {
	    TrimSrfsLast -> Pnext = MdlGetTrimmedSrf(Handler, ErrStr, ErrLine);
	    TrimSrfsLast = TrimSrfsLast -> Pnext;
	}
	if (TrimSrfsLast == NULL)
	    return NULL; /* Error occurred. */
    }

    /* Gets all the trimming segments in the model. */
    for (i = 0; i < NumOfTrimSegs; i++) {
	if (TrimSegs == NULL)
	    TrimSegsLast =
	        TrimSegs = MdlGetTrimmedSeg(Handler, ErrStr, ErrLine);
	else {
	    TrimSegsLast -> Pnext = MdlGetTrimmedSeg(Handler, ErrStr, ErrLine);
	    TrimSegsLast = TrimSegsLast -> Pnext;
	}
	if (TrimSegsLast == NULL)
	    return NULL; /* Error occurred. */
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    Model = MdlModelNew2(TrimSrfs, TrimSegs);

    /* Back patch the pointers in the trimmed surfaces to point to the */
    /* proper trimming segment in the trimming segment list.           */
    MdlPatchTrimmingSegPointers(Model);

    return Model;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one trimming segement structure.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:      A handler to the open stream.				     *
*   ErrStr:       Will be initialized if an error has occured.               *
*   ErrLine:      Line number in file FileName of the error, if occured.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSegStruct *:   Read trimming segment structure, NULL if error.    *
*****************************************************************************/
static MdlTrimSegStruct *MdlGetTrimmedSeg(int Handler,
					  char **ErrStr,
					  int *ErrLine)
{
    int i;
    IritIntPtrSizeType Srf1Num, Srf2Num;
    char StringToken[IRIT_LINE_LEN];
    IPMdlCurveMaskType
	CurveMask = IP_MDL_NO_CRV;
    CagdCrvStruct
	*UV1Crv = NULL,
        *UV2Crv = NULL,
	*EucCrv = NULL;

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MDLTSEG) {
        *ErrStr = IRIT_EXP_STR("MDLTSEG key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &i) != 1) {
	*ErrStr = IRIT_EXP_STR("MDLTSEG curve mask expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    CurveMask = (IPMdlCurveMaskType) i;

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &i) != 1) {
	*ErrStr = IRIT_EXP_STR("MDLTSEG surface index expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    Srf1Num = i;

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &i) != 1) {
	*ErrStr = IRIT_EXP_STR("MDLTSEG surface index expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    Srf2Num = i;

    /* Get the three curves or whatever is provided. */
    if (CurveMask & IP_MDL_UV1_CRV) {
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

	if ((UV1Crv = CagdCrvReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
	    return NULL; /* Error occured. */
    }
    if (CurveMask & IP_MDL_UV2_CRV) {
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

	if ((UV2Crv = CagdCrvReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
	    return NULL; /* Error occured. */
    }
    if (CurveMask & IP_MDL_EUC_CRV) {
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

	if ((EucCrv = CagdCrvReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
	    return NULL; /* Error occured. */
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    return MdlTrimSegNew(UV1Crv, UV2Crv, EucCrv,
			 (MdlTrimSrfStruct *) Srf1Num,
			 (MdlTrimSrfStruct *) Srf2Num);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads one trimmed surface structure.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Handler:      A handler to the open stream.				     *
*   ErrStr:       Will be initialized if an error has occured.               *
*   ErrLine:      Line number in file FileName of the error, if occured.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSrfStruct *:   Read trimmed surface structure, NULL if error.     *
*****************************************************************************/
static MdlTrimSrfStruct *MdlGetTrimmedSrf(int Handler,
					  char **ErrStr,
					  int *ErrLine)
{
    int i, NumOfLoops;
    char StringToken[IRIT_LINE_LEN];
    IPTokenType Token;
    CagdSrfStruct *Srf;
    MdlLoopStruct
	*Loops = NULL,
	*LoopsLast = NULL;

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MDLTSRF) {
        *ErrStr = IRIT_EXP_STR("MDLTSRF key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OTHER ||
	sscanf(StringToken, "%d", &NumOfLoops) != 1) {
	*ErrStr = IRIT_EXP_STR("MDLTSRF number of loops expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    /* Read the surface. */
    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"[\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }
    if ((Token = _IPGetToken(Handler, StringToken)) != IP_TOKEN_SURFACE ||
	Token == IP_TOKEN_EOF) {
	*ErrStr = IRIT_EXP_STR("SURFACE key words expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    if ((Srf = CagdSrfReadFromFile2(Handler, ErrStr, ErrLine)) == NULL)
	return NULL;

    /* Read all the loops. */
    for (i = 0; i < NumOfLoops; i++) {
	MdlTrimSegRefStruct
	    *SegRef = NULL,
	    *SegRefLast = NULL;

	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_OPEN_PAREN) {
	    *ErrStr = IRIT_EXP_STR("\"[\" expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}
	if (_IPGetToken(Handler, StringToken) != IP_TOKEN_MDLLOOP) {
	    *ErrStr = IRIT_EXP_STR("MDLLOOP key words expected");
	    *ErrLine = _IPStream[Handler].LineNum;
	    return NULL;
	}

	while (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	    int j;
	    IritIntPtrSizeType Index;

	    if (sscanf(StringToken, "%d", &j) != 1) {
		*ErrStr = IRIT_EXP_STR("MDLLOOP index of segment expected");
		*ErrLine = _IPStream[Handler].LineNum;
		return NULL;
	    }
	    Index = j;

	    if (SegRef == NULL)
	        SegRefLast = SegRef = MdlTrimSegRefNew((MdlTrimSegStruct *)
							      IRIT_ABS(Index));
	    else {
		SegRefLast -> Pnext = MdlTrimSegRefNew((MdlTrimSegStruct *)
							      IRIT_ABS(Index));
		SegRefLast = SegRefLast -> Pnext;
	    }
	    SegRefLast -> Reversed = Index < 0;
	}

	if (Loops == NULL)
	    LoopsLast = Loops = MdlLoopNew(SegRef);
	else {
	    LoopsLast -> Pnext = MdlLoopNew(SegRef);
	    LoopsLast = LoopsLast -> Pnext;
	}
	if (LoopsLast == NULL)
	    return NULL; /* Error occured. */
    }

    if (_IPGetToken(Handler, StringToken) != IP_TOKEN_CLOSE_PAREN) {
	*ErrStr = IRIT_EXP_STR("\"]\" expected");
	*ErrLine = _IPStream[Handler].LineNum;
	return NULL;
    }

    /* Build and return the trimmed surface... */
    return MdlTrimSrfNew(Srf, Loops, TRUE, FALSE);
}
