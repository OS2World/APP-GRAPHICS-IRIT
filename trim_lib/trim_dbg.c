/******************************************************************************
* Trim_dbg.c - Provide a routine to print Trimmed surface objects to stderr.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 94.					      *
******************************************************************************/

#include "trim_loc.h"
#include "iritprsr.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints trimmed surfaces to stderr. Should be linked to programs for        M
*  debugging purposes, so trimmed surfaces may be inspected from a debugger. M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A trimmed surface - to be printed to stderr.  		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimDbg, debugging                                                       M
*****************************************************************************/
void TrimDbg(const void *Obj)
{
    char *ErrorMsg;
    const TrimSrfStruct
	*TrimSrf = (const TrimSrfStruct *) Obj;

    fprintf(stderr, IRIT_EXP_STR("**************** TrimDbg: **************** \n"));

    TrimWriteTrimmedSrfToFile3(TrimSrf, stderr, 0, IRIT_EXP_STR("TrimDbg"),
			       &ErrorMsg);

    if (ErrorMsg)
	fprintf(stderr, IRIT_EXP_STR("TrimDbg Error: %s\n"), ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prints the trimming curves of a given trimmed surface..                  M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimCrv:   Trimming curves to print.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimDbg                                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimDbgPrintTrimCurves, debugging                                        M
*****************************************************************************/
void TrimDbgPrintTrimCurves(const TrimCrvStruct *TrimCrv)
{
    fprintf(stderr, IRIT_EXP_STR("**************** TrimDbgPrintTrimCurves: **************** \n"));

    for ( ; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
	TrimCrvSegStruct
	    *TrimCrvSeg = TrimCrv -> TrimCrvSegList;

	for ( ; TrimCrvSeg != NULL; TrimCrvSeg = TrimCrvSeg -> Pnext) {
	    CagdCrvStruct
		*UVCrv = TrimCrvSeg -> UVCrv;

	    CagdDbg(UVCrv);
	}
    }
}
