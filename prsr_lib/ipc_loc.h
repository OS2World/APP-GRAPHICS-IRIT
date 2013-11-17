/******************************************************************************
* Ipc_loc.h - header file for the data file\s compression parser library.     *
* This library is closely related to prsr_lib and should be linked with it.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Yura Zharkovsky, Dec 2003					      *
******************************************************************************/

#ifndef IPC_LOC_H
#define IPC_LOC_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void _IpcPredictPointsArcAux(CagdRType **QntPoints, int v); 
void _IpcErrHandler(int type);
const char *_IpcGetFileNameNoExt(const char *FileName, int WithPath);
const char *_IpcCompressMapFile(const char *FileName);
const char *_IpcDecompressArchiveFile(const char *FileName);
void _IpcRemoveTempFile(const char *TempFileName);
void _IpcSetToPt(CagdRType *Points[], int v, IrtPtType Pt, int MaxCoord);
void _IpcSetFromPt(CagdRType *Points[], int v, IrtPtType Pt, int MaxCoord);
void _IpcPtCopy(CagdRType *Pts1[], int v1,
                CagdRType *Pts2[], int v2,
                int MaxCoord);
void _IpcPtSub(CagdRType *PtsRes[], int vRes,
               CagdRType *Pts1[], int v1,
               CagdRType *Pts2[], int v2,
               int MaxCoord);
void _IpcPtAdd(CagdRType *PtsRes[], int vRes,
               CagdRType *Pts1[], int v1,
               CagdRType *Pts2[], int v2,
               int MaxCoord);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#ifdef IPC_BIN_COMPRESSION

/* If not in IPC debug mode, use GZip compression.  */
#if !defined(DEBUG_IPC) && !defined(HAVE_GZIP_LIB)
#    define HAVE_GZIP_LIB
#endif /* !defined(DEBUG_IPC) && !defined(HAVE_GZIP_LIB) */

#ifdef __WINNT__
#define ZLIB_WINAPI
#include <fcntl.h>
#include <io.h>
#endif /* __WINNT__ */

#ifdef HAVE_GZIP_LIB
#   include "zlib.h"
#endif /* HAVE_GZIP_LIB */

/* Specifies the IPC Version Format. */
#define IPC_VERSION_1 1
#define IPC_VERSION IPC_VERSION_1

/* Maximum safe quantization value, almost in all cases 	*/
/* (1e8) can also be used, but visual testing has to be done. 	*/
#define IPC_MAX_QUANTIZER (1e7)

/* Used in order to set number of different predictors 	*/
/* that will be used per spesific surface type. 	*/
#define IPC_CRV_NUM_PREDICTORS      1
#define IPC_SRF_NUM_PREDICTORS      2

/* Default predictors. */
//#define  IPC_DEFAULT_SRF_PREDICTOR      IPC_SRF_PREDICTOR_ANGLES
#define  IPC_DEFAULT_SRF_PREDICTOR      IPC_SRF_PREDICTOR_PARALLELOGRAM
#define  IPC_DEFAULT_CRV_PREDICTOR      IPC_CRV_PREDICTOR_UNIFORM
#define  IPC_DEFAULT_TRISRF_PREDICTOR   IPC_TRISRF_PREDICTOR_UNIFORM
#define  IPC_DEFAULT_TV_PREDICTOR       IPC_TV_PREDICTOR_UNIFORM
#define  IPC_DEFAULT_MV_PREDICTOR       IPC_MV_PREDICTOR_UNIFORM

#define IPC_EPS 1e-8

/* Global angle's definitions, used in angle's prediction. */
#define IPC_ANGLES_SET 1
#define IPC_ANGLES_SET_AUX IPC_ANGLES_SET + 1
#define IPC_ANGLES_MAX 10

/* IPC Errors. */
typedef enum {
    IPC_ERROR_OPEN_FILE,
    IPC_ERROR_ZLIB,
    IPC_ERROR_READ_FILE,
    IPC_ERROR_NOT_IPC_FILE,
    IPC_ERROR_FILE_CORRUPTED,
    IPC_ERROR_PREDICTOR_NOT_DEFINED,
    IPC_ERROR_INVALID_QUANTIZER_VALUE,
    IPC_ERROR_NO_TEMP_FILE,
    IPC_ERROR_WRITE_TO_STDOUT,
    IPC_ERROR_PREDICTOR_INVALID_NORMAL,
    IPC_ERROR_GENERAL
} IpcErrorType;

/* Used in order to compress the uniqe (common used) knot vectors optimally. */
typedef enum { 
    IPC_KNOT_VECTOR_GENERAL = 0, 
    IPC_KNOT_VECTOR_UNIFORM_FLOAT, 
    IPC_KNOT_VECTOR_UNIFORM_OPEN, 
 
    IPC_KNOT_VECTOR_NOT_QUANTIZED 
 
} IpcKnotVectorType;

/* Ipc predictors. */
typedef enum {
    /* Surfaces predictors. */
    IPC_SRF_PREDICTOR_PARALLELOGRAM = 0, 
    IPC_SRF_PREDICTOR_ANGLES,
    IPC_SRF_PREDICTOR_ANGLES_PERFECT1,
    IPC_SRF_PREDICTOR_ANGLES_REGULAR1,
    IPC_SRF_PREDICTOR_ANGLES_PERFECT2,
    IPC_SRF_PREDICTOR_ANGLES_REGULAR2,

    /* Curves predictors. */
    IPC_CRV_PREDICTOR_UNIFORM, 
    IPC_CRV_PREDICTOR_2D,
    IPC_CRV_PREDICTOR_ARC,
    
    /* Tri surfaces predictors. */
    IPC_TRISRF_PREDICTOR_UNIFORM,
    
    /* Trivariate predictors. */
    IPC_TV_PREDICTOR_UNIFORM,
    
    /* Multi Variate predictors. */
    IPC_MV_PREDICTOR_UNIFORM
} IpcPredictorType;

/* Mapping object constants, used in order to determine what parts      */
/* of object will be saved.                                             */
typedef enum {
    IPC_BITMAP_BBOX  = 0x01, 
    IPC_BITMAP_COUNT = 0x02,
    IPC_BITMAP_TAGS  = 0x04,
    IPC_BITMAP_NAME  = 0x08, 
    IPC_BITMAP_ATTR  = 0x10, 
    IPC_BITMAP_DPNDS = 0x20
 } IpcBitmapType; 
 
/* Maps which indicates what to compress/uncompress in object. */ 
typedef enum { 
    IPC_BSPLINE_SRF_MAP = 0x3E, 
    IPC_DEFAULT_MAP     = 0x3C
} IpcMappingType; 
 
/* Parameters passing structure and defines, used both in irtprsrc, irtprsrd.*/
/* Used in order to pass arguments to buttom functions. */ 
typedef struct IpcArgs { 
    float            QntError;      	/* Quantization error. 	            */
    int              Quantizer;    	/* Depends on QntError.             */
    IpcPredictorType SrfPredictor;  	/* Defines a way the surface's      */
					/* differences are calculated.      */
    IpcPredictorType CrvPredictor;  	/* Defines a way the curve's        */
				        /* differences are calculated.      */
    IpcPredictorType TriSrfPredictor;  	/* Defines a way the Triangular's   */
					/* differences are calculated.      */
    IpcPredictorType TVPredictor;   	/* Defines a way the tri-variate's  */
					/* differences are calculated.      */
    IpcPredictorType MVPredictor;   	/* Defines a way the multi-variate's*/
					/* differences are calculated.      */
    int SwapEndian;
} IpcArgs; 
 
/* Nurbs are pre-proccesed before entropy coder to that structure */ 
typedef struct IpcKnotVectorStruct { 
    CagdRType Scale;              /* Scale knot vector.                     */
    CagdRType DeltaK;             /* Quantizer of step size in error values */
				  /* of knot vector.                        */
    int       NumBreakValues;     /* Number of distinguish values.          */
    CagdRType InitBreakValue;     /* First value in knot vector.            */
    IpcKnotVectorType  KnotVectorMap;      
                                  /* Used in order to compress knot vector  */
				  /*     more efficiently.                  */
    int       *ErrIndexes;        /* Quantized indexes of break vector.     */
    int       *MultiplicityMap; 
    CagdRType *KVErrors;          /* Not quantized errors of break vector.  */
 
} IpcKnotVectorStruct; 
 
typedef struct IpcCtlPtStruct { 
    CagdRType Delta;          	             /* Step size quanizer of error  */
                                             /* values in ontrol points.     */
    CagdRType DRange;                        /* The maximum dynamic range.   */
				             /*	of the coordinates           */
    CagdRType InitialValue[CAGD_MAX_PT_SIZE];/* Initial predicted value for  */
					     /* control points and weights   */
    int *ErrIndexes[CAGD_MAX_PT_SIZE];	     /* Quantized indexes            */
					     /* of control points.           */
    CagdRType *PErrors[CAGD_MAX_PT_SIZE];    /* Pointer on each axis vector. */
    int  MaxIndex;                           /* Maximum index of weights and */
                                             /* control points               */
    long NumPoints;      		     /* Number of control points.    */
    long ExtNumPoints;                       /* Number of control points with*/
                                             /* coordinats -1                */
    IrtVecType Normal;        		     /* Used in 2D predictor.        */
    
    IrtVecType *Nrmls;                       
    IpcPredictorType *Predicts;
    int NumBlocks;
    int BlockLen;
    CagdRType **Angles;
    int *NumAngles;

    CagdPointType PType;         

} IpcCtlPtStruct; 
 
#define IPC_MAX_MULTIVAR_DIM 128 
 
typedef struct IpcSrfStruct { 
    struct IpcSrfStruct *Pnext; 
    int       ULength, VLength, WLength;                     
    int       UOrder,  VOrder,  WOrder;     
    CagdBType UPeriodic, VPeriodic, WPeriodic;      /* Valid only for       */ 
						    /* Bspline surfaces.    */
    IpcKnotVectorStruct UKV;     
    IpcKnotVectorStruct VKV; 
    IpcKnotVectorStruct WKV; 
    IpcCtlPtStruct  PointErrs;     
    CagdPointType PType; 
    CagdGeomType GType; 
     
    IpcKnotVectorStruct KVs[IPC_MAX_MULTIVAR_DIM];  /* Defined only         */
						    /* for multi variates.  */
    int Lengths[IPC_MAX_MULTIVAR_DIM]; 
    int Orders[IPC_MAX_MULTIVAR_DIM]; 
    int Dim; 
     
    CagdRType *Points[CAGD_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    IpcArgs *Args;     			     /* Compression state arguments. */
     
} IpcSrfStruct; 
 
typedef struct IpcCrvStruct { 
    struct IpcCrvStruct *Pnext; 
    int       Length;                     
    int       Order; 
    IpcKnotVectorStruct KV; 
    IpcCtlPtStruct  PointErrs;     
    CagdPointType PType; 
    CagdGeomType GType; 
    CagdBType Periodic; 
     
    IpcArgs *Args;                           /* Compression state arguments. */
 
} IpcCrvStruct; 
 
typedef struct IpcTriSrfStruct { 
    struct IpcCrvStruct *Pnext; 
    int                 Length;                     
    int                 Order; 
    IpcKnotVectorStruct KV; 
    IpcCtlPtStruct      PointErrs;     
    CagdPointType       PType; 
    TrngGeomType        GType; 
     
    IpcArgs *Args;                           /* Compression state arguments. */
} IpcTriSrfStruct; 
 
/* Used in order to compress/decompress list of strings. */  
typedef struct IpcStrings { 
    struct IpcStrings *Pnext; 
    char *Name;     
 
} IpcStrings; 

/* Define next variable in order to save vertex coordinates as float type. */
/* #define IPC_VERTEX_FLOAT						   */
 
/* Used in order to get a correct control point index. */ 
#define IPC_MESH_UV(Srf, i, j)    CAGD_MESH_UV(Srf, i, j) 
 
/* Used in order to get a correct quantized control point index. */ 
#define IPC_QNT_MESH_UV(Srf, i, j)    ((i+1) + ((Srf) -> ULength + 1) * (j+1)) 
 
/* Used in order to get a correct quantized control point index for Triv TV. */
#define IPC_QNT_TRIV_MESH_UVW(TV, i, j, k)\
        ((i+1) + (TV -> ULength + 1) * (j+1) + (TV -> UVPlane) * (k+1))\
	 
#define IPC_QUANTIZATION_USED(x) (x != IPC_QUANTIZATION_NONE)
#define IPC_CAT_VALUE(QntError)\
		     (int)(pow(10.0, ceil(log10(1/((QntError)+1e-8)))))\
 
#define IPC_LIMIT_ZERO(Val) ( (IRIT_FABS(Val) < IRIT_EPS) ? 0.0 : (Val))
#define IPC_LIMIT_ONE(Val)\
     ((IRIT_FABS(IRIT_FABS(Val) - 1) < IRIT_EPS) ? (1.0 * IRIT_SIGN(Val)) \
					         : (Val)) \
 
#define IPC_ROUND_QUANTIZER(QntError) ((float)1.0/IPC_CAT_VALUE(QntError))
#define IPC_ROUND10(x) (pow(10, ceil(log10(x))))
#define IPC_ZERO_PT(Pt) { \
	if (IRIT_APX_EQ_EPS((Pt)[0], 0, IPC_EPS)) (Pt)[0] = 0.0; \
	if (IRIT_APX_EQ_EPS((Pt)[1], 0, IPC_EPS)) (Pt)[1] = 0.0; \
	if (IRIT_APX_EQ_EPS((Pt)[2], 0, IPC_EPS)) (Pt)[2] = 0.0; } \
 
#define IPC_PT_CLEAR(Pt) {  if (IRIT_FABS((Pt)[0]) < IRIT_EPS) (Pt)[0] = 0; \
			    if (IRIT_FABS((Pt)[1]) < IRIT_EPS) (Pt)[1] = 0; \
			    if (IRIT_FABS((Pt)[2]) < IRIT_EPS) (Pt)[2] = 0; } \
				 
#define IPC_IRIT_PT_SET(Pt, Points, j)  {\
		IRIT_PT_SET((Pt), (Points)[1][j], (Points)[2][j], (Points)[3][j]);\
		IPC_PT_CLEAR(Pt);}\
 
#define IPC_VAL_SET(Val, Pt, j) { IPC_PT_CLEAR(Pt);\
                                  (Val)[1][j] = (Pt)[0];\
                                  (Val)[2][j] = (Pt)[1];\
                                  (Val)[3][j] = (Pt)[2];}\

#define IPC_IS_PREDICTOR_PERFECT(Predict) ((int)((Predict - IPC_SRF_PREDICTOR_ANGLES) & 0x01))

/* Arc predictor needs 3 points in order to predict 4th,	*/ 
/* so first 2 points predicted uniformlly. 			*/ 
#define IPC_ARC_INDX(j, i) (((j) >= 2 && i) ? (j)+1 : j) 
 
/* Next definitions generalize use in different compression tools. */  
 
/* Used with compression labrary. */ 
#define IPC_INPUT_BUFFER_LEN    (4000000) /* 2^30 bytes = 0.5 Mb */  
#define IPC_INPUT_BUFFER_FULL   (IPC_INPUT_BUFFER_LEN - 2 * sizeof(double)) 
#define IPC_OUTPUT_BUFFER_LEN   (int)(IPC_INPUT_BUFFER_LEN * 1.1 + 12) 
#define IPC_COMPRESS_LEVEL 9    /* between 0 .. 9 */ 

/* Set this flag, in order to write predicted data without compression. */
//#define IPC_WRITE_WITHOUT_COMPRESSION

/* File handling functions. */  
/* GZIP compressor is used. */
#if defined(IRIT_HAVE_GZIP_LIB) && !defined(IPC_WRITE_WITHOUT_COMPRESSION)
/* General file pointer for varios compression tools. */ 
typedef gzFile IpcFile; 
#define _IPC_OPEN_FILE(FileName, Mode) (gzopen(FileName, Mode)) 
#define _IPC_DOPEN_FILE(f, Mode)       (gzdopen(_fileno(f), Mode))
#define _IPC_INIT(f) { \
          if (gzsetparams(f, IPC_COMPRESS_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) \
	                   return IPC_ERROR_ZLIB;}
#define _IPC_CLOSE_FILE(f) gzclose(f)
#define _IPC_CLOSE_ALL(Handler) _IPC_CLOSE_FILE(_IPStream[Handler].f)
#define _IPC_WRITE(f, buffer, size) gzwrite(f, (buffer), (size))
#define _IPC_WRITE_BUFF(dest, destLen, source, sourceLen) \
    			compress(dest, &destLen, source, sourceLen);
#define _IPC_READ(f, buffer, size) gzread(f, (buffer), (size))
#else   /* HAVE_GZIP_LIB is not used */  
/* Alternative compresser is used.*/ 
/* General file pointer for varios compression tools. */ 
typedef FILE IpcFile; 
#define _IPC_OPEN_FILE(FileName, Mode) (fopen(FileName, Mode))
#define _IPC_DOPEN_FILE(f, Mode)       (fdopen(_fileno(f), Mode))
#define _IPC_INIT(f)
#define _IPC_CLOSE_FILE(f) fclose(f)
/* In Debug mode if used extern console compressor, */
/* compress temporary pre-processed file.           */
#define _IPC_CLOSE_ALL(Handler) { \
    _IPC_CLOSE_FILE(_IPStream[Handler].f); \
    if (!_IPStream[Handler].Read) \
        _IpcCompressMapFile(_IPStream[Handler].FileName); }
#define _IPC_WRITE(f, buffer, size) fwrite((buffer), sizeof(char), (size), f)
#define _IPC_WRITE_BUFF(dest, destLen, source, sourceLen) \
                        strncpy(dest, source, sourceLen); 
#define _IPC_READ(f, buffer, size) fread((buffer), sizeof(char), (size), f)

#define IPC_COMMAND_LINE     80
#define IPC_TEMP_FILENAME    "_ipc"

#endif  /* IRIT_HAVE_GZIP_LIB && !IPC_WRITE_WITHOUT_COMPRESSION */

#if defined(DEBUG_IPC) && defined(IRIT_HAVE_RAR_LIB)
/* Rar console compressor is used. */
#define _IPC_COMPRESS_COMMAND_LINE \
    "rar.exe a -y -m5 -ep %s %s > nul"
#define _IPC_DECOMPRESS_COMMAND_LINE \
    "rar.exe e -y %s.icd > nul"
#define _IPC_COMPRESS_EXT "rar"
#endif /* defined(DEBUG_IPC) && defined(IRIT_HAVE_RAR_LIB) */

#if defined(DEBUG_IPC) && defined(IRIT_HAVE_7ZIP_LIB)
/* 7zip console compressor is used. */
#define _IPC_COMPRESS_COMMAND_LINE \
    "7za a -y -bd %s %s > nul"
#define _IPC_DECOMPRESS_COMMAND_LINE \
    "7za e -y -bd %s.icd > nul"
#define _IPC_COMPRESS_EXT "7z"
#endif /* DEBUG_IPC && IRIT_HAVE_7ZIP_LIB */

/* Make sure it is in binary mode. */
#if defined(__WINNT__) 
#define _IPC_SET_BINARY_MODE(file) setmode(fileno((FILE *) file), _O_BINARY)
#else
#if defined(__OS2GCC__)
#define _IPC_SET_BINARY_MODE(file) setmode(fileno((FILE *) file), O_BINARY)
#else
#define _IPC_SET_BINARY_MODE(file) 
#endif /* __OS2GCC__ */
#endif  /* __WINNT__ */

#else  /* IPC_BIN_COMPRESSION */
    #define _IPC_OPEN_FILE(FileName, Mode) NULL
    #define _IPC_CLOSE_FILE(f)
    #define _IPC_CLOSE_ALL(Handler)
#endif /* IPC_BIN_COMPRESSION */

#endif /* IPC_LOC_H */
