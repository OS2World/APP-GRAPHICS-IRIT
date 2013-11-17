/******************************************************************************
* Bzr_Wrt.c - Bezier handling routines - write to file.			      *
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
* Writes Bezier curve(s) list into file. Returns TRUE if succesful, FALSE    M
* otherwise.								     M
*   If Comment is NULL, no comment is wrriten, if "" only internal comment   M
* is written.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:       To write to file FileName.                                   M
*   FileName:   Name of file to open so we can write Crvs in.                M
*   Indent:     Primary indentation. All information will be written         M
*               from the column specified by Indent.                         M
*   Comment:    Optional, to describe the geometry.                          M
*   ErrStr:     If an error occurs, to describe the error.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvWriteToFile, files, write                                          M
*****************************************************************************/
int BzrCrvWriteToFile(const CagdCrvStruct *Crvs,
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

    i = BzrCrvWriteToFile2(Crvs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes Bezier curve(s) list into file. Returns TRUE if succesful, FALSE    M
* otherwise. The file descriptor is not closed.				     M
*   If Comment is NULL, no comment is wrriten, if "" only internal comment   M
* is written.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:       To write to open stream.                                     M
*   Handler:    A handler to the open stream.				     M
*   Indent:     Primary indentation. All information will be written         M
*               from the column specified by Indent.                         M
*   Comment:    Optional, to describe the geometry.                          M
*   ErrStr:     If an error occurs, to describe the error.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvWriteToFile2, files, write                                         M
*****************************************************************************/
int BzrCrvWriteToFile2(const CagdCrvStruct *Crvs,
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int i, j, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# cagd_lib - bezier/power curve(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (Crvs) {
        if (CAGD_IS_BSPLINE_CRV(Crvs)) {
	    /* Usually one should not mix Bezier and Bspline curves. */
	    return BspCrvWriteToFile2(Crvs, Handler, Indent, Comment,
				      ErrStr);
        }

	MaxCoord = CAGD_NUM_OF_PT_COORD(Crvs -> PType);

	if (Crvs -> GType == CAGD_CBEZIER_TYPE) {
	    _IPFprintf(Handler, Indent, "[CURVE BEZIER %d %c%c\n",
		       Crvs -> Length,
		       CAGD_IS_RATIONAL_PT(Crvs -> PType) ? 'P' : 'E',
		       MaxCoord + '0');
	}
	else if (Crvs -> GType == CAGD_CPOWER_TYPE) {
	    _IPFprintf(Handler, Indent, "[CURVE POWER %d %c%c\n",
		       Crvs -> Length,
		       CAGD_IS_RATIONAL_PT(Crvs -> PType) ? 'P' : 'E',
		       MaxCoord + '0');
	}
	else {
	    *ErrStr = IRIT_EXP_STR("Given curve(s) is (are) not BEZIER or POWER curve(s)");
	    break;
	}

	Indent += 4;

	for (i = 0; i < Crvs -> Length; i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(Crvs -> PType))
		_IPFprintf(Handler, 0, "%s ", _IPReal2Str(Crvs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s", _IPReal2Str(Crvs -> Points[j][i]));
		if (j < MaxCoord) _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	Crvs = Crvs -> Pnext;
    }

    return *ErrStr == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes Bezier surface(s) list into file. Returns TRUE if succesful, FALSE  M
* otherwise.								     M
*   If Comment is NULL, no comment is wrriten, if "" only internal comment   M
* is written.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:       To write to file FileName.                                   M
*   FileName:   Name of file to open so we can write Srfs in.                M
*   Indent:     Primary indentation. All information will be written         M
*               from the column specified by Indent.                         M
*   Comment:    Optional, to describe the geometry.                          M
*   ErrStr:     If an error occurs, to describe the error.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfWriteToFile, files, write                                          M
*****************************************************************************/
int BzrSrfWriteToFile(const CagdSrfStruct *Srfs,
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

    i = BzrSrfWriteToFile2(Srfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes Bezier surface(s) list into file. Returns TRUE if succesful, FALSE  M
* otherwise. The file descriptor is not closed.				     M
*   If Comment is NULL, no comment is wrriten, if "" only internal comment   M
* is written.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:       To write to open stream.                                     M
*   Handler:    A handler to the open stream.				     M
*   Indent:     Primary indentation. All information will be written         M
*               from the column specified by Indent.                         M
*   Comment:    Optional, to describe the geometry.                          M
*   ErrStr:     If an error occurs, to describe the error.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfWriteToFile2, files, write                                         M
*****************************************************************************/
int BzrSrfWriteToFile2(const CagdSrfStruct *Srfs, 
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int i, j, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# cagd_lib - bezier/power srf(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (Srfs) {
        if (CAGD_IS_BSPLINE_SRF(Srfs)) {
	    /* Usually one should not mix Bezier and B-spline curves. */
	    return BspSrfWriteToFile2(Srfs, Handler, Indent, Comment,
				      ErrStr);
        }

	MaxCoord = CAGD_NUM_OF_PT_COORD(Srfs -> PType);

	if (Srfs -> GType == CAGD_SBEZIER_TYPE) {
	    _IPFprintf(Handler, Indent, "[SURFACE BEZIER %d %d %c%c\n",
		       Srfs -> ULength, Srfs -> VLength,
		       CAGD_IS_RATIONAL_PT(Srfs -> PType) ? 'P' : 'E',
		       MaxCoord + '0');
	}
	else if (Srfs -> GType == CAGD_SPOWER_TYPE) {
	    _IPFprintf(Handler, Indent, "[SURFACE POWER %d %d %c%c\n",
		       Srfs -> ULength, Srfs -> VLength,
		       CAGD_IS_RATIONAL_PT(Srfs -> PType) ? 'P' : 'E',
		       MaxCoord + '0');
	}
	else {
	    *ErrStr = IRIT_EXP_STR("Given surface(s) is (are) not BEZIER/POWER surface(s)");
	    break;
	}

	Indent += 4;

	for (i = 0; i < Srfs -> VLength * Srfs -> ULength; i++) {
	    if (i && i % Srfs -> ULength == 0)
		_IPFprintf(Handler, 0, "\n");/* Put empty lines between raws.*/

	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(Srfs -> PType))
		_IPFprintf(Handler, 0, "%s ", _IPReal2Str(Srfs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s", _IPReal2Str(Srfs -> Points[j][i]));
		if (j < MaxCoord) _IPFprintf(Handler, 0, " ");
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	Indent -= 4;
	_IPFprintf(Handler, Indent, "]\n");

	Srfs = Srfs -> Pnext;
    }

    return *ErrStr == NULL;
}
