/*****************************************************************************
* MdlCnvrt.c - model conversion routines.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Gershon Elber	       		      	    January 1998     *
*****************************************************************************/

#include "mdl_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converting the model into the list of trimming surfaces.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:  Model to convert to a list of trimmed surfaces.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  List of trimming surfaces.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimCrvSegNew, TrimCrvNew, TrimsrfNew, MdlExtractUVCrv,		     M
*   MdlCnvrtSrf2Mdl, MdlCnvrtTrimmedSrf2Mdl				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlCnvrtMdl2TrimmedSrfs                                                  M
*****************************************************************************/
TrimSrfStruct *MdlCnvrtMdl2TrimmedSrfs(const MdlModelStruct *Model)
{
    int IsFirstSeg, IsFirstCrv, Length,
        IsFirstSrf = TRUE;
    TrimSrfStruct
	*CurrSrf = NULL,
	*TrimSrfs = NULL;
    CagdSrfStruct *Srf;
    CagdCrvStruct *SegCrv;
    TrimCrvStruct
	*FirstCrv = NULL,
	*CurrCrv = NULL;
    TrimCrvSegStruct
	*FirstSeg = NULL,
	*CurrSeg = NULL;
    MdlLoopStruct *MdlLoop;
    MdlTrimSegRefStruct *MdlRef;
    MdlTrimSegStruct *MdlSeg;
    MdlTrimSrfStruct    
	*MdlSrf = Model -> TrimSrfList;

    while (MdlSrf) {
	Srf = MdlSrf -> Srf;
	MdlLoop = MdlSrf -> LoopList;
	IsFirstCrv = TRUE;

	if (MdlLoop != NULL) {
	    while (MdlLoop) { 
	        MdlRef = MdlLoop -> SegRefList;
		IsFirstSeg = TRUE;

		while (MdlRef) {
		    MdlSeg = MdlRef -> TrimSeg;
		    SegCrv = CagdCrvCopy(MdlExtractUVCrv(MdlSrf, MdlSeg));
		    Length = SegCrv -> Length;

		    if (IRIT_APX_EQ(SegCrv -> Points[1][0],
				    SegCrv -> Points[1][Length - 1]) &&
			IRIT_APX_EQ(SegCrv -> Points[2][0],
				    SegCrv -> Points[2][Length - 1])) {
		        SegCrv -> Points[1][Length - 1] =
			                               SegCrv -> Points[1][0];
			SegCrv -> Points[2][Length - 1] =
			                               SegCrv -> Points[2][0];
		    }

		    if (IsFirstSeg) {
		        IsFirstSeg = FALSE;
			FirstSeg = CurrSeg = TrimCrvSegNew(SegCrv, NULL);
		    }
		    else { 
		        CurrSeg -> Pnext = TrimCrvSegNew(SegCrv, NULL);
 
			CurrSeg = CurrSeg -> Pnext;
		    }
		    MdlRef = MdlRef -> Pnext;
		}

		if (IsFirstCrv) {
		    IsFirstCrv = FALSE;
		    FirstCrv = CurrCrv = TrimCrvNew(FirstSeg);
		}
		else {
		    CurrCrv -> Pnext = TrimCrvNew(FirstSeg);
		    CurrCrv = CurrCrv -> Pnext;
		}
		MdlLoop = MdlLoop -> Pnext;
	    }

	    if (IsFirstSrf) {
	        IsFirstSrf = FALSE;
		TrimSrfs = CurrSrf = TrimSrfNew(CagdSrfCopy(Srf),
						FirstCrv, TRUE);
	    }
	    else {
	        CurrSrf -> Pnext = TrimSrfNew(CagdSrfCopy(Srf), 
					      FirstCrv, TRUE); 
		CurrSrf = CurrSrf -> Pnext;
	    } 
	}
#	ifdef DEBUG
	else {
	    fprintf(stderr, "A trimmed surface with no trimming curves found and ignored in model.\n");
	}
#	endif /* DEBUG */

	MdlSrf = MdlSrf -> Pnext;

    }			    

    return TrimSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converting the given surface into a model.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:  A surface to convert to a model.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  A model.			                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlCnvrtMdl2TrimmedSrfs, MdlCnvrtTrimmedSrf2Mdl			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlCnvrtSrf2Mdl                                                          M
*****************************************************************************/
MdlModelStruct *MdlCnvrtSrf2Mdl(const CagdSrfStruct *Srf)
{
    return MdlModelNew(CagdSrfCopy(Srf), NULL, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converting the given trimmed surface into a model.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:  A trimmed surface to convert to a model.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  A model.			                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlCnvrtMdl2TrimmedSrfs, MdlCnvrtSrf2Mdl				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlCnvrtTrimmedSrf2Mdl                                                   M
*****************************************************************************/
MdlModelStruct *MdlCnvrtTrimmedSrf2Mdl(const TrimSrfStruct *TSrf)
{
    return MdlModelNew(CagdSrfCopy(TSrf -> Srf),
		       TrimGetTrimmingCurves(TSrf, TRUE, FALSE),
		       TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracting the UV curve from MdlTrimSegStruct depending on the position  M
* of the current model surface.			                             M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlSrf: Model's trimming surface.                                        M
*   MdlSeg: Model's trimming curve segment.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Extracted UV curve.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlExtractUVCrv                                                          M
*****************************************************************************/
CagdCrvStruct *MdlExtractUVCrv(const MdlTrimSrfStruct *MdlSrf, 
			       const MdlTrimSegStruct *MdlSeg)
{
    if (MdlSeg -> SrfFirst -> Srf == MdlSrf -> Srf)
        return MdlSeg -> UVCrvFirst;
    else {
        if (MdlSeg -> SrfSecond -> Srf == MdlSrf -> Srf)
	    return MdlSeg -> UVCrvSecond;
        else 
	    return NULL;
    }
} 
