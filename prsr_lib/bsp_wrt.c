/******************************************************************************
* Bsp_Wrt.c - Bspline handling routines - write to file.		      *
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
* Writes Bspline curve(s) list into file. Returns TRUE if succesful, FALSE   M
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
*   BspCrvWriteToFile, files, write                                          M
*****************************************************************************/
int BspCrvWriteToFile(const CagdCrvStruct *Crvs,
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

    i = BspCrvWriteToFile2(Crvs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes Bspline curve(s) list into file. Returns TRUE if succesful, FALSE   M
* otherwise. The file descriptor is not closed.				     M
*   If Comment is NULL, no comment is wrriten, if "" only internal comment   M
* is written.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:       To write to open stream.	                             M
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
*   BspCrvWriteToFile2, files, write                                         M
*****************************************************************************/
int BspCrvWriteToFile2(const CagdCrvStruct *Crvs,
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int i, j, KVLen, MaxCoord;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# cagd_lib - bspline curve(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (Crvs) {
        if (CAGD_IS_BEZIER_CRV(Crvs)) {
	    /* Usually one should not mix Bezier and Bspline curves. */
	    return BzrCrvWriteToFile2(Crvs, Handler, Indent, Comment,
				      ErrStr);
        }
	else if (!CAGD_IS_BSPLINE_CRV(Crvs)) {
	    *ErrStr = IRIT_EXP_STR("Given curve(s) is (are) not bspline surface(s)");
	    break;
	}

	MaxCoord = CAGD_NUM_OF_PT_COORD(Crvs -> PType);

	if (Crvs -> GType != CAGD_CBSPLINE_TYPE) {
	    *ErrStr = IRIT_EXP_STR("Given curve(s) is (are) not Bspline curve(s)");
	    break;
	}
	_IPFprintf(Handler, Indent, "[CURVE BSPLINE %d %d %c%c\n",
		   Crvs -> Length, Crvs -> Order,
		   CAGD_IS_RATIONAL_PT(Crvs -> PType) ? 'P' : 'E',
		   MaxCoord + '0');
	Indent += 4;

	/* Put out the knot vectors: */
	_IPFprintf(Handler, Indent, Crvs -> Periodic ? "[KVP" : "[KV");
	KVLen = Crvs -> Order + Crvs -> Length +
				(Crvs -> Periodic ? Crvs -> Order - 1 : 0);
	for (i = 0; i < KVLen; i++) {
	    if (i && i % MAX_KNOTS_PER_LINE == 0) {
		_IPFprintf(Handler, 0, "\n");
		_IPFprintf(Handler, Indent + 4, "");
	    }
	    _IPFprintf(Handler, 0, " %s", _IPReal2Str(Crvs -> KnotVector[i]));
	}
	_IPFprintf(Handler, 0, "]\n");

	/* Put out the control polygon. */
	for (i = 0; i < Crvs -> Length; i++) {
	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(Crvs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(Crvs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(Crvs -> Points[j][i]));
		if (j < MaxCoord)
		    _IPFprintf(Handler, 0, " ");
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
* Writes Bspline surface(s) list into file. Returns TRUE if succesful, FALSE M
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
*   BspSrfWriteToFile, files, write                                          M
*****************************************************************************/
int BspSrfWriteToFile(const CagdSrfStruct *Srfs,
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

    i = BspSrfWriteToFile2(Srfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Writes Bspline surface(s) list into file. Returns TRUE if succesful, FALSE M
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
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfWriteToFile2, files, write                                         M
*****************************************************************************/
int BspSrfWriteToFile2(const CagdSrfStruct *Srfs,
		       int Handler,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    int i, j, KVLen, MaxCoord;
    CagdRType *KnotVector;

    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# cagd_lib - b-spline Srf(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    while (Srfs) {
        if (CAGD_IS_BEZIER_SRF(Srfs)) {
	    /* Usually one should not mix Bezier and B-spline curves. */
	    return BzrSrfWriteToFile2(Srfs, Handler, Indent, Comment,
				      ErrStr);
        }
	else if (!CAGD_IS_BSPLINE_SRF(Srfs)) {
	    *ErrStr = IRIT_EXP_STR("Given surface(s) is (are) not B-spline surface(s)");
	    break;
	}

	MaxCoord = CAGD_NUM_OF_PT_COORD(Srfs -> PType);

	_IPFprintf(Handler, Indent, "[SURFACE BSPLINE %d %d %d %d %c%c\n",
		   Srfs -> ULength, Srfs -> VLength,
		   Srfs -> UOrder, Srfs -> VOrder,
		   CAGD_IS_RATIONAL_PT(Srfs -> PType) ? 'P' : 'E',
		   MaxCoord + '0');
	Indent += 4;

	/* Put out the knot vectors: */
	for (i = 0; i < 2; i++) {
	    if (i == 0) {
		KnotVector = Srfs -> UKnotVector;
		KVLen = Srfs -> ULength + Srfs -> UOrder +
				(Srfs -> UPeriodic ? Srfs -> UOrder - 1 : 0);
		_IPFprintf(Handler, Indent,
			   Srfs -> UPeriodic ? "[KVP" : "[KV");
	    }
	    else {
		KnotVector = Srfs -> VKnotVector;
		KVLen = Srfs -> VLength + Srfs -> VOrder +
				(Srfs -> VPeriodic ? Srfs -> VOrder - 1 : 0);
		_IPFprintf(Handler, Indent,
			   Srfs -> VPeriodic ? "[KVP" : "[KV");
	    }

	    for (j = 0; j < KVLen; j++) {
		if (j && j % MAX_KNOTS_PER_LINE == 0) {
		    _IPFprintf(Handler, 0, "\n");
		    _IPFprintf(Handler, Indent + 4, "");
		}
		_IPFprintf(Handler, 0, " %s", _IPReal2Str(KnotVector[j]));
	    }
	    _IPFprintf(Handler, 0, "]\n");
	}

	/* Put out the control mesh. */
	for (i = 0; i < Srfs -> VLength * Srfs -> ULength; i++) {
	    if (i && i % Srfs -> ULength == 0)
		_IPFprintf(Handler, 0, "\n");/* Put empty lines between raws.*/

	    _IPFprintf(Handler, Indent, "[");
	    if (CAGD_IS_RATIONAL_PT(Srfs -> PType))
		_IPFprintf(Handler, 0, "%s ",
			   _IPReal2Str(Srfs -> Points[0][i]));
	    for (j = 1; j <= MaxCoord; j++) {
		_IPFprintf(Handler, 0, "%s",
			   _IPReal2Str(Srfs -> Points[j][i]));
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
