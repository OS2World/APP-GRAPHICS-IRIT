/******************************************************************************
* Cagd_dbg.c - Provide a routine to print Surface/Curve objects to stderr.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints curves and surfaces to stderr.	Should be linked to programs for     M
* debugging purposes, so curves and surfaces may be inspected from the       M
* debugger.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       Either a curve or a surface - to be printed to stderr.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDbg, debugging                                                       M
*****************************************************************************/
void CagdDbg(const void *Obj)
{
    char
	*ErrorMsg = NULL;
    const CagdCrvStruct
	*Crv = (const CagdCrvStruct *) Obj;
    const CagdSrfStruct
	*Srf = (const CagdSrfStruct *) Obj;
    CagdGeomType 
	GType = Crv -> GType;

    switch (GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
	case CAGD_CPOWER_TYPE:
	    CagdCrvWriteToFile3(Crv, stderr, 0, IRIT_EXP_STR("CagdDbg"),
				&ErrorMsg);
	    break;
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	case CAGD_SPOWER_TYPE:
	    CagdSrfWriteToFile3(Srf, stderr, 0, IRIT_EXP_STR("CagdDbg"),
				&ErrorMsg);
	    break;
	default:
	    ErrorMsg = "Undefined geometry";
	    break;
    }

    if (ErrorMsg)
	IRIT_WARNING_MSG_PRINTF("CagdDbg Error: %s\n", ErrorMsg);
}

#ifdef DEBUG

IRIT_STATIC_DATA
    int GlblPrgmIO = -1;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Views curves and surfaces in a display device. Should be linked to         M
* programs for debugging purposes, so curves and surfaces may be inspected   M
* from the debugger.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       Either a curve or a surface - to be printed to stderr.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDbgV, debugging                                                      M
*****************************************************************************/
void CagdDbgV(const void *Obj)
{
    const char
        *ErrorMsg = NULL,
	*Program = getenv("IRIT_DISPLAY");
    const CagdCrvStruct
	*Crv = (const CagdCrvStruct *) Obj;
    const CagdSrfStruct
	*Srf = (const CagdSrfStruct *) Obj;
    CagdGeomType 
	GType = Crv -> GType;
    IPObjectStruct *PObj;

#ifdef __WINNT__
    if (Program == NULL)
	Program = "wntgdrvs -s-";
#endif /* __WINNT__ */
#ifdef __UNIX__
    if (Program == NULL)
	Program = "x11drvs -s-";
#endif /* __UNIX__ */

    if (GlblPrgmIO < 0) {
        if ((GlblPrgmIO = IPSocExecAndConnect(Program,
				       getenv("IRIT_BIN_IPC") != NULL)) < 0)
	    return;
    }

    switch (GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
	case CAGD_CPOWER_TYPE:
	    PObj = IPGenCRVObject(CagdCrvCopy(Crv));
    	    break;
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	case CAGD_SPOWER_TYPE:
	    PObj = IPGenSRFObject(CagdSrfCopy(Srf));
	    break;
	default:
	    ErrorMsg = "Undefined geometry";
	    PObj = NULL;
	    break;
    }

    if (ErrorMsg) {
	IRIT_WARNING_MSG_PRINTF("CagdDbg Error: %s\n", ErrorMsg);
    }
    else {
        AttrSetObjectColor(PObj, IG_IRIT_CYAN);
	IPSocWriteOneObject(GlblPrgmIO, PObj);
	IPFreeObject(PObj);
    }
}

#endif /* DEBUG */
