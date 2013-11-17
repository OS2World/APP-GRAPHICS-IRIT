/******************************************************************************
* CagdEdit.c - Editing tools of surfaces and Curves.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Provides the way to modify/get a single control point into/from the curve. M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to be modified/query.                                  M
*   CtlPt:      New control point to be substituted into Crv. Must carry the M
*               same PType as Crv if to be written to Crv.                   M
*   Index:      In curve CRV's control polygon to substitute/query CtlPt.    M
*   Write:      If TRUE CtlPt is copied into Crv, if FALSE the point is      M
*               copied from Crv to CtlPt.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: If Write is TRUE, the new modified curve, if WRITE is   M
*                    FALSE, NULL.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEditSingleCrvPt, curve editing                                       M
*****************************************************************************/
CagdCrvStruct *CagdEditSingleCrvPt(CagdCrvStruct *Crv,
				   CagdCtlPtStruct *CtlPt,
				   int Index,
				   CagdBType Write)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Length = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
	*NewCrv = Write ? CagdCrvCopy(Crv) : NULL;
    CagdRType
	**Points = Write ? NewCrv -> Points : Crv -> Points;

    if (Index < 0 || Index >= Length)
	CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

    if (Write) {
	if (Crv -> PType != CtlPt -> PtType)
	    CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);

	CAGD_DEL_GEOM_TYPE(NewCrv);

	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][Index] = CtlPt -> Coords[i];
    }
    else {
        CtlPt -> PtType = Crv -> PType;

	for (i = IsNotRational; i <= MaxCoord; i++)
	    CtlPt -> Coords[i] = Points[i][Index];
    }

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Inserts a new point Pt at Index into curve Crv.		             M
* Returned curve's length is larger by one than the length of Crv.           M
*   Knot vector is updated (if Bspline curve) to a uniform open.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     Input curve to insert a new point into.                         M
*   Index:   Index of control point to insert into Crv. Zero inserts at      M
*	     first location in Crv.			                     M
*   Pt:      New point to insert that will be coerced to Crv point type.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A new curve of length larger by one than Crv.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDeletePoint                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvInsertPoint		                                             M
*****************************************************************************/
CagdCrvStruct *CagdCrvInsertPoint(const CagdCrvStruct *Crv,
				  int Index,
				  const CagdPType Pt)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
	*NewCrv = BspCrvNew(Crv -> Length + 1, Crv -> Order, Crv -> PType);

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    /* Copy old points. */
    for (i = !IsRational; i <= MaxAxis; i++) {
	if (Index > 0)
	    CAGD_GEN_COPY(&NewCrv -> Points[i][0], &Crv -> Points[i][0],
			  sizeof(CagdRType) * Index);
	if (Index < Crv -> Length)
	    CAGD_GEN_COPY(&NewCrv -> Points[i][Index + 1],
			  &Crv -> Points[i][Index],
			  sizeof(CagdRType) * (Crv -> Length - Index));
    }

    /* Insert the new point. */
    if (IsRational)
	NewCrv -> Points[0][Index] = 1.0;
    for (i = 1; i <= MaxAxis; i++)
        NewCrv -> Points[i][Index] = i > 3 ? 0.0 : Pt[i - 1];

    /* Update the knot sequence, if has one. */
    if (NewCrv -> KnotVector != NULL)
	BspKnotUniformOpen(NewCrv -> Length, Crv -> Order,
			   NewCrv -> KnotVector);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Delete a point at Index from curve Crv.			             M
* Returned curve's length is smaller by one than the length of Crv.          M
*   Knot vector is updated (if Bspline curve) to a uniform open.	     M
*   Order of curve is reduced if greater than new number of control points.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     Input curve to delete a point from.        	             M
*   Index:   Index of control point to delete from Crv. 	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A new curve of length smaller by one than Crv.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvInsertPoint                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDeletePoint		                                             M
*****************************************************************************/
CagdCrvStruct *CagdCrvDeletePoint(const CagdCrvStruct *Crv, int Index)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	NewOrder = IRIT_MIN(Crv -> Length - 1, Crv -> Order),
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *NewCrv;

    if (NewOrder < 1) {
	CAGD_FATAL_ERROR(CAGD_ERR_LIN_NO_SUPPORT);
	return NULL;
    } 

    NewCrv = BspCrvNew(Crv -> Length - 1, NewOrder, Crv -> PType);

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    /* Copy old points, excluding point Index. */
    for (i = !IsRational; i <= MaxAxis; i++) {
	if (Index > 0)
	    CAGD_GEN_COPY(&NewCrv -> Points[i][0], &Crv -> Points[i][0],
			  sizeof(CagdRType) * Index);
	if (Index < Crv -> Length - 1)
	    CAGD_GEN_COPY(&NewCrv -> Points[i][Index],
			  &Crv -> Points[i][Index + 1],
			  sizeof(CagdRType) * (Crv -> Length - Index - 1));
    }

    /* Update the knot sequence, if has one. */
    if (NewCrv -> KnotVector != NULL)
	BspKnotUniformOpen(NewCrv -> Length, Crv -> Order,
			   NewCrv -> KnotVector);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Provides the way to modify/get a single control point into/from a surface. M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to be modified/query.                                M
*   CtlPt:      New control point to be substituted into Srf. Must carry the M
*               same PType as Srf if to be written to Srf.                   M
*   UIndex, VIndex: In surface Srf's control mesh to substitute/query CtlPt. M
*   Write:      If TRUE CtlPt is copied into Srf, if FALSE the point is      M
*               copied from Srf to CtlPt.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: If Write is TRUE, the new modified curve, if WRITE is   M
*                    FALSE, NULL.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEditSingleSrfPt, surface editing                                     M
*****************************************************************************/
CagdSrfStruct *CagdEditSingleSrfPt(CagdSrfStruct *Srf,
				   CagdCtlPtStruct *CtlPt,
				   int UIndex,
				   int VIndex,
				   CagdBType Write)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*NewSrf = Write ? CagdSrfCopy(Srf) : NULL;
    CagdRType
	**Points = Write ? NewSrf -> Points : Srf -> Points;

    if (UIndex < 0 || UIndex >= ULength ||
	VIndex < 0 || VIndex >= VLength)
	CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

    if (Write) {
	if (Srf -> PType != CtlPt -> PtType)
	    CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);

	CAGD_DEL_GEOM_TYPE(NewSrf);

	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][CAGD_MESH_UV(NewSrf, UIndex, VIndex)] =
		CtlPt -> Coords[i];
    }
    else {
        CtlPt -> PtType = Srf -> PType;

	for (i = IsNotRational; i <= MaxCoord; i++)
	    CtlPt -> Coords[i] =
		Points[i][CAGD_MESH_UV(Srf, UIndex, VIndex)];
    }

    return NewSrf;
}
