/******************************************************************************
* MvarEdit.c - Editing tools of multi-variates.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Provides the way to modify/get a single control point into/from a	     M
* multi-variate.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        Multi-variate to be modified/query.                           M
*   CtlPt:     New control point to be substituted into MV. Must carry the   M
*              same PType as MV if to be written to MV.                      M
*   Indices:   In multi-variate MV's control mesh to substitute/query CtlPt. M
*   Write:     If TRUE CtlPt is copied into MV, if FALSE the point is        M
*              copied from MV to CtlPt.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: If Write is TRUE, the new modified multi-variate, if     M
*		    WRITE is FALSE, NULL.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarEditSingleMVPt, multi-variate editing                                M
*****************************************************************************/
MvarMVStruct *MvarEditSingleMVPt(MvarMVStruct *MV,
				 CagdCtlPtStruct *CtlPt,
				 int *Indices,
				 CagdBType Write)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, Index,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct
	*NewMV = Write ? MvarMVCopy(MV) : NULL;
    CagdRType
	**Points = Write ? NewMV -> Points : MV -> Points;

    for (i = 0; i < MV -> Dim; i++) {
	if (Indices[i] < 0 || Indices[i] >= MV -> Lengths[i]) {
	    MVAR_FATAL_ERROR(MVAR_ERR_INDEX_NOT_IN_MESH);
	    return NULL;
	}
    }
    Index = MvarGetPointsMeshIndices(MV, Indices);

    if (Write) {
	if (MV -> PType != CtlPt -> PtType)
	    MVAR_FATAL_ERROR(MVAR_ERR_PT_OR_LEN_MISMATCH);

	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][Index] = CtlPt -> Coords[i];
    }
    else {
        CtlPt -> PtType = (CagdPointType) MV -> PType;

	for (i = IsNotRational; i <= MaxCoord; i++)
	    CtlPt -> Coords[i] = Points[i][Index];
    }

    return NewMV;
}
