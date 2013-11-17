/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 0.1, Jan. 1992   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Main definition Header file  for Irit - the 3d polygonal solid modeller.   *
*****************************************************************************/

#ifndef	IRIT_SM_H
#define	IRIT_SM_H

#ifdef HAVE_CONFIG_H
#include <autoconfig.h>
#endif

/* Note program version should also be updated in *.c modules, in few        */
/* places, as some system can not chain strings in the pre-processor level.  */
#ifdef DEBUG
    #define  IRIT_VERSION	"Ver. 11D"		 /* Program version. */
#else
    #define  IRIT_VERSION	"Ver. 11"		 /* Program version. */
#endif
#define  IRIT_COPYRIGHT	"(C) Copyright 1989-2012 Gershon Elber, Technion"

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifndef	NULL
#define	NULL	0
#endif /* NULL */

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif /* TRUE */

#ifdef VoidPtr
#undef VoidPtr
#endif /* VoidPtr */

#ifdef NO_VOID_PTR
#define VoidPtr		char *
#else
#define VoidPtr		void *
#endif /* NO_VOID_PTR */

#if !defined(IRIT_FLOAT) && !defined(IRIT_DOUBLE)
#if defined(__MSDOS__) || defined(MAKE_REAL_IRIT_FLOAT)
#define IRIT_FLOAT
typedef float           IrtRType;
#define REAL_TYPE_SIZE	0x04
#else
#define IRIT_DOUBLE
typedef double          IrtRType;
#define REAL_TYPE_SIZE	0x08
#endif /* __MSDOS__ || MAKE_REAL_IRIT_FLOAT */
#endif /* !IRIT_FLOAT && !IRIT_DOUBLE */

/* No supprot for synamic memory testing in 64 bits. */
#if defined(_WIN64) && defined(DEBUG_IRIT_MALLOC)
#   undef DEBUG_IRIT_MALLOC
#endif /* _WIN64 && DEBUG_IRIT_MALLOC */

#ifdef DEBUG_IRIT_MALLOC
#define DEBUG_IP_MALLOC
#define DEBUG_ATTR_MALLOC
#endif /* DEBUG_IRIT_MALLOC */

typedef	unsigned char	IrtBType;

typedef	IrtRType	IrtMinMaxType[2];		     /* For 3 reals. */
typedef	IrtRType	IrtUVType[2];         /* For UV texture coordinates. */
typedef	IrtRType	IrtPtType[3];	/* For X, Y, Z coordinates of point. */
typedef IrtRType	IrtVecType[3]; /* For X, Y, Z coordinates of vector. */
typedef IrtRType	*IrtVecGnrlType;           /* A general size vector. */
typedef IrtRType	IrtLnType[3];	      /* A, B, C in Ax + By + C = 0. */
typedef IrtRType	IrtNrmlType[3];	    /* Unit normalized normal coeff. */
typedef IrtRType	IrtPlnType[4];		    /* Plane equation coeff. */
typedef IrtRType	IrtHmgnMatType[4][4];	   /* Homogeneous transform. */
typedef IrtRType	*IrtGnrlMatType;           /* A general size matrix. */
typedef IrtPtType	IrtBboxType[2];	      /* Axis parallel bounding box. */

typedef struct IrtE2PtStruct {
     IrtRType Pt[2];
} IrtE2PtStruct;

typedef struct IrtE3PtStruct {
     IrtRType Pt[3];
} IrtE3PtStruct;

/* The following is an integer that equal in size toa pointer in the system. */
#if defined(LINUX386) || defined(SUN4) || defined(__MACOSX__)
typedef long		IritIntPtrSizeType;
#elif defined(__WINNT__)
typedef intptr_t	IritIntPtrSizeType;
#else
typedef int		IritIntPtrSizeType;
#endif /* LINUX386 || SUN4 */

#define IRIT_EPS	1e-5
#define IRIT_LARGE	1e5
#define IRIT_INFNTY	2.3197171528332553e+25
#define IRIT_MAX_INT	0x7fffffff

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif /* M_PI */

#define M_PI_DIV_2	1.57079632679489661923
#define M_PI_MUL_2	6.28318530717958647692
#define M_1_DIV_2PI	0.15915494309189533577
#define M_1_DIV_PI	0.31830988618379067154

#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif /* M_SQRT2 */

#define IRIT_LINE_LEN_MAX_CFG	10000
#define IRIT_LINE_LEN_XLONG	4096
#define IRIT_LINE_LEN_VLONG	1024
#define IRIT_LINE_LEN_LONG	512
#define IRIT_LINE_LEN		256
#define IRIT_LINE_LEN_SHORT	31

/* Follows by general purpose helpful macros: */
#define IRIT_MIN(x, y)		((x) > (y) ? (y) : (x))
#define IRIT_MAX(x, y)		((x) > (y) ? (x) : (y))
#define IRIT_MIN3(x, y, z)	IRIT_MIN(IRIT_MIN((x), (y)), (z))
#define IRIT_MAX3(x, y, z)	IRIT_MAX(IRIT_MAX((x), (y)), (z))
#define IRIT_BOUND(x, Min, Max)	(IRIT_MAX(IRIT_MIN((x), Max), Min))

#define IRIT_ABS(x)		((x) > 0 ? (x) : (-(x)))
#ifdef IRIT_FLOAT
#define IRIT_FABS(x)		fabsf(x)
#else
#define IRIT_FABS(x)		fabs(x)
#endif /* IRIT_FLOAT */
#define IRIT_SQR(x)		((x) * (x))
#define IRIT_CUBE(x)		((x) * (x) * (x))
#define IRIT_SIGN(x)		((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

#define IRIT_SWAP(type, x, y)	{ type _temp = (x); x = (y); y = _temp; }

#define IRIT_BLEND(x, y, t)	((x) * t + (y) * (1 - t))

#define IRIT_REAL_TO_INT(R)	((int) ((R) > 0.0 ? (R) + 0.5 : (R) - 0.5))
#define IRIT_REAL_PTR_TO_INT(R)	IRIT_REAL_TO_INT(*(R))

#define IRIT_GEN_COPY(Dest, Src, Size) memcpy((char *) (Dest), (char *) (Src), \
					      Size)
#define IRIT_GEN_CMP(Dest, Src, Size)  memcmp((char *) (Dest), (char *) (Src), \
					      Size)
#define IRIT_ZAP_MEM(Dest, Size)       memset((char *) (Dest), 0, Size);

#ifdef IRIT_QUIET_STRINGS
#    define IRIT_EXP_STR(Str)			""
#    define IRIT_FATAL_ERROR(Str)		IritFatalError("")
#    define IRIT_FATAL_ERROR_PRINTF(...) 	IritFatalErrorPrintf("")
#    define IRIT_WARNING_MSG(Str)
#    define IRIT_WARNING_MSG_PRINTF(...)	
#    define IRIT_INFO_MSG(Str)
#    define IRIT_INFO_MSG_PRINTF(...)
#    define _IRIT_PT_NORMALIZE_MSG_ZERO(Size)
#else
#    define IRIT_EXP_STR(Str)			Str
#    define IRIT_FATAL_ERROR(Str)		IritFatalError(Str)
#    define IRIT_FATAL_ERROR_PRINTF		IritFatalErrorPrintf
#    define IRIT_WARNING_MSG(Str)		IritWarningMsg(Str)
#    define IRIT_WARNING_MSG_PRINTF		IritWarningMsgPrintf
#    define IRIT_INFO_MSG(Str)			IritInformationMsg(Str)
#    define IRIT_INFO_MSG_PRINTF		IritInformationMsgPrintf
#    define _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
	if (Size < IRIT_PT_NORMALIZE_ZERO) { \
	    IritWarningMsg("Attempt to normalize a zero length vector\n"); \
	} \
        else
#endif /* IRIT_QUIET_STRINGS */

#define CAGD_LARGE_BEZIER_CACHE

#define IRIT_APX_EQ(x, y)	(IRIT_FABS((x) - (y)) < IRIT_EPS)
#define IRIT_APX_UEQ(x, y)	(IRIT_FABS((x) - (y)) < IRIT_UEPS)
#define IRIT_APX_EQ_EPS(x, y, EPS)	(IRIT_FABS((x) - (y)) < EPS)
#define IRIT_PT_APX_EQ(Pt1, Pt2) (IRIT_APX_EQ((Pt1)[0], (Pt2)[0]) && \
				  IRIT_APX_EQ((Pt1)[1], (Pt2)[1]) && \
				  IRIT_APX_EQ((Pt1)[2], (Pt2)[2]))
#define IRIT_PT_APX_EQ_E2(Pt1, Pt2)	(IRIT_APX_EQ((Pt1)[0], (Pt2)[0]) && \
					 IRIT_APX_EQ((Pt1)[1], (Pt2)[1]))
#define IRIT_PT_APX_EQ_EPS(Pt1, Pt2, EPS) \
				(IRIT_APX_EQ_EPS((Pt1)[0], (Pt2)[0], EPS) && \
				 IRIT_APX_EQ_EPS((Pt1)[1], (Pt2)[1], EPS) && \
				 IRIT_APX_EQ_EPS((Pt1)[2], (Pt2)[2], EPS))
#define IRIT_PT_APX_EQ_E2_EPS(Pt1, Pt2, EPS) \
				(IRIT_APX_EQ_EPS((Pt1)[0], (Pt2)[0], EPS) && \
				 IRIT_APX_EQ_EPS((Pt1)[1], (Pt2)[1], EPS))
#define IRIT_PLANE_APX_EQ(Pl1, Pl2)	((IRIT_APX_EQ((Pl1)[0], (Pl2)[0]) && \
				  IRIT_APX_EQ((Pl1)[1], (Pl2)[1]) && \
				  IRIT_APX_EQ((Pl1)[2], (Pl2)[2]) && \
				  IRIT_APX_EQ((Pl1)[3], (Pl2)[3])) || \
				 (IRIT_APX_EQ((Pl1)[0], -(Pl2)[0]) && \
				  IRIT_APX_EQ((Pl1)[1], -(Pl2)[1]) && \
				  IRIT_APX_EQ((Pl1)[2], -(Pl2)[2]) && \
				  IRIT_APX_EQ((Pl1)[3], -(Pl2)[3])))
#define IRIT_PLANE_APX_EQ_EPS(Pl1, Pl2, EPS) \
			((IRIT_APX_EQ_EPS((Pl1)[0], (Pl2)[0], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[1], (Pl2)[1], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[2], (Pl2)[2], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[3], (Pl2)[3], EPS)) || \
			 (IRIT_APX_EQ_EPS((Pl1)[0], -(Pl2)[0], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[1], -(Pl2)[1], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[2], -(Pl2)[2], EPS) && \
			  IRIT_APX_EQ_EPS((Pl1)[3], -(Pl2)[3], EPS)))

#define IRIT_PT_APX_EQ_ZERO_EPS(Pt, EPS) \
				(IRIT_FABS((Pt)[0]) < EPS && \
				 IRIT_FABS((Pt)[1]) < EPS && \
				 IRIT_FABS((Pt)[2]) < EPS)
#define IRIT_PT_EQ_ZERO(Pt)    ((Pt)[0] == 0.0 && \
                                (Pt)[1] == 0.0 && \
                                (Pt)[2] == 0.0)

#ifdef IRIT_DOUBLE
#define IRIT_UEPS		1e-14
#else
#define IRIT_UEPS		1e-6
#endif /* IRIT_DOUBLE */

#define IRIT_PT_SCALE(Pt, Scalar)	{ (Pt)[0] *= (Scalar); \
					  (Pt)[1] *= (Scalar); \
					  (Pt)[2] *= (Scalar); \
				        }
#define IRIT_PT4D_SCALE(Pt, Scalar)	{ (Pt)[0] *= (Scalar); \
					  (Pt)[1] *= (Scalar); \
					  (Pt)[2] *= (Scalar); \
					  (Pt)[3] *= (Scalar); \
				        }
#define IRIT_PLANE_SCALE(Pl, Scalar)	IRIT_PT4D_SCALE(Pl, Scalar)

#define IRIT_PT_SCALE2(Res, Pt, Scalar) \
				{ (Res)[0] = (Pt)[0] * (Scalar); \
				  (Res)[1] = (Pt)[1] * (Scalar); \
				  (Res)[2] = (Pt)[2] * (Scalar); \
			        }

/* The memcpy is sometimes defined to get (char *) pointers and sometimes    */
/* (void *) pointers. To be compatible with both it is coerced to (char *).  */
#define IRIT_PT_COPY(PtDest, PtSrc) \
	    memcpy((char *) (PtDest), (char *) (PtSrc), 3 * sizeof(IrtRType))
#define IRIT_PT4D_COPY(PlDest, PlSrc) \
	    memcpy((char *) (PlDest), (char *) (PlSrc), 4 * sizeof(IrtRType))
#define IRIT_PLANE_COPY(PlDest, PlSrc) IRIT_PT4D_COPY(PlDest, PlSrc)

#define IRIT_HMGN_MAT_COPY(Dest, Src) \
            memcpy((char *) (Dest), (char *) (Src), 16 * sizeof(IrtRType))

#define IRIT_PT_SQR_LENGTH(Pt) (IRIT_SQR((Pt)[0]) + \
				IRIT_SQR((Pt)[1]) + \
				IRIT_SQR((Pt)[2]))
#define IRIT_PT_LENGTH(Pt)     sqrt(IRIT_PT_SQR_LENGTH(Pt))

#define IRIT_PT_RESET(Pt)	IRIT_ZAP_MEM((Pt), 3 * sizeof(IrtRType))
#define IRIT_PT4D_RESET(Pt)     IRIT_ZAP_MEM((Pt), 4 * sizeof(IrtRType))
#define IRIT_PLANE_RESET(Pl)    IRIT_PT4D_RESET(Pl)

#define IRIT_PT_NORMALIZE_ZERO	1e-30

#define IRIT_PT_NORMALIZE(Pt)	{ \
				     IrtRType Size = IRIT_PT_LENGTH((Pt)); \
				     _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
				     { \
					 Size = 1.0 / Size; \
				         IRIT_PT_SCALE(Pt, Size); \
				     } \
				}
#define IRIT_PT_SAFE_NORMALIZE(Pt)	{ \
				     IrtRType Size = IRIT_PT_LENGTH((Pt)); \
				     if (Size > IRIT_PT_NORMALIZE_ZERO) { \
					 Size = 1.0 / Size; \
				         IRIT_PT_SCALE(Pt, Size); \
				     } \
				}
#define IRIT_PT_CLEAN_NORMALIZE(Pt)	{ \
				     if (IRIT_FABS(Pt[0]) < IRIT_UEPS) Pt[0] = 0; \
				     if (IRIT_FABS(Pt[1]) < IRIT_UEPS) Pt[1] = 0; \
				     if (IRIT_FABS(Pt[2]) < IRIT_UEPS) Pt[2] = 0; \
				}
#define IRIT_PT_NORMALIZE_FLOAT(Pt)	{ \
				     float Size = (float) IRIT_PT_LENGTH((Pt)); \
				     _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
				     { \
					 Size = 1.0F / Size; \
				         IRIT_PT_SCALE(Pt, Size); \
				     } \
				}


#define	IRIT_PT_SET(Pt, Pt1, Pt2, Pt3) \
				{ (Pt)[0] = (Pt1); \
				  (Pt)[1] = (Pt2); \
				  (Pt)[2] = (Pt3); \
			        }

#define IRIT_PT_SCALE_AND_ADD(Res, Pt1, Pt2, t) \
			{ (Res)[0] = (Pt1)[0] + (Pt2)[0] * (t); \
			  (Res)[1] = (Pt1)[1] + (Pt2)[1] * (t); \
			  (Res)[2] = (Pt1)[2] + (Pt2)[2] * (t); \
			}

#define IRIT_PT_BLEND(Res, Pt1, Pt2, t) \
			{ (Res)[0] = (Pt1)[0] * t + (Pt2)[0] * (1 - t); \
			  (Res)[1] = (Pt1)[1] * t + (Pt2)[1] * (1 - t); \
			  (Res)[2] = (Pt1)[2] * t + (Pt2)[2] * (1 - t); \
			}

#define IRIT_PT_BLEND_BARYCENTRIC(Res, Pt1, Pt2, Pt3, W) { \
	Res[0] = Pt1[0] * W[0] + Pt2[0] * W[1] + Pt3[0] * W[2]; \
	Res[1] = Pt1[1] * W[0] + Pt2[1] * W[1] + Pt3[1] * W[2]; \
	Res[2] = Pt1[2] * W[0] + Pt2[2] * W[1] + Pt3[2] * W[2]; } \

#define IRIT_PT_ADD(Res, Pt1, Pt2) { (Res)[0] = (Pt1)[0] + (Pt2)[0]; \
				     (Res)[1] = (Pt1)[1] + (Pt2)[1]; \
				     (Res)[2] = (Pt1)[2] + (Pt2)[2]; \
			           }

#define IRIT_PT_SUB(Res, Pt1, Pt2) { (Res)[0] = (Pt1)[0] - (Pt2)[0]; \
				     (Res)[1] = (Pt1)[1] - (Pt2)[1]; \
				     (Res)[2] = (Pt1)[2] - (Pt2)[2]; \
				   }

#define IRIT_PT_SWAP(Pt1, Pt2)	{ IRIT_SWAP(IrtRType, (Pt1)[0], (Pt2)[0]); \
				  IRIT_SWAP(IrtRType, (Pt1)[1], (Pt2)[1]); \
				  IRIT_SWAP(IrtRType, (Pt1)[2], (Pt2)[2]); \
				}

#define IRIT_PT_PT_DIST(Pt1, Pt2)    sqrt(IRIT_SQR((Pt1)[0] - (Pt2)[0]) + \
				          IRIT_SQR((Pt1)[1] - (Pt2)[1]) + \
				          IRIT_SQR((Pt1)[2] - (Pt2)[2]))

#define IRIT_PT_PT_DIST_SQR(Pt1, Pt2) (IRIT_SQR((Pt1)[0] - (Pt2)[0]) + \
				       IRIT_SQR((Pt1)[1] - (Pt2)[1]) + \
				       IRIT_SQR((Pt1)[2] - (Pt2)[2]))

/* Now the same thing but for 2D points. */

#define IRIT_PT2D_SCALE(Pt, Scalar)	{ (Pt)[0] *= Scalar; \
				  	  (Pt)[1] *= Scalar; \
			        	}
#define IRIT_PT2D_SCALE2(Res, Pt, Scalar) { (Res)[0] = (Pt)[0] * (Scalar); \
				            (Res)[1] = (Pt)[1] * (Scalar); \
			                  }
#define IRIT_PT2D_COPY(PtDest, PtSrc) \
	     memcpy((char *) (PtDest), (char *) (PtSrc), 2 * sizeof(IrtRType))

#define IRIT_PT2D_SQR_LENGTH(Pt)     (IRIT_SQR((Pt)[0]) + IRIT_SQR((Pt)[1]))
#define IRIT_PT2D_LENGTH(Pt)	     sqrt(IRIT_PT2D_SQR_LENGTH(Pt))

#define IRIT_PT2D_RESET(Pt)	     IRIT_ZAP_MEM((Pt), 2 * sizeof(IrtRType))

#define IRIT_PT2D_NORMALIZE(Pt)	{ \
				     IrtRType Size = IRIT_PT2D_LENGTH((Pt)); \
				     _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
				     { \
					 Size = 1.0 / Size; \
				         IRIT_PT2D_SCALE(Pt, Size); \
				     } \
				}
#define IRIT_PT2D_SAFE_NORMALIZE(Pt) \
                              { \
				   IrtRType Size = IRIT_PT2D_LENGTH((Pt)); \
				   if (Size > IRIT_PT_NORMALIZE_ZERO) { \
				        Size = 1.0 / Size; \
				        IRIT_PT2D_SCALE(Pt, Size); \
				   } \
			      }
#define IRIT_PT2D_NORMALIZE_FLOAT(Pt) \
                            { \
				 float Size = (float) IRIT_PT2D_LENGTH((Pt)); \
				 _IRIT_PT_NORMALIZE_MSG_ZERO(Size) \
				 { \
				     Size = 1.0F / Size; \
				     IRIT_PT2D_SCALE(Pt, Size); \
				 } \
			    }

#define	IRIT_PT2D_SET(Pt, Pt1, Pt2) \
				{ (Pt)[0] = (Pt1); \
				  (Pt)[1] = (Pt2); \
			        }

#define IRIT_PT2D_SCALE_AND_ADD(Res, Pt1, Pt2, t) \
			{ (Res)[0] = (Pt1)[0] + (Pt2)[0] * t; \
			  (Res)[1] = (Pt1)[1] + (Pt2)[1] * t; \
			}

#define IRIT_PT2D_BLEND(Res, Pt1, Pt2, t) \
			{ (Res)[0] = (Pt1)[0] * t + (Pt2)[0] * (1 - t); \
			  (Res)[1] = (Pt1)[1] * t + (Pt2)[1] * (1 - t); \
			}

#define IRIT_PT2D_ADD(Res, Pt1, Pt2) { (Res)[0] = (Pt1)[0] + (Pt2)[0]; \
				       (Res)[1] = (Pt1)[1] + (Pt2)[1]; \
			             }

#define IRIT_PT2D_SUB(Res, Pt1, Pt2) { (Res)[0] = (Pt1)[0] - (Pt2)[0]; \
				       (Res)[1] = (Pt1)[1] - (Pt2)[1]; \
				     }

#define IRIT_PT2D_SWAP(Pt1, Pt2) { IRIT_SWAP(IrtRType, (Pt1)[0], (Pt2)[0]); \
				   IRIT_SWAP(IrtRType, (Pt1)[1], (Pt2)[1]); \
				 }

#define IRIT_PT2D_DIST(Pt1, Pt2)  sqrt(IRIT_SQR((Pt1)[0] - (Pt2)[0]) + \
				       IRIT_SQR((Pt1)[1] - (Pt2)[1]))

#define IRIT_PT2D_DIST_SQR(Pt1, Pt2) (IRIT_SQR((Pt1)[0] - (Pt2)[0]) + \
				      IRIT_SQR((Pt1)[1] - (Pt2)[1]))


#define IRIT_VEC_COPY(VDest, VSrc)      IRIT_PT_COPY(VDest, VSrc)
#define IRIT_VEC_SCALE(V, Scalar)       IRIT_PT_SCALE(V, Scalar)
#define IRIT_VEC_SCALE2(Res, V, Scalar) IRIT_PT_SCALE2(Res, V, Scalar)
#define IRIT_VEC_SQR_LENGTH(V)	        IRIT_PT_SQR_LENGTH(V)
#define IRIT_VEC_LENGTH(V)              IRIT_PT_LENGTH(V)
#define IRIT_VEC_RESET(V)               IRIT_PT_RESET(V)
#define IRIT_VEC_NORMALIZE(V)	        IRIT_PT_NORMALIZE(V)
#define IRIT_VEC_SAFE_NORMALIZE(V)	IRIT_PT_SAFE_NORMALIZE(V)
#define	IRIT_VEC_SET(V, V1, V2, V3)     IRIT_PT_SET(V, V1, V2, V3)
#define IRIT_VEC_BLEND(VRes, V1, V2, t) IRIT_PT_BLEND(VRes, V1, V2, t)
#define IRIT_VEC_ADD(VRes, V1, V2)	IRIT_PT_ADD(VRes, V1, V2)
#define IRIT_VEC_SUB(VRes, V1, V2)	IRIT_PT_SUB(VRes, V1, V2)
#define IRIT_VEC_SWAP(V1, V2)	        IRIT_PT_SWAP(V1, V2)

/* Now the same thing but for 2D vectors. */

#define IRIT_VEC2D_COPY(VDest, VSrc)      IRIT_PT2D_COPY(VDest, VSrc)
#define IRIT_VEC2D_SCALE(V, Scalar)       IRIT_PT2D_SCALE(V, Scalar)
#define IRIT_VEC2D_SCALE2(Res, V, Scalar) IRIT_PT2D_SCALE2(Res, V, Scalar)
#define IRIT_VEC2D_SQR_LENGTH(V)	  IRIT_PT2D_SQR_LENGTH(V)
#define IRIT_VEC2D_LENGTH(V)              IRIT_PT2D_LENGTH(V)
#define IRIT_VEC2D_RESET(V)               IRIT_PT2D_RESET(V)
#define IRIT_VEC2D_NORMALIZE(V)	          IRIT_PT2D_NORMALIZE(V)
#define IRIT_VEC2D_SAFE_NORMALIZE(V)	  IRIT_PT2D_SAFE_NORMALIZE(V)
#define	IRIT_VEC2D_SET(V, V1, V2)	  IRIT_PT2D_SET(V, V1, V2)
#define IRIT_VEC2D_BLEND(VRes, V1, V2, t) IRIT_PT2D_BLEND(VRes, V1, V2, t)
#define IRIT_VEC2D_ADD(VRes, V1, V2)	  IRIT_PT2D_ADD(VRes, V1, V2)
#define IRIT_VEC2D_SUB(VRes, V1, V2)	  IRIT_PT2D_SUB(VRes, V1, V2)
#define IRIT_VEC2D_SWAP(V1, V2)	          IRIT_PT2D_SWAP(V1, V2)

#define IRIT_UV_COPY(UVDst, UVSrc)      IRIT_GEN_COPY(UVDst, UVSrc, \
                                                 2 * sizeof(IrtRType))
#define IRIT_UV_RESET(Uv)	        IRIT_ZAP_MEM((Uv), 2 * sizeof(IrtRType))
#define IRIT_UV_BLEND(UVRes, UV1, UV2, t) \
			{ (UVRes)[0] = (UV1)[0] * t + (UV2)[0] * (1 - t); \
			  (UVRes)[1] = (UV1)[1] * t + (UV2)[1] * (1 - t); \
			}

#define IRIT_VEC_BLEND_BARYCENTRIC(Res, Pt1, Pt2, Pt3, W) \
			IRIT_PT_BLEND_BARYCENTRIC(Res, Pt1, Pt2, Pt3, W)

#define IRIT_DOT_PROD(Pt1, Pt2)	((Pt1)[0] * (Pt2)[0] + \
				 (Pt1)[1] * (Pt2)[1] + \
				 (Pt1)[2] * (Pt2)[2])

#define IRIT_CROSS_PROD(PtRes, Pt1, Pt2) \
		{ (PtRes)[0] = (Pt1)[1] * (Pt2)[2] - (Pt1)[2] * (Pt2)[1]; \
		  (PtRes)[1] = (Pt1)[2] * (Pt2)[0] - (Pt1)[0] * (Pt2)[2]; \
		  (PtRes)[2] = (Pt1)[0] * (Pt2)[1] - (Pt1)[1] * (Pt2)[0]; }

#define IRIT_DOT_PROD_2D(Pt1, Pt2) ((Pt1)[0] * (Pt2)[0] + (Pt1)[1] * (Pt2)[1])

#define IRIT_CROSS_PROD_2D(Pt1, Pt2) ((Pt1)[0] * (Pt2)[1] - \
				      (Pt1)[1] * (Pt2)[0])

#define IRIT_LIST_PUSH(New, List) { (New) -> Pnext = (List); (List) = (New); }
#define IRIT_LIST_POP(Head, List) { (Head) = (List); \
				     (List) = (List) -> Pnext; \
				     (Head) -> Pnext = NULL; }
#define IRIT_LIST_LAST_ELEM(Elem)   { if (Elem) \
					 while ((Elem) -> Pnext) \
					     (Elem) = (Elem) -> Pnext; }

#define IRIT_DEG2RAD_CNVRT		0.0174532925199432957692
#define IRIT_RAD2DEG_CNVRT		57.2957795130823208768

#define IRIT_DEG2RAD(Deg)		((Deg) * IRIT_DEG2RAD_CNVRT)
#define IRIT_RAD2DEG(Rad)		((Rad) * IRIT_RAD2DEG_CNVRT)

#define IRT_STR_NULL_ZERO_LEN(Str)	(Str == NULL || Str[0] == 0)
#define IRT_STR_ZERO_LEN(Str)		(Str[0] == 0)

#if defined(__WINNT__)
#   define IRIT_STATIC_DATA static
#   define IRIT_GLOBAL_DATA 
#   define IRIT_GLOBAL_DATA_HEADER extern
#else
#   define IRIT_STATIC_DATA static
#   define IRIT_GLOBAL_DATA
#   define IRIT_GLOBAL_DATA_HEADER extern
#endif /* __WINNT__ */

#ifdef DEBUG
#define IRIT_SET_DEBUG_PARAMETER(DbgPrm, Val) \
	IRIT_STATIC_DATA int DbgPrm = Val
#define IRIT_IF_DEBUG_ON_PARAMETER(DbgPrm) \
        if (DbgPrm)
#define IRIT_SET_IF_DEBUG_ON_PARAMETER(DbgPrm, Val) \
	IRIT_STATIC_DATA int DbgPrm = Val; \
        if (DbgPrm)
#endif /* DEBUG */

#if defined(__WINNT__) && _MSC_VER >= 1400 /* VC++ 2005 */
/* Under windows we can compute a-priori the length of the expected string. */
#define IRIT_VSPRINTF(p, Format, ArgPtr) { \
    IRIT_STATIC_DATA char \
	*_Line = NULL; \
    IRIT_STATIC_DATA int \
	_LineLen = 0; \
    int ThisLen; \
\
    if (_Line == NULL) \
      _Line = (char *) IritMalloc(_LineLen = IRIT_LINE_LEN); \
    if ((ThisLen = _vscprintf(Format, ArgPtr) + 2) >= _LineLen) { \
	IritFree(_Line); \
	_Line = (char *) IritMalloc(_LineLen = ThisLen); \
    } \
    vsprintf_s(p = _Line, _LineLen, Format, ArgPtr); \
}
#else
#define IRIT_INPUT_LINE_LEN	10000
#define IRIT_VSPRINTF(p, Format, ArgPtr) { \
    IRIT_STATIC_DATA char \
	*_Line = NULL; \
\
    if (_Line == NULL) \
	_Line = (char *) IritMalloc(IRIT_INPUT_LINE_LEN); \
    vsprintf(p = _Line, Format, ArgPtr); \
}
#endif /* __WINNT__ && _MSC_VER */

#if defined(_AIX) || defined(sgi) || defined(SUN4) || defined(DJGCC) || defined(OS2GCC) || defined(__WINNT__) || defined(__WINCE__) || defined(AMIGA) || defined(_INCLUDE_HPUX_SOURCE) || defined(OSF1DEC) || defined(__FreeBSD__) || defined(__osf__) || defined(_AlphaLinux) || defined(LINUX386) || defined(__CYGWIN__) || defined(__MACOSX__)
#    include <stdlib.h>

#    if defined(OSF1DEC) || defined(sgi) || defined(SUN4) || defined(OS2GCC) || defined(__FreeBSD__)
#	ifdef OSF1DEC
#	   ifdef _XOPEN_SOURCE
#	       undef _XOPEN_SOURCE /* For usleep */
#	   endif /*  _XOPEN_SOURCE */
#	endif /* OSF1DEC */
#	include <unistd.h>
#    endif /* OSF1DEC || sgi || SUN4 || OS2GCC || FreeBSD */
#    ifdef __WINNT__
#	include <direct.h>
#       if _MSC_VER >= 1400  /* Visual 8, 2005 */
#	    define mkdir(Dir, Permit)		_mkdir(Dir)
#       else
#	    define mkdir(Dir, Permit)		mkdir(Dir)
#       endif /* _MSC_VER >= 1400 */
#    endif /* __WINNT__ */
#else
    VoidPtr malloc(unsigned int Size);
    void free(VoidPtr p);
    char *getenv(char *Name);
    int atoi(char *str);
#endif /* _AIX || sgi || SUN4 || DJGCC || OS2GCC || __WINNT__ || __WINCE__ || AMIGA || _INCLUDE_HPUX_SOURCE || OSF1DEC || __FreeBSD__ || _AlphaLinux || LINUX386 ||__CYGWIN__ || __MACOSX__ */

#ifdef SGI64  /* Machine with 64 bit longs */
#   define sizeof(x)	((unsigned int) sizeof(x))
#   define strlen(x)	((unsigned int) strlen(x))
#endif /* SGI64 */

#ifdef __WINCE__
#   define getenv(Str)	NULL
#   define putenv(Str)
#   define signal(x, y)
#   define getcwd(s, l)	strcpy(s, ".")
#   define chdir(s)
#   define system(s)	0
#   define popen(s, rw)	fopen(s, rw)
#   define time(x)	0
#   define ctime(x)	""
#   define unlink(s)
#   define strerror(x)	0
#   define time_t       int
#   define abort()	exit(1)	
#else
#   include <signal.h>
#endif /* __WINCE__ */

#endif	/* IRIT_SM_H */
