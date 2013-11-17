/******************************************************************************
* TrimCntr.c - Converts a set of contours in parametric space to trimmed srfs *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 96.					      *
******************************************************************************/

#include "irit_sm.h"
#include "trim_loc.h"
#include "user_lib.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_lib.h"

#define CONTOUR_EPS	1e-3

#define VRTX_BOUNDARY_TAG	0x10
#define IS_BOUNDARY_VRTX(Vrtx)	(((Vrtx) -> Tags) & VRTX_BOUNDARY_TAG)
#define SET_BOUNDARY_VRTX(Vrtx)	((Vrtx) -> Tags |= VRTX_BOUNDARY_TAG)
#define RST_BOUNDARY_VRTX(Vrtx)	((Vrtx) -> Tags &= ~VRTX_BOUNDARY_TAG)

#define ASSIGN_VERTEX_COORDS(V, X, Y, Z) { (V) -> Coord[0] = (X); \
					   (V) -> Coord[1] = (Y); \
					   (V) -> Coord[2] = (Z); \
					   SET_BOUNDARY_VRTX(V); }

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugPrintVertexList, FALSE);
#endif /* DEBUG */

IRIT_STATIC_DATA TrimSrfStruct
    *GlblCntrTrimSrfs = NULL;

static void TrimSrfsFromTrimPlsAux(TrimCrvStruct *TrimCrvsHierarchy,
				   const CagdSrfStruct *Srf,
				   int Depth);
static void UpdateBoundaryCntrEdges(IPPolygonStruct *Cntrs);
static CagdCrvStruct *TrimPolylines2LinBsplineCrvs(const IPPolygonStruct *Polys,
						   CagdRType UMin,
						   CagdRType UMax,
						   CagdRType VMin,
						   CagdRType VMax);
static TrimCrvStruct *TrimPolylines2LinTrimCrvs2(const IPPolygonStruct *Polys,
						 CagdRType UMin,
						 CagdRType UMax,
						 CagdRType VMin,
						 CagdRType VMax);
static IPPolygonStruct *ClosedCntrsFromOpenCntrs(IPPolygonStruct *OpenCntrs,
						 IPPolygonStruct *Domain,
						 CagdRType UMin,
						 CagdRType UMax,
						 CagdRType VMin,
						 CagdRType VMax);
static IPPolygonStruct *SplitDomainByCntr(IPPolygonStruct *Domain,
					  IPPolygonStruct *Cntr,
					  CagdRType UMin,
					  CagdRType UMax,
					  CagdRType VMin,
					  CagdRType VMax);
static int WeightRelationInside(CagdRType V1, CagdRType V2, CagdRType V);
static CagdRType VertexWeight(IPVertexStruct *V,
			      CagdRType UMin,
			      CagdRType UMax,
			      CagdRType VMin,
			      CagdRType VMax);

#ifdef DEBUG
static void PrintVrtxList(IPVertexStruct *V);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creats a set of trimmed surfaces as defined by the given set of contours M
* that can contain either closed or open contours.  Open contours must       M
* terminate at the boundary of the parameteric domain of the surface. Closed M
* contours must be completely contained in the parametric domain with last   M
* point equals first.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To trim into pieces.                                         M
*   CCntrs:     Polylines to use as separating edges.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   List of trimmed surface pieces.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfsFromTrimPlsHierarchy                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfsFromContours                                                     M
*****************************************************************************/
TrimSrfStruct *TrimSrfsFromContours(const CagdSrfStruct *Srf,
				    const IPPolygonStruct *CCntrs)
{
    CagdRType UMin, UMax, VMin, VMax;
    IPPolygonStruct *Cntr, *SrfDomain,
	*OpenCntrs = NULL,
	*ClosedCntrs = NULL;
    IPVertexStruct *VTmp;
    TrimSrfStruct
	*RetTrimSrfs = NULL;
    IPPolygonStruct *Cntrs;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    Cntrs = IPCopyPolygonList(CCntrs);         /* Keep the original intact. */

    /* Split the contours into closed and open loops. */
    while (Cntrs) {
	IPPolygonStruct *Cntr;
	IPVertexStruct *V, *VLast;

	IRIT_LIST_POP(Cntr, Cntrs);
	V = Cntr -> PVertex,
	VLast = IPGetLastVrtx(V);

	if (IRIT_PT_APX_EQ_EPS(V -> Coord, VLast -> Coord, CONTOUR_EPS) &&
	    GMPolyLength(Cntr) > CONTOUR_EPS) {
	    IRIT_LIST_PUSH(Cntr, ClosedCntrs);
	}
	else if (CAGD_PT_ON_BNDRY(V -> Coord[0], V -> Coord[1],
				  UMin, UMax, VMin, VMax, CONTOUR_EPS) &&
		 CAGD_PT_ON_BNDRY(VLast -> Coord[0], VLast -> Coord[1],
				  UMin, UMax, VMin, VMax, CONTOUR_EPS)) {
	    IRIT_LIST_PUSH(Cntr, OpenCntrs);
	}
	else {
	    IPFreePolygon(Cntr);
	}
    }

    /* Create a domain polygon of entire surface as top level. */
    VTmp = IPAllocVertex2(NULL);
    ASSIGN_VERTEX_COORDS(VTmp, UMin, VMin, 0.0);
    VTmp = IPAllocVertex2(VTmp);
    ASSIGN_VERTEX_COORDS(VTmp, UMax, VMin, 0.0);
    VTmp = IPAllocVertex2(VTmp);
    ASSIGN_VERTEX_COORDS(VTmp, UMax, VMax, 0.0);
    VTmp = IPAllocVertex2(VTmp);
    ASSIGN_VERTEX_COORDS(VTmp, UMin, VMax, 0.0);
    VTmp = IPAllocVertex2(VTmp);
    ASSIGN_VERTEX_COORDS(VTmp, UMin, VMin, 0.0);
    SrfDomain = IPAllocPolygon(0, IPReverseVrtxList2(VTmp), NULL);

    /* Split open contours into closed regions with the aid of the boundary. */
    if (OpenCntrs != NULL) {
	/* Mark all vertices in the open contours as non boundary. */
	for (Cntr = OpenCntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	    IPVertexStruct *V;

	    for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext)
	        RST_BOUNDARY_VRTX(V);	    
	}
	OpenCntrs = ClosedCntrsFromOpenCntrs(OpenCntrs, SrfDomain,
					     UMin, UMax, VMin, VMax);
	UpdateBoundaryCntrEdges(OpenCntrs);

	IPFreePolygon(SrfDomain);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugOpenContours, FALSE) {
		IPStderrObject(IPGenPOLYObject(OpenCntrs));
	    }
	}
#	endif /* DEBUG */
    }
    else {
	/* Place the entire domain as one open contour. */
	OpenCntrs = SrfDomain;
    }

    /* We now have only open contours. OpenCntrs contains a set of disjoint */
    /* loops formed with the boundary.  However, ClosedCntrs contains	    */
    /* contours that must be classified into the contours in OpenCntrs as a */
    /* whole hierarchy.							    */
    while (OpenCntrs) {
	IPPolygonStruct *ClosedCntr,
	    *PrevCntr = NULL,
	    *ClosedCntrsOfOpenCntr = NULL,
	    *OpenCntr = OpenCntrs;
	TrimSrfStruct *TrimSrfs;

	OpenCntrs = OpenCntrs -> Pnext;
	OpenCntr -> Pnext = NULL;

	for (ClosedCntr = ClosedCntrs; ClosedCntr != NULL; ) {
	    if (GMPolygonRayInter(OpenCntr,
				  ClosedCntr -> PVertex -> Coord, 0) & 0x01) {
		IPPolygonStruct
		    *NextCntr = ClosedCntr -> Pnext;

		/* We have odd number of intersections - this closed loop is */
		/* to be classified inside the open loop, OpenCntr.	     */
		if (PrevCntr == NULL)
		    ClosedCntrs = NextCntr;
		else
		    PrevCntr -> Pnext = NextCntr;

		IRIT_LIST_PUSH(ClosedCntr, ClosedCntrsOfOpenCntr);

		ClosedCntr = NextCntr;
	    }
	    else {
		PrevCntr = ClosedCntr;
		ClosedCntr = ClosedCntr -> Pnext;
	    }
	}

	/* Create trimmed surfaces using the trimming loops, */
	TrimSrfs = TrimSrfsFromTrimPlsHierarchy(OpenCntr,
						ClosedCntrsOfOpenCntr, Srf);
	IPFreePolygonList(OpenCntr);
	IPFreePolygonList(ClosedCntrsOfOpenCntr);
	RetTrimSrfs = CagdListAppend(TrimSrfs, RetTrimSrfs);
    }

    if (ClosedCntrs != NULL) {
	/* We should have classified all ClosedCntrs at this time. */
	TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
    }

    return RetTrimSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct trimmed surface from the given hierarchy of trimming           M
* polylines.  If TopLevel is provided, it serves as the top level outer loop M
* and both Odd and Even nested trimmed surfaces are extracted.  If TopLevel  M
* is NULL only Odd nested trimmed surfaces are extracted.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TopLevel:      The top level outer loop or NULL if none.	             M
*   TrimPls:       Hierarchy of trimming polylines.       	             M
*   Srf:           Surface to trim out.	                 	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   List of trimmed surface out of the given counters.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrfsFromContours                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrfsFromTrimPlsHierarchy                                             M
*****************************************************************************/
TrimSrfStruct *TrimSrfsFromTrimPlsHierarchy(IPPolygonStruct *TopLevel,
					    IPPolygonStruct *TrimPls,
					    const CagdSrfStruct *Srf)
{
    CagdRType UMin, UMax, VMin, VMax;
    TrimCrvStruct *TrimCrvs;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    GlblCntrTrimSrfs = NULL;

    /* First Pass without the top level loop. */
    if (TrimPls != NULL) {	
	TrimCrvs = TrimPolylines2LinTrimCrvs2(TrimPls,
					      UMin, UMax, VMin, VMax);

	TrimClassifyTrimmingLoops(&TrimCrvs);
	TrimSrfsFromTrimPlsAux(TrimCrvs, Srf, 0);/* Free TrimCrvs on the fly.*/
    }

    /* Second Pass with the top level loop. */
    if (TopLevel) {
	IPPolygonStruct
	    *Pnext = TopLevel -> Pnext;

	TopLevel -> Pnext = TrimPls;
	TrimCrvs = TrimPolylines2LinTrimCrvs2(TopLevel,
					      UMin, UMax, VMin, VMax);

	TrimClassifyTrimmingLoops(&TrimCrvs);
	TrimSrfsFromTrimPlsAux(TrimCrvs, Srf, 0);/* Free TrimCrvs on the fly.*/

	TopLevel -> Pnext = Pnext;
    }

    return GlblCntrTrimSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs a sequence of trimmed surfaces out of a given hierarchy of    *
* trimming curves.  Auxiliary function of  TrimSrfsFromTrimPlsHierarchy      *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimCrvsHierarchy:    Hierarchy of curves to trim Srf with.              *
*   Srf:                  Surface to trim.                                   *
*   Depth:                Nesting depths of loops.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:		                                                     *
*****************************************************************************/
static void TrimSrfsFromTrimPlsAux(TrimCrvStruct *TrimCrvsHierarchy,
				   const CagdSrfStruct *Srf,
				   int Depth)
{
    TrimCrvStruct *TrimLoop, *TrimSubLoops, *TrimSubLoopsCp;

    while (TrimCrvsHierarchy != NULL) {
	TrimSrfStruct *TSrf;

	IRIT_LIST_POP(TrimLoop, TrimCrvsHierarchy);

	if ((TrimSubLoops =
	     (TrimCrvStruct *) AttrGetPtrAttrib(TrimLoop -> Attr,
						"_subTrims")) != NULL) {
	    TrimSubLoopsCp = TrimCrvCopyList(TrimSubLoops);

	    AttrFreeOneAttribute(&TrimLoop -> Attr, "_subTrims");

	    TrimSrfsFromTrimPlsAux(TrimSubLoops, Srf, Depth + 1);
	}
	else
	    TrimSubLoopsCp = NULL;

	TrimLoop -> Pnext = TrimSubLoopsCp;
	if ((Depth & 0x01) == 0) {
	    TSrf = TrimSrfNew(CagdSrfCopy(Srf), TrimLoop, TRUE);

	    IRIT_LIST_PUSH(TSrf, GlblCntrTrimSrfs);
	}
	else {
	    TrimCrvFreeList(TrimLoop);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update every edge on the boundary to be fine enough.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Cntrs:   Contours to update, in place, so boundary edges are fine.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateBoundaryCntrEdges(IPPolygonStruct *Cntrs)
{
    CagdRType t,
	Dt = 10.0 / _TrimUVCrvApproxTolSamples;
    IPPolygonStruct *Pl;

    /* Update points along the boundaries so there are dense "enough". */
    for (Pl = Cntrs; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct 
	    *V = Pl -> PVertex;

	while (V -> Pnext != NULL) {
	    if (IS_BOUNDARY_VRTX(V) && IS_BOUNDARY_VRTX(V -> Pnext)) {
	        IrtRType
		    *P1 = V -> Coord,
		    *P2 = V -> Pnext -> Coord;

		for (t = Dt; t < 1.0 - Dt * 0.5; t += Dt) {
		    V -> Pnext = IPAllocVertex2(V -> Pnext);
		    V = V -> Pnext;
		    SET_BOUNDARY_VRTX(V);
		    IRIT_PT_BLEND(V -> Coord, P2, P1, t);
		}
	    }

	    V = V -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a list of linear Bspline curves constructed from given polylines.  *
* Adjust the trimming curves so that the importance of curves on the surface *
* boundary will get much more importance, parametrically.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Polys:    To convert to linear bspline curves.                           *
*   UMin, UMax, VMin, VMax:   Domain of surface whose Polys are to serve as  *
*			      trimming curves.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Linear Bspline curves representing Poly.               *
*****************************************************************************/
static CagdCrvStruct *TrimPolylines2LinBsplineCrvs(const IPPolygonStruct *Polys,
						   CagdRType UMin,
						   CagdRType UMax,
						   CagdRType VMin,
						   CagdRType VMax)
{
    CagdCrvStruct *Crv,
	*Crvs = UserPolylines2LinBsplineCrvs(Polys, TRUE);

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	int i;
	CagdRType
	    *PtsU = Crv -> Points[1],
	    *PtsV = Crv -> Points[2],
	    *KV = &Crv -> KnotVector[2],
	    ParamOffset = 0.0;

	for (i = Crv -> Length; i > 1; i--, PtsU++, PtsV++) {
	    if ((IRIT_APX_EQ(PtsU[0], UMin) ||
		 IRIT_APX_EQ(PtsU[0], UMax) ||
		 IRIT_APX_EQ(PtsV[0], VMin) ||
		 IRIT_APX_EQ(PtsV[0], VMax)) &&
		(IRIT_APX_EQ(PtsU[1], UMin) ||
		 IRIT_APX_EQ(PtsU[1], UMax) ||
		 IRIT_APX_EQ(PtsV[1], VMin) ||
		 IRIT_APX_EQ(PtsV[1], VMax)))
		ParamOffset += 1.0;
	    *KV++ += ParamOffset;
	}
	
	*KV++ += ParamOffset;
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a list of linear Bspline curves constructed from given polylines.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Polys:    Input polylines to convert to lienar bspline curves.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimCrvStruct *:   Linear Bspline curves representing Polys.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimPolylines2LinTrimCrvs                                                M
*****************************************************************************/
TrimCrvStruct *TrimPolylines2LinTrimCrvs(const IPPolygonStruct *Polys)
{
    return TrimPolylines2LinTrimCrvs2(Polys,
				      -IRIT_INFNTY, IRIT_INFNTY,
				      -IRIT_INFNTY, IRIT_INFNTY);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns a list of linear Bspline curves constructed from given polylines.  *
* Adjust the trimming curves so that the importance of curves on the surface *
* boundary will get much more importance, parametrically.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Polys:    To convert to linear bspline curves.                           *
*   UMin, UMax, VMin, VMax:   Domain of surface whose Polys are to serve as  *
*			      trimming curves.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrimCrvStruct *:  Linear Bspline curves representing Polys.              *
*****************************************************************************/
static TrimCrvStruct *TrimPolylines2LinTrimCrvs2(const IPPolygonStruct *Polys,
						 CagdRType UMin,
						 CagdRType UMax,
						 CagdRType VMin,
						 CagdRType VMax)
{
    CagdCrvStruct *Crv,
	*Crvs = TrimPolylines2LinBsplineCrvs(Polys, UMin, UMax, VMin, VMax);
    TrimCrvStruct *TCrv,
	*TCrvs = NULL;

    while (Crvs != NULL) {
	IRIT_LIST_POP(Crv, Crvs);

	TCrv = TrimCrvNew(TrimCrvSegNew(Crv, NULL));
	IRIT_LIST_PUSH(TCrv, TCrvs);
    }

    return TCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorts, in place, the open contours into order so we can clip the         *
* rectangular surface domain's boundary with.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   OpenCntrs:                To sort along the boundary.                    *
*   Domain:		      A polyline representing the current domain,    *
*			      starting from the entire surface's domain.     *
*   UMin, UMax, VMin, VMax:   Domain of surface.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:        Sorted, in place, list of contours.            *
*****************************************************************************/
static IPPolygonStruct *ClosedCntrsFromOpenCntrs(IPPolygonStruct *OpenCntrs,
						 IPPolygonStruct *Domain,
						 CagdRType UMin,
						 CagdRType UMax,
						 CagdRType VMin,
						 CagdRType VMax)
{
    IPPolygonStruct
	*InsideCntrs = NULL,
	*OutsideCntrs = NULL,
	*SubdivCntr = OpenCntrs;
    IPVertexStruct
	*VFirst = SubdivCntr -> PVertex,
	*VLast = IPGetLastVrtx(VFirst);
    CagdRType
        VFirstWeight = VertexWeight(VFirst, UMin, UMax, VMin, VMax),
        VLastWeight = VertexWeight(VLast, UMin, UMax, VMin, VMax);

    OpenCntrs = OpenCntrs -> Pnext;
    SubdivCntr -> Pnext = NULL;
    if (VFirstWeight > VLastWeight)
	IRIT_SWAP(CagdRType, VFirstWeight, VLastWeight);

    if (OpenCntrs != NULL) {
	IPPolygonStruct *LastCntr, *Cntrs1, *Cntrs2,
	    *InsideDomain, *OutsideDomain;

	/* Split all other open contours to contours inside weights' range   */
	/* assigned to the end points of SplitCntr and outside that range.   */
	while (OpenCntrs) {
	    IPPolygonStruct
	        *Cntr = OpenCntrs;
	    IPVertexStruct
	        *V2First = Cntr -> PVertex,
	        *V2Last = IPGetLastVrtx(V2First);
	    CagdRType
	        V2FirstWeight = VertexWeight(V2First, UMin, UMax, VMin, VMax),
	        V2LastWeight = VertexWeight(V2Last, UMin, UMax, VMin, VMax);

	    OpenCntrs = OpenCntrs -> Pnext;
	    Cntr -> Pnext = NULL;

	    if (V2FirstWeight > VFirstWeight && V2FirstWeight < VLastWeight &&
		V2LastWeight > VFirstWeight && V2LastWeight < VLastWeight) {
		IRIT_LIST_PUSH(Cntr, InsideCntrs);
	    }
	    else if ((V2FirstWeight < VFirstWeight ||
		      V2FirstWeight > VLastWeight) &&
		     (V2LastWeight < VFirstWeight ||
		      V2LastWeight > VLastWeight)) {
		IRIT_LIST_PUSH(Cntr, OutsideCntrs);
	    }
	    else {
		TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
	    }
	}

	/* Split Domain into two using SubdivCntr. */
#	ifdef DEBUG
	{
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintVertexList) {
	        IRIT_INFO_MSG("Original:\n");
		PrintVrtxList(Domain -> PVertex);
		IRIT_INFO_MSG("Subdiv:\n");
		PrintVrtxList(SubdivCntr -> PVertex);
	    }
	}
#	endif /* DEBUG */

	InsideDomain = SplitDomainByCntr(Domain, SubdivCntr,
					 UMin, UMax, VMin, VMax);
	OutsideDomain = InsideDomain -> Pnext;
	InsideDomain -> Pnext = NULL;

#	ifdef DEBUG
	{
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintVertexList) {
	        IRIT_INFO_MSG("Splitted:\n");
		PrintVrtxList(InsideDomain -> PVertex);
		PrintVrtxList(OutsideDomain -> PVertex);
	    }
	}
#	endif /* DEBUG */

	/* Do the two recursive calls, merge results, and return. */
	if (InsideCntrs == NULL)
	    Cntrs1 = InsideDomain;
	else {
	    Cntrs1 = ClosedCntrsFromOpenCntrs(InsideCntrs, InsideDomain,
					      UMin, UMax, VMin, VMax);
	    IPFreePolygon(InsideDomain);
	}

	if (OutsideCntrs == NULL)
	    Cntrs2 = OutsideDomain;
	else {
	    Cntrs2 = ClosedCntrsFromOpenCntrs(OutsideCntrs, OutsideDomain,
					      UMin, UMax, VMin, VMax);
	    IPFreePolygon(OutsideDomain);
	}

	LastCntr = IPGetLastPoly(Cntrs1);
	LastCntr -> Pnext = Cntrs2;
	return Cntrs1;
    }
    else {
	IPPolygonStruct *Cntrs;

	/* We have no other contour - split and return the two domains. */

#	ifdef DEBUG
	{
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintVertexList) {
	        PrintVrtxList(Domain -> PVertex);
		IRIT_INFO_MSG("Subdiv:\n");
		PrintVrtxList(SubdivCntr -> PVertex);
	    }
	}
#	endif /* DEBUG */

	Cntrs = SplitDomainByCntr(Domain, SubdivCntr, UMin, UMax, VMin, VMax);

#	ifdef DEBUG
	{
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintVertexList) {
	        IRIT_INFO_MSG("Splitted:\n");
		PrintVrtxList(Cntrs -> PVertex);
		PrintVrtxList(Cntrs -> Pnext -> PVertex);
	    }
	}
#	endif /* DEBUG */

	return Cntrs;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits into two the given Domain that represents the current trimmed     *
* surface domain, by the given open contour.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Domain:      A closed polyline of the current trimmed surface domain.    *
*   Cntr:        An open polyline that has its end points on the boundary of *
*		 Domain.						     *
*   UMin, UMax, VMin, VMax:   Domain of surface.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   Two sub domains, results of splitting Domain,       *
*		inside sub-domain first, followed by outside sub-domain.     *
*****************************************************************************/
static IPPolygonStruct *SplitDomainByCntr(IPPolygonStruct *Domain,
					  IPPolygonStruct *Cntr,
					  CagdRType UMin,
					  CagdRType UMax,
					  CagdRType VMin,
					  CagdRType VMax)
{
    CagdBType
	ReversedSubdivCntr = FALSE;
    IPVertexStruct *VDomain, *VDmnStart, *VDmnEnd,
	*VFirst = Cntr -> PVertex,
	*VLast = IPGetLastVrtx(VFirst);
    CagdRType
        VFirstWeight = VertexWeight(VFirst, UMin, UMax, VMin, VMax),
        VLastWeight = VertexWeight(VLast, UMin, UMax, VMin, VMax);

    Cntr -> PVertex = NULL;
    IPFreePolygon(Cntr);

    if (VFirstWeight > VLastWeight) {
	ReversedSubdivCntr = TRUE;
	IRIT_SWAP(CagdRType, VFirstWeight, VLastWeight);
    }

    /* Split Domain into two using SubdivCntr. */
    VDomain = IPCopyVertexList(Domain -> PVertex);
    VDmnEnd = IPGetLastVrtx(VDomain);
    if (IRIT_PT_APX_EQ_E2(VDmnEnd -> Coord, VDomain -> Coord)) {
        /* Same point - purge last point. */
        IPVertexStruct *V;

	if (VDomain == VDmnEnd)
	    TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
	else {
	    for (V = VDomain;
		 V -> Pnext != VDmnEnd || V -> Pnext == NULL;
		 V = V -> Pnext);
	    if (V == NULL)
	        TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
	    IPFreeVertex(VDmnEnd);
	    V -> Pnext = NULL;
	}
    }

    for (VDmnStart = VDomain;
	 VDmnStart -> Pnext != NULL &&
	 (!IS_BOUNDARY_VRTX(VDmnStart) ||
	  !IS_BOUNDARY_VRTX(VDmnStart -> Pnext) ||
	  !WeightRelationInside(VertexWeight(VDmnStart,
					     UMin, UMax, VMin, VMax),
				VertexWeight(VDmnStart -> Pnext,
					     UMin, UMax, VMin, VMax),
				VFirstWeight));
	 VDmnStart = VDmnStart -> Pnext);

    for (VDmnEnd = VDomain;
	 VDmnEnd -> Pnext != NULL &&
	 (!IS_BOUNDARY_VRTX(VDmnEnd) ||
	  !IS_BOUNDARY_VRTX(VDmnEnd -> Pnext) ||
	  !WeightRelationInside(VertexWeight(VDmnEnd,
					     UMin, UMax, VMin, VMax),
				VertexWeight(VDmnEnd -> Pnext,
					     UMin, UMax, VMin, VMax),
				VLastWeight));
	 VDmnEnd = VDmnEnd -> Pnext);

    /* Make into a cyclic list */
    IPGetLastVrtx(VDomain) -> Pnext = VDomain;

    if (VDmnStart -> Pnext == NULL ||
	VDmnEnd -> Pnext == NULL ||
	!IS_BOUNDARY_VRTX(VDmnStart) ||
	!IS_BOUNDARY_VRTX(VDmnStart -> Pnext) ||
	!IS_BOUNDARY_VRTX(VDmnEnd) ||
	!IS_BOUNDARY_VRTX(VDmnEnd -> Pnext)) {
	TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
	return NULL;
    }
    else {
	IPPolygonStruct *Pl, *InsideDomain, *OutsideDomain;
	IPVertexStruct *VLastCopy, *VDmnNext,
	    *VFirstCopy = IPCopyVertexList(VFirst);

	VFirstCopy = IPReverseVrtxList2(VFirstCopy);
	VLastCopy = IPGetLastVrtx(VFirstCopy);

	SET_BOUNDARY_VRTX(VFirst);
	SET_BOUNDARY_VRTX(VLast);
	SET_BOUNDARY_VRTX(VFirstCopy);
	SET_BOUNDARY_VRTX(VLastCopy);

	if (ReversedSubdivCntr) {
	    VDmnNext = VDmnEnd -> Pnext;

	    /* Chain the portion of Domain that is inside. */ 
	    if (VDmnStart != VDmnEnd) {
		VLast -> Pnext = VDmnStart -> Pnext;
		VLast = VDmnEnd;
		VDmnEnd -> Pnext = NULL;
	    }

	    /* Create the inside domain and duplicate last point as first. */
	    InsideDomain = IPAllocPolygon(0, VFirst, NULL);

	    /* Create the outsize domain. */
	    VLastCopy -> Pnext = VDmnNext;
	    VDmnStart -> Pnext = VFirstCopy;

	    OutsideDomain = IPAllocPolygon(0, VDmnStart, NULL);
	}
	else {
	    VDmnNext = VDmnEnd -> Pnext;

	    /* Chain the portion of Domain that is inside. */ 
	    if (VDmnStart != VDmnEnd) {
		VLastCopy -> Pnext = VDmnStart -> Pnext;
		VLastCopy = VDmnEnd;
		VDmnEnd -> Pnext = NULL;
	    }

	    /* Create the inside domain and duplicate last point as first. */
	    InsideDomain = IPAllocPolygon(0, VFirstCopy, NULL);

	    VLast -> Pnext = VDmnNext;
	    VDmnStart -> Pnext = VFirst;

	    OutsideDomain = IPAllocPolygon(0, VDmnStart, NULL);
	}

	InsideDomain -> Pnext = OutsideDomain;

	for (Pl = InsideDomain; Pl != NULL; Pl = Pl -> Pnext) {
	    /* Make sure polylines are not cyclic any more and duplicate     */
	    /* the last point as first.					     */
	    VLast = IPGetLastVrtx(Pl -> PVertex);
	    VLast -> Pnext = NULL;
	    if (!IRIT_PT_APX_EQ(VLast -> Coord, Pl -> PVertex -> Coord)) {
	        VLast -> Pnext = IPAllocVertex2(NULL);
		VLast = VLast -> Pnext;
		VLast -> Tags = Pl -> PVertex -> Tags;
		IRIT_PT_COPY(VLast -> Coord, Pl -> PVertex -> Coord);
	    }
	}

	return InsideDomain;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns TRUE iff V is between V1 and V2, going counter clockwise along   *
* the boundary of the parametric domain.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2:    The ordered end points to verify that V is inside.            *
*   V:                                                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*                                                                            *
*****************************************************************************/
static int WeightRelationInside(CagdRType V1, CagdRType V2, CagdRType V)
{
    if (V1 < V2) {
        return V < V2 && V > V1;
    }
    else {
        return (V > V1 && V < 4.0) || (V < V2 && V > 0.0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a unique weights according to the location where the vertex     *
* meets the boundary:							     *
*                 VMin is (0-1), UMax is (1-2), VMax is (2-3), UMin is (3-4) *
*                                                                            *
* PARAMETERS:                                                                *
*   V:			      To assign a weight.                            *
*   UMin, UMax, VMin, VMax:   Domain of surface.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Weight of V                                                 *
*****************************************************************************/
static CagdRType VertexWeight(IPVertexStruct *V,
			      CagdRType UMin,
			      CagdRType UMax,
			      CagdRType VMin,
			      CagdRType VMax)
{
    CagdRType r;

    if (IRIT_APX_EQ_EPS(V -> Coord[0], UMin, CONTOUR_EPS)) {
        r = 3.0 + (V -> Coord[1] - VMax) / (VMin - VMax);

	if (IRIT_APX_EQ_EPS(r, 4.0, CONTOUR_EPS) && V -> Pnext != NULL)
	    r = 0.0;
    }
    else if (IRIT_APX_EQ_EPS(V -> Coord[0], UMax, CONTOUR_EPS)) {
	r = 1.0 + (V -> Coord[1] - VMin) / (VMax - VMin);
    }
    else if (IRIT_APX_EQ_EPS(V -> Coord[1], VMin, CONTOUR_EPS)) {
	r = 0.0 + (V -> Coord[0] - UMin) / (UMax - UMin);
    }
    else if (IRIT_APX_EQ_EPS(V -> Coord[1], VMax, CONTOUR_EPS)) {
	r = 2.0 + (V -> Coord[0] - UMax) / (UMin - UMax);
    }
    else {
	TRIM_FATAL_ERROR(TRIM_ERR_INCONSISTENT_CNTRS);
	r = IRIT_INFNTY;
    }

    return r;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given vertex list, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:       The vertex list to be printed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintVrtxList(IPVertexStruct *V)
{
    IPVertexStruct
	*VHead = V;

    IRIT_INFO_MSG("\nPolyline:\n");

    do {
	IRIT_INFO_MSG_PRINTF("    %12f %12f %12f\n",
			     V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	V = V -> Pnext;
    }
    while (V!= NULL && V != VHead);
}
#endif /* DEBUG */
