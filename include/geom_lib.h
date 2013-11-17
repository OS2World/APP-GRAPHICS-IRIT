/******************************************************************************
* GeomLib.h - header file of the geometry library.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 1996.					      *
******************************************************************************/

#ifndef	GEOM_LIB_H
#define GEOM_LIB_H

#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "bool_lib.h"
#include "ip_cnvrt.h"

typedef enum {
    GEOM_ERR_NO_OGL_SUPPORT,
    GEOM_ERR_OGL_NO_X_SERVER,
    GEOM_ERR_X_NO_OGL_SERVER,
    GEOM_ERR_X_NO_VISUAL,
    GEOM_ERR_X_NO_GCONTEXT,
    GEOM_ERR_CH_STACK_OVERFLOW,
    GEOM_ERR_CH_STACK_UNDERFLOW,
    GEOM_ERR_NO_INSTANCE_ORIGIN,
    GEOM_ERR_ANIM_MAT_OR_CRV,
    GEOM_ERR_UNKNOWN_ANIM_CRVS,
    GEOM_ERR_NO_ANIM_CRVS,
    GEOM_ERR_UNEQUAL_NUM_OF_POLYS,
    GEOM_ERR_UNEQUAL_NUM_OF_VRTXS,
    GEOM_ERR_TOO_MANY_ADJACENCIES,
    GEOM_ERR_NO_IRIT_PATH,
    GEOM_ERR_INVALID_FONT,
    GEOM_ERR_MSC_TOO_FEW_PTS,
    GEOM_ERR_MSC_COLIN_CIRC,
    GEOM_ERR_TRIANGLES_ONLY,
    GEOM_ERR_INVALID_POLYGON,
    GEOM_ERR_VRTX_MTCH_FAILED,
    GEOM_ERR_EXPCT_POLYHEDRA,
    GEOM_ERR_EXPCT_POLYLINE,
    GEOM_ERR_EXPCT_LIST_OBJ,
    GEOM_ERR_EXPCT_TWO_PTS,
    GEOM_ERR_PROJ_FAILED,
    GEOM_ERR_DECIM_BDRY_FAILED,
    GEOM_ERR_OPEN_OBJ_VOL_COMP,
    GEOM_ERR_NO_INV_MAT,
    GEOM_ERR_NO_POLY_PLANE,
    GEOM_ERR_NO_VRTX_NRML,
    GEOM_ERR_REGULAR_POLY,
    GEOM_ERR_REORIENT_STACK_OF,
    GEOM_ERR_DISJOINT_PARTS,
    GEOM_ERR_VRTX_MUST_HAVE_NRML,
    GEOM_ERR_MISS_VRTX_IDX,
    GEOM_ERR_CMPLX_T_JUNC,

    GEOM_ERR_UNDEFINE_ERR
} GeomFatalErrorType;

typedef enum {
    GM_FIT_OTHER = -1,
    GM_FIT_PLANE = 0,
    GM_FIT_SPHERE,
    GM_FIT_CYLINDER,
    GM_FIT_CIRCLE,
    GM_FIT_CONE,
    GM_FIT_TORUS
} GMFittingModelType;

typedef enum {
    GM_GEN_PRIM_POLYS = 0,
    GM_GEN_PRIM_SRFS,
    GM_GEN_PRIM_MDLS
} GMGenPrimType;

#define GM_FIT_MODEL_MAX_PARAM 10

/* Used by the Ray & Polygon intersection (Jordan theorem): */
#define GM_BELOW_RAY		1
#define GM_ON_RAY		2
#define GM_ABOVE_RAY		3
#define GM_ANIM_DEFAULT_FILE_NAME	"IAnim"
#define PRIM_MIN_RESOLUTION	4

#define GM_ANIM_NO_DEFAULT_TIME IRIT_INFNTY

#define GM_QUAT_COPY(SrcQ, DstQ) IRIT_GEN_COPY(DstQ, SrcQ, sizeof(GMQuatType))

typedef struct GMAnimationStruct {
    IrtRType
	StartT,		                     /* Starting time of animation. */
	FinalT,		                  /* Termination time of animation. */
	Dt,		                         /* Step size pf animation. */
	RunTime;		              /* Current time of animation. */
    int TwoWaysAnimation,   /* Should the animation bounce back and forth!? */
	SaveAnimationGeom,          /* Save animation geometry into files!? */
	SaveAnimationImage,           /* Save animation images into files!? */
	BackToOrigin,	           /* Should we terminate at the beginning? */
	NumOfRepeat,			            /* How many iterations? */
	StopAnim,		   /* If TRUE, must stop the animation now. */
	SingleStep,			     /* Are we in single step mode? */
	TextInterface,		/* Are we communicating using a textual UI? */
	MiliSecSleep,	   /* How many miliseconds to sleep between frames. */
	_Count;						/* Used internally. */
    const char
	*ExecEachStep;		      /* Program to execute each iteration. */
    char
	BaseFileName[IRIT_LINE_LEN]; /* Base name of animation files saved. */
} GMAnimationStruct;

typedef struct GMBBBboxStruct {
    IrtRType Min[3];
    IrtRType Max[3];
} GMBBBboxStruct;

#define GM_BBOX_HOLD_PT(Pt, BBox, Eps)   /* Point in BBox to within Eps. */ \
    ((Pt)[0] >= (BBox) -> Min[0] - (Eps) && \
     (Pt)[0] <= (BBox) -> Max[0] + (Eps) && \
     (Pt)[1] >= (BBox) -> Min[1] - (Eps) && \
     (Pt)[1] <= (BBox) -> Max[1] + (Eps) && \
     (Pt)[2] >= (BBox) -> Min[2] - (Eps) && \
     (Pt)[2] <= (BBox) -> Max[2] + (Eps))

typedef IrtRType GMLsPoint[3];   /* The Z component is pretty much ignored. */

typedef struct GMLsLineSegStruct {
    struct GMLsLineSegStruct *Pnext;
    GMLsPoint Pts[2];
    long Id;			   /* Lines with unique ID never intersect. */
    VoidPtr PAux;	   /* Auxiliary backpointer - not used by ln_sweep. */
    struct GMLsIntersectStruct *Inters;
    GMLsPoint _MinVals;			        /* Bounding box on the line */
    GMLsPoint _MaxVals;
    GMLsPoint _Vec;		    /* A vector from first point to second. */
    IrtRType _ABC[3];			   /* Line equation as Ax + By + C. */
} GMLsLineSegStruct;

typedef struct GMLsIntersectStruct {
    struct GMLsIntersectStruct *Pnext;
    IrtRType t;
    IrtRType OtherT;
    struct GMLsLineSegStruct *OtherSeg;
    long Id;				      /* Unique ID of intersection. */
} GMLsIntersectStruct;

typedef IrtRType (*GMPolyOffsetAmountFuncType)(IrtRType *Coord);

typedef enum {            /* Predefined indices for the TransformIrtVecType */
    GM_QUAT_ROT_X = 0,
    GM_QUAT_ROT_Y,
    GM_QUAT_ROT_Z, 
    GM_QUAT_TRANS_X,
    GM_QUAT_TRANS_Y,
    GM_QUAT_TRANS_Z, 
    GM_QUAT_SCALE
} GMQuatTransformationsType;

typedef enum {
    GM_ZBUF_Z_LARGER,
    GM_ZBUF_Z_LARGER_EQUAL,
    GM_ZBUF_Z_SMALLER,
    GM_ZBUF_Z_SMALLER_EQUAL,
    GM_ZBUF_Z_ALWAYS,
    GM_ZBUF_Z_NEVER
} GMZTestsType;

typedef	IrtRType GMQuatType[4];                            /* A Quaternion. */
typedef IrtRType GMQuatTransVecType[7];       /* Transformation parameters. */

typedef void (*GeomSetErrorFuncType)(GeomFatalErrorType);

typedef void (*GMZBufferUpdateFuncType)(VoidPtr ZbufferID, int x, int y);

typedef IrtRType (*GMDistEnergy1DFuncType)(IrtRType);

typedef void (*GMScanConvertApplyFuncType)(int x, int y);

typedef void (*GMTransObjUpdateFuncType)(const IPObjectStruct *OldPObj,
					 IPObjectStruct *NewPObj,
					 IrtHmgnMatType Mat,
					 int AnimUpdate);

typedef IrtRType (*GMFetchVertexPropertyFuncType)(IPVertexStruct *V,
						  IPPolygonStruct *Pl);

typedef void (* GMSphConeQueryCallBackFuncType)(IPVertexStruct *V);
typedef int (* GMSphConeQueryDirFuncType)(IrtVecType Vec, IrtRType ConeAngle);

typedef void (*GMPolyAdjacncyVertexFuncType)(IPVertexStruct *V1,
					     IPVertexStruct *V2,
					     IPPolygonStruct *Pl1,
					     IPPolygonStruct *Pl2);

typedef IPObjectStruct *(*GMTransObjUpdateAnimCrvsFuncType)(IPObjectStruct *PAnim,
							    IrtHmgnMatType Mat);
typedef int (*GMMergePolyVrtxCmpFuncType)(IPVertexStruct *V1,
					  IPVertexStruct *V2,
					  IrtRType Eps);

typedef void (*GMMergeGeomInitFuncType)(VoidPtr Entty);
typedef IrtRType (*GMMergeGeomDistFuncType)(VoidPtr Entty1, VoidPtr Entty2);
typedef IrtRType (*GMMergeGeomKeyFuncType)(VoidPtr Entty);
typedef int (*GMMergeGeomMergeFuncType)(void **Entty1, void **Entty2);

typedef void (*GMIdentifyTJunctionFuncType)(IPVertexStruct *V0,
					    IPVertexStruct *V1,
					    IPVertexStruct *V2,
					    IPPolygonStruct *Pl0,
					    IPPolygonStruct *Pl1,
					    IPPolygonStruct *Pl2);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER int _PrimGlblResolution;

/* And prototypes of the functions: */

IrtRType GMBasicSetEps(IrtRType Eps);
void GMVecCopy(IrtVecType Vdst, const IrtVecType Vsrc);
void GMVecNormalize(IrtVecType V);
IrtRType GMVecLength(const IrtVecType V);
void GMVecCrossProd(IrtVecType Vres, const IrtVecType V1, const IrtVecType V2);
IrtRType GMVecVecAngle(const IrtVecType V1,
		       const IrtVecType V2,
		       int Normalize);
IrtRType GMPlanarVecVecAngle(const IrtVecType V1,
			     const IrtVecType V2,
			     int Normalize);
int GMOrthogonalVector(const IrtVecType V, IrtVecType OV, int UnitLen);
int GMCollinear3Pts(const IrtPtType Pt1,
		    const IrtPtType Pt2,
		    const IrtPtType Pt3);
int GMCollinear3PtsInside(const IrtPtType Pt1,
			  const IrtPtType Pt2,
			  const IrtPtType Pt3);
int GMCoplanar4Pts(const IrtPtType Pt1,
		   const IrtPtType Pt2,
		   const IrtPtType Pt3,
		   const IrtPtType Pt4);
IrtRType GMVecDotProd(const IrtVecType V1, const IrtVecType V2);

IPObjectStruct *GMGenMatObjectRotX(const IrtRType *Degree);
IPObjectStruct *GMGenMatObjectRotY(const IrtRType *Degree);
IPObjectStruct *GMGenMatObjectRotZ(const IrtRType *Degree);
IPObjectStruct *GMGenMatObjectTrans(const IrtVecType Vec);
IPObjectStruct *GMGenMatObjectScale(const IrtVecType Vec);
IPObjectStruct *GMGetMatTransPortion(const IPObjectStruct *MatObj,
				     int TransPortion);
IPPolygonStruct *GMTransformPolyList(const IPPolygonStruct *Pls,
				     IrtHmgnMatType Mat,
				     int IsPolygon);
GMTransObjUpdateFuncType GMTransObjSetUpdateFunc(GMTransObjUpdateFuncType
								   UpdateFunc);
GMTransObjUpdateAnimCrvsFuncType GMTransObjSetAnimCrvUpdateFunc(
			      GMTransObjUpdateAnimCrvsFuncType AnimUpdateFunc);
IPObjectStruct *GMTransformObject(const IPObjectStruct *PObj,
				  IrtHmgnMatType Mat);
IPObjectStruct *GMTransformObjectList(const IPObjectStruct *PObj,
				      IrtHmgnMatType Mat);
IPObjectStruct *GMTransObjUpdateAnimCrvs(IPObjectStruct *PAnim,
					 IrtHmgnMatType Mat);
IPObjectStruct *GMGenMatObjectZ2Dir(const IrtVecType Dir);
IPObjectStruct *GMGenMatObjectZ2Dir2(const IrtVecType Dir,
				     const IrtVecType Dir2);
IPObjectStruct *GMGenMatObjectRotVec(const IrtVecType Vec,
				     const IrtRType *Degree);
IPObjectStruct *GMGenMatObjectV2V(const IrtVecType V1, const IrtVecType V2);
IPObjectStruct *GMGenMatrix3Pts2EqltrlTri(const IrtPtType Pt1,
					  const IrtPtType Pt2,
					  const IrtPtType Pt3);

/* General basic computational geometry routines: */

IrtRType GMDistPointPoint(const IrtPtType P1, const IrtPtType P2);
int GMFindLinComb2Vecs(const IrtVecType V1,
		       const IrtVecType V2,
		       const IrtVecType V,
		       IrtRType w[2]);
int GMLineFrom2Points(IrtLnType Line,
		      const IrtPtType Pt1, 
		      const IrtPtType Pt2);
void GMPointVecFromLine(const IrtLnType Line, IrtPtType Pt, IrtVecType Dir);
int GMPlaneFrom3Points(IrtPlnType Plane,
		       const IrtPtType Pt1,
		       const IrtPtType Pt2,
		       const IrtPtType Pt3);
void GMPointFromPointLine(const IrtPtType Point,
			  const IrtPtType Pl,
			  const IrtPtType Vl,
			  IrtPtType ClosestPoint);
IrtRType GMDistPointLine(const IrtPtType Point,
			 const IrtPtType Pl,
			 const IrtPtType Vl);
IrtRType GMDistPointPlane(const IrtPtType Point, const IrtPlnType Plane);
int GMPointFromPointPlane(const IrtPtType Pt,
			  const IrtPlnType Plane,
			  IrtPtType ClosestPoint);
int GMPointFromLinePlane(const IrtPtType Pl,
			 const IrtPtType Vl,
			 const IrtPlnType Plane,
			 IrtPtType InterPoint,
			 IrtRType *t);
int GMPointFromLinePlane01(const IrtPtType Pl,
			   const IrtPtType Vl,
			   const IrtPlnType Plane,
			   IrtPtType InterPoint,
			   IrtRType *t);
int GM2PointsFromLineLine(const IrtPtType Pl1,
			  const IrtPtType Vl1,
			  const IrtPtType Pl2,
			  const IrtPtType Vl2,
			  IrtPtType Pt1,
			  IrtRType *t1,
			  IrtPtType Pt2,
			  IrtRType *t2);
IrtRType GMDistLineLine(const IrtPtType Pl1,
			const IrtPtType Vl1,
			const IrtPtType Pl2,
			const IrtPtType Vl2);
int GMPointFrom3Planes(const IrtPlnType Pl1,
		       const IrtPlnType Pl2,
		       const IrtPlnType Pl3,
		       IrtPtType Pt);

IrtRType GMDistPolyPoly(const IPPolygonStruct *Pl1,
			const IPPolygonStruct *Pl2,
			IPVertexStruct **V1,
			IPVertexStruct **V2);

int GMPolygonPlaneInter(const IPPolygonStruct *Pl,
			const IrtPlnType Pln,
			IrtRType *MinDist);
int GMSplitPolygonAtPlane(IPPolygonStruct *Pl, const IrtPlnType Pln);
IrtRType GMPolyPlaneClassify(const IPPolygonStruct *Pl, const IrtPlnType Pln);

int GMTrianglePointInclusion(const IrtRType *V1,
			     const IrtRType *V2,
			     const IrtRType *V3,
			     const IrtPtType Pt);
int GMPolygonPointInclusion(const IPPolygonStruct *Pl, const IrtPtType Pt);
IrtRType GMAreaSphericalTriangle(const IrtVecType Dir1,
				 const IrtVecType Dir2,
				 const IrtVecType Dir3);
IrtRType GMAngleSphericalTriangle(const IrtVecType Dir,
				  const IrtVecType ODir1,
				  const IrtVecType ODir2);
int GMPolygonPointInclusion3D(const IPPolygonStruct *Pl, const IrtPtType Pt);

int GMPolygonRayInter(const IPPolygonStruct *Pl,
		      const IrtPtType PtRay,
		      int RayAxes);
int GMPolygonRayInter2(const IPPolygonStruct *Pl,
		       const IrtPtType PtRay,
		       int RayAxes,
		       IPVertexStruct **FirstInterV,
		       IrtRType *FirstInterP);
int GMPolygonRayInter3D(const IPPolygonStruct *Pl,
			const IrtPtType PtRay,
			int RayAxes);
IPPolygonStruct *GMPolyHierarchy2SimplePoly(IPPolygonStruct *Root,
					    IPPolygonStruct *Islands);
void GMGenTransMatrixZ2Dir(IrtHmgnMatType Mat,
			   const IrtVecType Trans,
			   const IrtVecType Dir,
			   IrtRType Scale);
void GMGenMatrixX2Dir(IrtHmgnMatType Mat, const IrtVecType Dir);
void GMGenMatrixY2Dir(IrtHmgnMatType Mat, const IrtVecType Dir);
void GMGenMatrixZ2Dir(IrtHmgnMatType Mat, const IrtVecType Dir);
void GMGenTransMatrixZ2Dir2(IrtHmgnMatType Mat,
			    const IrtVecType Trans,
			    const IrtVecType Dir,
			    const IrtVecType Dir2,
			    IrtRType Scale);
void GMGenMatrixZ2Dir2(IrtHmgnMatType Mat,
		       const IrtVecType Dir,
		       const IrtVecType Dir2);
int GMMatFromPosDir(const IrtPtType Pos,
		    const IrtVecType Dir,
		    const IrtVecType UpDir,
		    IrtHmgnMatType M);
void GMGenMatrixRotVec(IrtHmgnMatType Mat,
		       const IrtVecType Vec,
		       IrtRType Angle);
void GMGenMatrixRotV2V(IrtHmgnMatType Mat,
		       const IrtVecType V1,
		       const IrtVecType V2);
void GMGenProjectionMat(const IrtPlnType ProjPlane,
			const IrtRType EyePos[4],
			IrtHmgnMatType Mat);
void GMGenReflectionMat(const IrtPlnType ReflectPlane, IrtHmgnMatType Mat);
int GM3Pts2EqltrlTriMat(const IrtPtType Pt1Orig,
			const IrtPtType Pt2Orig,
			const IrtPtType Pt3Orig,
			IrtHmgnMatType Mat);
IrtRType *GMBaryCentric3Pts2D(const IrtPtType Pt1,
			      const IrtPtType Pt2,
			      const IrtPtType Pt3,
			      const IrtPtType Pt);
IrtRType *GMBaryCentric3Pts(const IrtPtType Pt1,
			    const IrtPtType Pt2,
			    const IrtPtType Pt3,
			    const IrtPtType Pt);
int GM2PointsFromCircCirc(const IrtPtType Center1,
			  IrtRType Radius1,
			  const IrtPtType Center2,
			  IrtRType Radius2,
			  IrtPtType Inter1,
			  IrtPtType Inter2);
int GM2PointsFromCircCirc3D(const IrtPtType Cntr1,
			    const IrtVecType Nrml1,
			    IrtRType Rad1,
			    const IrtPtType Cntr2,
			    const IrtVecType Nrml2,
			    IrtRType Rad2,
			    IrtPtType Inter1,
			    IrtPtType Inter2);
int GMCircleFrom3Points(IrtPtType Center,
			IrtRType *Radius,
			const IrtPtType Pt1,
			const IrtPtType Pt2,
			const IrtPtType Pt3);
int GMCircleFrom2Pts2Tans(IrtPtType Center,
			  IrtRType *Radius,
			  const IrtPtType Pt1,
			  const IrtPtType Pt2,
			  const IrtVecType Tan1,
			  const IrtVecType Tan2);
int GMCircleFromLstSqrPts(CagdPType Center,
			  IrtRType *Radius,
			  const CagdPType *Pts,
			  int PtsSize);
int GM2BiTansFromCircCirc(const IrtPtType Center1,
			  IrtRType Radius1,
			  const IrtPtType Center2,
			  IrtRType Radius2,
			  int OuterTans,
			  IrtPtType TanPts[2][2]);
int GM2TanLinesFromCircCirc(const IrtPtType Center1,
			    IrtRType Radius1,
			    const IrtPtType Center2,
			    IrtRType Radius2,
			    int OuterTans,
			    IrtLnType Tans[2]);
int GMIsPtInsideCirc(const IrtRType *Point,
		     const IrtRType *Center,
		     IrtRType Radius);
int GMIsPtOnCirc(const IrtRType *Point,
		 const IrtRType *Center,
		 IrtRType Radius);
IrtRType GMAreaOfTriangle(const IrtRType *Pt1,
			  const IrtRType *Pt2,
			  const IrtRType *Pt3);

/* Convex polygon - ray intersections in R3. */

int GMRayCnvxPolygonInter(const IrtPtType RayOrigin,
			  const IrtVecType RayDir,
			  const IPPolygonStruct *Pl,
			  IrtPtType InterPoint);
int GMPointInsideCnvxPolygon(const IrtPtType Pt, const IPPolygonStruct *Pl);
int GMPointOnPolygonBndry(const IrtPtType Pt,
			  const IPPolygonStruct *Pl,
			  IrtRType Eps);

/* Polynomial solvers. */

int GMSolveQuadraticEqn(IrtRType A, IrtRType B, IrtRType *Sols);
int GMSolveQuadraticEqn2(IrtRType B,
			 IrtRType C,
			 IrtRType *RSols,
			 IrtRType *ISols);
int GMSolveCubicEqn(IrtRType A, IrtRType B, IrtRType C, IrtRType *Sols);
int GMSolveCubicEqn2(IrtRType A,
		     IrtRType B,
		     IrtRType C, 
		     IrtRType *RSols,
		     IrtRType *ISols);
int GMSolveQuarticEqn(IrtRType A,
		      IrtRType B,
		      IrtRType C,
		      IrtRType D, 
		      IrtRType *Sols);
void GMComplexRoot(IrtRType RealVal,
		   IrtRType ImageVal,
		   IrtRType *RealRoot,
		   IrtRType *ImageRoot);

/* Geometric properties routines: */

IrtRType GMPolyLength(const IPPolygonStruct *Pl);
int GMPolyCentroid(const IPPolygonStruct *Pl, IrtPtType Centroid);
double GMPolyObjectArea(const IPObjectStruct *PObj);
double GMPolyOnePolyArea(const IPPolygonStruct *Pl);
double GMPolyObjectVolume(IPObjectStruct *PObj);

/* Functions from sphere's cone distribution - Sph_Cone.c. */

void GMSphConeSetConeDensity(int n);
const IrtVecType *GMSphConeGetPtsDensity(int *n);
VoidPtr GMSphConeQueryInit(IPObjectStruct *PObj);
void GMSphConeQueryFree(VoidPtr SphCone);
void GMSphConeQueryGetVectors(VoidPtr SphConePtr,
			      IrtVecType Dir,
			      IrtRType Angle,
			      GMSphConeQueryCallBackFuncType SQFunc);
void GMSphConeQuery2GetVectors(VoidPtr SphConePtr,
			       GMSphConeQueryDirFuncType SQQuery,
			       GMSphConeQueryCallBackFuncType SQFunc);

/* Functions from the convex hull computation package. */

int GMConvexHull(IrtE2PtStruct *DTPts, int *NumOfPoints);
int GMMonotonePolyConvex(IPVertexStruct *VHead, int Cnvx);

/* Functions from the minimum spanning circle/sphere packages. */

int GMMinSpanCirc(IrtE2PtStruct *DTPts,
		  int NumOfPoints,
		  IrtE2PtStruct *Center,
		  IrtRType *Radius);
int GMMinSpanConeAvg(IrtVecType *DTVecs,
		     int VecsNormalized,
		     int NumOfPoints,
		     IrtVecType Center,
		     IrtRType *Angle);
int GMMinSpanCone(IrtVecType *DTVecs,
		  int VecsNormalized,
		  int NumOfPoints,
		  IrtVecType Center,
		  IrtRType *Angle);

int GMMinSpanSphere(IrtE3PtStruct *DTPts,
		    int NumOfPoints,
		    IrtE3PtStruct *Center,
		    IrtRType *Radius);
int GMSphereWith3Pts(IrtE3PtStruct *Pts,
		     IrtRType *Center,
		     IrtRType *RadiusSqr);
int GMSphereWith4Pts(IrtE3PtStruct *Pts,
		     IrtRType *Center,
		     IrtRType *RadiusSqr);

/* Functions to extract silhouette and boundary curves from polygonal data. */

VoidPtr GMSilPreprocessPolys(IPObjectStruct *PObj, int n);
int GMSilPreprocessRefine(VoidPtr PrepSils, int n);
IPObjectStruct *GMSilExtractSilDirect(IPObjectStruct *PObj,
				      IrtHmgnMatType ViewMat);
IPObjectStruct *GMSilExtractSilDirect2(IPObjectStruct *PObjReg,
				       IrtHmgnMatType ViewMat);
IPObjectStruct *GMSilExtractSil(VoidPtr PrepSils, IrtHmgnMatType ViewMat);
IPObjectStruct *GMSilExtractDiscont(IPObjectStruct *PObjReg,
				    IrtRType MinAngle);
IPObjectStruct *GMSilExtractBndry(IPObjectStruct *PObj);
void GMSilProprocessFree(VoidPtr PrepSils);
int GMSilOrigObjAlive(int ObjAlive);

/* Functions from the animate package. */

void GMAnimResetAnimStruct(GMAnimationStruct *Anim);
void GMAnimGetAnimInfoText(GMAnimationStruct *Anim);
int GMAnimHasAnimation(const IPObjectStruct *PObjs);
int GMAnimHasAnimationOne(const IPObjectStruct *PObj);
void GMAnimFindAnimationTimeOne(GMAnimationStruct *Anim,
				const IPObjectStruct *PObj);
void GMAnimFindAnimationTime(GMAnimationStruct *Anim,
			     const IPObjectStruct *PObjs);
void GMAnimSaveIterationsToFiles(GMAnimationStruct *Anim,
				 IPObjectStruct *PObjs);
void GMAnimSaveIterationsAsImages(GMAnimationStruct *Anim,
				  IPObjectStruct *PObjs);
IrtRType GMExecuteAnimationEvalMat(IPObjectStruct *AnimationP,
				   IrtRType Time,
				   IrtHmgnMatType ObjMat);
void GMAnimDoAnimation(GMAnimationStruct *Anim, IPObjectStruct *PObjs);
int GMAnimSetAnimMatHierarchy(int AnimMatHierarchy);
int GMAnimSetAnimInternalNodes(int AnimInternalNodes);
void GMAnimEvalAnimation(IrtRType t, IPObjectStruct *PObj);
void GMAnimEvalAnimationList(IrtRType t, IPObjectStruct *PObjList);
IPObjectStruct *GMAnimEvalObjAtTime(IrtRType t, IPObjectStruct *PObj);
void GMAnimDoSingleStep(GMAnimationStruct *Anim, IPObjectStruct *PObjs);
int GMAnimCheckInterrupt(GMAnimationStruct *Anim);

/* Functions from the bbox package. */

int GMBBSetBBoxInvisibles(int BBoxInvisibles);
GMBBBboxStruct *GMBBComputeBboxObject(const IPObjectStruct *PObj);
GMBBBboxStruct *GMBBComputeBboxObjectList(const IPObjectStruct *PObj);
const IPObjectStruct *GMBBSetGlblBBObjList(const IPObjectStruct *BBObjList);
GMBBBboxStruct *GMBBComputeOnePolyBbox(const IPPolygonStruct *PPoly);
GMBBBboxStruct *GMBBComputePolyListBbox(const IPPolygonStruct *PPoly);
GMBBBboxStruct *GMBBComputePointBbox(const IrtRType *Pt);
GMBBBboxStruct *GMBBMergeBbox(const GMBBBboxStruct *Bbox1,
			      const GMBBBboxStruct *Bbox2);

/* Functions from the convex polygons package. */

int GMConvexPolyNormals(int HandleNormals);
int GMConvexRaysToVertices(int RaysToVertices);
IPObjectStruct *GMConvexPolyObjectN(IPObjectStruct *PObj);
void GMConvexPolyObject(IPObjectStruct *PObj);
int GMIsConvexPolygon2(const IPPolygonStruct *Pl);
int GMIsConvexPolygon(IPPolygonStruct *Pl);

IPPolygonStruct *GMSplitNonConvexPoly(IPPolygonStruct *Pl);
void GMGenRotateMatrix(IrtHmgnMatType Mat, const IrtVecType Dir);

/* Functions from the polygonal decimation package. */

IPObjectStruct *GMDecimateObject(IPObjectStruct *PObj);
void GMDecimateObjSetDistParam(IrtRType);
void GMDecimateObjSetPassNumParam(int);
void GMDecimateObjSetDcmRatioParam(int);
void GMDecimateObjSetMinAspRatioParam(IrtRType);

VoidPtr HDSCnvrtPObj2QTree(IPObjectStruct *PObjects, int Depth);
IPObjectStruct *HDSThreshold(VoidPtr Qt, IrtRType Threshold);
IPObjectStruct *HDSTriBudget(VoidPtr Qt, int TriBudget);
void HDSFreeQTree(VoidPtr Qt);
int HDSGetActiveListCount(VoidPtr Qt);
int HDSGetTriangleListCount(VoidPtr Qt);
int HDSGetDismissedTrianglesCount(VoidPtr Qt);

/* Functions from the normal/uv/rgb/etc. interpolation package. */

void GMUpdateVerticesByInterp(IPPolygonStruct *PlList,
			      const IPPolygonStruct *OriginalPl);
void GMUpdateVertexByInterp(IPVertexStruct *VUpdate,
			    const IPVertexStruct *V,
			    const IPVertexStruct *VNext,
			    int DoRgb,
			    int DoUV,
			    int DoNrml);
int GMCollinear3Vertices(const IPVertexStruct *V1,
			 const IPVertexStruct *V2,
			 const IPVertexStruct *V3);
int GMEvalWeightsVFromPl(const IrtRType *Coord,
			 const IPPolygonStruct *Pl,
			 IrtRType *Wgt);
void GMInterpVrtxNrmlBetweenTwo(IPVertexStruct *V,
				const IPVertexStruct *V1,
				const IPVertexStruct *V2);
void GMInterpVrtxNrmlBetweenTwo2(IrtPtType Pt,
				 IrtVecType Normal,
				 const IPVertexStruct *V1,
				 const IPVertexStruct *V2);
int GMInterpVrtxNrmlFromPl(IPVertexStruct *V, const IPPolygonStruct *Pl);
int GMInterpVrtxRGBBetweenTwo(IPVertexStruct *V,
			      const IPVertexStruct *V1,
			      const IPVertexStruct *V2);
int GMInterpVrtxRGBFromPl(IPVertexStruct *V, const IPPolygonStruct *Pl);
int GMInterpVrtxUVBetweenTwo(IPVertexStruct *V,
			     const IPVertexStruct *V1,
			     const IPVertexStruct *V2);
int GMInterpVrtxUVFromPl(IPVertexStruct *V, const IPPolygonStruct *Pl);
void GMBlendNormalsToVertices(IPPolygonStruct *PlList,
			      IrtRType MaxAngle);
void GMFixOrientationOfPolyModel(IPPolygonStruct *Pls);
void GMFixNormalsOfPolyModel(IPPolygonStruct *PlList, int TrustFixedPt);

/* Functions from the line sweep package. */

void GMLineSweep(GMLsLineSegStruct **Lines);

/* Functions from the polygonal cleaning package. */

int GMTwoPolySameGeom(const IPPolygonStruct *Pl1,
		      const IPPolygonStruct *Pl2,
		      IrtRType Eps);
IPPolygonStruct *GMCleanUpDupPolys(IPPolygonStruct **PPolygons, IrtRType Eps);
IPPolygonStruct *GMCleanUpPolygonList(IPPolygonStruct **PPolygons,
				      IrtRType Eps);
IPPolygonStruct *GMCleanUpPolylineList(IPPolygonStruct **PPolylines,
				       IrtRType Eps);
IPPolygonStruct *GMCleanUpPolylineList2(IPPolygonStruct *PPolylines);
void GMVrtxListToCircOrLin(IPPolygonStruct *Pls, int DoCirc);
void GMVrtxListToCircOrLinDup(IPPolygonStruct *Pls, int DoCirc);
IPVertexStruct *GMFilterInteriorVertices(IPVertexStruct *VHead,
					 IrtRType MinTol,
					 int n);
IPPolygonStruct *GMClipPolysAgainstPlane(IPPolygonStruct *PHead,
					 IPPolygonStruct **PClipped,
					 IPPolygonStruct **PInter,
					 IrtPlnType Plane);
IPVertexStruct *GMFindThirdPointInTriangle(const IPPolygonStruct *Pl,
					   const IPVertexStruct *V,
					   const IPVertexStruct *VNext);

/* Functions from the points on polygonal objects package. */

IPObjectStruct *GMConvertPolysToNGons(IPObjectStruct *PolyObj, int n);
IPObjectStruct *GMConvertPolysToTriangles(IPObjectStruct *PolyObj);
IPObjectStruct *GMConvertPolysToTriangles2(IPObjectStruct *PolyObj);
IPObjectStruct *GMConvertPolysToRectangles(IPObjectStruct *PolyObj);
IPPolygonStruct *GMLimitTrianglesEdgeLen(const IPPolygonStruct *OrigPls,
					 IrtRType MaxLen);
void GMAffineTransUVVals(IPObjectStruct *PObj,
			 const IrtRType Scale[2],
			 const IrtRType Trans[2]);
void GMGenUVValsForPolys(IPObjectStruct *PObj,
			 IrtRType UTextureRepeat,
			 IrtRType VTextureRepeat,
			 IrtRType WTextureRepeat,
			 int HasXYZScale);
int GMMergeSameGeometry(void **GeomEntities,
			int NumOfGEntities,
			IrtRType IdenticalEps,
			GMMergeGeomInitFuncType InitFunc,
			GMMergeGeomDistFuncType DistSqrFunc,
			GMMergeGeomKeyFuncType KeyFunc,
			GMMergeGeomMergeFuncType MergeFunc);
int GMMergeGeometry(void **GeomEntities,
		    int NumOfGEntities,
		    IrtRType Eps,
		    IrtRType IdenticalEps,
		    GMMergeGeomInitFuncType InitFunc,
		    GMMergeGeomDistFuncType DistSqrFunc,
		    GMMergeGeomKeyFuncType KeyFunc,
		    GMMergeGeomMergeFuncType MergeFunc);
IPPolygonStruct *GMMergePolylines(IPPolygonStruct *Polys, IrtRType Eps);
IPPolygonStruct *GMMatchPointListIntoPolylines(IPObjectStruct *PtsList,
					       IrtRType MaxTol);
IPObjectStruct *GMPointCoverOfPolyObj(IPObjectStruct *PolyObj,
				      int n,
				      IrtRType *Dir,
				      char *PlAttr);
IPObjectStruct *GMRegularizePolyModel(IPObjectStruct *PObj,
				      int SplitCollinear);
IPPolygonStruct *GMSplitPolysAtCollinearVertices(IPPolygonStruct *Pls);
IPPolygonStruct *GMSplitPolyInPlaceAtVertex(IPPolygonStruct *Pl,
					    IPVertexStruct *VHead);
IPPolygonStruct *GMSplitPolyInPlaceAt2Vertices(IPPolygonStruct *Pl,
					       IPVertexStruct *V1,
					       IPVertexStruct *V2);

/* Functions from the polygonal offsets package. */

IrtRType GMPolyOffsetAmountDepth(const IrtRType *Coord);
IPPolygonStruct *GMPolyOffset(const IPPolygonStruct *Poly,
			      int IsPolygon,
			      IrtRType Ofst,
			      GMPolyOffsetAmountFuncType AmountFunc);
IPPolygonStruct *GMPolyOffset3D(const IPPolygonStruct *Poly,
				IrtRType Ofst,
				int ForceSmoothing,
				int MiterEdges,
				GMPolyOffsetAmountFuncType AmountFunc);

/* Functions from the primitive constructions' package. */

int PrimSetGeneratePrimType(int PolygonalPrimitive);
int PrimSetSurfacePrimitiveRational(int SurfaceRational);

IPObjectStruct *PrimGenBOXObject(const IrtVecType Pt,
				 IrtRType WidthX,
				 IrtRType WidthY,
				 IrtRType WidthZ);
IPObjectStruct *PrimGenBOXWIREObject(const IrtVecType Pt,
				     IrtRType WidthX,
				     IrtRType WidthY,
				     IrtRType WidthZ);
IPObjectStruct *PrimGenGBOXObject(const IrtVecType Pt,
				  const IrtVecType Dir1,
				  const IrtVecType Dir2,
				  const IrtVecType Dir3);
IPObjectStruct *PrimGenCONEObject(const IrtVecType Pt,
				  const IrtVecType Dir,
				  IrtRType R,
				  int Bases);
IPObjectStruct *PrimGenCONE2Object(const IrtVecType Pt,
				   const IrtVecType Dir,
				   IrtRType R1,
				   IrtRType R2,
				   int Bases);
IPObjectStruct *PrimGenCYLINObject(const IrtVecType Pt,
				   const IrtVecType Dir,
				   IrtRType R,
				   int Bases);
IPObjectStruct *PrimGenSPHEREObject(const IrtVecType Center, IrtRType R);
IPObjectStruct *PrimGenTORUSObject(const IrtVecType Center,
				   const IrtVecType Normal,
				   IrtRType Rmajor,
				   IrtRType Rminor);
IPObjectStruct *PrimGenPOLYDISKObject(const IrtVecType Nrml,
				      const IrtVecType Trns, 
				      IrtRType R);
IPObjectStruct *PrimGenPOLYGONObject(IPObjectStruct *PObjList, int IsPolyline);
IPObjectStruct *PrimGenObjectFromPolyList(IPObjectStruct *PObjList);
IPObjectStruct *PrimGenCROSSECObject(const IPObjectStruct *PObj);
IPObjectStruct *PrimGenSURFREVObject(const IPObjectStruct *Cross);
IPObjectStruct *PrimGenSURFREVAxisObject(IPObjectStruct *Cross,
					 const IrtVecType Axis);
IPObjectStruct *PrimGenSURFREV2Object(const IPObjectStruct *Cross,
				      IrtRType StartAngle,
				      IrtRType EndAngle);
IPObjectStruct *PrimGenSURFREV2AxisObject(IPObjectStruct *Cross,
					  IrtRType StartAngle,
					  IrtRType EndAngle,
					  const IrtVecType Axis);
IPObjectStruct *PrimGenEXTRUDEObject(const IPObjectStruct *Cross,
				     const IrtVecType Dir,
				     int Bases);
IPObjectStruct *PrimGenRULEDObject(const IPObjectStruct *Cross1,
				   const IPObjectStruct *Cross2);

IPPolygonStruct *PrimGenPolygon4Vrtx(const IrtVecType V1,
				     const IrtVecType V2,
				     const IrtVecType V3,
				     const IrtVecType V4,
				     const IrtVecType Vin,
				     int *VrtcsRvrsd,
				     IPPolygonStruct *Pnext);
IPPolygonStruct *PrimGenPolygon3Vrtx(const IrtVecType V1,
				     const IrtVecType V2,
				     const IrtVecType V3,
				     const IrtVecType Vin,
				     int *VrtcsRvrsd,
				     IPPolygonStruct *Pnext);

IPObjectStruct *PrimGenTransformController2D(const GMBBBboxStruct *BBox);
IPObjectStruct *PrimGenTransformController2DCrvs(const GMBBBboxStruct *BBox);
IPObjectStruct *PrimGenTransformController(const GMBBBboxStruct *BBox, 
					   int HasRotation,
					   int HasTranslation,
					   int HasUniformScale,
					   IrtRType BoxOpacity);
IPPolygonStruct *PrimGenPolyline4Vrtx(const IrtVecType V1,
				      const IrtVecType V2,
				      const IrtVecType V3,
				      const IrtVecType V4,
				      IPPolygonStruct *Pnext);

int PrimSetResolution(int Resolution);

/* Functions from the quaternions package. */

void GMQuatToMat(GMQuatType q, IrtHmgnMatType Mat);
void GMQuatMatToQuat(IrtHmgnMatType Mat, GMQuatType q);
void GMQuatRotationToQuat(IrtRType Xangle,
			  IrtRType Yangle, 
			  IrtRType Zangle,
			  GMQuatType q);
void GMQuatToRotation(GMQuatType q, IrtVecType *Angles, int *NumSolutions);
void GMQuatMul(GMQuatType q1, GMQuatType q2, GMQuatType QRes);
void GMQuatAdd(GMQuatType q1, GMQuatType q2, GMQuatType QRes);
int GMQuatIsUnitQuat(GMQuatType q);
void GMQuatNormalize(GMQuatType q);
void GMQuatInverse(GMQuatType SrcQ, GMQuatType DstQ);
void GMQuatRotateVec(IrtVecType OrigVec, GMQuatType RotQ, IrtVecType DestVec);
void GMQuatLog(GMQuatType SrcQ, IrtVecType DstVec);
void GMQuatExp(IrtVecType SrcVec, GMQuatType DstQ);
void GMQuatPow(GMQuatType MantisQ, IrtRType Expon, GMQuatType DstQ);
int GMQuatMatrixToAngles(IrtHmgnMatType Mat, IrtVecType *Vec);
void GMQuatMatrixToTranslation(IrtHmgnMatType Mat, IrtVecType Vec);
IrtRType GMQuatMatrixToScale(IrtHmgnMatType Mat);
int GMQuatMatrixToVector(IrtHmgnMatType Mat, GMQuatTransVecType TransVec);
void GMQuatVectorToMatrix(GMQuatTransVecType TransVec, IrtHmgnMatType Mat);
void GMQuatVecToScaleMatrix(GMQuatTransVecType TransVec,
			    IrtHmgnMatType ScaleMatrix);
void GMQuatVecToRotMatrix(GMQuatTransVecType TransVec, IrtHmgnMatType RotMatrix);
void GMQuatVecToTransMatrix(GMQuatTransVecType TransVec,
			    IrtHmgnMatType TransMatrix);
void GMMatrixToTransform(IrtHmgnMatType Mat, 
			 IrtVecType S,
			 GMQuatType R,
			 IrtVecType T);

/* Functions from the spherical coverage package. */

IPObjectStruct *GMPointCoverOfUnitHemiSphere(IrtRType HoneyCombSize);

/* Functions from the software z buffer. */

VoidPtr GMZBufferInit(int Width, int Height);
void GMZBufferFree(VoidPtr ZbufferID);
void GMZBufferClear(VoidPtr ZbufferID);
void GMZBufferClearSet(VoidPtr ZbufferID, IrtRType Depth);
GMZTestsType GMZBufferSetZTest(VoidPtr ZbufferID, GMZTestsType ZTest);
GMZBufferUpdateFuncType GMZBufferSetUpdateFunc(VoidPtr ZbufferID,
					       GMZBufferUpdateFuncType
					                           UpdateFunc);
VoidPtr GMZBufferInvert(VoidPtr ZbufferID);
VoidPtr GMZBufferRoberts(VoidPtr ZbufferID);
VoidPtr GMZBufferLaplacian(VoidPtr ZbufferID);
IrtRType GMZBufferQueryZ(VoidPtr ZbufferID, int x, int y);
VoidPtr GMZBufferQueryInfo(VoidPtr ZbufferID, int x, int y);
IrtRType GMZBufferUpdatePt(VoidPtr ZbufferID, int x, int y, IrtRType z);
VoidPtr GMZBufferUpdateInfo(VoidPtr ZbufferID, int x, int y, VoidPtr Info);
void GMZBufferUpdateHLn(VoidPtr ZbufferID,
			int x1,
			int x2,
			int y,
			IrtRType z1,
			IrtRType z2);
void GMZBufferUpdateLine(VoidPtr ZbufferID,
			 int x1,
			 int y1,
			 int x2,
			 int y2,
			 IrtRType z1,
			 IrtRType z2);
void GMZBufferUpdateTri(VoidPtr ZbufferID,
			int x1,
			int y1,
			IrtRType z1,
			int x2,
			int y2,
			IrtRType z2,
			int x3,
			int y3,
			IrtRType z3);

/* Functions from the z buffer based on Open GL package. */

IritIntPtrSizeType GMZBufferOGLInit(int Width,
				    int Height,
				    IrtRType ZMin,
				    IrtRType ZMax,
				    int OffScreen);
void GMZBufferOGLClear(void);
void GMZBufferOGLSetColor(int Red, int Green, int Blue);
void GMZBufferOGLMakeActive(IritIntPtrSizeType Id);
IrtRType GMZBufferOGLQueryZ(IrtRType x, IrtRType y);
void GMZBufferOGLQueryColor(IrtRType x,
			    IrtRType y,
			    int *Red,
			    int *Green,
			    int *Blue);
void GMZBufferOGLFlush(void);

/* Functions to fit analytic functions to point data sets. */

IrtPtType *GMSrfBilinearFit(IrtPtType *ParamDomainPts,
			    IrtPtType *EuclideanPts,
			    int FirstAtOrigin,
			    int NumPts);
IrtPtType *GMSrfQuadricFit(IrtPtType *ParamDomainPts,
			   IrtPtType *EuclideanPts,
			   int FirstAtOrigin,
			   int NumPts);
IrtPtType *GMSrfQuadricQuadOnly(IrtPtType *ParamDomainPts,
				IrtPtType *EuclideanPts,
				int FirstAtOrigin,
				int NumEucDim,
				int NumPts);
IrtPtType *GMSrfCubicQuadOnly(IrtPtType *ParamDomainPts,
			      IrtPtType *EuclideanPts,
			      int FirstAtOrigin,
			      int NumEucDim,
			      int NumPts);

/* Functions from the distribution of point on a line package. */

IrtRType *GMDistPoint1DWithEnergy(int N,
				  IrtRType XMin,
				  IrtRType XMax,
				  int Resolution,
				  GMDistEnergy1DFuncType EnergyFunc);

/* Metamorphosis of polygonal objects. */

IPPolygonStruct *GMPolygonalMorphosis(const IPPolygonStruct *Pl1,
				      const IPPolygonStruct *Pl2,
				      IrtRType t);

/* Scan conversion of polygons. */

void GMScanConvertTriangle(int Pt1[2],
			   int Pt2[2],
			   int Pt3[2],
			   GMScanConvertApplyFuncType ApplyFunc);

/* Text and string data sets. */

int GMLoadTextFont(const char *FName);
IPObjectStruct *GMMakeTextGeometry(const char *Str,
				   const IrtVecType Spacing,
				   const IrtRType *Scaling);

/* Curvature analysis over polygonal meshes. */

void GMPlCrvtrSetCurvatureAttr(IPPolygonStruct *PolyList,
			       int NumOfRings,
			       int EstimateNrmls);
int GMPlCrvtrSetFitDegree(int UseCubic);

/* Importance analysis over polygonal meshes. */

void GMPlSilImportanceAttr(IPPolygonStruct *PolyList);
IPPolygonStruct *GMPlSilImportanceRange(IPPolygonStruct *PolyList);


/* Extraction of properties from polygonal meshes. */

IPPolygonStruct *GMPolyPropFetchAttribute(IPPolygonStruct *Pls,
					  const char *PropAttr,
					  IrtRType Value);
IPPolygonStruct *GMPolyPropFetchIsophotes(IPPolygonStruct *Pls,
					  const IrtVecType ViewDir,
					  IrtRType InclinationAngle);
IPPolygonStruct *GMPolyPropFetchCurvature(IPPolygonStruct *Pls,
					  int CurvatureProperty,
					  int NumOfRings,
					  IrtRType CrvtrVal);
IPPolygonStruct *GMPolyPropFetch(IPPolygonStruct *Pls,
				 GMFetchVertexPropertyFuncType VertexProperty,
				 IrtRType ConstVal);
IPPolygonStruct *GMGenPolyline2Vrtx(IrtVecType V1,
				    IrtVecType V2,
				    IPPolygonStruct *Pnext);

/* Function for primitive fitting to point clouds. */

IrtRType GMFitData(IrtRType **PointData,
		   unsigned int NumberOfPointsToFit,
		   GMFittingModelType FittingModel,
		   IrtRType ModelParams[],
		   IrtRType Tolerance);
IrtRType GMFitDataWithOutliers(IrtRType **PointData,
			       unsigned int NumberOfPointsToFit,
			       GMFittingModelType FittingModel,
			       IrtRType ModelParams[],
			       IrtRType Tolerance,
			       unsigned int NumOfChecks);
IrtRType GMFitObjectWithOutliers(IPPolygonStruct *PPoly,
				 GMFittingModelType FittingModel,
				 IrtRType ModelExtParams[],
				 IrtRType Tolerance,
				 unsigned int NumOfChecks);	
IrtRType GMFitEstimateRotationAxis(IrtPtType *PointsOnObject,
				   IrtVecType *Normals,
				   unsigned int NumberOfPoints, 
				   IrtPtType PointOnRotationAxis,
				   IrtVecType RotationAxisDirection);

/* Functions to construct an adjacency data structure for polygonal meshes. */

VoidPtr GMPolyAdjacncyGen(IPObjectStruct *PObj, IrtRType EqlEps);
void GMPolyAdjacncyVertex(IPVertexStruct *V,
			  VoidPtr PolyAdj,
			  GMPolyAdjacncyVertexFuncType AdjVertexFunc);
void GMPolyAdjacncyFree(VoidPtr PolyAdj);
int GMIdentifyTJunctions(IPObjectStruct *PolyObj,
			 GMIdentifyTJunctionFuncType TJuncCB,
			 IrtRType Eps);

/* Functions to smooth poly data. */

IPObjectStruct *GMPolyMeshSmoothing(IPObjectStruct *PolyObj, int NumTimes);
void GMFindUnConvexPolygonNormal(const IPVertexStruct *VL, IrtVecType Nrml);
int GMFindPtInsidePolyKernel(const IPVertexStruct *VE, IrtPtType KrnlPt);
int GMIsVertexBoundary(int Index, const IPPolyVrtxIdxStruct *PVIdx);
int GMIsInterLinePolygon(const IPVertexStruct *VS, 
			 const IrtPtType V1, 
			 const IrtPtType V2);
int GMComputeAverageVertex(const IPVertexStruct *VS, 
			   IrtPtType Point, 
			   const char *Name, 
			   int Val);
IPVertexStruct *GMGet1RingPoly2VrtxIdx(int Index, 
				       const IPPolyVrtxIdxStruct *PVIdx);
IPVertexStruct *GMFindAdjacentEdge(const IPPolyVrtxIdxStruct *PVIdx, 
				   int FirstVertexIndex, 
				   int SecondVertexIndex);
IPPolygonStruct *GMFindAdjacentPoly(const IPPolyVrtxIdxStruct *PVIdx,
				    const IPVertexStruct *V,
				    const IPVertexStruct *VNext);

  /* Subdivision surfaces functions. */

IPObjectStruct *GMSubCatmullClark(IPObjectStruct *OriginalObj);
IPObjectStruct *GMSubLoop(IPObjectStruct *OriginalObj);
IPObjectStruct *GMSubButterfly(IPObjectStruct *OriginalObj, 
			       IrtRType ButterflyWCoef);

/* Error handling. */

GeomSetErrorFuncType GeomSetFatalErrorFunc(GeomSetErrorFuncType ErrorFunc);
void GeomFatalError(GeomFatalErrorType ErrID);
const char *GeomDescribeError(GeomFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* GEOMAT_3D_H */
