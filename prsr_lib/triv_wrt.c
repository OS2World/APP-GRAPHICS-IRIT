/******************************************************************************
* Triv_Wrt.c - Trivariate writing to files.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write trivariates to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVWriteToFile, files, write,trivariates                              M
*****************************************************************************/
int TrivTVWriteToFile(const TrivTVStruct *TVs,
		      const char *FileName,
		      int Indent,
		      const char *Comment,
		      char **ErrStr)
{
    switch (TVs -> GType) {
        case TRIV_TVBEZIER_TYPE:
	    return TrivBzrTVWriteToFile(TVs, FileName, Indent, Comment,
					ErrStr);
        case TRIV_TVBSPLINE_TYPE:
	    return TrivBspTVWriteToFile(TVs, FileName, Indent, Comment,
					ErrStr);
	default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write trivariates to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVWriteToFile2, files, write, trivariates                            M
*****************************************************************************/
int TrivTVWriteToFile2(const TrivTVStruct *TVs,
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    switch (TVs -> GType) {
        case TRIV_TVBEZIER_TYPE:
	    return TrivBzrTVWriteToFile2(TVs, Handler, Indent, Comment,
					 ErrStr);
        case TRIV_TVBSPLINE_TYPE:
	    return TrivBspTVWriteToFile2(TVs, Handler, Indent, Comment,
					 ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write trivariate(s) to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in file f.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVWriteToFile3, files, write                                         M
*****************************************************************************/
int TrivTVWriteToFile3(const TrivTVStruct *TVs,
		       FILE *f,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int
	Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = TrivTVWriteToFile2(TVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier trivariates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in file.                                          M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVWriteToFile, files, write,trivariates                           M
*****************************************************************************/
int TrivBzrTVWriteToFile(const TrivTVStruct *TVs,
			 const char *FileName,
			 int Indent,
			 const char *Comment,
			 char **ErrStr)
{
    int i, Handler;
    FILE *f;

    if ((f = fopen(FileName, "w")) == NULL) {
	*ErrStr = IRIT_EXP_STR("Fail to open file");
	return FALSE;
    }
    Handler = IPOpenStreamFromFile(f, FALSE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    i = TrivBzrTVWriteToFile2(TVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier trivariates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVWriteToFile2, files, write, trivariates                         M
*****************************************************************************/
int TrivBzrTVWriteToFile2(const TrivTVStruct *TVs,
			  int Handler,
			  int Indent,
			  const char *Comment,
			  char **ErrStr)
{
    int i, j, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - bezier TV(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (TVs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(TVs -> PType);

	if (TVs -> GType != TRIV_TVBEZIER_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given tri-variate(s) is (are) not BEZIER trivariate(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[TRIVAR BEZIER %d %d %d %c%c\n",
		TVs -> ULength, TVs -> VLength, TVs -> WLength,
		CAGD_IS_RATIONAL_PT(TVs -> PType) ? 'P' : 'E',
		MaxCoord + '0');
	Indent += 4;

	for (i = 0;
	     i < TVs -> VLength * TVs -> ULength * TVs -> WLength;
	     i++) {
	    if (i && i % TVs -> ULength == 0)
		_IPFprintf(Handler, 0, "\n"); /* Put empty line between raws.*/
	    if (i && i % TVs -> UVPlane == 0)
		_IPFprintf(Handler, 0, "\n");   /* Put 2 lns between planes. */

	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(TVs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(TVs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(TVs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	TVs = TVs -> Pnext;
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline trivariates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVWriteToFile, files, write,trivariates                           M
*****************************************************************************/
int TrivBspTVWriteToFile(const TrivTVStruct *TVs,
			 const char *FileName,
			 int Indent,
			 const char *Comment,
			 char **ErrStr)
{
    int i, Handler;
    FILE *f;

    if ((f = fopen(FileName, "w")) == NULL) {
	*ErrStr = IRIT_EXP_STR("Fail to open file");
	return FALSE;
    }
    Handler = IPOpenStreamFromFile(f, FALSE, IPSenseBinaryFile(FileName),
				   FALSE, FALSE);

    i = TrivBspTVWriteToFile2(TVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline trivariates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVWriteToFile2, files, write, trivariates                         M
*****************************************************************************/
int TrivBspTVWriteToFile2(const TrivTVStruct *TVs,
			  int Handler,
			  int Indent,
			  const char *Comment,
			  char **ErrStr)
{
    int i, j, Len, MaxCoord;
    CagdRType *KnotVector;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - bspline TV(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (TVs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(TVs -> PType);

	if (TVs -> GType != TRIV_TVBSPLINE_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given tri-variate(s) is (are) not Bspline trivariate(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[TRIVAR BSPLINE %d %d %d %d %d %d %c%c\n",
		TVs -> ULength, TVs -> VLength, TVs -> WLength,
		TVs -> UOrder, TVs -> VOrder, TVs -> WOrder,
		CAGD_IS_RATIONAL_PT(TVs -> PType) ? 'P' : 'E',
		MaxCoord + '0');
	Indent += 4;

	/* Put out the knot vectors: */
	for (i = 0; i < 3; i++) {
	    int Periodic;

	    switch (i) {
		case 0:
		    KnotVector = TVs -> UKnotVector;
		    Periodic = TVs -> UPeriodic;
		    Len = TVs -> ULength + TVs -> UOrder
					+ (Periodic ? TVs -> UOrder - 1 : 0);
		    break;
		case 1:
		    KnotVector = TVs -> VKnotVector;
		    Periodic = TVs -> VPeriodic;
		    Len = TVs -> VLength + TVs -> VOrder
					+ (Periodic ? TVs -> VOrder - 1 : 0);
		    break;
		default:
		case 2:
		    KnotVector = TVs -> WKnotVector;
		    Periodic = TVs -> WPeriodic;
		    Len = TVs -> WLength + TVs -> WOrder
					+ (Periodic ? TVs -> WOrder - 1 : 0);
		    break;
	    }

	    _IPFprintf(Handler, Indent, Periodic ? "[KVP" : "[KV");
	    for (j = 0; j < Len; j++) {
		if (j && j % MAX_KNOTS_PER_LINE == 0) {
		    _IPFprintf(Handler, 0, "\n");
		    _IPFprintf(Handler, Indent + 4, "");
		}
		_IPFprintf(Handler, 0, " %s", _IPReal2Str(KnotVector[j]));
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	/* Put out the control mesh. */
	for (i = 0;
	     i < TVs -> VLength * TVs -> ULength * TVs -> WLength;
	     i++) {
	    if (i && i % TVs -> ULength == 0)
		_IPFprintf(Handler, 0, "\n"); /* Put empty line between raws.*/
	    if (i && i % TVs -> UVPlane == 0)
		_IPFprintf(Handler, 0, "\n");   /* Put 2 lns between planes. */

	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(TVs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(TVs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(TVs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	TVs = TVs -> Pnext;
    }

    return *ErrStr == NULL;
}
