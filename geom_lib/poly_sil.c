/*****************************************************************************
* Fast silhouette extraction from polygonal geometry.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Fabien Benichou				Ver 1.0, Jul 1998    *
* Modified by: Gershon Elber				Ver 1.1, Aug 1998    *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "irit_sm.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "grap_lib.h"
#include "geom_loc.h"
#include "misc_lib.h"

#define GM_IVALID_INDEX		-32767
#define GM_NRML_EPS	 	1e-6
#define GM_DRCT_SIL_MERGE_EPS	1e-3
#define GM_DRCT_DSCNT_MERGE_EPS	1e-3
#define GM_NUM_PLANES		6
#define GM_NUM_BOUND_PLANE	4

#define GM_IRIT_CROSS_PROD_2D(Pt1, Pt2)   ((Pt1)[0] * (Pt2)[1] - \
			              (Pt1)[1] * (Pt2)[0])
#define GM_IRIT_PT2D_SUB(Res, Pt1, Pt2)   { (Res)[0] = (Pt1)[0] - (Pt2)[0]; \
				       (Res)[1] = (Pt1)[1] - (Pt2)[1]; \
				     }
#define GM_PLANE_TO_NORMAL(N, P)     { IRIT_VEC_COPY(N, P); \
				       IRIT_PT_NORMALIZE(N); }

typedef int GMIndicesType[2];
typedef IrtRType GMPoint2DType[2];
typedef GMPoint2DType GMSegment2DType[2];
typedef IrtRType GMRangeType[2];
typedef IrtPtType GMSegmentType[2];

typedef struct GMEdgeStruct {
    struct GMEdgeStruct *Pnext;
    IPVertexStruct *ObjectEdge;
    IrtPtType P1, P2;
} GMEdgeStruct;
    
typedef struct GMTableStruct {
    GMEdgeStruct *First;
    GMEdgeStruct *Last;
    int length;
} GMTableStruct;

typedef struct GMPlaneStruct {
    IrtPlnType PlaneEq;
    GMEdgeStruct *First;
    GMEdgeStruct *Last;
    int length;
    struct GMPlaneStruct *BoundPlanes[GM_NUM_PLANES];
    GMSegmentType BoundSegs[GM_NUM_PLANES];
    GMSegmentType Line;
} GMPlaneStruct;

typedef struct GMEdge2DStruct {
    IrtPtType V1, V2;
    IPVertexStruct *Vrtx1, *Vrtx2;
    GMSegment2DType Seg;
    int VisitMarker;
} GMEdge2DStruct;

typedef struct GMCell2DStruct {
    struct GMCell2DStruct *Pnext;
    GMEdge2DStruct *Edge;
} GMCell2DStruct;

typedef struct GMPlane2DStruct {
    GMCell2DStruct *First;
} GMPlane2DStruct;

typedef GMPlane2DStruct ***GMGridType;

typedef struct GMGridStruct {
    int NumSubdivisions;
    GMPlane2DStruct *EdgesPlane2D[GM_NUM_PLANES];
    GMGridType Grid[GM_NUM_PLANES];
    GMTableStruct *PermanentSegs;
    int Length;
} GMGridStruct;

IRIT_STATIC_DATA int
    GlblOrigPolyObjAlive = FALSE,
    GlblNumQueryRequest = 0;
IRIT_STATIC_DATA GMRangeType Range;
IRIT_STATIC_DATA GMPlaneStruct
    **GlblPlanes = NULL;
IRIT_STATIC_DATA IPPolygonStruct
    *GlblSilsResultPllns = NULL;

static void GMAppendGSTable(GMTableStruct *GSTableDest,
			    GMTableStruct *GSTableSrc);
static void GMFreeGSTable(GMTableStruct *);
static GMTableStruct *GMObjectToGS(IPObjectStruct *PObj);
static GMTableStruct *GMPolyhedraToGS(IPObjectStruct *PObjReg);
static void GMGSProjectedIntoPlanes(GMTableStruct *GSTable,
				    GMPlaneStruct **Planes);
static GMPlaneStruct *GMGenEmptyPlane(IrtPlnType PlaneEq);
static void GMFreePlane(GMPlaneStruct *Plane);


static void GMHPIntersectedWithPlanes(IrtNrmlType NormalHP,
				      GMPlaneStruct **Planes);
static void GMPlane2BasePlane(GMPlaneStruct *Plane);

static GMTableStruct *GMGenEmptyGSTable(void);
static void GMInsertEdgeInGS(GMTableStruct *GSTable,
			     IPVertexStruct *V,
			     IrtPtType P1,
			     IrtPtType P2);
static int GMDeleteEdgeFromGS(GMTableStruct *GSTable,
			      GMEdgeStruct *Edge);
static void GMFindProjectionPlane(GMPlaneStruct **Result,
				  GMPlaneStruct **Planes,
				  IrtPtType GSPoint,
				  IrtPtType ProjPoint); 
static void GMGenProjectionEdge(GMPlaneStruct **Planes,
				GMPlaneStruct **ProjPlane1,
				GMPlaneStruct **ProjPlane2, 
				IrtPtType P1,
				IrtPtType P2,
				IPVertexStruct *V);
static int GMDeleteEdgeFromPlane(GMPlaneStruct *Plane,
				 GMEdgeStruct *Edge);
static void GMInsertSegmentInPlane(GMPlaneStruct *Plane,
				   IrtPtType P1,
				   IrtPtType P2,
				   IPVertexStruct *EdgeObj);
static int GMPtInsidePlanes(IrtPtType P, GMPlaneStruct **Planes);
static int GMIsMember(GMPlaneStruct **PlaneSet,
		      GMPlaneStruct *PlaneQuery);
static void GMPtPlane2PtBasePlane(IrtPtType PDest,
				  IrtPtType PSrc,
				  IrtPlnType PlaneEq);
static int GMFindProjInBound(GMPlaneStruct *ProjPlane1,
			     GMPlaneStruct *BoundPlane,
			     IrtPtType P1,
			     IrtPtType P2,
			     IrtPtType P);
static void GMFindOrthVector(IrtVecType VRes, IrtVecType VSrc);

static GMCell2DStruct *GMGenEmptyCell2D(void);
static GMPlane2DStruct *GMGenEmptyPlane2D(void);
static GMEdge2DStruct *GMGenEdge2D(GMPoint2DType P1,
				   GMPoint2DType P2,
				   IPVertexStruct *EdgeObj);
static GMGridType GMGenEmptyGrid(int n);

static GMGridStruct *GMGsToGrid(GMTableStruct *GSTable, int n);
static GMPlane2DStruct *GMPlane2Plane2D(GMPlaneStruct *Plane,
					GMSegment2DType Line);
static GMPlane2DStruct *GMBasePlane2Plane2D(GMPlaneStruct *Plane,
					    GMSegment2DType Line);

static GMGridType GMBuildSubdividedPlanes(GMPlane2DStruct *Plane, int n);
static GMPlaneStruct **GMGenCubePlanes(void);

static void GMReportInterSegsOpti(GMGridType Grid,
				  GMSegment2DType Line,
				  int n,
				  IPObjectStruct *PObj);
static void GMReportInterSegs(GMPlane2DStruct *Plane,
			      GMSegment2DType Line,
			      IPObjectStruct *PObj);
static GMIndicesType *GMReportInterPlanes(GMSegment2DType Seg, int n);

static void GMFreePlane2D(GMPlane2DStruct *Plane);
static void GMFreeSubdivPlanes(GMGridStruct *Grids);
static void GMDelEdges2DFromPlane2D(GMPlane2DStruct *Plane);

static void GMInsertEdgeInPlane2D(GMPlane2DStruct *Plane,
				  GMEdge2DStruct *Edge);
static void GMInsertSegInPolylineObj(IPObjectStruct *PPolyObj,
				     IrtPtType Coord1,
				     IPVertexStruct *Vrtx1,
				     IrtPtType Coord2,
				     IPVertexStruct *Vrtx2);
static int GMIsInRange(IrtRType P, GMRangeType Range);
static int GMCoord2Index(int *Index,
			 GMRangeType Range,
			 IrtRType  RangeLength,
			 IrtRType P);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the Grid Structure of a polyhedral object.		     M
*   This is the preprocessing stage to the silhouette extraction method.     M
*   Polyhedra is assumed regular, and has adjacency information.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjReg:    Regular polyhedral Object.                                   M
*   n:          Subdivision resolution of the Grid (n by n).                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr: Grid Structure of preprocessing data structure.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractBndry, GMSilExtractSil, BoolGenAdjacencies                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilPreprocessPolys, silhouette, boundary.                              M
*****************************************************************************/
VoidPtr GMSilPreprocessPolys(IPObjectStruct *PObjReg, int n)
{
    GMTableStruct *GSTable;
    
    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);

    Range[0] = -1;
    Range[1] = 1;

    if (GlblPlanes == NULL)
        GlblPlanes = GMGenCubePlanes(); /* Generate planes which bound cube. */
    
    GSTable = GMObjectToGS(PObjReg);

    return GMGsToGrid(GSTable, n);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute a new Grid if the subdivision resolution has been changed.       M
*                                                                            *
* PARAMETERS:                                                                M
*   PrepSils:  Preprocessing data structure of silhouettes to refine to      M
*	       a new resultion n.		                             M
*   n:         New subdivision resolution of the grid.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if the grid was updated, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilPreprocessRefine, silhouette, boundary.                             M
*****************************************************************************/
int GMSilPreprocessRefine(VoidPtr PrepSils, int n)
{
    int i;
    GMGridStruct
	*Grids = (GMGridStruct *) PrepSils;

    if (n != Grids -> NumSubdivisions) {
        GMFreeSubdivPlanes(Grids);

        Grids -> NumSubdivisions = n;
        
        for (i = 0; i < GM_NUM_PLANES; i++) 
	  Grids -> Grid[i] = GMBuildSubdividedPlanes(Grids -> EdgesPlane2D[i],
						     n);
        return TRUE;
    }
    else
        return FALSE;
}             

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the silhouette from a polyhedral object, assumed already       M
*   regularized with the straight forward method.                            M
*									     *
* PARAMETERS:                                                                M
*   PObjReg:   Polyhedral Object to generate the silhouette for.             M
*   ViewMat:   View Matrix.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Silhouette Object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractBndry, GMSilExtractSil, GMSilExtractSilDirect2		     M
*   GMSilExtractDiscont							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilExtractSilDirect, silhouette.                                       M
*****************************************************************************/
IPObjectStruct *GMSilExtractSilDirect(IPObjectStruct *PObjReg,
				      IrtHmgnMatType ViewMat)
{
    int i;
    IrtRType Dot1, Dot2;
    IrtVecType ViewDir, NormalPl, NormalPAdj;
    IrtHmgnMatType InvViewMat;
    IPObjectStruct *PObjRes, *PObjTmp;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if (!MatInverseMatrix(ViewMat, InvViewMat))
	return NULL;

    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);
    PObjReg = GMTransformObject(PObjReg, ViewMat);

    /* Make sure vertices lists are circular. */
    GMVrtxListToCircOrLin(PObjReg -> U.Pl, TRUE);
    BoolGenAdjacencies(PObjReg);

    PObjRes = IPAllocObject("Sils", IP_OBJ_POLY, NULL);
    IP_SET_POLYLINE_OBJ(PObjRes);

    ViewDir[0] = ViewDir[1] = 0.0;
    ViewDir[2] = 1.0;
    
    for (Pl = PObjReg -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
        AttrSetIntAttrib(&(Pl -> Attr), "_Was_Visited", FALSE);

    for (Pl = PObjReg -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        GM_PLANE_TO_NORMAL(NormalPl, Pl -> Plane);            
        for (V = Pl -> PVertex, i = 0;
	     V && ((i == 0) || (V != Pl -> PVertex));
	     V = V -> Pnext) {
	    if (i == 0)
	        i++;
	    if ((V -> PAdj) &&
		!AttrGetIntAttrib(V -> PAdj -> Attr,"_Was_Visited")) {
	        if (V -> PAdj -> Plane == NULL)
		    IRIT_WARNING_MSG("GMSilExtractSilDirect: no plane for adjacent polygon!\n");
		GM_PLANE_TO_NORMAL(NormalPAdj, V -> PAdj -> Plane);    
		if (!IRIT_PT_APX_EQ_EPS(NormalPl, NormalPAdj, 1e-6)) {
		    Dot1 = IRIT_DOT_PROD(NormalPl, ViewDir);
		    Dot2 = IRIT_DOT_PROD(NormalPAdj, ViewDir);

		    if (((Dot1 < 0.0) && (Dot2 > 0.0)) ||
			((Dot1 > 0.0) && (Dot2 < 0.0))) {
		        GMInsertSegInPolylineObj(PObjRes, V -> Coord, V,
						 V -> Pnext -> Coord,
						 V -> Pnext);
		    }
		}
	    }
        }
        AttrSetIntAttrib(&(Pl -> Attr), "_Was_Visited", TRUE);        
    }

    /* Merge the silhouettes into longer polylines. */
    PObjRes -> U.Pl = GMMergePolylines(PObjRes -> U.Pl, GM_DRCT_SIL_MERGE_EPS);
    PObjTmp = GMTransformObject(PObjRes, InvViewMat);
    IPFreeObject(PObjRes);

    IPFreeObject(PObjReg);

    return PObjTmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if a triangle polygon contains silouhette line. If so, the smooth   *
* silhouette polyline is added to a global list.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   P:      Pointer to the triangle.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.								     *
*****************************************************************************/
static void GMSilsDirect2GenSmoothSilhouette(IPPolygonStruct *P)
{
    IPVertexStruct *V, *VertList[3];
    int i,
	Total = 0;

    for (V = P -> PVertex, i = 0; V != NULL && i < 3; V = V -> Pnext, i++) {
	if (V -> Normal[2] < 0)
	    Total++;

	VertList[i] = V;
    }

    if (Total == 1 || Total == 2) {
	for (i = 0; i < 3; i++) {
	    int i1 = (i + 1) % 3,
	        i2 = (i + 2) % 3;
	    IrtRType t;

	    if (VertList[i] -> Normal[2] * VertList[i1] -> Normal[2] < 0 &&
	        VertList[i1] -> Normal[2] * VertList[i2] -> Normal[2] < 0) {
		IrtPtType P0, P1;
		IPPolygonStruct *NewPolyLine;

		t = fabs(VertList[i] -> Normal[2]) /
				       (fabs(VertList[i1] -> Normal[2]) +
					fabs(VertList[i] -> Normal[2]));
		IRIT_PT_BLEND(P0, VertList[i1] -> Coord, VertList[i] -> Coord, t);

		t = fabs(VertList[i1] -> Normal[2]) /
				       (fabs(VertList[i2] -> Normal[2]) +
					fabs(VertList[i1] -> Normal[2]));
		IRIT_PT_BLEND(P1, VertList[i2] -> Coord, VertList[i1] -> Coord, t);

		NewPolyLine = GMGenPolyline2Vrtx(P0, P1, NULL);
		IRIT_LIST_PUSH(NewPolyLine, GlblSilsResultPllns);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the silhouette from a polyhedral object consisting of only     M
* triangles, and further assumed already regularized.  The silhouettes       M
* generated by this function are interior to the triangles and are computed  M
* for each triangle individually, based on its vertices' normals.	     M
*									     *
* PARAMETERS:                                                                M
*   PObjReg:   Polyhedral Object to generate the silhouette for.  Assumed to M
*	       be regular and hold triangles only.		             M
*   ViewMat:   View Matrix.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Silhouette Object.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractBndry, GMSilExtractSil, GMSilExtractSilDirect		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilExtractSilDirect2, silhouette.                                      M
*****************************************************************************/
IPObjectStruct *GMSilExtractSilDirect2(IPObjectStruct *PObjReg,
				       IrtHmgnMatType ViewMat)
{
    IPObjectStruct *Result;
    IrtHmgnMatType InvViewMat;

    if (!MatInverseMatrix(ViewMat, InvViewMat))
	return NULL;

    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);
    PObjReg = GMTransformObject(PObjReg, ViewMat);

    GlblSilsResultPllns = NULL;

    IPForEachPoly(PObjReg, GMSilsDirect2GenSmoothSilhouette);
    IPFreeObject(PObjReg);

    GlblSilsResultPllns = GMMergePolylines(GlblSilsResultPllns,
					   GM_DRCT_SIL_MERGE_EPS);
    PObjReg = IPGenPOLYLINEObject(GlblSilsResultPllns);
    Result = GMTransformObject(PObjReg, InvViewMat);
    IPFreeObject(PObjReg);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates edges along adjacent polygonals with a dihedral angle of more  M
* than MinAngle degrees.			                             M
*									     *
* PARAMETERS:                                                                M
*   PObjReg:   Polyhedral Object to generate the silhouette for.             M
*   MinAngle:  Minimal dihedral angle between adjacent polygons to consider  M
*              as discontinuity.  In radians.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Discontinuities Object.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractBndry, GMSilExtractSil, GMSilExtractSilDirect2		     M
*   GMSilExtractSilDirect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilExtractDiscont, discontinuities.	                             M
*****************************************************************************/
IPObjectStruct *GMSilExtractDiscont(IPObjectStruct *PObjReg,
				    IrtRType MinAngle)
{
    int i;
    IrtRType
        MinCosAngle = cos(MinAngle);
    IrtVecType NormalPl, NormalPAdj;
    IPObjectStruct *PObjRes;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);
    PObjReg = IPCopyObject(NULL, PObjReg, FALSE);

    /* Make sure vertices lists are circular. */
    GMVrtxListToCircOrLin(PObjReg -> U.Pl, TRUE);
    BoolGenAdjacencies(PObjReg);

    PObjRes = IPAllocObject("Dscnts", IP_OBJ_POLY, NULL);
    IP_SET_POLYLINE_OBJ(PObjRes);

    for (Pl = PObjReg -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
        AttrSetIntAttrib(&(Pl -> Attr), "_Was_Visited", FALSE);

    for (Pl = PObjReg -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        GM_PLANE_TO_NORMAL(NormalPl, Pl -> Plane);            
        for (V = Pl -> PVertex, i = 0;
	     V && ((i == 0) || (V != Pl -> PVertex));
	     V = V -> Pnext) {
	    if (i == 0)
	        i++;
	    if ((V -> PAdj) &&
		!AttrGetIntAttrib(V -> PAdj -> Attr,"_Was_Visited")) {
	        if (V -> PAdj -> Plane == NULL)
		    IRIT_WARNING_MSG("GMSilExtractDiscont: no plane for adjacent polygon!\n");
		GM_PLANE_TO_NORMAL(NormalPAdj, V -> PAdj -> Plane);    
		if (!IRIT_PT_APX_EQ_EPS(NormalPl, NormalPAdj, 1e-6) &&
		    IRIT_DOT_PROD(NormalPl, NormalPAdj) < MinCosAngle) {
		    GMInsertSegInPolylineObj(PObjRes, V -> Coord, V,
					     V -> Pnext -> Coord, V -> Pnext);
		}
	    }
        }
        AttrSetIntAttrib(&(Pl -> Attr), "_Was_Visited", TRUE);        
    }

    /* Merge the discontinuities into longer polylines. */
    PObjRes -> U.Pl = GMMergePolylines(PObjRes -> U.Pl,
				       GM_DRCT_DSCNT_MERGE_EPS);

    IPFreeObject(PObjReg);

    return PObjRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the boundary of the polyhdral object PObjReg, assumed already  M
*   regularized.				                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjReg:   Object to extract the Boundary.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The Boundary Object.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractSil, GMSilExtractSilDirect, BoolGenAdjacencies	             M
*   GMSilExtractDiscont							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilExtractBndry, boundary, silhouette.                                 M
*****************************************************************************/
IPObjectStruct *GMSilExtractBndry(IPObjectStruct *PObjReg)
{
    int i;
    IPObjectStruct *PObjRes;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;
    
    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);

    BoolGenAdjacencies(PObjReg);

    PObjRes = IPAllocObject("Bndry", IP_OBJ_POLY, NULL);
    IP_SET_POLYLINE_OBJ(PObjRes);

    for (Pl = PObjReg -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        for (V = Pl -> PVertex, i = 0;
	     V != NULL && ((i == 0) || (V != Pl -> PVertex));
	     V = V -> Pnext) {
	    if (V -> PAdj == NULL)
	        GMInsertSegInPolylineObj(PObjRes,
					 V -> Coord, V,
					 V -> Pnext -> Coord, V -> Pnext);
	    i = 1;
        }
    }

    /* Merge the silhouettes into longer polylines. */
    PObjRes -> U.Pl = GMMergePolylines(PObjRes -> U.Pl, GM_DRCT_SIL_MERGE_EPS);

    return PObjRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the silhouette of an object which has been already             M
* preprocessed and is associated with a grid structure.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PrepSils:   Associated silhouette processing data structure of a         M
*		polygonal object to generate its silhouettes.	             M
*   ViewMat:    View Matrix.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Silhouette Object.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilPreprocessPolys, GMSilOrigObjAlive, GMSilExtractSilDirect,          M
*   GMSilExtractSilDirect2						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilExtractSil, silhouette.                                             M
*****************************************************************************/
IPObjectStruct *GMSilExtractSil(VoidPtr PrepSils, IrtHmgnMatType ViewMat)
{
    GMSegment2DType Line[GM_NUM_PLANES];
    IrtVecType ViewDir;
    IPObjectStruct *PObjRes;
    GMEdgeStruct *GSEdge;
    GMGridStruct
	*Grids = (GMGridStruct *) PrepSils;
    int i,
        n = Grids -> NumSubdivisions;

    GlblNumQueryRequest++;
    PObjRes = IPAllocObject("Silhouette",IP_OBJ_POLY, NULL);
    IP_SET_POLYLINE_OBJ(PObjRes);
    /* Inserts the permanent segments in the object. */
    for (GSEdge = Grids -> PermanentSegs -> First;
	 GSEdge != NULL;
	 GSEdge = GSEdge -> Pnext) {
        if (GSEdge -> ObjectEdge && GSEdge -> ObjectEdge -> Pnext)
	    GMInsertSegInPolylineObj(PObjRes,
				     GSEdge -> ObjectEdge -> Coord,
				     GSEdge -> ObjectEdge,
				     GSEdge -> ObjectEdge -> Pnext -> Coord,
				     GSEdge -> ObjectEdge -> Pnext);
    }

    /* Inserts the appropriated segments according to the view point. */    
    ViewDir[0] =  ViewMat[0][2];
    ViewDir[1] =  ViewMat[1][2];
    ViewDir[2] =  ViewMat[2][2];
    IRIT_VEC_NORMALIZE(ViewDir);

    /* Finds the intersections between the view plane and the cube and       */
    /* stores the line intersections in each face of the cube.               */
    GMHPIntersectedWithPlanes(ViewDir, GlblPlanes);

    for (i = 0; i < GM_NUM_PLANES; i++) {
        /* Converts the intersection line stored in each plane in 2D. */
        GMPlane2Plane2D(GlblPlanes[i], Line[i]);
        GMReportInterSegsOpti(Grids -> Grid[i], Line[i], n, PObjRes);
    }    

    return PObjRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the proprocessing data structure of silhouettes.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PrepSils:  To free.                                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilProprocessFree, allocation                                          M
*****************************************************************************/
void GMSilProprocessFree(VoidPtr PrepSils)
{
    int i;
    GMGridStruct
	*Grids = (GMGridStruct *) PrepSils;

    if (!Grids)
        return;

    GMFreeGSTable(Grids -> PermanentSegs);  /* Frees the permanent segments. */
    GMFreeSubdivPlanes(Grids);               /* Frees the subdivided planes. */

    /* Frees the segments 2d in each planes. */
    for (i = 0; i < GM_NUM_PLANES; i++) {
        GMDelEdges2DFromPlane2D(Grids -> EdgesPlane2D[i]);
        GMFreePlane2D(Grids -> EdgesPlane2D[i]);
    }

    IritFree(Grids);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If TRUE, this module is allowed to assume that the original polygonal    M
* object is alive while silhouette queries are conducted.  Default to FALSE. M
*   Setting it to TRUE would allow certain optimization as well as the       M
* propagation of attributes of vertices from the original object to the      M
* detected silhouette edges.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjAlive:   If TRUE, assumes original object remains valid throughout.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Original vaolue of original object alive.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSilExtractSil                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSilOrigObjAlive                                                        M
*****************************************************************************/
int GMSilOrigObjAlive(int ObjAlive)
{
    int OldVal = GlblOrigPolyObjAlive;

    GlblOrigPolyObjAlive = ObjAlive;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a plane without segments inside.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   PlaneEq:   Equation of the plane to generate.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPlaneStruct *:  Created plane.                                         *
*****************************************************************************/
static GMPlaneStruct *GMGenEmptyPlane(IrtPlnType PlaneEq)
{
    GMPlaneStruct *Plane;
    int i;
    IrtPtType O;

    O[0] = O[1] = O[2] = 0;
    
    Plane = (GMPlaneStruct *) IritMalloc(sizeof(GMPlaneStruct));
    IRIT_PLANE_COPY(Plane -> PlaneEq, PlaneEq);
    for (i = 0; i < GM_NUM_PLANES; Plane -> BoundPlanes[i] = NULL, i++); 
    Plane -> length = 0;
    Plane -> First = Plane -> Last = NULL;
    IRIT_PT_COPY(Plane -> Line[0], O); 
    IRIT_PT_COPY(Plane -> Line[1], O); 
    for (i = 0; i < GM_NUM_PLANES; i++) {
        IRIT_PT_COPY(Plane -> BoundSegs[i][0], O);
        IRIT_PT_COPY(Plane -> BoundSegs[i][1], O);
    }
    return Plane;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates an empty GS table.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMTableStruct *:  Created Gaussian Sphere table.                         *
*****************************************************************************/
static GMTableStruct *GMGenEmptyGSTable(void)
{
    GMTableStruct *GSTable;

    GSTable = (GMTableStruct *) IritMalloc(sizeof(GMTableStruct));
    GSTable -> length = 0;
    GSTable -> First = GSTable -> Last = NULL;

    return GSTable;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert a list of polyhedral object to the Gaussian Sphere               *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjsReg:  Object list containing polyhedral object regularized          *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMTableStruct *:  Structure of the Gussian Sphere                        *
*****************************************************************************/
static GMTableStruct *GMObjectToGS(IPObjectStruct *PObjsReg)
{
    GMTableStruct *GSTable, *GSTable2;

    if (PObjsReg == NULL)
        return NULL;
    
    GSTable = GMGenEmptyGSTable();

    if (IP_IS_POLY_OBJ(PObjsReg)) {
        GMAppendGSTable(GSTable, (GSTable2 = GMPolyhedraToGS(PObjsReg)));
        IritFree(GSTable2);
    }
    else {
        GMAppendGSTable(GSTable, (GSTable2 = GMObjectToGS(PObjsReg -> Pnext)));
        IritFree(GSTable2);
    }        
    return GSTable;
}
    
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Maps a polyhedra onto the Gaussian Sphere .                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjReg: a regularized polyhedra.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMTableStruct *: Structure of the Gaussian Sphere                        *
*****************************************************************************/
static GMTableStruct *GMPolyhedraToGS(IPObjectStruct *PObjReg)
{
    IPPolygonStruct *PCurr;
    IrtPtType NormalPCurr, NormalPAdj;
    IPVertexStruct *VCurr;
    GMTableStruct *GSTable;
    int i, j;
    
    if (!IP_IS_POLY_OBJ(PObjReg))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);
    
    GSTable = GMGenEmptyGSTable();

    if (IP_IS_POLYLINE_OBJ(PObjReg))
        return GSTable;				 /* No adj. in polyline obj. */
    
    PCurr = PObjReg -> U.Pl;
    while (PCurr != NULL) {
        AttrSetIntAttrib(&(PCurr->Attr), "_Was_Visited", FALSE);
        PCurr = PCurr -> Pnext;
    }
    j=0;
    PCurr = PObjReg -> U.Pl;
    while (PCurr != NULL) {
        i = 0;
        GM_PLANE_TO_NORMAL(NormalPCurr, PCurr -> Plane);    
        VCurr = PCurr -> PVertex;
        while ((VCurr != NULL) && ((i==0) || (VCurr != PCurr -> PVertex))) {
	    i++;
	    if ((VCurr -> PAdj) &&
		!AttrGetIntAttrib(VCurr -> PAdj -> Attr, "_Was_Visited")) {
	        /* can be optimized by stroring the normals in attributes. */
	        if (VCurr -> PAdj -> Plane == NULL)
		    IRIT_WARNING_MSG("Fatal Error - plane equation not found!\n");
		GM_PLANE_TO_NORMAL(NormalPAdj, VCurr -> PAdj -> Plane);    
		/* Insert edge only if normals of the adjecent polygons are  */
		/* different.						     */
		if (!IRIT_PT_APX_EQ_EPS(NormalPCurr, NormalPAdj, GM_NRML_EPS)) {
		    GMInsertEdgeInGS(GSTable, VCurr, NormalPCurr, NormalPAdj);
		}
	    }
	    VCurr = VCurr->Pnext;
        }
        
        AttrSetIntAttrib(&(PCurr->Attr), "_Was_Visited", TRUE);
        PCurr = PCurr -> Pnext;
        j++;
    }
    return GSTable;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generate the intersection between the cube and a view plane.             *
*   Note : if a plane of the cube has no intersection with  the view plane   *
*   then the intersected line stored in the plane will be set to             *
*   [(0,0,0) (0,0,0)].                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   NormalHP:  normal of the view plane.                                     *
*   Planes:    Planes of the cube. The line intersection is stored in this   *
*              structure.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	                                                             *
*****************************************************************************/
static void GMHPIntersectedWithPlanes(IrtNrmlType NormalHP,
				      GMPlaneStruct **Planes)
{
    int i, j;
    IrtPtType P, O;
    IrtPlnType PlaneHP;
    GMPlaneStruct *BoundPlane;

    O[0] = O[1] = O[2] = 0;
    
    PlaneHP[0] = NormalHP[0];
    PlaneHP[1] = NormalHP[1];
    PlaneHP[2] = NormalHP[2];
    PlaneHP[3] = 0;

    /* Initialisation. */
    for (i = 0; i < GM_NUM_PLANES; i++) {
        IRIT_PT_COPY(Planes[i] -> Line[0], O);
        IRIT_PT_COPY(Planes[i] -> Line[1], O);
    }        
    
    for (i = 0; i < GM_NUM_PLANES; i++) {
	for (j = i + 1; j < GM_NUM_PLANES; j++) {
	    if (((BoundPlane = Planes[i] -> BoundPlanes[j]) != NULL)) {
	        if (GMPointFrom3Planes(PlaneHP, Planes[i] -> PlaneEq,
				       BoundPlane -> PlaneEq, P)) {
		    /* There is an intersection between the view plane and   */
		    /* an edge of projection object (herein it is a cube).   */
		    IrtVecType V, VSeg;

		    IRIT_VEC_SUB(VSeg, Planes[i] -> BoundSegs[j][0],
			          Planes[i] -> BoundSegs[j][1]);
		    IRIT_VEC_SUB(V, Planes[i] -> BoundSegs[j][0], P);

		    /* Check if the projection point is contained in a face  */
		    /* of the cube.					     */
		    if ((IRIT_DOT_PROD(VSeg,V) > 0) &&
			(IRIT_VEC_LENGTH(VSeg) >= IRIT_VEC_LENGTH(V))) {
		        IRIT_PT_COPY(Planes[i] -> Line[1], Planes[i] -> Line[0]);
			IRIT_PT_COPY(Planes[i] -> Line[0], P);
			IRIT_PT_COPY(Planes[j] -> Line[1], Planes[j] -> Line[0]);
			IRIT_PT_COPY(Planes[j] -> Line[0], P);
		    }
	        }
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Transforms the given plane to have the equation Z = 0.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane: Plane to be transformed.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	                                                             *
*****************************************************************************/
static void GMPlane2BasePlane(GMPlaneStruct *Plane)
{
    GMEdgeStruct *PCell;

    PCell = Plane -> First;

    while (PCell) {
        /* Rotate it into the plane Z=0. */
        GMPtPlane2PtBasePlane(PCell -> P1, PCell -> P1, Plane -> PlaneEq);
        GMPtPlane2PtBasePlane(PCell -> P2, PCell -> P2, Plane -> PlaneEq);    
        PCell = PCell -> Pnext;
    }
    
    GMPtPlane2PtBasePlane(Plane -> Line[0], Plane -> Line[0],
			  Plane -> PlaneEq);
    GMPtPlane2PtBasePlane(Plane -> Line[1], Plane -> Line[1],
			  Plane -> PlaneEq);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Projects the arcs on the Gaussian Sphere onto the planes of the          *
* circumscribing cube.						             *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTable:  Gaussian sphere that store the set of arcs.                    *
*   Planes:   Cube planes that store set of segments resulting from the      *
*	      projection of the arcs in GS.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	                                                             *
*****************************************************************************/
static void GMGSProjectedIntoPlanes(GMTableStruct *GSTable,
				    GMPlaneStruct **Planes)
{
    GMEdgeStruct
	*GSEdge = GSTable -> First;
    GMPlaneStruct *ProjPlane1[2], *ProjPlane2[2]; 
    IrtPtType P1, P2;         ;

    while (GSEdge != NULL) {
        GMEdgeStruct
	    *GSEdgeNext = GSEdge -> Pnext;
        
        if (!IRIT_APX_EQ(IRIT_DOT_PROD(GSEdge -> P1, GSEdge -> P2), -1)) {
	    GMFindProjectionPlane(ProjPlane1, Planes, GSEdge -> P1, P1);
	    GMFindProjectionPlane(ProjPlane2, Planes, GSEdge -> P2, P2);
	    GMGenProjectionEdge(Planes, ProjPlane1, ProjPlane2, P1, P2,
				GSEdge -> ObjectEdge);
	    GMDeleteEdgeFromGS(GSTable, GSEdge);
        }
        GSEdge = GSEdgeNext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find among a set of planes, the first intersected plane by the line      *
*   defined by [O GSPoint) .                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Result:    First plane intersected.                                      *
*   Planes:    Set of cube planes.                                           *
*   GSPoint:   Line to intersect.                                            *
*   ProjPoint: Intersection point.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFindProjectionPlane(GMPlaneStruct **Result,
				  GMPlaneStruct **Planes,
				  IrtPtType GSPoint,
				  IrtPtType ProjPoint)
{
    int i = 0;
    IrtRType t,
        TMin = IRIT_INFNTY;
    IrtPtType Proj;
    
    while (i < GM_NUM_PLANES) {
        if (GMPointFromLinePlane(GSPoint, GSPoint, Planes[i] -> PlaneEq,
				 Proj, &t) && t >= 0 && t <= TMin) {
	    if (IRIT_APX_EQ(t, TMin)) {
	        if (t < TMin)
		    TMin = t;
		Result[1] = Result[0];
	    }
	    else {
	        Result[1] = NULL;
		TMin = t;
	    }
	    Result[0] = Planes[i];
	    IRIT_PT_COPY(ProjPoint, Proj);
        }
        i++;
    }
    
    if (Result[0] == NULL)
        GEOM_FATAL_ERROR(GEOM_ERR_PROJ_FAILED);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the projection of the arc [P1,O,P2] onto the cube.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Planes:      Set of cube planes.                                         *
*   ProjPlane1:  Plane(s) of P1.                                             *
*   ProjPlane2:  Plane(s) of P2.                                             *
*   P1:          First segment of the arc.                                   *
*   P2:          Second segment of the arc.                                  *
*   V:           Edge whith associated arc [P1,O,P2].                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMGenProjectionEdge(GMPlaneStruct **Planes,
				GMPlaneStruct **ProjPlane1,
				GMPlaneStruct **ProjPlane2, 
				IrtPtType P1,
				IrtPtType P2,
				IPVertexStruct *V)
{
    if (ProjPlane1[0] != NULL &&
	(ProjPlane1[0] == ProjPlane2[0] ||
	 ProjPlane1[0] == ProjPlane2[1])) {
        GMInsertSegmentInPlane(ProjPlane1[0], P1, P2, V);
    }
    else if (ProjPlane1[1] != NULL &&
	     (ProjPlane1[1] == ProjPlane2[0] ||
	      ProjPlane1[1] == ProjPlane2[1])) {
	GMInsertSegmentInPlane(ProjPlane1[1], P1, P2, V);
    }
    else {
        GMPlaneStruct *PotentialIntersectedPlanes[GM_NUM_PLANES];
        GMPlaneStruct *ProjP1, *ProjP2;
        IrtPtType P;
        int i,j;
        
        if (ProjPlane1[0] != NULL && ProjPlane2[0] != NULL &&
	    GMIsMember(ProjPlane1[0] -> BoundPlanes, ProjPlane2[0])) {
	    ProjP1 = ProjPlane1[0];
	    ProjP2 = ProjPlane2[0];
        }
        else if (ProjPlane1[0] != NULL && ProjPlane2[1] != NULL &&
		 GMIsMember(ProjPlane1[0] -> BoundPlanes, ProjPlane2[1])) {
	    ProjP1 = ProjPlane1[0];
	    ProjP2 = ProjPlane2[1];
        }
        else if (ProjPlane1[1] != NULL && ProjPlane2[0] != NULL &&
		 GMIsMember(ProjPlane1[1] -> BoundPlanes, ProjPlane2[0])) {
	    ProjP1 = ProjPlane1[1];
	    ProjP2 = ProjPlane2[0];
        }
        else if (ProjPlane1[1] != NULL && ProjPlane2[1] != NULL &&
		 GMIsMember(ProjPlane1[1] -> BoundPlanes, ProjPlane2[1])) {
	    ProjP1 = ProjPlane1[1];
	    ProjP2 = ProjPlane2[1];
        }
        else if (ProjPlane1[0] != NULL && ProjPlane2[0] != NULL) {
	    /* Parallel planes and non in an edge. */
	    ProjP1 = ProjPlane1[0];
	    ProjP2 = ProjPlane2[0];
        }
	else
	    return;

        /* Generate the PotentialIntersectedPlanes. */
        PotentialIntersectedPlanes[0] = ProjP2;
        i = 1;
        j = 0;
        while (i < GM_NUM_PLANES && j < GM_NUM_PLANES) {
	    if (ProjP1 -> BoundPlanes[j] && ProjP2 -> BoundPlanes[j]) {
	        PotentialIntersectedPlanes[i] = ProjP1 -> BoundPlanes[j];
	        i++;
	    }
	    j++;
        }
        while (i < GM_NUM_PLANES)
	    PotentialIntersectedPlanes[i++] = NULL;
	          
        i = 0;
        while (PotentialIntersectedPlanes[i] &&
	       !(GMFindProjInBound(ProjP1, PotentialIntersectedPlanes[i],
				   P1, P2, P) &&
		 GMPtInsidePlanes(P, Planes)))
	    i++;

        /* Debug. */
        if (!PotentialIntersectedPlanes[i]) {
	    GMFindProjInBound(ProjP1, PotentialIntersectedPlanes[0], P1, P2, P);
	    GMPtInsidePlanes(P, Planes);
        }

        GMInsertSegmentInPlane(ProjP1, P1, P, V);
        if (PotentialIntersectedPlanes[i] == ProjPlane2[0])
	    GMInsertSegmentInPlane(ProjPlane2[0], P, P2, V);
        else if (ProjPlane2[1] &&
		 PotentialIntersectedPlanes[i] == ProjPlane2[1])
	    GMInsertSegmentInPlane(ProjPlane2[1], P, P2, V);
        else {
	    GMPlaneStruct *NewProjP1[2], *NewProjP2[2];

	    NewProjP1[0] = PotentialIntersectedPlanes[i];
	    NewProjP2[0] = ProjP2;
	    NewProjP1[1] = NewProjP2[1] = NULL;
	    GMGenProjectionEdge(Planes, NewProjP1, NewProjP2, P, P2, V);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if a point is inside the cube.                                     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   P:         Point to verify containment,                                  *
*   Planes:    Of cube.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if inside, FALSE otherwise.                                  *
*****************************************************************************/
static int GMPtInsidePlanes(IrtPtType P, GMPlaneStruct **Planes)
{
    int i = 0;

    while (i < GM_NUM_PLANES) {
        IrtRType
	    Sum = IRIT_DOT_PROD(Planes[i] -> PlaneEq, P) + Planes[i] -> PlaneEq[3];

        if ((Sum * Planes[i] -> PlaneEq[3] < 0.0) && !IRIT_APX_EQ(Sum, 0.0))
	    return FALSE;
        i++;
    }
    return TRUE;
}
        	
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds (if exists) the intersection point between the arc segment [P1 P2] *
* and the boundary formed by ProjPlane1 and BoundPlane.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   ProjPlane1:   First plane of the cube.                                   *
*   BoundPlane:   Second (adjacent) plane of the cube.                       *
*   P1, P2:       End points of arc.                                         *
*   P:            Intersection point (if exists).                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE, if found intersection), FALSE otherwise.             *
*****************************************************************************/
static int GMFindProjInBound(GMPlaneStruct *ProjPlane1,
			     GMPlaneStruct *BoundPlane,
			     IrtPtType P1,
			     IrtPtType P2,
			     IrtPtType P)
{
    IrtVecType POrth;
    IrtPlnType Plane;

    IRIT_CROSS_PROD(POrth, P1, P2);

    /* Test if [0 P1] and [0 P2] non colinear. */
    if (IRIT_PT_APX_EQ_ZERO_EPS(POrth, GM_NRML_EPS)) {
        /* Every vector orthogonal to P1 is available. */ 
        GMFindOrthVector(POrth, P1);
    }

    IRIT_VEC_NORMALIZE(POrth);
    
    IRIT_VEC_COPY(Plane, POrth);
    Plane[3] = 0;

    /* Find the intersection point between the 3 HPs. */
    if (!GMPointFrom3Planes(Plane, ProjPlane1 -> PlaneEq,
			    BoundPlane -> PlaneEq, P))
        return FALSE;

    return TRUE;
}
    
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if a PlaneQuery is a member of the set PlaneSet.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PlaneSet:     The set to test if PlaneQuery is in.                       *
*   PlaneQuery:   Element to search for.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if found, FALSE otherwise.                                  *
*****************************************************************************/
static int GMIsMember(GMPlaneStruct **PlaneSet,
		      GMPlaneStruct *PlaneQuery)
{
    int i ;
    
    for (i = 0; i < GM_NUM_PLANES; i++) {
	if (PlaneSet[i] == PlaneQuery)
	    return TRUE;
    }
    
    return FALSE;
}
    
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Change the point in a plane into a point in the base plane: Z = 0.       *
*                                                                            *
* PARAMETERS:                                                                *
*   PDest:    Modified point.                                                *
*   PSrc:     Original point.                                                *
*   PlaneEq:  The plane PSrc is on.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMPtPlane2PtBasePlane(IrtPtType PDest,
				  IrtPtType PSrc,
				  IrtPlnType PlaneEq)
{
    IrtHmgnMatType Mat1, Mat2;
    IrtVecType Vz;

    if (PlaneEq == NULL)
        IRIT_WARNING_MSG("Fatal Error - plane is NULL!\n");
    GM_PLANE_TO_NORMAL(Vz, PlaneEq);

    MatGenMatTrans(PlaneEq[0] * PlaneEq[3],
		   PlaneEq[1] * PlaneEq[3],
		   PlaneEq[2] * PlaneEq[3],
		   Mat1);
    GMGenMatrixZ2Dir(Mat2, Vz);
    MatTranspMatrix(Mat2, Mat2);		     /* Compute the inverse. */
    MatMultPtby4by4(PDest, PSrc, Mat1);
    MatMultPtby4by4(PDest, PDest, Mat2);
}
        
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Releases the memory allocated by GSTable.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTable:   Gaussian Sphere table to release its memory.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFreeGSTable(GMTableStruct *GSTable)
{
    GMEdgeStruct
        *PEdge = GSTable -> First;
    
    while (PEdge) {
        GMDeleteEdgeFromGS(GSTable ,PEdge);
        PEdge = GSTable -> First;
    }

    IritFree(GSTable);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Releases the memory allocated by Plane.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:  Plane to release its memory.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFreePlane(GMPlaneStruct *Plane)
{
    GMEdgeStruct
        *PCell = Plane -> First;
    
    while (PCell) {
        GMDeleteEdgeFromPlane(Plane, PCell);
        PCell = Plane -> First;
    }    
    IritFree(Plane);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Removes one edge from the given plane.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:   To remove the edge from.                                        *
*   Edge:    To remove.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     If found and removed, FALSE otherwise.                          *
*****************************************************************************/
static int GMDeleteEdgeFromPlane(GMPlaneStruct *Plane, GMEdgeStruct *Edge)
{
    GMEdgeStruct 
        *EdgeLast = NULL,
        *PEdge = Plane -> First;
    
    while (PEdge != NULL) {
        if  (PEdge == Edge) {
	    if (EdgeLast == NULL)
	        Plane -> First = PEdge -> Pnext;
	    else
	        EdgeLast -> Pnext = PEdge -> Pnext;
	    IritFree(PEdge);
	    return TRUE;
        }
        EdgeLast = PEdge; 
        PEdge = PEdge -> Pnext;
    }
    return FALSE;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Removes an edge from the Gaussian Sphere table.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTable:   The Gaussian Sphere table.                                    *
*   Edge:      The edge to remove.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if found and removed, FALSE otherwise.                   *
*****************************************************************************/
static int GMDeleteEdgeFromGS(GMTableStruct *GSTable,
			      GMEdgeStruct *Edge)
{
    GMEdgeStruct 
        *GSEdgeLast = NULL,
        *GSEdge = GSTable -> First;
    
    while (GSEdge != NULL) {
        if  (GSEdge == Edge) {
	    if (GSEdgeLast == NULL)
	        GSTable -> First = GSEdge -> Pnext;
	    else
	        GSEdgeLast -> Pnext = GSEdge -> Pnext;
	    IritFree(GSEdge);
	    return TRUE;
        }
        GSEdgeLast = GSEdge; 
        GSEdge = GSEdge -> Pnext;
    }
    return FALSE;    
}
             
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts an edge into the Gaussian Sphere.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTable:    The Gaussian Sphere Table.                                   *
*   V:          First Vertex of edge in Euclidean space.                     *
*   P1, P2:     The two end normal of the edge as points on Gaussian Sphere. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMInsertEdgeInGS(GMTableStruct *GSTable,
			     IPVertexStruct *V,
			     IrtPtType P1,
			     IrtPtType P2)
{
    GMEdgeStruct
	*PCell = (GMEdgeStruct *) IritMalloc(sizeof(GMEdgeStruct));
  
    PCell -> ObjectEdge = V;
    IRIT_PT_COPY(PCell -> P1, P1);
    IRIT_PT_COPY(PCell -> P2, P2);
      
    PCell -> Pnext = NULL; 

    /* The new edge is inserted as the next of the last cell. */
    if (GSTable -> length == 0)			      /* The table is empty. */
	GSTable -> First = PCell;
    else
        GSTable->Last->Pnext = PCell;

    /* The new edge is inserted as the last one in the table. */
    GSTable->Last =  PCell;
    GSTable->length ++;
}
   
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts the segment [P1 P2] associated with the edge EdgeObj	     *
* in the Plane which is then tarnsform to the base plane.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:     The plane to substitute the edge into.                        *
*   P1, P2:    The segment of the arc on the Gaussian Sphere.                *
*   EdgeObj:   The edge to insert.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMInsertSegmentInPlane(GMPlaneStruct *Plane,
				   IrtPtType P1,
				   IrtPtType P2,
				   IPVertexStruct *EdgeObj)
{
    GMEdgeStruct
	*PCell = (GMEdgeStruct *) IritMalloc(sizeof(GMEdgeStruct));

    PCell -> ObjectEdge = EdgeObj;
    IRIT_PT_COPY(PCell -> P1, P1);
    IRIT_PT_COPY(PCell -> P2, P2);    
    PCell -> Pnext = NULL; 

    /* The new edge is inserted as the next of the last cell. */
    if (Plane -> length == 0)			      /* The table is empty. */
        Plane -> First = PCell;
    else
        Plane -> Last ->  Pnext = PCell;

    /* The new edge becomes the last one in the plane. */
    Plane -> Last = PCell;
    Plane -> length++;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Appends the two lists.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTableDest:   Where to append to.                                       *
*   GSTableSrc:    Where to append from.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMAppendGSTable(GMTableStruct *GSTableDest,
			    GMTableStruct *GSTableSrc)
{
    if (GSTableSrc != NULL) {
        if (GSTableDest -> length == 0) {
	    GSTableDest -> First = GSTableSrc -> First;
	    GSTableDest -> Last = GSTableSrc -> Last;
	    GSTableDest -> length = GSTableSrc -> length;
        }	 
        else {
	    GSTableDest -> Last -> Pnext = GSTableSrc -> First;
	    GSTableDest -> Last = GSTableSrc -> Last;
	    GSTableDest -> length += GSTableSrc -> length;
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds an orthogonal vector to the given vector.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   VRes:   The created orthogonal vector.                                   *
*   VSrc:   The original vector                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFindOrthVector(IrtVecType VRes, IrtVecType VSrc)
{
    if (IRIT_APX_EQ(VSrc[0], 0.0)) {
        VRes[0] = 1;
        VRes[1] = 0;
        VRes[2] = 0;
    }
    else if (IRIT_APX_EQ(VSrc[1], 0.0)) {
        VRes[0] = 0;
        VRes[1] = 1;
        VRes[2] = 0;
    }
    else if (IRIT_APX_EQ(VSrc[2], 0.0)) {
        VRes[0] = 0;
        VRes[1] = 0;
        VRes[2] = 1;
    }
    else {
        VRes[0] = -VSrc[1] / VSrc[0];
        VRes[1] = 1;
        VRes[2] = 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates one 2D cell.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMCell2DStruct *:   Allocated cell.                                      *
*****************************************************************************/
static GMCell2DStruct *GMGenEmptyCell2D(void)
{
    GMCell2DStruct
	*PCell = (GMCell2DStruct *) IritMalloc(sizeof(GMCell2DStruct));

    PCell -> Pnext = NULL;
    PCell -> Edge = NULL;

    return PCell;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates one 2D plane.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPlane2DStruct *:  Allocated plane.                                     *
*****************************************************************************/
static GMPlane2DStruct *GMGenEmptyPlane2D(void)
{
    GMPlane2DStruct
	*Plane = (GMPlane2DStruct *) IritMalloc(sizeof(GMPlane2DStruct));

    Plane -> First = NULL;

    return Plane;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocated one edge out of the given two 2D points.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:   End points of edge.                                            *
*   EdgeObj:  Vertex generated this edge (with V->Pnext).                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMEdge2DStruct *:   Allocated edge.                                      *
*****************************************************************************/
static GMEdge2DStruct *GMGenEdge2D(GMPoint2DType P1,
				   GMPoint2DType P2,
				   IPVertexStruct *EdgeObj)
{
    GMEdge2DStruct
	*PCell = (GMEdge2DStruct *) IritMalloc(sizeof(GMEdge2DStruct));

    if (GlblOrigPolyObjAlive) {
	PCell -> Vrtx1 = EdgeObj;
	PCell -> Vrtx2 = EdgeObj -> Pnext;
    }
    else {
	PCell -> Vrtx1 = PCell -> Vrtx2 = NULL;
    }

    IRIT_PT_COPY(PCell -> V1, EdgeObj -> Coord);
    IRIT_PT_COPY(PCell -> V2, EdgeObj -> Pnext -> Coord);    
    PCell -> VisitMarker = 0;
    IRIT_UV_COPY(PCell -> Seg[0], P1);
    IRIT_UV_COPY(PCell -> Seg[1], P2);    

    return PCell;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocate one grid structure.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   n:      Size of gird (n x n).                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMGridType:  Allocated grid.                                             *
*****************************************************************************/
static GMGridType GMGenEmptyGrid(int n)
{
    int i, j;
    GMGridType
	Res = (GMGridType) IritMalloc(n * sizeof(GMPlane2DStruct **));

    for (i = 0; i < n; i++) { 
        Res[i] = (GMPlane2DStruct **) IritMalloc(n * sizeof(GMPlane2DStruct *));

        for (j = 0; j < n; j++)
	    Res[i][j] = NULL;
    }
    return Res;
}    

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts one segments into the existing polyline object.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PPolyObj:       Polyline object to insert a new polyline (of one edge).  *
*   Coord1, Coord2: Two coordinates of the new polyline.                     *
*   Vrtx1, Vrtx2: Vertices of original object's edge if GlblOrigPolyObjAlive *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMInsertSegInPolylineObj(IPObjectStruct *PPolyObj,
				     IrtPtType Coord1,
				     IPVertexStruct *Vrtx1,
				     IrtPtType Coord2,
				     IPVertexStruct *Vrtx2)
{
    IPPolygonStruct *NewSegment;
    IPVertexStruct *V1, *V2;

    if (!IP_IS_POLYLINE_OBJ(PPolyObj))
        GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYLINE);

    V2 = IPAllocVertex(0, NULL, NULL);
    V1 = IPAllocVertex(0, NULL, V2);

    NewSegment = IPAllocPolygon(0, V1, PPolyObj -> U.Pl);
    PPolyObj -> U.Pl = NewSegment;
    IRIT_PT_COPY(V1 -> Coord, Coord1);
    IRIT_PT_COPY(V2 -> Coord, Coord2);

    if (Vrtx1 != NULL && Vrtx2 != NULL) {
        V1 -> Attr = IP_ATTR_COPY_ATTRS(Vrtx1 -> Attr);
	V2 -> Attr = IP_ATTR_COPY_ATTRS(Vrtx2 -> Attr);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts an edge into a 2D plane.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:   Where the edge should be inserted.                              *
*   Edge:    Edge to be inserted.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMInsertEdgeInPlane2D(GMPlane2DStruct *Plane,
				  GMEdge2DStruct *Edge)
{
    GMCell2DStruct
        *Cell = GMGenEmptyCell2D();
    Cell -> Edge = Edge;

    if (Plane == NULL) 
        GEOM_FATAL_ERROR(GEOM_ERR_PROJ_FAILED);

    if (Plane -> First != NULL)
        Cell -> Pnext = Plane -> First;
    Plane -> First = Cell;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a plane into a 2D plane.                                        *
* This function can be optimized by visiting only once the edges.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:      On which Line is on.                                         *
*   Line:       A line on the plane.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPlane2DStruct *:   The constructed 2D plane.                           *
*****************************************************************************/
static GMPlane2DStruct *GMPlane2Plane2D(GMPlaneStruct *Plane,
					GMSegment2DType Line)
{
     GMPlane2BasePlane(Plane);
     return GMBasePlane2Plane2D(Plane, Line);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a line on a base plane to a 2D plane.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:      Base plane.                                                  *
*   Line:       A line on the plane.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPlane2DStruct *:  The constructed 2D plane.                            *
*****************************************************************************/
static GMPlane2DStruct *GMBasePlane2Plane2D(GMPlaneStruct *Plane,
					    GMSegment2DType Line)
{
    GMPoint2DType P1, P2;
    GMEdgeStruct
        *PCell = Plane -> First;
    GMPlane2DStruct
        *Plane2D = NULL;

    if (PCell) 
        Plane2D = GMGenEmptyPlane2D();
    
    while (PCell) {
        GMEdge2DStruct *Edge;
        
        IRIT_UV_COPY(P1, PCell -> P1);
        IRIT_UV_COPY(P2, PCell -> P2);
        /* Generates an edge which will be shared by the subdivided planes. */
        Edge = GMGenEdge2D(P1, P2, PCell -> ObjectEdge);

        /* Insert it in this plane. */
        GMInsertEdgeInPlane2D(Plane2D, Edge);
        PCell = PCell -> Pnext;
    }

    IRIT_UV_COPY(Line[0], Plane -> Line[0]);
    IRIT_UV_COPY(Line[1], Plane -> Line[1]);

    return Plane2D;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the GS data set into a Grid.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   GSTable:    Gaussian sphere data structure.                              *
*   n:          Resolution of grid.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMGridStruct *:   Constructed grid.                                      *
*****************************************************************************/
static GMGridStruct *GMGsToGrid(GMTableStruct *GSTable, int n)
{
    GMPlaneStruct **Planes2;
    GMSegment2DType Line;
    int i;
    GMGridStruct
        *Res = (GMGridStruct *) IritMalloc(sizeof(GMGridStruct));

    Res -> NumSubdivisions = n;
    Planes2 = GMGenCubePlanes();
    GMGSProjectedIntoPlanes(GSTable, Planes2);/* Projects GS into the cubes. */

    Res -> PermanentSegs = GSTable;
    for (i = 0; i < GM_NUM_PLANES; i++) {
        if ((Res -> EdgesPlane2D[i] = GMPlane2Plane2D(Planes2[i], Line))
								       == NULL)
	    Res -> EdgesPlane2D[i] = GMGenEmptyPlane2D();
        Res -> Grid[i] = GMBuildSubdividedPlanes(Res -> EdgesPlane2D[i], n);
        GMFreePlane(Planes2[i]);
    }

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the planes of the cube.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPlaneStruct **:   Constructed planes.                             *
*****************************************************************************/
static GMPlaneStruct **GMGenCubePlanes(void) 
{
    int i, j, k;
    GMPlaneStruct **Planes;
    IrtPlnType PlaneX1, PlaneY1, PlaneZ1, PlaneX2, PlaneY2, PlaneZ2;
    IrtPtType P;

    Planes = (GMPlaneStruct **)
		IritMalloc(GM_NUM_PLANES * sizeof(GMPlaneStruct *));
    
    PlaneX1[2] = PlaneX2[2] = PlaneX1[1] = PlaneX2[1] = 0.0;
    PlaneX1[0] = PlaneX2[0] = 1.0;
    PlaneX1[3] = -1.0;PlaneX2[3] = 1.0;
    
    PlaneY1[0] = PlaneY2[0] = PlaneY1[2] = PlaneY2[2] = 0.0;
    PlaneY1[1] = PlaneY2[1] = 1.0;
    PlaneY1[3] = -1.0;PlaneY2[3] = 1.0;
    
    PlaneZ1[0] = PlaneZ2[0] = PlaneZ1[1] = PlaneZ2[1] = 0.0;
    PlaneZ1[2] = PlaneZ2[2] = 1.0;
    PlaneZ1[3] = -1.0;PlaneZ2[3] = 1.0;
    

    /* Generates the planes. */
    Planes[0] = GMGenEmptyPlane(PlaneX1);
    Planes[1] = GMGenEmptyPlane(PlaneX2);
    Planes[2] = GMGenEmptyPlane(PlaneY1);
    Planes[3] = GMGenEmptyPlane(PlaneY2);
    Planes[4] = GMGenEmptyPlane(PlaneZ1);
    Planes[5] = GMGenEmptyPlane(PlaneZ2);


    /* Generates the bounding planes for each plane. */
    for (i = 0; i < GM_NUM_PLANES; i++)
        for (j = 0; j < GM_NUM_PLANES; j++)
	    Planes[i] -> BoundPlanes[j] = Planes[j];        
    Planes[0] -> BoundPlanes[0] = NULL;
    Planes[0] -> BoundPlanes[1] = NULL;
    Planes[1] -> BoundPlanes[0] = NULL;
    Planes[1] -> BoundPlanes[1] = NULL; 
    Planes[2] -> BoundPlanes[2] = NULL;
    Planes[2] -> BoundPlanes[3] = NULL; 
    Planes[3] -> BoundPlanes[2] = NULL; 
    Planes[3] -> BoundPlanes[3] = NULL; 
    Planes[4] -> BoundPlanes[4] = NULL; 
    Planes[4] -> BoundPlanes[5] = NULL; 
    Planes[5] -> BoundPlanes[4] = NULL; 
    Planes[5] -> BoundPlanes[5] = NULL;

    /* Generates the bounding segments for each plane. */
    for (i = 0; i < GM_NUM_PLANES; i++) {
	for (j = 0; j < GM_NUM_PLANES; j++) {
	    if (Planes[i] -> BoundPlanes[j]) {
		for (k = j + 1; k < GM_NUM_PLANES; k++) {
		    if (Planes[i] -> BoundPlanes[k]) {
			if (GMPointFrom3Planes(Planes[i] -> PlaneEq,
					Planes[i] -> BoundPlanes[j] -> PlaneEq,
					Planes[i] -> BoundPlanes[k] -> PlaneEq,
					P)) {
			    IRIT_PT_COPY(Planes[i] -> BoundSegs[j][1],
				    Planes[i] -> BoundSegs[j][0]);
			    IRIT_PT_COPY(Planes[i] -> BoundSegs[j][0], P);
			    IRIT_PT_COPY(Planes[i] -> BoundSegs[k][1],
				    Planes[i] -> BoundSegs[k][0]);
			    IRIT_PT_COPY(Planes[i] -> BoundSegs[k][0], P);
			}
		    }
		}
	    }
	}
    }

    return Planes;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Builds from a Plane with a set of segments, the equivalent grid.         *
*   Only the cells that are intersected by the segments are created.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:   Initial Plane with a set of segments.                           *
*   n:       Resolution of  subdivion that is requested for the Grid.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMGridType: The constructed grid.                                        *
*****************************************************************************/
static GMGridType GMBuildSubdividedPlanes(GMPlane2DStruct *Plane, int n)
{
    int i,
        j = 0;
    GMGridType Res;
    GMCell2DStruct *PCell; 
        
    Res = GMGenEmptyGrid(n);

    PCell = Plane -> First;

    while (PCell && PCell -> Edge) {
        int k = 0;
        GMIndicesType
	    *IndicesOfPlanes = GMReportInterPlanes(PCell -> Edge -> Seg, n);

	if (IndicesOfPlanes != NULL) {
	    while ((i = IndicesOfPlanes[k][0]) != -1 &&
		   (j = IndicesOfPlanes[k][1]) != -1) {
	        if (!Res[i][j])
		    Res[i][j] = GMGenEmptyPlane2D();
		GMInsertEdgeInPlane2D(Res[i][j], PCell -> Edge);
		k++;
	    }
	    IritFree(IndicesOfPlanes);
	}

        PCell = PCell -> Pnext;
    }
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reports the segments in the Grid that are intersected by a Line.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Grid:   Grid with the set of segments.                                   *
*   Line:   Query Line.                                                      *
*   n:      Resolution of the Grid.                                          *
*   PObj:   Object to place the reported segments.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMReportInterSegsOpti(GMGridType Grid,
				  GMSegment2DType Line,
				  int n,
				  IPObjectStruct *PObj)
{
    int i;
    GMIndicesType *IndicesOfPlanes;
    GMPlane2DStruct *Plane;

    if (IRIT_PT_APX_EQ_E2(Line[0],Line[1]))
        return ;
    IndicesOfPlanes = GMReportInterPlanes(Line, n);
    for (i=0; IndicesOfPlanes[i][0] != -1; i++) {
        if ((Plane = Grid[IndicesOfPlanes[i][0]][IndicesOfPlanes[i][1]]) != FALSE) {
	    GMReportInterSegs(Plane, Line, PObj);
        }
    }

    IritFree(IndicesOfPlanes);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reports segments in the Plane intersected by Line.                       *
*   Inserts result in PObj.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:  Plane with his segment set.                                      *
*   Line:   Query Line.                                                      *
*   PObj:   Object to insert the result.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMReportInterSegs(GMPlane2DStruct *Plane,
			      GMSegment2DType Line,
			      IPObjectStruct *PObj)
{
    IrtRType Angle10, Angle11;
    GMCell2DStruct *PCell;
    GMPoint2DType V00, V01, V10, V11;
        
    PCell = Plane -> First;
    while (PCell) {
        /* We visits this edge only if it hasn't been done during this       */
	/* query request.						     */
        if (PCell -> Edge -> VisitMarker != GlblNumQueryRequest) {
	    PCell -> Edge -> VisitMarker  = GlblNumQueryRequest;
	    GM_IRIT_PT2D_SUB(V00, PCell -> Edge -> Seg[0], Line[0]);
	    GM_IRIT_PT2D_SUB(V01, PCell -> Edge -> Seg[0], Line[1]);
	    GM_IRIT_PT2D_SUB(V10, PCell -> Edge -> Seg[1], Line[0]);
	    GM_IRIT_PT2D_SUB(V11, PCell -> Edge -> Seg[1], Line[1]);
	  
	    /* Segment intersected. */
	    Angle10 = GM_IRIT_CROSS_PROD_2D(V00,V01);
	    Angle11 = GM_IRIT_CROSS_PROD_2D(V10,V11);
	    if (Angle10 * Angle11 <= 0 ||
		(Angle10 == 0 && Angle11 != 0) ||
		(Angle10 != 0 && Angle11 == 0)) {
	        GMInsertSegInPolylineObj(PObj,
					 PCell -> Edge -> V1,
					 PCell -> Edge -> Vrtx1,
					 PCell -> Edge -> V2,
					 PCell -> Edge -> Vrtx2);
	    }
        }
        PCell = PCell -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reports the planes of the grid intersected by a segment.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Seg:  Query Segment.                                                     *
*   n:    Subdivision of the Grid.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMIndicesType *: List of indices that gives the location in the grid of  *
*                    the planes intersected.                                 *
*****************************************************************************/
static GMIndicesType *GMReportInterPlanes(GMSegment2DType Seg, int n)
{
    int IndexX0, IndexX1, IndexX, IndexY0, IndexY1, IndexY;
    int TabLength, i;                                 /* Size of the result. */
    IrtRType
	RangeLength = (Range[1] - Range[0]) / n;
    GMPoint2DType Dir;
    GMIndicesType *Res;
    int DirXNeg, DirXNul, DirXPos, DirYNeg, DirYNul, DirYPos;
  
    /* Dir is the direction verctor of segment Seg. */
    GM_IRIT_PT2D_SUB(Dir, Seg[1], Seg[0]);

    /* Determines the sense of direction. */
    DirXNeg = DirXNul = DirXPos = DirYNeg = DirYNul = DirYPos = FALSE;
    if (IRIT_APX_EQ(Dir[0],0))
        DirXNul = TRUE;
    else if (Dir[0] < 0)
        DirXNeg = TRUE;
    else
        DirXPos = TRUE;

    if (IRIT_APX_EQ(Dir[1],0))
        DirYNul = TRUE;
    else if (Dir[1] < 0)
        DirYNeg = TRUE;
    else
        DirYPos = TRUE;
        
    /* Allocates memory for the result. */
    TabLength = (int) (IRIT_FABS(Seg[1][0] - Seg[0][0]) / RangeLength) +
                (int) (IRIT_FABS(Seg[1][1] - Seg[0][1]) / RangeLength) + 4;

    /* Assigns values to IndexX0, IndexX1,IndexY0, IndexY1. */
    if (!GMCoord2Index(&IndexX0, Range, RangeLength, Seg[0][0])) {
        if (DirXNul) {
	    /* The point Seg[0] is on a vertical line shared by 2 planes. */
	    if (IndexX0 == n)
	        IndexX0--;
        }
        else if (DirXNeg)
	    IndexX0 --;
    }

    if (!GMCoord2Index(&IndexY0, Range, RangeLength, Seg[0][1])) {
        if (DirYNul) {
	    /* The point Seg[0] is on an horizontal line shared by 2 planes. */
	    if (IndexY0 == n)
	        IndexY0 --;
        }
        else if (DirYNeg)
	    IndexY0 --;
    }

    if (!GMCoord2Index(&IndexX1, Range, RangeLength, Seg[1][0])) {
        if (DirXNul) {
	    /* The point Seg[1] is on a vertical line shared by 2 planes. */
	    if (IndexX1 == n)
	        IndexX1 --;
        }
        else if (DirXPos)
	    IndexX1 --;
    }

    if (!GMCoord2Index(&IndexY1, Range, RangeLength, Seg[1][1])) {
        if (DirYNul) {
	    /* The point Seg[1] is on an horizontal  line shared by 2 planes. */
	    if (IndexY1 == n)
	        IndexY1 --;
        }
        else if (DirYPos)
	    IndexY1 --;
    }

    if (IndexX0 == GM_IVALID_INDEX || IndexX1 == GM_IVALID_INDEX ||
	IndexY0 == GM_IVALID_INDEX || IndexY1 == GM_IVALID_INDEX)
        return NULL;

    i = 0;
    Res = (GMIndicesType *) IritMalloc((1 + TabLength) * sizeof(GMIndicesType));

    /* Particular case where segment is very small then we don't split it. */
    if (DirXNeg) {
        if ((IndexX1 - IndexX0) > 0) {
	    IndexX1 = IndexX0;
	    DirXNul = TRUE;
	    DirXNeg = FALSE;
        }
    }
    else if (DirXPos) {
        if ((IndexX0 - IndexX1) > 0) {
	    IndexX0 = IndexX1;
	    DirXNul = TRUE;
	    DirXPos = FALSE;
        }
    }
    else if (DirXNul) {
        if (IndexX0 < IndexX1)
	    IndexX1 = IndexX0;
        else if (IndexX1 < IndexX0)
	    IndexX0 = IndexX1;
    }  

    if (DirYNeg) {
        if ((IndexY1 - IndexY0) > 0) {
	    IndexY1 = IndexY0;
	    DirYNul = TRUE;
	    DirYNeg = FALSE;
        }
    }
    else if (DirYPos) {
        if ((IndexY0 - IndexY1) > 0) {
	    IndexY0 = IndexY1;
	    DirYNul = TRUE;
	    DirYPos = FALSE;
        }
    }
    else if (DirYNul) {
        if (IndexY0 < IndexY1)
	    IndexY1 = IndexY0;
        else if (IndexY1 < IndexY0)
	    IndexY0 = IndexY1;
    }
    
    Res[i][0] = IndexX0;
    Res[i][1] = IndexY0;
    IndexX = IndexX0;
    IndexY = IndexY0;

    if (IndexX0 == IndexX1) {
        while (IndexY != IndexY1) {
	    i++;
	    DirYPos ? IndexY++ : IndexY--;
	    Res[i][0] = IndexX;
	    Res[i][1] = IndexY;
        }
    }
    else if (IndexY0 == IndexY1) {
        while (IndexX != IndexX1) {
	    i++;
	    DirXPos ? IndexX++ : IndexX--;
	    Res[i][0] = IndexX;
	    Res[i][1] = IndexY;
        }
    }
    else {
        IrtRType a, b, Delta, V, H;
        int Even = ((DirXPos && DirYPos) || (DirXNeg && DirYNeg));
  
        a = Range[0] + (IndexX0 + DirXPos) * RangeLength - Seg[0][0];
        b = Range[0] + (IndexY0 + DirYPos) * RangeLength - Seg[0][1];
        
        Delta = a * Dir[1] - b * Dir[0];
        
        H = - Dir[0] * RangeLength;
        V = Dir[1] * RangeLength;
        
        while ((IndexX != IndexX1) || (IndexY != IndexY1)) {
	    i++;
	    if ((Even && (Delta >= 0)) || (!Even && (Delta <= 0))) {
	        if (DirYPos) {
		    IndexY++;
		    Delta += H;
		}
		else {
		    IndexY--;
		    Delta -= H;
		}
	    }
	    else if ((!Even && (Delta > 0)) || (Even && (Delta < 0))) {
	        if (DirXPos) {
		    IndexX++;
		    Delta += V;
		}
		else {
		    IndexX--;
		    Delta -= V;
		}
	    }

	    if (i >= TabLength) {
	        i--;
		break;
	    }
	    else if (IndexX >= n || IndexX < 0 || IndexY >= n || IndexX < 0) {
	        Res[i][0] = -1;
	    }
	    else {
	        Res[i][0] = IndexX;
		Res[i][1] = IndexY;
	    }
        } 
    }

    /* Free the planes that are not used. */
    i++;
    while (i < TabLength + 1) {
        Res[i][0] = -1;
        Res[i][1] = -1;
        i++;
    }

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Translates a real coordinate into a grid index.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Index:        Index Result.                                              *
*   Range:        Range of the real scale.                                   *
*   RangeLength:  Length of one cell.                                        *
*   P:            Real Coordinate to transate.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  return FALSE if real coordinate is exactly on a separation line    *
*         of the cells.                                                      *
*****************************************************************************/
static int GMCoord2Index(int *Index,
			 GMRangeType Range,
			 IrtRType RangeLength,
			 IrtRType P)
{
    IrtRType Sum;
    
    *Index = 0;
    
    if (!GMIsInRange(P, Range)) {
        *Index = GM_IVALID_INDEX;
	return TRUE;
    }

    Sum = Range[0];
    while (Sum < P) {
        if (IRIT_APX_EQ(Sum, P))
	    return FALSE;
        Sum += RangeLength;
        (*Index)++;
    }
    if (IRIT_APX_EQ(Sum, P))
        return FALSE;
    else {
        (*Index)--;
        return TRUE;
    }
}
    
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifies that P is indeed in range.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   P:       Parameter to verify in range.                                   *
*   Range:   Range to verify against.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if inside, FALSE otherwise.                                 *
*****************************************************************************/
static int GMIsInRange(IrtRType P, GMRangeType Range)
{
    return (P < Range[1] || IRIT_APX_EQ(P,Range[1])) &&
	   (P > Range[0] || IRIT_APX_EQ(P,Range[0]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Frees the planes in the grid structure.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Grids:     To free its planes.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFreeSubdivPlanes(GMGridStruct *Grids)
{
    int i, j, k;

    for (i = 0; i < GM_NUM_PLANES; i++) {
        /* Free the subdivided 2D planes created for the plane i. */
        for (k = 0; k < Grids -> NumSubdivisions; k++) {
	    for (j = 0; j <  Grids -> NumSubdivisions; j++) {
	        if (Grids -> Grid[i][k][j] != NULL)
		    GMFreePlane2D(Grids -> Grid[i][k][j]);
	  }
	  IritFree(Grids -> Grid[i][k]);
        }
        IritFree(Grids -> Grid[i]);
   }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free a 2D plane structure.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:     To free.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMFreePlane2D(GMPlane2DStruct *Plane)
{
    GMCell2DStruct *PCellLast,
        *PCell = Plane -> First;

    while (PCell) {
        PCellLast = PCell;
        PCell = PCell -> Pnext;
        IritFree(PCellLast);
    }
    IritFree(Plane);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Delete the edges in a 2D plane.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Plane:    To free its edges.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMDelEdges2DFromPlane2D(GMPlane2DStruct *Plane)
{
    GMCell2DStruct
        *PCell = Plane -> First;    

    while (PCell && PCell -> Edge) {
	  IritFree(PCell -> Edge);
	  PCell = PCell -> Pnext;
    }
}      
