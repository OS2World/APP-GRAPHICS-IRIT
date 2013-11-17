/******************************************************************************
* MVar_lib.h - header file for the multi variate library.		      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 96.					      *
******************************************************************************/

#ifndef MVAR_LIB_H
#define MVAR_LIB_H

#include <stdio.h>
#include "irit_sm.h"
#include "miscattr.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "triv_lib.h"

typedef enum {
    MVAR_ERR_DIR_NOT_VALID,
    MVAR_ERR_UNDEF_CRV,
    MVAR_ERR_UNDEF_SRF,
    MVAR_ERR_UNDEF_MVAR,
    MVAR_ERR_UNDEF_GEOM,
    MVAR_ERR_GEOM_NO_SUPPORT,
    MVAR_ERR_RATIONAL_NO_SUPPORT,
    MVAR_ERR_RATIONAL_EXPECTED,
    MVAR_ERR_WRONG_ORDER,
    MVAR_ERR_KNOT_NOT_ORDERED,
    MVAR_ERR_NUM_KNOT_MISMATCH,
    MVAR_ERR_INDEX_NOT_IN_MESH,
    MVAR_ERR_POWER_NO_SUPPORT,
    MVAR_ERR_WRONG_DOMAIN,
    MVAR_ERR_INCONS_DOMAIN,
    MVAR_ERR_SCALAR_PT_EXPECTED,
    MVAR_ERR_INVALID_AXIS,
    MVAR_ERR_NO_CLOSED_POLYGON,
    MVAR_ERR_TWO_INTERSECTIONS,
    MVAR_ERR_NO_MATCH_PAIR,
    MVAR_ERR_FAIL_READ_FILE,
    MVAR_ERR_INVALID_STROKE_TYPE,
    MVAR_ERR_READ_FAIL,
    MVAR_ERR_MVS_INCOMPATIBLE,
    MVAR_ERR_PT_OR_LEN_MISMATCH,
    MVAR_ERR_TOO_FEW_PARAMS,
    MVAR_ERR_TOO_MANY_PARAMS,
    MVAR_ERR_FAIL_CMPT,
    MVAR_ERR_NO_CROSS_PROD,
    MVAR_ERR_BEZIER_EXPECTED,
    MVAR_ERR_BSPLINE_EXPECTED,
    MVAR_ERR_BEZ_OR_BSP_EXPECTED,
    MVAR_ERR_SAME_GTYPE_EXPECTED,
    MVAR_ERR_SAME_PTYPE_EXPECTED,
    MVAR_ERR_ONE_OR_THREE_EXPECTED,
    MVAR_ERR_POWER_EXPECTED,
    MVAR_ERR_MSC_TOO_FEW_OBJ,
    MVAR_ERR_MSC_FAILED,
    MVAR_ERR_MSS_INCONSISTENT_NUM_OBJ,
    MVAR_ERR_SCALAR_EXPECTED,
    MVAR_ERR_DIM_TOO_HIGH,
    MVAR_ERR_INVALID_MV,
    MVAR_ERR_CANNT_MIX_BSP_BEZ,
    MVAR_ERR_CH_FAILED,
    MVAR_ERR_MSC_CURVES,
    MVAR_ERR_ONLY_2D,
    MVAR_ERR_ONLY_3D,
    MVAR_ERR_2D_OR_3D,
    MVAR_ERR_1D_OR_3D,
    MVAR_ERR_WRONG_INDEX,
    MVAR_ERR_MSC_TOO_FEW_PTS,
    MVAR_ERR_ET_DFRNT_DOMAINS,
    MVAR_ERR_SRF_NOT_ADJ,
    MVAR_ERR_CURVATURE_CONT,
    MVAR_ERR_UNDEFINE_ERR
} MvarFatalErrorType;

typedef enum {
    MVAR_UNDEF_TYPE = 1240,
    MVAR_BEZIER_TYPE,
    MVAR_BSPLINE_TYPE,
    MVAR_POWER_TYPE
} MvarGeomType;

typedef enum {
    MVAR_CNSTRNT_ZERO = 1320,
    MVAR_CNSTRNT_ZERO_SUBDIV,    /* Examine zeros during subdiv. stage only. */
    MVAR_CNSTRNT_POSITIVE,  /* Examine positivity during subdiv. stage only. */
    MVAR_CNSTRNT_NEGATIVE   /* Examine negativity during subdiv. stage only. */
} MvarConstraintType;

/* The following type should match CagdPointType for the shared domain.      */
typedef enum {		/* Type of control point. The P-types are rationals. */
    MVAR_PT_NONE = 0,
    MVAR_PT_BASE = 1100,    /* Must be an even number, same as CAGD_PT_BASE. */
    MVAR_PT_E1_TYPE = 1100,                      /* same as CAGD_PT_E1_TYPE. */
    MVAR_PT_P1_TYPE,
    MVAR_PT_E2_TYPE,
    MVAR_PT_P2_TYPE,
    MVAR_PT_E3_TYPE,
    MVAR_PT_P3_TYPE,
    MVAR_PT_E4_TYPE,
    MVAR_PT_P4_TYPE,
    MVAR_PT_E5_TYPE,
    MVAR_PT_P5_TYPE,
    MVAR_PT_E6_TYPE,
    MVAR_PT_P6_TYPE,
    MVAR_PT_E7_TYPE,
    MVAR_PT_P7_TYPE,
    MVAR_PT_E8_TYPE,
    MVAR_PT_P8_TYPE,
    MVAR_PT_E9_TYPE,
    MVAR_PT_P9_TYPE,
    MVAR_PT_E10_TYPE,
    MVAR_PT_P10_TYPE,
    MVAR_PT_E11_TYPE,
    MVAR_PT_P11_TYPE,
    MVAR_PT_E12_TYPE,
    MVAR_PT_P12_TYPE,
    MVAR_PT_E13_TYPE,
    MVAR_PT_P13_TYPE,
    MVAR_PT_E14_TYPE,
    MVAR_PT_P14_TYPE,
    MVAR_PT_E15_TYPE,
    MVAR_PT_P15_TYPE,
    MVAR_PT_E16_TYPE,
    MVAR_PT_P16_TYPE,
    MVAR_PT_E17_TYPE,
    MVAR_PT_P17_TYPE,
    MVAR_PT_E18_TYPE,
    MVAR_PT_P18_TYPE,
    MVAR_PT_E19_TYPE,
    MVAR_PT_P19_TYPE,
    MVAR_PT_MAX_SIZE_TYPE	     /* See also MVAR_MAX_* constants below. */
} MvarPointType;

#define MVAR_MAX_PT_SIZE		20    /* Rational P19 has 20 coords. */
#define MVAR_MAX_PT_COORD		19		       /* Without w. */

typedef enum {
    MVAR_SK2D_PRIM_POINT,
    MVAR_SK2D_PRIM_LINE,
    MVAR_SK2D_PRIM_ARC,
    MVAR_SK2D_PRIM_CRV
} MvarSkel2DPrimType;

typedef enum {
    MVAR_ET_NODE_NONE,
    MVAR_ET_NODE_LEAF,
    MVAR_ET_NODE_ADD,
    MVAR_ET_NODE_SUB,
    MVAR_ET_NODE_MULT,
    MVAR_ET_NODE_DOT_PROD,
    MVAR_ET_NODE_CROSS_PROD,
    MVAR_ET_NODE_EXP,
    MVAR_ET_NODE_LOG,
    MVAR_ET_NODE_COS,
    MVAR_ET_NODE_SQRT,
    MVAR_ET_NODE_RECIP,
    MVAR_ET_NODE_COMMON_EXPR
} MvarExprTreeNodeType;

typedef int MvarMVDirType;
typedef CagdRType MvarMinMaxType[2];

#define MVAR_HF_DIST_MAX_PARAM		3

#define MVAR_IS_RATIONAL_PT(PType)  ((int) ((PType) & 0x01))
#define MVAR_IS_RATIONAL_MV(MV)		MVAR_IS_RATIONAL_PT((MV) -> PType)
#define MVAR_NUM_OF_PT_COORD(PType) ((((int) ((PType) - MVAR_PT_BASE)) >> 1) + 1)
#define MVAR_NUM_OF_MV_COORD(MV)    ((((int) (((MV) -> PType) - \
				              MVAR_PT_BASE)) >> 1) + 1)
#define MVAR_MAKE_PT_TYPE(IsRational, NumCoords) \
				    ((MvarPointType) (MVAR_PT_BASE + \
				         ((((IsRational) ? -1 : -2) \
						       + ((NumCoords) << 1)))))

#define MVAR_PREV_DIR(Dir) ((Dir) + 1)
#define MVAR_NEXT_DIR(Dir) ((Dir) - 1)

#define MVAR_PT_RESET(P) IRIT_ZAP_MEM((P) -> Pt, (P) -> Dim * sizeof(CagdRType))
#define MVAR_VEC_RESET(V) IRIT_ZAP_MEM((V) -> Vec, (V) -> Dim * sizeof(CagdRType))
#define MVAR_PLANE_RESET(P) IRIT_ZAP_MEM((P) -> Pln, (P) -> Dim * sizeof(CagdRType))

#define MVAR_PT_COPY(Dst, Src) IRIT_GEN_COPY((Dst) -> Pt, (Src) -> Pt, \
				             (Dst) -> Dim * sizeof(CagdRType))
#define MVAR_VEC_COPY(Dst, Src) IRIT_GEN_COPY((Dst) -> Vec, (Src) -> Vec, \
				              (Dst) -> Dim * sizeof(CagdRType))
#define MVAR_PLANE_COPY(Dst, Src) IRIT_GEN_COPY((Dst) -> Plane, (Src) -> Plane, \
				                (Dst) -> Dim * sizeof(CagdRType))

typedef struct MvarPtStruct {
    struct MvarPtStruct *Pnext;
    struct IPAttributeStruct *Attr;
    int Dim;				     /* Number of coordinates in Pt. */
    CagdRType *Pt;	       /* The coordinates of the multivariate point. */
} MvarPtStruct;

typedef struct MvarVecStruct {
    struct MvarVecStruct *Pnext;
    struct IPAttributeStruct *Attr;
    int Dim;				    /* Number of coordinates in Vec. */
    CagdRType *Vec;	      /* The coordinates of the multivariate vector. */
} MvarVecStruct;

typedef struct MvarPolyStruct {			    /* A polyline structure. */
    struct MvarPolyStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MvarPtStruct *Pl;
    VoidPtr PAux;
} MvarPolyStruct;

typedef struct MvarPlaneStruct {
    struct MvarPlaneStruct *Pnext;
    struct IPAttributeStruct *Attr;
    int Dim;    /* Number of coordinates in Pln (one above space dimension). */
    CagdRType *Pln;	       /* The coordinates of the multivariate plane. */
} MvarPlaneStruct;

typedef struct MvarBBoxStruct {
    struct MvarBBoxStruct *Pnext;
    struct IPAttributeStruct *Attr;
    int Dim;
    CagdRType Min[MVAR_MAX_PT_COORD];
    CagdRType Max[MVAR_MAX_PT_COORD];
} MvarBBoxStruct;

typedef struct MvarNormalConeStruct {        /* Normalized cone axis vector. */
    struct MvarNormalConeStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MvarVecStruct *ConeAxis;
    CagdRType ConeAngleCosine;
    CagdRType AxisMinMax[2];
} MvarNormalConeStruct;

typedef struct MvarMVStruct {
    struct MvarMVStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MvarGeomType GType;
    MvarPointType PType;
    int Dim;		      /* Number of dimensions in this multi variate. */
    int *Lengths;               /* Dimensions of mesh size in multi-variate. */
    int *SubSpaces;	   /* SubSpaces[i] = Prod(i = 0, i-1) of Lengths[i]. */
    int *Orders;                  /* Orders of multi variate (Bspline only). */
    CagdBType *Periodic;            /* Periodicity - valid only for Bspline. */
    CagdRType *Points[MVAR_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    CagdRType **KnotVectors;
    MvarMinMaxType *AuxDomain;		      /* Optional to hold MV domain. */
    VoidPtr PAux;			        /* Auxiliary data structure. */
    VoidPtr PAux2;			        /* Auxiliary data structure. */
} MvarMVStruct;

typedef struct MvarExprTreeStruct {
    MvarExprTreeNodeType NodeType;
    int Dim;
    int PtSize;					    /* vector function size. */
#ifndef SUN4                       /* No support form nameless union/struct. */
    union {
	struct {
#endif /* SUN4 */
	    MvarMVStruct *MV;
	    CagdBType IsRef;       /* TRUE if an MV reference - do not free. */
#ifndef SUN4
	}; /* Leaf node. */
	struct {
#endif /* SUN4 */
	    struct MvarExprTreeStruct *Left, *Right;
#ifndef SUN4
	}; /* Internal node. */
    };
#endif /* SUN4 */
    MvarNormalConeStruct *MVBCone;
    MvarBBoxStruct MVBBox;
    int IAux;			      /* Auxiliary integer for internal use. */
    VoidPtr PAux;		      /* Auxiliary pointer for internal use. */
    char *Info;			   /* Optional info on this expression tree. */
} MvarExprTreeStruct;

typedef struct MvarMVGradientStruct {
    int Dim;
    CagdBType IsRational, HasOrig;
    MvarMVStruct *MV;			       /* The original multivariate. */
    MvarMVStruct *MVGrad;		    /* The gradient if not rational. */
    MvarMVStruct *MVRGrad[MVAR_MAX_PT_COORD + 1];  /* The grad. if rational. */
} MvarMVGradientStruct;

/* Eqns structure - holds a set of expression trees and related info.        */
typedef struct MvarExprTreeEqnsStruct {
    MvarExprTreeStruct **Eqns;		          /* The equations to solve. */
    int NumEqns, NumZeroEqns, NumZeroSubdivEqns;
    MvarExprTreeStruct **CommonExprs;       /* The common expressions found. */
    int NumCommonExprs, MaxNumCommonExprs;
    MvarConstraintType *ConstraintTypes;
} MvarExprTreeEqnsStruct;

typedef struct MvarSkel2DPrimPointStruct {
    CagdPType Pt;
} MvarSkel2DPrimPointStruct;

typedef struct MvarSkel2DPrimLineStruct {
    CagdPType Pt1, Pt2;
} MvarSkel2DPrimLineStruct;

typedef struct MvarSkel2DPrimArcStruct {
    CagdPType Center;
    IrtRType StartAngle, EndAngle, Radius;
} MvarSkel2DPrimArcStruct;

typedef struct MvarSkel2DPrimCrvStruct {
    CagdCrvStruct *Crv;
} MvarSkel2DPrimCrvStruct;

typedef struct MvarSkel2DPrimStruct {
    struct MvarSkel2DPrimStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MvarSkel2DPrimType Type;
#ifndef SUN4                       /* No support form nameless union/struct. */
    union {
#endif /* SUN4 */
        MvarSkel2DPrimPointStruct Pt;
        MvarSkel2DPrimLineStruct Ln;
        MvarSkel2DPrimArcStruct Arc;
        MvarSkel2DPrimCrvStruct Crv;
#ifndef SUN4
    };
#endif /* SUN4 */
    int _Index;
    CagdCrvStruct *_CrvRep;
} MvarSkel2DPrimStruct;

typedef struct MvarSkel2DInter3PrimsStruct {
    struct MvarSkel2DInter3PrimsStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdPType PosPrim1, PosPrim2, PosPrim3;
    CagdPType EquadistPoint;
} MvarSkel2DInter3PrimsStruct;

typedef struct MvarHFDistParamStruct {
    int NumOfParams;  /* Number of locations where the Hausdorff dist holds. */
    int ManifoldDim;                        /* 1 for curves, 2 for surfaces. */
#ifndef SUN4                       /* No support form nameless union/struct. */
    union {
#endif /* SUN4 */
	CagdRType T[MVAR_HF_DIST_MAX_PARAM];		/* Params of curves. */
	CagdUVType UV[MVAR_HF_DIST_MAX_PARAM];	      /* Params of surfaces. */
#ifndef SUN4
    };
#endif /* SUN4 */
} MvarHFDistParamStruct;

typedef struct MvarHFDistPairParamStruct {
    struct MvarHFDistPairParamStruct *Pnext;
    MvarHFDistParamStruct Param1, Param2;	 /* Param. info of the pair. */
    CagdRType Dist;		     /* Euclidean distance at this location. */
} MvarHFDistPairParamStruct;

typedef void (*MvarSetErrorFuncType)(MvarFatalErrorType);
typedef void (*MvarExprTreePrintFuncType)(const char *);
typedef int (*MvarMVsZerosSubdivCallBackFunc)(MvarMVStruct ***MVs,
					      MvarConstraintType **Constraints,
					      int *NumOfMVs,
					      int *NumOfZeroMVs,
					      int Depth);

#define MVAR_MALLOC_STRUCT_ONCE     /* Faster allocation of MVAR structures. */

#define MVAR_IS_BEZIER_MV(MV)		((MV) -> GType == MVAR_BEZIER_TYPE)
#define MVAR_IS_POWER_MV(MV)		((MV) -> GType == MVAR_POWER_TYPE)
#define MVAR_IS_BSPLINE_MV(MV)		((MV) -> GType == MVAR_BSPLINE_TYPE)

#define MVAR_CTL_MESH_LENGTH(MV)	((MV) -> SubSpaces[(MV) -> Dim])

/******************************************************************************
*  Provides easy access to multivariates up to dimension six.		      *
******************************************************************************/
#define MVAR_NEXT_U(MV)			(1)         /* == MV -> SubSpaces[0] */
#define MVAR_NEXT_V(MV)			((MV) -> SubSpaces[1])
#define MVAR_NEXT_W(MV)			((MV) -> SubSpaces[2])
#define MVAR_NEXT_FOURTH(MV)		((MV) -> SubSpaces[3])
#define MVAR_NEXT_FIFTH(MV)		((MV) -> SubSpaces[4])
#define MVAR_NEXT_SIXTH(MV)		((MV) -> SubSpaces[5])
#define MVAR_NEXT_DIM(MV, Dim)		((MV) -> SubSpaces[(Dim)])

#define MVAR_MESH_UV(MV, i, j)		((i) + \
					 ((MV) -> SubSpaces[1]) * (j))
#define MVAR_MESH_UVW(MV, i, j, k)	((i) + \
					 ((MV) -> SubSpaces[1]) * (j) + \
					 ((MV) -> SubSpaces[2]) * (k))
#define MVAR_MESH_UVW4(MV, i, j, k, l)  ((i) + \
					 ((MV) -> SubSpaces[1]) * (j) + \
					 ((MV) -> SubSpaces[2]) * (k) + \
					 ((MV) -> SubSpaces[3]) * (l))
#define MVAR_MESH_UVW45(MV, i, j, k, l, m) \
					((i) + \
					 ((MV) -> SubSpaces[1]) * (j) + \
					 ((MV) -> SubSpaces[2]) * (k) + \
					 ((MV) -> SubSpaces[3]) * (l) + \
					 ((MV) -> SubSpaces[4]) * (m))
#define MVAR_MESH_UVW456(MV, i, j, k, l, m, n) \
					((i) + \
					 ((MV) -> SubSpaces[1]) * (j) + \
					 ((MV) -> SubSpaces[2]) * (k) + \
					 ((MV) -> SubSpaces[3]) * (l) + \
					 ((MV) -> SubSpaces[4]) * (m) + \
					 ((MV) -> SubSpaces[5]) * (n))

/* If a mvarariate is periodic, the control polygon/mesh should warp up.     */
/* Length does hold the real allocated length but the virtual periodic       */
/* length is a little larger. Note allocated KV's are larger.                */
#define MVAR_MVAR_UPT_LST_LEN(MV)	((MV) -> Lengths[0] + \
			 ((MV) -> Periodic[0] ? (MV) -> Orders[0] - 1 : 0))
#define MVAR_MVAR_VPT_LST_LEN(MV)	((MV) -> Lengths[1] + \
			 ((MV) -> Periodic[1] ? (MV) -> Orders[1] - 1 : 0))
#define MVAR_MVAR_WPT_LST_LEN(MV)	((MV) -> Lengths[2] + \
			 ((MV) -> Periodic[2] ? (MV) -> Orders[2] - 1 : 0))
#define MVAR_MVAR_FOURTH_PT_LST_LEN(MV)	((MV) -> Lengths[3] + \
			 ((MV) -> Periodic[3] ? (MV) -> Orders[3] - 1 : 0))
#define MVAR_MVAR_FIFTH_PT_LST_LEN(MV)	((MV) -> Lengths[4] + \
			 ((MV) -> Periodic[4] ? (MV) -> Orders[4] - 1 : 0))
#define MVAR_MVAR_SIXTH_PT_LST_LEN(MV)	((MV) -> Lengths[5] + \
			 ((MV) -> Periodic[5] ? (MV) -> Orders[5] - 1 : 0))
#define MVAR_MVAR_ITH_PT_LST_LEN(MV, i)	((MV) -> Lengths[i] + \
			 ((MV) -> Periodic[i] ? (MV) -> Orders[i] - 1 : 0))

#define MVAR_IS_UPERIODIC_MVAR(MV)	((MV) -> Periodic[0])
#define MVAR_IS_VPERIODIC_MVAR(MV)	((MV) -> Periodic[1])
#define MVAR_IS_WPERIODIC_MVAR(MV)	((MV) -> Periodic[2])
#define MVAR_IS_FOURTH_PERIODIC_MVAR(MV) ((MV) -> Periodic[3])
#define MVAR_IS_FIFTH_PERIODIC_MVAR(MV) ((MV) -> Periodic[4])
#define MVAR_IS_SIXTH_PERIODIC_MVAR(MV) ((MV) -> Periodic[5])
#define MVAR_IS_ITH_PERIODIC_MVAR(MV, i) ((MV) -> Periodic[i])

/* Ease the handling of the splitting of a multivariate to scalar fields. */
#define MVAR_CLEAR_SCALARS(MV) { \
	int ii; \
	for (ii = 0; ii < MVAR_MAX_PT_SIZE; ii++) \
	    (MV)[ii] = NULL; \
    } 
#define MVAR_FREE_SCALARS(MV) { \
        int ii; \
        if ((MV)[0] != NULL) \
	    MvarMVFree((MV)[0]); \
	for (ii = 1; ii <= MVAR_MAX_PT_COORD; ii++) { \
	    if ((MV)[ii] == NULL) \
	        break; \
	    MvarMVFree((MV)[ii]); \
	} \
    }
#define MVAR_SPLIT_SCALARS(MV, MVScalar) \
	    CAGD_GEN_COPY((MVScalar), MvarMVSplitScalar(MV), \
			  sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);

#define MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index) \
    (++(*Indices) >= MV -> Lengths[0] ? \
	_MvarIncrementMeshIndices(MV, Indices, &Index) : ++Index)

#define MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices, Index) \
    (++(*Indices) >= MV -> Orders[0] ? \
	_MvarIncrementMeshOrderIndices(MV, Indices, &Index) : ++Index)

#define MVAR_INC_SKIP_MESH_INDICES_1ST(MV, Indices) \
    (++Indices[1] >= MV -> Lengths[1] ? \
	_MvarIncSkipMeshIndices1st(MV, Indices) : 1)

#define MVAR_INC_SKIP_MESH_INDICES(MV, Indices, SkipDir, Index) \
    (MV -> Dim <= 1 ? (Index = 0) : \
     (++(Indices[SkipDir == 0]) >= MV -> Lengths[SkipDir == 0] ? \
	 _MvarIncSkipMeshIndices(MV, Indices, SkipDir, &Index) : \
	 (Index += MVAR_NEXT_DIM(MV, SkipDir == 0))))

#define MVAR_INC_BOUND_MESH_INDICES(MV, Indices, LowBound, UpBound, Index) \
    (++(*Indices) >= UpBound[0] ? \
	_MvarIncBoundMeshIndices(MV, Indices, LowBound, UpBound, \
				 &Index) : ++Index)

#define MVAR_BBOX_RESET(BBox) \
    (BBox).Dim = 0; \
    IRIT_ZAP_MEM((BBox).Min, sizeof(CagdRType) * MVAR_MAX_PT_COORD); \
    IRIT_ZAP_MEM((BBox).Max, sizeof(CagdRType) * MVAR_MAX_PT_COORD);

#define MVAR_IS_BBOX_RESET(BBox)  (BBox).Dim == 0

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* General routines of the Mvar library:					      *
******************************************************************************/
MvarMVStruct *MvarMVNew(int Dim,
			MvarGeomType GType,
			MvarPointType PType,
			const int *Lengths);
MvarMVStruct *MvarBspMVNew(int Dim,
			   const int *Lengths,
			   const int *Orders,
			   MvarPointType PType);
MvarMVStruct *MvarBzrMVNew(int Dim, const int *Lengths, MvarPointType PType);
MvarMVStruct *MvarPwrMVNew(int Dim, const int *Lengths, MvarPointType PType);
MvarMVStruct *MvarBuildParamMV(int Dim, int Dir, CagdRType Min, CagdRType Max);
MvarMVStruct *MvarMVCopy(const MvarMVStruct *MV);
MvarMVStruct *MvarMVCopyList(const MvarMVStruct *MVList);
void MvarMVFree(MvarMVStruct *MV);
void MvarMVFreeList(MvarMVStruct *MVList);

#ifdef DEBUG
#define MvarMVFree(MV)         { MvarMVFree(MV); MV = NULL; }
#define MvarMVFreeList(MVList) { MvarMVFreeList(MVList); MVList = NULL; }
#endif /* DEBUG */

MvarPtStruct *MvarPtNew(int Dim);
MvarPtStruct *MvarPtRealloc(MvarPtStruct *Pt, int NewDim);
MvarPtStruct *MvarPtCopy(const MvarPtStruct *Pt);
MvarPtStruct *MvarPtCopyList(const MvarPtStruct *PtList);
MvarPtStruct *MvarPtSortListAxis(MvarPtStruct *PtList, int Axis);
void MvarPtFree(MvarPtStruct *Pt);
void MvarPtFreeList(MvarPtStruct *PtList);

#ifdef DEBUG
#define MvarPtFree(Pt)         { MvarPtFree(Pt); Pt = NULL; }
#define MvarPtFreeList(PtList) { MvarPtFreeList(PtList); PtList = NULL; }
#endif /* DEBUG */

MvarPtStruct *MvarPolyReverseList(MvarPtStruct *Pts);

MvarPolyStruct *MvarPolyNew(MvarPtStruct *Pl);
MvarPolyStruct *MvarPolyCopy(const MvarPolyStruct *Poly);
MvarPolyStruct *MvarPolyCopyList(MvarPolyStruct *PolyList);
void MvarPolyFree(MvarPolyStruct *Poly);
void MvarPolyFreeList(MvarPolyStruct *PolyList);

#ifdef DEBUG
#define MvarPolyFree(Poly)         { MvarPolyFree(Poly); Poly = NULL; }
#define MvarPolyFreeList(PolyList) { MvarPolyFreeList(PolyList); \
				     PolyList = NULL; }
#endif /* DEBUG */

MvarVecStruct *MvarVecNew(int Dim);
MvarVecStruct *MvarVecArrayNew(int Size, int Dim);
MvarVecStruct *MvarVecRealloc(MvarVecStruct *Vec, int NewDim);
MvarVecStruct *MvarVecCopy(const MvarVecStruct *Vec);
MvarVecStruct *MvarVecCopyList(const MvarVecStruct *VecList);
void MvarVecFree(MvarVecStruct *Vec);
void MvarVecFreeList(MvarVecStruct *VecList);
void MvarVecArrayFree(MvarVecStruct *MVVecArray, int Size);
void MvarVecAdd(MvarVecStruct *VRes,
		const MvarVecStruct *V1,
		const MvarVecStruct *V2);
void MvarVecSub(MvarVecStruct *VRes,
		const MvarVecStruct *V1,
		const MvarVecStruct *V2);
CagdRType MvarVecDotProd(const MvarVecStruct *V1, const MvarVecStruct *V2);
CagdRType MvarVecSqrLength(const MvarVecStruct *V);
CagdRType MvarVecSqrLength2(const CagdRType *v, int Dim);
CagdRType MvarVecLength(const MvarVecStruct *V);
MvarVecStruct *MvarVecScale(MvarVecStruct *V, CagdRType ScaleFactor);
void MvarVecBlend(MvarVecStruct *VRes,
		  const MvarVecStruct *V1,
		  const MvarVecStruct *V2,
		  CagdRType t);
int MvarVecNormalize(MvarVecStruct *V);
int MvarVecOrthogonalize(MvarVecStruct *Dir, const MvarVecStruct *Vec);
int MvarVecOrthogonal2(MvarVecStruct *Dir,
		       const MvarVecStruct *Vec1,
		       const MvarVecStruct *Vec2);
int MvarPlaneNormalize(MvarPlaneStruct *Pln);
MvarVecStruct *MvarLinePlaneInter(const MvarVecStruct *P,
				  const MvarVecStruct *V,
				  const MvarPlaneStruct *Pln,
				  CagdRType *Param);

#ifdef DEBUG
#define MvarVecFree(Vec)         { MvarVecFree(Vec); Vec = NULL; }
#define MvarVecFreeList(VecList) { MvarVecFreeList(VecList); VecList = NULL; }
#define MvarVecArrayFree(VecArray, Size) { MvarVecArrayFree(VecArray, Size); \
					   VecArray = NULL; }
#endif /* DEBUG */

MvarPlaneStruct *MvarPlaneNew(int Dim);
MvarPlaneStruct *MvarPlaneCopy(const MvarPlaneStruct *Plane);
MvarPlaneStruct *MvarPlaneCopyList(const MvarPlaneStruct *PlaneList);
void MvarPlaneFree(MvarPlaneStruct *Plane);
void MvarPlaneFreeList(MvarPlaneStruct *PlaneList);

#ifdef DEBUG
#define MvarPlaneFree(Plane)         { MvarPlaneFree(Plane); Plane = NULL; }
#define MvarPlaneFreeList(PlaneList) { MvarPlaneFreeList(PlaneList); \
				       PlaneList = NULL; }
#endif /* DEBUG */

MvarNormalConeStruct *MvarNormalConeNew(int Dim);
MvarNormalConeStruct *MvarNormalConeCopy(const MvarNormalConeStruct
					                         *NormalCone);
void MvarNormalConeFree(MvarNormalConeStruct *NormalCone);
void MvarNormalConeFreeList(MvarNormalConeStruct *NormalConeList);

#ifdef DEBUG
#define MvarConeFree(Cone)         { MvarConeFree(Cone); PCone = NULL; }
#endif /* DEBUG */

MvarPtStruct *MvarGetLastPt(MvarPtStruct *Pts);
int MvarPtCmpTwoPoints(const MvarPtStruct *P1,
		       const MvarPtStruct *P2,
		       CagdRType Eps);
CagdRType MvarPtDistTwoPoints(const MvarPtStruct *P1, const MvarPtStruct *P2);
CagdRType MvarPtDistSqrTwoPoints(const MvarPtStruct *P1,
				 const MvarPtStruct *P2);
MvarPtStruct *MvarPtInBetweenPoint(const MvarPtStruct *Pt1,
				   const MvarPtStruct *Pt2,
				   CagdRType t);
MvarPolyStruct *MvarPolyMergePolylines(MvarPolyStruct *Polys, IrtRType Eps);
MvarPolyStruct *MvarMatchPointListIntoPolylines(const MvarPtStruct *PtsList,
						IrtRType MaxTol);
struct IPObjectStruct *MvarCnvrtMVPtsToCtlPts(const MvarPtStruct *MVPts,
					      IrtRType MergeTol);
struct IPObjectStruct *MvarCnvrtMVPtsToPolys(const MvarPtStruct *MVPts,
					     const MvarMVStruct *MV,
					     IrtRType MergeTol);
struct IPPolygonStruct *MvarCnvrtMVPtsToPolys2(const MvarPtStruct *InPts,
					       CagdRType FineNess,
					       int Dim,
					       IrtRType *ParamDomain);
struct IPObjectStruct *MvarCnvrtMVPolysToIritPolys(const MvarPolyStruct *MVPlls);
struct IPObjectStruct *MvarCnvrtMVPolysToIritPolys2(const MvarPolyStruct *MVPlls,
						    int IgnoreIndividualPts);

MvarMVStruct *MvarCnvrtBzr2BspMV(const MvarMVStruct *MV);
MvarMVStruct *MvarCnvrtBsp2BzrMV(const MvarMVStruct *MV);
MvarMVStruct *MvarCnvrtPwr2BzrMV(const MvarMVStruct *MV);
MvarMVStruct *MvarCnvrtBzr2PwrMV(const MvarMVStruct *MV);

MvarMVStruct *MvarBzrLinearInOneDir(int Dim, int Dir, MvarPointType PType);

void MvarMVTransform(MvarMVStruct *MV, CagdRType *Translate, CagdRType Scale);
void MvarMVMatTransform(MvarMVStruct *MV, CagdMType Mat);

MvarMVStruct *MvarCoerceMVsTo(const MvarMVStruct *MV, MvarPointType PType);
MvarMVStruct *MvarCoerceMVTo(const MvarMVStruct *MV, MvarPointType PType);
MvarPointType MvarMergeIrtPtType(MvarPointType PType1, MvarPointType PType2);

void MvarMVDomain(const MvarMVStruct *MV,
		  CagdRType *Min,
		  CagdRType *Max, 
		  int Axis);
void MvarMVDomainAlloc(const MvarMVStruct *MV,
		       CagdRType **MinDmn, 
		       CagdRType **MaxDmn);
void MvarMVDomainFree(CagdRType *MinDmn, CagdRType *MaxDmn);
MvarMVStruct *MvarMVSetDomain(MvarMVStruct *MV,
			      CagdRType Min,
			      CagdRType Max,
			      int Axis,
			      int InPlace);
IrtRType MvarMVVolumeOfDomain(MvarMVStruct * const MVs, int Dim);

void MvarMVAuxDomainSlotReset(MvarMVStruct *MV);
int MvarMVAuxDomainSlotCopy(MvarMVStruct *MVDst, const MvarMVStruct *MVSrc);
void MvarMVAuxDomainSlotSet(MvarMVStruct *MV,
			    CagdRType Min,
			    CagdRType Max,
			    int Dir);
void MvarMVAuxDomainSlotSetRel(MvarMVStruct *MV,
			       CagdRType Min,
			       CagdRType Max,
			       int Dir);
int MvarMVAuxDomainSlotGet(const MvarMVStruct *MV,
			   CagdRType *Min,
			   CagdRType *Max,
			   int Dir);

MvarMVStruct *MvarMVSetAllDomains(MvarMVStruct *MV,
				  CagdRType *Min,
				  CagdRType *Max,
				  int InPlace);
CagdBType MvarParamInDomain(const MvarMVStruct *MV,
			    CagdRType t,
			    MvarMVDirType Dir);
CagdBType MvarParamsInDomain(const MvarMVStruct *MV, const CagdRType *Params);
void MvarUpdateConstDegDomains(MvarMVStruct **MVs, int NumOfMVs);
MvarPtStruct *MvarMVIntersPtOnBndry(MvarMVStruct *MV, 
				    MvarPtStruct *PointIns, 
				    MvarPtStruct *PointOuts);

CagdRType *MvarMVEval(const MvarMVStruct *MV, CagdRType *Params);
CagdRType *MvarMVEval2(const MvarMVStruct *MV, CagdRType *Params);
CagdRType *MvarMVEvalGradient2(const MvarMVStruct *MV,
			       CagdRType *Params,
			       int *HasOrig);
MvarPlaneStruct *MvarMVEvalTanPlane(const MvarMVStruct *MV, CagdRType *Params);

MvarMVStruct *MvarMVFromMV(const MvarMVStruct *MV,
			   CagdRType t,
			   MvarMVDirType Dir);
MvarMVStruct *MvarMVFromMesh(const MvarMVStruct *MV,
			     int Index,
			     MvarMVDirType Dir);
MvarMVStruct *MvarCrvToMV(const CagdCrvStruct *Crv);
CagdCrvStruct *MvarMVToCrv(const MvarMVStruct *MV);
MvarMVStruct *MvarSrfToMV(const CagdSrfStruct *Srf);
CagdSrfStruct *MvarMVToSrf(const MvarMVStruct *MV);
MvarMVStruct *MvarTVToMV(const TrivTVStruct *TV);
TrivTVStruct *MvarMVToTV(const MvarMVStruct *MV);

MvarMVStruct *MvarMVRegionFromMV(const MvarMVStruct *MV,
				 CagdRType t1,
				 CagdRType t2,
				 MvarMVDirType Dir);
MvarMVStruct *MvarMVRefineAtParams(const MvarMVStruct *MV,
				   MvarMVDirType Dir,
				   CagdBType Replace,
				   CagdRType *t,
				   int n);
MvarMVStruct *MvarBspMVKnotInsertNDiff(const MvarMVStruct *MV,
				       MvarMVDirType Dir,
				       int Replace,
				       CagdRType *t,
				       int n);
MvarMVStruct *MvarMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir);
MvarMVStruct *MvarBzrMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir);
MvarMVStruct *MvarBspMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir);
void MvarMVDeriveBound(const MvarMVStruct *MV,
		       MvarMVDirType Dir,
		       CagdRType MinMax[2]);
void MvarBzrMVDeriveBound(const MvarMVStruct *MV,
			  MvarMVDirType Dir,
			  CagdRType MinMax[2]);
void MvarBspMVDeriveBound(const MvarMVStruct *MV,
			  MvarMVDirType Dir,
			  CagdRType MinMax[2]);
void MvarMVDeriveAllBounds(const MvarMVStruct *MV, CagdMinMaxType *MinMax);
void MvarBzrMVDeriveAllBounds(const MvarMVStruct *MV, CagdMinMaxType *MinMax);
void MvarBspMVDeriveAllBounds(const MvarMVStruct *MV, CagdMinMaxType *MinMax);
MvarMVGradientStruct *MvarMVPrepGradient(const MvarMVStruct *MV,
					 CagdBType Orig);
void MvarMVFreeGradient(MvarMVGradientStruct *MV);
CagdRType *MvarMVEvalGradient(const MvarMVGradientStruct *MV,
			      CagdRType *Params,
			      int Axis);
MvarMVGradientStruct *MvarMVBoundGradient(const MvarMVStruct *MV);
MvarMVStruct *MvarMVSubdivAtParam(const MvarMVStruct *MV,
				  CagdRType t,
				  MvarMVDirType Dir);
MvarMVStruct *MvarBspMVSubdivAtParam(const MvarMVStruct *MV,
				     CagdRType t,
				     MvarMVDirType Dir);
MvarMVStruct *MvarBzrMVSubdivAtParam(const MvarMVStruct *MV,
				     CagdRType t,
				     MvarMVDirType Dir);
MvarMVStruct *MvarMVDegreeRaise(const MvarMVStruct *MV, MvarMVDirType Dir);
MvarMVStruct *MvarMVDegreeRaiseN(const MvarMVStruct *MV, int *NewOrders);
MvarMVStruct *MvarMVPwrDegreeRaise(const MvarMVStruct *MV,
				   int Dir,
				   int IncOrder);
CagdBType MvarMakeMVsCompatible(MvarMVStruct **MV1,
				MvarMVStruct **MV2,
				CagdBType SameOrders,
				CagdBType SameKVs);
void MvarMVBBox(const MvarMVStruct *MV, MvarBBoxStruct *BBox);
void MvarMVListBBox(const MvarMVStruct *MVs, MvarBBoxStruct *BBox);
void MvarMergeBBox(MvarBBoxStruct *DestBBox, const MvarBBoxStruct *SrcBBox);
void MvarBBoxOfDotProd(const MvarBBoxStruct *BBox1,
		       const MvarBBoxStruct *BBox2,
		       MvarBBoxStruct *DProdBBox);
void MvarBBoxOfDotProd2(const MvarBBoxStruct *BBox1,
			const MvarBBoxStruct *BBox2,
			MvarBBoxStruct *DProdBBox);
void MvarBBoxOfCrossProd(const MvarBBoxStruct *BBox1,
			 const MvarBBoxStruct *BBox2,
			 MvarBBoxStruct *DCrossBBox);
int _MvarIncrementMeshIndices(const MvarMVStruct *MV, int *Indices, int *Index);
int _MvarIncrementMeshOrderIndices(const MvarMVStruct *MV,
				   int *Indices,
				   int *Index);
int _MvarIncSkipMeshIndices1st(const MvarMVStruct *MV, int *Indices);
int _MvarIncSkipMeshIndices(const MvarMVStruct *MV,
			    int *Indices,
			    int Dir,
			    int *Index);
int _MvarIncBoundMeshIndices(const MvarMVStruct *MV,
			     int *Indices,
			     int *LowerBound,
			     int *UpperBound,
			     int *Index);
int MvarGetPointsMeshIndices(const MvarMVStruct *MV, int *Indices);
int MvarGetPointsPeriodicMeshIndices(const MvarMVStruct *MV, int *Indices);
int MvarMeshIndicesFromIndex(int Index, const MvarMVStruct *MV, int *Indices);

MvarMVStruct *MvarEditSingleMVPt(MvarMVStruct *MV,
				 CagdCtlPtStruct *CtlPt,
				 int *Indices,
				 CagdBType Write);
CagdBType MvarMVsSameSpace(const MvarMVStruct *MV1,
			   const MvarMVStruct *MV2,
			   CagdRType Eps);
CagdBType MvarMVsSame(const MvarMVStruct *MV1,
		      const MvarMVStruct *MV2,
		      CagdRType Eps);
MvarMVStruct *MvarPromoteMVToMV(const MvarMVStruct *MV, int Axis);
MvarMVStruct *MvarPromoteMVToMV2(const MvarMVStruct *MV,
				 int NewDim, 
				 int StartAxis);
MvarMVStruct *MvarCrvMakeCtlPtParam(const CagdCrvStruct *Crv,
				    int CtlPtIdx,
				    CagdRType Min,
				    CagdRType Max);
MvarMVStruct *MvarMVShiftAxes(const MvarMVStruct *MV, int Axis);
MvarMVStruct *MvarMVReverse(const MvarMVStruct *MV, int Axis1, int Axis2);
MvarMVStruct *MvarMergeMVMV(const MvarMVStruct *MV1,
			    const MvarMVStruct *MV2,
			    MvarMVDirType Dir,
			    CagdBType Discont);

CagdBType MvarBspMVIsOpenInDir(const MvarMVStruct *MV, MvarMVDirType Dir);
CagdBType MvarBspMVIsOpen(const MvarMVStruct *MV);
CagdBType MvarBspMVIsPeriodicInDir(const MvarMVStruct *MV, MvarMVDirType Dir);
CagdBType MvarBspMVIsPeriodic(const MvarMVStruct *MV);
CagdBType MvarBspMVInteriorKnots(const MvarMVStruct *MV, CagdRType *Knot);

MvarMVStruct *MvarMVMultiLinearMV(const IrtRType *Min,
				  const IrtRType *Max, 
				  int Dim);

void MvarDbg(const void *Obj);
void MvarETDbg(const MvarExprTreeStruct *ET);

/******************************************************************************
* Fitting and interpolation						      *
******************************************************************************/
CagdCrvStruct *MvarBspCrvInterpVecs(const MvarVecStruct *PtList,
				    int Order,
				    int CrvSize,
				    CagdParametrizationType ParamType,
				    CagdBType Periodic);
MvarVecStruct *MvarPtsSortAxis(MvarVecStruct *PtList, int Axis);
void MvarPointFromPointLine(const MvarVecStruct *Point,
			    const MvarVecStruct *Pl,
			    const MvarVecStruct *Vl,
			    MvarVecStruct *ClosestPoint);
IrtRType MvarDistPointLine(const MvarVecStruct *Point,
			   const MvarVecStruct *Pl,
			   const MvarVecStruct *Vl);
CagdRType MvarLineFitToPts(const MvarVecStruct *PtList,
			   MvarVecStruct *LineDir,
			   MvarVecStruct *LinePos);

/******************************************************************************
* Symbolic computation over multivariates.				      *
******************************************************************************/
MvarMVStruct *MvarMVAdd(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarMVSub(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarMVMult(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarMVInvert(const MvarMVStruct *MV);
MvarMVStruct *MvarMVScalarScale(const MvarMVStruct *MV, CagdRType Scale);
MvarMVStruct *MvarMVMultScalar(const MvarMVStruct *MV1,
			       const MvarMVStruct *MV2);
MvarMVStruct *MvarMVDotProd(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarMVVecDotProd(const MvarMVStruct *MV, const CagdRType *Vec);
MvarMVStruct *MvarMVCrossProd(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarMVCrossProd2D(const MvarMVStruct *MV1X,
				const MvarMVStruct *MV1Y,
				const MvarMVStruct *MV2X,
				const MvarMVStruct *MV2Y);
MvarMVStruct *MvarMVRtnlMult(const MvarMVStruct *MV1X,
			     const MvarMVStruct *MV1W,
			     const MvarMVStruct *MV2X,
			     const MvarMVStruct *MV2W,
			     CagdBType OperationAdd);
MvarMVStruct **MvarMVSplitScalar(const MvarMVStruct *MV);
MvarMVStruct *MvarMVMergeScalar(MvarMVStruct * const *ScalarMVs);

int MvarBspMultComputationMethod(int BspMultUsingInter);

MvarMVStruct *MvarBzrMVMult(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarBspMVMult(const MvarMVStruct *MV1, const MvarMVStruct *MV2);

MvarMVStruct *MvarMVDeterminant2(const MvarMVStruct *MV11,
				 const MvarMVStruct *MV12,
				 const MvarMVStruct *MV21,
				 const MvarMVStruct *MV22);
MvarMVStruct *MvarMVDeterminant3(const MvarMVStruct *MV11,
				 const MvarMVStruct *MV12,
				 const MvarMVStruct *MV13,
				 const MvarMVStruct *MV21,
				 const MvarMVStruct *MV22,
				 const MvarMVStruct *MV23,
				 const MvarMVStruct *MV31,
				 const MvarMVStruct *MV32,
				 const MvarMVStruct *MV33);
MvarMVStruct *MvarMVDeterminant4(const MvarMVStruct *MV11,
				 const MvarMVStruct *MV12,
				 const MvarMVStruct *MV13,
				 const MvarMVStruct *MV14,
				 const MvarMVStruct *MV21,
				 const MvarMVStruct *MV22,
				 const MvarMVStruct *MV23,
				 const MvarMVStruct *MV24,
				 const MvarMVStruct *MV31,
				 const MvarMVStruct *MV32,
				 const MvarMVStruct *MV33,
				 const MvarMVStruct *MV34,
				 const MvarMVStruct *MV41,
				 const MvarMVStruct *MV42,
				 const MvarMVStruct *MV43,
				 const MvarMVStruct *MV44);
MvarMVStruct *MvarMVDeterminant5(const MvarMVStruct *MV11,
				 const MvarMVStruct *MV12,
				 const MvarMVStruct *MV13,
				 const MvarMVStruct *MV14,
				 const MvarMVStruct *MV15,
				 const MvarMVStruct *MV21,
				 const MvarMVStruct *MV22,
				 const MvarMVStruct *MV23,
				 const MvarMVStruct *MV24,
				 const MvarMVStruct *MV25,
				 const MvarMVStruct *MV31,
				 const MvarMVStruct *MV32,
				 const MvarMVStruct *MV33,
				 const MvarMVStruct *MV34,
				 const MvarMVStruct *MV35,
				 const MvarMVStruct *MV41,
				 const MvarMVStruct *MV42,
				 const MvarMVStruct *MV43,
				 const MvarMVStruct *MV44,
				 const MvarMVStruct *MV45,
				 const MvarMVStruct *MV51,
				 const MvarMVStruct *MV52,
				 const MvarMVStruct *MV53,
				 const MvarMVStruct *MV54,
				 const MvarMVStruct *MV55);
MvarPtStruct *MvarMVsZeros(MvarMVStruct **MVs,
			   MvarConstraintType *Constraints,
			   int NumOfMVs,
			   CagdRType SubdivTol,
			   CagdRType NumericTol);
CagdBType MvarMVsZerosSameSpace(MvarMVStruct **MVs, int NumOfMVs);
int MvarMVsZerosNormalConeTest(int NormalConeTest);
int MvarMVsZerosGradPreconditioning(int GradPreconditioning);
int MvarMVsZerosDomainReduction(int DomainReduction);
int MvarMVsZerosParallelHyperPlaneTest(int ParallelHPlaneTest);
int MvarMVsZerosKantorovichTest(int KantorovichTest);
MvarMVsZerosSubdivCallBackFunc MvarMVsZerosSetCallBackFunc(
			   MvarMVsZerosSubdivCallBackFunc SubdivCallBackFunc);
void MvarMVsZerosVerifier(MvarMVStruct * const *MVs,
			  int NumOfZeroMVs,
			  MvarPtStruct *Sols,
			  CagdRType NumerEps);

int MvarMinSpanConeAvg(MvarVecStruct *MVVecs,
		       int VecsNormalized,
		       int NumOfVecs,
		       MvarNormalConeStruct *MVCone);
int MvarMinSpanCone(MvarVecStruct *MVVecs,
		    int VecsNormalized,
		    int NumOfVecs,
		    MvarNormalConeStruct *MVCone);
int MVHyperPlaneFromNPoints(MvarPlaneStruct *MVPlane,
			    MvarVecStruct * const *Vecs,
			    int n);
int MVHyperConeFromNPoints(MvarNormalConeStruct *MVCone,
			   MvarVecStruct * const *Vecs,
			   int n);
int MVHyperConeFromNPoints2(MvarNormalConeStruct *MVCone,
			    MvarVecStruct * const *Vecs,
			    int m);
int MVHyperConeFromNPoints3(MvarNormalConeStruct *MVCone,
			    MvarVecStruct * const *Vecs,
			    int m);
MvarNormalConeStruct *MVarMVNormalCone(const MvarMVStruct *MV);
MvarNormalConeStruct *MVarMVNormalCone2(const MvarMVStruct *MV,
					CagdRType * const *GradPoints,
					int TotalLength,
					int *MaxDevIndex);
MvarNormalConeStruct *MVarMVNormalConeMainAxis(const MvarMVStruct *MV,
					       MvarVecStruct **MainAxis);
MvarNormalConeStruct *MVarMVNormalConeMainAxis2(const MvarMVStruct *MV,
					        CagdRType * const *GradPoints,
						int TotalLength,
						MvarVecStruct **MainAxis);
MvarNormalConeStruct *MvarMVNormal2Cones(const MvarMVStruct *MV,
					 CagdRType ExpandingFactor,
					 int NumOfZeroMVs,
					 MvarNormalConeStruct *Cone1,
					 MvarNormalConeStruct *Cone2);
CagdBType MvarMVConesOverlap(MvarMVStruct * const *MVs, int NumOfZeroMVs);

/******************************************************************************
* Multivariate expression trees.					      *
******************************************************************************/

/* Conversions between expression trees and multivariates. */
MvarExprTreeStruct *MvarExprTreeFromCrv(const CagdCrvStruct *Crv,
					int NewDim,
					int StartAxis);
MvarExprTreeStruct *MvarExprTreeFromSrf(const CagdSrfStruct *Srf,
					int NewDim,
					int StartAxis);
MvarExprTreeStruct *MvarExprTreeFromMV(const MvarMVStruct *MV,
				       int NewDim,
				       int StartAxis);
MvarExprTreeStruct *MvarExprTreeFromMV2(const MvarMVStruct *MV);
MvarMVStruct *MvarExprTreeToMV(const MvarExprTreeStruct *ET);

/* Maintenance function on multivariate expression trees. */
MvarExprTreeStruct *MvarExprTreeLeafNew(CagdBType IsRef,
					MvarMVStruct *MV,
					int NewDim,
					int StartAxis,
					MvarNormalConeStruct *MVBCone,
					const MvarBBoxStruct *MVBBox);
MvarExprTreeStruct *MvarExprTreeIntrnlNew(MvarExprTreeNodeType NodeType,
					  MvarExprTreeStruct *Left,
					  MvarExprTreeStruct *Right,
					  const MvarBBoxStruct *MVBBox);
MvarExprTreeStruct *MvarExprTreeCopy(const MvarExprTreeStruct *ET,
				     CagdBType ThisNodeOnly,
				     CagdBType DuplicateMVs);
void MvarExprTreeFreeSlots(MvarExprTreeStruct *ET, CagdBType ThisNodeOnly);
void MvarExprTreeFree(MvarExprTreeStruct *ET, CagdBType ThisNodeOnly);
int MvarExprTreeSize(MvarExprTreeStruct *ET);
CagdBType MvarExprTreesSame(const MvarExprTreeStruct *ET1,
			    const MvarExprTreeStruct *ET2,
			    CagdRType Eps);
void MvarExprTreePrintInfo(MvarExprTreeStruct *ET,
			   CagdBType CommonExprIdx,
			   MvarExprTreePrintFuncType PrintFunc);

/* Constraint constructors using simple math operations. */
MvarExprTreeStruct *MvarExprTreeAdd(MvarExprTreeStruct *Left,
				    MvarExprTreeStruct *Right);
MvarExprTreeStruct *MvarExprTreeSub(MvarExprTreeStruct *Left,
				    MvarExprTreeStruct *Right);
MvarExprTreeStruct *MvarExprTreeMult(MvarExprTreeStruct *Left,
				     MvarExprTreeStruct *Right);
MvarExprTreeStruct *MvarExprTreeDotProd(MvarExprTreeStruct *Left,
					MvarExprTreeStruct *Right);
MvarExprTreeStruct *MvarExprTreeCrossProd(MvarExprTreeStruct *Left,
					  MvarExprTreeStruct *Right);
MvarExprTreeStruct *MvarExprTreeExp(MvarExprTreeStruct *Left);
MvarExprTreeStruct *MvarExprTreeLog(MvarExprTreeStruct *Left);
MvarExprTreeStruct *MvarExprTreeCos(MvarExprTreeStruct *Left);
MvarExprTreeStruct *MvarExprTreeSqrt(MvarExprTreeStruct *Left);
MvarExprTreeStruct *MvarExprTreeRecip(MvarExprTreeStruct *Left);

/* Operations on multivariate expression trees. */
int MvarExprTreeSubdivAtParam(const MvarExprTreeStruct *ET,
			      CagdRType t,
			      MvarMVDirType Dir,
			      MvarExprTreeStruct **Left,
			      MvarExprTreeStruct **Right);
const MvarBBoxStruct *MvarExprTreeBBox(MvarExprTreeStruct *ET);
MvarBBoxStruct *MvarExprTreeCompositionDerivBBox(MvarExprTreeStruct *ET,
									  MvarBBoxStruct *BBox);
int MvarExprTreeDomain(const MvarExprTreeStruct *ET,
		       CagdRType *Min,
		       CagdRType *Max,
		       int Axis);
int MvarExprTreesVerifyDomain(MvarExprTreeStruct *ET1,
			      MvarExprTreeStruct *ET2);
void MvarExprAuxDomainReset(MvarExprTreeStruct *ET);
int MvarExprTreeCnvrtBsp2BzrMV(MvarExprTreeStruct *ET,
			       MvarMinMaxType *Domain);
int MvarExprTreeCnvrtBzr2BspMV(MvarExprTreeStruct *ET);

int MvarExprTreeInteriorKnots(const MvarExprTreeStruct *ET, CagdRType *Knot);
CagdRType *MvarExprTreeEval(const MvarExprTreeStruct *ET,
			    CagdRType *Params);
CagdRType *MvarExprTreeGradient(const MvarExprTreeStruct *ET,
				CagdRType *Params,
				int *Dim);
MvarPlaneStruct *MvarExprTreeEvalTanPlane(const MvarExprTreeStruct *ET,
					  CagdRType *Params);

/* Zero finding over multivariate expression trees. */
int MvarExprTreeZerosUseCommonExpr(int UseCommonExpr);
int MvarExprTreeZerosCnvrtBezier2MVs(int Bezier2MVs);
MvarMVsZerosSubdivCallBackFunc MvarExprTreeZerosSetCallBackFunc(
			    MvarMVsZerosSubdivCallBackFunc SubdivCallBackFunc);
MvarPtStruct *MvarExprTreesZeros(MvarExprTreeStruct **MVETs,
				 MvarConstraintType *Constraints,
				 int NumOfMVETs,
				 CagdRType SubdivTol,
				 CagdRType NumericTol);
MvarPtStruct *MvarExprTreeEqnsZeros(MvarExprTreeEqnsStruct *Eqns,
				    CagdRType SubdivTol,
				    CagdRType NumericTol);
MvarNormalConeStruct *MVarExprTreeNormalCone(MvarExprTreeStruct *Eqn);
CagdBType MvarExprTreeConesOverlap(MvarExprTreeEqnsStruct *Eqns);

/******************************************************************************
* Intersections/contacts/antipodal points in crvs and srfs.		      *
******************************************************************************/
MvarPtStruct *MvarCrvCrvInter(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2,
			      CagdRType SubdivTol,
			      CagdRType NumericTol,
			      CagdBType UseExprTree);
MvarPtStruct *MvarSrfSrfInter(const CagdSrfStruct *Srf1,
			      const CagdSrfStruct *Srf2,
			      CagdRType SubdivTol,
			      CagdRType NumericTol,
			      CagdBType UseExprTree);
MvarPtStruct *MvarSrfSrfSrfInter(const CagdSrfStruct *Srf1,
				 const CagdSrfStruct *Srf2,
				 const CagdSrfStruct *Srf3,
				 CagdRType SubdivTol,
				 CagdRType NumericTol,
				 CagdBType UseExprTree);
MvarPtStruct *MvarSrfSrfContact(const CagdSrfStruct *Srf1,
				const CagdSrfStruct *Srf2,
				const CagdCrvStruct *Srf1Motion,
				const CagdCrvStruct *Srf1Scale,
				CagdRType SubdivTol,
				CagdRType NumericTol,
				CagdBType UseExprTree);

MvarMVStruct *MvarMVBivarFactorUMinusV(const MvarMVStruct *MV);
MvarMVStruct *MvarMV4VarFactorUMinusV(const MvarMVStruct *MV);
MvarMVStruct *MvarMV4VarFactorUMinusR(const MvarMVStruct *MV);
MvarPtStruct *MvarCrvAntipodalPoints(const CagdCrvStruct *Crv,
				     CagdRType SubdivTol,
				     CagdRType NumericTol);
MvarPtStruct *MvarSrfAntipodalPoints(const CagdSrfStruct *Srf,
				     CagdRType SubdivTol,
				     CagdRType NumericTol);
MvarPtStruct *MvarCrvSelfInterDiagFactor(const CagdCrvStruct *Crv,
					 CagdRType SubdivTol,
					 CagdRType NumericTol);
MvarPtStruct *MvarCrvSelfInterNrmlDev(const CagdCrvStruct *Crv,
				      CagdRType SubdivTol,
				      CagdRType NumericTol,
				      CagdRType MinNrmlDeviation);
void MvarBzrSelfInter4VarDecomp(const CagdSrfStruct *Srf,
				MvarMVStruct **U1MinusU3Factor,
				MvarMVStruct **U2MinusU4Factor);
MvarPtStruct *MvarBzrSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					    CagdRType SubdivTol,
					    CagdRType NumericTol);
MvarPtStruct *MvarBspSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					    CagdRType SubdivTol,
					    CagdRType NumericTol);
MvarPtStruct *MvarAdjacentSrfSrfInter(const CagdSrfStruct *Srf1,
				      const CagdSrfStruct *Srf2,
				      CagdSrfBndryType Srf1Bndry,
				      CagdRType SubdivTol,
				      CagdRType NumericTol);
MvarPtStruct *MvarSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					 CagdRType SubdivTol,
					 CagdRType NumericTol);
MvarPtStruct *MvarSrfSelfInterNrmlDev(const CagdSrfStruct *Srf,
				      CagdRType SubdivTol,
				      CagdRType NumericTol,
				      CagdRType MinNrmlDeviation);
MvarPtStruct *MvarCntctTangentialCrvCrvC1(const CagdCrvStruct *Crv1,
					  const CagdCrvStruct *Crv2,
					  CagdRType Epsilon);

/******************************************************************************
* Bisectors and Trisectors of multivariates.				      *
******************************************************************************/
MvarMVStruct *MvarMVsBisector(const MvarMVStruct *MV1, const MvarMVStruct *MV2);
MvarMVStruct *MvarCrvSrfBisector(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2);
MvarMVStruct *MvarSrfSrfBisector(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2);
VoidPtr MvarCrvSrfBisectorApprox(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2,
				 int OutputType,
				 CagdRType SubdivTol,
				 CagdRType NumericTol);
VoidPtr MvarSrfSrfBisectorApprox(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2,
				 int OutputType,
				 CagdRType SubdivTol,
				 CagdRType NumericTol);
CagdCrvStruct *MvarCrvCrvBisector2D(CagdCrvStruct *Crv1,
				    CagdCrvStruct *Crv2, 
				    CagdRType Step, 
				    CagdRType SubdivTol,
				    CagdRType NumericTol, 
				    CagdRType *BBoxMin,
				    CagdRType *BBoxMax,
				    CagdBType SupportPrms);

MvarMVStruct **MvarTrisector3DCreateMVs(VoidPtr FF1, 
					VoidPtr FF2,
					VoidPtr FF3,
					CagdRType *BBoxMin,
					CagdRType *BBoxMax,
					int *Eqns);
MvarPolyStruct *MvarTrisectorCrvs(VoidPtr FF1,
				  VoidPtr FF2,
				  VoidPtr FF3,
				  CagdRType Step, 
				  CagdRType SubdivTol,
				  CagdRType NumericTol,
				  CagdRType *BBoxMin,
				  CagdRType *BBoxMax);

/******************************************************************************
* Voronoi cell computation						      *
******************************************************************************/
struct IPObjectStruct *MvarComputeVoronoiCell(CagdCrvStruct *Crv);
CagdCrvStruct *MvarBsctTrimCrvPt(CagdCrvStruct *Crv, 
				 CagdRType *Pt, 
				 CagdRType Alpha,
				 CagdCrvStruct *BaseCrv);
void MvarUniFuncsComputeLowerEnvelope(CagdCrvStruct *InputCurves, 
				      CagdCrvStruct **LowerEnvelope);

/******************************************************************************
* Bitangents/Tritangents.    						      *
******************************************************************************/
MvarPtStruct *MvarMVBiTangents(const MvarMVStruct *MV1,
			       const MvarMVStruct *MV2,
			       int Orientation,
			       CagdRType SubdivTol,
			       CagdRType NumericTol);
MvarPtStruct *MvarMVBiTangents2(const MvarMVStruct *MV1,
				const MvarMVStruct *MV2,
				CagdRType SubdivTol,
				CagdRType NumericTol);
MvarPtStruct *MvarMVTriTangents(const MvarMVStruct *MV1,
				const MvarMVStruct *MV2,
				const MvarMVStruct *MV3,
				int Orientation,
				CagdRType SubdivTol,
				CagdRType NumericTol);
MvarPtStruct *MvarCircTanTo2Crvs(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2,
				 CagdRType Radius,
				 CagdRType Tol);

/******************************************************************************
 * Kernel and related analysis of curves.				      *
******************************************************************************/
MvarMVStruct *MVarCrvKernel(const CagdCrvStruct *Crv);
MvarMVStruct *MVarCrvGammaKernel(const CagdCrvStruct *Crv, CagdRType Gamma);
MvarMVStruct *MVarCrvGammaKernelSrf(const CagdCrvStruct *Crv,
				    CagdRType ExtentScale,
				    CagdRType GammaMax);
struct IPObjectStruct *MVarCrvKernelSilhouette(const CagdCrvStruct *Crv,
					       CagdRType Gamma,
					       CagdRType SubEps,
					       CagdRType NumEps);
struct IPObjectStruct *MVarCrvDiameter(const CagdCrvStruct *Crv,
				       CagdRType SubEps,
				       CagdRType NumEps);

/******************************************************************************
* Distances between manifolds as multivariates.				      *
******************************************************************************/
CagdRType *MvarDistSrfPoint(const CagdSrfStruct *Srf,
			    const CagdPType Pt,
			    CagdBType MinDist,
			    CagdRType SubdivTol,
			    CagdRType NumericTol);
MvarPtStruct *MvarLclDistSrfPoint(const CagdSrfStruct *Srf,
				  const CagdPType Pt,
				  CagdRType SubdivTol,
				  CagdRType NumericTol);
CagdRType *MvarDistSrfLine(const CagdSrfStruct *Srf,
			   const CagdPType LnPt,
			   const CagdVType LnDir,
			   CagdBType MinDist,
			   CagdRType SubdivTol,
			   CagdRType NumericTol);
MvarPtStruct *MvarLclDistSrfLine(const CagdSrfStruct *Srf,
				 const CagdPType LnPt,
				 const CagdVType LnDir,
				 CagdRType SubdivTol,
				 CagdRType NumericTol);
MvarMVStruct *MvarMVDistCrvSrf(const CagdCrvStruct *Crv1,
			       const CagdSrfStruct *Srf2,
			       int DistType);
MvarMVStruct *MvarMVDistSrfSrf(const CagdSrfStruct *Srf1,
			       const CagdSrfStruct *Srf2,
			       int DistType);

/******************************************************************************
* Metamorphosis of multivariates.					      *
******************************************************************************/
MvarMVStruct *MvarTwoMVsMorphing(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2,
				 CagdRType Blend);

/******************************************************************************
* Packing problems.							      *
******************************************************************************/
MvarPtStruct *Mvar3CircsInTriangles(const CagdPType Pts[3],
				    CagdRType SubdivTol,
				    CagdRType NumericTol);
MvarPtStruct *Mvar6CircsInTriangles(const CagdPType Pts[3],
				    CagdRType SubdivTol,
				    CagdRType NumericTol);

/******************************************************************************
* Light ray traps between n curves/surface.				      *
******************************************************************************/
MvarPtStruct *MvarComputeRayTraps(const CagdCrvStruct *Crvs,
				  int Orient,
				  CagdRType SubdivTol,
				  CagdRType NumerTol,
				  CagdBType UseExprTree);
MvarPtStruct *MvarComputeRayTraps3D(const CagdSrfStruct *Srfs,
				    int Orient,
				    CagdRType SubdivTol,
				    CagdRType NumerTol,
				    CagdBType UseExprTree);

/******************************************************************************
* Surface/Check surface accessibility analysis.				      *
******************************************************************************/
MvarPtStruct *MvarSrfAccessibility(const CagdSrfStruct *PosSrf,
				   const CagdSrfStruct *OrientSrf,
				   const CagdSrfStruct *CheckSrf,
				   const CagdRType *AccessDir,
				   CagdRType SubdivTol,
				   CagdRType NumerTol);
MvarPtStruct *MvarSrfSilhInflections(const CagdSrfStruct *Srf,
				     const CagdVType ViewDir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol);
MvarMVStruct **MvarFlecnodalCrvsCreateMVCnstrnts(const CagdSrfStruct *CSrf);
MvarPtStruct *MvarSrfFlecnodalCrvs(const CagdSrfStruct *Srf,
				   CagdRType SubdivTol,
				   CagdRType NumerTol);
MvarPolyStruct *MvarSrfFlecnodalCrvs2(const CagdSrfStruct *Srf, 
				      CagdRType Step, 
				      CagdRType SubdivTol, 
				      CagdRType NumerTol);
MvarPtStruct *MvarSrfFlecnodalPts(const CagdSrfStruct *Srf,
				  CagdRType SubdivTol,
				  CagdRType NumerTol);
MvarMVStruct *MVarProjNrmlPrmt2MVScl(const CagdSrfStruct *Srf,
				     const CagdSrfStruct *NrmlSrf,
				     const MvarMVStruct *MVScl);

/******************************************************************************
* Freeform curvature analysis.						      *
******************************************************************************/
MvarPtStruct *MvarSrfRadialCurvature(const CagdSrfStruct *Srf,
				     const CagdVType ViewDir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol);
MvarMVStruct *MvarCrvCrvtrByOneCtlPt(const CagdCrvStruct *Crv,
				     int CtlPtIdx,
				     CagdRType Min,
				     CagdRType Max);

/******************************************************************************
* Freeform topology analysis.						      *
******************************************************************************/
MvarPtStruct *MvarImplicitCrvExtreme(const CagdSrfStruct *Srf,
				     const CagdSrfDirType Dir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol);

/******************************************************************************
* Silhouette (and related curves') tracing.				      *
******************************************************************************/
struct IPObjectStruct *MvarSrfSilhouette(const CagdSrfStruct *Srf,
					 const CagdVType VDir,
					 CagdRType Step,
					 CagdRType SubdivTol,
					 CagdRType NumericTol,
					 CagdBType Euclidean);

/******************************************************************************
* Routines to handle the computation of 2D skeletons and minimum spanning     *
* circles.								      *
******************************************************************************/
CagdRType MvarSkel2DSetEpsilon(CagdRType NewEps);
CagdRType MvarSkel2DSetFineNess(CagdRType NewFineNess);
CagdRType MvarSkel2DSetMZeroTols(CagdRType SubdivTol, CagdRType NumerTol);
CagdRType MvarSkel2DSetOuterExtent(CagdRType NewOutExtent);
MvarSkel2DInter3PrimsStruct *MvarSkel2DInter3Prims(MvarSkel2DPrimStruct *Prim1,
						   MvarSkel2DPrimStruct *Prim2,
						   MvarSkel2DPrimStruct *Prim3);
void MvarSkel2DInter3PrimsFree(MvarSkel2DInter3PrimsStruct *SK2DInt);
void MvarSkel2DInter3PrimsFreeList(MvarSkel2DInter3PrimsStruct *SK2DIntList);

int MvarMSCircOfTwoCurves(const CagdCrvStruct *Crv1,
			  const CagdCrvStruct *Crv2,
			  CagdRType Center[2],
			  CagdRType *Radius,
			  CagdRType SubdivTol,
			  CagdRType NumerTol);
int MvarMSCircOfThreeCurves(const CagdCrvStruct *Crv1,
			    const CagdCrvStruct *Crv2,
			    const CagdCrvStruct *Crv3,
			    CagdRType Center[2],
			    CagdRType *Radius,
			    CagdRType SubdivTol,
			    CagdRType NumerTol);
int MVarIsCrvInsideCirc(const CagdCrvStruct *Crv,
			CagdRType Center[2],
			CagdRType Radius,
			CagdRType Tolerance);
int MvarMinSpanCirc(struct IPObjectStruct *Objs,
		    CagdRType *Center,
		    CagdRType *Radius,
		    CagdRType SubdivTol,
		    CagdRType NumerTol);
MvarPtStruct *MvarTanHyperSpheresofNManifolds(MvarMVStruct **MVs,
					      int NumOfMVs,
					      CagdRType SubdivTol,
					      CagdRType NumerTol,
					      CagdBType UseExprTree);

/******************************************************************************
* Routines to handle offsets.						      *
******************************************************************************/
CagdCrvStruct *MvarCrvTrimGlblOffsetSelfInter(CagdCrvStruct *Crv,
					      const CagdCrvStruct *OffCrv,
					      CagdRType TrimAmount,
					      CagdRType SubdivTol,
					      CagdRType NumericTol);
struct IPObjectStruct *MvarSrfTrimGlblOffsetSelfInter(
						   CagdSrfStruct *Srf,
						   const CagdSrfStruct *OffSrf,
						   CagdRType TrimAmount,
						   int Validate,
						   int Euclidean,
						   CagdRType SubdivTol,
						   CagdRType NumerTol,
						   CagdBType NumerImp);
struct IPObjectStruct *MvarSrfTrimGlblOffsetSelfInterNI(
					 	struct IPPolygonStruct *Plls, 
						const CagdSrfStruct *OffSrf, 
						CagdRType SubdivTol, 
						CagdRType NumerTol,
						int Euclidean,
						CagdRType SameUVTol);

/******************************************************************************
* Routines to handle Hausdorff/minimal/maximal distances between freeforms.   *
******************************************************************************/

CagdRType MvarDistPointCrvC1(CagdPType P,
			     const CagdCrvStruct *Crv,
			     MvarHFDistParamStruct *Param,
			     CagdBType MinDist,
			     CagdRType Epsilon);
CagdRType MvarHFExtremeLclDistPointCrvC1(CagdPType P,
					 const CagdCrvStruct *Crv1,
					 const CagdCrvStruct *Crv2,
					 MvarHFDistParamStruct *Param2,
					 CagdRType Epsilon);
MvarPtStruct *MvarHFDistAntipodalCrvCrvC1(const CagdCrvStruct *Crv1,
					  const CagdCrvStruct *Crv2,
					  CagdRType Epsilon);
MvarHFDistPairParamStruct *MvarHFDistInterBisectCrvCrvC1(
						     const CagdCrvStruct *Crv1,
						     const CagdCrvStruct *Crv2,
						     CagdRType Epsilon);
CagdRType MvarHFDistFromCrvToCrvC1(const CagdCrvStruct *Crv1,
				   const CagdCrvStruct *Crv2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2,
				   CagdRType Epsilon);
CagdRType MvarHFDistCrvCrvC1(const CagdCrvStruct *Crv1,
			     const CagdCrvStruct *Crv2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2,
			     CagdRType Epsilon);

CagdRType MvarHFDistPointSrfC1(const CagdPType P,
			       const CagdSrfStruct *Srf,
			       MvarHFDistParamStruct *Param,
			       CagdBType MinDist);
CagdRType MvarHFExtremeLclDistPointSrfC1(const CagdPType P,
					 const CagdSrfStruct *Srf1,
					 const CagdSrfStruct *Srf2,
					 MvarHFDistParamStruct *Param2);
MvarPtStruct *MvarHFDistAntipodalCrvSrfC1(const CagdSrfStruct *Srf1,
					  const CagdCrvStruct *Crv2);
CagdRType MvarHFDistFromCrvToSrfC1(const CagdCrvStruct *Crv1,
				   const CagdSrfStruct *Srf2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2);
CagdRType MvarHFDistFromSrfToCrvC1(const CagdSrfStruct *Srf1,
				   const CagdCrvStruct *Crv2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2);
CagdRType MvarHFDistSrfCrvC1(const CagdSrfStruct *Srf1,
			     const CagdCrvStruct *Crv2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2);
MvarPtStruct *MvarHFDistAntipodalSrfSrfC1(const CagdSrfStruct *Srf1,
					  const CagdSrfStruct *Srf2);
MvarHFDistPairParamStruct *MvarHFDistBisectSrfSrfC1(const CagdSrfStruct *Srf1,
						    const CagdSrfStruct *Srf2);
MvarHFDistPairParamStruct *MvarHFDistInterBisectSrfSrfC1(
						    const CagdSrfStruct *Srf1,
						    const CagdSrfStruct *Srf2);
CagdRType MvarHFDistFromSrfToSrfC1(const CagdSrfStruct *Srf1,
				   const CagdSrfStruct *Srf2,
				   MvarHFDistParamStruct *Param1,
				   MvarHFDistParamStruct *Param2);
CagdRType MvarHFDistSrfSrfC1(const CagdSrfStruct *Srf1,
			     const CagdSrfStruct *Srf2,
			     MvarHFDistParamStruct *Param1,
			     MvarHFDistParamStruct *Param2);

MvarPtStruct *MvarCrvCrvMinimalDist(const CagdCrvStruct *Crv1,
				    const CagdCrvStruct *Crv2,
				    CagdRType *MinDist,
				    CagdBType ComputeAntipodals,
				    CagdRType Eps);
MvarPtStruct *MvarCrvSrfMinimalDist(const CagdSrfStruct *Srf1,
				    const CagdCrvStruct *Crv2,
				    CagdRType *MinDist);
MvarPtStruct *MvarSrfSrfMinimalDist(const CagdSrfStruct *Srf1,
				    const CagdSrfStruct *Srf2,
				    CagdRType *MinDist);
CagdRType MvarCrvMaxXYOriginDistance(const CagdCrvStruct *Crv,
				     CagdRType Epsilon,
				     CagdRType *Param);
CagdRType MvarSrfLineOneSidedMaxDist(const CagdSrfStruct *Srf,
				     const CagdUVType UV1,
				     const CagdUVType UV2,
				     CagdSrfDirType ClosedDir,
				     CagdRType Epsilon);

/******************************************************************************
* Routines to handle curve on surface projections.			      *
******************************************************************************/
MvarPolyStruct *MvarMVOrthoCrvProjOnSrf(const CagdCrvStruct *Crv,
					const CagdSrfStruct *Srf,
					CagdRType Tol);

/******************************************************************************
* Routines to handle basis function conversions.			      *
******************************************************************************/
MvarMVStruct *MvarCnvrtPeriodic2FloatMV(const MvarMVStruct *MV);
MvarMVStruct *MvarCnvrtFloat2OpenMV(const MvarMVStruct *MV);

/******************************************************************************
* Routines to compute surface-surface intersections.			      *
******************************************************************************/
MvarPolyStruct *MvarSrfSrfInter2(const CagdSrfStruct *Srf1, 
				 const CagdSrfStruct *Srf2,
				 CagdRType Step,
				 CagdRType SubdivTol,
				 CagdRType NumericTol);
MvarPolyStruct *MvarMVUnivarInter(MvarMVStruct * const *MVs,
				  CagdRType Step,
				  CagdRType SubdivTol,
				  CagdRType NumericTol);
int MvarMVUnivarInterMergeSingularPts(int MergeSingularPts); 

/******************************************************************************
* Routines to multivariate algebraic symbolic manipulation as strings.        *
******************************************************************************/
void *MvarZrAlgCreate();
void MvarZrAlgDelete(void *MVZrAlg);
int MvarZrAlgAssignExpr(void *MVZrAlg, const char *Name, const char *Expr);
int MvarZrAlgAssignNumVar(void *MVZrAlg, const char *Name, CagdRType Val);
int MvarZrAlgAssignMVVar(void *MVZrAlg,
			 const char *Name,
			 CagdRType DmnMin,
			 CagdRType DmnMax,
			 const MvarMVStruct *MV);
int MvarZrAlgGenMVCode(void *MVZrAlg, const char *Expr, FILE *f);

/******************************************************************************
* Routines to manipulate trivariate functions.				      *
******************************************************************************/
TrivTVStruct *MvarTrivarBoolOne(const CagdSrfStruct *Srf);
TrivTVStruct *MvarTrivarBoolSum(const CagdSrfStruct *Srf1,
				const CagdSrfStruct *Srf2,
				const CagdSrfStruct *Srf3,
				const CagdSrfStruct *Srf4,
				const CagdSrfStruct *Srf5,
				const CagdSrfStruct *Srf6);
  
/******************************************************************************
* Error handling.							      *
******************************************************************************/
MvarSetErrorFuncType MvarSetFatalErrorFunc(MvarSetErrorFuncType ErrorFunc);
void MvarFatalError(MvarFatalErrorType ErrID);
const char *MvarDescribeError(MvarFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* MVAR_LIB_H */
