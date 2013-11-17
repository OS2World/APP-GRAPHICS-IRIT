/******************************************************************************
* Trng_Wrt.c - Triangular surface writing to files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write triangular surfaces to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfWriteToFile, files, write, triangular surfaces                 M
*****************************************************************************/
int TrngTriSrfWriteToFile(const TrngTriangSrfStruct *TriSrfs,
			  const char *FileName,
			  int Indent,
			  const char *Comment,
			  char **ErrStr)
{
    switch (TriSrfs -> GType) {
        case TRNG_TRISRF_BEZIER_TYPE:
	    return TrngBzrTriSrfWriteToFile(TriSrfs, FileName, Indent,
					    Comment, ErrStr);
        case TRNG_TRISRF_BSPLINE_TYPE:
	    return TrngBspTriSrfWriteToFile(TriSrfs, FileName, Indent,
					    Comment, ErrStr);
 	case TRNG_TRISRF_GREGORY_TYPE:
	    return TrngGrgTriSrfWriteToFile(TriSrfs, FileName, Indent,
					    Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or GREGORY Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write triangular surfaces to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfWriteToFile2, files, write, triangular surfaces                M
*****************************************************************************/
int TrngTriSrfWriteToFile2(const TrngTriangSrfStruct *TriSrfs,
			   int Handler,
			   int Indent,
			   const char *Comment,
			   char **ErrStr)
{
    switch (TriSrfs -> GType) {
        case TRNG_TRISRF_BEZIER_TYPE:
	    return TrngBzrTriSrfWriteToFile2(TriSrfs, Handler, Indent,
					     Comment, ErrStr);
        case TRNG_TRISRF_BSPLINE_TYPE:
	    return TrngBspTriSrfWriteToFile2(TriSrfs, Handler, Indent,
					     Comment, ErrStr);
        case TRNG_TRISRF_GREGORY_TYPE:
	    return TrngGrgTriSrfWriteToFile2(TriSrfs, Handler, Indent,
					     Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or GREGORY Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write triangular surfaces(s) to the given file.         M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in file f.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfWriteToFile3, files, write                                     M
*****************************************************************************/
int TrngTriSrfWriteToFile3(const TrngTriangSrfStruct *TriSrfs,
			   FILE *f,
			   int Indent,
			   const char *Comment,
			   char **ErrStr)
{
    int
	Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = TrngTriSrfWriteToFile2(TriSrfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier triangular surfaces to the given file.     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in file.                                          M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfWriteToFile, files, write, triangular surfaces              M
*****************************************************************************/
int TrngBzrTriSrfWriteToFile(const TrngTriangSrfStruct *TriSrfs,
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

    i = TrngBzrTriSrfWriteToFile2(TriSrfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier triangular surfaces to the given file.     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfWriteToFile2, files, write, triangular surfaces             M
*****************************************************************************/
int TrngBzrTriSrfWriteToFile2(const TrngTriangSrfStruct *TriSrfs,
			      int Handler,
			      int Indent,
			      const char *Comment,
			      char **ErrStr)
{
    int i, j, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# cagd_lib - bezier TRISRF(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (TriSrfs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(TriSrfs -> PType);

	if (TriSrfs -> GType != TRNG_TRISRF_BEZIER_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given triangular surface(s) is (are) not BEZIER triangular surface(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[TRISRF BEZIER %d %c%c\n",
		TriSrfs -> Length,
		CAGD_IS_RATIONAL_PT(TriSrfs -> PType) ? 'P' : 'E',
		MaxCoord + '0');
	Indent += 4;

	for (i = 0; i < TRNG_TRISRF_MESH_SIZE(TriSrfs); i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(TriSrfs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(TriSrfs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(TriSrfs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	TriSrfs = TriSrfs -> Pnext;
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline triangular surfaces to the given file.    M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfWriteToFile, files, write,triangular surfaces               M
*****************************************************************************/
int TrngBspTriSrfWriteToFile(const TrngTriangSrfStruct *TriSrfs,
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

    i = TrngBspTriSrfWriteToFile2(TriSrfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline triangular surfaces to the given file.    M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfWriteToFile2, files, write, triangular surfaces             M
*****************************************************************************/
int TrngBspTriSrfWriteToFile2(const TrngTriangSrfStruct *TriSrfs,
			      int Handler,
			      int Indent,
			      const char *Comment,
			      char **ErrStr)
{
    int i, j, Len, MaxCoord;
    CagdRType *KnotVector;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - bspline TRISRF(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (TriSrfs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(TriSrfs -> PType);

	if (TriSrfs -> GType != TRNG_TRISRF_BSPLINE_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given triangular surface(s) is (are) not BSPLINE triangular surface(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[TRISRF BSPLINE %d %d %c%c\n",
		TriSrfs -> Length, TriSrfs -> Order,
		CAGD_IS_RATIONAL_PT(TriSrfs -> PType) ? 'P' : 'E',
		MaxCoord + '0');
	Indent += 4;

	/* Put out the knot vectors: */
	KnotVector = TriSrfs -> KnotVector;
	Len = TriSrfs -> Length + TriSrfs -> Order;

	_IPFprintf(Handler, Indent, "[KV");
	for (j = 0; j < Len; j++) {
	    if (j && j % MAX_KNOTS_PER_LINE == 0) {
		_IPFprintf(Handler, 0, "\n");
		_IPFprintf(Handler, Indent + 4, "");
	    }
	    _IPFprintf(Handler, 0, " %s", _IPReal2Str(KnotVector[j]));
	}
	_IPFprintf(Handler, 0, "]\n");

	/* Put out the control mesh. */
	for (i = 0; i < TRNG_TRISRF_MESH_SIZE(TriSrfs); i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(TriSrfs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(TriSrfs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(TriSrfs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	TriSrfs = TriSrfs -> Pnext;
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Gregory triangular surfaces to the given file.    M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in file.                                          M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGrgTriSrfWriteToFile, files, write, triangular surfaces              M
*****************************************************************************/
int TrngGrgTriSrfWriteToFile(const TrngTriangSrfStruct *TriSrfs,
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

    i = TrngGrgTriSrfWriteToFile2(TriSrfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Gregory triangular surfaces to the given file.    M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:   To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngGrgTriSrfWriteToFile2, files, write, triangular surfaces             M
*****************************************************************************/
int TrngGrgTriSrfWriteToFile2(const TrngTriangSrfStruct *TriSrfs,
			      int Handler,
			      int Indent,
			      const char *Comment,
			      char **ErrStr)
{
    int i, j, MaxCoord, Length;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - gregory TRISRF(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (TriSrfs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(TriSrfs -> PType);
	Length = TriSrfs -> Length;
	
	if (TriSrfs -> GType != TRNG_TRISRF_GREGORY_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given triangular surface(s) is (are) not GREGORY triangular surface(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[TRISRF GREGORY %d %c%c\n", Length,
		   CAGD_IS_RATIONAL_PT(TriSrfs -> PType) ? 'P' : 'E',
		   MaxCoord + '0');
	Indent += 4;

	for (i = 0; i < TRNG_LENGTH_MESH_SIZE(Length); i++) {
	    if (IP_GREGORY_DBL_POINT(i, Length)) {
		_IPFprintf(Handler, Indent, "[\n");
		_IPFprintf(Handler, Indent + 4, "[");
	    }
	    else
		_IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(TriSrfs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(TriSrfs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(TriSrfs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");

	    if (IP_GREGORY_DBL_POINT(i, Length)) {
		int ii = IP_GREGORY_DBL_PT_IDX(i, Length);

		_IPFprintf(Handler, Indent + 4, "[");
		if (CAGD_IS_RATIONAL_PT(TriSrfs -> PType))
		    _IPFprintf(Handler, 0, "%s ",
			       _IPReal2Str(TriSrfs -> Points[0][ii]));
		for (j = 1; j <= MaxCoord; j++) {
		    _IPFprintf(Handler, 0, "%s",
			       _IPReal2Str(TriSrfs -> Points[j][ii]));
		    if (j < MaxCoord)
			_IPFprintf(Handler, 0, " ");
		}
		_IPFprintf(Handler, 0, "]\n");
		_IPFprintf(Handler, Indent, "]\n");
	    }
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	TriSrfs = TriSrfs -> Pnext;
    }

    return *ErrStr == NULL;
}
