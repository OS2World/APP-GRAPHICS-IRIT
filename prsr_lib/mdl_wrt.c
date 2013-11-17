/******************************************************************************
* Mdl_Wrt.c - Models writing to files.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 96.					      *
******************************************************************************/

#include <string.h>
#include "prsr_loc.h"
#include "mdl_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write model(s) to the given file.	          	     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Models:    To be saved in stream.                                        M
*   FileName:  File name where output should go to.                          M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlWriteModelToFile, files, write 	                                     M
*****************************************************************************/
int MdlWriteModelToFile(const MdlModelStruct *Models,
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

    i = MdlWriteModelToFile2(Models, Handler, Indent, Comment, ErrStr);

    IPCloseStream(Handler, TRUE);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write models(s) to the given stream.		     M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Models:    To be saved in stream.                                        M
*   Handler:   A handler to the open stream.				     M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlWriteModelToFile2, files, write, stream                               M
*****************************************************************************/
int MdlWriteModelToFile2(const MdlModelStruct *Models,
			 int Handler,
			 int Indent,
			 const char *Comment,
			 char **ErrStr)
{
    if (Comment != NULL) {
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# prsr_lib - Model(s) dump.\n");
	_IPFprintf(Handler, Indent, "#\n");
	_IPFprintf(Handler, Indent, "# %s\n", Comment);
	_IPFprintf(Handler, Indent, "#\n");
    }

    *ErrStr = NULL;

    for ( ; Models != NULL; Models = Models -> Pnext) {
	MdlTrimSrfStruct *MdlTrimSrf;
	MdlTrimSegStruct *MdlTrimSeg;
	int NumOfTrimSrfs = CagdListLength(Models -> TrimSrfList),
	    NumOfTrimSegs = CagdListLength(Models -> TrimSegList);

	_IPFprintf(Handler, Indent, "[MODEL %d %d\n",
		   NumOfTrimSrfs, NumOfTrimSegs);

	Indent += 4;

	for (MdlTrimSrf = Models -> TrimSrfList;
	     MdlTrimSrf != NULL;
	     MdlTrimSrf = MdlTrimSrf -> Pnext) {
	    MdlLoopStruct
		*Loops = MdlTrimSrf -> LoopList;
	    int NumOfLoops = CagdListLength(Loops);

	    _IPFprintf(Handler, Indent, "[MDLTSRF %d\n", NumOfLoops);

	    if (!CagdSrfWriteToFile2(MdlTrimSrf -> Srf, Handler,
				     Indent + 4, NULL, ErrStr))
	        return FALSE; /* Error Occured. */

	    for ( ; Loops != NULL; Loops = Loops -> Pnext) {
		MdlTrimSegRefStruct
		    *LoopRef = Loops -> SegRefList;

		_IPFprintf(Handler, Indent + 4, "[MDLLOOP");
		for ( ; LoopRef != NULL; LoopRef = LoopRef -> Pnext) {
		    _IPFprintf(Handler, 0, " %d",
			       MdlGetLoopSegIndex(LoopRef,
						  Models -> TrimSegList));
		}
		_IPFprintf(Handler, 0, "]\n");
	    }
	    _IPFprintf(Handler, Indent, "]\n");
	}

	for (MdlTrimSeg = Models -> TrimSegList;
	     MdlTrimSeg != NULL;
	     MdlTrimSeg = MdlTrimSeg -> Pnext) {
	    IritIntPtrSizeType Index1, Index2;
	    int CurveMask = 
		   (MdlTrimSeg -> UVCrvFirst  != NULL ? IP_MDL_UV1_CRV : 0) +
		   (MdlTrimSeg -> UVCrvSecond != NULL ? IP_MDL_UV2_CRV : 0) +
		   (MdlTrimSeg -> EucCrv      != NULL ? IP_MDL_EUC_CRV : 0);

	    _IPFprintf(Handler, Indent, "[MDLTSEG %d %d %d\n", CurveMask,
		       Index1 = MdlGetSrfIndex(MdlTrimSeg -> SrfFirst,
					       Models -> TrimSrfList),
		       Index2 = MdlGetSrfIndex(MdlTrimSeg -> SrfSecond,
					       Models -> TrimSrfList));
	    if (Index1 == 0 && Index2 == 0) {
		IRIT_WARNING_MSG("Error: Failed to find surface index in MODEL.\n");
	    }

	    if (MdlTrimSeg -> UVCrvFirst != NULL &&
		!CagdCrvWriteToFile2(MdlTrimSeg -> UVCrvFirst, Handler,
				     Indent + 4, NULL, ErrStr))
	        return FALSE;
	    if (MdlTrimSeg -> UVCrvSecond != NULL &&
		!CagdCrvWriteToFile2(MdlTrimSeg -> UVCrvSecond, Handler,
				     Indent + 4, NULL, ErrStr))
	        return FALSE;
	    if (MdlTrimSeg -> EucCrv != NULL &&
		!CagdCrvWriteToFile2(MdlTrimSeg -> EucCrv, Handler,
				     Indent + 4, NULL, ErrStr))
	        return FALSE;
	    _IPFprintf(Handler, Indent, "]\n");
	}

	Indent -= 4;

	_IPFprintf(Handler, Indent, "]\n");
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generic routine to write model(s) to the given stream.	             M
*   If Comment is NULL, no comment is printed, if "" only internal comment.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Models:    To be saved in stream.                                        M
*   f:         File descriptor where output should go to.                    M
*   Indent:    Column in which all printing starts at.                       M
*   Comment:   Optional comment to describe the geometry.                    M
*   ErrStr:    If failed, ErrStr will be set to describe the problem.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlWriteModelToFile3, files, write                                       M
*****************************************************************************/
int MdlWriteModelToFile3(const MdlModelStruct *Models,
			 FILE *f,
			 int Indent,
			 const char *Comment,
			 char **ErrStr)
{
    int Handler = IPOpenStreamFromFile(f, FALSE, FALSE, FALSE, FALSE),
	i = MdlWriteModelToFile2(Models, Handler, Indent,
				 Comment, ErrStr);

    IPCloseStream(Handler, FALSE);

    return i;
}
