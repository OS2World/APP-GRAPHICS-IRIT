/******************************************************************************
* Cagd_Wrt.c - Generic Curve/Surface writing to files.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#ifdef USE_VARARGS
#    include <varargs.h>
#else
#    include <stdarg.h>
#endif /* USE_VARARGS */

#include <string.h>
#include "prsr_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write curve(s) to the given file.			     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:      To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvWriteToFile, files, write                                         M
*****************************************************************************/
int CagdCrvWriteToFile(const CagdCrvStruct *Crvs,
		       const char *FileName,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    switch (Crvs -> GType) {
        case CAGD_CBEZIER_TYPE:
        case CAGD_CPOWER_TYPE:
	    return BzrCrvWriteToFile(Crvs, FileName, Indent,
				     Comment, ErrStr);
        case CAGD_CBSPLINE_TYPE:
	    return BspCrvWriteToFile(Crvs, FileName, Indent,
				     Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write curve(s) to the given stream.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:      To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvWriteToFile2, files, write                                        M
*****************************************************************************/
int CagdCrvWriteToFile2(const CagdCrvStruct *Crvs,
			int Handler,
			int Indent,
			const char *Comment,
			char **ErrStr)
{
    switch (Crvs -> GType) {
        case CAGD_CBEZIER_TYPE:
        case CAGD_CPOWER_TYPE:
	    return BzrCrvWriteToFile2(Crvs, Handler, Indent,
				      Comment, ErrStr);
        case CAGD_CBSPLINE_TYPE:
	    return BspCrvWriteToFile2(Crvs, Handler, Indent,
				      Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write curve(s) to the given file.			     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:      To be saved in file f.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvWriteToFile3, files, write                                        M
*****************************************************************************/
int CagdCrvWriteToFile3(const CagdCrvStruct *Crvs,
			FILE *f,
			int Indent,
			const char *Comment,
			char **ErrStr)
{
    int Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = CagdCrvWriteToFile2(Crvs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write surface(s) to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:      To be saved in file f.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfWriteToFile, files, write                                         M
*****************************************************************************/
int CagdSrfWriteToFile(const CagdSrfStruct *Srfs,
		       const char *FileName,
		       int Indent,
		       const char *Comment,
		       char **ErrStr)
{
    switch (Srfs -> GType) {
        case CAGD_SBEZIER_TYPE:
        case CAGD_SPOWER_TYPE:
	    return BzrSrfWriteToFile(Srfs, FileName, Indent,
				     Comment, ErrStr);
        case CAGD_SBSPLINE_TYPE:
	    return BspSrfWriteToFile(Srfs, FileName, Indent,
				     Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write surface(s) to the given stream.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:      To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfWriteToFile2, files, write, stream                                M
*****************************************************************************/
int CagdSrfWriteToFile2(const CagdSrfStruct *Srfs,
			int Handler,
			int Indent,
			const char *Comment,
			char **ErrStr)
{
    switch (Srfs -> GType) {
        case CAGD_SBEZIER_TYPE:
        case CAGD_SPOWER_TYPE:
	    return BzrSrfWriteToFile2(Srfs, Handler, Indent,
				      Comment, ErrStr);
        case CAGD_SBSPLINE_TYPE:
	    return BspSrfWriteToFile2(Srfs, Handler, Indent,
				      Comment, ErrStr);
        default:
	    *ErrStr = IRIT_EXP_STR("BSPLINE, BEZIER or POWER Token expected");
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write surface(s) to the given file.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:      To be saved in file f.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfWriteToFile3, files, write                                        M
*****************************************************************************/
int CagdSrfWriteToFile3(const CagdSrfStruct *Srfs,
			FILE *f,
			int Indent,
			const char *Comment,
			char **ErrStr)
{
    int Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = CagdSrfWriteToFile2(Srfs, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}
