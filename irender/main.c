/*****************************************************************************
* Entry point of the program. Configuration and global data variables.       *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/
#include <irit_sm.h>
#include <misc_lib.h>
#include <geom_lib.h>
#include <rndr_lib.h>
#include "parser.h"
#include "config.h"

#define IREND_POINT_AS_SPHERE_RES	25

/* Contains all configuration options. Subject to change by config file and  */
/* and command line parameters.                                              */
IRIT_GLOBAL_DATA GlobalOptionsStruct Options;
IRIT_STATIC_DATA IrtRType GlblLastTime,
    GlblZMin = 0,
    GlblZMax = 0;
 
static void TakeTime(const char *msg);
static void ReverseVertexNormal(IPVertexStruct *v);
static void ReversePlaneNormal(IPPolygonStruct *Poly);
static IPObjectStruct *IObjectTriangulate(IPObjectStruct *PObj);
static void ComputeZDepth(IPVertexStruct *Vertex);
static void IObjectClip(IPObjectStruct *Object, IrtPlnType *ClippingPlanes);
static void ScanObjects(IRndrPtrType Rend,
			IPObjectStruct *Object,
			IrtBType DoClipping,
			IrtHmgnMatType ViewMat,
			IPObjectStruct **VisMapTriangles);
static void RenderPolyline(IRndrPtrType Rend,
			   IPPolygonStruct *Pl,
			   IrtRType Width);
static void DrawSilBndryEdges(IRndrPtrType Rend,
			      IPObjectStruct *PObjs,
			      IrtHmgnMatType ViewMat,
			      int NoShading);
static void LightListLoad(IRndrPtrType Rend, IPObjectStruct* Objects);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs configuration management, pre-processes data files and renders. M
*                                                                            *
* PARAMETERS:                                                                M
*   argc:    IN, command line parameters number.                             M
*   argv:    IN, command line parameters.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Exit reason code.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char *argv[])
{
    IPObjectStruct *Objects, *VisMapTriangles;
    char *BaseDirectory;
    IRndrPtrType Rend;
    IrtBType DoClipping;
    IrtHmgnMatType ViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    DoClipping = TRUE;
    GlblLastTime = IritCPUTime(FALSE);
    InitOptions();
    GetConfig(argv);
    GetOptions(argc, argv);
    BaseDirectory = argv[0];

    if (Options.VisMap)
        IPSurface2PolygonsGenDegenPolys(TRUE);

    Objects = ParseFiles(Options.NFiles, Options.Files,
			 Options.HasTime, Options.Time);

    Rend = IRndrInitialize(Options.XSize,
			   Options.YSize,
			   Options.FilterName ? 3 : 1,
			   Options.NPRClrQuant,
			   Options.Transp,
			   Options.BackFace,
			   Options.BackGround,
			   Options.Ambient,
			   Options.VisMap);

    if (Options.Stencil)
        IRndrClearStencil(Rend);

    IRndrSetShadeModel(Rend, (IRndrShadingType) Options.ShadeModel);
    IRndrSetViewPrsp(Rend,
		     IPWasViewMat ? IPViewMat : NULL,
		     IPWasPrspMat ? IPPrspMat : NULL,
		     NULL);
    if (IPWasViewMat)
        IRIT_HMGN_MAT_COPY(ViewMat, IPViewMat);
    else
        MatGenUnitMat(ViewMat);
    if (IPWasPrspMat)
        MatMultTwo4by4(ViewMat, ViewMat, IPPrspMat);

    if (Options.NormalReverse) {
        IPForEachVertex(Objects, ReverseVertexNormal);
        IPForEachPoly(Objects, ReversePlaneNormal);
    }

    if (Options.ZNear != -IRIT_INFNTY || Options.ZFar != -IRIT_INFNTY) {
        IRndrSetZBounds(Rend, Options.ZNear, Options.ZFar);
    }
    else {
        /* Default: no clipping. */
        DoClipping = FALSE;
    }
    if (Options.PllMaxW != Options.PllMinW) {
        GlblZMin = FAREST_Z;
	GlblZMax = NEAREST_Z;
	/* Used for polylines width computation. */
	IPForEachVertex(Objects, ComputeZDepth);
    }
	
    if (Options.VisMap) {
        IRndrVisMapPrepareUVValuesOfGeoObj(Objects, Options.XSize, 
            Options.YSize);
        IRndrVisMapEnable(Rend, Objects, Options.FilterName ? 3 : 1);   
    }

    TakeTime("Parsing and transform");

    IRndrSetFilter(Rend, Options.FilterName);
    LightListLoad(Rend, Objects);

    ScanObjects(Rend, Objects, DoClipping, ViewMat, &VisMapTriangles);

    if (Options.VisMap) {
        IRndrVisMapScan(Rend);
        IPFreeObject(VisMapTriangles);
    }

    TakeTime("Scan conversion");

    if (Options.ZDepth) {
        IRndrSaveFileDepth(Rend, BaseDirectory,
			   Options.OutFileName, Options.FileType);
    }
    else if (Options.Stencil) {
        IRndrSaveFileStencil(Rend, BaseDirectory,
			     Options.OutFileName, Options.FileType);
    }
    else if (Options.VisMap) {
        IRndrSaveFileVisMap(Rend, BaseDirectory, Options.OutFileName,
			    Options.FileType);
    }

    else {
        IRndrSaveFile(Rend, BaseDirectory,
		      Options.OutFileName, Options.FileType);
    }
    IRndrDestroy(Rend);
    TakeTime("Z-Buffer dump");

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the time the last action took.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   msg:       IN, the name of the action.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TakeTime(const char *Msg)
{
    IrtRType
        Time = IritCPUTime(FALSE);

    if (Options.Verbose)
        IRIT_INFO_MSG_PRINTF("\nirender: '%s' done in %0.2f s",
		             Msg, Time - GlblLastTime);

    GlblLastTime = Time;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Set vertex normal in opposite direction.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   v:       IN OUT, pointer to vertex object.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReverseVertexNormal(IPVertexStruct *v)
{
    IRIT_PT_SCALE(v -> Normal, -1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets plane equation to have normal defined in opposite direction.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:       IN OUT, pointer to the polygon.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReversePlaneNormal(IPPolygonStruct *Poly)
{
    IRIT_PT_SCALE(Poly -> Plane, -1.0);
    Poly -> Plane[3] = -Poly -> Plane[3];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tringulates an Irit object.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       IN OUT, pointer to the polygon.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct* : the result object.                                     *
*****************************************************************************/
static IPObjectStruct *IObjectTriangulate(IPObjectStruct *PObj)
{
    int OldCirc;
    IPObjectStruct *Result,
        *PTmp = IPCopyObject(NULL, PObj, TRUE);

    /* Make sure all polygons are convex. */
    OldCirc = IPSetPolyListCirc(TRUE);
    IPOpenPolysToClosed(PTmp -> U.Pl);
    GMConvexPolyObject(PTmp);
    IPSetPolyListCirc(OldCirc);
    IPClosedPolysToOpen(PTmp -> U.Pl);

    /* Make sure all polygons are triangle. */
    Result = GMConvertPolysToTriangles(PTmp);
    Result -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IPFreeObject(PTmp);
    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A helper callback function to compute the scene min and max depth.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Vertex:       IN, pointer to the vertex.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ComputeZDepth(IPVertexStruct *Vertex)
{
    IrtPtType Result;

    IRIT_PT_COPY(Result, Vertex -> Coord);
    if (IPWasViewMat)
        MatMultPtby4by4(Result, Result, IPViewMat);
    if (IPWasPrspMat)
        MatMultPtby4by4(Result, Result, IPPrspMat);

    if (Result[Z_AXIS] NEAR_THAN GlblZMin)
        GlblZMin = Result[Z_AXIS];
    if (Result[Z_AXIS] FARTHER_THAN GlblZMax)
        GlblZMax = Result[Z_AXIS];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Iterates the object list and clips out partially visible polygons.       *
*   Warning : very simplistic and naive.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Object:           IN, pointer to the object list.                        *
*   ClippingPlanes:   IN, pointer to the 6 clipping plane.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IObjectClip(IPObjectStruct *Object, IrtPlnType *ClippingPlanes)
{
    if (IP_IS_POLY_OBJ(Object) && !IP_IS_POLYLINE_OBJ(Object)) {
        IPPolygonStruct *PClipped, *PInter, *Pl;
	IPObjectStruct *PObj;

	/* ClippingPlanes[4] is important - it refers to ZClip near the eye. */
	Object -> U.Pl = GMClipPolysAgainstPlane(Object -> U.Pl,
						 &PClipped, &PInter,
						 ClippingPlanes[4]);

	IPFreePolygonList(PClipped);

	/* Clip the intersecting polygons. */
	while (PInter != NULL) {
	    IRIT_LIST_POP(Pl, PInter);

	    if (GMSplitPolygonAtPlane(Pl, ClippingPlanes[4])) {
		if (Pl -> Pnext)
		    IPFreePolygon(Pl -> Pnext);
		IRIT_LIST_PUSH(Pl, Object -> U.Pl);
	    }
	    else
	        IPFreePolygon(Pl);
	}

	/* Make sure we still have only triangles. */
	PObj = IObjectTriangulate(Object);
	IPFreePolygonList(Object -> U.Pl);
	Object -> U.Pl = PObj -> U.Pl;
	PObj -> U.Pl = NULL;
	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Iterates the object list and scans all polygons of them.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:       IN, the render context.                                      *
*   Object:     IN, pointer to the object list.                              *
*   DoClipping: IN, TRUE for clipping.			                     *
*   ViewMat:    IN, View from which objects are drawn.                       *
*   VisMapTriangles: OUT, will hold all the triangles the covers the vismap. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScanObjects(IRndrPtrType Rend,
			IPObjectStruct *Object,
			IrtBType DoClipping,
			IrtHmgnMatType ViewMat,
			IPObjectStruct **VisMapTriangles)
{
    IPPolygonStruct *Poly;
    IPObjectStruct *Next, *ObjectTri;
    IrtPlnType ClipPlanes[6];

    IRndrGetClippingPlanes(Rend, ClipPlanes);

    *VisMapTriangles = Options.VisMap ? IPGenPOLYObject(NULL) : NULL;

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintClipPlanes, FALSE) {
	    int i;

	    for (i = 0; i < 6; i++) {
	        IRIT_INFO_MSG_PRINTF("Plane(%d) = %8.6f  %8.6f  %8.6f  %8.6f\n", i,
			ClipPlanes[i][0], ClipPlanes[i][1], ClipPlanes[i][2],
			ClipPlanes[i][3]);
	    }
	}
    }
#endif /* DEBUG */

    for ( ; Object; Object = Next) {
        Next = Object -> Pnext;

	/* Convert to point list to individual point objects' list and push  */
	/* all points but one point that will replace Object.		     */
        if (IP_IS_POLY_OBJ(Object) && IP_IS_POINTLIST_OBJ(Object)) {
	    IPPolygonStruct *PtList;
	    IPObjectStruct *PTmp,
		*PPoint = NULL;

	    for (PtList = Object -> U.Pl;
		 PtList != NULL;
		 PtList = PtList -> Pnext) {
	        IPVertexStruct *V;

		for (V = PtList -> PVertex; V != NULL; V = V -> Pnext) {
		    IrtRType
		        *R = V -> Coord;

		    PTmp = IPGenPTObject(&R[0], &R[1], &R[2]);
		    PTmp -> Attr = IP_ATTR_COPY_ATTRS(Object -> Attr);

		    if (PPoint == NULL)
		        PPoint = PTmp;
		    else
		        IRIT_LIST_PUSH(PTmp, Next);
		}
	    }

	    IPFreeObject(Object);
	    Object = PPoint;
	}
    
	if (Options.VisMap) { 
	    if (ATTR_OBJ_ATTR_EXIST(Object, "tan_angle")) {
	        IrtRType
		    CosAng = AttrGetObjectRealAttrib(Object, "tan_angle");

		IRndrVisMapSetTanAngle(Rend, CosAng);
	    }
	    if (ATTR_OBJ_ATTR_EXIST(Object, "critic_ar")) {
	        IrtRType
		    CriticAR = AttrGetObjectRealAttrib(Object, "critic_ar");

		IRndrVisMapSetCriticAR(Rend, CriticAR);
	    }
	    if (ATTR_OBJ_ATTR_EXIST(Object, "uv_dilation")) {
	        int UvDilation = AttrGetObjectIntAttrib(Object, "uv_dilation");

		IRndrVisMapSetDilation(Rend, UvDilation);
	    }
	}

	/* If we are to render points - convert to a small polygonal sphere. */
	if (Options.DrawPoints &&
	    !ATTR_OBJ_ATTR_EXIST(Object, "light_source") &&
	    (IP_IS_POINT_OBJ(Object) ||
	     IP_IS_CTLPT_OBJ(Object) ||
	     IP_IS_VEC_OBJ(Object))) {
	    int OldRes,
		IsVec = IP_IS_VEC_OBJ(Object);
	    IrtRType
	        Rad = AttrGetObjectRealAttrib(Object, "width");
	    IPObjectStruct *SpObj,
	        *PtObj = IPCoerceObjectTo(Object, IP_OBJ_POINT);

	    if (IP_ATTR_IS_BAD_REAL(Rad))
	        Rad = Options.PointDfltRadius;

	    OldRes = PrimSetResolution(IREND_POINT_AS_SPHERE_RES);
	    SpObj = PrimGenSPHEREObject(PtObj -> U.Pt, Rad);
	    PrimSetResolution(OldRes);

	    IP_ATTR_FREE_ATTRS(SpObj -> Attr);
	    SpObj -> Attr = IP_ATTR_COPY_ATTRS(Object -> Attr);

	    /* If vector, also push a polyline from origin to this location. */
	    if (IsVec) {
	        IPVertexStruct
		    *V2 = IPAllocVertex2(NULL),
		    *V1 = IPAllocVertex2(V2);
		IPObjectStruct
		    *PllnObj = IPGenPOLYLINEObject(IPAllocPolygon(0, V1, NULL));

		IRIT_PT_RESET(V1 -> Coord);
		IRIT_PT_COPY(V2 -> Coord, PtObj -> U.Pt);

		PllnObj -> Attr = IP_ATTR_COPY_ATTRS(Object -> Attr);

		IRIT_LIST_PUSH(PllnObj, Next);
	    }

	    IPFreeObject(PtObj);

	    IPFreeObject(Object);
	    Object = SpObj;
	}

        if (IP_IS_POLY_OBJ(Object)) {
            int HasSilWidth = FALSE,
		HasSilColor = FALSE,
		IsPolyLine = IP_IS_POLYLINE_OBJ(Object);
	    IrtRType
	        Width = AttrGetObjectWidth(Object),
		SilWidth = AttrGetObjectRealAttrib(Object, "SilWidth");
	    IrtPtType OrigSilColor;

	    if (!IP_ATTR_IS_BAD_REAL(SilWidth)) {
	        const char *SilRGB;
		int Red, Green, Blue;

	        HasSilWidth = TRUE;
		IRIT_SWAP(IrtRType, Options.NPRSilWidth, SilWidth);
		if ((SilRGB = AttrGetObjectStrAttrib(Object,
						     "SilRGB")) != NULL &&
		    (sscanf(SilRGB, "%d,%d,%d", &Red, &Green, &Blue) == 3 ||
		     sscanf(SilRGB, "%d %d %d", &Red, &Green, &Blue) == 3)) {
		    HasSilColor = TRUE;
		    IRIT_PT_COPY(OrigSilColor, Options.NPRSilColor);
		    Options.NPRSilColor[0] = Red;
		    Options.NPRSilColor[1] = Green;
		    Options.NPRSilColor[2] = Blue;
		}
	    }

            /* Triangulate polygons and filter out polylines. */
	    if (IsPolyLine) {
	        ObjectTri = IPCopyObject(NULL, Object, TRUE);
	        GMCleanUpPolylineList(&ObjectTri -> U.Pl, IRIT_EPS);
	    }
	    else {
	        ObjectTri = IObjectTriangulate(Object);
		if (DoClipping)
		    IObjectClip(ObjectTri, ClipPlanes);
	    }

	    if (ObjectTri -> U.Pl != NULL) {
	        int NoShading =
		    !IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(Object,
							       "NoShading"));

		if (!IsPolyLine &&
		    (HasSilWidth ||
		     (Options.NPRRendering && Options.NPRSilWidth > 0.0)))
		    DrawSilBndryEdges(Rend, ObjectTri, ViewMat, NoShading);

	        IRndrBeginObject(Rend, ObjectTri, NoShading);

		for (Poly = ObjectTri -> U.Pl;
		     Poly != NULL;
		     Poly = Poly -> Pnext) {
		    if (IsPolyLine)
			RenderPolyline(Rend, Poly, Width);
		    else
		        IRndrPutTriangle(Rend, Poly);
		}

		IRndrEndObject(Rend);
	    }

	    /* Restore the Options' sil width attribute. */
	    if (HasSilWidth) {
		IRIT_SWAP(IrtRType, Options.NPRSilWidth, SilWidth);
		if (HasSilColor)
		    IRIT_PT_COPY(Options.NPRSilColor, OrigSilColor);
	    }
            if (Options.VisMap) {
	        (*VisMapTriangles) -> U.Pl = IPAppendPolyLists(
                           (*VisMapTriangles) -> U.Pl, ObjectTri -> U.Pl);
                ObjectTri -> U.Pl = NULL;
            }
            IPFreeObject(ObjectTri);
        }

        IPFreeObject(Object);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Render a polyline Pl with width Width.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:       IN, the render context.                                      *
*   Pl:         IN, the polyline to scan convert.                            *
*   Width:      IN, the width of polyline, if has one.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RenderPolyline(IRndrPtrType Rend,
			   IPPolygonStruct *Pl,
			   IrtRType Width)
{
    IPVertexStruct *V;

    /* Set the width if has one. */
    if (IP_ATTR_IS_BAD_REAL(Width))
        IRndrSetPllParams(Rend, Options.PllMinW, Options.PllMaxW,
			  GlblZMin, GlblZMax);
    else {
        IrtRType
	    Width2 = Width * Options.PllMinW / Options.PllMaxW;

	IRndrSetPllParams(Rend, Width2, Width, GlblZMin, GlblZMax);
    }

    IRndrBeginPll(Rend);

    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext)
        IRndrPutPllVertex(Rend, V);

    IRndrEndPll(Rend);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract boundary and silhouette edges of the given objects Objs and      *
* view matrix.                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:       IN, the render context.                                      *
*   PObjs:      IN, Object to extract silhouette and boundary edges for.     *
*   ViewMat:    IN, View from which objects are drawn (for silhouettes).     *
*   NoShading:  IN, if TRUE, ignore shading on this one.                     M
*                                                                            *
* RETURN VALUE:                                                              *
*   void 		                                                     *
*****************************************************************************/
static void DrawSilBndryEdges(IRndrPtrType Rend,
			      IPObjectStruct *PObjs,
			      IrtHmgnMatType ViewMat,
			      int NoShading)
{
    IPObjectStruct *PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
        if (IP_IS_POLY_OBJ(PObj) && !IP_IS_POLYLINE_OBJ(PObj)) {
	    IPPolygonStruct *Pl;
	    IPObjectStruct *PSil, *PBndry,
	        *PTmp = IPCopyObject(NULL, PObj, TRUE);

	    IPOpenPolysToClosed(PTmp -> U.Pl);
	    BoolGenAdjacencies(PTmp);

	    PSil = GMSilExtractSilDirect(PTmp, ViewMat);
	    AttrSetObjectRGBColor(PSil,
				  (int) Options.NPRSilColor[0],
				  (int) Options.NPRSilColor[1],
				  (int) Options.NPRSilColor[2]);

	    PBndry = GMSilExtractBndry(PTmp);
	    AttrSetObjectRGBColor(PBndry,
				  (int) Options.NPRSilColor[0],
				  (int) Options.NPRSilColor[1],
				  (int) Options.NPRSilColor[2]);

	    IRndrBeginObject(Rend, PSil, NoShading);

	    for (Pl = PSil -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	        RenderPolyline(Rend, Pl, Options.NPRSilWidth);
	    IPFreeObject(PSil);

	    IRndrEndObject(Rend);


	    IRndrBeginObject(Rend, PBndry, NoShading);

	    for (Pl = PBndry -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	        RenderPolyline(Rend, Pl, Options.NPRSilWidth);
	    IPFreeObject(PBndry);

	    IRndrEndObject(Rend);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Iterates the object list to find light sources.                          *
*   And then add them to the rendering context.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:       IN, the render context.                                      *
*   Object:     IN, pointer to the object list.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void LightListLoad(IRndrPtrType Rend, IPObjectStruct* Objects)
{
    IPObjectStruct *o;
    const char *t;
    int r, g, b,
        SeenLight = FALSE;
    IRndrLightType Type;
    IrtPtType Where, Color;

    for (o = Objects; o; o = o -> Pnext) {
        if (!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(o, "LIGHT_SOURCE"))) {
            t = AttrGetObjectStrAttrib(o, "TYPE");
            if (!t || stricmp(t, "POINT_POS")) {
                Type = IRNDR_LIGHT_VECTOR;
            }
            else {
                Type = IRNDR_LIGHT_POINT;
            }
            IRIT_PT_COPY(Where, o -> U.Pt);
            if (Type == IRNDR_LIGHT_VECTOR) {
                IRIT_PT_NORMALIZE(Where);
            }
            if (AttrGetObjectRGBColor(o, &r, &g, &b)) {
                Color[RED_CLR] = r;
                Color[GREEN_CLR] = g;
                Color[BLUE_CLR] = b;
                IRIT_PT_SCALE(Color, 1.0 / 0xff);
            }
            else {
                Color[RED_CLR] = 1.0;
                Color[GREEN_CLR] = 1.0;
                Color[BLUE_CLR] = 1.0;
            }
            IRndrAddLightSource(Rend, Type, Where, Color);
            SeenLight = TRUE;
            if (!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(o, "TWOLIGHT"))) {
                IRIT_PT_SCALE(Where, -1);
                IRndrAddLightSource(Rend, Type, Where, Color);
            }
        }
    }
    if (!SeenLight) {
        IrtPtType
	    Ones = { 1.0, 1.0, 1.0 },
            MinusOnes = { -1.0, -1.0, -1.0 };

        IRndrAddLightSource(Rend, IRNDR_LIGHT_VECTOR, Ones, Ones);
        IRndrAddLightSource(Rend, IRNDR_LIGHT_VECTOR, MinusOnes, Ones);
    }
}
