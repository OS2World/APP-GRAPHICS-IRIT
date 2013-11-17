/******************************************************************************
* Mdl_Bbox.c - Handle bounding boxes freeform models.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jan. 97.					      *
******************************************************************************/

#include "mdl_loc.h"
#include "geom_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a freeform model.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mdl:      A model to compute a bounding box for.                         M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlModelListBBox, CagdCrvListBBox, CagdSrfBBox, CagdTightBBox            M
*   MdlModelTSrfTCrvsBBox						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelBBox, bbox, bounding box                                         M
*****************************************************************************/
void MdlModelBBox(const MdlModelStruct *Mdl, CagdBBoxStruct *BBox)
{
    MdlTrimSrfStruct *TSrfs;
    CagdBBoxStruct TmpBBox;

    CAGD_RESET_BBOX(BBox);

    if (Mdl == NULL)
	return;

    for (TSrfs = Mdl -> TrimSrfList; TSrfs != NULL; TSrfs = TSrfs -> Pnext) {
	CagdSrfBBox(TSrfs -> Srf, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for freeform models.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mdls:     List of models to compute a bounding box for.                  M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlModelBBox, CagdCrvListBBox, CagdSrfBBox, CagdTightBBox                M
*   MdlModelTSrfTCrvsBBox						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelListBBox, bbox, bounding box                                     M
*****************************************************************************/
void MdlModelListBBox(const MdlModelStruct *Mdls, CagdBBoxStruct *BBox)
{
    CagdBBoxStruct TmpBBox;

    CAGD_RESET_BBOX(BBox);

    if (Mdls == NULL)
	return;

    for ( ; Mdls != NULL; Mdls = Mdls -> Pnext) {
	MdlModelBBox(Mdls, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for the trimming curves of one trimmed srf.        M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:     A trimmed surfaces to compute a bbox for its trimming curves.  M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlModelListBBox, CagdCrvListBBox, CagdSrfBBox, CagdTightBBox            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelTSrfTCrvsBBox, bbox, bounding box                                M
*****************************************************************************/
void MdlModelTSrfTCrvsBBox(const MdlTrimSrfStruct *TSrf, CagdBBoxStruct *BBox)
{
    CagdBBoxStruct TmpBBox;
    MdlLoopStruct *Loop;

    CAGD_RESET_BBOX(BBox);

    if (TSrf == NULL)
	return;

    for (Loop = TSrf -> LoopList; Loop != NULL; Loop = Loop -> Pnext) {
        MdlTrimSegRefStruct *SegRef;

        for (SegRef = Loop -> SegRefList;
	     SegRef != NULL;
	     SegRef = SegRef -> Pnext) {
	    MdlTrimSegStruct
	        *TSeg = SegRef -> TrimSeg;

	    if (TSeg -> SrfFirst == TSrf) {
	        CagdCrvBBox(TSeg -> UVCrvFirst, &TmpBBox);
	    }
	    else {
	        assert(TSeg -> SrfSecond == TSrf);
	        CagdCrvBBox(TSeg -> UVCrvSecond, &TmpBBox);
	    }
	    CagdMergeBBox(BBox, &TmpBBox);
	}
    }
}
