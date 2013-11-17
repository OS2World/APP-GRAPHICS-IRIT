/******************************************************************************
* nc_tpath.c - synthesize CNC toolpath for a given geometry.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb 2007.					      *
******************************************************************************/

#include <ctype.h>

#include "irit_sm.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "bool_lib.h"
#include "cagd_lib.h"
#include "rndr_lib.h"
#include "geom_lib.h"
#include "user_loc.h"

#define USER_NCCTP_CNTR_ZBUF_SIZE	100000
#define USER_NCCTP_YFUDGE		1.301060e-6
#define USER_NCPTP_POCKET_EPS		1e-10
#define USER_NCPTP_SUBDIV_TOL		1e-3
#define USER_NCPTP_NUMER_TOL		USER_NCPTP_POCKET_EPS

/* We ensure here an accuracy of about 0.1 millimeter. */
#define UTIL_NCCTP_SRF_TOLERANCE      (Units == IP_NC_GCODE_UNIT_MM ? 0.1 \
								    : 0.004)
#define UTIL_NCPTP_CRV_TOLERANCE      (Units == IP_NC_GCODE_UNIT_MM ? 0.1 \
								    : 0.004)

#define USER_NCCTP_OFST_VRTX(Vrtx, Len) { \
	    IrtVecType V; \
	    IRIT_VEC_COPY(V, Vrtx -> Normal); \
	    IRIT_VEC_SCALE(V, Len); \
	    IRIT_PT_ADD(Vrtx -> Coord, Vrtx -> Coord, V); \
	}


#ifdef USER_NC_TPATH_MERGE_ALT
/* Two pllns are mergeable iff they are at adjacent rows. */
#define USER_NC_TEST_CLOSEST_DIST(V1, V2) { \
	IrtRType DstSqr; \
	if ((IRIT_FABS(V1[0] - V2[0]) > GlblUserNCTPathSpace * 0.5 || \
	     IRIT_FABS(V1[0] - V2[0]) < GlblUserNCTPathSpace * 1.5) && \
	    ((DstSqr = IRIT_PT_PT_DIST_SQR(V1, V2)) < MinSqr)) { \
	    MinSqr = DstSqr; \
	} \
    }

IRIT_STATIC_DATA IrtRType
    GlblUserNCTPathSpace = 1.0;
#endif /* USER_NC_TPATH_MERGE_ALT */

IRIT_STATIC_DATA IrtRType
    GlblUserNCCTPOffset = 0.0;
IRIT_STATIC_DATA CagdCrvStruct
    *GlblUserNCPocketAccumCrvs = NULL;
IRIT_STATIC_DATA IPPolygonStruct
    *GlblUserNCCTPPolys = NULL;
IRIT_STATIC_DATA IPObjectStruct
    *GlblUserNCPocketAccumPls = NULL;

static IPPolygonStruct *UserNCContourInterpolate(IRndrZBuffer1DPtrType
						                  NCCTPZBuf,
						 int MainFlip,
						 IPPolygonStruct *NewPl,
						 IPPolygonStruct *LastPl,
						 IPPolygonStruct *TPath,
						 const GMBBBboxStruct *BBox,
						 IrtRType Offset,
						 IrtRType MaxDepthStep);
static IPPolygonStruct *UserNC2ContourZDiff(IPPolygonStruct *NewPl,
					    IPPolygonStruct *LastPl,
					    const GMBBBboxStruct *BBox,
					    IrtRType MaxDepthStep);
static IrtRType UserNCGetContourZLevel(IrtRType x, IPVertexStruct *V);
static void UserNCCTPTesselateGeometry(IPObjectStruct *PObj,
				       IrtHmgnMatType Mat);
static void UserNCCTPSOffsetPolys(const IPObjectStruct *PObj);

#if defined(ultrix) && defined(mips)
static int UserNCCompPtsY(VoidPtr P1, VoidPtr P2);
#else
static int UserNCCompPtsY(const VoidPtr P1, const VoidPtr P2);
#endif /* ultrix && mips (no const support) */

#ifdef USER_NC_TPATH_MERGE_ALT
static IrtRType UserMergeGeomDistSqr2Polys(VoidPtr VPl1, VoidPtr VPl2);
static IPPolygonStruct *UserNCMergePolylines(IPPolygonStruct *Polys,
					     IrtRType TPathSpace);
#endif /*  USER_NC_TPATH_MERGE_ALT */

static void UserNCAccumulatePocketGeometry(IPObjectStruct *PObj,
					   IrtHmgnMatType Mat);
static void UserNCPTPComputeOffset(const IPObjectStruct *PObj,
				   IrtRType ToolRadius,
				   IrtRType RoughOffset,
				   IPObjectStruct **PToolOffsetObj,
				   IPObjectStruct **PRoughOffsettObj,
				   IrtRType OffsetTolerance,
				   int TrimSelfInter);
static CagdCrvStruct *UserNCOffsetCrvList(CagdCrvStruct *CrvList,
					  CagdRType OffsetTolerance,
					  CagdRType OffsetAmount,
					  CagdBType TrimSelfInters);
static CagdCrvStruct *UserNCOffsetCrvListAux(CagdCrvStruct *CrvList,
					     CagdRType OffsetTolerance,
					     CagdRType OffsetAmount,
					     CagdBType TrimSelfInters);
static IPObjectStruct *UserNCOffsetPlList(IPObjectStruct *PlList,
					  CagdRType OffsetTolerance,
					  CagdRType OffsetAmount,
					  CagdBType TrimSelfInters);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tool path to 3-axis machine from +Z direction the given         M
* geometry (Polygonal meshes/surfaces).  Assumes a ball end tool. Stages:    M
* 1. Tessellate the model into polygons.				     M
* 2. Verify the existence of (or create approximation thereof) of normal at  M
*    the vertices of the polygons, & offset all polygons by desired offset.  M
* 3. For each Y-section,  contour the model at the desired Y-constant level  M
*    and compute the upper envelop of all contoured line segments as this    M
*    Y-constant level as one toolpath.					     M
* 4. Emit the toolpath flipping every second Y-constant path to create a     M
*    zigzag motion, and move the tool path down by offset amount so it will  M
*    be referencing the ball-end tool's bottom.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Object to process and create 3-axis machining tool path for.M
*   Offset:      Tool radius to offset the geometry in PObj with. 	     M
*   ZBaseLevel:  Bottom level to machine.  Created tool path will never be   M
*	         below this level.					     M
*   TPathSpace:  Space between adjacent paths.  Also used as a tolerance     M
*		 bound.							     M
*   Units:       Millimeters or inches.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of constant X polylines, in zigzag motion, that M
*		covers PObj from above (offseted by Offset).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserNCPocketToolPath                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserNCContourToolPath                                                    M
*****************************************************************************/
IPObjectStruct *UserNCContourToolPath(const IPObjectStruct *PObj,
				      IrtRType Offset,
				      IrtRType ZBaseLevel,
				      IrtRType TPathSpace,
				      IPNCGCodeUnitType Units)
{
    const char
	*BBoxStr = AttrGetObjectStrAttrib(PObj, "NCCntrBBox");
    int OldCopyRefCount,
	Flip = FALSE;
    IrtRType y,
        MaxDepthStep = AttrGetObjectRealAttrib(PObj, "NCCntrMaxDepthStep");
    GMBBBboxStruct BBoxTmp,
        BBox = *GMBBComputeBboxObject(PObj);
    IrtHmgnMatType Mat;
    IPPolygonStruct *Pl,
        *LastPl = NULL,
	*TPath = NULL;
    IPObjectStruct *PObjPolys, *PObjPllns, *PTmp,
        *ClipObj = AttrGetObjectObjAttrib(PObj, "NCCntrClip");
    IRndrZBuffer1DPtrType NCCTPZBuf;

    if (BBoxStr != NULL &&
	(sscanf(BBoxStr, "%lf %lf %lf %lf %lf %lf",
		&BBoxTmp.Min[0], &BBoxTmp.Max[0],
		&BBoxTmp.Min[1], &BBoxTmp.Max[1],
		&BBoxTmp.Min[2], &BBoxTmp.Max[2]) == 6 ||
	 sscanf(BBoxStr, "%lf, %lf, %lf, %lf, %lf, %lf",
		&BBoxTmp.Min[0], &BBoxTmp.Max[0],
		&BBoxTmp.Min[1], &BBoxTmp.Max[1],
		&BBoxTmp.Min[2], &BBoxTmp.Max[2]) == 6)) {
        /* We have an explicitly requested bbox - use it. */
        BBox = BBoxTmp;

	if (!IRIT_APX_EQ(ZBaseLevel, BBoxTmp.Min[2]))
	    IRIT_WARNING_MSG_PRINTF("Warning: Z BaseLevel and BBox Z min are different but should be the same.\n");
    }
    else {
        BBox.Min[0] -= Offset;
	BBox.Min[1] -= Offset;
	BBox.Max[0] += Offset;
	BBox.Max[1] += Offset;
	BBox.Min[2] = ZBaseLevel;
    }

    if (IP_ATTR_IS_BAD_REAL(MaxDepthStep))
        MaxDepthStep = IRIT_INFNTY;

    GlblUserNCCTPOffset = Offset;
    NCCTPZBuf = IRndr1DInitialize(USER_NCCTP_CNTR_ZBUF_SIZE,
				  BBox.Min[0], BBox.Max[0],
				  BBox.Min[2], BBox.Max[2], FALSE);

    /* Stage 1 & 2 - tesselate geometry, verify normals and offset. */
    IRIT_ZAP_MEM(&IPFFCState, sizeof(IPFreeformConvStateStruct));
    IPFFCState.FineNess = UTIL_NCCTP_SRF_TOLERANCE;  /* Tol of tessellation. */
    IPFFCState.OptimalPolygons = TRUE;
    IPFFCState.ComputeNrml = TRUE;             /* Wants normals at vertices. */
    IPFFCState.FourPerFlat = TRUE;/* 4 polygons per ~flat patch, 2 otherwise.*/
    IPFFCState.LinearOnePolyFlag = TRUE;   /* Linear srf generates one poly. */

    /* This traversal convert geometry to polys in place, so use a copy. */
    MatGenUnitMat(Mat);
    OldCopyRefCount = IPSetCopyObjectReferenceCount(FALSE);
    PTmp = IPCopyObject(NULL, PObj, TRUE);
    GlblUserNCCTPPolys = NULL;
    IPTraverseObjHierarchy(PTmp, NULL, UserNCCTPTesselateGeometry, Mat, FALSE);
    IPFreeObject(PTmp);
    IPSetCopyObjectReferenceCount(OldCopyRefCount);

    if (GlblUserNCCTPPolys == NULL) {
        USER_FATAL_ERROR(USER_ERR_NC_INVALID_PARAM);
	return NULL;      
    }
    PObjPolys = IPGenPOLYObject(GlblUserNCCTPPolys);
    GlblUserNCCTPPolys = NULL;

    /* Stage 3 - copy contours at the proper Y steps and compute the upper  */
    /* envelope of all the line segments in each contouring step.	    */
    BooleanMultiCONTOUR(PObjPolys, 0.0, 1, TRUE, FALSE);	   /* Init. */

    for (y = BBox.Min[1]; y <= BBox.Max[1] + IRIT_EPS; y += TPathSpace) {
        if ((PObjPllns = BooleanMultiCONTOUR(NULL,		/* Contour. */
					     y + USER_NCCTP_YFUDGE,
					     1, FALSE, FALSE)) != NULL) {
	    IPPolygonStruct *Pl, *NewPl;
	    IPVertexStruct *V;

	    /* Compute the upper envelop using a 1D zbuffer. */
	    IRndr1DClearDepth(NCCTPZBuf, BBox.Min[2]);
	    for (Pl = PObjPllns -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	        IRndr1DPutPolyline(NCCTPZBuf, Pl);

	    IPFreeObject(PObjPllns);

	    Pl = IRndr1DUpperEnvAsPolyline(NCCTPZBuf, TRUE);
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext)
	        V -> Coord[1] = y;
	    NewPl = IPCopyPolygon(Pl);

	    if (MaxDepthStep != IRIT_INFNTY && LastPl != NULL)
	        TPath = UserNCContourInterpolate(NCCTPZBuf, Flip, Pl, LastPl,
						 TPath, &BBox, Offset,
						 MaxDepthStep);
	    else {
	        if (Flip)
		    Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);

	        AttrSetRGBColor(&Pl -> Attr, 255, 255, 10);
	        IRIT_LIST_PUSH(Pl, TPath);
	    }

	    if (LastPl != NULL)
	        IPFreePolygon(LastPl);
	    LastPl = NewPl;
	}

	Flip = !Flip;
    }
    BooleanMultiCONTOUR(NULL, 0.0, 1, FALSE, TRUE);		   /* Done. */
    TPath = IPReversePlList(TPath);

    /* Stage 4 - Move the toolapth down by Offset amount but make sure it   */
    /* does not go below ZBaseLevel.					    */
    for (Pl = TPath; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct *V;

	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    V -> Coord[2] -= Offset;
	    if (V -> Coord[2] < ZBaseLevel)
	        V -> Coord[2] = ZBaseLevel;
	}
    }
    /* Filters new collinear points, if any, due to ZBaseLevel adjustments. */
    TPath = GMCleanUpPolylineList2(TPath);

    /* Clip the toolpath to be inslide ClipObj in the XY plane. */
    if (ClipObj != NULL && IP_IS_POLY_OBJ(ClipObj)) {
        IPPolygonStruct
	    *ClipPl = ClipObj -> U.Pl;

        for (Pl = TPath; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct
		*PrevV = NULL,
	        *V = Pl -> PVertex;

	    /* If first vertex is out, skip until some inside pt is found.  */
	    if (!GMPolygonPointInclusion(ClipPl, V -> Coord)) {
	        while (Pl -> PVertex != NULL &&
		         !GMPolygonPointInclusion(ClipPl,
						  Pl -> PVertex -> Coord)) {
		    PrevV = Pl -> PVertex;
		    Pl -> PVertex = Pl -> PVertex -> Pnext;
		}

		PrevV -> Pnext = NULL;
		
		IPFreeVertexList(V);
	    }

	    if (Pl -> PVertex == NULL)
	        continue;

	    /* If we are here Pl holds at least one in vertex to begin with. */
	    for (PrevV = NULL, V = Pl -> PVertex;
		 V != NULL && GMPolygonPointInclusion(ClipPl, V -> Coord);
		 PrevV = V, V = V -> Pnext);

	    if (V != NULL) {
	        /* V is out.  Break Pl here and Pl -> Pnext will be cleaned  */
	        /* in the next iteration of this loop.			     */
	        Pl -> Pnext = IPAllocPolygon(0, V, Pl -> Pnext);
		PrevV -> Pnext = NULL;
	    }
	}

	/* Clean zero length polylines. */
	TPath = GMCleanUpPolylineList(&TPath, IRIT_EPS);
    }

    /* Free all used data sets. */
    IRndr1DDestroy(NCCTPZBuf);
    IPFreeObject(PObjPolys);

    PObjPllns = IPGenPolylineObject("NCTPath", TPath, NULL);

    AttrSetObjectIntAttrib(PObjPllns, "NCUnits",
			   Units == IP_NC_GCODE_UNIT_MM);

    return PObjPllns;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Interpolate intermediate plugning steps if the plunge from previous      *
* counter is larger than MaxDepthStep.  The interpolated steps are also      *
* appended to the toolpath.		                                     *
*   The NewPL will be prepended (with possible intermediate steps) to TPath  *
* and will be returned.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   NCCTPZBuf:    The 1D zbuffer we use.                                     *
*   Flip:	  TRUE to reverse NewPl, FALSE to add as is.                 *
*   NewPl:        New contour to add to TPath.				     *
*   LastPl:       A copy of previous (last) contour.                         *
*		  Modified in place.					     *
*   TPath:        All previous contours to prepend NewPl contour to.         *
*   BBox:         Of Z buffer built region.				     *
*   MaxDepthStep: Maximal depth to allow plunging from last step without     *
*                 intermediate interpolation steps.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   Tpath with NewPl prepended.                         *
*****************************************************************************/
static IPPolygonStruct *UserNCContourInterpolate(IRndrZBuffer1DPtrType
						                    NCCTPZBuf,
						 int MainFlip,
						 IPPolygonStruct *NewPl,
						 IPPolygonStruct *LastPl,
						 IPPolygonStruct *TPath,
						 const GMBBBboxStruct *BBox,
						 IrtRType Offset,
						 IrtRType MaxDepthStep)
{
    IPPolygonStruct *TooDeepPls;

    if ((TooDeepPls = UserNC2ContourZDiff(NewPl, LastPl,
					  BBox, MaxDepthStep)) != NULL) {
        /* Process intermediate toolpaths generated due to large plunging. */
        while (TooDeepPls != NULL) {
	    int i, Steps,
	        Flip = FALSE;
	    IrtRType Dz;
	    IPPolygonStruct *Pl, *BlendPl;
	    IPVertexStruct *V;

	    IRIT_LIST_POP(Pl, TooDeepPls);

	    /* Filters out too small of interpolated curves. */
	    if (IPGetLastVrtx(Pl -> PVertex) -> Coord[0] -
		Pl -> PVertex -> Coord[0] < Offset) {
	        IPFreePolygon(Pl);
		continue;
	    }

	    /* Figure out the maximal Z plunging. */
	    for (Dz = 0.0, V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	        if (Dz < V -> Coord[2] - V -> Coord[1])
		    Dz = V -> Coord[2] - V -> Coord[1];
	    }
	    assert(Dz > MaxDepthStep);
	    Steps = (int) (Dz / MaxDepthStep);

	    /* Blend Steps paths along Pl, in deeper Z values. */
	    for (i = 1; i <= Steps; i++) {
	        IrtRType
		    BlendFactor = i / (Steps + 1.0);

		if (i == Steps)
		    BlendPl = Pl;		    /* Use the Pl in place. */
		else
		    BlendPl = IPCopyPolygon(Pl);

		for (V = BlendPl -> PVertex; V != NULL; V = V -> Pnext) {
		    /* Compute the actual Z depth of this step. */
		    V -> Coord[2] = IRIT_BLEND(V -> Coord[1], V -> Coord[2],
					       BlendFactor);

		    /* And correct the Y value. */
		    V -> Coord[1] = NewPl -> PVertex -> Coord[1];
		}

		if (Flip)
		    BlendPl -> PVertex = IPReverseVrtxList2(BlendPl -> PVertex);
		Flip = !Flip;

		AttrSetRGBColor(&BlendPl -> Attr, 255, 10, 10);
		IRIT_LIST_PUSH(BlendPl, TPath);
	    }
	}
    }

    if (MainFlip)
        NewPl -> PVertex = IPReverseVrtxList2(NewPl -> PVertex);

    AttrSetRGBColor(&NewPl -> Attr, 255, 255, 10);
    IRIT_LIST_PUSH(NewPl, TPath);

    return TPath;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare the given two polylines for Z differences and extract the domain *
* were the Z difference is larger than the maximal plunging allowed, as      *
* prescribed by MaxDepthStep.                                                *
*   The returned polylines will follow the X values of NewPl/LastPl and will *
* store the Z difference (larger than MaxDepthStep) in the Y and Z axis.     *
*                                                                            *
* PARAMETERS:                                                                *
*   NewPl:         New poly we wish to verify it does not plunge too deeply. *
*   LastPl:        Previous poly we will compare depth against.		     *
*		   Modified in place.					     *
*   BBox:         of Z buffer built region.				     *
*   MaxDepthStep:  Maximal Z plunging allowed.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:     A list of polylines designating the regions       *
*                          of too deep of a plunge.  Each vertex in the      *
*                          returned polylines will hold the LastPl Z depth   *
*                          but also the respective NewPl Z in the Y axis.    *
*****************************************************************************/
static IPPolygonStruct *UserNC2ContourZDiff(IPPolygonStruct *NewPl,
					    IPPolygonStruct *LastPl,
					    const GMBBBboxStruct *BBox,
					    IrtRType MaxDepthStep)
{
    int i;
    IrtRType x,
        Dx = (BBox -> Max[0] - BBox -> Min[0]) / (USER_NCCTP_CNTR_ZBUF_SIZE - 1);
    IPVertexStruct
        *CrntV = NULL,
        *StartV = NULL,
        *NewV = NewPl -> PVertex,
        *LastV = LastPl -> PVertex;
    IPPolygonStruct *Pl,
        *RetList = NULL;

    /* Same X values and incremental. */
    assert(IRIT_APX_EQ(NewV -> Coord[0], LastV -> Coord[0]) &&
	   LastV -> Pnext -> Coord[0] - LastV -> Coord[0] > 0 &&
	   NewV -> Pnext -> Coord[0] - NewV -> Coord[0] > 0);

    /* Sample the data at the same resolution as we used for the Zbuffer. */
    for (i = 0, x = BBox -> Min[0];
	 i < USER_NCCTP_CNTR_ZBUF_SIZE;
	 i++, x += Dx) {
        IrtRType LastZ, NewZ;

        /* Make sure x is between this and next vertex for New/Last Pls. */
        if (LastV != NULL && x > LastV -> Pnext -> Coord[0])
	    LastV = LastV -> Pnext;
        if (NewV != NULL && x > NewV -> Pnext -> Coord[0])
	    NewV = NewV -> Pnext;

	if (LastV != NULL && LastV ->Pnext != NULL &&
	    NewV != NULL && NewV ->Pnext != NULL) {
	    LastZ = UserNCGetContourZLevel(x, LastV);
	    NewZ = UserNCGetContourZLevel(x, NewV);

	    if (LastZ - NewZ > MaxDepthStep) {
	        if (StartV == NULL)
		    CrntV = StartV = IPAllocVertex2(NULL);
		else {
		    CrntV -> Pnext = IPAllocVertex2(NULL);
		    CrntV = CrntV -> Pnext;
		}

		CrntV -> Coord[0] = x;
		CrntV -> Coord[1] = NewZ;
		CrntV -> Coord[2] = LastZ;
	    }
	    else if (StartV != NULL) {
	        /* Move this polyline to returned list. */
	        Pl = IPAllocPolygon(0, StartV, NULL);
		IRIT_LIST_PUSH(Pl, RetList);
		StartV = NULL;	        
	    }
	}
    }

    if (StartV != NULL) {
        /* Move this polyline to returned list. */
        Pl = IPAllocPolygon(0, StartV, NULL);
	IRIT_LIST_PUSH(Pl, RetList);
	StartV = NULL;	        
    }

    return IPReversePlList(RetList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the Z level at x, based on V and Vnext that are assumed to       *
* enclose the value of x.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   x:   X value to compute the Z level at.                                  *
*   V:   Vertex in polyline that (V, VNext) are assumed to hold x level.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    Linearly interpolated Z level.                              *
*****************************************************************************/
static IrtRType UserNCGetContourZLevel(IrtRType x, IPVertexStruct *V)
{
    IrtRType t;

    assert(x >= V -> Coord[0] && x <= V -> Pnext -> Coord[0]);

    t = (x - V -> Coord[0]) / (V -> Pnext -> Coord[0] - V -> Coord[0]);

    return IRIT_BLEND(V -> Pnext -> Coord[2], V -> Coord[2], t);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjHierarchy. Called on every non list   *
* object found in hierarchy.	                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserNCCTPTesselateGeometry(IPObjectStruct *PObj,
				       IrtHmgnMatType Mat)
{
    IPObjectStruct *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext)
        UserNCCTPSOffsetPolys(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Offset the geometry to both _off and -Off and place in global poly list. *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Geometry to offset and copy to global polygon list.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserNCCTPSOffsetPolys(const IPObjectStruct *PObj)
{
    int OldCirc = IPSetPolyListCirc(TRUE);
    IPObjectStruct *PTriObj, *PTmp;

    if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYGON_OBJ(PObj))
        return;

    /* Make sure all polygons are triangles - offset can make non triangle  */
    /* polygons non planar...						    */
    PTmp = IPCopyObject(NULL, PObj, TRUE);
    IPOpenPolysToClosed(PTmp -> U.Pl);
    PTriObj = GMConvertPolysToTriangles(PTmp);
    IPFreeObject(PTmp);

    /* Make sure we have vertex normals on all data. */
    GMBlendNormalsToVertices(PTriObj -> U.Pl, 180.0);

    /* Create offsets to both ways. */
    PTmp = IPGenPOLYObject(GMPolyOffset3D(PTriObj -> U.Pl,
					  GlblUserNCCTPOffset,
					  TRUE, TRUE, NULL));
    GlblUserNCCTPPolys = IPAppendPolyLists(GlblUserNCCTPPolys, PTmp -> U.Pl);
    PTmp -> U.Pl = NULL;
    IPFreeObject(PTmp);

    PTmp = IPGenPOLYObject(GMPolyOffset3D(PTriObj -> U.Pl,
					  -GlblUserNCCTPOffset,
					  TRUE, TRUE, NULL));
    GlblUserNCCTPPolys = IPAppendPolyLists(GlblUserNCCTPPolys, PTmp -> U.Pl);
    PTmp -> U.Pl = NULL;
    IPFreeObject(PTmp);

    IPFreeObject(PTriObj);

    IPSetPolyListCirc(OldCirc);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorting key function for qsort (see below).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:  Two point to sort out and compute their relative order.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    1, 0, -1 based on the relation between P1 and P2.                *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int UserNCCompPtsY(VoidPtr P1, VoidPtr P2)
#else
static int UserNCCompPtsY(const VoidPtr P1, const VoidPtr P2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
        V1 = (*((CagdPtStruct **) P1)) -> Pt[1],
    	V2 = (*((CagdPtStruct **) P2)) -> Pt[1];
    
    if (V1 < V2)
	return -1;
    else if (V1 > V2)
	return 1;
    else
	return 0;
}

#ifdef USER_NC_TPATH_MERGE_ALT

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes minimal distance squared between end vertices of two polylines. *
* Consider as valid only pllns that are at adjacent contouring rows.         *
*                                                                            *
* PARAMETERS:                                                                *
*   VPl1, VPl2:  To compute the minimal distance between the end points of.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   Minimal distance squared computed.			     *
*****************************************************************************/
static IrtRType UserMergeGeomDistSqr2Polys(VoidPtr VPl1, VoidPtr VPl2)
{
    IrtRType
	MinSqr = IRIT_INFNTY;
    IPPolygonStruct
	*Pl1 = (IPPolygonStruct *) VPl1,
	*Pl2 = (IPPolygonStruct *) VPl2;
    IrtRType
	*V1Start = Pl1 -> PVertex -> Coord,
	*V1End = IPGetLastVrtx(Pl1 -> PVertex) -> Coord,
        *V2Start = Pl2 -> PVertex -> Coord,
        *V2End = IPGetLastVrtx(Pl2 -> PVertex) -> Coord;

    USER_NC_TEST_CLOSEST_DIST(V1Start, V2Start);
    USER_NC_TEST_CLOSEST_DIST(V1End,   V2Start);
    USER_NC_TEST_CLOSEST_DIST(V1Start, V2End);
    USER_NC_TEST_CLOSEST_DIST(V1End,   V2End);

    return MinSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges separated tool path polylines into longer ones, in place, between *
* adjacent polylines (polylines of adjacent row levels).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Polys:       Polylines to merge, in place.                               *
*   TPathSpace:  Spacing between different rows of pocket contouring - we    *
*		 will merge only pllns from adjacent rows.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Merged as possible polylines.                        *
*****************************************************************************/
static IPPolygonStruct *UserNCMergePolylines(IPPolygonStruct *Polys,
					     IrtRType TPathSpace)
{
    int i,
	NumOfPolys = IPPolyListLen(Polys);
    IPPolygonStruct **PlsVec, *Pl;

    if (NumOfPolys < 2)
        return Polys;

    PlsVec = (IPPolygonStruct **) IritMalloc(sizeof(IPPolygonStruct *) *
					     NumOfPolys);
    for (i = 0, Pl = Polys; i < NumOfPolys; i++, Pl = Pl -> Pnext)
        PlsVec[i] = Pl;

    GlblUserNCTPathSpace = TPathSpace;
    NumOfPolys = GMMergeGeometry((void **) PlsVec, NumOfPolys, IRIT_INFNTY,
				 IRIT_UEPS,
				 NULL, UserMergeGeomDistSqr2Polys, NULL, NULL);

    for (i = 1, Pl = Polys = PlsVec[0]; i < NumOfPolys; i++) {
        Pl -> Pnext = PlsVec[i];
	Pl = PlsVec[i];
    }
    Pl -> Pnext = NULL;

    IritFree(PlsVec);

    return Polys;
}

#endif /* USER_NC_TPATH_MERGE_ALT */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tool path to 2D pocket machining from +Z direction the given    M
* geometry (curves/polylines).  Geometry is assumed to be closed, possibly   M
* with closed islands.  Stages:		                                     M
* 1. Offset the shape in for roughing, by RoughOffset (+ToolRadius) amount.  M
* 2. Compute the contour lines (intersection of parallel lines) with the     M
*    offseted-in geometry.						     M
* 3. Merge all contour lines into one large toolpath.  Toolpath is sorted so M
*    adjacent paths are close within tool diameter, as much as possible to   M
*    minimize retractions.						     M
* 4. Add a final, finish, path, at ToolRadius offset from the shape.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Object to process and create pocket machining tool path for.M
*   ToolRadius:  Tool radius to offset the geometry in PObj with. 	     M
*   RoughOffset: Offset amount to use in the roughing stage.		     M
*   TPathSpace:  Space between adjacent parallel cut rows. If zero returns   M
*		 just the offset geometry.		  		     M
*   TPathJoin:   maximal distance between end points of tpath to join.	     <
*   Units:       Milimeters or inches.					     M
*   TrimSelfInters:  TRUE to try and trim self intersections.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of constant X polylines, in zigzag motion, that M
*		covers PObj from above (offseted by Offset).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserNCContourToolPath                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserNCPocketToolPath                                                     M
*****************************************************************************/
IPObjectStruct *UserNCPocketToolPath(const IPObjectStruct *PObj,
				     IrtRType ToolRadius,
				     IrtRType RoughOffset,
				     IrtRType TPathSpace,
				     IrtRType TPathJoin,
				     IPNCGCodeUnitType Units,
				     int TrimSelfInters)
{
    int i;
    IrtRType x;
    GMBBBboxStruct BBox;
    IPObjectStruct *PToolOffsetObj, *PRoughOffsetObj, *RetObject, *PTmp;
    IPVertexStruct *V1, *V2;
    IPPolygonStruct *Pl,
	*Pllns = NULL;

    if (IRIT_FABS(ToolRadius) > IRIT_FABS(RoughOffset)) {
        USER_FATAL_ERROR(USER_ERR_NC_INVALID_PARAM);
	return NULL;
    }

    /* Stage 1. Compute the needed offsets. */
    UserNCPTPComputeOffset(PObj, ToolRadius, RoughOffset,
			   &PToolOffsetObj, &PRoughOffsetObj,
			   UTIL_NCPTP_CRV_TOLERANCE, TrimSelfInters);
    if (PToolOffsetObj == NULL || PRoughOffsetObj == NULL)
        return NULL; /* Had an error. */

    if (TPathSpace <= 0.0) {
        /* Return just the offsets. */
        RetObject = IPGenLISTObject(PRoughOffsetObj);
	IPListObjectInsert(RetObject, 1, PToolOffsetObj);
	IPListObjectInsert(RetObject, 2, NULL);

	return RetObject;
    }

    /* Stage 2. Compute the contour lines. */
    BBox = *GMBBComputeBboxObject(PRoughOffsetObj);

    if (IP_IS_CRV_OBJ(PRoughOffsetObj)) {
        int n;
        CagdCrvStruct *Crv,
	    *Crvs = PRoughOffsetObj -> U.Crvs;
	CagdPtStruct **PtsVec, *Pt;

        for (x = BBox.Min[0] - TPathSpace * 0.50000301060;
	     x < BBox.Max[0];
	     x += TPathSpace) {
	    CagdPtStruct *Pts,
	        *CrvsPts = NULL;
	    CagdLType Line;

	    Line[0] = -1.0;
	    Line[1] = 0.0;
	    Line[2] = x;

	    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
		Pts = SymbLclDistCrvLine(Crv, Line, USER_NCPTP_POCKET_EPS,
					 TRUE, FALSE);

		/* Compute Y value of each location.  Note monotone          */
		/* parameter values do not ensure monotone Euclidean values. */
		for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
		    CagdRType *R;
		    CagdPType PtE2;

		    R = CagdCrvEval(Crv, Pt -> Pt[0]);
		    CagdCoerceToE2(PtE2, &R, -1, Crv -> PType);
		    Pt -> Pt[1] = PtE2[1];
		}
		CrvsPts = CagdListAppend(Pts, CrvsPts);
	    }

	    if (((n = CagdListLength(CrvsPts)) & 0x01) != 0) {/*Must be even.*/
	        IPFreePolygonList(Pllns);
		CagdPtFreeList(CrvsPts);
		IPFreeObject(PRoughOffsetObj);
		IPFreeObject(PToolOffsetObj);

		USER_FATAL_ERROR(USER_ERR_NC_INVALID_INTER);
		return NULL;
	    }

	    if (n > 0) {
	        /* Sort the points based on the Y value. */
	        PtsVec = (CagdPtStruct **)
		    IritMalloc(sizeof(CagdPtStruct *) * n);
		for (i = 0, Pt = CrvsPts; Pt != NULL; Pt = Pt -> Pnext, i++)
		    PtsVec[i] = Pt;
		qsort(PtsVec, n, sizeof(CagdPtStruct *), UserNCCompPtsY);

		for (i = 0; i < n; i += 2) {
		    V2 = IPAllocVertex2(NULL);
		    V1 = IPAllocVertex2(V2);

		    V1 -> Coord[0] = x;
		    V1 -> Coord[1] = PtsVec[i] -> Pt[1];
		    V1 -> Coord[2] = 0.0;

		    V2 -> Coord[0] = x;
		    V2 -> Coord[1] = PtsVec[i + 1] -> Pt[1];
		    V2 -> Coord[2] = 0.0;

		    Pllns = IPAllocPolygon(0, V1, Pllns);
		}

		IritFree(PtsVec);
		CagdPtFreeList(CrvsPts);
	    }
	}
    }
    else if IP_IS_POLY_OBJ(PRoughOffsetObj) {
	if (IP_IS_POLYLINE_OBJ(PObj)) {
	    IPFreeObject(PRoughOffsetObj);
	    IPFreeObject(PToolOffsetObj);

	    USER_FATAL_ERROR(USER_ERR_NC_NO_POLYLINES);
	    return NULL;
	}

	PTmp = GMConvexPolyObjectN(PRoughOffsetObj);
	IPFreeObject(PRoughOffsetObj);
	PRoughOffsetObj = PTmp;

        for (x = BBox.Min[0] - TPathSpace * 0.50000301060;
	     x < BBox.Max[0];
	     x += TPathSpace) {
	    IPObjectStruct *CntrObj;
	    IrtPlnType Plane;

	    Plane[0] = -1.0;
	    Plane[1] = 0.0;
	    Plane[2] = 0.0;
	    Plane[3] = x;

	    if ((CntrObj = BooleanCONTOUR(PRoughOffsetObj, Plane)) != NULL) {
	        if (CntrObj -> U.Pl != NULL) {
		    IPGetLastPoly(CntrObj -> U.Pl) -> Pnext = Pllns;
		    Pllns = CntrObj -> U.Pl;
		    CntrObj -> U.Pl = NULL;
		}
		IPFreeObject(CntrObj);
	    }
	}
    }
    IPFreeObject(PRoughOffsetObj);
    GMCleanUpPolylineList2(Pllns);
    Pllns = IPReversePlList(Pllns);      /* So we are X min to X max again. */

#ifdef USER_NC_TPATH_MERGE_ALT
    /* Stage 3. Chain contours into one large path, according to closest    */
    /* end points of different contours.			            */
    for (Pl = Pllns, i = 0; Pl != NULL; Pl = Pl -> Pnext) {
        V1 = Pl -> PVertex;
	V2 = IPGetLastVrtx(V1);

        /* Place a marker on first and last vertex on each polyline so we   */
        /* couldfind merged locations and break the large polyline to       */
        /* pieces again.  Then we will have optimal toolpath, minimizing    */
        /* retraction/connections.					    */

	AttrSetIntAttrib(&V1 -> Attr, "NCEndPt", i);
	AttrSetIntAttrib(&V2 -> Attr, "NCEndPt", i++);
    }
    /* Merge all polylines into one long one! */
    Pl = UserNCMergePolylines(Pllns, TPathSpace);

    /* Break the polylines again at the "NCEndPt" locations. */
    {
        IPVertexStruct
	    *V = Pl -> PVertex;

	Pl -> PVertex = NULL;
	IPFreePolygon(Pl);
	for (Pllns = NULL; V != NULL; ) {
	    V2 = V -> Pnext;

	    if (V2 != NULL) {
	        for ( ; V2 != NULL; V2 = V2 -> Pnext) {
		    int n = AttrGetIntAttrib(V2 -> Attr, "NCEndPt");

		    if (!IP_ATTR_IS_BAD_INT(n))
		        break;
		}

		if (V2 != NULL) {
		    /* Found an "NCEndPt" - break plln here. */
		    V1 = V2 -> Pnext;
		    V2 -> Pnext = NULL;
		}
		else {
		    V1 = NULL;
		}
		Pl = IPAllocPolygon(0, V, Pllns);
		Pllns = Pl;
		V = V1;
	    }
	    else {
	        /* Only one vertex as last vertex - purge it. */
	      IPFreeVertex(V);
	      V = NULL;
	    }
	}
    }

    Pllns = IPReversePlList(Pllns);
#else
    /* Stage 3. Chain contours into one large path, according to closest    */
    /* end points of different contours.			            */
    for (Pl = Pllns; Pl != NULL; ) {
        IPVertexStruct *V2, *V2Last,
	    *V = IPGetLastVrtx(Pl -> PVertex);
	IPPolygonStruct *Pl2, *Pl2Prev,
	    *MinDistPlPrev = NULL;
	IrtRType d,
	    MinDist = TPathJoin;

        /* Go over all polylines in Pl next X-level and find the closest    */
	/* one to end point of Pl.  If end point if Pl is closer to end     */
	/* point in the found polyline, reverse it.			    */
	for (Pl2Prev = Pl, Pl2 = Pl -> Pnext;
	     Pl2 != NULL;
	     Pl2Prev = Pl2, Pl2 = Pl2 -> Pnext) {
	    V2 = Pl2 -> PVertex,
	    V2Last = IPGetLastVrtx(V2);

	    /* Lets see if we are too near or too far. */
	    if (V2 -> Coord[0] - V -> Coord[0] < TPathSpace * 0.5 ||
		V2 -> Coord[0] - V -> Coord[0] > TPathSpace * 1.5)
		continue;

	    /* We are in next X level (Dx is fixed and hence ignore it). */
	    d = IRIT_MIN(IRIT_FABS(V2 -> Coord[1] - V -> Coord[1]),
		    IRIT_FABS(V2Last -> Coord[1] - V -> Coord[1]));
	    if (d < MinDist) {
	        MinDist = d;
		MinDistPlPrev = Pl2Prev;
	    }
	}

	if (MinDistPlPrev != NULL) {
	    /* Found a polyline.  Merge it with Pl. */
	    Pl2 = MinDistPlPrev -> Pnext;
	    MinDistPlPrev -> Pnext = Pl2 -> Pnext;

	    /* Reverse the new polyline if needed. */
	    V2 = Pl2 -> PVertex;
	    V2Last = IPGetLastVrtx(V2);
	    if (IRIT_FABS(V2 -> Coord[1] - V -> Coord[1]) >
		IRIT_FABS(V2Last -> Coord[1] - V -> Coord[1])) {
	        V -> Pnext = IPReverseVrtxList2(V2);
	    }
	    else
	        V -> Pnext = Pl2 -> PVertex;

	    Pl2 -> PVertex = NULL;
	    IPFreePolygon(Pl2);
	}
	else
	     Pl = Pl -> Pnext;
    }
#endif  /* USER_NC_TPATH_MERGE_ALT */

    RetObject = IPGenLISTObject(IPGenPolylineObject("Rough", Pllns, NULL));
    IPListObjectInsert(RetObject, 1, PToolOffsetObj);
    IPListObjectInsert(RetObject, 2, NULL);

    AttrSetObjectIntAttrib(RetObject, "NCUnits",
			   Units == IP_NC_GCODE_UNIT_MM);

    return RetObject;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Accumulate all the geometry representing the pocket into one long list.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Next leaf to accumulate.                                         *
*   Mat:        Transformation matrix to apply to this object.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserNCAccumulatePocketGeometry(IPObjectStruct *PObj,
					   IrtHmgnMatType Mat)
{
    if (IP_IS_CRV_OBJ(PObj)) {
      CagdCrvStruct *Crv, *TCrv;

	/* Make sure all segements are C^1 continuous. */
	for (Crv = PObj -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) != 2) {
	        TCrv = CagdCoerceCrvTo(Crv,
				       CAGD_IS_RATIONAL_CRV(Crv) ?
				           CAGD_PT_P2_TYPE :
				           CAGD_PT_E2_TYPE,
				       FALSE);
	        GlblUserNCPocketAccumCrvs =
		    CagdListAppend(GlblUserNCPocketAccumCrvs,
				   CagdCrvSubdivAtAllC1Discont(TCrv));

		CagdCrvFree(TCrv);
	    }
	    else
	        GlblUserNCPocketAccumCrvs =
		    CagdListAppend(GlblUserNCPocketAccumCrvs,
				   CagdCrvSubdivAtAllC1Discont(Crv));
	}
    }
    else if (IP_IS_POLY_OBJ(PObj)) {
        GlblUserNCPocketAccumPls =
	    IPAppendObjLists(GlblUserNCPocketAccumPls,
			     IPCopyObject(NULL, PObj, TRUE));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes two offset, in the plane, of the given geometry (curves or      *
* polys), one by the tool radius and one a bit more, for roughing.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:             Object to offset.                                      *
*   ToolRadius:       Tool radius to offset the geometry in PObj with. 	     *
*   RoughOffset:      Offset amount to use in the roughing stage.	     *
*   PToolOffsetObj:   Offset result, by the tool radius.                     *
*   PRoughOffsettObj: Offset result, for roughing (more than tool radius).   *
*   OffsetTolerance:  Tolerance of approximation.                            *
*   TrimSelfInters:   TRUE to try and trim self intersections.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserNCPTPComputeOffset(const IPObjectStruct *PObj,
				   IrtRType ToolRadius,
				   IrtRType RoughOffset,
				   IPObjectStruct **PToolOffsetObj,
				   IPObjectStruct **PRoughOffsettObj,
				   IrtRType OffsetTolerance,
				   int TrimSelfInters)
{
    IrtHmgnMatType Mat;

    *PToolOffsetObj = *PRoughOffsettObj = NULL;

    MatGenUnitMat(Mat);
    GlblUserNCPocketAccumCrvs = NULL;
    GlblUserNCPocketAccumPls = NULL;

    /* PObj is still const-safe as UserNCAccumulatePocketGeometry is. */
    IPTraverseObjHierarchy((IPObjectStruct *) PObj, NULL,
			    UserNCAccumulatePocketGeometry, Mat, FALSE);

    if (GlblUserNCPocketAccumCrvs != NULL &&
	GlblUserNCPocketAccumPls != NULL) {
        CagdCrvFreeList(GlblUserNCPocketAccumCrvs);
	IPFreeObjectList(GlblUserNCPocketAccumPls);
	
        USER_FATAL_ERROR(USER_ERR_NC_MIX_CRVS_PLLNS);
	return;
    }

    /* Compute the offset of all geometry, now in a one long linked list. */
    if (GlblUserNCPocketAccumCrvs != NULL) {
	*PToolOffsetObj =
	    IPGenCRVObject(UserNCOffsetCrvList(GlblUserNCPocketAccumCrvs,
					       OffsetTolerance,
					       ToolRadius,
					       TrimSelfInters));
	*PRoughOffsettObj =
	    IPGenCRVObject(UserNCOffsetCrvList(GlblUserNCPocketAccumCrvs,
					       OffsetTolerance,
					       RoughOffset,
					       TrimSelfInters));
	
    }
    else if (GlblUserNCPocketAccumPls != NULL) {
        *PToolOffsetObj = UserNCOffsetPlList(GlblUserNCPocketAccumPls,
					     OffsetTolerance,
					     ToolRadius,
					     TrimSelfInters);
        *PRoughOffsettObj = UserNCOffsetPlList(GlblUserNCPocketAccumPls,
					       OffsetTolerance,
					       RoughOffset,
					       TrimSelfInters);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the end of Crv1 and the begining of Crv2 to connect after an      M
* offset operation that was applied to them.  There are a few options:       M
* 1. End of Crv1 is the same of start of Crv (if the two curves are C^1      M
*    connected).  Do nothing.						     M
* 2. The two curves intersects.  Trim away end of Crv1 and start of Crv2     M
*    upto the intersection location.  This case is handled assuming small    M
*    interaction between the two curves only, seeking a single intersection. M
*      Note old OfCrv1/2 are freed and being substituted in place with the   M
*    new trimmed versions.						     M
* 3. The two curves do not intersect.  Add a joint round curve that is C^1   M
*    to both Crv1's end and Crv2's start. The new joint round curve is       M
*    returned.								     M
*                                                                            M
* PARAMETERS:                                                                M
*   OrigCrv1:   To examine its end against Crv2, if the same.                M
*   OrigCrv2:   To examine its start against Crv1, if the same.              M
*   OffCrv1:    To properly update its end against Crv2, if needs to.        M
*   OffCrv2:    To properly update its start against Crv1, if needs to.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A new round curve bnetween OffCrv1 and OffCrv2 if not   M
*		     NULL.						     M
*		     Note also that OffCrv1/2 might be changed in place.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserNCUpdateCrvOffsetJoint                                               M
*****************************************************************************/
CagdCrvStruct *UserNCUpdateCrvOffsetJoint(CagdCrvStruct *OrigCrv1,
					  CagdCrvStruct *OrigCrv2,
					  CagdCrvStruct **OffCrv1,
					  CagdCrvStruct **OffCrv2)
{
    CagdRType TMin, TMax;
    CagdPType Pt1, Pt2;
    CagdPtStruct *Pts;

    if (OrigCrv1 == OrigCrv2)
	return NULL;

    /* Lets see of ends of OrigCrv1 is the same as start of OrigCrv2. */
    CagdCoerceToE3(Pt1, OrigCrv1 -> Points, OrigCrv1 -> Length - 1,
		   OrigCrv1 -> PType);
    CagdCoerceToE3(Pt2, OrigCrv2 -> Points, 0, OrigCrv2 -> PType);
    if (!IRIT_PT_APX_EQ(Pt1, Pt2))
        return NULL;

    /* If the offset end points remains the same, do nothing (case 1). */
    CagdCoerceToE3(Pt1, (*OffCrv1) -> Points, (*OffCrv1) -> Length - 1,
		   (*OffCrv1) -> PType);
    CagdCoerceToE3(Pt2, (*OffCrv2) -> Points, 0, (*OffCrv2) -> PType);
    if (IRIT_PT_APX_EQ(Pt1, Pt2))
        return NULL;

    /* Lets see if the two curves intersect. */
    if ((Pts = CagdCrvCrvInter(*OffCrv1, *OffCrv2,
			       USER_NCPTP_POCKET_EPS)) != NULL) {
        CagdCrvStruct *OffCrv1New, *OffCrv2New;

        /* Case 2 - curves intersect - trim intersection location. */
	CagdCrvDomain(*OffCrv1, &TMin, &TMax);
	OffCrv1New = CagdCrvRegionFromCrv(*OffCrv1, TMin, Pts -> Pt[0]);
	OffCrv1New -> Pnext = (*OffCrv1) -> Pnext;

	CagdCrvDomain(*OffCrv2, &TMin, &TMax);
	OffCrv2New = CagdCrvRegionFromCrv(*OffCrv2, Pts -> Pt[1], TMax);
	OffCrv2New -> Pnext = (*OffCrv2) -> Pnext;

	CagdCrvFree(*OffCrv1);
	*OffCrv1 = OffCrv1New;
	CagdCrvFree(*OffCrv2);
	*OffCrv2 = OffCrv2New;

	CagdPtFreeList(Pts);
    }
    else {
	int i;
        IrtRType t1, t2, **Pts;
        IrtPtType Inter1, Inter2;
        CagdVType Dir1, Dir2;
        CagdVecStruct *Tan;
        CagdCrvStruct *Crv;

        /* Case 3 - curves do not intersect - add a C^1 rounded corner. */
	CagdCrvDomain(*OffCrv1, &TMin, &TMax);
	Tan = CagdCrvTangent(*OffCrv1, TMax, TRUE);
	IRIT_VEC_COPY(Dir1, Tan -> Vec);

	CagdCrvDomain(*OffCrv2, &TMin, &TMax);
	Tan = CagdCrvTangent(*OffCrv2, TMin, TRUE);
	IRIT_VEC_COPY(Dir2, Tan -> Vec);

	if (GM2PointsFromLineLine(Pt1, Dir1, Pt2, Dir2,
				  Inter1, &t1, Inter2, &t2)) {
	    /* Add quadratic curve with Pt1, Inter1, Pt2 as control points. */
	    Crv = BzrCrvNew(3, CAGD_PT_P3_TYPE);
	    Pts = Crv -> Points;
	    Pts[0][0] = Pts[0][2] = 1.0;	     /* Update the weights. */
	    Pts[0][1] = cos(acos(IRIT_DOT_PROD(Dir1, Dir2)) / 2.0);
	    for (i = 0; i < 3; i++) {		     /* And control points. */
	        Pts[i + 1][0] = Pt1[i];
		Pts[i + 1][1] = Inter1[i] * Pts[0][1];
		Pts[i + 1][2] = Pt2[i];
	    }

	    return Crv;
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the offset of a list of curves and properly trim C^1 disconts.  *
*   This functin can handle several loops chained together.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvList:          List of curves to offset.                              *
*   OffsetTolerance:  Accuracy of offset computation.                        *
*   OffsetAmount:     Radius of offset.				             *
*   TrimSelfInters:   TRUE to try and trim self intersections.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   List of offset curves approximations.                 *
*****************************************************************************/
static CagdCrvStruct *UserNCOffsetCrvList(CagdCrvStruct *CrvList,
					  CagdRType OffsetTolerance,
					  CagdRType OffsetAmount,
					  CagdBType TrimSelfInters)
{
    if (CrvList -> Pnext == NULL) {
        /* Only one curve here - can only be a single loop... */
        return UserNCOffsetCrvListAux(CrvList, OffsetTolerance,
				      OffsetAmount, TrimSelfInters);
    }
    else {
        CagdPType PStart, PEnd;
        CagdCrvStruct *CrvOffsets, *Crv,
	    *CrvStart = NULL,
	    *AllCrvOffsets = NULL;

        /* Detect end of loop/curve when end point of one curve is in a   */
        /* different position than the start point of the next curve.     */
        for (Crv = CrvStart = CrvList;
	     Crv -> Pnext != NULL;
	     Crv = Crv -> Pnext) {
	    CagdCrvStruct
	        *CNext = Crv -> Pnext;

	    CagdCoerceToE3(PEnd, Crv -> Points, Crv -> Length - 1,
			   Crv -> PType);
	    CagdCoerceToE3(PStart, CNext -> Points, 0, CNext -> PType);
	    if (!IRIT_PT_APX_EQ(PEnd, PStart)) {
	        /* Crv is the last curve segment of this loop. */
	        Crv -> Pnext = NULL;

		/* Compute offset for this curve/loop. */
		CrvOffsets = UserNCOffsetCrvListAux(CrvStart,
						    OffsetTolerance,
						    OffsetAmount,
						    TrimSelfInters);
		AllCrvOffsets = CagdListAppend(AllCrvOffsets, CrvOffsets);

		Crv -> Pnext = CrvStart = CNext;
	    }
	}

	/* Handle last curve/loop. */
	if (CrvStart != NULL) {
	    CrvOffsets = UserNCOffsetCrvListAux(CrvStart,
						OffsetTolerance,
						OffsetAmount,
						TrimSelfInters);
	    AllCrvOffsets = CagdListAppend(AllCrvOffsets, CrvOffsets);
	}

	return AllCrvOffsets;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the offset of a list of curves and properly trim C^1 disconts.  *
*   The list is assumed to hold one closed loop.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvList:          List of curves to offset.                              *
*   OffsetTolerance:  Accuracy of offset computation.                        *
*   OffsetAmount:     Radius of offset.				             *
*   TrimSelfInters:   TRUE to try and trim self intersections.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   List of offset curves approximations.                 *
*****************************************************************************/
static CagdCrvStruct *UserNCOffsetCrvListAux(CagdCrvStruct *CrvList,
					     CagdRType OffsetTolerance,
					     CagdRType OffsetAmount,
					     CagdBType TrimSelfInters)
{
    int i, n;
    CagdCrvStruct *Crv, *TCrv, **CrvOffVec,
	*CrvOffList = NULL;

    for (Crv = CrvList;  Crv != NULL; Crv = Crv -> Pnext) {
        CagdCrvStruct
	    *CrvOff = SymbCrvSubdivOffset(Crv, OffsetAmount,
					  OffsetTolerance, FALSE);
    
	if (TrimSelfInters) {
            CagdCrvStruct
		*CrvOffTrimmed = MvarCrvTrimGlblOffsetSelfInter(
				         Crv, CrvOff,
				         OffsetAmount - OffsetTolerance * 10.0,
					 USER_NCPTP_SUBDIV_TOL,
					 USER_NCPTP_NUMER_TOL);

	    CagdCrvFree(CrvOff);
	    CrvOff = CrvOffTrimmed;
	}

	for (TCrv = CrvOff; TCrv != NULL; TCrv = TCrv -> Pnext)
	    AttrSetPtrAttrib(&TCrv -> Attr, "_OrigCrv", Crv);

	CrvOffList = CagdListAppend(CrvOffList, CrvOff);
    }

    /* Convert the linked list of curves into a vector. */
    CrvOffVec = (CagdCrvStruct **)
        IritMalloc(sizeof(CagdRType *) * (n = CagdListLength(CrvOffList)));
    for (i = 0, TCrv = CrvOffList; TCrv != NULL; TCrv = TCrv -> Pnext)
        CrvOffVec[i++] = TCrv;
    assert(i == n);
    for (i = 0; i < n; )
        CrvOffVec[i++] -> Pnext = NULL;

    /* Now trim/round joints between adjacent curves that are C^1 discont. */
    for (i = 0; i < n; i++) {
        CagdCrvStruct
	    *CrvOrig1 = AttrGetPtrAttrib(CrvOffVec[i] -> Attr, "_OrigCrv"),
	    *CrvOrig2 = AttrGetPtrAttrib(CrvOffVec[(i + 1) % n] -> Attr,
					 "_OrigCrv");

        /* Update connection between adjacent offset segement, if not C^1. */
        if ((TCrv = UserNCUpdateCrvOffsetJoint(CrvOrig1, CrvOrig2,
			   &CrvOffVec[i], &CrvOffVec[(i + 1) % n])) != NULL) {
	    CrvOffVec[i] -> Pnext = TCrv;     /* We have a rounded corner. */
	}

	/* In case we change that - will be used again in next iteration. */
	AttrSetPtrAttrib(&CrvOffVec[i] -> Attr, "_OrigCrv", CrvOrig1);
	AttrSetPtrAttrib(&CrvOffVec[(i + 1) % n] -> Attr, "_OrigCrv",
			 CrvOrig2);
    }

    /* Convert back into a linked list of curves.  Note we might have more */
    /* than one curve in a vector slot now!				   */
    for (i = 0, CrvOffList = NULL; i < n; i++)
        CrvOffList = CagdListAppend(CrvOffList, CrvOffVec[i]);

    IritFree(CrvOffVec);

    return CrvOffList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the offset of a list of polys and properly trim C^1 disconts.   *
*                                                                            *
* PARAMETERS:                                                                *
*   PlList:           List of polys to offset.                               *
*   OffsetTolerance:  Accuracy of offset computation.                        *
*   OffsetAmount:     Radius of offset.				             *
*   TrimSelfInters:   TRUE to try and trim self intersections.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  List of offset polys.                                 *
*****************************************************************************/
static IPObjectStruct *UserNCOffsetPlList(IPObjectStruct *PlList,
					  CagdRType OffsetTolerance,
					  CagdRType OffsetAmount,
					  CagdBType TrimSelfInters)
{
    IPObjectStruct *PObj,
	*POffObj = IP_IS_POLYGON_OBJ(PlList) ? IPGenPOLYObject(NULL)
					     : IPGenPOLYLINEObject(NULL);

    for (PObj = PlList; PObj != NULL; PObj = PObj -> Pnext) {
        IPPolygonStruct *Pl, *PlOff,
	    *PlOffList = NULL;

	for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    PlOff = GMPolyOffset(Pl, IP_IS_POLYGON_OBJ(PObj),
				 OffsetAmount, NULL);

	    IRIT_LIST_PUSH(PlOff, PlOffList);
	}

	/* If more than one poly, it is a poly with holes.  Merge holes in. */
	if (PlOffList -> Pnext != NULL) {
	    PlOffList = IPReversePlList(PlOffList);
	    IRIT_LIST_POP(PlOff, PlOffList);
	    Pl = GMPolyHierarchy2SimplePoly(PlOff, PlOffList);
	    IPFreePolygon(PlOff);
	    IPFreePolygonList(PlOffList);
	    PlOffList = Pl;
	}

	POffObj -> U.Pl = IPAppendPolyLists(POffObj -> U.Pl, PlOffList);
    }

    return POffObj;
}
