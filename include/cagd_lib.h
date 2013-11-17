/******************************************************************************
* Cagd_lib.h - header file for the CAGD library.			      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#ifndef CAGD_LIB_H
#define CAGD_LIB_H

#include <stdio.h>
#include "irit_sm.h"
#include "miscattr.h"
#include "misc_lib.h"

typedef int CagdBType;
typedef IrtRType CagdRType;
typedef IrtMinMaxType CagdMinMaxType;
typedef IrtUVType CagdUVType;
typedef IrtPtType CagdPType;
typedef IrtVecType CagdVType;
typedef IrtHmgnMatType CagdMType;
typedef IrtLnType CagdLType;

typedef enum {
    CAGD_ERR_180_ARC = 1000,
    CAGD_ERR_AFD_NO_SUPPORT,
    CAGD_ERR_ALLOC_ERR,
    CAGD_ERR_BSPLINE_NO_SUPPORT,
    CAGD_ERR_BZR_CRV_EXPECT,
    CAGD_ERR_BZR_SRF_EXPECT,
    CAGD_ERR_BSP_CRV_EXPECT,
    CAGD_ERR_BSP_SRF_EXPECT,
    CAGD_ERR_CRV_FAIL_CMPT,
    CAGD_ERR_CRVS_INCOMPATIBLE,
    CAGD_ERR_CUBIC_EXPECTED,
    CAGD_ERR_DEGEN_ALPHA,
    CAGD_ERR_DIR_NOT_CONST_UV,
    CAGD_ERR_DIR_NOT_VALID,
    CAGD_ERR_INDEX_NOT_IN_MESH,
    CAGD_ERR_KNOT_NOT_ORDERED,
    CAGD_ERR_LIN_NO_SUPPORT,
    CAGD_ERR_NO_CROSS_PROD,
    CAGD_ERR_NOT_ENOUGH_MEM,
    CAGD_ERR_NOT_IMPLEMENTED,
    CAGD_ERR_NUM_KNOT_MISMATCH,
    CAGD_ERR_OUT_OF_RANGE,
    CAGD_ERR_PARSER_STACK_OV,
    CAGD_ERR_POWER_NO_SUPPORT,
    CAGD_ERR_PT_OR_LEN_MISMATCH,
    CAGD_ERR_POLYNOMIAL_EXPECTED,
    CAGD_ERR_RATIONAL_EXPECTED,
    CAGD_ERR_SCALAR_EXPECTED,
    CAGD_ERR_SRF_FAIL_CMPT,
    CAGD_ERR_SRFS_INCOMPATIBLE,
    CAGD_ERR_UNDEF_CRV,
    CAGD_ERR_UNDEF_SRF,
    CAGD_ERR_UNDEF_GEOM,
    CAGD_ERR_UNSUPPORT_PT,
    CAGD_ERR_T_NOT_IN_CRV,
    CAGD_ERR_U_NOT_IN_SRF,
    CAGD_ERR_V_NOT_IN_SRF,
    CAGD_ERR_WRONG_DOMAIN,
    CAGD_ERR_W_NOT_SAME,
    CAGD_ERR_W_ZERO,
    CAGD_ERR_WRONG_CRV,
    CAGD_ERR_WRONG_INDEX,
    CAGD_ERR_WRONG_ORDER,
    CAGD_ERR_WRONG_SRF,
    CAGD_ERR_WRONG_PT_TYPE,
    CAGD_ERR_CANNOT_COMP_VEC_FIELD,
    CAGD_ERR_CANNOT_COMP_NORMAL,
    CAGD_ERR_REPARAM_NOT_MONOTONE,
    CAGD_ERR_RATIONAL_NO_SUPPORT,
    CAGD_ERR_NO_SOLUTION,
    CAGD_ERR_TOO_COMPLEX,
    CAGD_ERR_REF_LESS_ORIG,
    CAGD_ERR_ONLY_2D_OR_3D,
    CAGD_ERR_ONLY_1D_TO_3D,
    CAGD_ERR_ONLY_2D,
    CAGD_ERR_DOMAIN_TOO_SMALL,
    CAGD_ERR_PERIODIC_EXPECTED,
    CAGD_ERR_PERIODIC_NO_SUPPORT,
    CAGD_ERR_OPEN_EC_EXPECTED,
    CAGD_ERR_POLYGON_EXPECTED,
    CAGD_ERR_POLYSTRIP_EXPECTED,
    CAGD_ERR_SWEEP_AXIS_TOO_COMPLEX,
    CAGD_ERR_INVALID_CONIC_COEF,
    CAGD_ERR_HYPERBOLA_NO_SUPPORT,
    CAGD_ERR_WRONG_DERIV_ORDER,
    CAGD_ERR_NO_TOL_TEST_FUNC,
    CAGD_ERR_NO_KV_FOUND,
    CAGD_ERR_WRONG_SIZE,
    CAGD_ERR_INVALID_CRV,
    CAGD_ERR_INVALID_SRF,
    CAGD_ERR_C0_KV_DETECTED,

    CAGD_ERR_UNDEFINE_ERR
} CagdFatalErrorType;

/* The following type should match MvarPointType for the shared domain.      */
typedef enum {		/* Type of control point. The P-types are rationals. */
    CAGD_PT_NONE = 0,
    CAGD_PT_BASE = 1100,			  /* Must be an even number. */
    CAGD_PT_E1_TYPE = 1100,
    CAGD_PT_P1_TYPE,
    CAGD_PT_E2_TYPE,
    CAGD_PT_P2_TYPE,
    CAGD_PT_E3_TYPE,
    CAGD_PT_P3_TYPE,
    CAGD_PT_E4_TYPE,
    CAGD_PT_P4_TYPE,
    CAGD_PT_E5_TYPE,
    CAGD_PT_P5_TYPE,
    CAGD_PT_E6_TYPE,
    CAGD_PT_P6_TYPE,
    CAGD_PT_E7_TYPE,
    CAGD_PT_P7_TYPE,
    CAGD_PT_E8_TYPE,
    CAGD_PT_P8_TYPE,
    CAGD_PT_E9_TYPE,
    CAGD_PT_P9_TYPE,
    CAGD_PT_E10_TYPE,
    CAGD_PT_P10_TYPE,
    CAGD_PT_E11_TYPE,
    CAGD_PT_P11_TYPE,
    CAGD_PT_E12_TYPE,
    CAGD_PT_P12_TYPE,
    CAGD_PT_E13_TYPE,
    CAGD_PT_P13_TYPE,
    CAGD_PT_E14_TYPE,
    CAGD_PT_P14_TYPE,
    CAGD_PT_E15_TYPE,
    CAGD_PT_P15_TYPE,
    CAGD_PT_E16_TYPE,
    CAGD_PT_P16_TYPE,
    CAGD_PT_E17_TYPE,
    CAGD_PT_P17_TYPE,
    CAGD_PT_E18_TYPE,
    CAGD_PT_P18_TYPE,
    CAGD_PT_MAX_SIZE_TYPE	     /* See also CAGD_MAX_* constants below. */
} CagdPointType;

#define CAGD_MAX_PT_SIZE		19    /* Rational P18 has 19 coords. */
#define CAGD_MAX_PT_COORD		18		       /* Without w. */
#define CAGD_MAX_E_POINT		CAGD_PT_E18_TYPE
#define CAGD_MAX_P_POINT		CAGD_PT_P18_TYPE

typedef enum {
    CAGD_POLY_APPROX_ERR_CENTER = 1,
    CAGD_POLY_APPROX_ERR_SAMPLES_MAX,
    CAGD_POLY_APPROX_ERR_SAMPLES_AVG
} CagdPolyErrEstimateType;

typedef enum {
    CAGD_END_COND_GENERAL = 0,
    CAGD_END_COND_OPEN = 1,
    CAGD_END_COND_FLOAT,
    CAGD_END_COND_PERIODIC
} CagdEndConditionType;

typedef enum {
    CAGD_PRIM_CAPS_NONE = 0,
    CAGD_PRIM_CAPS_BOTTOM = 1,
    CAGD_PRIM_CAPS_TOP = 2,
    CAGD_PRIM_CAPS_BOTH = 3
} CagdPrimCapsType;

typedef enum {
    CAGD_CONST_X_SYMMETRY = 1,
    CAGD_CONST_Y_SYMMETRY = 2,
    CAGD_CONST_Z_SYMMETRY = 4,
    CAGD_CONST_C_SYMMETRY = 32,
    CAGD_CONST_X_AREA = 64,
    CAGD_CONST_Y_AREA = 128
} CagdConstraintType;

typedef enum {
    CAGD_GEOM_CONST,
    CAGD_GEOM_LINEAR,
    CAGD_GEOM_CIRCULAR,
    CAGD_GEOM_CONIC_SEC,
    CAGD_GEOM_PLANAR,
    CAGD_GEOM_SPHERICAL,
    CAGD_GEOM_TORUS,
    CAGD_GEOM_CYLINDRICAL,
    CAGD_GEOM_CONICAL,
    CAGD_GEOM_SRF_OF_REV,
    CAGD_GEOM_BILINEAR,
    CAGD_GEOM_BOOL_SUM,
    CAGD_GEOM_EXTRUSION,
    CAGD_GEOM_RULED_SRF,
    CAGD_GEOM_DEVELOP_SRF,
    CAGD_GEOM_SWEEP_SRF
} CagdIsGeometryType;

typedef enum {
    CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE,
    CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST,
    CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER2ND,
    CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER3RD
} CagdBspBasisFuncMultEvalType;

#define CAGD_MALLOC_STRUCT_ONCE     /* Faster allocation of CAGD structures. */

#define CAGD_IS_RATIONAL_PT(PType)  ((int) ((PType) & 0x01))
#define CAGD_NUM_OF_PT_COORD(PType) ((((int) ((PType) - CAGD_PT_BASE)) >> 1) + 1)
#define CAGD_MAKE_PT_TYPE(IsRational, NumCoords) \
				    ((CagdPointType) (CAGD_PT_BASE + \
				         ((((IsRational) ? -1 : -2) \
						       + ((NumCoords) << 1)))))

#define CAGD_IS_RATIONAL_CRV(Crv)	CAGD_IS_RATIONAL_PT((Crv) -> PType)
#define CAGD_IS_RATIONAL_SRF(Srf)	CAGD_IS_RATIONAL_PT((Srf) -> PType)

/* Bezier curves may be evaluated using a predefined cache. The cache must   */
/* be of size (FINENESS) which is power of 2 up to the maximum order below.  */
/* See BzrCrvSetCache routine below.					     */
#ifdef CAGD_LARGE_BEZIER_CACHE
#define CAGD_MAX_BEZIER_CACHE_ORDER	99    /* See cbzr_tbl.c in cagd_lib. */
#else
#define CAGD_MAX_BEZIER_CACHE_ORDER	28    /* See cbzr_tbl.c in cagd_lib. */
#endif /* CAGD_LARGE_BEZIER_CACHE */

#define CAGD_MAX_BEZIER_CACHE_FINENESS	1024

#define CAGD_MAX_BEZIER_CACHE_ORDER2	15    /* See cbzr2tbl.c in cagd_lib. */

/* If a curve or a surface is periodic, their control polygon/mesh should    */
/* warp up. Length does hold the real allocated length but the virtual       */
/* periodic length is a little larger. Note allocated KV's are larger.       */
#define CAGD_CRV_PT_LST_LEN(Crv) ((Crv) -> Length + \
				  ((Crv) -> Periodic ? (Crv) -> Order - 1 : 0))
#define CAGD_SRF_UPT_LST_LEN(Srf)	((Srf) -> ULength + \
					 ((Srf) -> UPeriodic ? \
					      (Srf) -> UOrder - 1 : 0))
#define CAGD_SRF_VPT_LST_LEN(Srf)	((Srf) -> VLength + \
					 ((Srf) -> VPeriodic ? \
					      (Srf) -> VOrder - 1 : 0))
#define CAGD_IS_PERIODIC_CRV(Crv)	((Crv) -> Periodic)
#define CAGD_IS_UPERIODIC_SRF(Srf)	((Srf) -> UPeriodic)
#define CAGD_IS_VPERIODIC_SRF(Srf)	((Srf) -> VPeriodic)
#define CAGD_IS_PERIODIC_SRF(Srf)	(CAGD_IS_UPERIODIC_SRF(Srf) || \
					 CAGD_IS_VPERIODIC_SRF(Srf))


#define CAGD_RESET_BBOX(BBox) { \
    BBox -> Min[0] = BBox -> Min[1] = BBox -> Min[2] = IRIT_INFNTY; \
    BBox -> Max[0] = BBox -> Max[1] = BBox -> Max[2] = -IRIT_INFNTY; }

#define CAGD_DEL_GEOM_TYPE(Obj) \
			    AttrFreeOneAttribute(&(Obj) -> Attr, "GeomType")
#define CAGD_SET_GEOM_TYPE(Obj, Geom) \
			    AttrSetIntAttrib(&(Obj) -> Attr, "GeomType", Geom)
#define CAGD_PROPAGATE_ATTR(NewObj, Obj) { \
		  IP_ATTR_FREE_ATTRS((NewObj) -> Attr); \
                  if ((Obj) -> Attr != NULL) { \
		      (NewObj) -> Attr = AttrCopyAttributes((Obj) -> Attr); } }

#define CAGD_QUERY_VALUE	-1030603010

#define CAGD_EPS_ROUND_KNOT	  1e-12

#define CAGD_MAX_DOMAIN_EPS	(IRIT_UEPS * 10)
#define CAGD_VALIDATE_MIN_MAX_DOMAIN(t, TMin, TMax) { \
    if (t >= TMax - IRIT_MAX(t * CAGD_MAX_DOMAIN_EPS, CAGD_MAX_DOMAIN_EPS)) \
	t -= IRIT_MAX(t * CAGD_MAX_DOMAIN_EPS, CAGD_MAX_DOMAIN_EPS); \
    if (t < TMin) \
	t = TMin; }

typedef enum {
    CAGD_UNDEF_TYPE = 1200,
    CAGD_CBEZIER_TYPE,
    CAGD_CBSPLINE_TYPE,
    CAGD_CPOWER_TYPE,
    CAGD_SBEZIER_TYPE,
    CAGD_SBSPLINE_TYPE,
    CAGD_SPOWER_TYPE
} CagdGeomType;

typedef enum {
    CAGD_NO_DIR = 1300,
    CAGD_CONST_U_DIR,
    CAGD_CONST_V_DIR,
    CAGD_BOTH_DIR
} CagdSrfDirType;

typedef enum {
    CAGD_NO_BNDRY = 1400,
    CAGD_U_MIN_BNDRY,
    CAGD_U_MAX_BNDRY,
    CAGD_V_MIN_BNDRY,
    CAGD_V_MAX_BNDRY
} CagdSrfBndryType;

#define CAGD_OTHER_DIR(Dir) ((Dir) == CAGD_CONST_U_DIR ? CAGD_CONST_V_DIR \
						       : CAGD_CONST_U_DIR)

typedef enum {
    CAGD_REG_POLY_PER_LIN = 1400,
    CAGD_ONE_POLY_PER_LIN,
    CAGD_ONE_POLY_PER_COLIN
} CagdLin2PolyType;

typedef enum {
    CAGD_GENERAL_PARAM = 1500,
    CAGD_UNIFORM_PARAM,
    CAGD_CENTRIPETAL_PARAM,
    CAGD_CHORD_LEN_PARAM,
    CAGD_NIELSON_FOLEY_PARAM,
    CAGD_KV_NODAL_PARAM,
} CagdParametrizationType;

typedef enum {
    CAGD_POLYGON_TYPE_TRIANGLE,
    CAGD_POLYGON_TYPE_RECTANGLE,
    CAGD_POLYGON_TYPE_POLYSTRIP
} CagdPolygonType;

typedef enum {
    CAGD_PDM_FITTING,
    CAGD_SDM_FITTING
} CagdBspFittingType;

typedef struct CagdGenericStruct {
    struct CagdGenericStruct *Pnext;
    struct IPAttributeStruct *Attr;
} CagdGenericStruct;

typedef struct CagdUVStruct {
    struct CagdUVStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdUVType UV;
} CagdUVStruct;

typedef struct CagdPtStruct {
    struct CagdPtStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdPType Pt;
} CagdPtStruct;

typedef struct CagdSrfPtStruct {
    struct CagdSrfPtStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdUVType Uv;
    CagdPType Pt;
    CagdVType Nrml;
} CagdSrfPtStruct;

typedef struct CagdSrfAdapRectStruct {
    struct CagdSrfAdapRectStruct *Pnext;
    int UIndexBase;
    int UIndexSize;
    int VIndexBase;
    int VIndexSize;
    VoidPtr AuxSrfData;
    CagdRType Err;
} CagdSrfAdapRectStruct;

typedef struct CagdCtlPtStruct {
    struct CagdCtlPtStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdRType Coords[CAGD_MAX_PT_SIZE];
    CagdPointType PtType;
    int align8;
} CagdCtlPtStruct;

typedef struct CagdVecStruct {
    struct CagdVecStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdVType Vec;
} CagdVecStruct;

typedef struct CagdPlaneStruct {
    struct CagdPlaneStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdRType Plane[4];
} CagdPlaneStruct;

typedef struct CagdBBoxStruct {
    struct CagdBBoxStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdPType Min;
    CagdPType Max;
} CagdBBoxStruct;

typedef struct CagdCrvStruct {
    struct CagdCrvStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdGeomType GType;
    CagdPointType PType;
    int Length;            /* Number of control points (== order in Bezier). */
    int Order;	    /* Order of curve (only for Bspline, ignored in Bezier). */
    CagdBType Periodic;			   /* Valid only for Bspline curves. */
    CagdRType *Points[CAGD_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    CagdRType *KnotVector;
} CagdCrvStruct;

typedef struct CagdSrfStruct {
    struct CagdSrfStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdGeomType GType;
    CagdPointType PType;
    int ULength, VLength;	 /* Mesh size in the tensor product surface. */
    int UOrder, VOrder;   /* Order in tensor product surface (Bspline only). */
    CagdBType UPeriodic, VPeriodic;      /* Valid only for Bspline surfaces. */
    CagdRType *Points[CAGD_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    CagdRType *UKnotVector, *VKnotVector;

    VoidPtr PAux;                         /* Used internally - do not touch. */
} CagdSrfStruct;

typedef struct CagdPolygonStruct {
    struct CagdPolygonStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdPolygonType PolyType;
    union {
	struct {
	    CagdPType Pt;	 /* Polygon is either triangle or rectangle. */
	    CagdVType Nrml;
	    CagdUVType UV;
	} Polygon[4];
	struct {
	    /* Polygonal strip can have arbitrary # of polygons. */
	    CagdPType FirstPt[2];	      /* Base line - the first edge. */
	    CagdVType FirstNrml[2];
	    CagdUVType FirstUV[2];
	    CagdPType *StripPt;		       /* Arrays of size NumOfPolys. */
	    CagdVType *StripNrml;
	    CagdUVType *StripUV;
	    int NumOfPolys;
	} PolyStrip;
    } U;
} CagdPolygonStruct;

typedef struct {
    CagdPType Pt;
} CagdPolylnStruct;

typedef struct CagdPolylineStruct {
    struct CagdPolylineStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdPolylnStruct *Polyline; /* Polyline length is defined using Length. */
    int Length;
} CagdPolylineStruct;

typedef struct BspKnotAlphaCoeffStruct {
    int Order, Length, RefLength, Periodic;   /* Dimensions of alpha matrix. */
    CagdRType *Matrix;
    CagdRType *MatrixTransp;
    CagdRType **Rows;		     /* A column of pointers to Matrix rows. */
    CagdRType **RowsTransp;	     /* A row of pointers to Matrix columns. */
    int *ColIndex;	 /* A row of indices of first non zero value in col. */
    int *ColLength;	      /* A row of lengths of non zero values in col. */
    CagdRType *_CacheKVT;	    /* To compare input/output kvs in cache. */
    CagdRType *_CacheKVt;
} BspKnotAlphaCoeffStruct;

typedef struct CagdBlsmAlphaCoeffStruct {
    int Order, Length,
	NewOrder, NewLength, Periodic;/* Dimensions of blossom alpha matrix. */
    CagdRType *Matrix;
    CagdRType **Rows;		     /* A column of pointers to Matrix rows. */
    int *ColIndex;	 /* A row of indices of first non zero value in col. */
    int *ColLength;	      /* A row of lengths of non zero values in col. */
    CagdRType *KV;	     /* Knot sequences before and after the blossom. */
    CagdRType *NewKV;
} CagdBlsmAlphaCoeffStruct;

typedef struct CagdBspBasisFuncEvalStruct {
    int FirstBasisFuncIndex;
    CagdRType *BasisFuncsVals;
} CagdBspBasisFuncEvalStruct;

typedef void (*CagdSetErrorFuncType)(CagdFatalErrorType);
typedef void (*CagdPrintfFuncType)(char *Line);
typedef int (*CagdCompFuncType)(VoidPtr P1, VoidPtr P2);
typedef CagdCrvStruct *(*CagdCrvFuncType)(CagdCrvStruct *Crv, CagdRType R);
typedef CagdRType (*CagdMatchNormFuncType)(const CagdVType T1,
					   const CagdVType T2,
					   const CagdVType P1,
					   const CagdVType P2);
typedef CagdRType (*CagdSrfErrorFuncType)(const CagdSrfStruct *Srf);
typedef int (*CagdSrfAdapAuxDataFuncType)(const CagdSrfStruct *Srf,
					  VoidPtr AuxSrfData,
					  CagdRType t,
					  CagdSrfDirType Dir,
					  CagdSrfStruct *Srf1,
					  VoidPtr *AuxSrf1Data,
					  CagdSrfStruct *Srf2,
					  VoidPtr *AuxSrf2Data);
typedef CagdPolygonStruct *(*CagdSrfAdapPolyGenFuncType)(
					  const CagdSrfStruct *Srf,
					  CagdSrfPtStruct *SrfPtList,
					  const CagdSrfAdapRectStruct *Rect);

typedef CagdPolygonStruct *(*CagdSrfMakeTriFuncType)(CagdBType ComputeNormals,
						     CagdBType ComputeUV,
						     const CagdRType *Pt1,
						     const CagdRType *Pt2,
						     const CagdRType *Pt3,
						     const CagdRType *Nl1,
						     const CagdRType *Nl2,
						     const CagdRType *Nl3,
						     const CagdRType *UV1,
						     const CagdRType *UV2,
						     const CagdRType *UV3,
						     CagdBType *GenPoly);
typedef CagdPolygonStruct *(*CagdSrfMakeRectFuncType)(CagdBType ComputeNormals,
						      CagdBType ComputeUV,
						      const CagdRType *Pt1,
						      const CagdRType *Pt2,
						      const CagdRType *Pt3,
						      const CagdRType *Pt4,
						      const CagdRType *Nl1,
						      const CagdRType *Nl2,
						      const CagdRType *Nl3,
						      const CagdRType *Nl4,
						      const CagdRType *UV1,
						      const CagdRType *UV2,
						      const CagdRType *UV3,
						      const CagdRType *UV4,
						      CagdBType *GenPoly);
typedef CagdRType (*CagdPlgErrorFuncType)(const CagdPType P1,
					  const CagdPType P2,
					  const CagdPType P3);

#define CAGD_IS_BEZIER_CRV(Crv)		((Crv) -> GType == CAGD_CBEZIER_TYPE)
#define CAGD_IS_BEZIER_SRF(Srf)		((Srf) -> GType == CAGD_SBEZIER_TYPE)
#define CAGD_IS_BSPLINE_CRV(Crv)	((Crv) -> GType == CAGD_CBSPLINE_TYPE)
#define CAGD_IS_BSPLINE_SRF(Srf)	((Srf) -> GType == CAGD_SBSPLINE_TYPE)
#define CAGD_IS_POWER_CRV(Crv)		((Crv) -> GType == CAGD_CPOWER_TYPE)
#define CAGD_IS_POWER_SRF(Srf)		((Srf) -> GType == CAGD_SPOWER_TYPE)

/******************************************************************************
*		U -->			    The mesh is ordered raw after raw *
*       +-----------------------+	or the increments along U are 1 while *
*   V | |P0                 Pi-1|	the increment along V is full raw.    *
*     v	|Pi                P2i-1|	    To encapsulate it, NEXTU/V are    *
*	|			|	defined below.			      *
*	|Pn-i		    Pn-1|					      *
*	+-----------------------+					      *
******************************************************************************/
#define CAGD_NEXT_U(Srf)	(1)
#define CAGD_NEXT_V(Srf)	((Srf) -> ULength)
#define CAGD_MESH_UV(Srf, i, j)	((i) + ((Srf) -> ULength) * (j))

#define CAGD_GEN_COPY(Dst, Src, Size) memcpy((char *) (Dst), (char *) (Src), \
					     (Size))

#define CAGD_GEN_COPY_STEP(Dst, Src, Size, DstStep, SrcStep, Type) \
			{ \
			    int _ii; \
			    Type *_DstType = (Type *) (Dst), \
				 *_SrcType = (Type *) (Src); \
 \
			    for (_ii = 0; _ii < Size; _ii++) { \
				*_DstType = *_SrcType; \
				_DstType += DstStep; \
				_SrcType += SrcStep; \
			    } \
			}

/******************************************************************************
* Some points/vectors simplifying operators.				      *
******************************************************************************/
#define	CAGD_COPY_POINT(DstPt, SrcPt)	{ (DstPt) = (SrcPt); }
#define	CAGD_ADD_POINT(DstPt, SrcPt)    { (DstPt).Pt[0] += (SrcPt).Pt[0]; \
					  (DstPt).Pt[1] += (SrcPt).Pt[1]; \
					  (DstPt).Pt[2] += (SrcPt).Pt[2]; }
#define	CAGD_SUB_POINT(DstPt, SrcPt)    { (DstPt).Pt[0] -= (SrcPt).Pt[0]; \
					  (DstPt).Pt[1] -= (SrcPt).Pt[1]; \
					  (DstPt).Pt[2] -= (SrcPt).Pt[2]; }
#define	CAGD_MULT_POINT(DstPt, Scaler)  { (DstPt).Pt[0] *= (Scaler); \
					  (DstPt).Pt[1] *= (Scaler); \
					  (DstPt).Pt[2] *= (Scaler); }

#define	CAGD_COPY_VECTOR(DstVec, SrcVec) { (DstVec) = (SrcVec); }
#define	CAGD_ADD_VECTOR(DstVec, SrcVec) { (DstVec).Vec[0] += (SrcVec).Vec[0]; \
					  (DstVec).Vec[1] += (SrcVec).Vec[1]; \
					  (DstVec).Vec[2] += (SrcVec).Vec[2]; }
#define	CAGD_SUB_VECTOR(DstVec, SrcVec) { (DstVec).Vec[0] -= (SrcVec).Vec[0]; \
					  (DstVec).Vec[1] -= (SrcVec).Vec[1]; \
					  (DstVec).Vec[2] -= (SrcVec).Vec[2]; }
#define	CAGD_MULT_VECTOR(DstVec, Scaler){ (DstVec).Vec[0] *= (Scaler); \
					  (DstVec).Vec[1] *= (Scaler); \
					  (DstVec).Vec[2] *= (Scaler); }
#define	CAGD_DIV_VECTOR(DstVec, Scaler) { (DstVec).Vec[0] /= (Scaler); \
					  (DstVec).Vec[1] /= (Scaler); \
					  (DstVec).Vec[2] /= (Scaler); }
#define CAGD_SQR_LEN_VECTOR(V)		(IRIT_SQR((V).Vec[0]) + \
					     IRIT_SQR((V).Vec[1]) + \
					     IRIT_SQR((V).Vec[2]))
#define CAGD_LEN_VECTOR(V)		sqrt(CAGD_SQR_LEN_VECTOR(V))
#define CAGD_NORMALIZE_VECTOR(V)	{ CagdRType __t = CAGD_LEN_VECTOR(V); \
					  { \
					      __t = 1.0 / __t; \
					      CAGD_MULT_VECTOR((V), __t); \
					  } \
				        }
#define CAGD_NORMALIZE_VECTOR_MSG_ZERO(V) \
					{ CagdRType __t = CAGD_LEN_VECTOR(V); \
					  _IRIT_PT_NORMALIZE_MSG_ZERO(__t) \
					  { \
					      __t = 1.0 / __t; \
					      CAGD_MULT_VECTOR((V), __t); \
					  } \
				        }


#define	CAGD_COPY_UVVAL(DstUV, SrcUV) { (DstUV) = (SrcUV); }

#define CAGD_PT_ON_BNDRY(u, v, UMin, UMax, VMin, VMax, Eps) \
	(IRIT_APX_EQ_EPS(u, UMin, Eps) || IRIT_APX_EQ_EPS(u, UMax, Eps) || \
	 IRIT_APX_EQ_EPS(v, VMin, Eps) || IRIT_APX_EQ_EPS(v, VMax, Eps))

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call CagdFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define CAGD_FATAL_ERROR(MsgID)	CagdFatalError(MsgID)

/******************************************************************************
* Macros to verify the validity of the parametric domain.		      *
******************************************************************************/
#ifdef IRIT_DOUBLE
#define CAGD_DOMAIN_IRIT_EPS 1e-13
#else
#define CAGD_DOMAIN_IRIT_EPS 1e-5
#endif /* IRIT_DOUBLE */

#define CAGD_DOMAIN_T_VERIFY(t, TMin, TMax) \
    { \
	if (t < TMin) \
	    t += CAGD_DOMAIN_IRIT_EPS; \
	if (t > TMax) \
	    t -= CAGD_DOMAIN_IRIT_EPS; \
	if (t < TMin || t > TMax) \
	    CAGD_FATAL_ERROR(CAGD_ERR_T_NOT_IN_CRV); \
    }

#define CAGD_DOMAIN_GET_AND_VERIFY_CRV(t, Crv, TMin, TMax) \
    { \
	CagdCrvDomain(Crv, &TMin, &TMax); \
	CAGD_DOMAIN_T_VERIFY(t, TMin, TMax); \
    }

#define CAGD_DOMAIN_GET_AND_VERIFY_SRF(u, v, Srf, UMin, UMax, VMin, VMax) \
    { \
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax); \
	CAGD_DOMAIN_T_VERIFY(u, UMin, UMax); \
	CAGD_DOMAIN_T_VERIFY(v, VMin, VMax); \
    }

/******************************************************************************
* A fast macro to blend the original control polygon with the Alpha matrix.   *
******************************************************************************/

#define CAGD_ALPHA_LOOP_BLEND_NOT_PERIODIC_OLD(A, IMin, IMax, OrigPts, RefPts) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType \
	**_Rows = A -> Rows; \
 \
    IRIT_ZAP_MEM(RefPts, sizeof(CagdRType) * (IMax - IMin)); \
 \
    for (_i = IMin; _i < IMax; _i++) { \
	if (*_ColLength++ == 1) \
	    *RefPts++ = OrigPts[*_ColIndex++]; \
	else { \
	    int _Len = _ColLength[-1], \
	        _Idx = *_ColIndex++ + --_Len; \
	    CagdRType \
		*_Pts = &OrigPts[_Idx]; \
 \
	    for ( ; _Len-- >= 0; _Idx--) { \
		*RefPts += *_Pts-- * _Rows[_Idx][_i]; \
	    } \
 \
	    RefPts++; \
	} \
    } \
}

#define CAGD_ALPHA_LOOP_BLEND_PERIODIC_OLD(A, IMin, IMax, OrigPts, OrigLen, RefPts) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType \
	**_Rows = A -> Rows; \
 \
    IRIT_ZAP_MEM(RefPts, sizeof(CagdRType) * (IMax - IMin)); \
 \
    for (_i = IMin; _i < IMax; _i++) { \
	if (*_ColLength++ == 1) { \
	    int _Idx = *_ColIndex++; \
	    *RefPts++ = OrigPts[_Idx >= OrigLen ? _Idx - OrigLen \
			                        : _Idx]; \
	} \
	else { \
	    int _Len = _ColLength[-1], \
	        _Idx = *_ColIndex++ + --_Len; \
	    CagdRType \
		*_Pts = &OrigPts[_Idx]; \
 \
	    for ( ; _Len-- >= 0; _Idx--) { \
		*RefPts += *_Pts-- * _Rows[_Idx >= OrigLen ? _Idx - OrigLen \
			                                   : _Idx][_i]; \
	    } \
 \
	    RefPts++; \
	} \
    } \
}

#define CAGD_ALPHA_LOOP_BLEND_STEP_OLD(A, IMin, IMax, OrigPts, OrigPtsStep, \
				   OrigLen, RefPts, RefPtsStep) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType \
	**_Rows = A -> Rows; \
 \
    for (_i = IMin; _i < IMax; _i++) { \
	if (*_ColLength++ == 1) { \
	    *RefPts = OrigPts[*_ColIndex++ * OrigPtsStep]; \
	} \
	else { \
	    int _Len = _ColLength[-1], \
	        _Idx = *_ColIndex++ + --_Len; \
	    CagdRType \
		*_Pts = &OrigPts[_Idx * OrigPtsStep]; \
 \
	    for (*RefPts = 0.0; _Len-- >= 0; _Idx--, _Pts -= OrigPtsStep) { \
		*RefPts += *_Pts * _Rows[_Idx][_i]; \
	    } \
	} \
	RefPts += RefPtsStep; \
    } \
}

#define CAGD_ALPHA_LOOP_BLEND_NOT_PERIODIC(A, IMin, IMax, OrigPts, RefPts) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType *_p, *_r; \
  \
    IRIT_ZAP_MEM(RefPts, sizeof(CagdRType) * (IMax - IMin)); \
  \
    for (_i = IMin; _i < IMax; _i++) { \
	switch (*_ColLength++) { \
 	    case 1: \
		*RefPts++ = OrigPts[*_ColIndex++]; \
		break; \
 	    case 2: \
		_p = &OrigPts[*_ColIndex]; \
		_r = &A -> RowsTransp[_i][*_ColIndex++]; \
		*RefPts++ = _p[0] * _r[0] + \
		            _p[1] * _r[1]; \
		break; \
 	    case 3: \
		_p = &OrigPts[*_ColIndex]; \
		_r = &A -> RowsTransp[_i][*_ColIndex++]; \
		*RefPts++ = _p[0] * _r[0] + \
		            _p[1] * _r[1] + \
		            _p[2] * _r[2]; \
		break; \
 	    case 4: \
		_p = &OrigPts[*_ColIndex]; \
		_r = &A -> RowsTransp[_i][*_ColIndex++]; \
		*RefPts++ = _p[0] * _r[0] + \
		            _p[1] * _r[1] + \
		            _p[2] * _r[2] + \
		            _p[3] * _r[3]; \
		break; \
	    default: \
		{ \
		    int _Len = _ColLength[-1]; \
 \
		    _p = &OrigPts[*_ColIndex]; \
		    _r = &A -> RowsTransp[_i][*_ColIndex++]; \
 \
		    for (*RefPts = 0.0; _Len-- > 0; ) { \
			*RefPts += *_p++ * *_r++; \
		    } \
 \
		    RefPts++; \
	        } \
		break; \
	} \
    } \
}

#define CAGD_ALPHA_LOOP_BLEND_PERIODIC(A, IMin, IMax, OrigPts, OrigLen, RefPts) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType *_p, *_r; \
 \
    for (_i = IMin; _i < IMax; _i++) { \
        int _Len = *_ColLength++, \
	    _Idx = *_ColIndex++; \
 \
	if (_Idx + _Len <= OrigLen) { \
	    switch (_Len) { \
 	        case 1: \
		    *RefPts++ = OrigPts[_Idx]; \
		    break; \
 	        case 2: \
		    _p = &OrigPts[_Idx]; \
		    _r = &A -> RowsTransp[_i][_Idx]; \
		    *RefPts++ = _p[0] * _r[0] + \
		                _p[1] * _r[1]; \
		    break; \
 	        case 3: \
		    _p = &OrigPts[_Idx]; \
		    _r = &A -> RowsTransp[_i][_Idx]; \
		    *RefPts++ = _p[0] * _r[0] + \
		                _p[1] * _r[1] + \
		                _p[2] * _r[2]; \
		    break; \
 	        case 4: \
		    _p = &OrigPts[_Idx]; \
		    _r = &A -> RowsTransp[_i][_Idx]; \
		    *RefPts++ = _p[0] * _r[0] + \
		                _p[1] * _r[1] + \
		                _p[2] * _r[2] + \
		                _p[3] * _r[3]; \
		    break; \
	        default: \
	  	    { \
		        if (_Len == 1) { \
			    *RefPts++ = OrigPts[_Idx]; \
			} \
			else { \
			    _p = &OrigPts[_Idx]; \
			    _r = &A -> RowsTransp[_i][_Idx]; \
 \
			    for (*RefPts = 0.0 ; _Len-- > 0; ) { \
				*RefPts += *_p++ * *_r++; \
			    } \
 \
			    RefPts++; \
			} \
		    } \
	    } \
	} \
	else { \
	    if (_Len == 1) { \
		*RefPts++ = OrigPts[_Idx >= OrigLen ? _Idx - OrigLen : _Idx]; \
	    } \
	    else { \
		_p = &OrigPts[_Idx]; \
		_r = A -> RowsTransp[_i]; \
 \
		for (*RefPts = 0.0 ; _Len-- > 0; _Idx++) { \
		    *RefPts += *_p++ * _r[_Idx >= OrigLen ? _Idx - OrigLen : \
			                                    _Idx]; \
		} \
 \
		RefPts++; \
	    } \
	} \
    } \
}

#define CAGD_ALPHA_LOOP_BLEND_STEP(A, IMin, IMax, OrigPts, OrigPtsStep, \
				   OrigLen, RefPts, RefPtsStep) \
{ \
    int _i, \
	*_ColLength = &A -> ColLength[IMin], \
	*_ColIndex = &A -> ColIndex[IMin]; \
    CagdRType \
	**_Rows = A -> Rows; \
 \
    for (_i = IMin; _i < IMax; _i++) { \
	if (*_ColLength++ == 1) { \
	    *RefPts = OrigPts[*_ColIndex++ * OrigPtsStep]; \
	} \
	else { \
	    int _Len = _ColLength[-1], \
	        _Idx = *_ColIndex++ + --_Len; \
	    CagdRType \
		*_Pts = &OrigPts[_Idx * OrigPtsStep]; \
 \
	    for (*RefPts = 0.0; _Len-- >= 0; _Idx--, _Pts -= OrigPtsStep) { \
		*RefPts += *_Pts * _Rows[_Idx][_i]; \
	    } \
	} \
	RefPts += RefPtsStep; \
    } \
}

/******************************************************************************
* A fast macro to evaluate a point out of a control points' mesh/vector.      *
******************************************************************************/

#define BSP_CRV_EVAL_VEC_AT_PARAM(Res, Pts, Inc, Order, Len, \
				  t, BasisFuncs, IndexFirst) \
    { \
	int _i, \
	    _IFirst = IndexFirst; \
	CagdRType \
            *_B = BasisFuncs; \
 \
	*Res = 0.0; \
	if (Inc == 1) { \
	    for (_i = 0; _i < Order; _i++) { \
		*Res += *_B++ * Pts[_IFirst]; \
		if (++_IFirst >= Len) \
		    _IFirst -= Len; \
	    } \
        } \
	else { \
	    int _IFirstInc = _IFirst; \
 \
	    _IFirstInc *= Inc; \
	    for (_i = 0; _i < Order; _i++) { \
		*Res += *_B++ * Pts[_IFirstInc]; \
	        _IFirstInc += Inc; \
	        if (++_IFirst >= Len) { \
	            _IFirst -= Len; \
		    _IFirstInc -= Len * Inc; \
	        } \
	    } \
        } \
    }

/******************************************************************************
* There are some circular links to symb_lib and we include symb.lib.h         *
* to resolve that. This may cause some problems in linkage time.              *
* These are the known circularities:				              *
* Bsp/BzrCrvMult, Bsp/BzrSrfMult, Bsp/BzrCrvDeriveRational.	              *
******************************************************************************/
#include "symb_lib.h"

IRIT_GLOBAL_DATA_HEADER CagdBType _CagdSrfMakeOnlyTri;
IRIT_GLOBAL_DATA_HEADER CagdSrfMakeTriFuncType _CagdSrfMakeTriFunc;
IRIT_GLOBAL_DATA_HEADER CagdSrfMakeRectFuncType _CagdSrfMakeRectFunc;

/******************************************************************************
* Routines prototypes. Routines are prefixed as follows:		      *
* Cagd    - General routines such as dynamic memory handlers etc.	      *
* BzrCrv  - Bezier curves routines.					      *
* BzrSrf  - Bezier surface routines.					      *
* BspKnot - Bspline knot vector routines.				      *
* BspCrv  - Bspline curves routines.					      *
* BspSrf  - Bspline surface routines.					      *
* CagdCnvrt   - Conversion routines such as Bezier to Power basis.	      *
******************************************************************************/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* General global variable of the Cagd library:				      *
******************************************************************************/
IRIT_GLOBAL_DATA_HEADER const CagdRType
    CagdIChooseKTable[CAGD_MAX_BEZIER_CACHE_ORDER + 1]
	             [CAGD_MAX_BEZIER_CACHE_ORDER + 1];


IRIT_GLOBAL_DATA_HEADER const CagdRType 
    CagdIcKJcMIJcKMTable[CAGD_MAX_BEZIER_CACHE_ORDER2 + 1]
                        [CAGD_MAX_BEZIER_CACHE_ORDER2 + 1]
                        [CAGD_MAX_BEZIER_CACHE_ORDER2 + 1]
			[CAGD_MAX_BEZIER_CACHE_ORDER2 + 1];

/******************************************************************************
* General routines of the Cagd library:					      *
******************************************************************************/

CagdUVStruct *CagdUVNew(void);
CagdPtStruct *CagdPtNew(void);
CagdSrfPtStruct *CagdSrfPtNew(void);
CagdCtlPtStruct *CagdCtlPtNew(CagdPointType PtType);
CagdVecStruct *CagdVecNew(void);
CagdPlaneStruct *CagdPlaneNew(void);
CagdBBoxStruct *CagdBBoxNew(void);
CagdCrvStruct *CagdCrvNew(CagdGeomType GType, CagdPointType PType, int Length);
CagdCrvStruct *CagdPeriodicCrvNew(CagdGeomType GType,
				  CagdPointType PType,
				  int Length,
				  CagdBType Periodic);
CagdSrfStruct *CagdSrfNew(CagdGeomType GType,
			  CagdPointType PType,
			  int ULength,
			  int VLength);
CagdSrfStruct *CagdPeriodicSrfNew(CagdGeomType GType,
				  CagdPointType PType,
				  int ULength,
				  int VLength,
				  CagdBType UPeriodic,
				  CagdBType VPeriodic);
CagdPolygonStruct *CagdPolygonNew(int Len);
CagdPolygonStruct *CagdPolygonStripNew(int Len);
CagdPolylineStruct *CagdPolylineNew(int Length);

CagdUVStruct *CagdUVArrayNew(int Size);
CagdPtStruct *CagdPtArrayNew(int Size);
CagdCtlPtStruct *CagdCtlPtArrayNew(CagdPointType PtType, int Size);
CagdVecStruct *CagdVecArrayNew(int Size);
CagdPlaneStruct *CagdPlaneArrayNew(int Size);
CagdBBoxStruct *CagdBBoxArrayNew(int Size);
CagdPolygonStruct *CagdPolygonArrayNew(int Size);
CagdPolylineStruct *CagdPolylineArrayNew(int Length, int Size);

CagdCrvStruct *CagdCrvCopy(const CagdCrvStruct *Crv);
CagdSrfStruct *CagdSrfCopy(const CagdSrfStruct *Srf);
CagdUVStruct *CagdUVCopy(const CagdUVStruct *UV);
CagdPtStruct *CagdPtCopy(const CagdPtStruct *Pt);
CagdSrfPtStruct *CagdSrfPtCopy(const CagdSrfPtStruct *Pt);
CagdCtlPtStruct *CagdCtlPtCopy(const CagdCtlPtStruct *CtlPt);
CagdVecStruct *CagdVecCopy(const CagdVecStruct *Vec);
CagdPlaneStruct *CagdPlaneCopy(const CagdPlaneStruct *Plane);
CagdBBoxStruct *CagdBBoxCopy(const CagdBBoxStruct *BBoc);
CagdPolygonStruct *CagdPolygonCopy(const CagdPolygonStruct *Poly);
CagdPolylineStruct *CagdPolylineCopy(const CagdPolylineStruct *Poly);

CagdCrvStruct *CagdCrvCopyList(const CagdCrvStruct *CrvList);
CagdSrfStruct *CagdSrfCopyList(const CagdSrfStruct *SrfList);
CagdUVStruct *CagdUVCopyList(const CagdUVStruct *UVList);
CagdPtStruct *CagdPtCopyList(const CagdPtStruct *PtList);
CagdSrfPtStruct *CagdSrfPtCopyList(const CagdSrfPtStruct *SrfPtList);
CagdCtlPtStruct *CagdCtlPtCopyList(const CagdCtlPtStruct *CtlPtList);
CagdVecStruct *CagdVecCopyList(const CagdVecStruct *VecList);
CagdPlaneStruct *CagdPlaneCopyList(const CagdPlaneStruct *PlaneList);
CagdBBoxStruct *CagdBBoxCopyList(const CagdBBoxStruct *BBoxList);
CagdPolylineStruct *CagdPolylineCopyList(const CagdPolylineStruct *PolyList);
CagdPolygonStruct *CagdPolygonCopyList(const CagdPolygonStruct *PolyList);

void CagdCrvFree(CagdCrvStruct *Crv);
void CagdCrvFreeList(CagdCrvStruct *CrvList);
void CagdSrfFree(CagdSrfStruct *Srf);
void CagdSrfFreeList(CagdSrfStruct *SrfList);
void CagdSrfFreeCache(CagdSrfStruct *Srf);
void CagdUVFree(CagdUVStruct *UV);
void CagdUVFreeList(CagdUVStruct *UVList);
void CagdUVArrayFree(CagdUVStruct *UVArray, int Size);
void CagdPtFree(CagdPtStruct *Pt);
void CagdPtFreeList(CagdPtStruct *PtList);
void CagdSrfPtFree(CagdSrfPtStruct *SrfPt);
void CagdSrfPtFreeList(CagdSrfPtStruct *SrfPtList);
void CagdPtArrayFree(CagdPtStruct *PtArray, int Size);
void CagdCtlPtFree(CagdCtlPtStruct *CtlPt);
void CagdCtlPtFreeList(CagdCtlPtStruct *CtlPtList);
void CagdCtlPtArrayFree(CagdCtlPtStruct *CtlPtArray, int Size);
void CagdVecFree(CagdVecStruct *Vec);
void CagdVecFreeList(CagdVecStruct *VecList);
void CagdVecArrayFree(CagdVecStruct *VecArray, int Size);
void CagdPlaneFree(CagdPlaneStruct *Plane);
void CagdPlaneFreeList(CagdPlaneStruct *PlaneList);
void CagdPlaneArrayFree(CagdPlaneStruct *PlaneArray, int Size);
void CagdBBoxFree(CagdBBoxStruct *BBox);
void CagdBBoxFreeList(CagdBBoxStruct *BBoxList);
void CagdBBoxArrayFree(CagdBBoxStruct *BBoxArray, int Size);
void CagdPolylineFree(CagdPolylineStruct *Poly);
void CagdPolylineFreeList(CagdPolylineStruct *PolyList);
void CagdPolygonFree(CagdPolygonStruct *Poly);
void CagdPolygonFreeList(CagdPolygonStruct *PolyList);

#ifdef DEBUG
#define CagdCrvFree(Crv)         { CagdCrvFree(Crv); Crv = NULL; }
#define CagdCrvFreeList(CrvList) { CagdCrvFreeList(CrvList); CrvList = NULL; }
#define CagdSrfFree(Srf)         { CagdSrfFree(Srf); Srf = NULL; }
#define CagdSrfFreeList(SrfList) { CagdSrfFreeList(SrfList); SrfList = NULL; }
#define CagdUVFree(UV)           { CagdUVFree(UV); UV = NULL; }
#define CagdUVFreeList(UVList)   { CagdUVFreeList(UVList); UVList = NULL; }
#define CagdUVArrayFree(UVArray, \
			Size)    { CagdUVArrayFree(UVArray, Size); \
				   UVArray = NULL; }
#define CagdPtFree(Pt)           { CagdPtFree(Pt); Pt = NULL; }
#define CagdPtFreeList(PtList)   { CagdPtFreeList(PtList); PtList = NULL; }
#define CagdSrfPtFree(SrfPt)     { CagdSrfPtFree(SrfPt); SrfPt = NULL; }
#define CagdSrfPtFreeList(SrfPtList) { CagdSrfPtFreeList(SrfPtList); \
				       SrfPtList = NULL; }
#define CagdPtArrayFree(PtArray, \
			Size)    { CagdPtArrayFree(PtArray, Size); \
				   PtArray = NULL; }
#define CagdCtlPtFree(CtlPt)     { CagdCtlPtFree(CtlPt); CtlPt = NULL; }
#define CagdCtlPtFreeList(CtlPtList) { CagdCtlPtFreeList(CtlPtList); \
				       CtlPtList = NULL; }
#define CagdCtlPtArrayFree(CtlPtArray, \
			   Size) { CagdCtlPtArrayFree(CtlPtArray, Size); \
				   CtlPtArray = NULL; }
#define CagdVecFree(Vec)         { CagdVecFree(Vec); Vec = NULL; }
#define CagdVecFreeList(VecList) { CagdVecFreeList(VecList); VecList = NULL; }
#define CagdVecArrayFree(VecArray, \
			 Size)   { CagdVecArrayFree(VecArray, Size); \
				   VecArray = NULL; }
#define CagdPlaneFree(Plane)     { CagdPlaneFree(Plane); Plane = NULL; }
#define CagdPlaneFreeList(PlaneList) { CagdPlaneFreeList(PlaneList); \
				       PlaneList = NULL; }
#define CagdPlaneArrayFree(PlaneArray, \
			   Size) { CagdPlaneArrayFree(PlaneArray, Size); \
				   PlaneArray = NULL; }
#define CagdBBoxFree(BBox)       { CagdBBoxFree(BBox); BBox = NULL; }
#define CagdBBoxFreeList(BBoxList) { CagdBBoxFreeList(BBoxList); \
				     BBoxList = NULL; }
#define CagdBBoxArrayFree(BBoxArray, \
			  Size) { CagdBBoxArrayFree(BBoxArray, Size); \
				  BBoxArray = NULL; }
#define CagdPolylineFree(Poly)   { CagdPolylineFree(Poly); Poly = NULL; }
#define CagdPolylineFreeList(PolyList) { CagdPolylineFreeList(PolyList); \
					 PolyList = NULL; }
#define CagdPolylineArrayFree(PolyArray, \
			      Size) { CagdPolylineArrayFree(PolyArray, Size); \
				      PolyArray = NULL; }
#define CagdPolygonFree(Poly)    { CagdPolygonFree(Poly); Poly = NULL; }
#define CagdPolygonFreeList(PolyList) { CagdPolygonFreeList(PolyList); \
					PolyList = NULL; }
#define CagdPolygonArrayFree(PolyArray, \
			     Size) { CagdPolygonArrayFree(PolyArray, Size); \
				     PolyArray = NULL; }
#endif /* DEBUG */

VoidPtr CagdListInsert(VoidPtr List,
		       VoidPtr NewElement,
		       CagdCompFuncType CompFunc,
		       CagdBType InsertEqual);
int CagdListLength(const VoidPtr List);
VoidPtr CagdListAppend(VoidPtr List1, VoidPtr List2);
VoidPtr CagdListReverse(VoidPtr List);
VoidPtr CagdListLast(VoidPtr List);
VoidPtr CagdListPrev(VoidPtr List, VoidPtr Item);
void CagdCoerceToE2(CagdRType *E2Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType);
void CagdCoerceToE3(CagdRType *E3Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType);
void CagdCoerceToP2(CagdRType *P2Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType);
void CagdCoerceToP3(CagdRType *P3Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType);
void CagdCoercePointTo(CagdRType *NewPoint,
		       CagdPointType NewPType,
		       CagdRType * const Points[CAGD_MAX_PT_SIZE],
		       int Index,
		       CagdPointType OldPType);
void CagdCoercePointsTo(CagdRType *Points[],
			int Len,
			CagdPointType OldPType,
			CagdPointType NewPType);
VoidPtr CagdStructOnceCoercePointsTo(CagdRType * const OldPoints[],
				     const VoidPtr OldStruct,
				     int OldStructLen,
				     int ExtraMem,
				     int PtsLen,
				     CagdPointType OldPType,
				     CagdPointType NewPType);
CagdRType CagdDistTwoCtlPt(CagdRType * const Pt1[CAGD_MAX_PT_SIZE],
			   int Index1,
			   CagdRType * const Pt2[CAGD_MAX_PT_SIZE],
			   int Index2,
			   CagdPointType PType);
CagdBType CagdCtlMeshsSame(CagdRType * const Mesh1[],
			   CagdRType * const Mesh2[],
			   int Len,
			   CagdRType Eps);
CagdBType CagdCtlMeshsSameUptoRigidScl2D(CagdRType * const Mesh1[],
					 CagdRType * const Mesh2[],
					 int Len,
					 IrtPtType Trans,
					 CagdRType *Rot,
					 CagdRType *Scl,
					 CagdRType Eps);
CagdCrvStruct *CagdCoerceCrvsTo(const CagdCrvStruct *Crv,
				CagdPointType PType,
				CagdBType AddParametrization);
CagdCrvStruct *CagdCoerceCrvTo(const CagdCrvStruct *Crv,
			       CagdPointType PType,
			       CagdBType AddParametrization);
CagdSrfStruct *CagdCoerceSrfsTo(const CagdSrfStruct *Srf,
				CagdPointType PType,
				CagdBType AddParametrization);
CagdSrfStruct *CagdCoerceSrfTo(const CagdSrfStruct *Srf,
			       CagdPointType PType,
			       CagdBType AddParametrization);
CagdPointType CagdMergeIrtPtType(CagdPointType PType1, CagdPointType PType2);
void CagdDbg(const void *Obj);
#ifdef DEBUG
void CagdDbgV(const void *Obj);
#endif /* DEBUG */
void CagdSetLinear2Poly(CagdLin2PolyType Lin2Poly);
CagdBType CagdTightBBox(CagdBType TightBBox);
CagdBType CagdIgnoreNonPosWeightBBox(CagdBType IgnoreNonPosWeightBBox);
void CagdMergeBBox(CagdBBoxStruct *DestBBox, const CagdBBoxStruct *SrcBBox);
void CagdPointsBBox(CagdRType * const *Points,
		    int Length,
		    int Dim,
		    CagdRType *BBoxMin,
		    CagdRType *BBoxMax);
CagdRType CagdIChooseK(int i, int k);
void CagdTransform(CagdRType **Points,
		   int Len,
		   int MaxCoord,
		   CagdBType IsNotRational,
		   const CagdRType *Translate,
		   CagdRType Scale);
void CagdScale(CagdRType **Points,
	       int Len,
	       int MaxCoord,
	       const CagdRType *Scale);
void CagdMatTransform(CagdRType **Points,
		      int Len,
		      int MaxCoord,
		      CagdBType IsNotRational,
		      CagdMType Mat);
CagdBType CagdPointsHasPoles(CagdRType * const *Points, int Len);
CagdBType CagdAllWeightsNegative(CagdRType * const *Points,
				 CagdPointType PType,
				 int Len,
				 CagdBType Flip);
CagdBType CagdAllWeightsSame(CagdRType * const *Points, int Len);
CagdPlgErrorFuncType CagdPolygonSetErrFunc(CagdPlgErrorFuncType Func);
CagdSrfMakeTriFuncType CagdSrfSetMakeTriFunc(CagdSrfMakeTriFuncType Func);
CagdSrfMakeRectFuncType CagdSrfSetMakeRectFunc(CagdSrfMakeRectFuncType Func);
CagdBType CagdSrfSetMakeOnlyTri(CagdBType OnlyTri);
CagdPolygonStruct *CagdMakeTriangle(CagdBType ComputeNormals,
				    CagdBType ComputeUV,
				    const CagdRType *Pt1,
				    const CagdRType *Pt2,
				    const CagdRType *Pt3,
				    const CagdRType *Nl1,
				    const CagdRType *Nl2,
				    const CagdRType *Nl3,
				    const CagdRType *UV1,
				    const CagdRType *UV2,
				    const CagdRType *UV3,
				    CagdBType *GenPoly);
CagdPolygonStruct *CagdMakeRectangle(CagdBType ComputeNormals,
				     CagdBType ComputeUV,
				     const CagdRType *Pt1,
				     const CagdRType *Pt2,
				     const CagdRType *Pt3,
				     const CagdRType *Pt4,
				     const CagdRType *Nl1,
				     const CagdRType *Nl2,
				     const CagdRType *Nl3,
				     const CagdRType *Nl4,
				     const CagdRType *UV1,
				     const CagdRType *UV2,
				     const CagdRType *UV3,
				     const CagdRType *UV4,
				     CagdBType *GenPoly);
CagdPolylineStruct *CagdPtPolyline2E3Polyline(
				CagdRType * const Polyline[CAGD_MAX_PT_SIZE],
				int n,
				int MaxCoord,
				int IsRational);
CagdPtStruct *CagdPtsSortAxis(CagdPtStruct *PtList, int Axis);
CagdBType CagdCrvOnOneSideOfLine(const CagdCrvStruct *Crv,
				 CagdRType X1,
				 CagdRType Y1,
				 CagdRType X2,
				 CagdRType Y2);

void CagdPolygonBBox(const CagdPolygonStruct *Poly, CagdBBoxStruct *BBox);
void CagdPolygonListBBox(const CagdPolygonStruct *Polys, CagdBBoxStruct *BBox);

/******************************************************************************
* Blossoming								      *
******************************************************************************/

CagdBlsmAlphaCoeffStruct *CagdBlsmAllocAlphaCoef(int Order,
						 int Length,
						 int NewOrder,
						 int NewLength,
						 int Periodic);
CagdBlsmAlphaCoeffStruct *CagdBlsmCopyAlphaCoef(const CagdBlsmAlphaCoeffStruct
						                           *A);
void CagdBlsmFreeAlphaCoef(CagdBlsmAlphaCoeffStruct *A);
void CagdBlsmAddRowAlphaCoef(CagdBlsmAlphaCoeffStruct *A,
			     CagdRType *Coefs,
			     int ARow,
			     int ColIndex,
			     int ColLength);
void CagdBlsmSetDomainAlphaCoef(CagdBlsmAlphaCoeffStruct *A);
void CagdBlsmScaleAlphaCoef(CagdBlsmAlphaCoeffStruct *A, CagdRType Scl);

CagdRType *CagdBlsmEvalSymb(int Order,
			    const CagdRType *Knots,
			    int KnotsLen,
			    const CagdRType *BlsmVals,
			    int BlsmLen,
			    int *RetIdxFirst,
			    int *RetLength);
CagdRType CagdBlossomEval(const CagdRType *Pts,
			  int PtsStep,
			  int Order,
			  const CagdRType *Knots,
			  int KnotsLen,
			  const CagdRType *BlsmVals,
			  int BlsmLen);
CagdRType *CagdCrvBlossomEval(const CagdCrvStruct *Crv,
			      const CagdRType *BlsmVals,
			      int BlsmLen);
CagdRType *CagdSrfBlossomEval(const CagdSrfStruct *Srf,
			      const CagdRType *BlsmUVals,
			      int BlsmULen,
			      const CagdRType *BlsmVVals,
			      int BlsmVLen);
CagdCrvStruct *CagdSrfBlossomEvalU(const CagdSrfStruct *Srf,
				   const CagdRType *BlsmUVals,
				   int BlsmULen);

CagdCrvStruct *CagdCrvBlossomDegreeRaiseN(const CagdCrvStruct *Crv,
					  int NewOrder);
CagdCrvStruct *CagdCrvBlossomDegreeRaise(const CagdCrvStruct *Crv);
CagdSrfStruct *CagdSrfBlossomDegreeRaiseN(const CagdSrfStruct *Srf,
					  int NewUOrder,
					  int NewVOrder);
CagdSrfStruct *CagdSrfBlossomDegreeRaise(const CagdSrfStruct *Srf,
					 CagdSrfDirType Dir);

CagdBlsmAlphaCoeffStruct *CagdDegreeRaiseMatProd(CagdBlsmAlphaCoeffStruct *A1,
						 CagdBlsmAlphaCoeffStruct *A2);
CagdBlsmAlphaCoeffStruct *CagdBlossomDegreeRaiseMat(const CagdRType *KV,
						    int Order,
						    int Len);
CagdBlsmAlphaCoeffStruct *CagdBlossomDegreeRaiseNMat(const CagdRType *KV,
						     int Order,
						     int NewOrder,
						     int Len);

/******************************************************************************
* Matrix/Vector/Plane/Transformation routines:				      *
******************************************************************************/
CagdRType CagdFitPlaneThruCtlPts(CagdPlaneStruct *Plane,
				 CagdPointType PType,
			         CagdRType * const *Points,
			         int Index1,
				 int Index2,
				 int Index3,
				 int Index4);
CagdRType CagdDistPtPlane(const CagdPlaneStruct *Plane,
			  CagdRType * const *Points,
			  int Index,
			  int MaxDim);
CagdRType CagdDistPtLine(const CagdVecStruct *LineDir,
			 CagdRType * const *Points,
			 int Index,
			 int MaxDim);
CagdCrvStruct *CagdCrvMatTransform(const CagdCrvStruct *Crv,
				   CagdMType Mat);
CagdSrfStruct *CagdSrfMatTransform(const CagdSrfStruct *Srf,
				   CagdMType Mat);
void CagdCrvScale(CagdCrvStruct *Crv, const CagdRType *Scale);
void CagdCrvTransform(CagdCrvStruct *Crv,
		      const CagdRType *Translate,
		      CagdRType Scale);
void CagdSrfScale(CagdSrfStruct *Srf, const CagdRType *Scale);
void CagdSrfTransform(CagdSrfStruct *Srf,
		      const CagdRType *Translate,
		      CagdRType Scale);
CagdCrvStruct *CagdCrvUnitMaxCoef(CagdCrvStruct *Crv);
CagdSrfStruct *CagdSrfUnitMaxCoef(CagdSrfStruct *Srf);
CagdCrvStruct *CagdCrvRotateToXY(const CagdCrvStruct *Crv);
int CagdCrvRotateToXYMat(const CagdCrvStruct *Crv, IrtHmgnMatType Mat);

/******************************************************************************
* Routines to handle curves generically.				      *
******************************************************************************/
CagdRType *CagdCrvNodes(const CagdCrvStruct *Crv);
CagdRType CagdEstimateCrvCollinearity(const CagdCrvStruct *Crv);
void CagdCrvDomain(const CagdCrvStruct *Crv, CagdRType *TMin, CagdRType *TMax);
CagdCrvStruct *CagdCrvSetDomain(CagdCrvStruct *Crv,
				CagdRType TMin,
				CagdRType TMax);
CagdRType *CagdCrvEval(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *CagdCrvDerive(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvDeriveScalar(const CagdCrvStruct *Crv);
void CagdCrvScalarCrvSlopeBounds(const CagdCrvStruct *Crv,
				 CagdRType *MinSlope,
				 CagdRType *MaxSlope);
CagdCrvStruct *CagdCrvIntegrate(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrv2DNormalField(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvMoebiusTransform(const CagdCrvStruct *Crv, CagdRType c);
CagdCrvStruct *CagdCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *CagdCrvSubdivAtParams(const CagdCrvStruct *Crv,
				     const CagdPtStruct *Pts,
				     CagdRType Eps,
				     int *Proximity);
CagdCrvStruct *CagdCrvSubdivAtParams2(const CagdCrvStruct *CCrv,
				      const CagdPtStruct *Pts,
				      int Idx,
				      CagdRType Eps,
				      int *Proximity);
CagdCrvStruct *CagdCrvRegionFromCrv(const CagdCrvStruct *Crv,
				    CagdRType t1,
				    CagdRType t2);
CagdCrvStruct *CagdCrvRefineAtParams(const CagdCrvStruct *Crv,
				     CagdBType Replace,
				     CagdRType *t,
				     int n);
CagdVecStruct *CagdCrvTangent(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize);
CagdVecStruct *CagdCrvBiNormal(const CagdCrvStruct *Crv,
			       CagdRType t,
			       CagdBType Normalize);
CagdVecStruct *CagdCrvNormal(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize);
CagdVecStruct *CagdCrvNormalXY(const CagdCrvStruct *Crv,
			       CagdRType t,
			       CagdBType Normalize);
CagdPolylineStruct *CagdCrv2Polyline(const CagdCrvStruct *Crv,
				     int SamplesPerCurve,
				     CagdBType OptiLin);
CagdCrvStruct *CagdCrvReverse(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvReverseUV(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvSubdivAtAllC1Discont(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvDegreeRaise(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder);
CagdCrvStruct *CagdCrvDegreeReduce(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCrvCreateArc(const CagdPtStruct *Center,
				CagdRType Radius,
				CagdRType StartAngle,
				CagdRType EndAngle);
CagdCrvStruct *CagdCrvCreateArcCCW(const CagdPtStruct *Start,
				   const CagdPtStruct *Center,
				   const CagdPtStruct *End);
CagdCrvStruct *CagdCrvCreateArcCW(const CagdPtStruct *Start,
				  const CagdPtStruct *Center,
				  const CagdPtStruct *End);
CagdCrvStruct *CagdCreateConicCurve(CagdRType A,
				    CagdRType B,
				    CagdRType C,
				    CagdRType D,
				    CagdRType E,
				    CagdRType F,
				    CagdRType ZLevel,
				    CagdBType RationalEllipses);
CagdCrvStruct *CagdCreateConicCurve2(CagdRType A,
				     CagdRType B,
				     CagdRType C,
				     CagdRType D,
				     CagdRType E,
				     CagdRType F,
				     CagdRType ZLevel,
				     const CagdRType *PStartXY,
				     const CagdRType *PEndXY,
				     CagdBType RationalEllipses);
int CagdEllipse3Points(CagdPType Pt1,
		       CagdPType Pt2,
		       CagdPType Pt3,
		       CagdRType *A,
		       CagdRType *B,
		       CagdRType *C,
		       CagdRType *D,
		       CagdRType *E,
		       CagdRType *F);
int CagdEllipse4Points(CagdPType Pt1,
		       CagdPType Pt2,
		       CagdPType Pt3,
		       CagdPType Pt4,
		       CagdRType *A,
		       CagdRType *B,
		       CagdRType *C,
		       CagdRType *D,
		       CagdRType *E,
		       CagdRType *F);
int CagdEllipseOffset(CagdRType *A,
		      CagdRType *B,
		      CagdRType *C,
		      CagdRType *D,
		      CagdRType *E,
		      CagdRType *F,
		      CagdRType Offset);
int CagdConicMatTransform(CagdRType *A,
			  CagdRType *B,
			  CagdRType *C,
			  CagdRType *D,
			  CagdRType *E,
			  CagdRType *F,
			  CagdMType Mat);
int CagdQuadricMatTransform(CagdRType *A,
			    CagdRType *B,
			    CagdRType *C,
			    CagdRType *D,
			    CagdRType *E,
			    CagdRType *F,
			    CagdRType *G,
			    CagdRType *H,
			    CagdRType *I,
			    CagdRType *J,
			    CagdMType Mat);
int CagdConic2Quadric(CagdRType *A,
		      CagdRType *B,
		      CagdRType *C,
		      CagdRType *D,
		      CagdRType *E,
		      CagdRType *F,
		      CagdRType *G,
		      CagdRType *H,
		      CagdRType *I,
		      CagdRType *J);
CagdSrfStruct *CagdCreateQuadricSrf(CagdRType A,
				    CagdRType B,
				    CagdRType C,
				    CagdRType D,
				    CagdRType E,
				    CagdRType F,
				    CagdRType G,
				    CagdRType H,
				    CagdRType I,
				    CagdRType J);
CagdCrvStruct *CagdMergeCrvCrv(const CagdCrvStruct *Crv1,
			       const CagdCrvStruct *Crv2,
			       int InterpDiscont);
CagdCrvStruct *CagdMergeCrvList(const CagdCrvStruct *CrvList,
				int InterpDiscont);
CagdCrvStruct *CagdMergeCrvList2(CagdCrvStruct *CrvList,
				 IrtRType Tolerance);
CagdCrvStruct *CagdMergeCrvPt(const CagdCrvStruct *Crv,
			      const CagdPtStruct *Pt);
CagdCrvStruct *CagdMergePtCrv(const CagdPtStruct *Pt,
			      const CagdCrvStruct *Crv);
CagdCrvStruct *CagdMergePtPt(const CagdPtStruct *Pt1, const CagdPtStruct *Pt2);
CagdCrvStruct *CagdMergePtPt2(const CagdPType Pt1, const CagdPType Pt2);
CagdCrvStruct *CagdMergeUvUv(const CagdUVType UV1, const CagdUVType UV2);
CagdCrvStruct *CagdMergeCtlPtCtlPt(const CagdCtlPtStruct *Pt1,
				   const CagdCtlPtStruct *Pt2,
				   int MinDim);
CagdRType CagdCrvArcLenPoly(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdLimitCrvArcLen(const CagdCrvStruct *Crv, CagdRType MaxLen);
CagdPolylineStruct *CagdCrv2CtrlPoly(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdEditSingleCrvPt(CagdCrvStruct *Crv,
				   CagdCtlPtStruct *CtlPt,
				   int Index,
				   CagdBType Write);
CagdCrvStruct *CagdCrvDeletePoint(const CagdCrvStruct *Crv, int Index);
CagdCrvStruct *CagdCrvInsertPoint(const CagdCrvStruct *Crv,
				  int Index,
				  const CagdPType Pt);
CagdBType CagdMakeCrvsCompatible(CagdCrvStruct **Crv1,
				 CagdCrvStruct **Crv2,
				 CagdBType SameOrder,
				 CagdBType SameKV);
void CagdCrvBBox(const CagdCrvStruct *Crv, CagdBBoxStruct *BBox);
void CagdCrvListBBox(const CagdCrvStruct *Crvs, CagdBBoxStruct *BBox);
void CagdCrvMinMax(const CagdCrvStruct *Crv,
		   int Axis,
		   CagdRType *Min,
		   CagdRType *Max);
int CagdIsCrvInsideCirc(const CagdCrvStruct *Crv,
			const CagdRType Center[2],
			CagdRType Radius);
int CagdIsCrvInsideCH(const CagdCrvStruct *Crv,
		      const IrtE2PtStruct *CHPts,
		      int NumCHPts);
int CagdCrvEvalToPolyline(const CagdCrvStruct *Crv,
			  int FineNess,
			  CagdRType *Points[],
			  BspKnotAlphaCoeffStruct *A,
			  CagdBType OptiLin);
void CagdCrvFirstMoments(const CagdCrvStruct *Crv,
			 int n,
			 CagdPType Pt,
			 CagdVType Dir);
CagdCrvStruct *CagdCubicHermiteCrv(const CagdPType Pt1,
				   const CagdPType Pt2,
				   const CagdVType Dir1,
				   const CagdVType Dir2);
CagdRType CagdMatchDistNorm(const CagdVType T1,
			    const CagdVType T2,
			    const CagdVType P1,
			    const CagdVType P2);
CagdRType CagdMatchRuledNorm(const CagdVType T1,
			     const CagdVType T2,
			     const CagdVType P1,
			     const CagdVType P2);
CagdRType CagdMatchBisectorNorm(const CagdVType T1,
				const CagdVType T2,
				const CagdVType P1,
				const CagdVType P2);
CagdRType CagdMatchMorphNorm(const CagdVType T1,
			     const CagdVType T2,
			     const CagdVType P1,
			     const CagdVType P2);

void CagdMatchingFixVector(int *OldVec, CagdRType *NewVec, int Len);
void CagdMatchingFixCrv(CagdCrvStruct *Crv);
void CagdMatchingPolyTransform(CagdRType **Poly,
			       int Len,
			       CagdRType NewBegin,
			       CagdRType NewEnd);
void CagdMatchingVectorTransform(CagdRType *Vec,
				 CagdRType NewBegin,
				 CagdRType NewEnd,
				 int Len);
CagdCrvStruct *CagdMatchingTwoCurves(CagdCrvStruct *Crv1,
				     CagdCrvStruct *Crv2,
				     int Reduce,
				     int SampleSet,
				     int ReparamOrder,
				     int RotateFlag,
				     int AllowNegativeNorm,
				     int ReturnReparamFunc,
				     CagdMatchNormFuncType MatchNormFunc);
int CagdCrvTwoCrvsOrient(CagdCrvStruct *Crv1, CagdCrvStruct *Crv2, int n);
CagdBType CagdIsClosedCrv(const CagdCrvStruct *Crv);
CagdBType CagdIsZeroLenCrv(const CagdCrvStruct *Crv, CagdRType Eps);
CagdBType CagdCrvTanAngularSpan(const CagdCrvStruct *Crv,
				CagdVType ConeDir,
				CagdRType *AngularSpan);
CagdRType CagdDistCrvLine(const CagdCrvStruct *Crv, CagdLType Line);
CagdPtStruct *CagdCrvCrvInter(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2,
			      CagdRType Eps);
CagdPtStruct *CagdInsertInterPointInit(void);
void CagdInsertInterPoints(CagdRType t1, CagdRType t2, CagdRType Eps);
CagdCrvStruct *CagdCrvCrvInterArrangment(const CagdCrvStruct *ArngCrvs,
					 CagdBType SplitCrvs,
					 CagdRType Eps);
CagdBType CagdCrvsSame(const CagdCrvStruct *Crv1,
		       const CagdCrvStruct *Crv2,
		       CagdRType Eps);
CagdBType CagdCrvsSameUptoRigidScl2D(const CagdCrvStruct *Crv1,
				     const CagdCrvStruct *Crv2,
				     IrtPtType Trans,
				     CagdRType *Rot,
				     CagdRType *Scl,
				     CagdRType Eps);

/******************************************************************************
* Routines to handle surfaces generically.				      *
******************************************************************************/
CagdRType *CagdSrfNodes(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdRType CagdEstimateSrfPlanarity(const CagdSrfStruct *Srf);
void CagdSrfDomain(const CagdSrfStruct *Srf,
		   CagdRType *UMin,
		   CagdRType *UMax,
		   CagdRType *VMin,
		   CagdRType *VMax);
CagdSrfStruct *CagdSrfSetDomain(CagdSrfStruct *Srf,
				CagdRType UMin,
				CagdRType UMax,
				CagdRType VMin,
				CagdRType VMax);
CagdRType *CagdSrfEval(const CagdSrfStruct *Srf, CagdRType u, CagdRType v);
void CagdSrfEstimateCurveness(const CagdSrfStruct *Srf,
			      CagdRType *UCurveness,
			      CagdRType *VCurveness);
CagdPolygonStruct *CagdSrf2Polygons(const CagdSrfStruct *Srf,
				    int FineNess,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV);
CagdPolygonStruct *CagdSrf2PolygonsN(const CagdSrfStruct *Srf,
				     int Nu,
				     int Nv,
				     CagdBType ComputeNormals,
				     CagdBType FourPerFlat,
				     CagdBType ComputeUV);
CagdSrfErrorFuncType CagdSrf2PolyAdapSetErrFunc(CagdSrfErrorFuncType Func);
CagdSrfAdapAuxDataFuncType
	      CagdSrf2PolyAdapSetAuxDataFunc(CagdSrfAdapAuxDataFuncType Func);
CagdSrfAdapPolyGenFuncType
	      CagdSrf2PolyAdapSetPolyGenFunc(CagdSrfAdapPolyGenFuncType Func);
CagdRType CagdSrfAdap2PolyDefErrFunc(const CagdSrfStruct *Srf);
CagdRType CagdSrfIsLinearCtlMeshOneRowCol(const CagdSrfStruct *Srf,
					  int Idx,
					  CagdSrfDirType Dir);
CagdRType CagdSrfIsLinearCtlMesh(const CagdSrfStruct *Srf, CagdBType Interior);
CagdRType CagdSrfIsLinearBndryCtlMesh(const CagdSrfStruct *Srf);
CagdRType CagdSrfIsCoplanarCtlMesh(const CagdSrfStruct *Srf);
CagdRType CagdSrfIsLinearBndryCtlMesh(const CagdSrfStruct *Srf);
CagdPolygonStruct *CagdSrfAdap2Polygons(const CagdSrfStruct *Srf,
					CagdRType Tolerance,
					CagdBType ComputeNormals,
					CagdBType FourPerFlat,
					CagdBType ComputeUV,
					VoidPtr AuxSrfData);
CagdPolygonStruct *CagdSrf2PolygonsGenPolys(const CagdSrfStruct *Srf,
					    CagdBType FourPerFlat,
					    CagdRType *PtWeights,
					    CagdPtStruct *PtMesh,
					    CagdVecStruct *PtNrml,
					    CagdUVStruct *UVMesh,
					    int FineNessU,
					    int FineNessV);
CagdPolygonStruct *CagdSrfAdapRectPolyGen(const CagdSrfStruct *Srf,
					  CagdSrfPtStruct *SrfPtList,
					  const CagdSrfAdapRectStruct *Rect);
CagdRType *CagdSrfAdap2PolyEvalNrmlBlendedUV(const CagdRType *UV1,
					     const CagdRType *UV2,
					     const CagdRType *UV3);
int CagdSrf2PolygonStrip(int PolygonStrip);
int CagdSrf2PolygonFast(int PolygonFast);
int CagdSrf2PolygonMergeCoplanar(int MergeCoplanarPolys);
int Cagd2PolyClipPolysAtPoles(int ClipPolysAtPoles);
CagdPolylineStruct *CagdSrf2Polylines(const CagdSrfStruct *Srf,
				      int NumOfIsocurves[2],
				      int SamplesPerCurve);
CagdCrvStruct *CagdSrf2Curves(const CagdSrfStruct *Srf,
			      int NumOfIsocurves[2]);
void CagdEvaluateSurfaceVecField(CagdVType Vec,
				 CagdSrfStruct *VecFieldSrf,
				 CagdRType U,
				 CagdRType V);
CagdSrfStruct *CagdSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfDeriveScalar(const CagdSrfStruct *Srf,
				   CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfMoebiusTransform(const CagdSrfStruct *Srf,
				       CagdRType c,
				       CagdSrfDirType Dir);
CagdCrvStruct *CagdCrvFromSrf(const CagdSrfStruct *Srf,
			      CagdRType t,
			      CagdSrfDirType Dir);
CagdCrvStruct *CagdCrvFromMesh(const CagdSrfStruct *Srf,
			       int Index,
			       CagdSrfDirType Dir);
CagdCrvStruct **CagdBndryCrvsFromSrf(const CagdSrfStruct *Srf);
CagdCrvStruct *CagdBndryCrvFromSrf(const CagdSrfStruct *Srf,
				   CagdSrfBndryType Bndry);
void CagdCrvToMesh(const CagdCrvStruct *Crv,
		   int Index,
		   CagdSrfDirType Dir,
		   CagdSrfStruct *Srf);
CagdSrfStruct *CagdSrfSubdivAtParam(const CagdSrfStruct *Srf,
				    CagdRType t,
				    CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfRegionFromSrf(const CagdSrfStruct *Srf,
				    CagdRType t1,
				    CagdRType t2,
				    CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfRefineAtParams(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdBType Replace,
				     CagdRType *t,
				     int n);
CagdVecStruct *CagdSrfTangent(const CagdSrfStruct *Srf,
			      CagdRType u,
			      CagdRType v,
			      CagdSrfDirType Dir,
			      CagdBType Normalize);
CagdVecStruct *CagdSrfNormal(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdBType Normalize);
CagdUVType *CagdSrfUVDirOrthoE3(const CagdSrfStruct *Srf,
				const CagdUVType *UV,
				const CagdUVType *UVDir);
CagdSrfStruct *CagdSrfDegreeRaise(const CagdSrfStruct *Srf,
				  CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				   int NewUOrder,
				   int NewVOrder);
CagdSrfStruct *CagdSrfReverse(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdSrfReverseDir(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfReverse2(const CagdSrfStruct *Srf);
CagdPolylineStruct *CagdSrf2CtrlMesh(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdMergeSrfSrf(const CagdSrfStruct *CSrf1,
			       const CagdSrfStruct *CSrf2,
			       CagdSrfDirType Dir,
			       CagdBType SameEdge,
			       int InterpolateDiscont);
CagdSrfStruct *CagdMergeSrfList(const CagdSrfStruct *SrfList,
				CagdSrfDirType Dir,
				CagdBType SameEdge,
				int InterpolateDiscont);
CagdRType CagdSrfAvgArgLenMesh(const CagdSrfStruct *Srf,
			       CagdRType *AvgULen,
			       CagdRType *AvgVLen);
CagdSrfStruct *CagdExtrudeSrf(const CagdCrvStruct *Crv,
			      const CagdVecStruct *Vec);
CagdSrfStruct *CagdSurfaceRev(const CagdCrvStruct *Crv);
CagdSrfStruct *CagdSurfaceRevAxis(const CagdCrvStruct *Crv, CagdVType Axis);
CagdSrfStruct *CagdSurfaceRev2(const CagdCrvStruct *Crv,
			       CagdRType StartAngle,
			       CagdRType EndAngle);
CagdSrfStruct *CagdSurfaceRev2Axis(const CagdCrvStruct *Crv,
				   CagdRType StartAngle,
				   CagdRType EndAngle,
				   const CagdVType Axis);
CagdSrfStruct *CagdSurfaceRevPolynomialApprox(const CagdCrvStruct *Crv);
CagdSrfStruct *CagdSweepSrf(CagdCrvStruct *CrossSection,
			    CagdCrvStruct *Axis,
			    const CagdCrvStruct *ScalingCrv,
			    CagdRType Scale,
			    const VoidPtr Frame,
			    CagdBType FrameIsCrv);
CagdCrvStruct *CagdSweepAxisRefine(const CagdCrvStruct *Axis,
				   const CagdCrvStruct *ScalingCrv,
				   int RefLevel);
CagdBType CagdCrvOrientationFrame(CagdCrvStruct *Crv,
				  CagdRType CrntT,
				  CagdVecStruct *Tangent,
				  CagdVecStruct *Normal,
				  CagdBType FirstTime);
CagdSrfStruct *CagdBoolSumSrf(const CagdCrvStruct *CrvLeft,
			      const CagdCrvStruct *CrvRight,
			      const CagdCrvStruct *CrvTop,
			      const CagdCrvStruct *CrvBottom);
CagdSrfStruct *CagdOneBoolSumSrf(const CagdCrvStruct *BndryCrv);
CagdSrfStruct *CagdRuledSrf(const CagdCrvStruct *Crv1,
			    const CagdCrvStruct *Crv2,
			    int OtherOrder,
			    int OtherLen);
CagdSrfStruct *CagdBilinearSrf(const CagdPtStruct *Pt00,
			       const CagdPtStruct *Pt01,
			       const CagdPtStruct *Pt10,
			       const CagdPtStruct *Pt11);
CagdSrfStruct *CagdPromoteCrvToSrf(const CagdCrvStruct *Crv,
				   CagdSrfDirType Dir);
CagdSrfStruct *CagdSrfFromCrvs(const CagdCrvStruct *CrvList,
			       int OtherOrder,
			       CagdEndConditionType OtherEC,
			       IrtRType *OtherParamVals);
CagdRType *CagdSrfInterpolateCrvsChordLenParams(const CagdCrvStruct *CrvList);
CagdSrfStruct *CagdSrfInterpolateCrvs(const CagdCrvStruct *CrvList,
				      int OtherOrder,
				      CagdEndConditionType OtherEC,
				      CagdParametrizationType OtherParam,
				      IrtRType *OtherParamVals);
CagdBType CagdMakeSrfsCompatible(CagdSrfStruct **Srf1,
				 CagdSrfStruct **Srf2,
				 CagdBType SameUOrder,
				 CagdBType SameVOrder,
				 CagdBType SameUKV,
				 CagdBType SameVKV);
CagdSrfStruct *CagdEditSingleSrfPt(CagdSrfStruct *Srf,
				   CagdCtlPtStruct *CtlPt,
				   int UIndex,
				   int VIndex,
				   CagdBType Write);
void CagdSrfBBox(const CagdSrfStruct *Srf, CagdBBoxStruct *BBox);
void CagdSrfListBBox(const CagdSrfStruct *Srfs, CagdBBoxStruct *BBox);
void CagdSrfMinMax(const CagdSrfStruct *Srf,
		   int Axis,
		   CagdRType *Min,
		   CagdRType *Max);
CagdRType CagdPolyApproxMaxErr(const CagdSrfStruct *Srf,
			       const CagdPolygonStruct *Polys);
CagdRType *CagdPolyApproxErrs(const CagdSrfStruct *Srf,
			      const CagdPolygonStruct *Polys);
int CagdPolyApproxErrEstimate(const CagdPolyErrEstimateType Method,
			      int Samples);
CagdSrfStruct *CagdCubicHermiteSrf(const CagdCrvStruct *CPos1Crv,
				   const CagdCrvStruct *CPos2Crv,
				   const CagdCrvStruct *CDir1Crv,
				   const CagdCrvStruct *CDir2Crv);
CagdBType CagdIsClosedSrf(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdBType CagdIsZeroLenSrfBndry(const CagdSrfStruct *Srf,
				CagdSrfBndryType Bdnry,
				CagdRType Eps);
CagdBType CagdSrfsSame(const CagdSrfStruct *Srf1,
		       const CagdSrfStruct *Srf2,
		       CagdRType Eps);
CagdCrvStruct *CagdCrvUpdateLength(CagdCrvStruct *Crv, int NewLength);
CagdSrfStruct *CagdSrfUpdateLength(CagdSrfStruct *Srf,
				   int NewLength,
				   CagdSrfDirType Dir);
CagdBType CagdSrfsSameUptoRigidScl2D(const CagdSrfStruct *Srf1,
				     const CagdSrfStruct *Srf2,
				     IrtPtType Trans,
				     CagdRType *Rot,
				     CagdRType *Scl,
				     CagdRType Eps);
void CagdSrfEffiNrmlPrelude(const CagdSrfStruct *Srf);
CagdVecStruct *CagdSrfEffiNrmlEval(CagdRType u,
				   CagdRType v,
				   CagdBType Normalize);
void CagdSrfEffiNrmlPostlude(void);

/******************************************************************************
* Routines to handle Power basis curves.				      *
******************************************************************************/
CagdCrvStruct *PwrCrvNew(int Length, CagdPointType PType);
CagdRType *PwrCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *PwrCrvDerive(const CagdCrvStruct *Crv);
CagdCrvStruct *PwrCrvDeriveScalar(const CagdCrvStruct *Crv);
CagdCrvStruct *PwrCrvIntegrate(const CagdCrvStruct *Crv);
CagdCrvStruct *PwrCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder);
CagdCrvStruct *PwrCrvDegreeRaise(const CagdCrvStruct *Crv);

/******************************************************************************
* Routines to handle Power basis surfaces.				      *
******************************************************************************/
CagdSrfStruct *PwrSrfNew(int ULength, int VLength, CagdPointType PType);
CagdSrfStruct *PwrSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder);
CagdSrfStruct *PwrSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir);

/******************************************************************************
* Routines to handle Bezier curves.					      *
******************************************************************************/
CagdCrvStruct *BzrCrvNew(int Length, CagdPointType PType);
CagdRType BzrCrvEvalBasisFunc(int i, int k, CagdRType t);
CagdRType *BzrCrvEvalBasisFuncs(int k, CagdRType t);
CagdRType BzrCrvEvalVecAtParam(const CagdRType *Vec,
			       int VecInc,
			       int Order,
			       CagdRType t);
CagdRType *BzrCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t);
void BzrCrvSetCache(int FineNess, CagdBType EnableCache);
void BzrCrvEvalToPolyline(const CagdCrvStruct *Crv,
			  int FineNess,
			  CagdRType *Points[]);
CagdCrvStruct *BzrCrvCreateArc(const CagdPtStruct *Start,
			       const CagdPtStruct *Center,
			       const CagdPtStruct *End);
void BzrCrvSubdivCtlPoly(CagdRType * const *Points,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int Length,
			 CagdPointType PType,
			 CagdRType t);
void BzrCrvSubdivCtlPolyStep(CagdRType * const *Points,
			     CagdRType **LPoints,
			     CagdRType **RPoints,
			     int Length,
			     CagdPointType PType,
			     CagdRType t,
			     int Step);
CagdCrvStruct *BzrCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *BzrCrvDegreeRaise(const CagdCrvStruct *Crv);
CagdCrvStruct *BzrCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder);
CagdCrvStruct *BzrCrvDegreeReduce(const CagdCrvStruct *Crv);
CagdCrvStruct *BzrCrvDerive(const CagdCrvStruct *Crv);
CagdCrvStruct *BzrCrvDeriveScalar(const CagdCrvStruct *Crv);
CagdCrvStruct *BzrCrvIntegrate(const CagdCrvStruct *Crv);
CagdCrvStruct *BzrCrvMoebiusTransform(const CagdCrvStruct *Crv, CagdRType c);
CagdVecStruct *BzrCrvTangent(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize);
CagdVecStruct *BzrCrvBiNormal(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize);
CagdVecStruct *BzrCrvNormal(const CagdCrvStruct *Crv,
			    CagdRType t,
			    CagdBType Normalize);
CagdPolylineStruct *BzrCrv2Polyline(const CagdCrvStruct *Crv,
				    int SamplesPerCurve);
int BzrCrvInterp2(IrtRType *Result, const IrtRType *Input, int Size);

/******************************************************************************
* Routines to handle Bezier surfaces.					      *
******************************************************************************/
CagdSrfStruct *BzrSrfNew(int ULength, int VLength, CagdPointType PType);
CagdRType *BzrSrfEvalAtParam(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v);
CagdCrvStruct *BzrSrfCrvFromSrf(const CagdSrfStruct *Srf,
				CagdRType t,
				CagdSrfDirType Dir);
CagdCrvStruct *BzrSrfCrvFromMesh(const CagdSrfStruct *Srf,
				 int Index,
				 CagdSrfDirType Dir);
void BzrSrfSubdivCtlMesh(CagdRType * const *Points,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int ULength,
			 int VLength,
			 CagdPointType PType,
			 CagdRType t,
			 CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfSubdivAtParam(const CagdSrfStruct *Srf,
				   CagdRType t,
				   CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder);
CagdSrfStruct *BzrSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfDeriveScalar(const CagdSrfStruct *Srf,
				  CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BzrSrfMoebiusTransform(const CagdSrfStruct *Srf,
				      CagdRType c,
				      CagdSrfDirType Dir);
CagdVecStruct *BzrSrfTangent(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdSrfDirType Dir,
			     CagdBType Normalize);
CagdVecStruct *BzrSrfNormal(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    CagdBType Normalize);
CagdVecStruct *BzrSrfMeshNormals(const CagdSrfStruct *Srf,
				 int UFineNess,
				 int VFineNess);
int BzrSrf2PolygonsSamples(const CagdSrfStruct *Srf,
			   int FineNess,
			   CagdBType ComputeNormals,
			   CagdBType ComputeUV,
			   CagdRType **PtWeights,
			   CagdPtStruct **PtMesh,
			   CagdVecStruct **PtNrml,
			   CagdUVStruct **UVMesh,
			   int *FineNessU,
			   int *FineNessV);
int BzrSrf2PolygonsSamplesNuNv(const CagdSrfStruct *Srf,
			       int Nu,
			       int Nv,
			       CagdBType ComputeNormals,
			       CagdBType ComputeUV,
			       CagdRType **PtWeights,
			       CagdPtStruct **PtMesh,
			       CagdVecStruct **PtNrml,
			       CagdUVStruct **UVMesh);
CagdPolygonStruct *BzrSrf2Polygons(const CagdSrfStruct *Srf,
				   int FineNess,
				   CagdBType ComputeNormals,
				   CagdBType FourPerFlat,
				   CagdBType ComputeUV);
CagdPolygonStruct *BzrSrf2PolygonsN(const CagdSrfStruct *Srf,
				    int Nu,
				    int Nv,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV);
CagdPolylineStruct *BzrSrf2Polylines(const CagdSrfStruct *Srf,
				     int NumOfIsocurves[2],
				     int SamplesPerCurve);
CagdCrvStruct *BzrSrf2Curves(const CagdSrfStruct *Srf, int NumOfIsocurves[2]);

/******************************************************************************
* Routines to handle Bspline knot vectors.				      *
******************************************************************************/
CagdBType BspCrvHasBezierKV(const CagdCrvStruct *Crv);
CagdBType BspSrfHasBezierKVs(const CagdSrfStruct *Srf);
CagdBType BspKnotHasBezierKV(const CagdRType *KnotVector, int Len, int Order);
CagdBType BspCrvHasOpenEC(const CagdCrvStruct *Crv);
CagdBType BspSrfHasOpenEC(const CagdSrfStruct *Srf);
CagdBType BspSrfHasOpenECDir(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdBType BspKnotHasOpenEC(const CagdRType *KnotVector, int Len, int Order);
CagdBType BspKnotParamInDomain(const CagdRType *KnotVector,
			       int Len,
			       int Order,
			       CagdBType Periodic,
			       CagdRType t);
int BspKnotLastIndexLE(const CagdRType *KnotVector, int Len, CagdRType t);
int BspKnotLastIndexL(const CagdRType *KnotVector, int Len, CagdRType t);
int BspKnotFirstIndexG(const CagdRType *KnotVector, int Len, CagdRType t);
CagdRType *BspKnotUniformPeriodic(int Len, int Order, CagdRType *KnotVector);
CagdRType *BspKnotUniformFloat(int Len, int Order, CagdRType *KnotVector);
CagdRType *BspKnotUniformOpen(int Len, int Order, CagdRType *KnotVector);
CagdRType *BspKnotDiscontUniformOpen(int Len, int Order, CagdRType *KnotVector);
CagdEndConditionType BspIsKnotUniform(int Len,
				      int Order,
				      const CagdRType *KnotVector);
CagdEndConditionType BspIsKnotDiscontUniform(int Len,
					     int Order,
					     const CagdRType *KnotVector);
CagdRType *BspKnotDegreeRaisedKV(const CagdRType *KV,
				 int Len,
				 int Order,
				 int NewOrder,
				 int *NewLen);
CagdRType *BspKnotSubtrTwo(const CagdRType *KnotVector1,
			   int Len1,
			   const CagdRType *KnotVector2,
			   int Len2,
			   int *NewLen);
CagdRType *BspKnotMergeTwo(const CagdRType *KnotVector1,
			   int Len1,
			   const CagdRType *KnotVector2,
			   int Len2,
			   int Mult,
			   int *NewLen);
CagdRType *BspKnotContinuityMergeTwo(const CagdRType *KnotVector1,
				     int Len1,
				     int Order1,
				     const CagdRType *KnotVector2,
				     int Len2,
				     int Order2,
				     int ResOrder,
				     int *NewLen);
CagdRType *BspKnotDoubleKnots(const CagdRType *KnotVector, 
			      int *Len,
			      int Order);
CagdRType *BspKnotAverage(const CagdRType *KnotVector, int Len, int Ave);
CagdRType *BspKnotNodes(const CagdRType *KnotVector, int Len, int Order);
CagdRType *BspKnotPeriodicNodes(const CagdRType *KnotVector,
				int Len,
				int Order);
CagdRType BspCrvMaxCoefParam(const CagdCrvStruct *Crv,
			     int Axis,
			     CagdRType *MaxVal);
CagdRType *BspSrfMaxCoefParam(const CagdSrfStruct *Srf,
			      int Axis,
			      CagdRType *MaxVal);
CagdRType *BspKnotPrepEquallySpaced(int n, CagdRType Tmin, CagdRType Tmax);
CagdRType *BspKnotReverse(const CagdRType *KnotVector, int Len);
void BspKnotScale(CagdRType *KnotVector, int Len, CagdRType Scale);
void BspKnotAffineTrans(CagdRType *KnotVector,
			int Len,
			CagdRType Translate,
			CagdRType Scale);
void BspKnotAffineTrans2(CagdRType *KnotVector,
			 int Len,
			 CagdRType MinVal,
			 CagdRType MaxVal);
void BspKnotAffineTransOrder(CagdRType *KnotVector,
			     int Order,
			     int Len,
			     CagdRType Translate,
			     CagdRType Scale);
void BspKnotAffineTransOrder2(CagdRType *KnotVector,
			      int Order,
			      int Len,
			      CagdRType MinVal,
			      CagdRType MaxVal);
CagdRType *BspKnotCopy(CagdRType *DstKV, const CagdRType *SrcKV, int Len);
BspKnotAlphaCoeffStruct *BspKnotEvalAlphaCoef(int k,
					      CagdRType *KVT,
					      int LengthKVT,
					      CagdRType *KVt,
					      int LengthKVt,
					      int Periodic);
BspKnotAlphaCoeffStruct *BspKnotEvalAlphaCoefMerge(int k,
						   CagdRType *KVT,
						   int LengthKVT,
						   CagdRType *NewKV,
						   int LengthNewKV,
						   int Periodic);
BspKnotAlphaCoeffStruct *BspKnotCopyAlphaCoef(const BspKnotAlphaCoeffStruct
					                                  *A);
void BspKnotFreeAlphaCoef(BspKnotAlphaCoeffStruct *A);
void BspKnotAlphaLoopBlendNotPeriodic(const BspKnotAlphaCoeffStruct *A,
				      int IMin,
				      int IMax,
				      const CagdRType *OrigPts,
				      CagdRType *RefPts);
void BspKnotAlphaLoopBlendPeriodic(const BspKnotAlphaCoeffStruct *A,
				   int IMin,
				   int IMax,
				   const CagdRType *OrigPts,
				   int OrigLen,  
				   CagdRType *RefPts);
void BspKnotAlphaLoopBlendStep(const BspKnotAlphaCoeffStruct *A,
			       int IMin,
			       int IMax,
			       const CagdRType *OrigPts,
			       int OrigPtsStep,
			       int OrigLen,  
			       CagdRType *RefPts,
			       int RefPtsStep);
CagdRType *BspKnotInsertOne(const CagdRType *KnotVector,
			    int Order,
			    int Len,
			    CagdRType t);
CagdRType *BspKnotInsertMult(const CagdRType *KnotVector,
			     int Order,
			     int *Len,
			     CagdRType t,
			     int Mult);
int BspKnotFindMult(const CagdRType *KnotVector,
		    int Order,
		    int Len,
		    CagdRType t);
int BspKnotsMultiplicityVector(const CagdRType *KnotVector,
			       int Len,
			       CagdRType *KnotValues,
			       int *KnotMultiplicities);
CagdBType BspKnotC0Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t);
CagdBType BspKnotC1Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t);
CagdBType BspKnotC2Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t);
CagdRType *BspKnotAllC0Discont(const CagdRType *KnotVector,
			       int Order,
			       int Length,
			       int *n);
CagdRType *BspKnotAllC1Discont(const CagdRType *KnotVector,
			       int Order,
			       int Length,
			       int *n);
CagdRType *BspKnotParamValues(CagdRType PMin,
			      CagdRType PMax,
			      int NumSamples,
			      CagdRType *C1Disconts,
			      int NumC1Disconts);
int BspKnotMakeRobustKV(CagdRType *KV, int Len);
CagdBType BspKnotVectorsSame(const CagdRType *KV1,
			     const CagdRType *KV2,
			     int Len,
			     CagdRType Eps);
void BspKnotVerifyPeriodicKV(CagdRType *KV, int Order, int Len);
int BspKnotVerifyKVValidity(CagdRType *KV, int Order, int Len, CagdRType Tol);
int BspVecSpreadEqualItems(CagdRType *Vec, int Len, CagdRType MinDist);

/******************************************************************************
* Routines to handle Bspline curves.					      *
******************************************************************************/
CagdCrvStruct *BspCrvNew(int Length, int Order, CagdPointType PType);
CagdCrvStruct *BspPeriodicCrvNew(int Length,
				 int Order,
				 CagdBType Periodic,
				 CagdPointType PType);
void BspCrvDomain(const CagdCrvStruct *Crv, CagdRType *TMin, CagdRType *TMax);

CagdRType *BspCrvCoxDeBoorBasis(const CagdRType *KnotVector,
				int Order,
				int Len,
				CagdBType Periodic,
				CagdRType t,
				int *IndexFirst);
int BspCrvCoxDeBoorIndexFirst(const CagdRType *KnotVector,
			      int Order,
			      int Len,
			      CagdRType t);
CagdRType *BspCrvEvalCoxDeBoor(const CagdCrvStruct *Crv, CagdRType t);
CagdBspBasisFuncEvalStruct *BspBasisFuncMultEval(const CagdRType *KnotVector,
						 int KVLength,
						 int Order,
						 CagdBType Periodic,
						 CagdRType *Params,
						 int NumOfParams,
						 CagdBspBasisFuncMultEvalType
								     EvalType);
void BspBasisFuncMultEvalPrint(const CagdBspBasisFuncEvalStruct *Evals,
			       int Order,
			       CagdRType *Params,
			       int NumOfParams);
void BspBasisFuncMultEvalFree(CagdBspBasisFuncEvalStruct *Evals,
			      int NumOfEvals);
CagdRType BspCrvEvalVecAtParam(const CagdRType *Vec,
			       int VecInc,
			       const CagdRType *KnotVector,
			       int Order,
			       int Len,
			       CagdBType Periodic,
			       CagdRType t);
CagdRType *BspCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *BspCrvCreateCircle(const CagdPtStruct *Center,
				  CagdRType Radius);
CagdCrvStruct *BspCrvCreateUnitCircle(void);
CagdCrvStruct *BspCrvCreatePCircle(const CagdPtStruct *Center,
				   CagdRType Radius);
CagdCrvStruct *BspCrvCreateUnitPCircle(void);
CagdCrvStruct *BspCrvCreateApproxSpiral(CagdRType NumOfLoops,
					CagdRType Pitch,
					int Sampling,
					int CtlPtsPerLoop);
CagdCrvStruct *BspCrvCreateApproxHelix(CagdRType NumOfLoops,
				       CagdRType Pitch,
				       CagdRType Radius,
				       int Sampling,
				       int CtlPtsPerLoop);
CagdCrvStruct *BspCrvCreateApproxSine(CagdRType NumOfCycles,
				      int Sampling,
				      int CtlPtsPerCycle);
CagdCrvStruct *BspCrvKnotInsert(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *BspCrvKnotInsertNSame(const CagdCrvStruct *Crv,
				     CagdRType t,
				     int n);
CagdCrvStruct *BspCrvKnotInsertNDiff(const CagdCrvStruct *Crv,
				     CagdBType Replace,
				     CagdRType *t,
				     int n);
void BspCrvSubdivCtlPoly(const CagdCrvStruct *Crv,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int LLength,
			 int RLength,
			 CagdRType t,
			 int Mult);
CagdCrvStruct *BspCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t);
CagdCrvStruct *BspCrvOpenEnd(const CagdCrvStruct *Crv);
CagdBType BspCrvKnotC0Discont(const CagdCrvStruct *Crv, CagdRType *t);
CagdBType BspCrvKnotC1Discont(const CagdCrvStruct *Crv, CagdRType *t);
CagdBType BspCrvKnotC2Discont(const CagdCrvStruct *Crv, CagdRType *t);
CagdBType BspCrvMeshC1Continuous(const CagdCrvStruct *Crv, int Idx);
CagdCrvStruct *BspCrvDegreeRaise(const CagdCrvStruct *Crv);
CagdCrvStruct *BspCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder);
CagdCrvStruct *BspCrvDerive(const CagdCrvStruct *Crv);
CagdCrvStruct *BspCrvDeriveScalar(const CagdCrvStruct *Crv);
CagdCrvStruct *BspCrvIntegrate(const CagdCrvStruct *Crv);
CagdCrvStruct *BspCrvMoebiusTransform(const CagdCrvStruct *Crv, CagdRType c);
CagdVecStruct *BspCrvTangent(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize);
CagdVecStruct *BspCrvBiNormal(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize);
CagdVecStruct *BspCrvNormal(const CagdCrvStruct *Crv,
			    CagdRType t,
			    CagdBType Normalize);
CagdPolylineStruct *BspCrv2Polyline(const CagdCrvStruct *Crv,
				    int SamplesPerCurve,
				    BspKnotAlphaCoeffStruct *A,
				    CagdBType OptiLin);
CagdCrvStruct *BspCrvInterpPts(const CagdPtStruct *PtList,
			       int Order,
			       int CrvSize,
			       CagdParametrizationType ParamType,
			       CagdBType Periodic);
CagdCrvStruct *BspCrvInterpPts2(const CagdCtlPtStruct *PtList,
				int Order,
				int CrvSize,
				CagdParametrizationType ParamType,
				CagdBType Periodic);
void BspCrvInterpBuildKVs(const CagdCtlPtStruct *PtList,
			  int Order,
			  int CrvSize,
			  CagdParametrizationType ParamType,
			  CagdBType Periodic,
			  CagdRType **RetPtKnots,
			  CagdRType **RetKV);
CagdCrvStruct *BspCrvInterpolate(const CagdCtlPtStruct *PtList,
				 const CagdRType *Params,
				 const CagdRType *KV,
				 int Length,
				 int Order,
				 CagdBType Periodic);
CagdCrvStruct *BspCrvFitLstSqr(const CagdCrvStruct *Crv,
			       int Order,
			       int Size, 
			       CagdParametrizationType ParamType,
			       int EndPtInterp,
			       int EvalPts,
			       CagdRType *Err);
CagdRType *BspPtSamplesToKV(const CagdRType *PtsSamples,
			    int NumPts,
			    int CrvOrder,
			    int CrvLength);
CagdRType BspCrvInterpPtsError(const CagdCrvStruct *Crv,
			       const CagdPtStruct *PtList,
			       CagdParametrizationType ParamType,
			       CagdBType Periodic);
CagdCrvStruct *BspMakeReparamCurve(const CagdPtStruct *PtsList,
				   int Order,
				   int DegOfFreedom);
CagdRType CagdLineFitToPts(CagdPtStruct *PtList,
			   CagdVType LineDir,
			   CagdPType LinePos);
CagdRType CagdPlaneFitToPts(CagdPtStruct *PtList,
			    IrtPlnType Pln,
			    IrtVecType MVec,
			    IrtPtType Cntr,
			    IrtRType *CN);
CagdRType CagdPlaneFitToPts2(CagdRType **Points,
			     int NumPts,
			     CagdPointType PType,
			     IrtPlnType Pln,
			     IrtVecType MVec,
			     IrtPtType Cntr,
			     IrtRType *CN);
CagdRType CagdPlaneFitToPts3(CagdPType *Points,
			     int NumPts,
			     IrtPlnType Pln,
			     IrtVecType MVec,
			     IrtPtType Cntr,
			     IrtRType *CN);
void BspReparameterizeCrv(CagdCrvStruct *Crv,
			  CagdParametrizationType ParamType);

/******************************************************************************
* Routines to handle Bspline surfaces.					      *
******************************************************************************/
CagdSrfStruct *BspSrfNew(int ULength,
			 int VLength,
			 int UOrder,
			 int VOrder,
			 CagdPointType PType);
CagdSrfStruct *BspPeriodicSrfNew(int ULength,
				 int VLength,
				 int UOrder,
				 int VOrder,
				 CagdBType UPeriodic,
				 CagdBType VPeriodic,
				 CagdPointType PType);
void BspSrfDomain(const CagdSrfStruct *Srf,
		  CagdRType *UMin,
		  CagdRType *UMax,
		  CagdRType *VMin,
		  CagdRType *VMax);
CagdRType *BspSrfEvalAtParam(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v);
CagdRType *BspSrfEvalAtParam2(const CagdSrfStruct *Srf,
			      CagdRType u,
			      CagdRType v);
CagdCrvStruct *BspSrfCrvFromSrf(const CagdSrfStruct *Srf,
				CagdRType t,
				CagdSrfDirType Dir);
CagdCrvStruct *BspSrfC1DiscontCrvs(const CagdSrfStruct *Srf);
CagdBType BspSrfHasC1Discont(const CagdSrfStruct *Srf);
CagdBType BspSrfIsC1DiscontAt(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType t);
CagdCrvStruct *BspSrfCrvFromMesh(const CagdSrfStruct *Srf,
				 int Index,
				 CagdSrfDirType Dir);
CagdSrfStruct *BspSrfKnotInsert(const CagdSrfStruct *Srf,
				CagdSrfDirType Dir,
				CagdRType t);
CagdSrfStruct *BspSrfKnotInsertNSame(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdRType t, int n);
CagdSrfStruct *BspSrfKnotInsertNDiff(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     int Replace,
				     CagdRType *t,
				     int n);
CagdSrfStruct *BspSrfSubdivAtParam(const CagdSrfStruct *Srf,
				   CagdRType t,
				   CagdSrfDirType Dir);
CagdSrfStruct *BspSrfOpenEnd(const CagdSrfStruct *Srf);
CagdBType BspSrfKnotC0Discont(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType *t);
CagdBType BspSrfKnotC1Discont(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType *t);
CagdBType BspSrfMeshC1Continuous(const CagdSrfStruct *Srf,
				 CagdSrfDirType Dir,
				 int Idx);
CagdSrfStruct *BspSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BspSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder);
CagdSrfStruct *BspSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BspSrfDeriveScalar(const CagdSrfStruct *Srf,
				  CagdSrfDirType Dir);
CagdSrfStruct *BspSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir);
CagdSrfStruct *BspSrfMoebiusTransform(const CagdSrfStruct *Srf,
				      CagdRType c,
				      CagdSrfDirType Dir);
CagdVecStruct *BspSrfTangent(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdSrfDirType Dir,
			     CagdBType Normalize);
CagdVecStruct *BspSrfNormal(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    CagdBType Normalize);
CagdVecStruct *BspSrfMeshNormals(const CagdSrfStruct *Srf,
				 int UFineNess,
				 int VFineNess);
CagdSrfErrorFuncType BspSrf2PolygonSetErrFunc(CagdSrfErrorFuncType Func);
CagdPolygonStruct *BspSrf2Polygons(const CagdSrfStruct *Srf,
				   int FineNess,
				   CagdBType ComputeNormals,
				   CagdBType FourPerFlat,
				   CagdBType ComputeUV);
CagdPolygonStruct *BspSrf2PolygonsN(const CagdSrfStruct *Srf,
				    int Nu,
				    int Nv,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV);
int BspSrf2PolygonsSamplesNuNv(const CagdSrfStruct *Srf,
			       int Nu,
			       int Nv,
			       CagdBType ComputeNormals,
			       CagdBType ComputeUV,
			       CagdRType **PtWeights,
			       CagdPtStruct **PtMesh,
			       CagdVecStruct **PtNrml,
			       CagdUVStruct **UVMesh);
int BspC1Srf2PolygonsSamples(const CagdSrfStruct *Srf,
			     int FineNess,
			     CagdBType ComputeNormals,
			     CagdBType ComputeUV,
			     CagdRType **PtWeights,
			     CagdPtStruct **PtMesh,
			     CagdVecStruct **PtNrml,
			     CagdUVStruct **UVMesh,
			     int *FineNessU,
			     int *FineNessV);
CagdPolylineStruct *BspSrf2Polylines(const CagdSrfStruct *Srf,
				     int NumOfIsocurves[2],
				     int SamplesPerCurve);
CagdCrvStruct *BspSrf2Curves(const CagdSrfStruct *Srf, int NumOfIsocurves[2]);
CagdSrfStruct *BspSrfInterpPts(const CagdPtStruct **PtList,
			       int UOrder,
			       int VOrder,
			       int SrfUSize,
			       int SrfVSize,
			       CagdParametrizationType ParamType);
CagdSrfStruct *BspSrfInterpolate(const CagdCtlPtStruct *PtList,
				 int NumUPts,
				 int NumVPts,
				 const CagdRType *UParams,
				 const CagdRType *VParams,
				 const CagdRType *UKV,
				 const CagdRType *VKV,
				 int ULength,
				 int VLength,
				 int UOrder,
				 int VOrder);
CagdSrfStruct *BspSrfFitLstSqr(const CagdSrfStruct *Srf,
			       int UOrder,
			       int VOrder,
			       int USize,
			       int VSize, 
			       CagdParametrizationType ParamType,
			       CagdRType *Err);
CagdSrfStruct *BspSrfInterpScatPts(const CagdCtlPtStruct *PtList,
				   int UOrder,
				   int VOrder,
				   int USize,
				   int VSize,
				   CagdRType *UKV,
				   CagdRType *VKV);
CagdSrfStruct *BspSrfInterpScatPts2(const CagdCtlPtStruct *PtList,
				    int UOrder,
				    int VOrder,
				    int USize,
				    int VSize,
				    CagdRType *UKV,
				    CagdRType *VKV,
				    CagdRType *MatrixCondition);
void BspReparameterizeSrf(CagdSrfStruct *Srf,
			  CagdSrfDirType Dir,
			  CagdParametrizationType ParamType);

/******************************************************************************
* Routines to construct primitive objects.				      *
******************************************************************************/
CagdCrvStruct *CagdPrimRectangleCrv(CagdRType MinX,
				    CagdRType MinY,
				    CagdRType MaxX,
				    CagdRType MaxY,
				    CagdRType ZLevel);
CagdSrfStruct *CagdPrimPlaneSrf(CagdRType MinX,
				CagdRType MinY,
				CagdRType MaxX,
				CagdRType MaxY,
				CagdRType ZLevel);
CagdSrfStruct *CagdPrimPlaneSrfOrderLen(CagdRType MinX,
					CagdRType MinY,
					CagdRType MaxX,
					CagdRType MaxY,
					CagdRType ZLevel,
					int Order,
					int Len);
CagdSrfStruct *CagdPrimPlaneXZSrf(CagdRType MinX,
				  CagdRType MinZ,
				  CagdRType MaxX,
				  CagdRType MaxZ,
				  CagdRType YLevel);
CagdSrfStruct *CagdPrimPlaneYZSrf(CagdRType MinY,
				  CagdRType MinZ,
				  CagdRType MaxY,
				  CagdRType MaxZ,
				  CagdRType XLevel);
CagdSrfStruct *CagdPrimBoxSrf(CagdRType MinX,
			      CagdRType MinY,
			      CagdRType MinZ,
			      CagdRType MaxX,
			      CagdRType MaxY,
			      CagdRType MaxZ);
CagdSrfStruct *CagdPrimSphereSrf(const CagdVType Center,
				 CagdRType Radius,
				 CagdBType Rational);
CagdSrfStruct *CagdPrimCubeSphereSrf(const CagdVType Center,
				     CagdRType Radius);
CagdSrfStruct *CagdPrimTorusSrf(const CagdVType Center,
				CagdRType MajorRadius,
				CagdRType MinorRadius,
				CagdBType Rational);
CagdSrfStruct *CagdPrimCone2Srf(const CagdVType Center,
				CagdRType MajorRadius,
				CagdRType MinorRadius,
				CagdRType Height,
				CagdBType Rational,
				CagdPrimCapsType Caps);
CagdSrfStruct *CagdPrimConeSrf(const CagdVType Center,
			       CagdRType Radius,
			       CagdRType Height,
			       CagdBType Rational,
			       CagdPrimCapsType Caps);
CagdSrfStruct *CagdPrimCylinderSrf(const CagdVType Center,
				   CagdRType Radius,
				   CagdRType Height,
				   CagdBType Rational,
				   CagdPrimCapsType Caps);

/******************************************************************************
* Routines to handle basis function conversions.			      *
******************************************************************************/
CagdCrvStruct *CagdCnvrtPwr2BzrCrv(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCnvrtBzr2PwrCrv(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCnvrtBsp2BzrCrv(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCnvrtBzr2BspCrv(const CagdCrvStruct *Crv);

CagdSrfStruct *CagdCnvrtPwr2BzrSrf(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdCnvrtBzr2PwrSrf(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdCnvrtBzr2BspSrf(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdCnvrtBsp2BzrSrf(const CagdSrfStruct *CSrf);

CagdCrvStruct *CagdCnvrtPeriodic2FloatCrv(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCnvrtFloat2OpenCrv(const CagdCrvStruct *Crv);
CagdCrvStruct *CagdCnvrtBsp2OpenCrv(const CagdCrvStruct *Crv);
CagdSrfStruct *CagdCnvrtPeriodic2FloatSrf(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdCnvrtFloat2OpenSrf(const CagdSrfStruct *Srf);
CagdSrfStruct *CagdCnvrtBsp2OpenSrf(const CagdSrfStruct *Srf);

CagdCtlPtStruct *CagdCnvrtCrvToCtlPts(const CagdCrvStruct *Crv);

CagdPolylineStruct *CagdCnvrtPtList2Polyline(const CagdPtStruct *Pts,
					     CagdPolylineStruct **Params);
CagdPtStruct *CagdCnvrtPolyline2PtList(const CagdPolylineStruct *Poly);
CagdCrvStruct *CagdCnvrtPolyline2LinBspCrv(const CagdPolylineStruct *Poly);
CagdPolylineStruct *CagdCnvrtLinBspCrv2Polyline(const CagdCrvStruct *Crv);

/******************************************************************************
* Routines to handle adaptive forward differencing basis functions.	      *
******************************************************************************/
void AfdCnvrtCubicBzrToAfd(CagdRType Coef[4]);
void AfdApplyEStep(CagdRType Coef[4]);
void AfdApplyLn(CagdRType Coef[4], int n);
void AfdApplyLStep(CagdRType Coef[4]);
void AfdApplyAntiLStep(CagdRType Coef[4]);
void AfdComputePolyline(CagdRType Coef[4],
			CagdRType *Poly,
			int Log2Step,
			CagdBType NonAdaptive);
void AfdBzrCrvEvalToPolyline(const CagdCrvStruct *Crv,
			     int FineNess,
			     CagdRType *Points[]);

/******************************************************************************
* Routines to handle Bezier clipping based ray tracing.			      *
******************************************************************************/
CagdBType CagdRayTraceBzrSrf(CagdPType StPt,
                             CagdVType Dir,
                             const CagdSrfStruct *BzrSrf,
                             CagdUVStruct **IntrPrm,
                             CagdPtStruct **IntrPt);

/******************************************************************************
* B-Spline SDM fitting algorithm interface functions                          *
******************************************************************************/
CagdCrvStruct *CagdBsplineCrvFittingWithInitCrv(
		        CagdPType *PtList,	           /* Points cloud. */
		        int NumOfPoints,
			CagdCrvStruct *InitCrv,
		        CagdBspFittingType AgorithmType,
			int MaxIter,
		        CagdRType ErrorLimit,
			CagdRType ErrorChangeLimit,
		        CagdRType Lambda);

CagdCrvStruct *CagdBsplineCrvFitting(CagdPType *PtList,	      /* Pts cloud. */
				     int NumOfPoints,
				     int Length, 
				     int Order,
				     CagdBType IsPeriodic,
				     CagdBspFittingType AgorithmType,
				     int MaxIter,
				     CagdRType ErrorLimit,
				     CagdRType ErrorChangeLimit,
				     CagdRType Lambda);

/******************************************************************************
* Error handling.							      *
******************************************************************************/
CagdSetErrorFuncType CagdSetFatalErrorFunc(CagdSetErrorFuncType ErrorFunc);
void CagdFatalError(CagdFatalErrorType ErrID);
const char *CagdDescribeError(CagdFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* CAGD_LIB_H */
