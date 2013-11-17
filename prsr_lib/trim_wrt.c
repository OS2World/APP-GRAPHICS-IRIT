/******************************************************************************
* Trim_Wrt.c - Trimmed surfaces writing to files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 94.					      *
******************************************************************************/

#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write a trimmed surface to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfs:  To be saved in stream.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimWriteTrimmedSrfToFile, files, write                                  M
*****************************************************************************/
int TrimWriteTrimmedSrfToFile(const TrimSrfStruct *TrimSrfs,
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

    i = TrimWriteTrimmedSrfToFile2(TrimSrfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write trimmed surface(s) to the given stream.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfs:  To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimWriteTrimmedSrfToFile2, files, write, stream                         M
*****************************************************************************/
int TrimWriteTrimmedSrfToFile2(const TrimSrfStruct *TrimSrfs,
			       int Handler,
			       int Indent,
			       const char *Comment,
			       char **ErrStr)
{
    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent,
		   "# prsr_lib - Bspline Trimmed surface(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    for ( ; TrimSrfs != NULL; TrimSrfs = TrimSrfs -> Pnext) {
	TrimCrvStruct
	    *TCrvs = TrimSrfs -> TrimCrvList;

	_IPFprintf(Handler, Indent, "[TRIMSRF\n");
	if (!CagdSrfWriteToFile2(TrimSrfs -> Srf, Handler, Indent + 4,
				 NULL, ErrStr))
	    return FALSE;

	for ( ; TCrvs != NULL; TCrvs = TCrvs -> Pnext) {
	    TrimCrvSegStruct
		*TSCrvs = TCrvs -> TrimCrvSegList;

	    _IPFprintf(Handler, Indent + 4, "[TRIMCRV\n");

	    for ( ; TSCrvs != NULL; TSCrvs = TSCrvs -> Pnext) {
		_IPFprintf(Handler, Indent + 8, "[TRIMCRVSEG\n");

		if (!CagdCrvWriteToFile2(TSCrvs -> UVCrv, Handler, Indent + 12,
					 NULL, ErrStr))
		    return FALSE;

		_IPFprintf(Handler, Indent + 8, "]\n");
	    }

	    _IPFprintf(Handler, Indent + 4, "]\n");
	}

	_IPFprintf(Handler, Indent, "]\n");
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write trimmed surface(s) to the given stream.	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfs:  To be saved in stream.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimWriteTrimmedSrfToFile3, files, write                                 M
*****************************************************************************/
int TrimWriteTrimmedSrfToFile3(const TrimSrfStruct *TrimSrfs,
			       FILE *f,
			       int Indent,
			       const char *Comment,
			       char **ErrStr)
{
    int Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = TrimWriteTrimmedSrfToFile2(TrimSrfs, Handler, Indent,
				       Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}
