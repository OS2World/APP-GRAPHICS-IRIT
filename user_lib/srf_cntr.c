/******************************************************************************
* SrfCntr.c - contour a surface with a plane.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June. 95.					      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "bool_lib.h"
#include "allocate.h"
#include "geom_lib.h"
#include "user_loc.h"

#define MAX_PLANE_SIZE	        100.0
#define CONTOUR_MERGE_EPS	2e-3

CagdBType GlblAllWeightsSame;

static CagdRType SrfOnContour(const CagdSrfStruct *Srf);
static CagdRType TriangleOnContour(const CagdPType P1,
				   const CagdPType P2,
				   const CagdPType P3);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A call back function of the subdivision process for contouring.          *
* This function tests if given surface intersects or on with the contouring  *
* plane.  If no intersection occurs, this function returns a value that will *
* terminate the subdivision process.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:          Current sub-surface in the subdivision tree.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    Negative value if given surface does no intersect the	     *
*		contouring plane, positive otherwise.			     *
*                                                                            *
* SEE ALSO:                                                                  *
*   UserCntrSrfWithPlane                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   SrfOnContour                                                             *
*****************************************************************************/
static CagdRType SrfOnContour(const CagdSrfStruct *Srf)
{
    int i,
	Len = Srf -> ULength * Srf -> VLength,
	AboveOrOn = FALSE,
	Below = FALSE;
    CagdRType
	*Axs = Srf -> Points[3];

    /* Tests for surface control mesh being above and below the plane. */
    if (GlblAllWeightsSame) {
	for (i = 0; i < Len; i++, Axs++) {
	    if (*Axs < 0.0) {
	        Below = TRUE;
		if (AboveOrOn)
		    break;
	    }
	    if (*Axs >= 0.0) {
	        AboveOrOn = TRUE;
		if (Below)
		    break;
	    }
	}
    }
    else {
        CagdRType
	    *WAxs = Srf -> Points[0];

	for (i = 0; i < Len; i++, Axs++, WAxs++) {
	    if (*Axs * *WAxs < 0.0) {
	        Below = TRUE;
		if (AboveOrOn)
		    break;
	    }
	    if (*Axs * *WAxs >= 0.0) {
	        AboveOrOn = TRUE;
		if (Below)
		    break;
	    }
	}
    }

    return AboveOrOn && Below ? 1.0 : -1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A call back function of the subdivision process for contouring.          *
* This function tests if given triangle intersects or on with the contouring *
* plane.  If no intersection occurs, this function returns a value that will *
* terminate the subdivision process.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2, P3:	The three points of the triangle.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    Negative value if given triangle does not intersect the    *
*		contouring plane, Positive if intersects.		     *
*                                                                            *
* SEE ALSO:                                                                  *
*   UserCntrSrfWithPlane                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   TriangleOnContour                                                        *
*****************************************************************************/
static CagdRType TriangleOnContour(const CagdPType P1,
				   const CagdPType P2,
				   const CagdPType P3)
{
    int AboveOrOn, Below;

    Below = P1[2] < 0.0 || P2[2] < 0.0 || P3[2] < 0.0;
    AboveOrOn = P1[2] >= 0.0 || P2[2] >= 0.0 || P3[2] >= 0.0;

    return AboveOrOn && Below ? 1.0 : -1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection of a freeform surface and a plane by	     M
* approximating the surface by polygons and calling Boolean tools.	     M
*   If Srf is a scalar surface then it is promoted first into a              M
* 3D Euclidean surface with UV as YZ coordinates (and X has scalar field).   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To approximate its intersection with the given plane.           M
*   Plane:   To intersect with the given surface. If NULL the XY plane is    M
*	     used.							     M
*   FineNess:   Control of polygonal approximation of surface. See	     M
*	     IritSurface2Polygons function.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A list of polylines approximating the contour.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCntrEvalToE3							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCntrSrfWithPlane, contouring                                         M
*****************************************************************************/
IPPolygonStruct *UserCntrSrfWithPlane(const CagdSrfStruct *Srf,
				      const IrtPlnType Plane,
				      IrtRType FineNess)
{
    int OldMergeVal, OldInterCurve, OldRes, OldCirc;
    CagdRType Width, t, r;
    CagdVType NrmlPlane;
    CagdPType SrfCenter, PlaneCenter, Pt;
    IrtHmgnMatType Mat, InvMat;
    CagdSrfStruct *Srf3Space, *TSrf;
    IPObjectStruct *PlaneObj, *SrfObj, *CntrObj;
    IPPolygonStruct *Pl, *CntrPoly, *SrfPolys;
    CagdSrfErrorFuncType OldSrfFunc;
    CagdPlgErrorFuncType OldPlgFunc;

    /* Rotate the surface so the Plane is the Z = 0 plane. */
    if ((t = IRIT_DOT_PROD(Plane, Plane)) == 0.0) {
	USER_FATAL_ERROR(USER_ERR_INVALID_PLANE);
	return NULL;
    }
    r = 1 / sqrt(t);
    IRIT_VEC_COPY(NrmlPlane, Plane);
    IRIT_VEC_SCALE(NrmlPlane, r);             /* A unit normal of the plane. */

    t = -Plane[3] / t;
    IRIT_PT_COPY(PlaneCenter, Plane);
    IRIT_PT_SCALE(PlaneCenter, t);	            /* A point on the plane. */

    /* Compute a rotation of the plane's normal to the Z axis. */
    GMGenMatrixZ2Dir(Mat, NrmlPlane);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    /* And now translate. */
    MatMultPtby4by4(Pt, PlaneCenter, InvMat);
    MatGenMatTrans(0.0, 0.0, -Pt[2], Mat);

    MatMultTwo4by4(Mat, InvMat, Mat);    /* Combine the rot+trans into one. */

    switch (CAGD_NUM_OF_PT_COORD(Srf -> PType)) {
	case 1:
	case 2:
	    if (CAGD_IS_RATIONAL_SRF(Srf))
	        Srf3Space = CagdCoerceSrfTo(Srf, CAGD_PT_P3_TYPE, TRUE);
	    else
	        Srf3Space = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, TRUE);
	    break;
	default:
	    Srf3Space = CagdSrfCopy(Srf);
	    break;
    }
    TSrf = CagdSrfMatTransform(Srf3Space, Mat);
    CagdSrfFree(Srf3Space);
    Srf3Space = TSrf;

    MatInverseMatrix(Mat, InvMat);

    /* Extract the polygons that cross the Z = 0 plane: */
    OldCirc = IPSetPolyListCirc(TRUE);
    OldMergeVal = CagdSrf2PolygonMergeCoplanar(FALSE);
    OldSrfFunc = BspSrf2PolygonSetErrFunc(SrfOnContour);
    OldPlgFunc = IPPolygonSetErrFunc(TriangleOnContour);

    GlblAllWeightsSame = CagdAllWeightsSame(Srf3Space -> Points,
					    Srf3Space -> ULength *
					        Srf3Space -> VLength);

    SrfPolys = IPSurface2Polygons(Srf3Space, FALSE, FineNess, FALSE, FALSE, 0);

    IPPolygonSetErrFunc(OldPlgFunc);
    BspSrf2PolygonSetErrFunc(OldSrfFunc);
    CagdSrf2PolygonMergeCoplanar(OldMergeVal);

#ifdef DUMP_SRF_AND_POLYS
    {
	int Handle = IPOpenDataFile("srf_cntr.itd", FALSE, TRUE);

	IPPutObjectToHandler(Handle, IPGenSRFObject(Srf3Space));
	IPPutObjectToHandler(Handle, IPGenPOLYObject(SrfPolys));
	IPCloseStream(Handle, TRUE);
    }
#endif /* DUMP_SRF_AND_POLYS */

    if (SrfPolys != NULL) {
	/* Find location of surface so we can position the plane properly. */
        GMBBBboxStruct
	    *BBox = GMBBComputePolyListBbox(SrfPolys);

	IRIT_PT_RESET(SrfCenter);
	Width = IRIT_MAX(BBox -> Max[0] - BBox -> Min[0],
		    BBox -> Max[1] - BBox -> Min[1]);
	if (Width > MAX_PLANE_SIZE)
	    Width = MAX_PLANE_SIZE;

	PlaneCenter[0] = (BBox -> Min[0] + BBox -> Max[0]) * 0.5;
	PlaneCenter[1] = (BBox -> Min[1] + BBox -> Max[1]) * 0.5;
	PlaneCenter[2] = 0.0;
	NrmlPlane[0] = NrmlPlane[1] = 0.0;
	NrmlPlane[2] = 1.0;

	/* Constructs the simplest contouring plane - a triangle. */
	OldRes = PrimSetResolution(3);
	PlaneObj = PrimGenPOLYDISKObject(NrmlPlane, PlaneCenter, Width * 2.0);
	PrimSetResolution(OldRes);

	SrfObj = IPGenPOLYObject(SrfPolys);

	OldInterCurve = BoolSetOutputInterCurve(TRUE);
	CntrObj = BooleanAND(SrfObj, PlaneObj);
	BoolSetOutputInterCurve(OldInterCurve);

	CntrPoly = CntrObj -> U.Pl;
	CntrObj -> U.Pl = NULL;

	IPFreeObject(CntrObj);
        IPFreeObject(SrfObj);
	IPFreeObject(PlaneObj);
    }
    else {
	CntrPoly = NULL;
    }

    CagdSrfFree(Srf3Space);

    IPSetPolyListCirc(OldCirc);

    CntrPoly = GMMergePolylines(CntrPoly, CONTOUR_MERGE_EPS);

    Pl = GMTransformPolyList(CntrPoly, InvMat, FALSE);
    IPFreePolygonList(CntrPoly);
    return Pl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate contours Cntrs, in palce, into Euclidean space.  Input UV       M
* contours are mapped through Srf to yield 3-space points.  In the process   M
* all contour points are being validated through ValidCntrPtFunc, if not     M
* NULL.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             Surface to evaluate contours on.                        M
*   Cntrs:           Contours to evaluate, in place.			     M
*		     The input contours should hold UV coodinates in the YZ  M
*		     coefficients of the points.			     M
*   ValidCntrPtFunc: Each point along the contours is validated through this M
*		     function if !NULL.  In valid points break the contour   M
*		     and are purged away.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:    Mapped and validated contour in 3-space.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCntrSrfWithPlane                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCntrEvalToE3                                                         M
*****************************************************************************/
IPPolygonStruct *UserCntrEvalToE3(const CagdSrfStruct *Srf,
				  IPPolygonStruct *Cntrs,
				  UserCntrIsValidCntrPtFuncType ValidCntrPtFunc)
{
    CagdRType UMin, UMax, VMin, VMax;
    IPPolygonStruct *Cntr;
    IPVertexStruct *V;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	for (V = Cntr -> PVertex; V != NULL; ) {
	    if (ValidCntrPtFunc == NULL ||
		ValidCntrPtFunc(Srf, V -> Coord[1], V -> Coord[2])) {
	        CagdRType
		   *P = CagdSrfEval(Srf,
				    IRIT_BOUND(V -> Coord[1], UMin, UMax),
				    IRIT_BOUND(V -> Coord[2], VMin, VMax));

		CagdCoerceToE3(V -> Coord, &P, -1, Srf -> PType);
		V = V -> Pnext;
	    }
	    else {
	        /* Invalid contour point - purge it. */
	        if (V != Cntr -> PVertex) {
		    /* Break from previous contour and start a new one. */
		    IPVertexStruct
			*VPrev = IPGetPrevVrtx(Cntr -> PVertex, V);

		    VPrev -> Pnext = NULL;
		    Cntr -> Pnext = IPAllocPolygon(Cntr -> Tags, V,
						   Cntr -> Pnext);
		    Cntr = Cntr -> Pnext;
		}

		Cntr -> PVertex = V -> Pnext;
		IPFreeVertex(V);
		V = Cntr -> PVertex;
	    }
	}
    }

    /* Purge empty contours. */
    while (Cntrs != NULL && Cntrs -> PVertex == NULL) {
        Cntr = Cntrs;
	Cntrs = Cntrs -> Pnext;
	IPFreePolygon(Cntr);
    }

    if (Cntrs != NULL) {
        for (Cntr = Cntrs; Cntr -> Pnext != NULL; ) {
	    if (Cntr -> Pnext -> PVertex == NULL) {
	        IPPolygonStruct *TCntr;

	        TCntr = Cntr -> Pnext;
		Cntr -> Pnext = TCntr -> Pnext;
		IPFreePolygon(TCntr);
	    }
	    else
		Cntr = Cntr -> Pnext;
	}
    }

    return Cntrs;
}
