/******************************************************************************
* Prsr_loc.h - header file for the data file\s parser library.		      *
* This library is closely related to cagd_lib and should be linked with it.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 94.					      *
******************************************************************************/

#ifndef PRSR_LOC_H
#define PRSR_LOC_H

#ifdef AMIGA
#include <exec/exec.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "irit_sm.h"
#include "iritprsr.h"

#include "ipc_loc.h"

#ifdef OS2GCC
#define INCL_DOSPROCESS
#include <os2.h>
#endif /* OS2GCC */

#ifdef __WINNT__
#include <winsock.h>
#endif /* __WINNT__ */

#ifdef IRIT_DOUBLE
#define IP_IRIT_FLOAT_READ "%lf"
#else
#define IP_IRIT_FLOAT_READ "%f"
#endif /* IRIT_DOUBLE */

#define UNGET_STACK_SIZE	5		     /* Internal stack size. */

#define MAX_KNOTS_PER_LINE	5	/* How many knots to print per line. */

typedef enum {			  /* List of all possible tokens enumerated. */
    IP_TOKEN_NONE,

    IP_TOKEN_OPEN_PAREN,
    IP_TOKEN_CLOSE_PAREN,

    IP_TOKEN_E1,
    IP_TOKEN_P1,
    IP_TOKEN_E2,
    IP_TOKEN_P2,
    IP_TOKEN_E3,
    IP_TOKEN_P3,
    IP_TOKEN_E4,
    IP_TOKEN_P4,
    IP_TOKEN_E5,
    IP_TOKEN_P5,
    IP_TOKEN_E6,
    IP_TOKEN_P6,
    IP_TOKEN_E7,
    IP_TOKEN_P7,
    IP_TOKEN_E8,
    IP_TOKEN_P8,
    IP_TOKEN_E9,
    IP_TOKEN_P9,

    IP_TOKEN_NUMBER,
    IP_TOKEN_STRING,
    IP_TOKEN_POINT,
    IP_TOKEN_VECTOR,
    IP_TOKEN_MATRIX,
    IP_TOKEN_CTLPT,
    IP_TOKEN_VERTEX,
    IP_TOKEN_POLYGON,
    IP_TOKEN_POLYLINE,
    IP_TOKEN_POINTLIST,
    IP_TOKEN_POLYSTRIP,
    IP_TOKEN_OBJECT,
    IP_TOKEN_COLOR,
    IP_TOKEN_RGB,
    IP_TOKEN_INTERNAL,
    IP_TOKEN_NORMAL,
    IP_TOKEN_PLANE,
    IP_TOKEN_CURVE,
    IP_TOKEN_SURFACE,

    IP_TOKEN_BEZIER,
    IP_TOKEN_BSPLINE,
    IP_TOKEN_POWER,
    IP_TOKEN_GREGORY,
    IP_TOKEN_TRIVAR,
    IP_TOKEN_PTYPE,
    IP_TOKEN_NUM_PTS,
    IP_TOKEN_ORDER,
    IP_TOKEN_KV,
    IP_TOKEN_KVP,
    IP_TOKEN_TRIMMDL,
    IP_TOKEN_TRIMSRF,
    IP_TOKEN_TRIMCRV,
    IP_TOKEN_TRIMCRVSEG,
    IP_TOKEN_INSTANCE,
    IP_TOKEN_TRISRF,
    IP_TOKEN_MODEL,
    IP_TOKEN_MDLTSEG,
    IP_TOKEN_MDLTSRF,
    IP_TOKEN_MDLLOOP,
    IP_TOKEN_MULTIVAR,

    IP_TOKEN_OTHER	= 100,		/* Probably names & numbers. */
    IP_TOKEN_QUOTED,			/* A quoted string. */

    IP_TOKEN_EOF = -1
} IPTokenType;

typedef enum {
    IGS_COLOR_NONE,
    IGS_COLOR_BLACK,
    IGS_COLOR_RED,
    IGS_COLOR_GREEN,
    IGS_COLOR_BLUE,
    IGS_COLOR_YELLOW,
    IGS_COLOR_MAGENTA,
    IGS_COLOR_CYAN,
    IGS_COLOR_WHITE
} IgesColorType;

typedef enum {
    IGS_ENTYPE_CIRCULAR_ARC = 100,
    IGS_ENTYPE_COMPOSITE_CURVE = 102,
    IGS_ENTYPE_CONIC_ARC = 104,
    IGS_ENTYPE_COPIOUS_DATA = 106,
    IGS_ENTYPE_PLANE = 108,
    IGS_ENTYPE_LINE = 110,
    IGS_ENTYPE_PARAM_SPLINE_CURVE = 112,
    IGS_ENTYPE_PARAM_SPLINE_SURFACE = 114,
    IGS_ENTYPE_POINT = 116,
    IGS_ENTYPE_RULED_SRF = 118,
    IGS_ENTYPE_SRF_OF_REV = 120,
    IGS_ENTYPE_TABULAR_CYL = 122,
    IGS_ENTYPE_TRANS_MAT = 124,
    IGS_ENTYPE_FLASH = 125,
    IGS_ENTYPE_RATIONAL_BSPLINE_CURVE = 126,
    IGS_ENTYPE_RATIONAL_BSPLINE_SURFACE = 128,
    IGS_ENTYPE_OFFSET_CURVE = 130,
    IGS_ENTYPE_OFFSET_SURFACE = 140,
    IGS_ENTYPE_BOUNDARY = 141,
    IGS_ENTYPE_CURVE_ON_PARAM_SRF = 142,
    IGS_ENTYPE_BOUNDED_SRF = 143,
    IGS_ENTYPE_TRIMMED_PARAM_SRF = 144,

    IGS_ENTYPE_BLOCK = 150,
    IGS_ENTYPE_RIGHT_ANGULAR_WEDGE = 152,
    IGS_ENTYPE_RIGHT_CIRC_CYL = 154,
    IGS_ENTYPE_RIGHT_CIRC_CONE = 156,
    IGS_ENTYPE_SPHERE = 158,
    IGS_ENTYPE_TORUS = 160,
    IGS_ENTYPE_SOLID_OF_REV = 162,
    IGS_ENTYPE_SOLID_OF_LINEAR_EXTRUDE = 164,
    IGS_ENTYPE_ELLIPSOID = 168,

    IGS_ENTYPE_BOOLEAN_TREE = 180,
    IGS_ENTYPE_SELECTED_COMP = 182,
    IGS_ENTYPE_SOLID_ASSEMBLY = 184,
    IGS_ENTYPE_SOLID_INSTANCE = 430,

    IGS_ENTYPE_ANGULAR_DIM = 202,
    IGS_ENTYPE_CURVE_DIM = 204,
    IGS_ENTYPE_DIAMETER_DIM = 206,
    IGS_ENTYPE_FLAG_NOTE = 208,
    IGS_ENTYPE_GENERAL_LABEL = 210,
    IGS_ENTYPE_GENERAL_NOTE = 212,
    IGS_ENTYPE_NEW_GENERAL_NOTE = 213,
    IGS_ENTYPE_LEADER_ARROW = 214,
    IGS_ENTYPE_LINEAR_DIM = 216,
    IGS_ENTYPE_ORDINATE_DIM = 218,
    IGS_ENTYPE_POINT_DIM = 220,
    IGS_ENTYPE_RADIUS_DIM = 222,
    IGS_ENTYPE_GENERAL_SYMBOL = 228,
    IGS_ENTYPE_SECTIONED_AREA = 230,

    IGS_ENTYPE_CONNECT_POINT = 132,
    IGS_ENTYPE_NODE = 134,
    IGS_ENTYPE_FINITE_ELEMENT = 136,
    IGS_ENTYPE_NODAL_DISP_ROT = 138,
    IGS_ENTYPE_NODAL_RESULTS = 146,
    IGS_ENTYPE_ELEMENT_RESULTS = 148,
    IGS_ENTYPE_ASSOC_DEF = 302,
    IGS_ENTYPE_LINE_FONT_DEF = 304,
    IGS_ENTYPE_MACRO_DEF = 306,
    IGS_ENTYPE_SUBFIG_DEF = 308,
    IGS_ENTYPE_TEXT_FONT_DEF = 310,
    IGS_ENTYPE_TEXT_DISPL_TEMP = 312,
    IGS_ENTYPE_COLOR_DEF = 314,
    IGS_ENTYPE_UNITS_DATA = 316,
    IGS_ENTYPE_NET_SUBFIG_DEF = 320,
    IGS_ENTYPE_ATTR_TBL_DEF = 322,
    IGS_ENTYPE_ASSOC_INSTNCE = 402,
    IGS_ENTYPE_DRAWING = 404,
    IGS_ENTYPE_PROPERTY = 406,
    IGS_ENTYPE_SNGL_SUBFIG_INSTNCE = 408,
    IGS_ENTYPE_VIEW = 410,
    IGS_ENTYPE_RECT_ARRAY_SUBFIG_INSTNCE = 412,
    IGS_ENTYPE_CIRC_ARRAY_SUBFIG_INSTNCE = 414,
    IGS_ENTYPE_EXTRNL_REF = 416,
    IGS_ENTYPE_NODE_LOAD_CONST = 418,
    IGS_ENTYPE_NET_SUBFIG_INSTNCE = 420,
    IGS_ENTYPE_ATTR_TBL_INSTNCE = 422
} IgesEntryType;

typedef enum {
    IGS_IRT_OP_CIRC_ARC,
    IGS_IRT_OP_POINT,
    IGS_IRT_OP_CTLPT,
    IGS_IRT_OP_POLY,
    IGS_IRT_OP_CURVE,
    IGS_IRT_OP_CONIC_ARC,
    IGS_IRT_OP_PLANE,
    IGS_IRT_OP_LINE,
    IGS_IRT_OP_TRANS_MAT,
    IGS_IRT_OP_SPLINE_CRV,
    IGS_IRT_OP_SPLINE_SRF,
    IGS_IRT_OP_RULED_SRF,
    IGS_IRT_OP_SRF_REV_AXES,
    IGS_IRT_OP_SRF_REV2AXES,
    IGS_IRT_OP_EXTRUDE,
    IGS_IRT_OP_RAT_BSP_CRV,
    IGS_IRT_OP_RAT_BSP_SRF,
    IGS_IRT_OP_CRV_ON_SRF,
    IGS_IRT_OP_TRIM_SRF,
    IGS_IRT_OP_OFFSET_CRV,
    IGS_IRT_OP_OFFSET_SRF,
    IGS_IRT_OP_LIST,
    IGS_IRT_OP_INSTANCE
} IgesIritOperatorType;

#if defined(__WINNT__) || defined(__OS2GCC__)
#   define IP_WRITE_BIN_MODE  "wb"
#   define IP_READ_BIN_MODE   "rb"
#else
#   define IP_WRITE_BIN_MODE  "w"
#   define IP_READ_BIN_MODE   "r"
#endif /* __WINNT__ || __OS2GCC__ */

#define IP_IS_TOKEN_POINT(Token)  ((Token) >= IP_TOKEN_E1 &&\
				   (Token) <= IP_TOKEN_P9)

#define IP_GREGORY_DBL_POINT(i, Len) (((i) == (Len) + 1) || \
				      ((i) == 2 * (Len) - 3) || \
				      ((i) == (((Len) * ((Len) + 1)) / 2) - 5))
#define IP_GREGORY_DBL_PT_IDX(i, Len) \
	((i) == (Len) + 1) ? \
	    ((Len) * ((Len) + 1)) / 2 : \
	    (((i) == 2 * (Len) - 3) ? ((Len) * ((Len) + 1)) / 2 + 1 \
				    : ((Len) * ((Len) + 1)) / 2 + 2);

#define IP_SOC_TIME_OUT		1000		  /* In 10th of miliseconds. */
#define IP_SOC_IRIT_DEF_PORT	5050		/* Default listening socket. */
#define IP_MAX_NUM_OF_STREAMS	50

typedef enum {
    IP_MDL_NO_CRV = 0,
    IP_MDL_UV1_CRV = 1,
    IP_MDL_UV2_CRV = 2,
    IP_MDL_EUC_CRV = 4
} IPMdlCurveMaskType;

typedef struct IPStreamInfoStruct {
    int InUse;
    int IsPipe;
    int Soc;
    int EchoInput;
    FILE *f;
    char FileName[IRIT_LINE_LEN_VLONG];
    IPStreamReadCharFuncType ReadCharFunc;
    IPStreamWriteBlockFuncType WriteBlockFunc;
    IPStreamFormatType Format;
    IPFileType FileType;
    float QntError;
    int SwapEndian;
    int TokenStackPtr;
    char TokenStack[UNGET_STACK_SIZE][IRIT_LINE_LEN];
    int UnGetChar;
    int LineNum;
    int Read;
    int BufferSize;
    int BufferPtr;
    unsigned char Buffer[IRIT_LINE_LEN_VLONG];
} IPStreamInfoStruct;

IRIT_GLOBAL_DATA_HEADER IPStreamInfoStruct
    _IPStream[IP_MAX_NUM_OF_STREAMS];

IRIT_GLOBAL_DATA_HEADER jmp_buf
    _IPLongJumpBuffer;
IRIT_GLOBAL_DATA_HEADER int
    _IPLongJumpActive,
    _IPMaxActiveStream,
    _IPFilterDegeneracies,
    _IPPolyListCirc,
    _IPReadWriteBinary,
    _IPReadOneObject;
IRIT_GLOBAL_DATA_HEADER char
    *_IPNCGcodeFloatFormat,
    *_IPGlblFloatFormat;
IRIT_GLOBAL_DATA_HEADER IPProcessLeafObjType
    _IPGlblProcessLeafFunc;

void _IPUnGetToken(int Handler, char *StringToken);
IPTokenType _IPGetToken(int Handler, char *StringToken);
void _IPGetCloseParenToken(int Handler);
int _IPSkipToCloseParenToken(int Handler);
int _IPThisLittleEndianHardware(void);

const char *_IPReal2Str(IrtRType R);

char *_IPGetCurveAttributes(int Handler);
char *_IPGetSurfaceAttributes(int Handler);

#ifdef USE_VARARGS
void _IPFprintf(int Handler, int Indent, char *va_alist, ...);
#else
void _IPFprintf(int Handler, int Indent, char *Format, ...);
#endif /* USE_VARARGS */

/* Error handling. */
#define IP_FATAL_ERROR_EX(MsgID, ErrLine, ErrDesc) \
				_IPFatalErrorEx(MsgID, ErrLine, ErrDesc)

void _IPFatalErrorEx(IPFatalErrorType ErrID,
		     int ErrLine,
		     const char *ErrDesc);
void _IPParseResetError();



#ifdef AMIGA
/* These are the names of two environment variables created and destroyed on
   the fly. They're used to make sure that the server and the client know of
   each other's existence. Yes, I know it's a kludge.
*/
#define SERVER_VAR	"IritRunning"
#define CLIENT_VAR	"ClientRunning"

struct IritMessage {
  struct Message msg;
  short		 nbytes;
  unsigned char	 txt[IRIT_LINE_LEN_LONG];
};
#endif /* AMIGA */

#endif /* PRSR_LOC_H */
