/******************************************************************************
* Mdl_dbg.c - Provide a routine to print model objects to stderr.             *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jun. 97.					      *
******************************************************************************/

#include "mdl_loc.h"
#include "iritprsr.h"
#include "grap_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints model objects to stderr. Should be linked to programs for debugging M
* purposes, so model objects may be inspected from a debugger.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A model object - to be printed to stderr.  		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbg, debugging                                                        M
*****************************************************************************/
void MdlDbg(void *Obj)
{
    char *ErrorMsg;
    MdlModelStruct
	*Mdl = (MdlModelStruct *) Obj;

    MdlWriteModelToFile3(Mdl, stderr, 0, "MdlDbg", &ErrorMsg);

    if (ErrorMsg)
	fprintf(stderr, "MdlDbg Error: %s\n", ErrorMsg);
}

#ifdef DEBUG

IRIT_STATIC_DATA
    int GlblPrgmIO = -1;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump to stder/display the UV trimming curves of all     M
* surfaces in a given model.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mdl:     To dump to stderr/display all its trimming curves in UV space,  M
*            for each surface.					             M
*   Format:  0 to write it to stderr all data,			             M
*            1 to write it to stderr but only end points of trimming curves. M
*            9 to display the data in a display device.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbgMC, debugging. 		                                     M
*****************************************************************************/
int MdlDbgMC(const MdlModelStruct *Mdl, int Format)
{
    int i = 0;
    CagdPType Trans;
    const MdlTrimSrfStruct *TSrf;

    Trans[1] = Trans[2] = 0.0;
    for (TSrf = Mdl -> TrimSrfList; TSrf != NULL; TSrf = TSrf -> Pnext) {
        Trans[0] = i++ * 10.0;

        if (Format < 9)
	    fprintf(stderr, "********* Trimmed Surface %d: **********\n", i);

	MdlDebugHandleTCrvLoops(TSrf, TSrf -> LoopList, Trans,
				Format == 9, Format == 1);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump to stder/display the UV trimming curves of the     M
* list of trimming curves.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSegs:   To dump to stderr/display all its trimming curves in UV space.  M
*   Format:  0 to write it to stderr all data,			             M
*            1 to write it to stderr but only end points of trimming curves. M
*            9 to display the data in a display device.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbgTC, debugging. 		                                     M
*****************************************************************************/
int MdlDbgTC(const MdlTrimSegStruct *TSegs, int Format)
{
    return MdlDebugHandleTSrfCrvs(TSegs, NULL, NULL, Format == 9, Format == 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump the UV trimming curves of a given surface.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:    To dump its trimming curves in UV space.                        M
*   Format:  0 to write it to stderr all data,			             M
*            1 to write it to stderr but only end points of trimming curves. M
*            9 to display the data in a display device.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbgSC, debugging.		                                     M
*****************************************************************************/
int MdlDbgSC(const MdlTrimSrfStruct *TSrf, int Format)
{
    CagdPType Trans;

    IRIT_PT_RESET(Trans);

    return MdlDebugHandleTCrvLoops(TSrf, TSrf -> LoopList, Trans,
				   Format == 9, Format == 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump the UV trimming curves in the given ref. list.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Refs:    To dump its trimming curves in UV space.                        M
*   Format:  0 to write it to stderr all data,			             M
*            1 to write it to stderr but only end points of trimming curves. M
*            9 to display the data in a display device.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbgRC, debugging.		                                     M
*****************************************************************************/
int MdlDbgRC(const MdlTrimSegRefStruct *Refs, int Format)
{
    CagdPType Trans;

    IRIT_PT_RESET(Trans);

    return MdlDebugHandleTSrfRefCrvs(Refs, NULL, Trans, 1,
				     Format == 9, Format == 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump the UV trimming curves in the given ref. list.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Refs:    To dump its trimming curves in UV space.                        M
*   TSrf:    Trimmed surface these references should list (and its trimming  M
*	     curves).							     M
*   Format:  0 to write it to stderr all data,			             M
*            1 to write it to stderr but only end points of trimming curves. M
*            9 to display the data in a display device.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDbgRC2, debugging.		                                     M
*****************************************************************************/
int MdlDbgRC2(const MdlTrimSegRefStruct *Refs,
	      const MdlTrimSrfStruct *TSrf,
	      int Format)
{
    CagdPType Trans;

    IRIT_PT_RESET(Trans);

    return MdlDebugHandleTSrfRefCrvs(Refs, TSrf, Trans, 1,
				     Format == 9, Format == 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump the UV trimming curve loops.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:     To dump its trimming loops in UV space.  Can be NULL.          M
*   Loops:    To dump as trimming curves in UV space.                        M
*   Trans:    To translate all the curves.				     M
*   Display:  TRUE to display the data, FALSE to write it to stdout.         M
*   TrimEndPts:  TRUE to dump only the trimming curves' end points.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDebugHandleTCrvLoops, debugging.                                      M
*****************************************************************************/
int MdlDebugHandleTCrvLoops(const MdlTrimSrfStruct *TSrf,
			    const MdlLoopStruct *Loops,
			    const CagdPType Trans,
			    int Display,
			    int TrimEndPts)
{
    int l = 0;

    for ( ; Loops != NULL; Loops = Loops -> Pnext) {
        MdlTrimSegRefStruct
	    *LoopRef = Loops -> SegRefList;

	if (!Display)
	    fprintf(stderr, "[OBJECT LOOP%d\n", ++l);

	if (!MdlDebugHandleTSrfRefCrvs(LoopRef, TSrf, Trans, l, Display,
				       TrimEndPts))
	    return FALSE;

	if (!Display)
	    fprintf(stderr, "]\n");
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump a list of trimming curves.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TCrvs:   List of trimming curves to dump.				     M
*   TSrf:    If not NULL - dump only its trimming curves.                    M
*   Trans:   To translate all the curves.  Can be NULL.			     M
*   Display: TRUE to display the data, FALSE to write it to stdout.          M
*   TrimEndPts:  TRUE to dump only the trimming curves' end points.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDebugHandleTSrfCrvs, debugging.	                                     M
*****************************************************************************/
int MdlDebugHandleTSrfCrvs(const MdlTrimSegStruct *TCrvs,
			   const MdlTrimSrfStruct *TSrf,
			   const CagdPType Trans,
			   int Display,
			   int TrimEndPts)
{
    IRIT_STATIC_DATA const IrtPtType
        TransZero = { 0.0, 0.0, 0.0 };
    int c = 0;
    char *ErrStr,
	*Program = getenv("IRIT_DISPLAY");

#ifdef __WINNT__
    if (Program == NULL)
	Program = "wntgdrvs -s-";
#endif /* __WINNT__ */
#ifdef __UNIX__
    if (Program == NULL)
	Program = "x11drvs -s-";
#endif /* __UNIX__ */

    if (Trans == NULL)
        Trans = TransZero;

    if (Display && GlblPrgmIO < 0) {
        if ((GlblPrgmIO = IPSocExecAndConnect(Program,
				       getenv("IRIT_BIN_IPC") != NULL)) < 0)
	    return FALSE;
    }

    for ( ; TCrvs != NULL; TCrvs = TCrvs -> Pnext) {
        CagdCrvStruct *Crv;
	IPObjectStruct *PCrv;

	if (Display) {		  /* Present the domain as a red rectangle. */
	    IrtRType UMin, UMax, VMin, VMax;

	    if (TSrf == NULL) {
	        if (TCrvs -> SrfFirst != NULL) {
		    CagdSrfDomain(TCrvs -> SrfFirst -> Srf,
				  &UMin, &UMax, &VMin, &VMax);
		    Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					       VMin + Trans[1],
					       UMax + Trans[0],
					       VMax + Trans[1], -1.0);
		    PCrv = IPGenCRVObject(Crv);
		    AttrSetObjectColor(PCrv, IG_IRIT_YELLOW);
		    IPSocWriteOneObject(GlblPrgmIO, PCrv);
		    IPFreeObject(PCrv);
		}
	        if (TCrvs -> SrfSecond != NULL) {
		    CagdSrfDomain(TCrvs -> SrfSecond -> Srf,
				  &UMin, &UMax, &VMin, &VMax);
		    Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					       VMin + Trans[1] + 10.0,
					       UMax + Trans[0],
					       VMax + Trans[1] + 10.0, -1.0);
		}
		else
		    Crv = NULL;
	    }
	    else {
	        CagdSrfDomain(TSrf -> Srf, &UMin, &UMax, &VMin, &VMax);
		Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					   VMin + Trans[1],
					   UMax + Trans[0],
					   VMax + Trans[1], -1.0);
	    }

	    if (Crv != NULL) {
	        PCrv = IPGenCRVObject(Crv);
		AttrSetObjectColor(PCrv, IG_IRIT_YELLOW);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	}
	else {
	    fprintf(stderr, "    [OBJECT Crv%d\n", c++);
	}

	if ((TSrf == NULL || TCrvs -> SrfFirst == TSrf) &&
	    TCrvs -> UVCrvFirst != NULL) {
	    Crv = CagdCoerceCrvsTo(TCrvs -> UVCrvFirst,
				   CAGD_PT_E3_TYPE, FALSE);
	    if (Display) {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        PCrv = IPGenCRVObject(Crv);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	    else if (TrimEndPts) {
	        fprintf(stderr, "\t[From [%.10f %.10f] to [%.10f, %.10f]]\n",
			Crv -> Points[1][0],
			Crv -> Points[2][0],
			Crv -> Points[1][Crv -> Length - 1],
			Crv -> Points[2][Crv -> Length - 1]);
		CagdCrvFree(Crv);
	    }
	    else {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
		    return FALSE;
		CagdCrvFree(Crv);
	    }
	}

	if ((TSrf == NULL || TCrvs -> SrfSecond == TSrf) &&
	    TCrvs -> UVCrvSecond != NULL) {
	    Crv = CagdCoerceCrvsTo(TCrvs -> UVCrvSecond,
				   CAGD_PT_E3_TYPE, FALSE);
	    if (Display) {
	        CagdCrvTransform(Crv, Trans, 1.0);
		PCrv = IPGenCRVObject(Crv);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	    else if (TrimEndPts) {
	        fprintf(stderr, "\t[From [%.10f %.10f] to [%.10f, %.10f]]\n",
			Crv -> Points[1][0],
			Crv -> Points[2][0],
			Crv -> Points[1][Crv -> Length - 1],
			Crv -> Points[2][Crv -> Length - 1]);
		CagdCrvFree(Crv);
	    }
	    else {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
		    return FALSE;
		CagdCrvFree(Crv);
	    }
	}

	if (!Display)
	    fprintf(stderr, "    ]\n");
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump a list of refs to trimming curves.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Refs:    List of references to trimming curves to dump.		     M
*   TSrf:    If not NULL - dump only its trimming curves.                    M
*   Trans:   To translate all the curves.  Can be NULL.			     M
*   Loop:    Unique loop index.						     M
*   Display: TRUE to display the data, FALSE to write it to stdout.          M
*   TrimEndPts:  TRUE to dump only the trimming curves' end points.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDebugHandleTSrfRefCrvs, debugging.                                    M
*****************************************************************************/
int MdlDebugHandleTSrfRefCrvs(const MdlTrimSegRefStruct *Refs,
			      const MdlTrimSrfStruct *TSrf,
			      const CagdPType Trans,
			      int Loop,
			      int Display,
			      int TrimEndPts)
{
    IRIT_STATIC_DATA const IrtPtType
        TransZero = { 0.0, 0.0, 0.0 };
    int c = 0;
    char *ErrStr,
	*Program = getenv("IRIT_DISPLAY");

#ifdef __WINNT__
    if (Program == NULL)
	Program = "wntgdrvs -s-";
#endif /* __WINNT__ */
#ifdef __UNIX__
    if (Program == NULL)
	Program = "x11drvs -s-";
#endif /* __UNIX__ */

    if (Trans == NULL)
        Trans = TransZero;

    if (Display && GlblPrgmIO < 0) {
        if ((GlblPrgmIO = IPSocExecAndConnect(Program,
				       getenv("IRIT_BIN_IPC") != NULL)) < 0)
	    return FALSE;
    }

    for ( ; Refs != NULL; Refs = Refs -> Pnext) {
        CagdCrvStruct *Crv;
	IPObjectStruct *PCrv;

	if (Display) {		  /* Present the domain as a red rectangle. */
	    IrtRType UMin, UMax, VMin, VMax;

	    if (TSrf == NULL) {
	        if (Refs -> TrimSeg -> SrfFirst != NULL) {
		    CagdSrfDomain(Refs -> TrimSeg -> SrfFirst -> Srf,
				  &UMin, &UMax, &VMin, &VMax);
		    Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					       VMin + Trans[1],
					       UMax + Trans[0],
					       VMax + Trans[1], -1.0);
		    PCrv = IPGenCRVObject(Crv);
		    AttrSetObjectColor(PCrv, IG_IRIT_YELLOW);
		    IPSocWriteOneObject(GlblPrgmIO, PCrv);
		    IPFreeObject(PCrv);
		}
	        if (Refs -> TrimSeg -> SrfSecond != NULL) {
		    CagdSrfDomain(Refs -> TrimSeg -> SrfSecond -> Srf,
				  &UMin, &UMax, &VMin, &VMax);
		    Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					       VMin + Trans[1] + 10.0,
					       UMax + Trans[0],
					       VMax + Trans[1] + 10.0, -1.0);
		}
		else
		    Crv = NULL;
	    }
	    else {
	        CagdSrfDomain(TSrf -> Srf, &UMin, &UMax, &VMin, &VMax);
		Crv = CagdPrimRectangleCrv(UMin + Trans[0], 
					   VMin + Trans[1],
					   UMax + Trans[0],
					   VMax + Trans[1], -1.0);
	    }

	    if (Crv != NULL) {
	        PCrv = IPGenCRVObject(Crv);
		AttrSetObjectColor(PCrv, IG_IRIT_YELLOW);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	}
	else {
	    int In = AttrGetIntAttrib(Refs -> Attr, "_Inside");

	    fprintf(stderr, "    [OBJECT [COLOR %d] Loop%dTCrv%d\n",
		    In == TRUE ? IG_IRIT_GREEN :
		                 (In == FALSE ? IG_IRIT_RED : IG_IRIT_CYAN),
		    Loop, c++);
	}

	if ((TSrf == NULL || Refs -> TrimSeg -> SrfFirst == TSrf) &&
	    Refs -> TrimSeg -> UVCrvFirst != NULL) {
	    Crv = CagdCoerceCrvsTo(Refs -> TrimSeg -> UVCrvFirst,
				   CAGD_PT_E3_TYPE, FALSE);
	    if (Display) {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        PCrv = IPGenCRVObject(Crv);
		if (AttrGetIntAttrib(Refs -> Attr, "_Inside") == TRUE)
		    AttrSetObjectColor(PCrv, IG_IRIT_GREEN);
		else if (AttrGetIntAttrib(Refs -> Attr, "_Inside") == FALSE)
		    AttrSetObjectColor(PCrv, IG_IRIT_RED);
		else
		    AttrSetObjectColor(PCrv, IG_IRIT_CYAN);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	    else if (TrimEndPts) {
	        fprintf(stderr, "\t[From [%.10f %.10f] to [%.10f, %.10f]]\n",
			Crv -> Points[1][0],
			Crv -> Points[2][0],
			Crv -> Points[1][Crv -> Length - 1],
			Crv -> Points[2][Crv -> Length - 1]);
		CagdCrvFree(Crv);
	    }
	    else {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
		    return FALSE;
		CagdCrvFree(Crv);
	    }
	}

	if ((TSrf == NULL || Refs -> TrimSeg -> SrfSecond == TSrf) &&
	    Refs -> TrimSeg -> UVCrvSecond != NULL) {
	    Crv = CagdCoerceCrvsTo(Refs -> TrimSeg -> UVCrvSecond,
				   CAGD_PT_E3_TYPE, FALSE);
	    if (Display) {
	        CagdCrvTransform(Crv, Trans, 1.0);
		PCrv = IPGenCRVObject(Crv);
		if (AttrGetIntAttrib(Refs -> Attr, "_Inside") == TRUE)
		    AttrSetObjectColor(PCrv, IG_IRIT_GREEN);
		else if (AttrGetIntAttrib(Refs -> Attr, "_Inside") == FALSE)
		    AttrSetObjectColor(PCrv, IG_IRIT_RED);
		else
		    AttrSetObjectColor(PCrv, IG_IRIT_CYAN);
		IPSocWriteOneObject(GlblPrgmIO, PCrv);
		IPFreeObject(PCrv);
	    }
	    else if (TrimEndPts) {
	        fprintf(stderr, "\t[From [%.10f %.10f] to [%.10f, %.10f]]\n",
			Crv -> Points[1][0],
			Crv -> Points[2][0],
			Crv -> Points[1][Crv -> Length - 1],
			Crv -> Points[2][Crv -> Length - 1]);
		CagdCrvFree(Crv);
	    }
	    else {
	        CagdCrvTransform(Crv, Trans, 1.0);
	        if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
		    return FALSE;
		CagdCrvFree(Crv);
	    }
	}

	if (!Display)
	    fprintf(stderr, "    ]\n");
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to dump a list of UV trimming curves model.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSegs:     To dump all its trimming curves in the list.		     M
*   TSrf:      If not NULL - dump only its trimming curves.                  M
*   Trans:     To translate all the curves.  Can be NULL.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDebugWriteTrimSegs, debugging.                                        M
*****************************************************************************/
int MdlDebugWriteTrimSegs(const MdlTrimSegStruct *TSegs,
			  const MdlTrimSrfStruct *TSrf,
			  const CagdPType Trans)
{
    IRIT_STATIC_DATA const IrtPtType
	TransZero = { 0.0, 0.0, 0.0 };
    const MdlTrimSegStruct *TSeg;

    if (Trans == NULL)
	Trans = TransZero;

    for (TSeg = TSegs; TSeg != NULL; TSeg = TSeg -> Pnext) {
	CagdCrvStruct *Crv;
	char *ErrStr;

	if ((TSrf == NULL || TSeg -> SrfFirst == TSrf) &&
	    TSeg -> UVCrvFirst != NULL) {
	    Crv = CagdCoerceCrvsTo(TSeg -> UVCrvFirst,
				   CAGD_PT_E3_TYPE, FALSE);
	    CagdCrvTransform(Crv, Trans, 1.0);
	    if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
	        return FALSE;
	    CagdCrvFree(Crv);
	}

	if ((TSrf == NULL || TSeg -> SrfSecond == TSrf) &&
	    TSeg -> UVCrvSecond != NULL) {
	    Crv = CagdCoerceCrvsTo(TSeg -> UVCrvSecond,
				   CAGD_PT_E3_TYPE, FALSE);
	    CagdCrvTransform(Crv, Trans, 1.0);
	    if (!CagdCrvWriteToFile3(Crv, stderr, 8, NULL, &ErrStr))
	        return FALSE;
	    CagdCrvFree(Crv);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Debug routine to verify consistency of a model.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:       To verify.	                                             M
*   TestLoops:   TRUE to also verify that each look is indeed a loop.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if successful, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDebugVerify, debugging.		                                     M
*****************************************************************************/
int MdlDebugVerify(const MdlModelStruct *Model, int TestLoops)
{
    MdlTrimSrfStruct *MdlTrimSrf;
    MdlTrimSegStruct *MdlTrimSeg;

    for (MdlTrimSrf = Model -> TrimSrfList;
	 MdlTrimSrf != NULL;
	 MdlTrimSrf = MdlTrimSrf -> Pnext) {
        MdlLoopStruct
	    *Loops = MdlTrimSrf -> LoopList;

	for ( ; Loops != NULL; Loops = Loops -> Pnext) {
	    MdlTrimSegRefStruct *TRef;

	    for (TRef = Loops -> SegRefList;
		 TRef != NULL;
		 TRef = TRef -> Pnext) {
	        MdlTrimSegStruct
		    *TSeg = TRef -> TrimSeg;

		if (TSeg -> SrfFirst != MdlTrimSrf &&
		    TSeg -> SrfSecond != MdlTrimSrf) {
		    fprintf(stderr, "Verify failed: Back references do not point on tsrf\n");
		    return FALSE;
		}
	    }

	    if (TestLoops) {
	        CagdCrvStruct *UVCrv,
		    *UVCrvs = NULL;

		for (TRef = Loops -> SegRefList;
		     TRef != NULL;
		     TRef = TRef -> Pnext) {
		    MdlTrimSegStruct
		        *TSeg = TRef -> TrimSeg;

		    if (TSeg -> SrfFirst == MdlTrimSrf)
		        UVCrv = TSeg -> UVCrvFirst;
		    else
		        UVCrv = TSeg -> UVCrvSecond;

		    UVCrv = CagdCrvCopy(UVCrv);
		    IRIT_LIST_PUSH(UVCrv, UVCrvs);
		}

		if ((UVCrv = TrimChainTrimmingCurves2Loops2(UVCrvs,
					       MDL_BOOL_NEAR_UV_EPS)) != NULL) {
		    CagdCrvFree(UVCrv);
		}
		else {
		    fprintf(stderr, "Verify failed: A loop that is not closed has been detected\n");
		    return FALSE;
		}
	    }
	}
    }

    for (MdlTrimSeg = Model -> TrimSegList;
	 MdlTrimSeg != NULL;
	 MdlTrimSeg = MdlTrimSeg -> Pnext) {
        if (MdlTrimSeg -> SrfFirst == NULL || MdlTrimSeg -> UVCrvFirst == NULL) {
	    fprintf(stderr, "Verify failed: trimming segments with no first TSrfs\n");
	    return FALSE;
	}
	if ((MdlTrimSeg -> SrfSecond == NULL && MdlTrimSeg -> UVCrvSecond != NULL) ||
	    (MdlTrimSeg -> SrfSecond != NULL && MdlTrimSeg -> UVCrvSecond == NULL)) {
	    fprintf(stderr, "Verify failed: trimming segments with invalid second TSrf\n");
	    return FALSE;
	}
    }

    return TRUE;
}

#endif /* DEBUG */

