/*****************************************************************************
* Rendering algorithm definitions and data structures interface.         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
*****************************************************************************/

#ifndef _RNDR_LOC_H_
#define _RNDR_LOC_H_

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "allocate.h"
#include "grap_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"
#include "rndr_lib.h"

#define RNDR_X_AXIS		0
#define RNDR_Y_AXIS		1
#define RNDR_Z_AXIS		2
#define RNDR_W_AXIS		3

#define RNDR_RED_CLR		0
#define RNDR_GREEN_CLR		1
#define RNDR_BLUE_CLR		2
#define RNDR_ALPHA_CLR		3

#define RNDR_POINT_LIGHT	IRNDR_LIGHT_POINT
#define RNDR_VECTOR_LIGHT	IRNDR_LIGHT_VECTOR
#define RNDR_SHADING_FLAT	IRNDR_SHADING_FLAT
#define RNDR_SHADING_GOURAUD	IRNDR_SHADING_GOURAUD
#define RNDR_SHADING_PHONG	IRNDR_SHADING_PHONG
#define RNDR_SHADING_NONE	IRNDR_SHADING_NONE

#define RNDR_ZBUF_SAME_EPS	1e-2

#define RNDR_MAX_NOISE		60

/* VisMap constants. */
#define RNDR_UV_CRITIC_AR	20
#define RNDR_UV_CRITIC_TAN	0.1
#define RNDR_UV_DILATION_ITERATIONS 0
#define RNDR_MIN_TRI_EDGE_LENGTH 1e-5
#define VIS_MAP_X_ATTRIB "VIS_MAP_X_ATTRIB"
#define VIS_MAP_Y_ATTRIB "VIS_MAP_Y_ATTRIB"

/* Left/right coordinate system dependences. */
#define RNDR_NEAREST_Z		IRIT_INFNTY
#define RNDR_FAREST_Z		-IRIT_INFNTY
#define RNDR_VIEWER_SIGHT	1
#define RNDR_MAX_LIGHTS_NUM	24

#define RNDR_MALLOC(type, n)	(type *) IritMalloc(sizeof(type) * (n))
#define RNDR_FREE(p)		IritFree(p)
#define RNDR_MAXM(a, b)	{ if ((a) < (b)) a = b; }
#define RNDR_MINM(a, b)	{ if ((a) > (b)) a = b; }
#define RNDR_IN(value, lower, upper) ((lower) <= (value) && (value) <= (upper))
#define RNDR_PLANE_EQ_EVAL(pl, pt)   (IRIT_DOT_PROD((pl), (pt)) + (pl)[3])

/* To use a an internal color representation other than IrtRType, define    */
/* one of the IRNDR_COLOR_* macros via -D switch or similar when building   */
/* rndr_lib.								    */
#define IRNDR_COLOR_INT8 1

#if defined IRNDR_COLOR_INT8
    #define RNDR_REAL_TO_COL(x) ((unsigned char)(0.5 + (x) * 0xff))
    #define RNDR_COL_TO_REAL(x) (((IrtRType)(x)) * (1.0/0xff))
    typedef unsigned char IRndrChannelType;
#elif defined IRNDR_COLOR_INT16
    #define RNDR_REAL_TO_COL(x) ((unsigned short)(0.5 + (x) * 0xffff))
    #define RNDR_COL_TO_REAL(x) (((IrtRType)(x)) * (1.0/0xffff))
    typedef unsigned short IRndrChannelType;
#elif defined IRNDR_COLOR_FP32
    #define RNDR_REAL_TO_COL(x) ((float)(x))
    #define RNDR_COL_TO_REAL(x) ((IrtRType)(x))
    typedef float IRndrChannelType;
#elif defined IRNDR_COLOR_FP64
    #define RNDR_REAL_TO_COL(x) ((double)(x))
    #define RNDR_COL_TO_REAL(x) ((IrtRType)(x))
    typedef double IRndrChannelType;
#else
    #define RNDR_REAL_TO_COL(x) ((IrtRType)(x))
    #define RNDR_COL_TO_REAL(x) ((IrtRType)(x))
    typedef IrtRType IRndrChannelType;
#endif

typedef IRndrChannelType IRndrPixelType[3];

#define RNDR_SET_COL_FROM_REAL(x, y) {\
    (x)[RNDR_RED_CLR]   = RNDR_REAL_TO_COL(y[RNDR_RED_CLR]); \
    (x)[RNDR_GREEN_CLR] = RNDR_REAL_TO_COL(y[RNDR_GREEN_CLR]); \
    (x)[RNDR_BLUE_CLR]  = RNDR_REAL_TO_COL(y[RNDR_BLUE_CLR]); }

#define RNDR_SET_REAL_FROM_COL(x, y) {\
    (x)[RNDR_RED_CLR]   = RNDR_COL_TO_REAL(y[RNDR_RED_CLR]); \
    (x)[RNDR_GREEN_CLR] = RNDR_COL_TO_REAL(y[RNDR_GREEN_CLR]); \
    (x)[RNDR_BLUE_CLR]  = RNDR_COL_TO_REAL(y[RNDR_BLUE_CLR]); }

#define RNDR_REAL_TO_BYTE(x) ((IrtBType)(0.5 + (x) * 0xff))

/* Mapping of predefined color constants to colors. */
IRIT_GLOBAL_DATA_HEADER IRndrColorType Colors[];

typedef IrtPlnType IrtPtType4;
#define IRIT_PT_COPY4(PlDest, PlSrc) memcpy((char *) (PlDest), (char *) (PlSrc), \
                            4 * sizeof(IrtRType))

void _IRndrReportWarning(const char *Fmt, ...);
void _IRndrReportError(const char *Fmt, ...);
void _IRndrReportFatal(const char *Fmt, ...);

#endif /* _RNDR_LOC_H_ */
