/*****************************************************************************
*   Module to build primitive shapes in model form.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0, June 2011    *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "mdl_loc.h"

IRIT_STATIC_DATA int
    GlblMdlCubeSpherePrim = FALSE,
    GlblMdlStitchSelfSrfPrim = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compare two given trimming curve segments for similarity in Euclidean    M
* space.  Comparison is made at the end points only.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TSeg1, TSeg2:   The two segments to compare.                             M
*   Tol:            Tolerance of comparison of end points.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   0 if not the same Euclidean representation, 1 if same and start   M
*          and end points match, -1 if same but reversed.	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTwoTrimSegsSameEndPts                                                 M
*****************************************************************************/
int MdlTwoTrimSegsSameEndPts(MdlTrimSegStruct *TSeg1,
			     MdlTrimSegStruct *TSeg2,
			     CagdRType Tol)
{
    CagdRType t1, t2, *R;
    CagdPType Pt1Start, Pt1End, Pt2Start, Pt2End;

    if (TSeg1 -> EucCrv != NULL && TSeg2 -> EucCrv != NULL) {
        CagdCrvDomain(TSeg1 -> EucCrv, &t1, &t2);

	R = CagdCrvEval(TSeg1 -> EucCrv, t1);
	CagdCoerceToE3(Pt1Start, &R, -1, TSeg1 -> EucCrv -> PType);
	R = CagdCrvEval(TSeg1 -> EucCrv, t2);
	CagdCoerceToE3(Pt1End, &R, -1, TSeg1 -> EucCrv -> PType);

        CagdCrvDomain(TSeg2 -> EucCrv, &t1, &t2);

	R = CagdCrvEval(TSeg2 -> EucCrv, t1);
	CagdCoerceToE3(Pt2Start, &R, -1, TSeg2 -> EucCrv -> PType);
	R = CagdCrvEval(TSeg2 -> EucCrv, t2);
	CagdCoerceToE3(Pt2End, &R, -1, TSeg2 -> EucCrv -> PType);
    }
    else if (TSeg1 -> UVCrvFirst != NULL && TSeg2 -> UVCrvFirst != NULL) {
        CagdRType UV[2];

        CagdCrvDomain(TSeg1 -> UVCrvFirst, &t1, &t2);

	R = CagdCrvEval(TSeg1 -> UVCrvFirst, t1);
	CagdCoerceToE2(UV, &R, -1, TSeg1 -> UVCrvFirst -> PType);
	R = CagdSrfEval(TSeg1 -> SrfFirst -> Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt1Start, &R, -1, TSeg1 -> SrfFirst -> Srf -> PType);

	R = CagdCrvEval(TSeg1 -> UVCrvFirst, t2);
	CagdCoerceToE2(UV, &R, -1, TSeg1 -> UVCrvFirst -> PType);
	R = CagdSrfEval(TSeg1 -> SrfFirst -> Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt1Start, &R, -1, TSeg1 -> SrfFirst -> Srf -> PType);


        CagdCrvDomain(TSeg2 -> UVCrvFirst, &t1, &t2);

	R = CagdCrvEval(TSeg2 -> UVCrvFirst, t1);
	CagdCoerceToE2(UV, &R, -1, TSeg2 -> UVCrvFirst -> PType);
	R = CagdSrfEval(TSeg2 -> SrfFirst -> Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt1Start, &R, -1, TSeg2 -> SrfFirst -> Srf -> PType);

	R = CagdCrvEval(TSeg2 -> UVCrvFirst, t2);
	CagdCoerceToE2(UV, &R, -1, TSeg2 -> UVCrvFirst -> PType);
	R = CagdSrfEval(TSeg2 -> SrfFirst -> Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt1Start, &R, -1, TSeg2 -> SrfFirst -> Srf -> PType);
    }
    else
        return 0;

    if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2Start, Tol) &&
	IRIT_PT_APX_EQ_EPS(Pt1End, Pt2End, Tol))
        return 1;

    if (IRIT_PT_APX_EQ_EPS(Pt1Start, Pt2End, Tol) &&
	IRIT_PT_APX_EQ_EPS(Pt1End, Pt2Start, Tol))
        return -1;
	
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find the reference to TSeg in TSrf.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:   Trimmed surface to search the trimming segment reference in.     M
*   TSeg:   The segment to seek the trimmed curve reference for.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegRefStruct *:   Reference if found, NULL otherwise.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlGetSrfTrimSegRef                                                      M
*****************************************************************************/
MdlTrimSegRefStruct *MdlGetSrfTrimSegRef(MdlTrimSrfStruct *TSrf,
					 MdlTrimSegStruct *TSeg)
{
    MdlLoopStruct
        *Loops = TSrf -> LoopList;

    for ( ; Loops != NULL; Loops = Loops -> Pnext) {
        MdlTrimSegRefStruct *TRef;

	for (TRef = Loops -> SegRefList;
	     TRef != NULL;
	     TRef = TRef -> Pnext) {
	    if (TRef -> TrimSeg == TSeg)
	        return TRef;
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scans the given model for trimming curves that could be stitched         M
* together.   A pair of trimming curves can be stitched together if they     M
* have the same Euclidean representation and they now have no neighbors.     M
*   Two trimming curves are considered with "same Euclidean representation"  M
* if their end points are the same upto given tolerance (should probably do  M
* somewhat better here).				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mdl:        Model to seek trimming curves to stitch together, in place.  M
*   StitchTol:  Tolerance to use in stitching two trimmed curves as one.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Number of trimming curves stitched together, in place.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlBooleanMerge                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlStitchModel                                                           M
*****************************************************************************/
int MdlStitchModel(MdlModelStruct *Mdl, CagdRType StitchTol)
{
    int Dir,
        n = 0;
    MdlTrimSegStruct *TSeg1, *TSeg2, *TSeg2Prev;
    MdlTrimSegRefStruct *TSeg2Ref;

    for (TSeg1 = Mdl -> TrimSegList; TSeg1 != NULL; TSeg1 = TSeg1 -> Pnext) {
        for (TSeg2Prev = TSeg1, TSeg2 = TSeg2Prev -> Pnext;
	     TSeg2 != NULL;
	     TSeg2Prev = TSeg2, TSeg2 = TSeg2 -> Pnext) {
	    if (TSeg1 -> SrfSecond == NULL &&
		TSeg2 -> SrfSecond == NULL &&
		(Dir = MdlTwoTrimSegsSameEndPts(TSeg1, TSeg2, 
						StitchTol)) != 0) {
	        /* Merge them and remove TSeg2 from the trimming segs list. */
	        n++;

		/* Find the reference to TSeg2 in the trimmed surface and   */
		/* update the back pointer to point to TSeg1.               */
		TSeg2Ref = MdlGetSrfTrimSegRef(TSeg2 -> SrfFirst, TSeg2);
		assert(TSeg2Ref != NULL);
		TSeg2Ref -> TrimSeg = TSeg1;

		if ((TSeg2Ref -> Reversed = Dir == -1) != FALSE)
		    TSeg1 -> UVCrvSecond = CagdCrvReverse(TSeg2 -> UVCrvFirst);
		else {
		    TSeg1 -> UVCrvSecond = TSeg2 -> UVCrvFirst;
		    TSeg2 -> UVCrvFirst = NULL;
		}
	        TSeg1 -> SrfSecond = TSeg2 -> SrfFirst;
	        TSeg2 -> SrfFirst = NULL;

		/* Free TSeg2. */
		TSeg2Prev -> Pnext = TSeg2 -> Pnext;
		MdlTrimSegFree(TSeg2);
		break;
	    }
	}
    }

    return n;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a plane, parallel to XY plane at level Zlevel.    M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY:  Minimum XY coordinates of plane.                            M
*   MaxX, MaxY:  Maximum XY coordinates of plane.                            M
*   ZLevel:      Z level of plane, parallel to the XY plane.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:    Constructed plane model, as a bilinear surface.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, MdlPrimPlaneSrfOrderLen                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimPlane                                                             M
*****************************************************************************/
MdlModelStruct *MdlPrimPlane(CagdRType MinX,
			     CagdRType MinY,
			     CagdRType MaxX,
			     CagdRType MaxY,
			     CagdRType ZLevel)
{
    CagdSrfStruct
        *Srf = CagdPrimPlaneSrf(MinX, MinY, MaxX, MaxY, ZLevel);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a plane, parallel to XY plane at level Zlevel.    M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY:  Minimum XY coordinates of plane.                            M
*   MaxX, MaxY:  Maximum XY coordinates of plane.                            M
*   ZLevel:      Z level of plane, parallel to the XY plane.                 M
*   Order:	 Order of plane surface that is requested.		     M
*   Len:         Number of control points (via refinement).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  Constructed plane model, as a bi-Order surface.       M
*                                                                            *
* SEE ALSO:                                                                  M
*  CagdPrimPlaneSrfOrderLen, MdlPrimPlane                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimPlaneSrfOrderLen                                                  M
*****************************************************************************/
MdlModelStruct *MdlPrimPlaneSrfOrderLen(CagdRType MinX,
					CagdRType MinY,
					CagdRType MaxX,
					CagdRType MaxY,
					CagdRType ZLevel,
					int Order,
					int Len)
{
    CagdSrfStruct
        *Srf = CagdPrimPlaneSrfOrderLen(MinX, MinY, MaxX, MaxY, ZLevel,
					Order, Len);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a box, parallel to main axes.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY, MinZ:   Minimum range of box model.                          M
*   MaxX, MaxY, MaxZ:   Maximum range of box model.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  Constructed box model, as a set of bilinear surfaces. M
*                                                                            *
* SEE ALSO:                                                                  M
*  CagdPrimBoxSrf			                                     M
**                                                                           *
* KEYWORDS:                                                                  M
*   MdlPrimBox                                                               M
*****************************************************************************/
MdlModelStruct *MdlPrimBox(CagdRType MinX,
			   CagdRType MinY,
			   CagdRType MinZ,
			   CagdRType MaxX,
			   CagdRType MaxY,
			   CagdRType MaxZ)
{
    int n,
        OldVal = MdlBoolSetOutputInterCrv(FALSE);
    CagdSrfStruct
        *SrfXYMin = CagdPrimPlaneSrf(MaxX, MinY, MinX, MaxY, MinZ),
        *SrfXYMax = CagdPrimPlaneSrf(MinX, MinY, MaxX, MaxY, MaxZ),
        *SrfXZMin = CagdPrimPlaneXZSrf(MinX, MinZ, MaxX, MaxZ, MinY),
        *SrfXZMax = CagdPrimPlaneXZSrf(MaxX, MinZ, MinX, MaxZ, MaxY),
        *SrfYZMin = CagdPrimPlaneYZSrf(MaxY, MinZ, MinY, MaxZ, MinX),
        *SrfYZMax = CagdPrimPlaneYZSrf(MinY, MinZ, MaxY, MaxZ, MaxX);
    MdlModelStruct *M,
        *Mdl1 = MdlModelNew(SrfXYMin, NULL, FALSE),
        *Mdl2 = MdlModelNew(SrfXYMax, NULL, FALSE),
        *Mdl3 = MdlModelNew(SrfXZMin, NULL, FALSE),
        *Mdl4 = MdlModelNew(SrfXZMax, NULL, FALSE),
        *Mdl5 = MdlModelNew(SrfYZMin, NULL, FALSE),
        *Mdl6 = MdlModelNew(SrfYZMax, NULL, FALSE);
    IPObjectStruct
        *Mdl12 = MdlBooleanMerge(Mdl1, Mdl2, FALSE),
        *Mdl34 = MdlBooleanMerge(Mdl3, Mdl4, FALSE),
        *Mdl56 = MdlBooleanMerge(Mdl5, Mdl6, FALSE),
        *Mdl1234 = MdlBooleanMerge(Mdl12 -> U.Mdls, Mdl34 -> U.Mdls, FALSE),
        *Mdl = MdlBooleanMerge(Mdl1234 -> U.Mdls, Mdl56 -> U.Mdls, FALSE);

    MdlBoolSetOutputInterCrv(OldVal);

    MdlModelFree(Mdl1);
    MdlModelFree(Mdl2);
    MdlModelFree(Mdl3);
    MdlModelFree(Mdl4);
    MdlModelFree(Mdl5);
    MdlModelFree(Mdl6);
    IPFreeObject(Mdl12);
    IPFreeObject(Mdl34);
    IPFreeObject(Mdl56);
    IPFreeObject(Mdl1234);

    M = Mdl -> U.Mdls;
    Mdl -> U.Mdls = NULL;
    IPFreeObject(Mdl);

    n = MdlStitchModel(M, IRIT_EPS);
    assert(n == 12);

    return M;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a sphere, centered at Center and radius Radius.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   of constructed sphere.                                         M
*   Radius:   of constructed sphere.                                         M
*   Rational: If TRUE exact ration sphere is created.  If FALSE an	     M
*		 approximated integral surface is created.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:   Constructed sphere model.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimSphereSrf, MdlStitchSelfSrfPrims, MdlCreateCubeSpherePrim        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimSphere                                                            M
*****************************************************************************/
MdlModelStruct *MdlPrimSphere(const CagdVType Center,
			      CagdRType Radius,
			      CagdBType Rational)
{
    if (GlblMdlCubeSpherePrim) {           /* Use the cube-topology sphere. */
        int n,
	    OldVal = MdlBoolSetOutputInterCrv(FALSE);
        CagdSrfStruct
	    *SprSrf1 = CagdPrimCubeSphereSrf(Center, Radius),
	    *SprSrf2 = SprSrf1 -> Pnext,
	    *SprSrf3 = SprSrf2 -> Pnext,
	    *SprSrf4 = SprSrf3 -> Pnext,
  	    *SprSrf5 = SprSrf4 -> Pnext,
	    *SprSrf6 = SprSrf5 -> Pnext;
        MdlModelStruct *M,
	    *Mdl1 = MdlModelNew(SprSrf1, NULL, FALSE),
	    *Mdl2 = MdlModelNew(SprSrf2, NULL, FALSE),
	    *Mdl3 = MdlModelNew(SprSrf3, NULL, FALSE),
	    *Mdl4 = MdlModelNew(SprSrf4, NULL, FALSE),
	    *Mdl5 = MdlModelNew(SprSrf5, NULL, FALSE),
	    *Mdl6 = MdlModelNew(SprSrf6, NULL, FALSE);
	IPObjectStruct
	    *Mdl12 = MdlBooleanMerge(Mdl1, Mdl2, FALSE),
	    *Mdl34 = MdlBooleanMerge(Mdl3, Mdl4, FALSE),
	    *Mdl56 = MdlBooleanMerge(Mdl5, Mdl6, FALSE),
	    *Mdl1234 = MdlBooleanMerge(Mdl12 -> U.Mdls, Mdl34 -> U.Mdls, FALSE),
	    *Mdl = MdlBooleanMerge(Mdl1234 -> U.Mdls, Mdl56 -> U.Mdls, FALSE);

	MdlBoolSetOutputInterCrv(OldVal);

	MdlModelFree(Mdl1);
	MdlModelFree(Mdl2);
	MdlModelFree(Mdl3);
	MdlModelFree(Mdl4);
	MdlModelFree(Mdl5);
	MdlModelFree(Mdl6);
	IPFreeObject(Mdl12);
	IPFreeObject(Mdl34);
	IPFreeObject(Mdl56);
	IPFreeObject(Mdl1234);

	M = Mdl -> U.Mdls;
	Mdl -> U.Mdls = NULL;
	IPFreeObject(Mdl);

	n = MdlStitchModel(M, IRIT_EPS);
	assert(n == 12);

	return M;
    }
    else {
        CagdSrfStruct
	    *Srf = CagdPrimSphereSrf(Center, Radius, Rational);
	MdlModelStruct
	    *Mdl = MdlModelNew(Srf, NULL, FALSE);

	if (GlblMdlStitchSelfSrfPrim)
	    MdlStitchModel(Mdl, IRIT_EPS);

	return Mdl;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a torus, centered at Center and radii of          M
* MajorRadius and MinorRadius.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed torus.                                     M
*   MajorRadius:   of constructed torus.                                     M
*   MinorRadius:   of constructed torus.                                     M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:   Constructed torus model.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimTorusSrf, MdlStitchSelfSrfPrims                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimTorus                                                             M
*****************************************************************************/
MdlModelStruct *MdlPrimTorus(const CagdVType Center,
			     CagdRType MajorRadius,
			     CagdRType MinorRadius,
			     CagdBType Rational)
{
    CagdSrfStruct
        *Srf = CagdPrimTorusSrf(Center, MajorRadius, MinorRadius, Rational);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    if (GlblMdlStitchSelfSrfPrim)
        MdlStitchModel(Mdl, IRIT_EPS);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a truncated cone, centered at Center and radii    M
* of MajorRadius and MinorRadius. A MinorRadius of zero would construct a    M
* regular cone.  Otherwise, a truncated cone.  Axis of cone is Z axis.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed cone (center of its base).                 M
*   MajorRadius:   of constructed cone.                                      M
*   MinorRadius:   of constructed cone.                                      M
*   Height:	   of constructed cone.                                      M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  Constructed truncated cone model.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimCone2Srf, MdlStitchSelfSrfPrims                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimCone2                                                             M
*****************************************************************************/
MdlModelStruct *MdlPrimCone2(const CagdVType Center,
			     CagdRType MajorRadius,
			     CagdRType MinorRadius,
			     CagdRType Height,
			     CagdBType Rational,
			     CagdPrimCapsType Caps)
{
    CagdSrfStruct
        *Srf = CagdPrimCone2Srf(Center, MajorRadius, MinorRadius, Height,
				Rational, Caps);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    if (GlblMdlStitchSelfSrfPrim)
        MdlStitchModel(Mdl, IRIT_EPS);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a cone, centered at Center, radii of Radius,      M
* and height of Height.	 Axis of cone is Z axis.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed cone (center of its base).                 M
*   Radius:        of constructed cone's base.                               M
*   Height:	   of constructed cone.                                      M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:  Constructed cone model.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimConeSrf, MdlStitchSelfSrfPrims                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimCone                                                              M
*****************************************************************************/
MdlModelStruct *MdlPrimCone(const CagdVType Center,
			    CagdRType Radius,
			    CagdRType Height,
			    CagdBType Rational,
			    CagdPrimCapsType Caps)
{
    CagdSrfStruct
        *Srf = CagdPrimConeSrf(Center, Radius, Height, Rational, Caps);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    if (GlblMdlStitchSelfSrfPrim)
        MdlStitchModel(Mdl, IRIT_EPS);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A model constructor of a Cylinder, centered at Center, radii of          M
* Radius, and height of Height.	 Axis of cylinder is Z axis.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed Cylinder (center of its base).             M
*   Radius:        of constructed Cylinder.                                  M
*   Height:	   of constructed Cylinder.                                  M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:    Constructed cylinder model.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimCylinderSrf, MdlStitchSelfSrfPrims                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPrimCylinder                                                          M
*****************************************************************************/
MdlModelStruct *MdlPrimCylinder(const CagdVType Center,
				CagdRType Radius,
				CagdRType Height,
				CagdBType Rational,
				CagdPrimCapsType Caps)
{
    CagdSrfStruct
        *Srf = CagdPrimCylinderSrf(Center, Radius, Height, Rational, Caps);
    MdlModelStruct
        *Mdl = MdlModelNew(Srf, NULL, FALSE);

    if (GlblMdlStitchSelfSrfPrim)
        MdlStitchModel(Mdl, IRIT_EPS);

    return Mdl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set if different boundaries on the same primitive surface are to be      M
* stitched as well or not.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Stitch:   TRUE to stitch different boundaries of same surface.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old value setting.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlStitchSelfSrfPrims                                                    M
*****************************************************************************/
int MdlStitchSelfSrfPrims(int Stitch)
{
    int OldVal = GlblMdlStitchSelfSrfPrim;

    GlblMdlStitchSelfSrfPrim = Stitch;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Select to create sphere using cube-topology, if TRUE.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   CubeTopoSphere:   TRUE to construct spheres using cube-topology.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old value setting.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlCreateCubeSpherePrim                                                  M
*****************************************************************************/
int MdlCreateCubeSpherePrim(int CubeTopoSphere)
{
    int OldVal = GlblMdlCubeSpherePrim;

    GlblMdlCubeSpherePrim = CubeTopoSphere;

    return OldVal;
}

