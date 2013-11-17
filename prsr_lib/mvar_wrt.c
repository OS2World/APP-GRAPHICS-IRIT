/******************************************************************************
* Mvar_Wrt.c - Multi-Variate writing to files.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write multi-variates to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if succesful, FALSE otherwise.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVWriteToFile, files, write, multi-variates                          M
*****************************************************************************/
int MvarMVWriteToFile(const MvarMVStruct *MVs,
		      const char *FileName,
		      int Indent,
		      const char *Comment,
		      char **ErrStr)
{
    switch (MVs -> GType) {
        case MVAR_BEZIER_TYPE:
        case MVAR_POWER_TYPE:
	    return MvarBzrMVWriteToFile(MVs, FileName, Indent, Comment,
					ErrStr);
        case MVAR_BSPLINE_TYPE:
	    return MvarBspMVWriteToFile(MVs, FileName, Indent, Comment,
					ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write multi-variates to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if succesful, FALSE otherwise.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVWriteToFile2, files, write, multi-variates                         M
*****************************************************************************/
int MvarMVWriteToFile2(const MvarMVStruct *MVs,
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    switch (MVs -> GType) {
        case MVAR_BEZIER_TYPE:
        case MVAR_POWER_TYPE:
	    return MvarBzrMVWriteToFile2(MVs, Handler, Indent, Comment,
					 ErrStr);
        case MVAR_BSPLINE_TYPE:
	    return MvarBspMVWriteToFile2(MVs, Handler, Indent, Comment,
					 ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE or BEZIER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write multi-variate(s) to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in file f.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVWriteToFile3, files, write                                         M
*****************************************************************************/
int MvarMVWriteToFile3(const MvarMVStruct *MVs,
		       FILE *f,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int
	Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = MvarMVWriteToFile2(MVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier multi-variates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in file.                                          M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVWriteToFile, files, write,multi-variates                        M
*****************************************************************************/
int MvarBzrMVWriteToFile(const MvarMVStruct *MVs,
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

    i = MvarBzrMVWriteToFile2(MVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bezier multi-variates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVWriteToFile2, files, write, multi-variates                      M
*****************************************************************************/
int MvarBzrMVWriteToFile2(const MvarMVStruct *MVs,
			  int Handler,
			  int Indent,
			  const char *Comment,
			  char **ErrStr)
{
    int i, j, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - bezier MV(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (MVs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(MVs -> PType);

	if (MVs -> GType == MVAR_BEZIER_TYPE) {
	    _IPFprintf(Handler, Indent, "[MULTIVAR BEZIER %d  ", MVs -> Dim);
	}
	else if (MVs -> GType == MVAR_POWER_TYPE) {
	    _IPFprintf(Handler, Indent, "[MULTIVAR POWER %d  ", MVs -> Dim);
	}
	else {
	    *ErrStr = IRIT_EXP_STR("Given multi-variate(s) is (are) not BEZIER or POWER multi-variate(s)");
	    break;
	}

	for (i = 0; i < MVs -> Dim; i++)
	    _IPFprintf(Handler, 0, " %d", MVs -> Lengths[i]);
	_IPFprintf(Handler, 0, " %c%c\n",
		   CAGD_IS_RATIONAL_PT(MVs -> PType) ? 'P' : 'E',
		   MaxCoord + '0');
	Indent += 4;

	/* Put out the control mesh. */
	for (i = 0; i < MVAR_CTL_MESH_LENGTH(MVs); i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(MVs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(MVs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(MVs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	MVs = MVs -> Pnext;
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline multi-variates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVWriteToFile, files, write,multi-variates                        M
*****************************************************************************/
int MvarBspMVWriteToFile(const MvarMVStruct *MVs,
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

    i = MvarBspMVWriteToFile2(MVs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write Bspline multi-variates to the given file.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVWriteToFile2, files, write, multi-variates                      M
*****************************************************************************/
int MvarBspMVWriteToFile2(const MvarMVStruct *MVs,
			  int Handler,
			  int Indent,
			  const char *Comment,
			  char **ErrStr)
{
    int i, j, Len, MaxCoord;
    CagdRType *KnotVector;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - bspline MV(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (MVs) {
	MaxCoord = CAGD_NUM_OF_PT_COORD(MVs -> PType);

	if (MVs -> GType != MVAR_BSPLINE_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given multi-variate(s) is (are) not Bspline multi-variate(s)");
	    break;
	}

	_IPFprintf(Handler, Indent, "[MULTIVAR BSPLINE %d  ", MVs -> Dim);
	for (i = 0; i < MVs -> Dim; i++)
	    _IPFprintf(Handler, 0, " %d", MVs -> Lengths[i]);
	_IPFprintf(Handler, 0, "  ");
	for (i = 0; i < MVs -> Dim; i++)
	    _IPFprintf(Handler, 0, " %d", MVs -> Orders[i]);
	_IPFprintf(Handler, 0, " %c%c\n",
		   CAGD_IS_RATIONAL_PT(MVs -> PType) ? 'P' : 'E',
		   MaxCoord + '0');
	Indent += 4;

	/* Put out the knot vectors: */
	for (i = 0; i < MVs -> Dim; i++) {
	    KnotVector = MVs -> KnotVectors[i];
	    Len = MVs -> Lengths[i] + MVs -> Orders[i]
			   + (MVs -> Periodic[i] ? MVs -> Orders[i] - 1 : 0);

	    _IPFprintf(Handler, Indent, MVs -> Periodic[i] ? "[KVP" : "[KV");
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
	for (i = 0; i < MVAR_CTL_MESH_LENGTH(MVs); i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(MVs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(MVs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(MVs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	MVs = MVs -> Pnext;
    }

    return *ErrStr == NULL;
}
