/******************************************************************************
* Skel2D.c - 2D skeleton computation.                     		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 99.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"

IRIT_STATIC_DATA CagdRType
    Skel2DOutExtent = 10.0,
    Skel2DSubdivTol = 1e-2,
    Skel2DNumerTol = -1e-10,
    Skel2DEpsilon = 1e-5,
    Skel2DFineNess = 150.0;

static CagdCrvStruct *Skel2DComputeBisector(MvarSkel2DPrimStruct *Prim1,
					    MvarSkel2DPrimStruct *Prim2);
static CagdCrvStruct *Skel2DConvertToBsplineCrv(MvarSkel2DPrimStruct *Prim);
static MvarSkel2DInter3PrimsStruct *Skel2DInterBisectors(
						 CagdCrvStruct *Bisect12,
						 CagdCrvStruct *Bisect13,
						 MvarSkel2DPrimStruct *Prim1,
						 MvarSkel2DPrimStruct *Prim2,
						 MvarSkel2DPrimStruct *Prim3);
static int UpdatePrimPosition(CagdPType EquadistantPt,
			      MvarSkel2DPrimStruct *Prim,
			      MvarSkel2DPrimStruct *OtherPrim,
			      CagdPType Pos,
			      CagdCrvStruct *Bisector,
			      CagdRType t,
			      CagdBType FirstPrim);
static MvarSkel2DInter3PrimsStruct *Skel2DEqPts3Crvs(
						  MvarSkel2DPrimStruct *Prim1,
						  MvarSkel2DPrimStruct *Prim2,
						  MvarSkel2DPrimStruct *Prim3);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the epsilon of the 2D skeleton computation.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   NewEps:    New epsilon to use.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Old epsilon value.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   Skel2DInter3Primitives                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DSetEpsilon, skeleton, bisector                                 M
*****************************************************************************/
CagdRType MvarSkel2DSetEpsilon(CagdRType NewEps)
{
    CagdRType
	OldEps = Skel2DEpsilon;

    Skel2DEpsilon = NewEps;
    
    return OldEps;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the tolerances to be used in this module for he multivariate zero   M
* set solver.                                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   SubdivTol, NumerTol:  Subdivision and numeric tolerance of mvar solver.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:     Old SubdivTol.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarSkel2DSetEpsilon, MvarSkel2DSetFineNess                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DSetMZeroTols                                                   M
*****************************************************************************/
CagdRType MvarSkel2DSetMZeroTols(CagdRType SubdivTol, CagdRType NumerTol)
{
    CagdRType
	OldSubdivTol = Skel2DSubdivTol;

    Skel2DSubdivTol = SubdivTol;
    Skel2DNumerTol = NumerTol;
    
    return OldSubdivTol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the fineness of the 2D skeleton computation.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   NewFineNess:    New fineness to use.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Old fineness value.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   Skel2DInter3Primitives                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DSetFineNess, skeleton, bisector                                M
*****************************************************************************/
CagdRType MvarSkel2DSetFineNess(CagdRType NewFineNess)
{
    CagdRType
	OldFineNess = Skel2DFineNess;

    Skel2DFineNess = NewFineNess;
    
    return OldFineNess;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the outer extent of created (infinite) primitives in the 2D         M
* skeleton computation. 				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   NewOutExtent:    New outer extent to use.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Old outer extent value.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   Skel2DInter3Primitives                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DSetOuterExtent, skeleton, bisector                             M
*****************************************************************************/
CagdRType MvarSkel2DSetOuterExtent(CagdRType NewOutExtent)
{
    CagdRType
	OldOutExtent = Skel2DOutExtent;

    Skel2DOutExtent = NewOutExtent;

    return OldOutExtent;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all points in R2 that are equadistant from the given three      M
* primitives.  A primitive can be a point, a line, an arc, or a freeform     M
* curve.  The end points of the line/arc/curve are NOT considered.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Prim1, Prim2, Prim3:   The three input primitives to consider.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarSkel2DInter3PrimsStruct *:  A linked list of all equadistant         M
*		                    points computed, or NULL if none found.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSkel2DSetEpsilon, MvarSkel2DSetFineness, MvarSkel2DSetOuterExtent    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DInter3Prims, skeleton, bisector                                M
*****************************************************************************/
MvarSkel2DInter3PrimsStruct *MvarSkel2DInter3Prims(MvarSkel2DPrimStruct *Prim1,
						   MvarSkel2DPrimStruct *Prim2,
						   MvarSkel2DPrimStruct *Prim3)
{
    MvarSkel2DInter3PrimsStruct *EquadistPts;

    /* Keep original order and sort according to complexity of geometry. */
    Prim1 -> _Index = 0;
    Prim2 -> _Index = 1;
    Prim3 -> _Index = 2;

    Prim1 -> _CrvRep = Prim2 -> _CrvRep = Prim3 -> _CrvRep = NULL;

    if (Prim3 -> Type < Prim1 -> Type)
        IRIT_SWAP(MvarSkel2DPrimStruct *, Prim1, Prim3);
    if (Prim2 -> Type < Prim1 -> Type)
        IRIT_SWAP(MvarSkel2DPrimStruct *, Prim1, Prim2);
    if (Prim3 -> Type < Prim2 -> Type)
        IRIT_SWAP(MvarSkel2DPrimStruct *, Prim2, Prim3);

    /* Create a polynomial representation to the geometry. */
    Prim1 -> _CrvRep = Skel2DConvertToBsplineCrv(Prim1);
    Prim2 -> _CrvRep = Skel2DConvertToBsplineCrv(Prim2);
    Prim3 -> _CrvRep = Skel2DConvertToBsplineCrv(Prim3);

    if (Prim1 -> Type >= MVAR_SK2D_PRIM_LINE &&
	Prim3 -> Type >= MVAR_SK2D_PRIM_ARC) {
	/* All primitive are lines or curves and at least one is freeform. */
	EquadistPts = Skel2DEqPts3Crvs(Prim1, Prim2, Prim3);
    }
    else {
	CagdCrvStruct
	    *Bisect12 = Skel2DComputeBisector(Prim1, Prim2),
	    *Bisect13 = Skel2DComputeBisector(Prim1, Prim3);

	EquadistPts = Skel2DInterBisectors(Bisect12, Bisect13,
					   Prim1, Prim2, Prim3);

	CagdCrvFreeList(Bisect12);
	CagdCrvFreeList(Bisect13);
    }

    CagdCrvFree(Prim1 -> _CrvRep);
    CagdCrvFree(Prim2 -> _CrvRep);
    CagdCrvFree(Prim3 -> _CrvRep);

    return EquadistPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of 2d skeleton intersection structure.     M
*                                                                            *
* PARAMETERS:                                                                M
*   SK2DInt:       To be deallocated.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DInter3PrimsFree, free                                          M
*****************************************************************************/
void MvarSkel2DInter3PrimsFree(MvarSkel2DInter3PrimsStruct *SK2DInt)
{
    IP_ATTR_FREE_ATTRS(SK2DInt -> Attr);
    IritFree(SK2DInt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a 2d skeleton intersection structure list:	     M
*                                                                            *
* PARAMETERS:                                                                M
*   SK2DIntList:  To be deallocated.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSkel2DInter3PrimsFreeList, free                                      M
*****************************************************************************/
void MvarSkel2DInter3PrimsFreeList(MvarSkel2DInter3PrimsStruct *SK2DIntList)
{
    MvarSkel2DInter3PrimsStruct *SK2DIntTemp;

    while (SK2DIntList) {
	SK2DIntTemp = SK2DIntList -> Pnext;
	MvarSkel2DInter3PrimsFree(SK2DIntList);
	SK2DIntList = SK2DIntTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Compute the bisector curve(s) between the given two planar entities and   *
* return it/them as a list of Bspline curve(s).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Prim1, Prim2:    Two planar entities to compute their bisector.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The computed bisector(s) as a list of Bspline curve.  *
*****************************************************************************/
static CagdCrvStruct *Skel2DComputeBisector(MvarSkel2DPrimStruct *Prim1,
					    MvarSkel2DPrimStruct *Prim2)
{
    IrtRType t1, t2;
    IrtPtType Pos1, Pos2;
    CagdPtStruct Pt1, Pt2;
    IrtVecType Dir1, Dir2, Dir12;
    CagdCrvStruct *TCrv,
	*Crv1 = Prim1 -> _CrvRep,
	*Crv2 = Prim2 -> _CrvRep,
	*Bisector = NULL;
    MvarSkel2DPrimStruct Prim1Tmp, Prim2Tmp;

    switch (Prim1 -> Type) {
	case MVAR_SK2D_PRIM_POINT:
	    switch (Prim2 -> Type) {
		case MVAR_SK2D_PRIM_POINT:
		    if (IRIT_PT_APX_EQ_E2(Prim1 -> Pt.Pt, Prim2 -> Pt.Pt))
		        return NULL;

		    /* Compute the middle line between the two points! */
		    IRIT_PT_ADD(Pos1, Prim1 -> Pt.Pt, Prim2 -> Pt.Pt);
		    IRIT_PT_SCALE(Pos1, 0.5);
		    IRIT_PT_SUB(Dir1, Prim1 -> Pt.Pt, Prim2 -> Pt.Pt);
		    IRIT_SWAP(IrtRType, Dir1[0], Dir1[1]);
		    Dir1[0] = -Dir1[0];
		    IRIT_VEC_NORMALIZE(Dir1);
		    IRIT_VEC_SCALE(Dir1, Skel2DOutExtent);
		    IRIT_PT_ADD(Pt1.Pt, Pos1, Dir1);
		    IRIT_VEC_SCALE(Dir1, -1.0);
		    IRIT_PT_ADD(Pt2.Pt, Pos1, Dir1);
		    Bisector = CagdMergePtPt(&Pt1, &Pt2);
		    break;
		case MVAR_SK2D_PRIM_LINE:
		case MVAR_SK2D_PRIM_ARC:
		case MVAR_SK2D_PRIM_CRV:
		    Bisector = SymbCrvPtBisectorCrv2D(Crv2, Prim1 -> Pt.Pt,
						      0.5);
		    break;
	    }
	    break;
	case MVAR_SK2D_PRIM_LINE:
	    switch (Prim2 -> Type) {
		case MVAR_SK2D_PRIM_LINE:
		    IRIT_PT_SUB(Dir1, Prim1 -> Ln.Pt2, Prim1 -> Ln.Pt1);
		    IRIT_PT_SUB(Dir2, Prim2 -> Ln.Pt2, Prim2 -> Ln.Pt1);
		    IRIT_VEC_NORMALIZE(Dir1);
		    IRIT_VEC_NORMALIZE(Dir2);
		    if (IRIT_DOT_PROD(Dir1, Dir2) > 1.0 - Skel2DEpsilon)
		        return NULL;       /* Two lines are almost parallel. */

		    /* Find the intersection point. */
		    GM2PointsFromLineLine(Prim1 -> Ln.Pt1, Dir1,
					  Prim2 -> Ln.Pt1, Dir2,
					  Pos1, &t1, Pos2, &t2);
		    IRIT_PT_ADD(Pos1, Pos1, Pos2);
		    IRIT_PT_SCALE(Pos1, 0.5);

		    /* Compute the first bisector line. */
		    IRIT_VEC_ADD(Dir12, Dir1, Dir2);
		    IRIT_VEC_NORMALIZE(Dir12);
		    IRIT_VEC_SCALE(Dir12, Skel2DOutExtent);
		    IRIT_PT_ADD(Pt1.Pt, Pos1, Dir12);
		    IRIT_PT_SUB(Pt2.Pt, Pos1, Dir12);
		    /* This bisector should probably be trimmed to the       */
		    /* proper length.  Right now it extends to "infinity".   */
		    Bisector = CagdMergePtPt(&Pt1, &Pt2);

		    /* Compute the second bisector line. */
		    IRIT_VEC_SUB(Dir12, Dir1, Dir2);
		    IRIT_VEC_NORMALIZE(Dir12);
		    IRIT_VEC_SCALE(Dir12, Skel2DOutExtent);
		    IRIT_PT_ADD(Pt1.Pt, Pos1, Dir12);
		    IRIT_PT_SUB(Pt2.Pt, Pos1, Dir12);
		    /* This bisector should probably be trimmed to the       */
		    /* proper length.  Right now it extends to "infinity".   */
		    Bisector -> Pnext = CagdMergePtPt(&Pt1, &Pt2);
		    break;
		case MVAR_SK2D_PRIM_ARC:
		    /* Convert to an offset of the line by the arc radius    */
		    /* and a point which is an offset of the arc.            */
		    IRIT_PT_SUB(Dir1, Prim1 -> Ln.Pt2, Prim1 -> Ln.Pt1);
		    IRIT_VEC_NORMALIZE(Dir1);
		    IRIT_SWAP(IrtRType, Dir1[0], Dir1[1]);
		    Dir1[0] = -Dir1[0];
		    IRIT_VEC_SCALE(Dir1, Prim2 -> Arc.Radius);
		    IRIT_PT_ADD(Pt1.Pt, Prim1 -> Ln.Pt1, Dir1);
		    IRIT_PT_ADD(Pt2.Pt, Prim1 -> Ln.Pt2, Dir1);
		    TCrv = CagdMergePtPt(&Pt1, &Pt2);
		    Bisector = SymbCrvPtBisectorCrv2D(TCrv,
						      Prim2 -> Arc.Center,
						      0.5);
		    CagdCrvFree(TCrv);
		    break;
		case MVAR_SK2D_PRIM_CRV:
		    /* This is a crv-crv case which is solved numerically. */
		    Crv1 -> Pnext = Crv2;
		    Bisector = SymbCrvBisectors(Crv1, TRUE, Skel2DFineNess,
						TRUE, FALSE, TRUE);
		    Crv1 -> Pnext = NULL;
		    break;
	        default:
		    assert(0);
	    }
	    break;
	case MVAR_SK2D_PRIM_ARC:
	    switch (Prim2 -> Type) {
		case MVAR_SK2D_PRIM_ARC:
		    /* Reduce to a point-arc case and resolve. */
		    if (Prim1 -> Arc.Radius > Prim2 -> Arc.Radius) {
			Prim2Tmp.Type = MVAR_SK2D_PRIM_POINT;
			IRIT_PT_COPY(Prim2Tmp.Pt.Pt, Prim2 -> Arc.Center);
			Prim1Tmp = *Prim1;
			Prim1Tmp.Arc.Radius -= Prim2 -> Arc.Radius;
			Bisector = Skel2DComputeBisector(&Prim2Tmp,
							 &Prim1Tmp);
		    }
		    else if (Prim1 -> Arc.Radius < Prim2 -> Arc.Radius) {
			Prim1Tmp.Type = MVAR_SK2D_PRIM_POINT;
			IRIT_PT_COPY(Prim1Tmp.Pt.Pt, Prim1 -> Arc.Center);
			Prim2Tmp = *Prim2;
			Prim2Tmp.Arc.Radius -= Prim1 -> Arc.Radius;
			Bisector = Skel2DComputeBisector(&Prim1Tmp,
							 &Prim2Tmp);
		    }
		    else {
		        /* Solve as in the point-point bisector case. */
			Prim1Tmp.Type = Prim2Tmp.Type = MVAR_SK2D_PRIM_POINT;
			IRIT_PT_COPY(Prim1Tmp.Pt.Pt, Prim1 -> Arc.Center);
			IRIT_PT_COPY(Prim2Tmp.Pt.Pt, Prim2 -> Arc.Center);
			Bisector = Skel2DComputeBisector(&Prim1Tmp,
							 &Prim2Tmp);
		    }
		    break;
		case MVAR_SK2D_PRIM_CRV:
		    /* This is a crv-crv case which is solved numerically. */
		    Crv1 -> Pnext = Crv2;
		    Bisector = SymbCrvBisectors(Crv1, TRUE, Skel2DFineNess,
						TRUE, FALSE, TRUE);
		    Crv1 -> Pnext = NULL;
		    break;
	        default:
		    assert(0);
	    }
	    break;
	case MVAR_SK2D_PRIM_CRV:
	    switch (Prim2 -> Type) {
		case MVAR_SK2D_PRIM_CRV:
		    /* This is a crv-crv case which is solved numerically. */
		    Crv1 -> Pnext = Crv2;
		    Bisector = SymbCrvBisectors(Crv1, TRUE, Skel2DFineNess,
						TRUE, FALSE, TRUE);
		    Crv1 -> Pnext = NULL;
		    break;
	        default:
		    assert(0);
	    }
	    break;
    }

    return Bisector;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the (line/arc/curve) primitives to a (Bspline/Bezier) curve.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Prim:     Input primitive to convert to a curve.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The primitive as a curve if possible, NULL otherwise. *
*****************************************************************************/
static CagdCrvStruct *Skel2DConvertToBsplineCrv(MvarSkel2DPrimStruct *Prim)
{
    CagdPtStruct Pt1, Pt2;
    CagdCrvStruct
	*Crv = NULL;

    switch (Prim -> Type) {
	case MVAR_SK2D_PRIM_POINT:
	    break;
	case MVAR_SK2D_PRIM_LINE:
	    IRIT_PT_COPY(Pt1.Pt, Prim -> Ln.Pt1);
	    IRIT_PT_COPY(Pt2.Pt, Prim -> Ln.Pt2);
	    Crv = CagdMergePtPt(&Pt1, &Pt2);
	    break;
	case MVAR_SK2D_PRIM_ARC:
	    IRIT_PT_COPY(Pt1.Pt, Prim -> Arc.Center);
	    Crv = CagdCrvCreateArc(&Pt1,
				   Prim -> Arc.Radius,
				   Prim -> Arc.StartAngle,
				   Prim -> Arc.EndAngle);
	    break;
	case MVAR_SK2D_PRIM_CRV:
	    Crv = CagdCrvCopy(Prim -> Crv.Crv);
	    break;
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the intersection points of the given two (bisector) curves and   *
* return them with relevant information on the three primitives.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Bisect12, Bisect13:  The two (bisector) curves to intersect against each *
*                        other.						     *
*   Prim1, Prim2, Prim3: The three original primitives so we could reference *
*			 them in the returned data structures.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarSkel2DInter3PrimsStruct *:    List of intersection points, if any,   *
*                                     or NULL otherwise.                     *
*****************************************************************************/
static MvarSkel2DInter3PrimsStruct *Skel2DInterBisectors(
						 CagdCrvStruct *Bisect12,
						 CagdCrvStruct *Bisect13,
						 MvarSkel2DPrimStruct *Prim1,
						 MvarSkel2DPrimStruct *Prim2,
						 MvarSkel2DPrimStruct *Prim3)
{
    CagdCrvStruct *Crv12, *Crv13;
    MvarSkel2DInter3PrimsStruct
	*InterList = NULL;

    for (Crv12 = Bisect12; Crv12 != NULL; Crv12 = Crv12 -> Pnext) {
	for (Crv13 = Bisect13; Crv13 != NULL; Crv13 = Crv13 -> Pnext) {
	    CagdCrvStruct
		*Crv12Aux = Crv12 -> PType == CAGD_PT_E4_TYPE
		        ? CagdCoerceCrvTo(Crv12, CAGD_PT_E2_TYPE, FALSE)
			: CagdCrvCopy(Crv12),
		*Crv13Aux = Crv13 -> PType == CAGD_PT_E4_TYPE
		        ? CagdCoerceCrvTo(Crv13, CAGD_PT_E2_TYPE, FALSE)
			: CagdCrvCopy(Crv13);
    	    CagdPtStruct *Pt,
		*Pts = CagdCrvCrvInter(Crv12Aux, Crv13Aux, Skel2DEpsilon);

	    CagdCrvFree(Crv12Aux);
	    CagdCrvFree(Crv13Aux);

	    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
		CagdRType *R;
		CagdPType Pt1, Pt2;
		MvarSkel2DInter3PrimsStruct
		    *Inter = (MvarSkel2DInter3PrimsStruct *)
			IritMalloc(sizeof(MvarSkel2DInter3PrimsStruct));

		/* Compute the equadistant point as an average of the two    */
		/* locations on the two parametric curves Crv12/13.	     */
		R = CagdCrvEval(Crv12, Pt -> Pt[0]);
		CagdCoerceToE3(Pt1, &R, -1, Crv12 -> PType);
		R = CagdCrvEval(Crv13, Pt -> Pt[1]);
		CagdCoerceToE3(Pt2, &R, -1, Crv13 -> PType);
		IRIT_PT_ADD(Inter -> EquadistPoint, Pt1, Pt2);
		IRIT_PT_SCALE(Inter -> EquadistPoint, 0.5);

		/* Compute the locations on the three varieties. */
		if (UpdatePrimPosition(Inter -> EquadistPoint,
				       Prim1, Prim2, Inter -> PosPrim1,
				       Crv12, Pt -> Pt[0], TRUE) &&
		    UpdatePrimPosition(Inter -> EquadistPoint,
				       Prim2, Prim1, Inter -> PosPrim2,
				       Crv12, Pt -> Pt[0], FALSE) &&
		    UpdatePrimPosition(Inter -> EquadistPoint,
				       Prim3, Prim1, Inter -> PosPrim3,
				       Crv13, Pt -> Pt[1], FALSE))  {
		    Inter -> Attr = NULL;
		    IRIT_LIST_PUSH(Inter, InterList);
		}
		else
		    IritFree(Inter);
	    }

	    CagdPtFreeList(Pts);
	}
    }

    return InterList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the location on the primitive that is associated with this        *
* equadistance to three primitives.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   EquadistantPt:  The equadistant location from the three primitives.      *
*   Prim:       Original input primitive.				     *
*   OtherPrim:  Other original input primitive supporting given Bisector.    *
*   Pos:        Position to update into.                                     *
*   Bisector:   The bisector which Prim is one of its two supports.          *
*   t:          The parameter value of the Bisector point at equadistance.   *
*   FirstPrim:  TRUE for first primitive, FALSE for second in two primitives *
*		that formed the Bisector curve.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	TRUE if position inside domain, FALSE othewise.              *
*****************************************************************************/
static int UpdatePrimPosition(CagdPType EquadistantPt,
			      MvarSkel2DPrimStruct *Prim,
			      MvarSkel2DPrimStruct *OtherPrim,
			      CagdPType Pos,
			      CagdCrvStruct *Bisector,
			      CagdRType t,
			      CagdBType FirstPrim)
{
    CagdRType *R;
    CagdVType Dir, Dir2;

    switch (Prim -> Type) {
	case MVAR_SK2D_PRIM_POINT:
	    IRIT_PT_COPY(Pos, Prim -> Pt.Pt);
	    break;
	case MVAR_SK2D_PRIM_LINE:
	    IRIT_PT_SUB(Dir, Prim -> Ln.Pt2, Prim -> Ln.Pt1);
	    GMPointFromPointLine(EquadistantPt, Prim -> Ln.Pt1, Dir, Pos);
	    /* CHeck solution is inside line segment. */
	    IRIT_PT_SUB(Dir2, Prim -> Ln.Pt2, EquadistantPt);
	    if (IRIT_DOT_PROD(Dir, Dir2) < 0.0)
		return FALSE;
	    IRIT_PT_SUB(Dir2, EquadistantPt, Prim -> Ln.Pt1);
	    if (IRIT_DOT_PROD(Dir, Dir2) < 0.0)
		return FALSE;
	    break;
	case MVAR_SK2D_PRIM_ARC:
	    IRIT_PT_SUB(Dir, Prim -> Arc.Center, EquadistantPt);
	    IRIT_PT_NORMALIZE(Dir);
	    IRIT_PT_SCALE(Dir, Prim -> Arc.Radius);
	    IRIT_PT_ADD(Pos, Prim -> Arc.Center, Dir);
	    break;
	case MVAR_SK2D_PRIM_CRV:
	    IRIT_PT_RESET(Pos);
	    switch (OtherPrim -> Type) {
		case MVAR_SK2D_PRIM_POINT:
		    R = CagdCrvEval(Prim -> _CrvRep, t);
		    CagdCoerceToE3(Pos, &R, -1, Prim -> _CrvRep -> PType);
		    break;
		case MVAR_SK2D_PRIM_LINE:
		case MVAR_SK2D_PRIM_ARC:
		case MVAR_SK2D_PRIM_CRV:
		    /* Get from the intersection location on the Bisector,   */
		    /* the parameter location of the original curve there.   */
		    R = CagdCrvEval(Bisector, t);
		    t = FirstPrim ? R[3] : R[4];

		    /* And evaluate the original curve at that location. */
		    R = CagdCrvEval(Prim -> _CrvRep, t);
		    CagdCoerceToE3(Pos, &R, -1, Prim -> _CrvRep -> PType);
		    break;
	    }
	    break;
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Formulate multivariate constraints for the points that are at equal      *
* distance from the three primitives and solve for them.  Assumes that the   *
* three primitives are lines or curves and that they do NOT intersect.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Prim1, Prim2, Prim3:   The three input primitives to consider.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarSkel2DInter3PrimsStruct *:  A linked list of all equadistant         *
*		                    points computed, or NULL if none found.  *
*****************************************************************************/
static MvarSkel2DInter3PrimsStruct *Skel2DEqPts3Crvs(
						  MvarSkel2DPrimStruct *Prim1,
						  MvarSkel2DPrimStruct *Prim2,
						  MvarSkel2DPrimStruct *Prim3)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *DCrv,
	*Crv1 = CagdCrvCopy(Prim1 -> _CrvRep),
	*Crv2 = CagdCrvCopy(Prim2 -> _CrvRep),
	*Crv3 = CagdCrvCopy(Prim3 -> _CrvRep);
    MvarMVStruct *MVCrv1, *MVCrv2, *MVCrv3, *MVTan1, *MVTan2, *MVTan3,
	*MVVec[MVAR_MAX_PT_SIZE], *MVA1Split[MVAR_MAX_PT_SIZE],**MVA2Split,
	*MVTmp1, *MVTmp2, *MVb1, *MVb2, *MVA1, *MVA2, *MVPDenom, *MVPNumer;
    MvarConstraintType Constraints[3];
    MvarPtStruct *MVPts, *MVPt;
    MvarSkel2DInter3PrimsStruct
	*EquadistPts = NULL;

    /* Make sure all curves are with domain zero to one. */
    CagdCrvDomain(Crv1, &TMin, &TMax);
    if (CAGD_IS_BSPLINE_CRV(Crv1))
	BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
				 Crv1 -> Length + Crv1 -> Order, 0.0, 1.0);
    CagdCrvDomain(Crv2, &TMin, &TMax);
    if (CAGD_IS_BSPLINE_CRV(Crv2))
	BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);
    CagdCrvDomain(Crv3, &TMin, &TMax);
    if (CAGD_IS_BSPLINE_CRV(Crv3))
	BspKnotAffineTransOrder2(Crv3 -> KnotVector, Crv3 -> Order,
				 Crv3 -> Length + Crv3 -> Order, 0.0, 1.0);

    /* Convert position curves. */
    MVTmp1 = MvarCrvToMV(Crv1);
    MVCrv1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv3);
    MVCrv3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);

    /* Convert tangent curves. */
    DCrv = CagdCrvDerive(Crv1);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv2);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv3);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    /* Formulate distance constraints so that distance between Prim1 and     */
    /* Prim2 is similar and the distance between Prim1 and Prim3 is similar: */
    /*									     */
    /* < P - C1(u), P - C1(u) > = < P - C2(v), P - C2(v) >,		     */
    /* < P - C1(u), P - C1(u) > = < P - C3(w), P - C3(w) >,  or		     */
    /*									     */
    /* 2(C2(v) - C1(u)) P = C2(v)^2 - C1(u)^2,               		     */
    /* 2(C3(w) - C1(u)) P = C3(w)^2 - C1(u)^2, and solve for P = P(u, v, w). */
    MVA1 = MvarMVSub(MVCrv2, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv2, MVCrv1);
    MVb1 = MvarMVDotProd(MVA1, MVTmp1);
    MvarMVTransform(MVA1, NULL, 2.0);
    MvarMVFree(MVTmp1);
    
    MVA2 = MvarMVSub(MVCrv3, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv3, MVCrv1);
    MVb2 = MvarMVDotProd(MVA2, MVTmp1); 
    MvarMVTransform(MVA2, NULL, 2.0);
    MvarMVFree(MVTmp1);

    IRIT_GEN_COPY(MVA1Split, MvarMVSplitScalar(MVA1),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVA2Split = MvarMVSplitScalar(MVA2);
    MvarMVFree(MVA1);
    MvarMVFree(MVA2);

    /* Solve for P = P(u, v, w). */
    IRIT_ZAP_MEM(MVVec, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVPDenom = MvarMVDeterminant2(MVA1Split[1], MVA1Split[2],
				  MVA2Split[1], MVA2Split[2]);
    MVVec[1] = MvarMVDeterminant2(MVb1, MVA1Split[2],
				  MVb2, MVA2Split[2]);
    MVVec[2] = MvarMVDeterminant2(MVA1Split[1], MVb1,
				  MVA2Split[1], MVb2);
    MvarMVFree(MVA1Split[1]);
    MvarMVFree(MVA1Split[2]);
    if (MVA1Split[3] != NULL)
	MvarMVFree(MVA1Split[3]);

    MvarMVFree(MVA2Split[1]);
    MvarMVFree(MVA2Split[2]);
    if (MVA2Split[3] != NULL)
	MvarMVFree(MVA2Split[3]);

    MvarMVFree(MVb1);
    MvarMVFree(MVb2);

    MVPNumer = MvarMVMergeScalar(MVVec);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /* Now formulate out the three following constraints with P's solution. */
    MVTmp1 = MvarMVMultScalar(MVCrv1, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[0] = MvarMVDotProd(MVTmp2, MVTan1);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv2, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[1] = MvarMVDotProd(MVTmp2, MVTan2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv3, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[2] = MvarMVDotProd(MVTmp2, MVTan3);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);
    MvarMVFree(MVCrv3);
    MvarMVFree(MVTan1);
    MvarMVFree(MVTan2);
    MvarMVFree(MVTan3);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
    
    MVPts = MvarMVsZeros(MVVec, Constraints, 3, Skel2DSubdivTol,
			 Skel2DNumerTol);
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /* Convert the computed points to our form. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	CagdRType *R, t, d1, d2, d3;
	MvarSkel2DInter3PrimsStruct
	    *EqPt = (MvarSkel2DInter3PrimsStruct *)
	        IritMalloc(sizeof(MvarSkel2DInter3PrimsStruct));

	EqPt -> Attr = NULL;

	/* Update the position on three original primitive curves. */
	R = CagdCrvEval(Crv1, MVPt -> Pt[0]);
	CagdCoerceToE3(EqPt -> PosPrim1, &R, -1, Crv1 -> PType);

	R = CagdCrvEval(Crv2, MVPt -> Pt[1]);
	CagdCoerceToE3(EqPt -> PosPrim2, &R, -1, Crv2 -> PType);

	R = CagdCrvEval(Crv3, MVPt -> Pt[2]);
	CagdCoerceToE3(EqPt -> PosPrim3, &R, -1, Crv3 -> PType);

	/* And compute the singular equadistant point. */
	R = MvarMVEval(MVPNumer, MVPt -> Pt);
	CagdCoerceToE3(EqPt -> EquadistPoint, &R, -1,
		       (CagdPointType) MVPNumer -> PType);

	R = MvarMVEval(MVPDenom, MVPt -> Pt);
	t = R[1] == 0 ? IRIT_INFNTY : 1.0 / R[1];
	IRIT_PT_SCALE(EqPt -> EquadistPoint, t);

	/* Validate solutions - might get bogus answers if input geometry  */
	/* intersect itself.						   */
	d1 = IRIT_PT2D_DIST_SQR(EqPt -> PosPrim1, EqPt -> EquadistPoint);
	d2 = IRIT_PT2D_DIST_SQR(EqPt -> PosPrim2, EqPt -> EquadistPoint);
	d3 = IRIT_PT2D_DIST_SQR(EqPt -> PosPrim3, EqPt -> EquadistPoint);
	if( IRIT_APX_EQ_EPS(d1, d2, Skel2DEpsilon) &&
	    IRIT_APX_EQ_EPS(d1, d3, Skel2DEpsilon)) {
	    IRIT_LIST_PUSH(EqPt, EquadistPts);
	}
	else
	    IritFree(EqPt);
    }

    MvarMVFree(MVPNumer);
    MvarMVFree(MVPDenom);

    MvarPtFreeList(MVPts);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Crv3);

    return EquadistPts;
}

