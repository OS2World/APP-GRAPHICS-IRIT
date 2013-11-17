/*****************************************************************************
*   Compute the visibility of the curves using an Open GL Z buffer.          *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "program.h"

#ifdef __OPENGL__
#    ifdef __WINNT__
#	include <windows.h>
#    endif /* __WINNT__ */
#    include <GL/gl.h>
#endif /* __OPENGL__ */

#define Z_BUFFER_SAFE_Z		100
#define ZBUFFER_EPS		-5
#define OBJ_2_SCRN(x)		((int) ((x) + 0.5))
#define IS_POINT_VISIBLE(Pt)	(GMZBufferQueryZ(IGGlblZbufferID, \
						 (int) Pt[0], (int) Pt[1]) + \
							   ZBUFFER_EPS < Pt[2])
IRIT_STATIC_DATA int
    GlblZBufferSize;
IRIT_STATIC_DATA IrtRType
    GlblWidthScale;
IRIT_STATIC_DATA IrtHmgnMatType
    InvCrntViewMat,
    CrntViewMat;
IRIT_STATIC_DATA IPObjectStruct
    *GlblVisCurves = NULL;

IRIT_GLOBAL_DATA int
    IGObjTransNumActiveObjs = 0,
    IGCrvEditActive = FALSE,
    IGSrfEditActive = FALSE;
IRIT_GLOBAL_DATA VoidPtr
    IGGlblZbufferID = NULL;
IRIT_GLOBAL_DATA CagdCrvStruct
    *IGCrvEditCurrentCrv = NULL;
IRIT_GLOBAL_DATA CagdSrfStruct
    *IGSrfEditCurrentSrf = NULL;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGCrvEditCurrentObj = NULL,
    *IGSrfEditCurrentObj = NULL,
    *IGCrvEditPreloadEditCurveObj = NULL,
    *IGSrfEditPreloadEditSurfaceObj = NULL,
    **IGObjTransCurrentObjs = NULL;

static void DrawSrfObjectHierarchy(IPObjectStruct *PObjects);
static void DrawOneObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void ProcessPolyObject(IPObjectStruct *PObj);
static void TestCrvsObject(CagdCrvStruct *Crvs);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a ZBuffer and scan converts the given surfaces and polygons into M
* it.  Given geometry is mapped from [-1, 1] in X andY into 0 to ZBufferSize.M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjs:              To scan convert                                      M
*   ZBufferSize:        Size of ZBuffer (must be square).                    M
*   Depth:		Initial depth to set the ZBuffer to.                 M
*   ImageOperator:	Type of operation to apply to the ZBuffer.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   ScanConvertPolySrfs                                                      M
*****************************************************************************/
void ScanConvertPolySrfs(IPObjectStruct *PObjs,
			 int ZBufferSize,
			 IrtRType Depth,
			 int ImageOperator)
{
    IPObjectStruct *PObjects;
    IrtHmgnMatType Mat;

    MatGenUnitMat(CrntViewMat);
    MatGenMatTrans(1.0, 1.0, 0.0, Mat);
    MatMultTwo4by4(CrntViewMat, CrntViewMat, Mat);
    MatGenMatUnifScale(ZBufferSize >> 1, Mat);
    MatMultTwo4by4(CrntViewMat, CrntViewMat, Mat);

    PObjects = GMTransformObjectList(PObjs, CrntViewMat);
    if (!MatInverseMatrix(CrntViewMat, InvCrntViewMat)) {
	IRIT_WARNING_MSG_PRINTF("Failed to compute the inverse transformation\n");
	ShaderExit(1);
    }

    /* Initialize the ZBuffer. */
    IGGlblZbufferID = GMZBufferInit(ZBufferSize, ZBufferSize);
    GlblZBufferSize = ZBufferSize;
    GMZBufferClearSet(IGGlblZbufferID, Depth);

    if (GlblTalkative) {
	IRIT_INFO_MSG_PRINTF("Initializing Zbuffer of size %d\n",
		ZBufferSize);
    }

    /* Render the polygonal/surface objects into the Z buffer. */
    DrawSrfObjectHierarchy(PObjects);
    IPFreeObjectList(PObjects);

    if (ImageOperator == 1) {
	VoidPtr
	    ZBufID = GMZBufferRoberts(IGGlblZbufferID);

	GMZBufferFree(IGGlblZbufferID);
	IGGlblZbufferID = ZBufID;
    }
    else if (ImageOperator == 2) {
	VoidPtr
	    ZBufID = GMZBufferLaplacian(IGGlblZbufferID);

	GMZBufferFree(IGGlblZbufferID);
	IGGlblZbufferID = ZBufID;
    }
    else if (ImageOperator == 3) {
	VoidPtr
	    ZBufID = GMZBufferInvert(IGGlblZbufferID);

	GMZBufferFree(IGGlblZbufferID);
	IGGlblZbufferID = ZBufID;
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpZBuf, FALSE)
	    DumpZBufferAsSrf("ZBuffer.itd");
    }
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dumps the ZBuffer into the prescribed file name.   			     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   Name of file to save the ZBuffer.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DumpZBufferAsSrf                                                         M
*****************************************************************************/
void DumpZBufferAsSrf(char *FileName)
{
    int x, y;
    CagdSrfStruct
	*Srf = BspSrfNew(GlblZBufferSize, GlblZBufferSize,
			 2, 2, CAGD_PT_E3_TYPE);
    CagdRType
	**Points = Srf -> Points;
    FILE *f;

    BspKnotUniformFloat(GlblZBufferSize, 2, Srf -> UKnotVector);
    BspKnotUniformFloat(GlblZBufferSize, 2, Srf -> VKnotVector);

    for (y = 0; y < GlblZBufferSize; y++) {
        for (x = 0; x < GlblZBufferSize; x++) {
	    Points[1][CAGD_MESH_UV(Srf, x, y)] = x;
	    Points[2][CAGD_MESH_UV(Srf, x, y)] = y;
	    Points[3][CAGD_MESH_UV(Srf, x, y)] = GetPointDepth(x, y);
	}
    }

    if ((f = fopen(FileName, "w")) != NULL) {
        IPObjectStruct
	    *PObj = IPGenSrfObject("ZBuffer", Srf, NULL);

        IPPutObjectToFile(f, PObj, FALSE);
	fclose(f);
	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests the visibility of given curves.  May return more than one curve    M
* in a list.  Returned objects are piecewise linear approximation of Crvs.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:         Curves to check their visibility.	                     M
*   WidthScale:   Scaling factor of variable width of stroked curves.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List of visible curve segments.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   TestCurveVisibility                                                      M
*****************************************************************************/
IPObjectStruct *TestCurveVisibility(CagdCrvStruct *Crvs, IrtRType WidthScale)
{
    GlblWidthScale = WidthScale;
    GlblVisCurves = NULL;

    if (GlblSamplesPerCurve < 2)
	GlblSamplesPerCurve = 2;

    TestCrvsObject(Crvs);

    return GlblVisCurves;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests the visibility of given point.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y, Z:      tests the visibility of the given Euclidean point.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if visible, FALSE otherwise.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TestPointVisibility                                                      M
*****************************************************************************/
int TestPointVisibility(IrtRType X, IrtRType Y, IrtRType Z)
{
    IrtVecType V, VRes;

    V[0] = X;
    V[1] = Y;
    V[2] = Z;

    MatMultPtby4by4(VRes, V, CrntViewMat);

    return GMZBufferQueryZ(IGGlblZbufferID, (int) VRes[0], (int) VRes[1])
						+ ZBUFFER_EPS < VRes[2];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests the depth of given point.	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:      tests the depth of the given ZBuffer location.	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Detected depth or zero of out of ZBuffer domain.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPointDepth	                                                     M
*****************************************************************************/
IrtRType GetPointDepth(int x, int y)
{
    if (x >= 0 && x < GlblZBufferSize && y >= 0 && y < GlblZBufferSize)
        return GMZBufferQueryZ(IGGlblZbufferID, x, y);
    else
        return 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Poly object using current modes and transformations.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to draw.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPoly                                                               M
*****************************************************************************/
void IGDrawPoly(IPObjectStruct *PObj)
{
#ifdef __OPENGL__
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (IP_IS_POLYGON_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IP_IS_STRIP_POLY(Pl)) {
		glBegin(GL_TRIANGLE_STRIP);

		glVertex3dv(V -> Coord);
		V = V -> Pnext;
		glVertex3dv(V -> Coord);
		V = V -> Pnext;
		do {
		    glVertex3dv(V -> Coord);
		    V = V -> Pnext;
		}
		while (V != NULL);

		glEnd();
	    }
	    else {
		glBegin(GL_POLYGON);
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    glVertex3dv(V -> Coord);
		}
		glEnd();
	    }
	}
    }
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traverses the objects and scan converts polygons/surfaces into Z buffer. *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:	Object Hierarchy to draw.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawSrfObjectHierarchy(IPObjectStruct *PObjects)
{
    IrtHmgnMatType IritView;

    MatGenUnitMat(IritView);

    IPTraverseObjListHierarchy(PObjects, IritView, DrawOneObject);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the IPTraverseObjListHierarchy above, through      *
* DrawSrfObjectHierarchy above.	 Process the geometry into triangles and     *
* render into the Z buffer.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to process.                                            *
*   Mat:       Viewing matrix (ignored).                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawOneObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    int i;
    CagdRType r, t, Ru, Rv;
    IPObjectStruct
	*PolyObj = NULL;
    CagdSrfStruct **Srfs;

    if (IP_IS_CRV_OBJ(PObj) ||
	(IP_IS_POLY_OBJ(PObj) && IP_IS_POLYLINE_OBJ(PObj)))
	return;

    r = AttrGetObjectRealAttrib(PObj, "resolution");
    if (IP_ATTR_IS_BAD_REAL(r))
	r = 1.0;

    t = AttrGetObjectRealAttrib(PObj, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        Ru = r * t;
    else
        Ru = r;

    t = AttrGetObjectRealAttrib(PObj, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        Rv = r * t;
    else
        Rv = r;

    /* Skip transparent objects and do not render them into the Z buffer. */
    if (!IP_ATTR_IS_BAD_REAL(AttrGetObjectRealAttrib(PObj, "transp")))
	return;

    switch (PObj -> ObjType) {
        case IP_OBJ_POLY:
	    if (IP_IS_POLYGON_OBJ(PObj))
	        ProcessPolyObject(PObj);
	    break;
        case IP_OBJ_SURFACE:
	    AttrSetRealAttrib(&PObj -> U.Srfs -> Attr, "u_resolution", Ru);
	    AttrSetRealAttrib(&PObj -> U.Srfs -> Attr, "v_resolution", Rv);

	    PolyObj = IPGenPOLYObject(IPSurface2Polygons(PObj -> U.Srfs,
							 TRUE,
							 GlblPolyFineNess,
							 FALSE, FALSE, 0));
	    ProcessPolyObject(PolyObj);
	    break;
	case IP_OBJ_TRIMSRF:
	    AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "u_resolution", Ru);
	    AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "v_resolution", Rv);

	    PolyObj = IPGenPOLYObject(IPTrimSrf2Polygons(PObj -> U.TrimSrfs,
							 TRUE,
							 GlblPolyFineNess,
							 FALSE, FALSE, 0));
	    ProcessPolyObject(PolyObj);
	    break;
	case IP_OBJ_TRIVAR:
	    Srfs = TrivBndrySrfsFromTV(PObj -> U.Trivars);

	    for (i = 0; i < 6; i++) {
	        AttrSetRealAttrib(&Srfs[i] -> Attr, "u_resolution", Ru);
		AttrSetRealAttrib(&Srfs[i] -> Attr, "v_resolution", Rv);

	        PolyObj = IPGenPOLYObject(IPSurface2Polygons(Srfs[i], TRUE,
							     GlblPolyFineNess,
							     FALSE, FALSE, 0));
		ProcessPolyObject(PolyObj);
		IPFreeObject(PolyObj);
		CagdSrfFree(Srfs[i]);
	    }
	    PolyObj = NULL;
	    break;
	case IP_OBJ_TRISRF:
	    PolyObj = IPGenPOLYObject(IPTriSrf2Polygons(PObj -> U.TriSrfs,
							GlblPolyFineNess,
							FALSE, FALSE, 0));
	    ProcessPolyObject(PolyObj);
	    break;
	default:
	    PolyObj = NULL;
	    break;
    }

    if (PolyObj != NULL)
        IPFreeObject(PolyObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Render the polygonal representation into the Z buffer.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object representing the freeform shape.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessPolyObject(IPObjectStruct *PObj)
{
    int OldCirc;
    IPPolygonStruct *Pl;
    IPObjectStruct
	*PTmp = IPCopyObject(NULL, PObj, TRUE);

    /* Make sure all polygons are convex. */
    OldCirc = IPSetPolyListCirc(TRUE);
    IPOpenPolysToClosed(PTmp -> U.Pl);
    GMConvexPolyObject(PTmp);
    IPSetPolyListCirc(OldCirc);
    IPClosedPolysToOpen(PTmp -> U.Pl);

    /* Make sure all polygons are triangle. */
    PObj = GMConvertPolysToTriangles(PTmp);
    IPFreeObject(PTmp);

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct
	    *V1 = Pl -> PVertex,
	    *V2 = V1 -> Pnext,
	    *V3 = V2 -> Pnext;
	IrtRType
	    *R1 = V1 -> Coord,
	    *R2 = V2 -> Coord,
	    *R3 = V3 -> Coord;

	GMZBufferUpdateTri(IGGlblZbufferID,
			   OBJ_2_SCRN(R1[0]),
			   OBJ_2_SCRN(R1[1]),
			   R1[2],
			   OBJ_2_SCRN(R2[0]),
			   OBJ_2_SCRN(R2[1]),
			   R2[2],
			   OBJ_2_SCRN(R3[0]),
			   OBJ_2_SCRN(R3[1]),
			   R3[2]);
    }
    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test the visibility of the given curves against the ZBuffer.             *
*   Saves visible segments in global GlblVisCurves			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      Curves to test for visibility                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TestCrvsObject(CagdCrvStruct *Crvs)
{
    int i, j;
    IPObjectStruct
	*VarWidthObj = AttrGetObjAttrib(Crvs -> Attr, "VarWidth");
    CagdCrvStruct *Crv, *TCrv, *C;
    IrtRType
        WScale = VarWidthObj != NULL ? GlblWidthScale : 1.0;

    for (C = Crvs; C != NULL; C = C -> Pnext) {
	CagdRType TMin, TMax, TStart, t, dt, *R, Pt[3];
	CagdBType
	    Visible = FALSE;

	Crv = CagdCrvMatTransform(C, CrntViewMat);

	CagdCrvDomain(Crv, &TMin, &TMax);
	TStart = TMin;
	dt = (TMax - TMin) / (GlblSamplesPerCurve - 1.0);

	if (VarWidthObj == NULL) {
	    for (i = 0; i < GlblSamplesPerCurve; i++) {
		t = TMin + dt * i;
		R = CagdCrvEval(Crv, t);
		CagdCoerceToE3(Pt, &R, -1, Crv -> PType);

		if (IS_POINT_VISIBLE(Pt)) {
#		    ifdef DEBUG
		    {
		        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintDepth1,
						       FALSE) {
			    printf("[OBJECT [COLOR 2] NONE [POINT %f %f %f] ]\n",
				   Pt[0], Pt[1], Pt[2]);
			}
		    }
#		    endif /* DEBUG */

		    if (!Visible) {
			TStart = t;
			Visible = TRUE;
		    }
		    else {
		    }
		}
		else {
#		    ifdef DEBUG
		    {
		        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintDepth2,
						       FALSE) {
			    printf("[OBJECT [COLOR 3] NONE [POINT %f %f %f] ]\n",
				   Pt[0], Pt[1], Pt[2]);
			}
		    }
#		    endif /* DEBUG */

		    if (Visible) {
			/* End of visible domain. */
			if (TStart < t) {
			    CagdCrvStruct
				*Region = CagdCrvRegionFromCrv(Crv, TStart, t);
			    IPObjectStruct *PObj;

			    TCrv = CagdCrvMatTransform(Region, InvCrntViewMat);
			    CagdCrvFree(Region);
			    Region = TCrv;

			    PObj = IPGenCRVObject(Region);
			    IRIT_LIST_PUSH(PObj, GlblVisCurves);
			}
			Visible = FALSE;
		    }
		    else {
		    }		
		}
	    }

	    if (Visible) {
		if (TStart < t) {
		    CagdCrvStruct
			*Region = CagdCrvRegionFromCrv(Crv, TStart, t);
		    IPObjectStruct *PObj;

		    TCrv = CagdCrvMatTransform(Region, InvCrntViewMat);
		    CagdCrvFree(Region);
		    Region = TCrv;

		    PObj = IPGenCRVObject(Region);
		    IRIT_LIST_PUSH(PObj, GlblVisCurves);
		}
	    }
	}
	else {
	    /* We do have a variable width curve. */
	    CagdRType LastPt[3];
	    CagdBType LastVisible,
	        CoercedDistCrv = FALSE;
	    CagdCrvStruct *DistCrv;
	    
	    if (VarWidthObj -> U.Crvs -> PType != CAGD_PT_E1_TYPE) {
		DistCrv = CagdCoerceCrvTo(VarWidthObj -> U.Crvs,
					  CAGD_PT_E1_TYPE, FALSE);
		CoercedDistCrv = TRUE;
	    }
	    else 
		DistCrv = VarWidthObj -> U.Crvs;

	    t = TMin;
	    R = CagdCrvEval(Crv, t);
	    CagdCoerceToE3(LastPt, &R, -1, Crv -> PType);
	    LastVisible = IS_POINT_VISIBLE(LastPt);

	    for (i = 1; i < GlblSamplesPerCurve; i++) {
		t = TMin + dt * i;
		R = CagdCrvEval(Crv, t);
		CagdCoerceToE3(Pt, &R, -1, Crv -> PType);

		if ((Visible = IS_POINT_VISIBLE(Pt)) && LastVisible) {
#ifdef DUMP_POLY_ITD_FILE
		    IrtPtType TransPt, TransLastPt;

		    MatMultPtby4by4(TransPt, Pt, InvCrntViewMat);
		    MatMultPtby4by4(TransLastPt, LastPt, InvCrntViewMat);

		    R = CagdCrvEval(DistCrv, t - dt * 0.5);

		    printf("[OBJECT [Width %lf] NONE\n    [POLYLINE 2\n",
			   R[1]);
		    printf("\t[%lf %lf %lf]\n\t[%lf %lf %lf]\n    ]\n]\n",
			   TransLastPt[0], TransLastPt[1], TransLastPt[2],
			   TransPt[0], TransPt[1], TransPt[2]);
#else
		    IPObjectStruct *PObjPoly, *PObjTransPoly;
		    IPVertexStruct *V;

		    R = CagdCrvEval(DistCrv, t - dt * 0.5);

		    V = IPAllocVertex2(IPAllocVertex2(NULL));
		    PObjPoly =
			IPGenPOLYObject(IPAllocPolygon(0, V, NULL));
		    IP_SET_POLYLINE_OBJ(PObjPoly);
		    AttrSetObjectRealAttrib(PObjPoly, "width", R[1] * WScale);
		    for (j = 0; j < 3; j++) {
			V -> Coord[j] = LastPt[j];
			V -> Pnext -> Coord[j] = Pt[j];
		    }
		    PObjTransPoly = GMTransformObject(PObjPoly,
						      InvCrntViewMat);
		    IRIT_LIST_PUSH(PObjTransPoly, GlblVisCurves);
#endif /* DUMP_POLY_ITD_FILE */
		}
		LastVisible = Visible;
		IRIT_PT_COPY(LastPt, Pt);
	    }

	    if (CoercedDistCrv)
		CagdCrvFree(DistCrv);
	}

	CagdCrvFree(Crv);
    }
}
