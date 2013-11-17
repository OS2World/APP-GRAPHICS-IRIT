/******************************************************************************
* TrivEdit.c - Editing tools of trivariates.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mat. 97.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Provides the way to modify/get a single control point into/from a	     M
* trivariate.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        Trivar to be modified/query.                                  M
*   CtlPt:     New control point to be substituted into TV. Must carry the   M
*              same PType as TV if to be written to TV.                      M
*   UIndex, VIndex, WIndex: In trivar TV's control mesh to substitute/query  M
*	       CtlPt.							     M
*   Write:     If TRUE CtlPt is copied into TV, if FALSE the point is        M
*              copied from TV to CtlPt.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  If Write is TRUE, the new modified TV, if WRITE is      M
*                    FALSE, NULL.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEditSingleTVPt, trivar editing                                       M
*****************************************************************************/
TrivTVStruct *TrivEditSingleTVPt(TrivTVStruct *TV,
				 CagdCtlPtStruct *CtlPt,
				 int UIndex,
				 int VIndex,
				 int WIndex,
				 CagdBType Write)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
	*NewTV = Write ? TrivTVCopy(TV) : NULL;
    CagdRType
	**Points = Write ? NewTV -> Points : TV -> Points;

    if (UIndex < 0 || UIndex >= ULength ||
	VIndex < 0 || VIndex >= VLength ||
	WIndex < 0 || WIndex >= WLength)
	TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

    if (Write) {
	if (TV -> PType != CtlPt -> PtType)
	    TRIV_FATAL_ERROR(TRIV_ERR_PT_OR_LEN_MISMATCH);

	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][TRIV_MESH_UVW(NewTV, UIndex, VIndex, WIndex)] =
		CtlPt -> Coords[i];
    }
    else {
        CtlPt -> PtType = TV -> PType;

	for (i = IsNotRational; i <= MaxCoord; i++)
	    CtlPt -> Coords[i] =
		Points[i][TRIV_MESH_UVW(TV, UIndex, VIndex, WIndex)];
    }

    return NewTV;
}
