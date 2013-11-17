/******************************************************************************
* Triv_lib.h - header file for the TRIV library.			      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#ifndef TRIV_LIB_H
#define TRIV_LIB_H

#include <stdio.h>
#include "irit_sm.h"
#include "miscattr.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"

#define TV_PT_COPY(PtDest, PtSrc) \
            memcpy((char *) (PtDest), (char *) (PtSrc), 4 * sizeof(CagdRType))
#define TV_PLANE_COPY(PlDest, PlSrc) \
	    memcpy((char *) (PlDest), (char *) (PlSrc), 5 * sizeof(CagdRType))
#define TV_PT_SQR_LENGTH(Pt)	(IRIT_SQR(Pt[0]) + IRIT_SQR(Pt[1]) + \
				 IRIT_SQR(Pt[2]) + IRIT_SQR(Pt[3]))

#define TV_PT_LENGTH(Pt)	sqrt(TV_PT_SQR_LENGTH(Pt))

#define TV_PT_RESET(Pt)		IRIT_ZAP_MEM((Pt), 4 * sizeof(IrtRType))

#define TV_PT_NORMALIZE(Pt)	{    CagdRType Size = TV_PT_LENGTH(Pt); \
				     _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
				     { \
					 Pt[0] /= Size; \
					 Pt[1] /= Size; \
				         Pt[2] /= Size; \
				         Pt[3] /= Size; \
				     } \
				}

#define TV_PT_BLEND(Res, Pt1, Pt2, t) \
				{ Res[0] = Pt1[0] * t + Pt2[0] * (1 - t); \
				  Res[1] = Pt1[1] * t + Pt2[1] * (1 - t); \
				  Res[2] = Pt1[2] * t + Pt2[2] * (1 - t); \
				  Res[3] = Pt1[3] * t + Pt2[3] * (1 - t); \
			        }

#define TV_PT_ADD(Res, Pt1, Pt2) { Res[0] = Pt1[0] + Pt2[0]; \
				   Res[1] = Pt1[1] + Pt2[1]; \
				   Res[2] = Pt1[2] + Pt2[2]; \
				   Res[3] = Pt1[3] + Pt2[3]; \
			         }

#define TV_PT_SUB(Res, Pt1, Pt2) { Res[0] = Pt1[0] - Pt2[0]; \
				   Res[1] = Pt1[1] - Pt2[1]; \
				   Res[2] = Pt1[2] - Pt2[2]; \
				   Res[3] = Pt1[3] - Pt2[3]; \
				 }

#define TV_PT_SWAP(Pt1, Pt2)	{ IRIT_SWAP(CagdRType, Pt1[0], Pt2[0]); \
				  IRIT_SWAP(CagdRType, Pt1[1], Pt2[1]); \
				  IRIT_SWAP(CagdRType, Pt1[2], Pt2[2]); \
				  IRIT_SWAP(CagdRType, Pt1[3], Pt2[3]); \
				}

#define TV_PT_PT_DIST(Pt1, Pt2) sqrt(IRIT_SQR(Pt1[0] - Pt2[0]) + \
				     IRIT_SQR(Pt1[1] - Pt2[1]) + \
				     IRIT_SQR(Pt1[2] - Pt2[2]) + \
				     IRIT_SQR(Pt1[3] - Pt2[3]))

#define TV_PT_PT_DIST_SQR(Pt1, Pt2) (IRIT_SQR(Pt1[0] - Pt2[0]) + \
				     IRIT_SQR(Pt1[1] - Pt2[1]) + \
				     IRIT_SQR(Pt1[2] - Pt2[2]) + \
				     IRIT_SQR(Pt1[3] - Pt2[3]))

#define TV_DOT_PROD(Pt1, Pt2)	(Pt1[0] * Pt2[0] + \
				 Pt1[1] * Pt2[1] + \
				 Pt1[2] * Pt2[2] + \
				 Pt1[3] * Pt2[3])

typedef CagdRType TrivPType[4];
typedef CagdRType TrivVType[4];
typedef CagdRType TrivUVWType[3];
typedef CagdRType TrivIrtPlnType[5];

typedef enum {
    TRIV_ERR_DIR_NOT_VALID,
    TRIV_ERR_UNDEF_CRV,
    TRIV_ERR_UNDEF_SRF,
    TRIV_ERR_UNDEF_TRIVAR,
    TRIV_ERR_UNDEF_GEOM,
    TRIV_ERR_UNSUPPORT_PT,
    TRIV_ERR_RATIONAL_NO_SUPPORT,
    TRIV_ERR_WRONG_ORDER,
    TRIV_ERR_KNOT_NOT_ORDERED,
    TRIV_ERR_NUM_KNOT_MISMATCH,
    TRIV_ERR_INDEX_NOT_IN_MESH,
    TRIV_ERR_POWER_NO_SUPPORT,
    TRIV_ERR_WRONG_DOMAIN,
    TRIV_ERR_INCONS_DOMAIN,
    TRIV_ERR_DIR_NOT_CONST_UVW,
    TRIV_ERR_SCALAR_PT_EXPECTED,
    TRIV_ERR_INVALID_AXIS,
    TRIV_ERR_NO_CLOSED_POLYGON,
    TRIV_ERR_TWO_INTERSECTIONS,
    TRIV_ERR_NO_MATCH_PAIR,
    TRIV_ERR_2_OR_4_INTERS,
    TRIV_ERR_FAIL_FIND_PT,
    TRIV_ERR_FAIL_READ_FILE,
    TRIV_ERR_INVALID_STROKE_TYPE,
    TRIV_ERR_READ_FAIL,
    TRIV_ERR_TVS_INCOMPATIBLE,
    TRIV_ERR_PT_OR_LEN_MISMATCH,
    TRIV_ERR_BSP_TV_EXPECT,
    TRIV_ERR_PERIODIC_EXPECTED,
    TRIV_ERR_UNSUPPORT_DERIV,

    TRIV_ERR_UNDEFINE_ERR
} TrivFatalErrorType;

typedef enum {
    TRIV_UNDEF_TYPE = 1220,
    TRIV_TVBEZIER_TYPE,
    TRIV_TVBSPLINE_TYPE,
    TRIV_TVPOWER_TYPE
} TrivGeomType;

typedef enum {
    TRIV_NO_DIR = CAGD_NO_DIR,
    TRIV_CONST_U_DIR = CAGD_CONST_U_DIR,
    TRIV_CONST_V_DIR = CAGD_CONST_V_DIR,
    TRIV_CONST_W_DIR,
    TRIV_END_DIR
} TrivTVDirType;
#define TRIV_PREV_DIR(Dir) (((int) Dir) + 1 > ((int) TRIV_CONST_W_DIR) ? \
			TRIV_CONST_U_DIR : (TrivTVDirType) ((int) Dir) + 1)
#define TRIV_NEXT_DIR(Dir) (((int) Dir) - 1 < ((int) TRIV_CONST_U_DIR) ? \
			TRIV_CONST_W_DIR : (TrivTVDirType) ((int) Dir) - 1)

typedef enum {
    TRIV_NO_BNDRY = CAGD_NO_BNDRY,
    TRIV_U_MIN_BNDRY = CAGD_U_MIN_BNDRY,
    TRIV_U_MAX_BNDRY = CAGD_U_MAX_BNDRY,
    TRIV_V_MIN_BNDRY = CAGD_V_MIN_BNDRY,
    TRIV_V_MAX_BNDRY = CAGD_V_MAX_BNDRY,
    TRIV_W_MIN_BNDRY,
    TRIV_W_MAX_BNDRY
} TrivTVBndryType;

typedef enum {
    TRIV_GEOM_CONST,
    TRIV_GEOM_LINEAR,
    TRIV_GEOM_TV_OF_REV,
    TRIV_GEOM_TRILINEAR,
    TRIV_GEOM_EXTRUSION,
    TRIV_GEOM_RULED_TV
} TrivIsGeometryType;

typedef struct TrivTriangleStruct {
    struct TrivTriangleStruct *Pnext;
    struct IPAttributeStruct *Attr;
    struct {
	CagdPType Pt;
	CagdVType Nrml;
	TrivUVWType UVW;
    } T[3];
} TrivTriangleStruct;

typedef struct TrivTVStruct {
    struct TrivTVStruct *Pnext;
    struct IPAttributeStruct *Attr;
    TrivGeomType GType;
    CagdPointType PType;
    int ULength, VLength, WLength;/* Mesh size in tri-variate tensor product.*/
    int UVPlane;	  /* Should equal ULength * VLength for fast access. */
    int UOrder, VOrder, WOrder;       /* Order in trivariate (Bspline only). */
    CagdBType UPeriodic, VPeriodic, WPeriodic;    /* Valid only for Bspline. */
    CagdRType *Points[CAGD_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    CagdRType *UKnotVector, *VKnotVector, *WKnotVector;
} TrivTVStruct;

typedef struct TrivTVBlockEvalStruct {
    CagdPType Pos;
    CagdRType Jcbn[3][3];
} TrivTVBlockEvalStruct;

typedef void (*TrivSetErrorFuncType)(TrivFatalErrorType);

#define TRIV_IS_BEZIER_TV(TV)		(TV -> GType == TRIV_TVBEZIER_TYPE)
#define TRIV_IS_BSPLINE_TV(TV)		(TV -> GType == TRIV_TVBSPLINE_TYPE)
#define TRIV_IS_POWER_TV(TV)		(TV -> GType == TRIV_TVPOWER_TYPE)

#define TRIV_IS_RATIONAL_TV(TV)		CAGD_IS_RATIONAL_PT(TV -> PType)
#define TRIV_NUM_OF_PT_COORD(TV)	CAGD_NUM_OF_PT_COORD(TV -> PType)

/******************************************************************************
*           +-----------------------+					      *
*       W  /                       /|					      *
*      /  /                       / |					      *
*     /  /	U -->		 /  |	    The mesh is ordered raw after raw *
*       +-----------------------+   |	or the increments along U are 1 while *
*   V | |P0                 Pi-1|   +	the increment along V is full raw.    *
*     v	|Pi                P2i-1|  /	    Once a full UV plane is complete  *
*	|			| /	W is incremented by 1.		      *
*	|Pn-i		    Pn-1|/          To encapsulate it, NEXTU/V/W are  *
*	+-----------------------+	defined below.			      *
******************************************************************************/
#define TRIV_NEXT_U(TV)			(1)
#define TRIV_NEXT_V(TV)			(TV -> ULength)
#define TRIV_NEXT_W(TV)			(TV -> UVPlane)
#define TRIV_MESH_UVW(TV, i, j, k)	((i) + (TV -> ULength) * (j) + (TV -> UVPlane) * (k))

/* If a trivariate is periodic, the control polygon/mesh should warp up.     */
/* Length does hold the real allocated length but the virtual periodic       */
/* length is a little larger. Note allocated KV's are larger.                */
#define TRIV_TV_UPT_LST_LEN(TV)	((TV) -> ULength + \
				 ((TV) -> UPeriodic ? (TV) -> UOrder - 1 : 0))
#define TRIV_TV_VPT_LST_LEN(TV)	((TV) -> VLength + \
				 ((TV) -> VPeriodic ? (TV) -> VOrder - 1 : 0))
#define TRIV_TV_WPT_LST_LEN(TV)	((TV) -> WLength + \
				 ((TV) -> WPeriodic ? (TV) -> WOrder - 1 : 0))
#define TRIV_IS_UPERIODIC_TV(TV)	((TV) -> UPeriodic)
#define TRIV_IS_VPERIODIC_TV(TV)	((TV) -> VPeriodic)
#define TRIV_IS_WPERIODIC_TV(TV)	((TV) -> WPeriodic)
#define TRIV_IS_PERIODIC_TV(TV)	(TRIV_IS_UPERIODIC_TV(TV) || \
				 TRIV_IS_VPERIODIC_TV(TV) || \
				 TRIV_IS_WPERIODIC_TV(TV))

#define TRIV_DEL_GEOM_TYPE(Obj)		 CAGD_DEL_GEOM_TYPE(Obj)
#define TRIV_SET_GEOM_TYPE(Obj, Geom)	 CAGD_SET_GEOM_TYPE(Obj, Geom)
#define TRIV_PROPAGATE_ATTR(NewObj, Obj) CAGD_PROPAGATE_ATTR(NewObj, Obj)

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* General routines of the Triv library:					      *
******************************************************************************/
TrivTVStruct *TrivTVNew(TrivGeomType GType,
			CagdPointType PType,
			int ULength,
			int VLength,
			int WLength);
TrivTVStruct *TrivBspTVNew(int ULength,
			   int VLength,
			   int WLength,
			   int UOrder,
			   int VOrder,
			   int WOrder,
			   CagdPointType PType);
TrivTVStruct *TrivBzrTVNew(int ULength,
			   int VLength,
			   int WLength,
			   CagdPointType PType);
TrivTVStruct *TrivPwrTVNew(int ULength,
			   int VLength,
			   int WLength,
			   CagdPointType PType);
TrivTVStruct *TrivTVCopy(const TrivTVStruct *TV);
TrivTVStruct *TrivTVCopyList(const TrivTVStruct *TVList);
void TrivTVFree(TrivTVStruct *TV);
void TrivTVFreeList(TrivTVStruct *TVList);

#ifdef DEBUG
#define TrivTVFree(TV)         { TrivTVFree(TV); TV = NULL; }
#define TrivTVFreeList(TVList) { TrivTVFreeList(TVList); TVList = NULL; }
#endif /* DEBUG */

TrivTriangleStruct *TrivTriangleNew(void);
TrivTriangleStruct *TrivTriangleCopy(const TrivTriangleStruct *Triangle);
TrivTriangleStruct *TrivTriangleCopyList(const TrivTriangleStruct
					                        *TriangleList);
void TrivTriangleFree(TrivTriangleStruct *Triangle);
void TrivTriangleFreeList(TrivTriangleStruct *TriangleList);

#ifdef DEBUG
#define TrivTriangleFree(Triangle)         { TrivTriangleFree(Triangle); \
					     Triangle = NULL; }
#define TrivTriangleFreeList(TriangleList) { TrivTriangleFreeList(TriangleList); \
					     TriangleList = NULL; }
#endif /* DEBUG */

TrivTVStruct *TrivCnvrtBzr2BspTV(const TrivTVStruct *TV);
TrivTVStruct *TrivCnvrtBsp2BzrTV(const TrivTVStruct *TV);

void TrivTVTransform(TrivTVStruct *TV, CagdRType *Translate, CagdRType Scale);
void TrivTVMatTransform(TrivTVStruct *TV, CagdMType Mat);

TrivTVStruct *TrivCoerceTVsTo(const TrivTVStruct *TV, CagdPointType PType);
TrivTVStruct *TrivCoerceTVTo(const TrivTVStruct *TV, CagdPointType PType);

void TrivTVDomain(const TrivTVStruct *TV,
		  CagdRType *UMin,
		  CagdRType *UMax,
		  CagdRType *VMin,
		  CagdRType *VMax,
		  CagdRType *WMin,
		  CagdRType *WMax);
CagdBType TrivParamInDomain(const TrivTVStruct *TV,
			    CagdRType t,
			    TrivTVDirType Dir);
CagdBType TrivParamsInDomain(const TrivTVStruct *TV,
			     CagdRType u,
			     CagdRType v,
			     CagdRType w);

CagdRType *TrivTVEval(const TrivTVStruct *TV,
		      CagdRType u,
		      CagdRType v,
		      CagdRType w);
CagdRType *TrivTVEval2(const TrivTVStruct *TV,
		       CagdRType u,
		       CagdRType v,
		       CagdRType w);
CagdSrfStruct *TrivSrfFromTV(const TrivTVStruct *TV,
			     CagdRType t,
			     TrivTVDirType Dir,
			     int OrientBoundary);
CagdSrfStruct **TrivBndrySrfsFromTV(const TrivTVStruct *TV);
CagdSrfStruct *TrivSrfFromMesh(const TrivTVStruct *TV,
			       int Index,
			       TrivTVDirType Dir);
void TrivSrfToMesh(const CagdSrfStruct *Srf,
		   int Index,
		   TrivTVDirType Dir,
		   TrivTVStruct *TV);
CagdRType *TrivTVMultEval(CagdRType *UKnotVector,
			  CagdRType *VKnotVector,
			  CagdRType *WKnotVector,
			  int ULength,
			  int VLength,
			  int WLength,
			  int UOrder,
			  int VOrder,
			  int WOrder,
			  CagdPType *Mesh,
			  CagdPType *Params,
			  int NumOfParams,
			  int *RetSize,
			  CagdBspBasisFuncMultEvalType EvalType);
void TrivTVBlockEvalInit(CagdRType *UKnotVector,
			 CagdRType *VKnotVector,
			 CagdRType *WKnotVector,
			 int Lengths[3],
			 int Orders[3],
			 int BlockSizes[3],
			 CagdPType *Params,
			 int NumOfParams[3]);
void TrivTVBlockEvalSetMesh(CagdPType *Mesh);
TrivTVBlockEvalStruct *TrivTVBlockEvalOnce(int i, int j, int k);
void TrivTVBlockEvalDone(void);
TrivTVStruct *TrivTVRegionFromTV(const TrivTVStruct *TV,
				 CagdRType t1,
				 CagdRType t2,
				 TrivTVDirType Dir);
TrivTVStruct *TrivTVRefineAtParams(const TrivTVStruct *TV,
				   TrivTVDirType Dir,
				   CagdBType Replace,
				   CagdRType *t,
				   int n);
TrivTVStruct *TrivBspTVKnotInsertNDiff(const TrivTVStruct *TV,
				       TrivTVDirType Dir,
				       int Replace,
				       const CagdRType *t,
				       int n);
TrivTVStruct *TrivTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivBzrTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivBzrTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivBspTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivBspTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivTVSubdivAtParam(const TrivTVStruct *TV,
				  CagdRType t,
				  TrivTVDirType Dir);
TrivTVStruct *TrivTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivTVDegreeRaiseN(const TrivTVStruct *TV,
				 TrivTVDirType Dir,
				 int NewOrder);
TrivTVStruct *TrivBspTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivBzrTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivTVBlossomDegreeRaise(const TrivTVStruct *TV,
				       TrivTVDirType Dir);
TrivTVStruct *TrivTVBlossomDegreeRaiseN(const TrivTVStruct *TV,
					int NewUOrder,
					int NewVOrder,
					int NewWOrder);
TrivTVStruct *TrivTVReverseDir(const TrivTVStruct *TV, TrivTVDirType Dir);
TrivTVStruct *TrivTVReverse2Dirs(const TrivTVStruct *TV,
				 TrivTVDirType Dir1,
				 TrivTVDirType Dir2);
CagdBType TrivMakeTVsCompatible(TrivTVStruct **TV1,
				TrivTVStruct **TV2,
				CagdBType SameUOrder,
				CagdBType SameVOrder,
				CagdBType SameWOrder,
				CagdBType SameUKV,
				CagdBType SameVKV,
				CagdBType SameWKV);
void TrivTVBBox(const TrivTVStruct *TV, CagdBBoxStruct *BBox);
void TrivTVListBBox(const TrivTVStruct *TVs, CagdBBoxStruct *BBox);
CagdPolylineStruct *TrivTV2CtrlMesh(const TrivTVStruct *Trivar);
TrivTVStruct *TrivInterpTrivar(const TrivTVStruct *TV);
TrivTVStruct *TrivTVInterpPts(const TrivTVStruct *PtGrid,
			      int UOrder,
			      int VOrder,
			      int WOrder,
			      int TVUSize,
			      int TVVSize,
			      int TVWSize);
TrivTVStruct *TrivTVInterpolate(const TrivTVStruct *PtGrid,
				int ULength,
				int VLength,
				int WLength,
				int UOrder,
				int VOrder,
				int WOrder);
TrivTVStruct *TrivTVInterpScatPts(const CagdCtlPtStruct *PtList,
				  int USize,
				  int VSize,
				  int WSize,
				  int UOrder,
				  int VOrder,
				  int WOrder,
				  CagdRType *UKV,
				  CagdRType *VKV,
				  CagdRType *WKV);
TrivTVStruct *TrivTVFromSrfs(const CagdSrfStruct *SrfList,
			     int OtherOrder,
			     CagdEndConditionType OtherEC,
			     IrtRType *OtherParamVals);
CagdRType *TrivTVInterpolateSrfsChordLenParams(const CagdSrfStruct *SrfList);
TrivTVStruct *TrivTVInterpolateSrfs(const CagdSrfStruct *SrfList,
				    int OtherOrder,
				    CagdEndConditionType OtherEC,
				    CagdParametrizationType OtherParam,
				    IrtRType *OtherParamVals);
TrivTVStruct *TrivRuledTV(const CagdSrfStruct *Srf1,
			  const CagdSrfStruct *Srf2,
			  int OtherOrder,
			  int OtherLen);
TrivTVStruct *TrivTrilinearSrf(const CagdPtStruct *Pt000,
			       const CagdPtStruct *Pt001,
			       const CagdPtStruct *Pt010,
			       const CagdPtStruct *Pt011,
			       const CagdPtStruct *Pt100,
			       const CagdPtStruct *Pt101,
			       const CagdPtStruct *Pt110,
			       const CagdPtStruct *Pt111);
TrivTVStruct *TrivExtrudeTV(const CagdSrfStruct *Srf,
			    const CagdVecStruct *Vec);
TrivTVStruct *TrivTVOfRev(const CagdSrfStruct *Srf);
TrivTVStruct *TrivTVOfRevAxis(const CagdSrfStruct *Srf, TrivVType Axis);
TrivTVStruct *TrivTVOfRevPolynomialApprox(const CagdSrfStruct *Srf);
TrivTVStruct *TrivEditSingleTVPt(TrivTVStruct *TV,
				 CagdCtlPtStruct *CtlPt,
				 int UIndex,
				 int VIndex,
				 int WIndex,
				 CagdBType Write);
CagdBType TrivTVsSame(const TrivTVStruct *Tv1,
		      const TrivTVStruct *Tv2,
		      CagdRType Eps);
CagdBType TrivBspTVHasBezierKVs(const TrivTVStruct *TV);
CagdBType TrivBspTVHasOpenEC(const TrivTVStruct *TV);

void TrivDbg(const void *Obj);

/******************************************************************************
* Routines to handle basis function conversions.			      *
******************************************************************************/
TrivTVStruct *TrivCnvrtPeriodic2FloatTV(const TrivTVStruct *TV);
TrivTVStruct *TrivCnvrtFloat2OpenTV(const TrivTVStruct *TV);
TrivTVStruct *TrivTVOpenEnd(const TrivTVStruct *TV);

/******************************************************************************
* Metamorphosis of trivariates.						      *
******************************************************************************/
TrivTVStruct *TrivTwoTVsMorphing(const TrivTVStruct *TV1,
				 const TrivTVStruct *TV2,
				 CagdRType Blend);

/******************************************************************************
* Local curvature processing.						      *
******************************************************************************/
CagdBType TrivEvalTVCurvaturePrelude(const TrivTVStruct *TV);
CagdBType TrivEvalCurvature(CagdPType Pos,
			    CagdRType *PCurv1,
			    CagdRType *PCurv2,
			    CagdVType PDir1,
			    CagdVType PDir2);
CagdBType TrivEvalGradient(CagdPType Pos, CagdVType Gradient);
CagdBType TrivEvalHessian(CagdPType Pos, CagdVType Hessian[3]);
void TrivEvalTVCurvaturePostlude(void);

/******************************************************************************
* Geometry in R^4.							      *
******************************************************************************/
int TrivPlaneFrom4Points(const TrivPType Pt1,
			 const TrivPType Pt2,
			 const TrivPType Pt3,
			 const TrivPType Pt4,
			 TrivIrtPlnType Plane);
void TrivVectCross3Vecs(const TrivVType A,
			const TrivVType B,
			const TrivVType C,
			TrivVType Res);

/******************************************************************************
* Error handling.							      *
******************************************************************************/
TrivSetErrorFuncType TrivSetFatalErrorFunc(TrivSetErrorFuncType ErrorFunc);
void TrivFatalError(TrivFatalErrorType ErrID);
const char *TrivDescribeError(TrivFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* TRIV_LIB_H */
