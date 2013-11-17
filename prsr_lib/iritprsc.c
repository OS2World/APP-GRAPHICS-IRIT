/******************************************************************************
* Irtprscc.c - Provide a routine to compress Irit objects.                    *
*              Reference paper: "Coding of 3D virtual objects with NURBS"     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Yura Zharkovsky 2003                                             *
******************************************************************************/
#include <stdio.h>
#include <math.h>

#ifdef IPC_BIN_COMPRESSION

#ifdef __WINNT__
#include <io.h>
#include <direct.h>
#endif /* __WINNT__ */

#include "irit_sm.h"
#include "prsr_loc.h"
#include "cagd_lib.h"
#include "obj_dpnd.h"
#include "allocate.h"
#include "geom_lib.h"
#include "misc_lib.h"

/* Minimum buffer, used in Test mode. */
#define IPC_TEST_BUFF_SIZE              IPC_INPUT_BUFFER_LEN
#define IPC_TEST_COMP_SIZE              (int)(IPC_INPUT_BUFFER_LEN * 1.1)
#define IPC_TEST_PREDICTOR_FAVOR(x)     (x = (long)((double)x * 0.9))
#define IPC_BUFF_SIZE                   100*1024

/* Used in order to free allocated buffers when compression ends. */
#define IPC_TEST_FREE_BUFFERS   (-1)

/* Test mode constants, used in order to specify current test state. */
typedef enum {
    IPC_TEST_ON,
    IPC_TEST_OFF,
    IPC_TEST_SIZE,
    IPC_TEST_RUNNING
} IpcTestMode;

/* Global variable indicate that Test mode for predictors is on/off. */
IRIT_STATIC_DATA IpcTestMode 
    _IpcGlobalTestMode = IPC_TEST_OFF;

/* Global set of angles, used for angle predictor. */
CagdRType
    _IpcAnglesCos[IPC_ANGLES_SET_AUX][IPC_ANGLES_MAX] = 
    { 
        {0.0, 1.0}, 
        {-0.5, -1.0, -0.5, 1.0, 1.0, 1.0}
    };

int
    _IpcAnglesNum[IPC_ANGLES_SET_AUX] = { 2, 6 };

/* private declarations of predictors. */
static void IpcPredictPointsSrf(CagdRType *QntPoints[], 
		                IpcCtlPtStruct *Errs,
				CagdSrfStruct *Srf,
				IpcArgs *Args);
static void IpcPredictPointsAnglesSrf(CagdRType *QntPoints[], 
                                          IpcCtlPtStruct *Errs, 
                                          CagdSrfStruct *Srf, 
                                          IpcArgs *Args);
static void IpcPredictPointsUniform(IpcCtlPtStruct *PtDiffs, 
				    CagdRType *Points[], 
				    int NumPoints, 
				    IpcArgs *Args);
static void IpcPredictPointsMultiVar(MvarMVStruct *MV, 
				     IpcCtlPtStruct *PtDiffs, 
				     float QntError);
static void IpcPredictPointsTrivTV(CagdRType *QntPoints[], 
				   IpcCtlPtStruct *PtDiffs, 
				   TrivTVStruct *TV, 
				   float QntError);
static void IpcPredictPoints2D(IpcCtlPtStruct *PtDiffs, 
				CagdRType *Points[], 
				int NumPoints, 
				IpcArgs *Args);
static void IpcPredictPointsArc(IpcCtlPtStruct *PtDiffs, 
				CagdRType *Points[], 
				int NumPoints, 
				IpcArgs *Args);
static IrtBType IpcPredictPointsAnglesAux(IpcCtlPtStruct *Errs,
                                          CagdRType *QPts[],
                                          CagdRType *Pts[],
                                          int Block,
                                          int BlockLen,
                                          IpcArgs *Args);
static void IpcArcFindCenterAndRadius(IrtRType *R2, 
				      IrtPtType Pt0, 
				      IrtPtType Pt1, 
				      IrtPtType Pt2, 
				      IrtPtType Pt3);
static void IpcPredictPointsSetAngles(IpcCtlPtStruct *Errs,
                                      CagdRType *Points[],
                                      int NumPoints);
static void IpcPredictWeightsUniform(IpcCtlPtStruct *Errs, 
                                     CagdRType *Weights,
                                     IpcArgs *Args);
static void IpcCompressObjAux(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args);
static void IpcCompressObjAux1(IpcFile *f, 
			       IPObjectStruct *PObj, 
			       IrtBType ObjMap, 
			       IpcArgs *Args);
static int IpcCompressObj2(IpcFile *f, 
                              IPObjectStruct *PObj, 
                              float QntError);
static IrtBType IpcSetArgumentsObj(IpcFile *f, 
				   IPObjectStruct *PObj, 
				   IrtBType ObjMap, 
				   IpcArgs *Args);
static IpcPredictorType IpcSetArgumentsCrv(IpcFile *f,
                                           IPObjectStruct *PObj,
                                           IrtBType ObjMap,
                                           IpcArgs *Args);
static IpcPredictorType IpcSetArgumentsSrf(IpcFile *f,
                                           IPObjectStruct *PObj,
                                           IrtBType ObjMap,
                                           IpcArgs *Args);
static int IpcCompressTestAux(IpcTestMode Mode, const void *Data, long DataLen);
static int IpcCompressMode(void);
static char *IpcGetMapFileName(FILE *f);
static void _IpcFileToStream(const char *MapFileName, FILE *Stream);
static void IpcCompressContinue(IpcFile *f, IrtBType StreamContinue);
static void IpcCompressDynamicObj(IpcFile *f, 
				  IPObjectStruct *PObj, 
				  IrtBType Map, 
				  IpcArgs *Args);
static void IpcCompressDependencies(IpcFile *f, 
			            IPODObjectDpndncyStruct *Dpnds, 
				    IpcArgs *Args);
static void IpcCompressAttributes(IpcFile *f, 
				  IPAttributeStruct *Attr,  
				  IpcArgs *Args);
static int  IpcCompressUnion(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args);
static void IpcCompressSrf(IpcFile *f, CagdSrfStruct *Srfs, IpcArgs *Args);
static void IpcCompressTrimSrf(IpcFile *f, 
			       TrimSrfStruct *TrimSrfs, 
			       IpcArgs *Args);
static void IpcCompressTrimCrvList(IpcFile *f, 
				   TrimCrvStruct *Crvs, 
				   IpcArgs *Args);
static void IpcCompressTrimCrvSegList(IpcFile *f, 
				      TrimCrvSegStruct *TrimCrvSegs,
				      IpcArgs *Args);
static void IpcCompressCrvs(IpcFile *f, CagdCrvStruct *Crvs, IpcArgs *Args);
static void IpcCompressCtlPt(IpcFile *f, 
			     CagdCtlPtStruct *CtlPt, 
			     IpcArgs *Args);
static void IpcCompressKV(IpcFile *f, IpcKnotVectorStruct *KV, float QntError);
static void IpcCompressPoints(IpcFile *f, 
			      IpcCtlPtStruct *Points, 
			      float QntError);
static void IpcCompressPointsAngles(IpcFile *f, 
                                    IpcCtlPtStruct *Points, 
                                    float QntError);
static void IpcCompressString(IpcFile *f, const char *String);
static void IpcCompressStrings(IpcFile *f, IpcStrings *Strings);
static int  IpcCompressAsByte(IpcFile *f, void *Data, long DataLen);
static int  IpcCompressDoubleAsInt(IpcFile *f, void *Data, long DataLen);
static int  IpcCompress(IpcFile *f, const void *Data, long DataLen);
static int  IpcCompressAux(IpcFile *f, IrtBType *Buffer, long Size);
static long IpcCompressTest(IpcTestMode Mode);
static void IpcCompressTriSrfs(IpcFile *f, 
			       TrngTriangSrfStruct *TriSrfs, 
			       IpcArgs *Args);
static void IpcCompressInstances(IpcFile *f, 
	        		 IPInstanceStruct *Instances, 
				 IpcArgs *Args);
static void IpcCompressMultiVar(IpcFile *f, MvarMVStruct *MVs, IpcArgs *Args);
static void IpcCompressTrivTV(IpcFile *f, TrivTVStruct *TVs, IpcArgs *Args);
static void IpcCompressPolygons(IpcFile *f, 
				IPPolygonStruct *Pl, 
				IpcArgs *Args);
static void IpcCompressVertex(IpcFile *f, 
			      IPVertexStruct *PVertex, 
			      int IsSame, 
			      IpcArgs *Args);
static void IpcCompressPointFloat(IpcFile *f, IrtPtType *Pt, IpcArgs *Args);
static void IpcCompressModels(IpcFile *f, MdlModelStruct *Mdls, IpcArgs *Args);
static void IpcCompressTrimSrfList(IpcFile *f, 
				   MdlTrimSrfStruct *TrimSrfList, 
				   IpcArgs *Args);
static void IpcCompressLoopList(IpcFile *f, 
				MdlLoopStruct *LoopList, 
				IpcArgs *Args);
static void IpcCompressTrimSegList(IpcFile *f, 
				   MdlTrimSegStruct *TrimSegs, 
				   IpcArgs *Args);
static void IpcEncodeTriSrf(TrngTriangSrfStruct *TriSrf, 
		            IpcTriSrfStruct *IpcTriSrf);
static void IpcEncodePointsTriSrf(TrngTriangSrfStruct *TriSrf, 
				  IpcTriSrfStruct *IpcTriSrf);
static void IpcEncodeMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf);
static void IpcEncodePointsMultiVar(MvarMVStruct *MV, 
				    IpcCtlPtStruct *PtDiffs, 
				    IpcArgs *Args);
static void IpcEncodeTrivTV(TrivTVStruct *TV, IpcSrfStruct *IpcSrf) ;
static void IpcEncodeTrivTVPoints(TrivTVStruct *TV, IpcSrfStruct *IpcSrf);
static void IpcEncodeSrf(CagdSrfStruct *Srfs, IpcSrfStruct *IpcSrfs);
static void IpcEncodeCrv(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv);
static void IpcEncodeCrvPoints(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv);
static void IpcEncodeKnotVector(CagdRType *KnotVector, 
				int Order, 
				int Length, 
				CagdBType Periodic, 
                                IpcKnotVectorStruct *KnotVectorStruct, 
				float QntError);
static IpcKnotVectorType IpcUniqeEncodeKnotVector(CagdRType *KnotVector,
			                          int Len, 
			                          int Length,
			                          int Order, 
			        	          IpcKnotVectorStruct *KV);
static void IpcEncodePointsSrf(CagdSrfStruct *Srfs, IpcSrfStruct *IpcSrfs);
static void IpcWriteHeader(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args);
static void IpcFlush(IpcFile *f);
static float IpcSetQuantizationError(float QntError);
static IrtBType IpcSetObjectMap(int Version, IPObjectStruct *PObj);
static void IpcInitWeightsTrivTV(CagdRType *QntPoints[], TrivTVStruct *TV);
static CagdRType IpcFindDeltaTrivTV(TrivTVStruct *TV, 
				    IpcCtlPtStruct *Errs, 
				    float QntError);
static void IpcSaveInitialValuesTrivTV(IpcCtlPtStruct *Errs, TrivTVStruct *TV);
static void IpcInitPointsTrivTV(CagdRType *QntPoints[], TrivTVStruct *TV);
static void IpcAnalizeBreakValues(CagdRType **BreakValues, 
				  int *NumBreakValues, 
				  CagdRType *KnotVector, 
				  int Len);
static CagdRType IpcNormalizeBreakvalues(CagdRType *BreakValues, 
					 int NumBreakValue);
static int* IpcFindMultiplicity(int NumBreakValues, 
				CagdRType *KnotVector, 
				int Len); 
static CagdRType IpcFindDeltaK(CagdRType *BreakValues, 
			       int len, 
			       float QntError);
static int* IpcKVFindErrorsIndexes(CagdRType *BreakValues, 
				   IpcKnotVectorStruct *KVStruct,
                                   CagdRType *KnotVector, 
				   int Order, 
				   int Length, 
				   float QntError);
static void IpcPointsAllocateMemory(CagdRType *CtlPtErrors[], 
				   IpcSrfStruct *IpcSrf, 
				   int extend);
static void IpcQntIndexesAllocateMemory(int *qntIndexes[], 
					IpcSrfStruct *IpcSrf);
static void IpcInitPoints(CagdRType *QntPoints[], CagdSrfStruct *Srf);
static void IpcInitWeights(CagdRType *QntPoints[], CagdSrfStruct *Srf);
static CagdRType IpcFindDelta(CagdSrfStruct *Srf, 
			      IpcCtlPtStruct *Errs, 
			      float QntError);
static void IpcSaveInitialValues(IpcCtlPtStruct *Errs, CagdSrfStruct *Srf);
static void IpcAngleSetBlockPredictor(IpcCtlPtStruct *Errs,
                                      int Block,
                                      int Predict,
                                      CagdBType Perfect);

/******************************************************************************
* DESCRIPTION:								      M
*  Compress a given IPObject to a file.                                       M
*                                                                             *
* PARAMETERS:                                                                 M
*   FileName:   Name of a compression file to save in.                        M
*   PObj:       Irit format objects list.                                     M
*   QntError:   Quantization step between(0..1). 			      M
*               Specifies maximum error for values.    		              M
*               IPC_QUANTIZATION_NONE - no quanization is used.		      M
*                                                                             *
* RETURN VALUE:                                                               M
*   int:        Internal Error code.                                          M
*                                                                             *
* SEE ALSO:                                                                   M
*   IpcDecompressObjFromFile.                                                 M
*                                                                             *
* KEYWORDS:                                                                   M
*   IpcCompressObjToFile, files, parser, predictor, compress, quantization.   M
******************************************************************************/
int IpcCompressObjToFile(const char *FileName,
			 IPObjectStruct *PObj,
			 float QntError)
{
    IpcFile *f;
    int code;

    /* Open a file for write. */
    if ((f = _IPC_OPEN_FILE(FileName, IP_WRITE_BIN_MODE)) == NULL)
        return IPC_ERROR_OPEN_FILE;

    code = IpcCompressObj2(f, PObj, QntError);
    
    _IPC_CLOSE_FILE(f);
    
    /* Use extern compressor if needed. */
    _IpcCompressMapFile(FileName);

    return code;
}

/******************************************************************************
* DESCRIPTION:                                                                M
*  Compress a given IPObject to a file using Handler.                         M
*                                                                             *
* PARAMETERS:                                                                 M
*   Handler:    A handler to the open stream.				      M
*   PObj:       Irit format objects list.                                     M
*                                                                             *
* RETURN VALUE:                                                               M
*   int:        Internal Error code.                                          M
*                                                                             *
* SEE ALSO:                                                                   M
*   IpcDecompressObj.                                                         M
*                                                                             *
* KEYWORDS:                                                                   M
*   IpcCompressObj, files, parser, predictor, compress, quantization.         M
******************************************************************************/
int IpcCompressObj(int Handler, IPObjectStruct *PObj)
{
    int Code;
    IpcFile *f;
    char
        *MapFileName = NULL;

    if (_IPStream[Handler].f == stdout) {
        /* Write to output file in binary format. */
        _IPC_SET_BINARY_MODE(stdout);

        /* Create new temparary file. */
        if (MapFileName = IpcGetMapFileName(_IPStream[Handler].f)) {
	    if ((f = _IPC_OPEN_FILE(MapFileName, IP_WRITE_BIN_MODE)) == NULL) {
	        IritFree(MapFileName);
                return IPC_ERROR_OPEN_FILE;
	    }
        }
	else { 
            if ((f = _IPC_DOPEN_FILE(stdout, IP_WRITE_BIN_MODE)) == NULL)
                return IPC_ERROR_OPEN_FILE;
        }
    }
    else {
        /* file is not stdout. */
        f = (IpcFile *) _IPStream[Handler].f;
    }

    /* Compress a given IPObject to a file. */
    Code = IpcCompressObj2(f, PObj, _IPStream[Handler].QntError);

    if (_IPStream[Handler].f == stdout) {
        /* Close temparary file. A main file in Handle is not closed.*/
        _IPC_CLOSE_FILE(f);
        if (MapFileName) {
            /* In Debug mode, used extern console compressor. */
            _IpcCompressMapFile(MapFileName);
            _IpcFileToStream(MapFileName, stdout);
	    IritFree(MapFileName);
        }
    }

    return Code;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*  Compress a given IPObject to a file.                                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   Name of a compression file to save in.                        *
*   PObj:       Irit format objects list.                                     *
*   QntError:   Quantization step between(0..1). 			      *
*               Specifies maximum error for values.    		              *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.		      *
*                                                                             *
* RETURN VALUE:                                                               *
*   int:        Internal Error code.                                          *
******************************************************************************/
static int IpcCompressObj2(IpcFile *f, IPObjectStruct *PObj, float QntError)
{
    int Code;
    IpcArgs Args;

    /* Its the setjmp itself call! */
    if (Code = setjmp(_IPLongJumpBuffer) != 0) { 
        _IPLongJumpActive = FALSE;
        return Code;
    }
    _IPLongJumpActive = TRUE;

    /* Initialize different compression parameters. */
    _IPC_INIT(f);

    /* Set arguments of a compression state. */
    Args.QntError = IpcSetQuantizationError(QntError);

    /* Default predictors. */
    Args.SrfPredictor    =  IPC_DEFAULT_SRF_PREDICTOR;    
    Args.CrvPredictor    =  IPC_DEFAULT_CRV_PREDICTOR;   
    Args.TriSrfPredictor =  IPC_DEFAULT_TRISRF_PREDICTOR;
    Args.TVPredictor     =  IPC_DEFAULT_TV_PREDICTOR;
    Args.MVPredictor     =  IPC_DEFAULT_MV_PREDICTOR;

    /* Set indicator if the stream is big endian or little endian style. */
    Args.SwapEndian = _IPThisLittleEndianHardware();

    /* Write header of a compressed file. */
    IpcWriteHeader(f, PObj, &Args);

    /* Compress all objects. */
    IpcCompressObjAux(f, PObj, &Args);

    /* Write the last block and stream end to the file. */
    IpcFlush(f);

    _IPLongJumpActive = FALSE;

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*  Checks that quntiztion error is valid.                                     *
*  Also rounds quantization error to the closest pow of 1/10.                 *
*  If a quantization error is not pow of 1/10 the effectiveness 	      *
*  of the compressor is decreasing.                                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntError:   Quantization step between(0..1). 			      *
*   		Specifies maximum error for values.    			      *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.	              *
*                                                                             *
* RETURN VALUE:                                                               *
*   float:      Quantization step.                                            *
******************************************************************************/
static float IpcSetQuantizationError(float  QntError)
{
    int Digits;

    if (!IPC_QUANTIZATION_USED(QntError))
        return QntError;

    if (QntError <= 0 || QntError >= 1)
        _IpcErrHandler(IPC_ERROR_INVALID_QUANTIZER_VALUE);
    
    for (Digits = 0; QntError <= 0.9f; QntError *= 10, Digits += 1);
    
    QntError= (float) (1.0 / pow(10, Digits));
    
    return QntError; 
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Get file name without extention.                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name.                                                    *
*   WithPath:   File name with full path.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   char*: File name without extention.                                       *
******************************************************************************/
const char *_IpcGetFileNameNoExt(const char *FileName, int WithPath)
{
#if !defined(IRIT_HAVE_GZIP_LIB) && defined(__WINNT__)
    char Drive[_MAX_DRIVE], Dir[_MAX_DIR], Fname[_MAX_FNAME], Ext[_MAX_EXT], 
         FullPathName[IPC_COMMAND_LINE], *p;
    
    _splitpath(FileName,Drive, Dir, Fname, Ext);

    if (WithPath) {
        strcpy(FullPathName, FileName);
        *(p = strrchr(FullPathName, '.')) = '\0';
        return IritStrdup(FullPathName);
    }
    else
        return IritStrdup(Fname);
#endif /* !IRIT_HAVE_GZIP_LIB && __WINNT__ */

    return FileName;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress mapping file using extern compressor.                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name of a extern file to compress.                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   char* : Archive name.                                                     *
******************************************************************************/
const char *_IpcCompressMapFile(const char *FileName)
{
    const char 
        *ArchiveName = _IpcGetFileNameNoExt(FileName, FALSE);

#if !defined(IRIT_HAVE_GZIP_LIB) && defined(__WINNT__)
    /* Alternative compressor is defined. */
    char Command[IPC_COMMAND_LINE];

    /* Rename input file. */
    rename(FileName, IPC_TEMP_FILENAME);

    /* Compress mapping file. */
    sprintf(Command, _IPC_COMPRESS_COMMAND_LINE,
	    ArchiveName, IPC_TEMP_FILENAME);
    system(Command);

    /* Remove mapping file. */ 
    remove(IPC_TEMP_FILENAME);

    /* Rename archive to icd file. */
    sprintf(Command, "%s.%s", ArchiveName, _IPC_COMPRESS_EXT);  
    rename(Command, FileName);
#endif /* !IRIT_HAVE_GZIP_LIB && __WINNT__ */

    return ArchiveName;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Print map file contest to stream.                                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name of a extern file to compress.                       *
*   Stream:     Open stream.                                                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void _IpcFileToStream(const char *MapFileName, FILE *Stream)
{
    FILE *f;
    size_t bytesread;
    IrtBType Buff[IPC_BUFF_SIZE];

    rewind(stdout);
    
    if ((f = fopen(MapFileName, "rb")) == NULL)
        return;
   
    while (!feof(f)) {
        bytesread = fread(Buff, sizeof(IrtBType), IPC_BUFF_SIZE, f);
        fwrite(Buff, sizeof(IrtBType), bytesread, Stream);
    };

    fclose(f);
    
    /* Remove map file. */
    remove(MapFileName);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Get map file name.                                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name of a extern file to compress.                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   char *: Map file name.                                                    *
******************************************************************************/
static char *IpcGetMapFileName(FILE *f)
{
#ifndef IRIT_HAVE_GZIP_LIB
    char TempFile[IPC_COMMAND_LINE];

    /* Create temporary file name. */
    sprintf(TempFile, "%s", IPC_TEMP_FILENAME);
    return IritStrdup(TempFile);
#endif /* IRIT_HAVE_GZIP_LIB */

    return NULL;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Convert each input int to IrtBType and compress it.                       *
*   									      *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Input Buffer of integers.                                     *
*   DataLen:    Number of elements in input buffer.                           *
*   									      *
*   RETURN VALUE:                                                             *
*   int:        Error code.                                                   *
******************************************************************************/
static int IpcCompressAsByte(IpcFile *f, void *Data, long DataLen)
{
    int i;
    IrtBType Byte;

    for (i = 0; i < DataLen; ++i) {
        Byte = (IrtBType) ((int*)Data)[i];
        IpcCompress(f, &Byte, sizeof(IrtBType));
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Convert each input int to Float and compress it.                          *
*   									      *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Input Buffer of integers.                                     *
*   DataLen:    Number of elements in input buffer.                           *
*   									      *
*   RETURN VALUE:                                                             *
*   int:        Error code.                                                   *
******************************************************************************/
static int IpcCompressDoubleAsInt(IpcFile *f, void *Data, long DataLen)
{
    int i, Quant;
    CagdRType Real;

    for (i = 0; i < DataLen; ++i) {
        Real = ((CagdRType *)Data)[i];
        if (IRIT_APX_UEQ(Real, 0.0))
            Real = 0.0;
        Quant = IRIT_REAL_TO_INT(Real / IPC_QUANTIZATION_DEFAULT);
        IpcCompress(f, &Quant, sizeof(int));
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Write data to the buffer. If the buffer is full 			      *
*   compress it and flush to file.      				      *
*   									      *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Source buffer.                                                *
*   DataLen:    Size of the source buffer.                                    *
*   									      *
*   RETURN VALUE:                                                             *
*   int:        Error code.                                                   *
******************************************************************************/
static int IpcCompress(IpcFile *f, const void *Data, long DataLen)
{
    IRIT_STATIC_DATA IrtBType Buffer[IPC_INPUT_BUFFER_LEN];
    IRIT_STATIC_DATA
	int Size = 0;
    int j;

    /* In Test mode. */
    if (IpcCompressMode()) {
        IpcCompressTestAux(IPC_TEST_RUNNING, Data, DataLen);
        return 0;
    }
    
    /* Flush */
    if (Data == NULL) {
        /* Flush the last buffer. */
        IpcCompressAux(f, Buffer, Size);
        /* Set a stream end. */
        IpcCompressAux(f, NULL, 0);
        Size = 0;
        return 0;
    }

    for (j = 0; j < DataLen; j++) {
        if (Size >= IPC_INPUT_BUFFER_FULL) { 
            IpcCompressAux(f, Buffer, Size);
            Size = 0;
        }

        /* Save data in main buffer. */
        Buffer[Size++] = ((IrtBType*)Data)[j];
    }

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcCompress            		     *
*****************************************************************************/
static int IpcCompressAux(IpcFile *f, IrtBType *SrcBuffer, long SrcSize) 
{
    unsigned long DestSize;

    /* Write block length. */
    _IPC_WRITE(f, &SrcSize, sizeof(long));

    /* Write end of stream. */
    if (SrcSize == 0) 
        return 0;

    /* Write compressed data to file. */
    DestSize = (unsigned long)_IPC_WRITE(f, SrcBuffer, SrcSize);

    return DestSize;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Enables/Disables compression Test.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*       OffOn:  TRUE = Enable, FALSE = Disable                                *
*                                                                             *
* RETURN VALUE:                                                               *
*       int:    Size of a compressed object.                                  *
******************************************************************************/
static long IpcCompressTest(IpcTestMode Mode) 
{ 
    /* Switch test mode. */ 
    _IpcGlobalTestMode = Mode; 
    return IpcCompressTestAux(Mode, NULL, 0); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Indicator of compression Test mode.                                       *
*                                                                             *
* PARAMETERS:                                                                 *
*                                                                             *
* RETURN VALUE:                                                               *
*       int:  TRUE = Enable, FALSE = Disable                                 *
*									      *
******************************************************************************/
static int IpcCompressMode(void)  
{  
    return !(_IpcGlobalTestMode == IPC_TEST_OFF); 
} 
 
/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcCompressTest            		     *
*****************************************************************************/
static int IpcCompressTestAux(IpcTestMode Mode, const void *Data, long DataLen)
{ 
    IRIT_STATIC_DATA 
	IrtBType TestBuff[IPC_TEST_BUFF_SIZE], CompBuff[IPC_TEST_COMP_SIZE];
    IRIT_STATIC_DATA 
	long BuffSize = 0; 
    long j,
	CompSize = IPC_TEST_COMP_SIZE; 
 
    /* Empty test buffer. */ 
    if (BuffSize + DataLen >= IPC_TEST_BUFF_SIZE)  
            BuffSize = 0; 

    /* Turn Test on. */ 
    if (Mode == IPC_TEST_ON) { 
	    BuffSize = 0; 
	    return 0; 
    } 
    /* Compress object and return compressed size of the test buffer. */ 
    if (Mode == IPC_TEST_SIZE) { 
	    _IPC_WRITE_BUFF(CompBuff, CompSize, TestBuff, BuffSize); 
	    BuffSize = 0; 
	    return CompSize; 
    }	 
    /* Save data in test buffer. */ 
    if (Mode == IPC_TEST_RUNNING)  
	    for (j = 0; j < DataLen; j++)  
		    TestBuff[BuffSize++] = ((IrtBType*)Data)[j]; 
    return 0; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*  Write to the file indicator if stream of same values is ended or not.      *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:               A handler to compressed file.                            *
*   StreamContinue:  FALSE = Stream is ended TRUE = Stream is not ended.      *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressContinue(IpcFile *f, IrtBType StreamContinue) 
{ 
    IpcCompress(f, &StreamContinue, sizeof(IrtBType));  
} 

/******************************************************************************
* DESCRIPTION:                                                                *
* Write the last block and stream end to the file.                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:               A handler to compressed file.                            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcFlush(IpcFile *f) 
{
    IpcCompress(f, NULL, 0);  
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Write header of a compressed file.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcWriteHeader(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args) 
{ 
    int Data; 

    /* Write information if a stream is a big endian or little endian. */
    _IPC_WRITE(f, &(Args -> SwapEndian), sizeof(Args -> SwapEndian));
 
    /* Indicator that a file of compress type. */  
    _IPC_WRITE(f, "IC", sizeof(char)*2); 
 
    /* Write a Version of the compressed file. */ 
    Data = IPC_VERSION; 
    _IPC_WRITE(f, &Data, sizeof(int));     
 
    /* Write compress level. */ 
    _IPC_WRITE(f, &(Args -> QntError), sizeof(float)); 
 
    /* Write a predictor Version. */ 
    _IPC_WRITE(f, &(Args -> SrfPredictor),    sizeof(IpcPredictorType)); 
    _IPC_WRITE(f, &(Args -> CrvPredictor),    sizeof(IpcPredictorType)); 
    _IPC_WRITE(f, &(Args -> TriSrfPredictor), sizeof(IpcPredictorType)); 
    _IPC_WRITE(f, &(Args -> TVPredictor) ,    sizeof(IpcPredictorType)); 
    _IPC_WRITE(f, &(Args -> MVPredictor) ,    sizeof(IpcPredictorType));
} 
 
/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcCompressObj           		     *
*****************************************************************************/
static void IpcCompressObjAux(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args) 
{ 
    IrtBType ObjPredictMap, ObjMap; 
 
    /* Used in order not to nast objects. */ 
    if (IpcCompressMode())  
	    return; 
 
    ObjMap = IpcSetObjectMap(IPC_VERSION, PObj); 
     
    /* Do not compress undefined object. */ 
    if (PObj -> ObjType == IP_OBJ_UNDEF) 
        return; 

    /* Select predictors that will be used to compress current object.*/ 
    ObjPredictMap = IpcSetArgumentsObj(f, PObj, ObjMap, Args); 
        
    /* Save predictors map. */ 
    IpcCompress(f, &ObjPredictMap, sizeof(IrtBType)); 
        
    /* Compress single object. */     
    IpcCompressObjAux1(f, PObj, ObjMap, Args); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*    Save information about predictors that used to compress current object.  *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   ObjMap:     Defines the way an object internal fields are compressed.     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   IrtBType:   Map indicates what predictors are used to compress objetc.    *
******************************************************************************/
static IrtBType IpcSetArgumentsObj(IpcFile *f, 
                                   IPObjectStruct *PObj, 
                                   IrtBType ObjMap, 
                                   IpcArgs *Args)
{
    IrtBType ObjPredictMap;
    IpcPredictorType
        BestCrvPredictor = IpcSetArgumentsCrv(f, PObj, ObjMap, Args), 
        BestSrfPredictor = IpcSetArgumentsSrf(f, PObj, ObjMap, Args);
 
    ObjPredictMap = BestSrfPredictor;
    ObjPredictMap <<= 4;
    ObjPredictMap |= BestCrvPredictor;

    return ObjPredictMap;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Set best curve predictor that used to compress current object.           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   ObjMap:     Defines the way an object internal fields are compressed.     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   IpcPredictorType:   Best curve predictor.                                 *
******************************************************************************/
static IpcPredictorType IpcSetArgumentsCrv(IpcFile *f,
                                           IPObjectStruct *PObj,
                                           IrtBType ObjMap,
                                           IpcArgs *Args)
{
    /* The buffer used in order to save sizes of by different predictors. */ 
    long Indx,
        SizeBuffer[32] = {0},
        MinSize = IPC_TEST_BUFF_SIZE; 
    IpcPredictorType 
        BestCrvPredictor = Args -> CrvPredictor,  
        LastCrvPredictor = IPC_CRV_PREDICTOR_UNIFORM;

    if (IPC_CRV_NUM_PREDICTORS <= 1)
        return (Args -> CrvPredictor = IPC_CRV_PREDICTOR_UNIFORM);

    /* Memory compression. */ 
    /* Compress object without actually writing to the compressed file.*/ 
    IpcCompressTest(IPC_TEST_ON); 
             
    for (Indx = 1; Indx <= IPC_CRV_NUM_PREDICTORS; Indx++) { 
        /* Choose best curve predictor. */ 
        switch (Indx) { 
            case 1: /* Test uniform curve predictor. */ 
                Args -> CrvPredictor = IPC_CRV_PREDICTOR_UNIFORM; 
		break; 
        
            case 2: /* Test Arc curve predictor. */ 
	        Args -> CrvPredictor = IPC_CRV_PREDICTOR_ARC; 
		break; 

            case 3: /* Test 2D curve predictor. */ 
	        Args -> CrvPredictor = IPC_CRV_PREDICTOR_2D; 
		break;

            default: 
	        break; 
        } 

        /* Compress single object in test mode. */     
        IpcCompressObjAux1(f, PObj, ObjMap, Args); 

        SizeBuffer[Indx] = IpcCompressTest(IPC_TEST_SIZE); 
        
        /* If previos object predictor is same as current one make it fovour.*/
        if (Args -> CrvPredictor == LastCrvPredictor) 
            IPC_TEST_PREDICTOR_FAVOR(SizeBuffer[Indx]); 

        if (SizeBuffer[Indx] < MinSize) { 
            MinSize= SizeBuffer[Indx]; 
            BestCrvPredictor = LastCrvPredictor = Args -> CrvPredictor; 
        } 
    } 

    /* Reset memory compression to pre-compressed state. */ 
    IpcCompressTest(IPC_TEST_OFF); 
     
    return (Args -> CrvPredictor = BestCrvPredictor); 
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Set best surface predictor that used to compress current object.         *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   ObjMap:     Defines the way an object internal fields are compressed.     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   IpcPredictorType:   Best surface predictor.                               *
******************************************************************************/
static IpcPredictorType IpcSetArgumentsSrf(IpcFile *f,
                                           IPObjectStruct *PObj,
                                           IrtBType ObjMap,
                                           IpcArgs *Args)
{
    /* The buffer used in order to save sizes of by different predictors. */ 
    long Indx,
        SizeBuffer[32] = {0},
        MinSize = IPC_TEST_BUFF_SIZE; 
    IpcPredictorType BestSrfPredictor, DefaultSrfPredictor;

    BestSrfPredictor = DefaultSrfPredictor = Args -> SrfPredictor;

    if (IPC_SRF_NUM_PREDICTORS <= 1)
        return Args -> SrfPredictor;

    /* Memory compression. */ 
    /* Compress object without actually writing to the compressed file.*/ 
    IpcCompressTest(IPC_TEST_ON); 
             
    for (Indx = 1; Indx <= IPC_SRF_NUM_PREDICTORS; Indx++) { 
        /* Choose best surface predictor. */ 
	switch (Indx) { 
            case 1: /* Test parallelogram surface predictor. */
                Args -> SrfPredictor = IPC_SRF_PREDICTOR_PARALLELOGRAM; 
		break; 
        
            case 2: /* Test angle surface predictor. */
                Args -> SrfPredictor = IPC_SRF_PREDICTOR_ANGLES;
		break; 

            default: 
                break; 
        } 

        /* Compress single object in test mode. */     
        IpcCompressObjAux1(f, PObj, ObjMap, Args); 

        SizeBuffer[Indx] = IpcCompressTest(IPC_TEST_SIZE); 

        /* If previos object predictor is same as current one make it fovour.*/
        if (Args -> SrfPredictor == DefaultSrfPredictor)
            IPC_TEST_PREDICTOR_FAVOR(SizeBuffer[Indx]);

        if (SizeBuffer[Indx] < MinSize) { 
            MinSize= SizeBuffer[Indx]; 
            BestSrfPredictor = Args -> SrfPredictor;
        } 
    } 

    if (MinSize > 50) {
        BestSrfPredictor == IPC_SRF_PREDICTOR_ANGLES ? 
                                  printf("Angles") : printf("Parallelogram");
        printf("\t%d\t%d\n", SizeBuffer[1], SizeBuffer[2]);
    }

    /* Reset memory compression to pre-compressed state. */
    IpcCompressTest(IPC_TEST_OFF); 
     
    return (Args -> SrfPredictor = BestSrfPredictor);
}
 
/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcCompressObjAux1            		     *
*****************************************************************************/
static void IpcCompressObjAux1(IpcFile *f, 
                               IPObjectStruct *PObj, 
                               IrtBType ObjMap, 
                               IpcArgs *Args)
{ 
    IpcCompress(f, &ObjMap, sizeof(IrtBType)); 
         
    IpcCompressAsByte(f, &(PObj -> ObjType), 1); 
         
    /* Compress Union dynamic part. */ 
    if (IpcCompressUnion(f, PObj, Args)) { 
        /* Compress object's dynamic part. */ 
        IpcCompressDynamicObj(f, PObj, ObjMap, Args); 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Set defined fields of the object. Different values of the map can be used *
*   in order to debug or choose which parts of the object would be saved.     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Version:    Version of the IPC Format.                                    *
*   PObj:       Irit format objects list.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   IrtBType:   Mapping of the object.                                        *
******************************************************************************/
static IrtBType IpcSetObjectMap(int Version, IPObjectStruct *PObj)  
{ 
    IrtBType
	Map = IPC_DEFAULT_MAP; 
     
    switch (Version) { 
        case IPC_VERSION_1:  
            switch (PObj -> ObjType) { 
                case IP_OBJ_SURFACE: 
                    if (CAGD_IS_BSPLINE_SRF(PObj -> U.Srfs)) 
                        Map = IPC_BSPLINE_SRF_MAP; 
                    break; 
            } 
        
        default: 
	    Map = IPC_DEFAULT_MAP; 
            break;                 
    } 
 
    return Map; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress object's dynamic part.                                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   Map:        Mapping of the object.                                        *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressDynamicObj(IpcFile *f, 
                                  IPObjectStruct *PObj, 
                                  IrtBType Map, 
                                  IpcArgs *Args) 
{ 
    if (Map & IPC_BITMAP_BBOX) 
        if (IRIT_PT_EQ_ZERO(PObj -> BBox[0]) &&
	    IRIT_PT_EQ_ZERO(PObj -> BBox[1])) {
            IpcCompressContinue(f, TRUE);
            IpcCompress(f, &(PObj -> BBox), sizeof(IrtBboxType));
        }
        else
            IpcCompressContinue(f, FALSE);
 
    if (Map & IPC_BITMAP_COUNT) 
        IpcCompress(f, &(PObj -> Count), sizeof(int)); 
 
    if (Map & IPC_BITMAP_TAGS) 
        IpcCompress(f, &(PObj -> Tags), sizeof(int)); 
 
    if (Map & IPC_BITMAP_NAME)  
        IpcCompressString(f, IP_GET_OBJ_NAME(PObj));  
 
    if (Map & IPC_BITMAP_ATTR) 
        IpcCompressAttributes(f, PObj -> Attr, Args); 
     
    if (Map & IPC_BITMAP_DPNDS) 
        IpcCompressDependencies(f, PObj -> Dpnds, Args); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress object's attributes.                                             *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Attr:       Object's attributes.                                          *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressAttributes(IpcFile *f, 
                                  IPAttributeStruct *Attr, 
                                  IpcArgs *Args) 
{ 
    for (; Attr != NULL; Attr = Attr -> Pnext) { 
        /* Indicates that stream of attributes is not ended. */ 
        IpcCompressContinue(f, TRUE); 
         
        IpcCompressString(f, AttrGetAttribName(Attr)); 
        IpcCompressAsByte(f, &(Attr -> Type), 1); 
 
        switch (Attr -> Type) { 
            case IP_ATTR_NONE:  
                break; 
 
            case IP_ATTR_INT:  
                IpcCompress(f, &(Attr -> U.I), sizeof(int)); 
                break; 
 
            case IP_ATTR_REAL:  
                IpcCompress(f, &(Attr -> U.R), sizeof(IrtRType)); 
                break; 
 
            case IP_ATTR_UV:  
                IpcCompress(f, &(Attr -> U.UV), 2 * sizeof(float)); 
                break; 
 
            case IP_ATTR_STR:  
                IpcCompressString(f, Attr -> U.Str); 
                break; 
 
            case IP_ATTR_OBJ: 
                IpcCompressObjAux(f, Attr -> U.PObj, Args); 
                break; 
 
            case IP_ATTR_PTR:  
            case IP_ATTR_REFPTR: 
                IpcCompress(f, &(Attr -> U.Ptr), sizeof(VoidPtr)); 
                break; 
        } 
    } 
 
    /* Indicates that stream of attributes is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress object's dependencies.                                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Dpnds:      Object's dependencies.                                        *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressDependencies(IpcFile *f, 
                                    IPODObjectDpndncyStruct *Dpnds, 
                                    IpcArgs *Args) 
{ 
    for (; Dpnds != NULL; Dpnds = Dpnds -> Pnext) { 
        /* Indicates that stream of dependencies is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        IpcCompressAttributes(f, Dpnds -> Attr, Args); 
        IpcCompressStrings(f, (IpcStrings*)(Dpnds  -> ObjParams)); 
        IpcCompressStrings(f, (IpcStrings*)(Dpnds  -> ObjDepends)); 
        IpcCompressString(f, Dpnds -> EvalExpr); 
        IpcCompress(f, &(Dpnds -> EvalIndex), sizeof(int)); 
        IpcCompress(f, &(Dpnds -> NumVisits), sizeof(int)); 
        IpcCompress(f, &(Dpnds -> NumParams), sizeof(int)); 
    } 
    /* Indicates that stream of dependencies is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress string.                                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   String:     String to compress.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressString(IpcFile *f, const char *String)
{ 
    int Size = 0; 
 
    if (String) { 
        /* In order to distinguish between Str == NULL and Str == "" add+1. */
        Size = (int) strlen(String)+1; 
        IpcCompress(f, &Size, sizeof(int)); 
        Size--; 
        IpcCompress(f, String, Size); 
    }
    else { 
        IpcCompress(f, &Size, sizeof(int)); 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress strings list.                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Strs:       Strings list to compress.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
*                                                                             *
******************************************************************************/
static void IpcCompressStrings(IpcFile *f, IpcStrings *Strs)  
{ 
    for (; Strs != NULL; Strs = Strs -> Pnext) {  
        /* Indicates that stream of names is not ended. */     
        IpcCompressContinue(f, TRUE); 
        IpcCompressString(f, Strs -> Name); 
    } 

    /* Indicates that stream of names is ended. */     
    IpcCompressContinue(f, FALSE); 
} 
  
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress object's union(dynamic part). This function defines what kind    *
*   of surface the compreesor can handle.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   int:        TRUE == Object is compressed, FALSE == Object  not compressed.*
*                                                                             *
******************************************************************************/
static int IpcCompressUnion(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args) 
{ 
    IPObjectStruct *PObjTmp; 
    int i; 
 
    switch (PObj -> ObjType) { 
        case IP_OBJ_SURFACE:  
            IpcCompressSrf(f, PObj -> U.Srfs, Args); 
            break; 
        case IP_OBJ_TRIVAR:  
            IpcCompressTrivTV(f, PObj -> U.Trivars, Args); 
            break; 
        case IP_OBJ_MULTIVAR:  
            IpcCompressMultiVar(f, PObj -> U.MultiVars, Args); 
            break; 
        case IP_OBJ_TRISRF:  
            IpcCompressTriSrfs(f, PObj -> U.TriSrfs, Args); 
            break; 
        case IP_OBJ_TRIMSRF:  
            IpcCompressTrimSrf(f, PObj -> U.TrimSrfs, Args); 
            break; 
        case IP_OBJ_POLY:  
            IpcCompressPolygons(f, PObj -> U.Pl, Args); 
            break; 
        case IP_OBJ_CURVE:  
            IpcCompressCrvs(f, PObj -> U.Crvs, Args); 
            break; 
        case IP_OBJ_MATRIX: 
            IpcCompress(f, PObj -> U.Mat, sizeof(IrtHmgnMatType));  
            break; 
        case IP_OBJ_NUMERIC: 
            IpcCompress(f, &(PObj -> U.R), sizeof(IrtRType));  
            break; 
        case IP_OBJ_POINT: 
            IpcCompress(f, PObj -> U.Pt, sizeof(IrtPtType));  
            break; 
        case IP_OBJ_VECTOR: 
            IpcCompress(f, PObj -> U.Vec, sizeof(IrtVecType));  
            break; 
        case IP_OBJ_PLANE: 
            IpcCompress(f, PObj -> U.Plane, sizeof(IrtPlnType));  
            break; 
        case IP_OBJ_CTLPT: 
            IpcCompressCtlPt(f, &(PObj -> U.CtlPt), Args);  
            break; 
        case IP_OBJ_STRING: 
            IpcCompressString(f, PObj -> U.Str); 
            break; 
        case IP_OBJ_INSTANCE: 
            IpcCompressInstances(f, PObj -> U.Instance, Args); 
            break; 
        case IP_OBJ_MODEL: 
            IpcCompressModels(f, PObj -> U.Mdls, Args); 
            break; 
        case IP_OBJ_LIST_OBJ: 
            for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
                if (PObjTmp == PObj)
		    IP_FATAL_ERROR(IP_ERR_LIST_CONTAIN_SELF);
                else {
                    /* List of objects is not ended. */
                    IpcCompressContinue(f, TRUE);
		    IpcCompressObjAux(f, PObjTmp, Args);
                }
	    }
            /* List of objects is ended. */
            IpcCompressContinue(f, FALSE);
            break; 
        default: 
            printf("The object %s is not compressed.\n", IP_GET_OBJ_NAME(PObj)); 
            return FALSE; 
    } 
 
    return TRUE; 
} 
  
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress tri-variate surfaces to file.                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   TVs:        tri-variate surfaces to compress.                             *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTrivTV(IpcFile *f, TrivTVStruct *TVs, IpcArgs *Args) 
{ 
    /* Create new encoding structure. */ 
    IpcSrfStruct IpcSrf;  
 
    /* Encode */ 
    for (; TVs != NULL; TVs = TVs -> Pnext) { 
        /* Indicates that stream of tri-variate is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Clear encoding structure. */ 
        memset(&IpcSrf, 0, sizeof(IpcSrfStruct)); 
 
        /* Write GType and PType. */ 
        IpcCompress(f, &(TVs -> GType), sizeof(TrivGeomType)); 
        IpcCompress(f, &(TVs -> PType), sizeof(CagdPointType)); 
 
        /* Encode surface. */ 
        IpcSrf.Args = Args; 
        IpcEncodeTrivTV(TVs, &IpcSrf); 
 
        /* Write maximum error and index. */  
        IpcCompress(f, &(IpcSrf.PointErrs.MaxIndex), sizeof(int)); 
 
        IpcCompress(f, &(TVs -> ULength), sizeof(int));         
        IpcCompressAsByte(f, &(TVs -> UOrder), 1); 
 
        IpcCompress(f, &(TVs -> VLength), sizeof(int));         
        IpcCompressAsByte(f, &(TVs -> VOrder), 1); 
 
        IpcCompress(f, &(TVs -> WLength), sizeof(int));         
        IpcCompressAsByte(f, &(TVs -> WOrder), 1);
 
        /* Knot vector , Periodic and order exist only in BSplines. */ 
        if (TRIV_IS_BSPLINE_TV(TVs)) { 
 
            IpcCompress(f, &(TVs -> UPeriodic), sizeof(CagdBType)); 
            IpcCompress(f, &(TVs -> VPeriodic), sizeof(CagdBType)); 
            IpcCompress(f, &(TVs -> WPeriodic), sizeof(CagdBType)); 
 
            /* Write and compress U knot vector. */ 
            IpcCompressKV(f, &(IpcSrf.UKV), IpcSrf.Args -> QntError); 
 
            /* Write and compress V knot vector. */ 
            IpcCompressKV(f, &(IpcSrf.VKV), IpcSrf.Args -> QntError); 
 
            /* Write and compress W knot vector. */ 
            IpcCompressKV(f, &(IpcSrf.WKV), IpcSrf.Args -> QntError); 
        } 
 
        /* Write and compress control points and weights. */ 
        IpcCompressPoints(f, &IpcSrf.PointErrs, Args -> QntError); 
         
        IpcCompressAttributes(f, TVs -> Attr, Args);             
    } 
 
    /* Indicates that stream of tri-variate is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress BSpline surfaces to file.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Srfs:       BSpline surfaces to compress.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressSrf(IpcFile *f, CagdSrfStruct *Srfs, IpcArgs *Args) 
{ 
    /* Create new encoding structure. */ 
    IpcSrfStruct IpcSrf;  
 
    /* Encode */ 
    for (; Srfs != NULL; Srfs = Srfs -> Pnext) { 
        /* Indicates that stream of surfaces is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Clear encoding structure. */ 
        memset(&IpcSrf, 0, sizeof(IpcSrfStruct)); 
 
        /* Write GType. */ 
        IpcCompress(f, &(Srfs -> GType), sizeof(CagdGeomType)); 
 
        /* Write PType. */ 
        IpcCompress(f, &(Srfs -> PType), sizeof(CagdPointType)); 
 
        /* Encode surface. */ 
        IpcSrf.Args = Args; 
 
        IpcEncodeSrf(Srfs, &IpcSrf);

        /* Write maximum error and index. */  
        IpcCompress(f, &(IpcSrf.PointErrs.MaxIndex), sizeof(int)); 
 
        IpcCompress(f, &(Srfs -> ULength), sizeof(int));         
        IpcCompressAsByte(f, &(Srfs -> UOrder), 1); 
 
        IpcCompress(f, &(Srfs -> VLength), sizeof(int));         
        IpcCompressAsByte(f, &(Srfs -> VOrder), 1); 
 
        /* Knot vector , Periodic and order exist only in BSplines. */ 
        if (CAGD_IS_BSPLINE_SRF(Srfs)) { 
 
            IpcCompress(f, &(Srfs -> UPeriodic), sizeof(CagdBType)); 
            IpcCompress(f, &(Srfs -> VPeriodic), sizeof(CagdBType)); 
 
            /* Write and compress U knot vector. */ 
            IpcCompressKV(f, &(IpcSrf.UKV), IpcSrf.Args -> QntError); 
 
            /* Write and compress V knot vector. */ 
            IpcCompressKV(f, &(IpcSrf.VKV), IpcSrf.Args -> QntError); 
        } 
        
        /* Write and compress control points and weights. */
        if (Args -> SrfPredictor == IPC_SRF_PREDICTOR_ANGLES)
            IpcCompressPointsAngles(f, &IpcSrf.PointErrs, Args -> QntError);
        else
            IpcCompressPoints(f, &IpcSrf.PointErrs, Args -> QntError);
 
        IpcCompressAttributes(f, Srfs -> Attr, Args);             
    } 
 
    /* Indicates that stream of surfaces is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress Trimmed surfaces to file.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   TrimSrfs:   BSpline trimmed surfaces to compress.                         *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTrimSrf(IpcFile *f, 
                               TrimSrfStruct *TrimSrfs, 
                               IpcArgs *Args) 
{ 
    for ( ; TrimSrfs != NULL; TrimSrfs = TrimSrfs -> Pnext) { 
        /* Indicates that stream of trimmed surfaces  is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Compress Tags. */ 
        IpcCompress(f, &(TrimSrfs -> Tags), sizeof(int)); 
 
        /* Compress trimmed surface. */ 
        IpcCompressSrf(f, TrimSrfs -> Srf, Args);             
         
        /* Compress attributes. */ 
        IpcCompressAttributes(f, TrimSrfs -> Attr, Args); 
 
        /* Compress  list of trimming curves. */ 
        IpcCompressTrimCrvList(f, TrimSrfs -> TrimCrvList, Args);  
    } 
 
    /* Indicates that stream of trimmed surfaces is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress Models to file.                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Mdls:       Models to compress.                                           *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressModels(IpcFile *f, MdlModelStruct *Mdls, IpcArgs *Args) 
{ 
    for ( ; Mdls != NULL; Mdls = Mdls -> Pnext) { 
        /* Indicates that stream of models  is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, Mdls -> Attr, Args); 
 
        IpcCompressTrimSrfList(f, Mdls -> TrimSrfList, Args); 
        IpcCompressTrimSegList(f, Mdls -> TrimSegList, Args); 
    } 
     
    /* Indicates that stream of models is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress trimmed surfaces list to file.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   TrimSrfList:  Trimmed surfaces to compress.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
*                                                                             *
******************************************************************************/
static void IpcCompressTrimSrfList(IpcFile *f, 
                                   MdlTrimSrfStruct *TrimSrfList, 
                                   IpcArgs *Args) 
{ 
    for ( ; TrimSrfList != NULL; TrimSrfList = TrimSrfList -> Pnext) { 
        /* Indicates that stream of trimmed surfaces list is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, TrimSrfList -> Attr, Args); 
 
        IpcCompressSrf(f, TrimSrfList -> Srf, Args); 
        IpcCompressLoopList(f, TrimSrfList -> LoopList, Args); 
    } 
     
    /* Indicates that stream of trimmed surfaces list is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress trimmed surfaces loop list to file.                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   LoopList:     Trimmed surfaces loop list to compress.                     *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressLoopList(IpcFile *f, 
                                MdlLoopStruct *LoopList, 
                                IpcArgs *Args) 
{ 
    MdlTrimSegRefStruct *SegRefList; 
     
    for ( ; LoopList != NULL; LoopList = LoopList -> Pnext) { 
        /* Indicates that stream of trimmed surfaces list is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, LoopList -> Attr, Args); 
         
        for (SegRefList=LoopList -> SegRefList; SegRefList!=NULL; 
                                        SegRefList=SegRefList -> Pnext) { 
            IpcCompressContinue(f, TRUE); 
            /* Compress attributes. */ 
            IpcCompressAttributes(f, SegRefList -> Attr, Args); 
            IpcCompressTrimSegList(f, SegRefList ->TrimSeg, Args); 
 
            IpcCompress(f, &(SegRefList -> Reversed), sizeof(int)); 
        } 
        IpcCompressContinue(f, FALSE); 
    } 
     
    /* Indicates that stream of trimmed surfaces list is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress trimmed segments to file.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   TrimSegs:     Trimmed segments to compress.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTrimSegList(IpcFile *f, 
                                   MdlTrimSegStruct *TrimSegs, 
                                   IpcArgs *Args) 
{ 
    for ( ; TrimSegs != NULL; TrimSegs = TrimSegs -> Pnext) { 
        /* Indicates that stream of trimmed segments is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, TrimSegs -> Attr, Args); 
         
        IpcCompressTrimSrfList(f, TrimSegs -> SrfFirst, Args); 
        IpcCompressTrimSrfList(f, TrimSegs -> SrfSecond, Args); 
 
        IpcCompressCrvs(f, TrimSegs -> UVCrvFirst, Args); 
        IpcCompressCrvs(f, TrimSegs -> UVCrvSecond, Args); 
        IpcCompressCrvs(f, TrimSegs -> EucCrv, Args); 
    } 
     
    /* Indicates that stream of trimmed segments is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress list of trimming curve to file.                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Crvs:       Trimmed curves to compress.                                   *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTrimCrvList(IpcFile *f,                                 
                                   TrimCrvStruct *Crvs,                        
                                   IpcArgs *Args)                              
{                                                                              
    for ( ; Crvs != NULL; Crvs = Crvs -> Pnext) {                              
        /* Indicates that stream of curves is not ended. */                    
        IpcCompressContinue(f, TRUE);                                          
                                                                               
        /* Compress attributes. */                                             
        IpcCompressAttributes(f, Crvs -> Attr, Args);                          
                                                                               
        /* Compress list of trimming curve segments. */                        
        IpcCompressTrimCrvSegList(f, Crvs -> TrimCrvSegList, Args); 
    } 
 
    /* Indicates that stream of curves is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress list of trimming curve segments to file.                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   TrimCrvSegs:  Trimmed curves segments to compress.                        *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTrimCrvSegList(IpcFile *f, 
                                      TrimCrvSegStruct *TrimCrvSegs, 
                                      IpcArgs *Args) 
{ 
    for ( ; TrimCrvSegs != NULL; TrimCrvSegs = TrimCrvSegs -> Pnext) {     
        /* Indicates that stream of curves is not ended. */ 
        IpcCompressContinue(f, TRUE);         
         
        /* Compress attributes. */ 
        IpcCompressAttributes(f, TrimCrvSegs -> Attr, Args); 
 
        /* Compress curves. */ 
        IpcCompressCrvs(f, TrimCrvSegs -> EucCrv, Args); 
        IpcCompressCrvs(f, TrimCrvSegs -> UVCrv , Args); 
    } 
 
    /* Indicates that stream of curves is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress list of curves to file.                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   Crvs:   List of curves to compress.                                       *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressCrvs(IpcFile *f, CagdCrvStruct *Crvs, IpcArgs *Args) 
{ 
    IpcCrvStruct IpcCrv; 
     
    for ( ; Crvs != NULL; Crvs = Crvs -> Pnext) { 
        /* Indicates that stream of curves is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Clear encoding structure. */ 
        memset(&IpcCrv, 0, sizeof(IpcCrvStruct)); 
 
        /* Encode curve. */ 
        IpcCrv.Args  = Args; 
        IpcCrv.PointErrs.PType = Crvs -> PType; 
        IpcCrv.PointErrs.NumPoints = Crvs -> Length; 
 
        IpcEncodeCrv(Crvs, &IpcCrv); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, Crvs -> Attr, Args);             
    
        /* Compress PType. */ 
        IpcCompress(f, &(Crvs -> PType), sizeof(CagdPointType)); 
 
        /* Compress GType. */ 
        IpcCompress(f, &(Crvs -> GType), sizeof(CagdGeomType)); 
 
        IpcCompress(f, &(Crvs -> Length), sizeof(int));         
        IpcCompressAsByte(f, &(Crvs -> Order), 1); 
 
        if (CAGD_IS_BSPLINE_CRV(Crvs)) { 
         
            /* Compress Periodic. */ 
            IpcCompress(f, &(Crvs -> Periodic), sizeof(CagdBType)); 
 
            /* Write and compress knot vector. */ 
            IpcCompressKV(f, &IpcCrv.KV, Args -> QntError); 
        } 
 
        if (Args -> CrvPredictor == IPC_CRV_PREDICTOR_2D) { 
            if (!IRIT_PT_EQ_ZERO(IpcCrv.PointErrs.Normal)) { 
                IpcCompressContinue(f, TRUE); 
                IpcCompress(f, IpcCrv.PointErrs.Normal, sizeof(IrtVecType)); 
            } 
            else 
                IpcCompressContinue(f, FALSE); 
        }
	else 
            /* Between n points there are n-1 differences. */ 
	    IpcCrv.PointErrs.NumPoints = Crvs -> Length - 1;  
 
        /* Write and compress control points and weights. */ 
        IpcCompressPoints(f, &IpcCrv.PointErrs, Args -> QntError); 
    } 
 
    /* Indicates that stream of curves is ended. */ 
    IpcCompressContinue(f, FALSE);   
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress Triangular surfaces to file.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:        A handler to compressed file.                                   *
*   TriSrfs:  Triangular surfaces to compress.                                *
*   Args:     User defined arguments.                                         *
*             Specifies different parameters of compression.                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressTriSrfs(IpcFile *f, 
                               TrngTriangSrfStruct *TriSrfs, 
                               IpcArgs *Args) 
{ 
    IpcTriSrfStruct IpcTriSrf; 
     
    for ( ; TriSrfs != NULL; TriSrfs = TriSrfs -> Pnext) { 
        /* Indicates that stream of Triangulars is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Clear encoding structure. */ 
        memset(&IpcTriSrf, 0, sizeof(IpcTriSrfStruct)); 
 
        /* Encode curve. */ 
        IpcTriSrf.Args  = Args; 
        IpcTriSrf.PointErrs.PType     = TriSrfs -> PType; 
        IpcTriSrf.PointErrs.NumPoints = TRNG_TRISRF_MESH_SIZE(TriSrfs); 
 
        IpcEncodeTriSrf(TriSrfs, &IpcTriSrf); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, TriSrfs -> Attr, Args);             
    
        /* Compress Gtype and PType. */ 
        IpcCompress(f, &(TriSrfs -> GType), sizeof(TrngGeomType)); 
        IpcCompress(f, &(TriSrfs -> PType), sizeof(CagdPointType)); 
 
        IpcCompress(f, &(TriSrfs -> Length), sizeof(int));         
        IpcCompressAsByte(f, &(TriSrfs -> Order), 1); 
 
        if (TRNG_IS_BSPLINE_TRISRF(TriSrfs)) { 
            /* Write and compress knot vector. */ 
            IpcCompressKV(f, &IpcTriSrf.KV, Args -> QntError); 
        } 
         
        /* Write and compress control points and weights. */ 
        IpcCompressPoints(f, &IpcTriSrf.PointErrs, Args -> QntError); 
    } 
 
    /* Indicates that stream of Triangulars is ended. */ 
    IpcCompressContinue(f, FALSE);   
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress Multi-variate to file.                                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   MVs:    Multi-variate to compress.                                        *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressMultiVar(IpcFile *f, MvarMVStruct *MVs, IpcArgs *Args) 
{ 
    IpcSrfStruct IpcSrf; 
    int i; 
     
    for ( ; MVs != NULL; MVs = MVs -> Pnext) { 
        /* Indicates that stream of Multi-variates is not ended. */ 
        IpcCompressContinue(f, TRUE); 
 
        /* Clear encoding structure. */ 
        memset(&IpcSrf, 0, sizeof(IpcSrfStruct)); 
 
        /* Encode curve. */ 
        IpcSrf.Args  = Args; 
 
        IpcEncodeMultiVar(MVs, &IpcSrf); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, MVs -> Attr, Args);             
    
        /* Compress GType and PType and Dim. */ 
        IpcCompress(f, &(MVs -> GType), sizeof(MvarGeomType)); 
        IpcCompress(f, &(MVs -> PType), sizeof(CagdPointType)); 
        IpcCompress(f, &(MVs -> Dim)  , sizeof(int));     
 
        /* Compress Lengths. */ 
        IpcCompress(f, MVs -> Lengths, MVs -> Dim * sizeof(int));         
        IpcCompressAsByte(f, MVs -> Orders , MVs -> Dim); 
 
        if (MVs -> GType == MVAR_BSPLINE_TYPE) { 
            /* Compress Periodic. */ 
            IpcCompressAsByte(f, MVs -> Periodic, MVs -> Dim); 
             
            /* Write and compress knot vectors. */ 
            for (i = 0; i < MVs -> Dim; i++) { 
                IpcCompressKV(f, &(IpcSrf.KVs[i]), Args -> QntError); 
            } 
        } 
         
        /* Write and compress control points and weights. */ 
        IpcCompressPoints(f, &IpcSrf.PointErrs, Args -> QntError); 
    } 
 
    /* Indicates that stream of Multi-variates is ended. */ 
    IpcCompressContinue(f, FALSE);   
} 
  
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress control point to file.                                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   CtlPt:  Control points to compress.                                       *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressCtlPt(IpcFile *f, CagdCtlPtStruct *CtlPt, IpcArgs *Args)
{ 
    int i,
        MaxCoord =  CAGD_NUM_OF_PT_COORD(CtlPt -> PtType); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(CtlPt -> PtType); 
     
    /* Compress attributes. */ 
    IpcCompressAttributes(f, CtlPt -> Attr, Args);     
    /* Compress type of a point. */ 
    IpcCompress(f, &(CtlPt -> PtType), sizeof(CagdPointType)); 
 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        IpcCompress(f, &(CtlPt -> Coords[i]), sizeof(CagdRType));        
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress Instance to file.                                                *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Instances:  Instance to compress.                                         *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressInstances(IpcFile *f, 
                                 IPInstanceStruct *Instances, 
                                 IpcArgs *Args) 
{ 
    IpcCompressString(f, Instances -> Name); 
    IpcCompress(f, Instances -> Mat, sizeof(IrtHmgnMatType));         
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode list of curves to IPC curve's structure.                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   IpcCrv: IPC curve structure includes entire information about curve.      *
*   Crv:    Curve to encode.                                                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeCrv(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv) 
{ 
    if (CAGD_IS_BSPLINE_CRV(Crv)) { 
        /* Encode knot vector. */ 
        if (Crv -> KnotVector) { 
            IpcEncodeKnotVector(Crv -> KnotVector, 
                                Crv -> Order, 
                                Crv -> Length, 
                                Crv -> Periodic, 
                                &(IpcCrv -> KV), 
                                IpcCrv -> Args -> QntError); 
 
            IpcCrv -> Length = Crv -> Length; 
            IpcCrv -> Order  = Crv -> Order; 
        } 
    } 
     
    /* Encode control points. */ 
    IpcEncodeCrvPoints(Crv, IpcCrv); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode Triangular surface to IPC Triangular surface's structure.          *
*                                                                             *
* PARAMETERS:                                                                 *
*   TriSrf:    Triangular surface to compress.                                *
*   IpcTriSrf: IPC structure includes entire information about Triangular.    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeTriSrf(TrngTriangSrfStruct *TriSrf, 
                            IpcTriSrfStruct *IpcTriSrf) 
{
    if (TRNG_IS_BSPLINE_TRISRF(TriSrf)) { 
        /* Encode knot vector. */ 
        if (TriSrf -> KnotVector) { 
            IpcEncodeKnotVector(TriSrf -> KnotVector, 
                                TriSrf -> Order, 
                                TriSrf -> Length, 
                                FALSE, 
                                &(IpcTriSrf -> KV), 
                                IpcTriSrf -> Args -> QntError); 
 
            IpcTriSrf -> Length = TriSrf -> Length; 
            IpcTriSrf -> Order  = TriSrf -> Order; 
        } 
    } 
     
    /* Encode control points. */ 
    IpcEncodePointsTriSrf(TriSrf, IpcTriSrf); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode Multi-variates to IPC structure.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   MV:       Multi-variate to encode.                                        *
*   IpcSrf:   IPC structure includes entire information about multi-variate.  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf) 
{ 
    int i; 
 
    if(MV -> GType == MVAR_BSPLINE_TYPE) { 
        /* Encode knot vector. */ 
        for (i = 0; i < MV -> Dim; i++) { 
           if (MV -> KnotVectors[i])  { 
               IpcEncodeKnotVector(MV -> KnotVectors[i],  
                                   MV -> Orders[i], 
                                   MV -> Lengths[i], 
                                   MV -> Periodic[i], 
                                   &(IpcSrf -> KVs[i]), 
                                   IpcSrf -> Args -> QntError); 
           } 
             
           IpcSrf -> Lengths[i] = MV -> Lengths[i]; 
           IpcSrf -> Orders[i]  = MV -> Orders[i]; 
        } 
    } 
      
    IpcSrf -> PointErrs.PType = MV -> PType; 
    IpcSrf -> PointErrs.NumPoints = MVAR_CTL_MESH_LENGTH(MV); 
     
    /* Encode control points. */ 
    IpcEncodePointsMultiVar(MV, &(IpcSrf -> PointErrs), IpcSrf -> Args); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode Multi-variate control points to IPC s structure.                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   MV:       Multi-variate to encode.                                        *
*   PtDiffs:  IPC points structure includes information about encoded points. *
*   Args:     User defined arguments.                                         *
*             Specifies different parameters of compression.                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodePointsMultiVar(MvarMVStruct *MV, 
                                    IpcCtlPtStruct *PtDiffs, 
                                    IpcArgs *Args) 
{ 
    int i, v,
        MaxCoord =  CAGD_NUM_OF_PT_COORD(MV -> PType), 
        Len = MVAR_CTL_MESH_LENGTH(MV); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(MV -> PType); 
 
    /* Init values. */ 
    PtDiffs -> MaxIndex = 0; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Allocate memory for non-quantized differences. */     
        PtDiffs -> PErrors[i] = (CagdRType*)IritMalloc(sizeof(CagdRType)*Len); 
 
        /* Allocate memory for quantized differences. */ 
        PtDiffs -> ErrIndexes[i] = (int*)IritMalloc(sizeof(int)*Len); 
    } 
 
    /* Set initial values. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        PtDiffs -> InitialValue[i] = MV -> Points[i][0]; 
 
    /* Find maximum point value. */ 
    PtDiffs -> DRange = 0; 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        for (v = 0; v < Len; v++)  
	    PtDiffs -> DRange = IRIT_MAX(PtDiffs -> DRange,
				    IRIT_FABS(MV -> Points[i][v]));
 
    /* Find control points and weights differences. */ 
    switch (Args -> MVPredictor) { 
        case IPC_MV_PREDICTOR_UNIFORM: 
            IpcPredictPointsMultiVar(MV, PtDiffs, Args -> QntError); 
            break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Find points structure parameters. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) 
        for (v = 0; v < Len; v++) { 
	    if (IPC_QUANTIZATION_USED(Args -> QntError)) 
	        PtDiffs -> MaxIndex = 
			    IRIT_MAX(PtDiffs -> MaxIndex,
				     IRIT_ABS(PtDiffs -> ErrIndexes[i][v]));
	    else 
	        PtDiffs -> MaxIndex = 0; 
	}
 
    PtDiffs -> Delta = Args -> QntError; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode list of curves control points to IPC curve's structure.            *
*                                                                             *
* PARAMETERS:                                                                 *
*   Crv:      Curve to encode.                                                *
*   IpcCrv:   IPC curve structure includes entire information about curve.    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeCrvPoints(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv) 
{ 
    int i, v, 
        MaxCoord =  CAGD_NUM_OF_PT_COORD(Crv -> PType), 
        NumPoints = IpcCrv -> PointErrs.NumPoints; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(Crv -> PType); 
    IpcCtlPtStruct  
        *PtDiffs = &IpcCrv -> PointErrs; 
    CagdRType 
        **Points = Crv -> Points; 
    IpcArgs  
        *Args = IpcCrv -> Args; 
 
    /* Init values. */ 
    PtDiffs -> MaxIndex = 0; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Allocate memory for non-quantized differences. */     
        PtDiffs -> PErrors[i] = 
                        (CagdRType*)IritMalloc(sizeof(CagdRType)*NumPoints); 
 
        /* Allocate memory for quantized differences. */ 
        PtDiffs -> ErrIndexes[i] = (int*)IritMalloc(sizeof(int)*NumPoints); 
    } 
 
    /* Set initial values. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        PtDiffs -> InitialValue[i] = Points[i][0]; 
 
    /* Find maximum point value. */ 
    PtDiffs -> DRange = 0; 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        for (v = 0; v < NumPoints; v++)  
            PtDiffs -> DRange = IRIT_MAX(PtDiffs -> DRange,
				    IRIT_FABS(Points[i][v])); 
      
    /* Find control points and weights differences. */ 
    switch (Args -> CrvPredictor) { 
        case IPC_CRV_PREDICTOR_2D: 
	    IpcPredictPoints2D(PtDiffs, Crv -> Points, NumPoints, Args); 
	    break;
 
        case IPC_CRV_PREDICTOR_ARC: 
            IpcPredictPointsArc(PtDiffs, Crv -> Points, NumPoints, Args); 
	    break; 
 
        case IPC_CRV_PREDICTOR_UNIFORM: 
            IpcPredictPointsUniform(PtDiffs, Crv -> Points, NumPoints, Args);
	    break; 
 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Find points structure parameters. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) 
        for (v = 0; v < NumPoints; v++) { 
            if (IPC_QUANTIZATION_USED(Args -> QntError)) 
                PtDiffs -> MaxIndex = 
                    IRIT_MAX(PtDiffs -> MaxIndex,
			     IRIT_ABS(PtDiffs -> ErrIndexes[i][v]));
            else 
                PtDiffs -> MaxIndex = 0; 
        } 
 
        PtDiffs -> Delta = Args -> QntError; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode Triangular surface control points to IPC Triangular's structure.   *
*                                                                             *
* PARAMETERS:                                                                 *
*   TriSrf:     Triangular surface to encode from.                            *
*   IpcTriSrf:  IPC structure includes entire information about Triangular.   *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodePointsTriSrf(TrngTriangSrfStruct *TriSrf, 
                                  IpcTriSrfStruct *IpcTriSrf) 
{ 
    int i, v, 
        MaxCoord =  CAGD_NUM_OF_PT_COORD(TriSrf -> PType), 
        NumPoints = IpcTriSrf -> PointErrs.NumPoints; 
    CagdBType  
        IsNotRational = !TRNG_IS_RATIONAL_TRISRF(TriSrf); 
    IpcCtlPtStruct  
        *PtDiffs = &IpcTriSrf -> PointErrs; 
    CagdRType 
        **Points = TriSrf -> Points; 
    IpcArgs  
        *Args = IpcTriSrf -> Args; 
  
    /* Init values. */ 
    PtDiffs -> MaxIndex = 0; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Allocate memory for non-quantized differences. */     
        PtDiffs -> PErrors[i] = 
                (CagdRType*)IritMalloc(sizeof(CagdRType)*NumPoints); 
 
        /* Allocate memory for quantized differences. */ 
        PtDiffs -> ErrIndexes[i] = (int*)IritMalloc(sizeof(int)*NumPoints); 
    } 
 
    /* Set initial values. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        PtDiffs -> InitialValue[i] = Points[i][0]; 
 
    /* Find maximum point value. */ 
    PtDiffs -> DRange = 0; 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        for (v = 0; v < NumPoints; v++)  
            PtDiffs -> DRange = IRIT_MAX(PtDiffs -> DRange,
				    IRIT_FABS(Points[i][v])); 
 
    /* Find control points and weights differences. */ 
    switch (Args -> TriSrfPredictor) { 
        case IPC_TRISRF_PREDICTOR_UNIFORM: 
            IpcPredictPointsUniform(PtDiffs, TriSrf -> Points,
				    NumPoints, Args); 
            break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Find points structure parameters. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) 
        for (v = 0; v < NumPoints; v++) { 
            if (IPC_QUANTIZATION_USED(Args -> QntError)) 
                PtDiffs -> MaxIndex = 
                    
		IRIT_MAX(PtDiffs -> MaxIndex,
			 IRIT_ABS(PtDiffs -> ErrIndexes[i][v]));
            else 
                PtDiffs -> MaxIndex = 0; 
        } 
 
    PtDiffs -> Delta = Args -> QntError; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress list of polygons to file.                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Pl:         Polygons to compress.                                         *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressPolygons(IpcFile *f, IPPolygonStruct *Pl, IpcArgs *Args)
{ 
    int IsSame; 
 
    for ( ; Pl != NULL; Pl = Pl -> Pnext) { 
        /* Indicates that stream of polygons is not ended. */ 
        IpcCompressContinue(f, TRUE); 
         
        /* Compress Tags. */ 
        IpcCompress(f, &(Pl -> Tags), sizeof(IrtBType)); 
 
        /* Compress attributes. */ 
        IpcCompressAttributes(f, Pl -> Attr, Args); 
 
        /* Compress varaibles. */ 
        if (Pl -> PAux || Pl -> IAux || Pl -> IAux2) { 
	    /* Set indicator that next fields to be compressed. */ 
	    IpcCompressContinue(f, TRUE);          
	    IpcCompress(f, &(Pl -> PAux), sizeof(VoidPtr)); 
	    IpcCompress(f, &(Pl -> IAux), sizeof(int)); 
	    IpcCompress(f, &(Pl -> IAux2), sizeof(int)); 
        } 
        else  
	    IpcCompressContinue(f, FALSE); 
 
        /* Compress  BBox of polygons. */ 
        if (IP_HAS_BBOX_POLY(Pl)) 
	    IpcCompress(f, &(Pl -> BBox), sizeof(IrtBboxType)); 
 
        /* Compare polygon normal to vertex normal. */ 
        IsSame = IRIT_PT_APX_EQ(Pl -> PVertex -> Normal, Pl -> Plane); 
	IpcCompressVertex(f, Pl -> PVertex, IsSame, Args); 
    } 
 
    /* Indicates that stream of polygons is ended. */ 
    IpcCompressContinue(f, FALSE); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress points to file if defined as float.                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Pt:         Points to compress.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressPointFloat(IpcFile *f, IrtPtType *Pt, IpcArgs *Args) 
{ 
    int i, QntValueIndx; 
    float TmpValue;
 
#ifdef IPC_VERTEX_FLOAT 
    for (i = 0; i < 3; i++) { 
        TmpValue = (float)((*Pt)[i]); 
        IpcCompress(f, &TmpValue, sizeof(float)); 
    } 
#else 
    if (IPC_QUANTIZATION_USED(Args -> QntError)) { 
	for (i = 0; i < 3; i++) { 
	    TmpValue = (float)((*Pt)[i]); 
            QntValueIndx = IRIT_REAL_TO_INT(TmpValue / Args -> QntError); 
            IpcCompress(f, &QntValueIndx, sizeof(int)); 
        }                 
    } 
    else       
        IpcCompress(f, Pt , sizeof(IrtPtType)); 
#endif 
} 
  
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress list of vertecies to file.                                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PVertex:    Vertices to compress.                                         *
*   IsSame:     TRUE = if adj polygon normal is same as vertex normal.        *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressVertex(IpcFile *f, 
                              IPVertexStruct *PVertex, 
                              int IsSame, 
                              IpcArgs *Args) 
{ 
    IPVertexStruct 
        *PHeadVertex = PVertex;

    if (PVertex) {
        do {
            /* Indicates that stream of vertexes is not ended. */ 
            IpcCompressContinue(f, TRUE); 
     
            /* Compress attributes. */ 
            IpcCompressAttributes(f, PVertex -> Attr, Args); 
     
            /* Compress varaibles. */ 
            IpcCompress(f, &(PVertex -> Tags)  , sizeof(IrtBType)); 
             
            IpcCompressPointFloat(f, &(PVertex -> Coord), Args); 
     
            /* Compress vertex normal. */  
            if (IP_HAS_NORMAL_VRTX(PVertex) && !IsSame) { 
                IpcCompressContinue(f, TRUE); 
                IpcCompressPointFloat(f, &(PVertex -> Normal), Args);
            }
	    else 
                IpcCompressContinue(f, FALSE); 
            /* Iterate to next vertex. */
            PVertex = PVertex -> Pnext;
        }
	while (PVertex != PHeadVertex && PVertex != NULL);
    }
 
    /* Indicates that stream of vertexes is ended. */
    IpcCompressContinue(f, FALSE);
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress knot vector.                                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   KV:         IPC knot vector structure which includes entire information   *
*               about knot vector to compress.                                *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressKV(IpcFile *f, IpcKnotVectorStruct *KV, float QntError) 
{ 
    int Size;

    IpcCompressAsByte(f, &(KV -> KnotVectorMap), 1);
    IpcCompress(f, &(KV -> Scale), sizeof(CagdRType)); 
    IpcCompress(f, &(KV -> InitBreakValue), sizeof(CagdRType)); 
 
    if (KV -> KnotVectorMap != IPC_KNOT_VECTOR_GENERAL)  
        return;
 
    IpcCompress(f, &(KV -> NumBreakValues), sizeof(int)); 
     
    /* Write and compress Multiplicity Map. */ 
    IpcCompressAsByte(f, (KV -> MultiplicityMap), KV -> NumBreakValues);
 
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcCompress(f, &(KV -> DeltaK), sizeof(CagdRType)); 
        /* Write and compress Error Indexes. */ 
        Size = (KV -> NumBreakValues - 1) * sizeof(int); 
        IpcCompress(f, (KV -> ErrIndexes), Size); 
    }
    else { 
        Size = (KV -> NumBreakValues - 1) * sizeof(CagdRType); 
        IpcCompress(f, (KV -> KVErrors), Size); 
    } 
 
    /* Free allocted memory. */ 
    IritFree(KV -> ErrIndexes); 
    IritFree(KV -> KVErrors); 
    IritFree(KV -> MultiplicityMap); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress control points and weights.                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Points:     IPC structure includes all information about control points.  *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressPoints(IpcFile *f, 
                              IpcCtlPtStruct *Points, 
                              float QntError) 
{ 
    int Size, i, j,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Points -> PType), 
        MaxIndex    = Points -> MaxIndex; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(Points -> PType);
    signed char TmpIndex; 
    long 
        NumPoints = Points -> NumPoints; 
 
    IpcCompress(f, &(Points -> PType), sizeof(CagdPointType)); 
 
    /* Write and compress initial values of control points and weights. */ 
    Size = sizeof(CagdRType) * (MaxCoord + !IsNotRational); 
    IpcCompress(f, (Points -> InitialValue + IsNotRational), Size); 
         
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcCompress(f, &(Points -> Delta), sizeof(CagdRType)); 
        IpcCompress(f, &(Points -> DRange), sizeof(CagdRType)); 
        IpcCompress(f, &(Points -> MaxIndex), sizeof(int)); 
 
         /* Write and compress error indexes of control points and weights. */ 
	for (i = IsNotRational; i <= MaxCoord; i++) 
            if (MaxIndex != (signed char)MaxIndex) 
	        IpcCompress(f, (Points -> ErrIndexes[i]),
			    NumPoints * sizeof(int)); 
	    else  
	        for (j = 0; j < NumPoints; j++) { 
		    TmpIndex = Points -> ErrIndexes[i][j]; 
		    IpcCompress(f, &TmpIndex, sizeof(signed char)); 
		} 
    }
    else { 
         /* Write and compress errors of control points and weights. */ 
         for (i = IsNotRational; i <= MaxCoord; i++)  
             IpcCompress(f, (Points -> PErrors[i]),
			 NumPoints * sizeof(CagdRType));
    }

    /* Free allocated memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        IritFree(Points -> ErrIndexes[i]); 
        IritFree(Points -> PErrors[i]); 
    } 
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compress control points and weights using angles predictor.               *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Points:     IPC structure includes all information about control points.  *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcCompressPointsAngles(IpcFile *f, 
                                    IpcCtlPtStruct *Points, 
                                    float QntError)
{
    int Block, i, TypeSize,
        MaxCoord  = CAGD_NUM_OF_PT_COORD(Points -> PType), 
        MaxIndex  = Points -> MaxIndex,
        BlockLen  = Points -> BlockLen,
        NumBlocks = Points -> NumBlocks;
    IrtBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(Points -> PType);
    void *BlockPtr;

    IpcCompress(f, &(Points -> PType), sizeof(CagdPointType));
         
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcCompress(f, &(Points -> Delta), sizeof(CagdRType)); 
        IpcCompress(f, &(Points -> DRange), sizeof(CagdRType)); 
        IpcCompress(f, &(Points -> MaxIndex), sizeof(int));

        TypeSize = sizeof(int);
    }
    else {
        TypeSize = sizeof(IrtRType);
    }

    /* Number of points in block > 2, compress normal and predictors. */
    if (BlockLen > 2) {
        /* Compress x, y of normals (normalized). */
        for (Block = 0; Block < NumBlocks; Block++)
            IpcCompressDoubleAsInt(f, Points -> Nrmls[Block], 2);

        /* Compress predictors for each block. */
        IpcCompressAsByte(f, Points -> Predicts, NumBlocks);
    }

    /* Compress P0 and P1 in each block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        for (i = 1; i <= MaxCoord; i++) {
            if (IPC_QUANTIZATION_USED(QntError)) 
                BlockPtr = &(Points -> ErrIndexes[i][Block * BlockLen]);
            else
                BlockPtr = &(Points -> PErrors[i][Block * BlockLen]);
            
            if (IPC_QUANTIZATION_USED(QntError)) {
                if (MaxIndex == (signed char)MaxIndex)
                    IpcCompressAsByte(f, BlockPtr, 2);
            }

            IpcCompress(f, BlockPtr, TypeSize * 2);
        }
    }
    
    /* If predictor not match perfect, compress rest of the points in block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        if (!IPC_IS_PREDICTOR_PERFECT(Points -> Predicts[Block])) {
            for (i = 1; i <= MaxCoord; i++) {
                if (IPC_QUANTIZATION_USED(QntError)) 
		    BlockPtr = &(Points -> ErrIndexes[i][Block * BlockLen + 2]);
                else
                    BlockPtr = &(Points -> PErrors[i][Block * BlockLen + 2]);

                if (IPC_QUANTIZATION_USED(QntError) && 
                                         (MaxIndex == (signed char)MaxIndex)) {
		    IpcCompressAsByte(f, BlockPtr, BlockLen - 2);
                }

                IpcCompress(f, BlockPtr, (BlockLen - 2) * TypeSize);
            }
        }
    }

    /* Compress errors of control weights. */
    if(!IsNotRational) {
        if (IPC_QUANTIZATION_USED(QntError))
            BlockPtr = Points -> ErrIndexes[0];
        else
            BlockPtr = Points -> PErrors[0];

        IpcCompress(f, BlockPtr, Points -> NumPoints * TypeSize);
    }

    /* Free allocated memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) {
        IritFree(Points -> ErrIndexes[i]);
        IritFree(Points -> PErrors[i]);
    }

    IritFree(Points -> Nrmls);
    IritFree(Points -> Predicts);
    IritFree(Points -> NumAngles);
    IritFree(Points -> Angles);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode a given surface to IPC format.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   IpcSrf:   IPC surface structure includes entire information about surface.*
*             Used in order to decompress surface effectively.                *
*   Srfs:     BSpline surfaces to encode from.                                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeSrf(CagdSrfStruct *Srf, IpcSrfStruct *IpcSrf)  
{ 
    if (CAGD_IS_BSPLINE_SRF(Srf)) { 
        /* Encode U knot vector. */ 
        if (Srf -> UKnotVector) { 
            IpcEncodeKnotVector(Srf -> UKnotVector, 
                                Srf -> UOrder, 
                                Srf -> ULength, 
                                Srf -> UPeriodic, 
                                &(IpcSrf -> UKV),    
                                IpcSrf -> Args -> QntError); 
 
            IpcSrf -> ULength = Srf -> ULength; 
            IpcSrf -> UOrder  = Srf -> UOrder; 
        } 
 
        /* Encode V knot vector. */ 
        if (Srf -> VKnotVector) { 
            IpcEncodeKnotVector(Srf -> VKnotVector, 
                                Srf -> VOrder, 
                                Srf -> VLength, 
                                Srf -> VPeriodic, 
                                &(IpcSrf -> VKV), 
                                IpcSrf -> Args -> QntError); 
 
            IpcSrf -> VLength = Srf -> VLength; 
            IpcSrf -> VOrder  = Srf -> VOrder; 
        } 
    } 
 
    IpcSrf -> PointErrs.NumPoints = Srf -> ULength * Srf -> VLength; 
    IpcSrf -> PointErrs.ExtNumPoints = (Srf -> ULength+1)*(Srf -> VLength+1); 
    IpcSrf -> PointErrs.PType = Srf -> PType; 
    IpcSrf -> PType = Srf -> PType; 
    IpcSrf -> GType = Srf -> GType; 
 
    /* Encode control points. */ 
    IpcEncodePointsSrf(Srf, IpcSrf); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode a given tri_variate to IPC format.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   IpcSrf:   IPC surface structure includes entire information about surface.*
*             Used in order to decompress surface effectively.                *
*   TV:       Tri_variate to encode from.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeTrivTV(TrivTVStruct *TV, IpcSrfStruct *IpcSrf)  
{ 
    if (TRIV_IS_BSPLINE_TV(TV)) { 
        /* Encode U knot vector. */ 
        if (TV -> UKnotVector) { 
            IpcEncodeKnotVector(TV -> UKnotVector, 
                                TV -> UOrder, 
                                TV -> ULength, 
                                TV -> UPeriodic, 
                                &(IpcSrf -> UKV),
                                IpcSrf -> Args -> QntError); 
 
            IpcSrf -> ULength = TV -> ULength; 
            IpcSrf -> UOrder  = TV -> UOrder; 
        } 
 
        /* Encode V knot vector. */ 
        if (TV -> VKnotVector) { 
            IpcEncodeKnotVector(TV -> VKnotVector, 
                                TV -> VOrder, 
                                TV -> VLength, 
                                TV -> VPeriodic, 
                                &(IpcSrf -> VKV), 
                                IpcSrf -> Args -> QntError); 
 
            IpcSrf -> VLength = TV -> VLength; 
            IpcSrf -> VOrder  = TV -> VOrder; 
        } 
 
        /* Encode W knot vector. */ 
        if (TV -> WKnotVector) { 
            IpcEncodeKnotVector(TV -> WKnotVector, 
                                TV -> WOrder, 
                                TV -> WLength, 
                                TV -> WPeriodic, 
                                &(IpcSrf -> WKV), 
                                IpcSrf -> Args -> QntError); 
 
            IpcSrf -> WLength = TV -> WLength; 
            IpcSrf -> WOrder  = TV -> WOrder; 
        } 
    } 
 
    IpcSrf -> PointErrs.NumPoints = 
                    TV -> ULength * TV -> VLength * TV -> WLength; 
    IpcSrf -> PointErrs.ExtNumPoints = 
                    (TV -> ULength+1)*(TV -> VLength+1)*(TV -> WLength+1); 
    IpcSrf -> PointErrs.PType = TV -> PType; 
    IpcSrf -> PType = TV -> PType; 
    IpcSrf -> GType = TV -> GType; 
 
    /* Encode control points. */ 
    IpcEncodeTrivTVPoints(TV, IpcSrf); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode a given knot vector to IPC format.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   KnotVector:         Knot vector values.                                   *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*   Periodic:           Valid only for Bspline surfaces.                      *
*   KnotVectorStruct:   IPC knot vector structure which includes              *
*                       entire information about knot vector.                 *
*   QntError:           Quantization step between(0..1).                      *
*                       Specifies maximum error for values.                   *
*                       In addition can accept next values:                   *
*                       IPC_QUANTIZATION_NONE - no quanization is used.       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeKnotVector(CagdRType *KnotVector, 
                                int Order, 
                                int Length, 
                                CagdBType Periodic,  
                                IpcKnotVectorStruct *KnotVectorStruct, 
                                float QntError) 
{ 
    CagdRType *BreakValues; 
    int Len = Order + Length + (Periodic ? Order - 1 : 0); 
 
    /* Test and set knot vector for common wide used forms. */ 
    if (IpcUniqeEncodeKnotVector(KnotVector, Len, Length, Order,  
                                 KnotVectorStruct) != IPC_KNOT_VECTOR_GENERAL) 
        return; 
 
    /* The Map of knot vector is not uniqe. */  
    KnotVectorStruct -> KnotVectorMap = IPC_KNOT_VECTOR_GENERAL; 
 
    /* Find number of break values and their values. */ 
    IpcAnalizeBreakValues(&BreakValues, 
                          &(KnotVectorStruct -> NumBreakValues), 
                          KnotVector, Len); 
 
    KnotVectorStruct -> Scale = 
        IpcNormalizeBreakvalues(BreakValues,
				KnotVectorStruct -> NumBreakValues);
 
    KnotVectorStruct -> MultiplicityMap =  
        IpcFindMultiplicity(KnotVectorStruct -> NumBreakValues,
			    KnotVector, Len);
     
    /* Find minimum quantization step for errors in break values. */   
    KnotVectorStruct -> DeltaK =  
	IpcFindDeltaK(BreakValues, KnotVectorStruct -> NumBreakValues,
		      QntError);
 
    IpcKVFindErrorsIndexes(BreakValues, 
                           KnotVectorStruct, 
                           KnotVector, 
                           Order, 
                           Length, 
                           QntError);  
 
    IritFree(BreakValues); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Find number of break values and their values.                             *
*                                                                             *
* PARAMETERS:                                                                 *
*   BreakValues:        Knot vector's different values.                       *
*   NumBreakValues:     Number of different knot vector values.               *
*   KnotVector:         Knot vector values.                                   *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcAnalizeBreakValues(CagdRType **BreakValues, 
                                  int *NumBreakValues,
                                  CagdRType *KnotVector, 
                                  int Len)  
{ 
    int i, j; 
     
    *NumBreakValues = 1;

    /* Find number of break values. */ 
    for (i  = 0; i < Len-1; i++) { 
        if (!IRIT_APX_EQ(KnotVector[i+1], KnotVector[i])) 
            *NumBreakValues += 1; 
    } 
 
    *BreakValues = 
        (CagdRType *) IritMalloc(sizeof(CagdRType) * (*NumBreakValues + 2));
     
    /* Set first break value. */ 
    (*BreakValues)[0] = KnotVector[0]; 
 
    for (i = j = 0; i < Len; i++) { 
        if (!IRIT_APX_EQ(KnotVector[i], (*BreakValues)[j])) 
            (*BreakValues)[++j] = KnotVector[i]; 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Normalize break vector.                                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   BreakValues:        Knot vector's different values.                       *
*   NumBreakValues:     Number of different knot vector values.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdRType:      Scale factor for knot vector.                             *
*                                                                             *
******************************************************************************/
static CagdRType IpcNormalizeBreakvalues(CagdRType *BreakValues, 
                                         int NumBreakValue) 
{ 
    int i; 
    CagdRType LastElem, Scale, InitValue; 
     
    InitValue = BreakValues[0]; 
    LastElem = BreakValues[NumBreakValue - 1] - InitValue;  
    Scale = (LastElem) ? 1/LastElem : 1;  
 
    for (i = 0; i < NumBreakValue; i++) 
        BreakValues[i] = (BreakValues[i] - InitValue) * Scale; 
 
    return Scale; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Find multiplicity of break values in knot vector.                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   NumBreakValues:     Number of different knot vector values.               *
*   KnotVector:         Knot vector values.                                   *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static int* IpcFindMultiplicity(int NumBreakValues, 
                                CagdRType *KnotVector, 
                                int Len)  
{ 
    int i, j, *Multiplicity; 
 
    Multiplicity = (int*)IritMalloc(sizeof(int) * (NumBreakValues));     
    /* Zeros multiplicity vector. */ 
    IRIT_ZAP_MEM(Multiplicity, sizeof(int) * NumBreakValues); 
     
    /* Set multiplicity map. */ 
    for (i = j = 0; i < Len - 1; i++) { 
        if (IRIT_APX_EQ(KnotVector[i+1], KnotVector[i])) 
            Multiplicity[j] += 1; 
        else  
            j++; 
    } 
 
    return Multiplicity; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Find minimum quantization step for errors in break values.                *
*                                                                             *
* PARAMETERS:                                                                 *
*   BreakValues:    Knot vector's different values.                           *
*   Len:            Size of the BreakValues array.                            *
*   QntError:       Quantization step between(0..1).                          *
*                   Specifies maximum error for values.                       *
*                   In addition can accept next values:                       *
*                   IPC_QUANTIZATION_NONE - no quanization is used.           *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static CagdRType IpcFindDeltaK(CagdRType *BreakValues, int Len, float QntError)
{ 
    return QntError / 10; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Find quantized differences for  knot vector.                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   BreakValues:   Knot vector's different values.                            *
*   KVrStruct:     IPC knot vector structure which includes                   *
*                  entire information about knot vector.                      *
*   KnotVector:    Knot vector values.                                        *
*   Order:         Order in tensor product surface (Bspline only).            *
*   Length:        Mesh size in the tensor product surface.                   *
*   QntError:      Quantization step between(0..1).                           *
*                  Specifies maximum error for values.                        *
*                  In addition can accept next values:                        *
*                  IPC_QUANTIZATION_NONE - no quanization is used.            *
*                                                                             *
* RETURN VALUE:                                                               *
*   int*:       Array of quantized differences of knot vector.                *
******************************************************************************/
static int* IpcKVFindErrorsIndexes(CagdRType *BreakValues, 
                                   IpcKnotVectorStruct *KVStruct, 
                                   CagdRType *KnotVector, 
                                   int Order, 
                                   int Length,  
                                   float QntError)  
{ 
    CagdRType *Errors, *QntErrors, *qntBreakValues, BKInit,
        DeltaK = KVStruct -> DeltaK; 
    int i, *qntIndexes,
        NumBreakValues = KVStruct -> NumBreakValues; 
 
    if (NumBreakValues <= 0) 
        CagdFatalError(CAGD_ERR_ALLOC_ERR); 
 
    Errors =  
        (CagdRType *) IritMalloc(sizeof(CagdRType) * NumBreakValues); 
 
    QntErrors =  
        (CagdRType *) IritMalloc(sizeof(CagdRType) * NumBreakValues); 
 
    qntBreakValues =  
        (CagdRType *) IritMalloc(sizeof(CagdRType) * NumBreakValues); 
 
    qntIndexes =  
        (int *) IritMalloc(sizeof(int) * (NumBreakValues)); 
 
    /* Loop initialization. */
    qntBreakValues[0] = 0; 
     
    /* Used in order to minimize Error(0) in common case. */ 
    BKInit = -1.0 /(NumBreakValues - 1); 
 
    Errors[0] = BreakValues[1] - 2 * qntBreakValues[0] + BKInit; 
    qntIndexes[0] = IRIT_REAL_TO_INT(Errors[0] / DeltaK); 
 
    if (IPC_QUANTIZATION_USED(QntError)) 
        QntErrors[0] = qntIndexes[0] * DeltaK; 
    else 
        QntErrors[0] = Errors[0]; 
    
    qntBreakValues[1] = QntErrors[0] + 2 * qntBreakValues[0] - BKInit; 
 
    for (i = 1; i < NumBreakValues - 1; i++) { 
	Errors[i] = BreakValues[i+1] - 2 * qntBreakValues[i]
							+ qntBreakValues[i-1];
	qntIndexes[i] = IRIT_REAL_TO_INT(Errors[i] / DeltaK); 
 
	if (IPC_QUANTIZATION_USED(QntError)) 
            QntErrors[i] = qntIndexes[i] * DeltaK; 
	else  
            QntErrors[i] = Errors[i]; 
      
	qntBreakValues[i+1] = 
                QntErrors[i] + 2 * qntBreakValues[i] - qntBreakValues[i-1]; 
    } 
 
    KVStruct -> KVErrors = Errors; 
    KVStruct -> ErrIndexes = qntIndexes; 
 
    IritFree(QntErrors); 
    IritFree(qntBreakValues); 
 
    return qntIndexes; 
}     

/******************************************************************************
* DESCRIPTION:                                                                *
*  Test knot vector for common wide used forms.                               *
*  If knot vector of special form save it in IPC knot vector surface.         *
*                                                                             *
* PARAMETERS:                                                                 *
*   KnotVector:         Knot vector values.                                   *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*   Periodic:           Valid only for Bspline surfaces.                      *
*   KnotVectorStruct:   IPC knot vector structure includes                    *
*                       entire information about knot vector.                 *
*                                                                             *
* RETURN VALUE:                                                               *
*   IpcKnotVectorType:                Knot vector type.                       *
******************************************************************************/
static IpcKnotVectorType IpcUniqeEncodeKnotVector(CagdRType *KnotVector,
			                          int Len, 
			                          int Length,
			                          int Order, 
			        	          IpcKnotVectorStruct *KV)
{ 
    CagdEndConditionType  
        KnotVectorType = BspIsKnotUniform(Length, Order, KnotVector); 
     
    /* Set the scale factor to be maximum knot vector value. */     
    KV -> Scale = KnotVector[Len - 1]; 
    KV -> InitBreakValue = KnotVector[0];     
 
    switch (KnotVectorType) { 
        case CAGD_END_COND_FLOAT:  
            KV -> KnotVectorMap = IPC_KNOT_VECTOR_UNIFORM_FLOAT; 
            /* Periodic == TRUE no scale is needed. */ 
            if (Len != Length)  
                KV -> Scale = 1; 
            break; 
 
        case CAGD_END_COND_OPEN:  
            KV -> KnotVectorMap = IPC_KNOT_VECTOR_UNIFORM_OPEN; 
            KV -> Scale -= KnotVector[0]; 
            break; 
 
        default:  
            KV -> KnotVectorMap = IPC_KNOT_VECTOR_GENERAL; 
            KV -> Scale -= KnotVector[0]; 
    } 
 
    return KV -> KnotVectorMap; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode control points.                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:     BSpline surfaces to encode.                                     *
*   IpcSrf:   IPC surface structure includes entire information about surface.*
*             Used in order to decompress surface effectively.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodePointsSrf(CagdSrfStruct *Srf, IpcSrfStruct *IpcSrf) 
{ 
    CagdRType *QntPoints[CAGD_MAX_PT_SIZE]; 
    IpcCtlPtStruct  
        *Errs = &(IpcSrf -> PointErrs); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(IpcSrf); 
    int i,  
        MaxCoord = CAGD_NUM_OF_PT_COORD(IpcSrf -> PType); 
    IpcArgs  
        *Args = IpcSrf -> Args;
 
    /* Allocate memory for quantized control points. */ 
    IpcPointsAllocateMemory(QntPoints, IpcSrf, TRUE);   
 
    /* Allocate memory for not quantized errors of control points. */ 
    IpcPointsAllocateMemory(Errs -> PErrors, IpcSrf, FALSE); 
     
    /* Allocate memory for quantized indexes of control points. */ 
    IpcQntIndexesAllocateMemory(Errs -> ErrIndexes, IpcSrf); 
 
    /* Set initial values for control points and weights */ 
    IpcSaveInitialValues(Errs, Srf); 
 
    /* Set initial values for quantized control points. */  
    IpcInitPoints(QntPoints, Srf); 
 
    /* Set initial values for quantized control points weights. */ 
    IpcInitWeights(QntPoints, Srf); 
 
    /* Find uniform scalar quantizer */ 
    Errs -> Delta = IpcFindDelta(Srf, Errs, Args -> QntError); 
 
    /* Find error indexes */ 
    switch (Args -> SrfPredictor) { 
        case IPC_SRF_PREDICTOR_ANGLES: 
            IpcPredictPointsAnglesSrf(QntPoints, Errs, Srf, Args);
	    break;

        case IPC_SRF_PREDICTOR_PARALLELOGRAM: 
            IpcPredictPointsSrf(QntPoints, Errs, Srf, Args); 
	    break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Free memory for quantized coordinate. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        IritFree(QntPoints[i]); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Encode control points of Trivariate.                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:       Trivariate to encode.                                           *
*   IpcSrf:   IPC surface structure includes entire information about surface.*
*             Used in order to decompress surface effectively.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcEncodeTrivTVPoints(TrivTVStruct *TV, IpcSrfStruct *IpcSrf) 
{ 
    CagdRType *QntPoints[CAGD_MAX_PT_SIZE]; 
    IpcCtlPtStruct  
        *Errs = &(IpcSrf -> PointErrs); 
    IpcArgs  
        *Args = IpcSrf -> Args; 
 
    /* Allocate memory for quantized control points. */ 
    IpcPointsAllocateMemory(QntPoints, IpcSrf, TRUE);   
 
    /* Allocate memory for not quantized errors of control points. */ 
    IpcPointsAllocateMemory(Errs -> PErrors, IpcSrf, FALSE); 
     
    /* Allocate memory for quantized indexes of control points. */ 
    IpcQntIndexesAllocateMemory(Errs -> ErrIndexes, IpcSrf); 
 
    /* Set initial values for control points and weights */ 
    IpcSaveInitialValuesTrivTV(Errs, TV); 
 
    /* Set initial values for quantized control points. */  
    IpcInitPointsTrivTV(QntPoints, TV); 
 
    /* Set initial values for quantized control points weights. */ 
    IpcInitWeightsTrivTV(QntPoints, TV); 
 
    /* Find uniform scalar quantizer */ 
    Errs -> Delta = IpcFindDeltaTrivTV(TV, Errs, Args -> QntError); 
     
    /* Find error indexes */ 
    switch (Args -> TVPredictor) {
        case IPC_TV_PREDICTOR_UNIFORM:
            IpcPredictPointsTrivTV(QntPoints, Errs, TV, Args -> QntError); 
            break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for quantized control points.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:      Arrays of quantized control points and weights.           *
*   TV:             Trivariate to encode.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcInitPointsTrivTV(CagdRType *QntPoints[], TrivTVStruct *TV) 
{ 
    int u, v, w, i,
        MaxCoord =  TRIV_NUM_OF_PT_COORD(TV); 
    CagdBType  
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV); 
    CagdRType  
        **Points = TV -> Points; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* set qP(-1, -1, -1) = P(0, 0, 0) */ 
        QntPoints[i][IPC_QNT_TRIV_MESH_UVW(TV, -1, -1, -1)] = 
                                    Points[i][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
 
        /* Set the first top row of quantized control points. */ 
         
        /* qntP(u, -1, -1) = P(0,0,0) */ 
        for (u = 0; u < TV -> ULength; u++)  
            QntPoints[i][IPC_QNT_TRIV_MESH_UVW(TV, u, -1, -1)] = 
                                    Points[i][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
 
        /* Set the first left column of quantized control points. */ 
        /* qntP(-1, v, -1) = P(0,0,0) */ 
        for (v = 0; v < TV -> VLength; v++)  
            QntPoints[i][IPC_QNT_TRIV_MESH_UVW(TV, -1, v, -1)] = 
                                    Points[i][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
 
        /* qntP(-1, -1, w) = P(0,0,0) */ 
        for (w = 0; w < TV -> WLength; w++)  
            QntPoints[i][IPC_QNT_TRIV_MESH_UVW(TV, -1, -1, w)] = 
                                    Points[i][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for quantized weights.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:      Arrays of quantized control points and weights.           *
*   TV:             Trivariate to encode.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcInitWeightsTrivTV(CagdRType *QntPoints[], TrivTVStruct *TV)  
{ 
    int u, v, w, 
        MaxCoord =  TRIV_NUM_OF_PT_COORD(TV); 
    CagdRType  
        **Points = TV -> Points; 
 
    /* Mo weights are defined for current surface. */ 
    if (!TRIV_IS_RATIONAL_TV(TV)) 
        return; 
 
    QntPoints[0][IPC_QNT_TRIV_MESH_UVW(TV, -1, -1, -1)] = 
                                        Points[0][IPC_MESH_UV(TV,  0,  0)]; 
 
    /* qntP(u, -1, -1) = P(0,0,0) */ 
    for (u = 0; u < TV -> ULength; u++)  
        QntPoints[0][IPC_QNT_TRIV_MESH_UVW(TV, u, -1, -1)] = 
                                        Points[0][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
 
    /* Set the first left column of quantized control points. */ 
    /* qntP(-1, v, -1) = P(0,0,0) */ 
    for (v = 0; v < TV -> VLength; v++)  
        QntPoints[0][IPC_QNT_TRIV_MESH_UVW(TV, -1, v, -1)] = 
                                        Points[0][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
    /* qntP(-1, -1, w) = P(0,0,0) */ 
    for (w = 0; w < TV -> WLength; w++)  
        QntPoints[0][IPC_QNT_TRIV_MESH_UVW(TV, -1, -1, w)] = 
                                        Points[0][TRIV_MESH_UVW(TV, 0, 0, 0)]; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Find minimum quantization step for control points and weights.            *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:         Trivariate to encode.                                         *
*   Errs:       IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static CagdRType IpcFindDeltaTrivTV(TrivTVStruct *TV, 
                                    IpcCtlPtStruct *Errs, 
                                    float QntError) 
{     
    int i, u, v, w,
        MaxCoord =  TRIV_NUM_OF_PT_COORD(TV); 
    CagdBType  
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV); 
    CagdRType MaxPoint,
        **Points =  TV -> Points; 
 
    MaxPoint = -IRIT_INFNTY; 
    for (i = IsNotRational; i <= MaxCoord; i++) 
        for (u = 0; u < TV -> ULength; u++)  
	    for (v = 0; v < TV -> VLength; v++)  
	        for (w = 0; w < TV -> WLength; w++) { 
		    MaxPoint = IRIT_MAX(MaxPoint,
				   IRIT_FABS(Points[i][TRIV_MESH_UVW(TV, u,
								v, w)])); 
		}
 
    /* Set maximum overall dynamic range of the coordinates. */ 
    Errs -> DRange = MaxPoint;     
     
    return QntError; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for control points and weights.                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:         IPC control points and weights structure. Used in order     *
*                 to encode control points and weights more effectively.      *
*   TV:           Trivariate to encode.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcSaveInitialValuesTrivTV(IpcCtlPtStruct *Errs, TrivTVStruct *TV) 
{ 
    int i, 
        MaxCoord =  TRIV_NUM_OF_PT_COORD(TV); 
    CagdBType  
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV); 
 
    /* Set control points initial value. */  
    for (i = IsNotRational; i <= MaxCoord; i++) 
        Errs -> InitialValue[i] = TV -> Points[i][0]; 
 
    /* Set weight initial value. */ 
    if (IsNotRational) 
        Errs -> InitialValue[0] = 1; 
    else 
        Errs -> InitialValue[0] = TV -> Points[0][0]; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Allocate memory for quantized control points or for                       *
*   control points prediction errros.                                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   CtlPtErrors:    Arrays of control points and weights.                     *
*   IpcSrf:         Includes entire information about surface.                *
*                   Used in order to decompress surface effectively.          *
*   extend:         TRUE  = Points and weights with indexes (-1) included.    *
*                   FASLE = Regular allocation.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPointsAllocateMemory(CagdRType *CtlPtErrors[], 
                                    IpcSrfStruct *IpcSrf, 
                                    int extend) 
{ 
    int i, MaxCoord, Number; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(IpcSrf); 
 
    /* Maximum coordinates of Control Points. */ 
    MaxCoord = CAGD_NUM_OF_PT_COORD(IpcSrf -> PType); 
 
    Number = (extend == TRUE) ? IpcSrf -> PointErrs.ExtNumPoints : 
                                IpcSrf -> PointErrs.NumPoints; 
 
    /* Allocate memory for each coordinate. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        CtlPtErrors[i] = (CagdRType*)IritMalloc(sizeof(CagdRType)*Number); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Allocate memory for quantized indexes of control points.                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   qntIndexes:     Arrays of quantized control points and weights.           *
*   IpcSrf:         Includes entire information about surface.                *
*                   Used in order to decompress surface effectively.          *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcQntIndexesAllocateMemory(int *qntIndexes[], 
					IpcSrfStruct *IpcSrf)
{
    int i, MaxCoord; 
 
    /* Maximum coordinates of Control Points. */ 
    MaxCoord = CAGD_NUM_OF_PT_COORD(IpcSrf -> PType); 
 
    /* Allocate memory for each coordinate. */ 
    for (i = 0; i <= MaxCoord; i++) { 
        qntIndexes[i] = 
            (int*)IritMalloc(sizeof(int)*IpcSrf -> PointErrs.NumPoints); 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for quantized control points.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:      Arrays of quantized control points and weights.           *
*   Srfs:           BSpline surfaces to encode.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcInitPoints(CagdRType *QntPoints[], CagdSrfStruct *Srf) 
{ 
    int u, v, i, MaxCoord; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf); 
    CagdRType  
        **Points = Srf -> Points; 
 
    MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType); 
 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* set qP(-1, -1) = P(0, 0) */ 
        QntPoints[i][IPC_QNT_MESH_UV(Srf, -1, -1)] = 
                                    Points[i][IPC_MESH_UV(Srf,  0,  0)]; 
 
        /* Set the first top row of quantized control points. */ 
        /* qntP(-1, u) = P(0,0) */ 
        for (u = 0; u < Srf -> ULength; u++)  
            QntPoints[i][IPC_QNT_MESH_UV(Srf, u, -1)] = 
                                    Points[i][IPC_MESH_UV(Srf,  0,  0)]; 
 
        /* Set the first left column of quantized control points. */ 
        /* qntP(v, -1) = P(0,0) */ 
        for (v = 0; v < Srf -> VLength; v++)  
            QntPoints[i][IPC_QNT_MESH_UV(Srf, -1, v)] = 
                                    Points[i][IPC_MESH_UV(Srf,  0,  0)]; 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for quantized weights.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:      Arrays of quantized control points and weights.           *
*   Srfs:           BSpline surfaces to encode.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcInitWeights(CagdRType *QntPoints[], CagdSrfStruct *Srf)  
{ 
    int u, v, 
        MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType); 
 
    /* Mo weights are defined for current surface. */ 
    if (!CAGD_IS_RATIONAL_SRF(Srf)) 
        return; 
 
    QntPoints[0][IPC_QNT_MESH_UV(Srf, -1, -1)] = 
                                Srf -> Points[0][IPC_MESH_UV(Srf,  0,  0)]; 
 
    /* Set the first top row of quantized control points weights. */ 
    /* qntW(-1, u) = 1 */ 
    for (u = 0; u < Srf -> ULength; u++)  
        QntPoints[0][IPC_QNT_MESH_UV(Srf, u, -1)] = 
                                Srf -> Points[0][IPC_MESH_UV(Srf,  0,  0)]; 
 
    /* Set the first left column of quantized control points weights. */ 
    /* qntW(v, -1) = 1 */ 
    for (v = 0; v < Srf -> VLength; v++)  
        QntPoints[0][IPC_QNT_MESH_UV(Srf, -1, v)] = 
                                Srf -> Points[0][IPC_MESH_UV(Srf,  0,  0)]; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Find minimum quantization step for control points and weights.            *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:       BSpline surfaces to encode.                                   *
*   Errs:       IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static CagdRType IpcFindDelta(CagdSrfStruct *Srf, 
                              IpcCtlPtStruct *Errs, 
                              float QntError)
{     
    int i, u, v,
        MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType); 
    CagdRType MaxPoint,
        **Points =  Srf -> Points; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf); 

    MaxPoint = -IRIT_INFNTY; 
    for (i = IsNotRational; i <= MaxCoord; i++) 
      for (u = 0; u < Srf -> ULength; u++)  
        for (v = 0; v < Srf -> VLength; v++) { 
            MaxPoint = IRIT_MAX(MaxPoint,
			        IRIT_FABS(Points[i][CAGD_MESH_UV(Srf, u, v)]));
        } 
 
    /* Set maximum overall dynamic range of the coordinates. */ 
    Errs -> DRange = MaxPoint;     
     
    return QntError; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set initial values for control points and weights.                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:         IPC control points and weights structure. Used in order     *
*                 to encode control points and weights more effectively.      *
*   Srfs:         BSpline surfaces to encode.                                 *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcSaveInitialValues(IpcCtlPtStruct *Errs, CagdSrfStruct *Srf) 
{ 
    int i, 
        MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf); 
 
    /* Set control points initial value. */  
    for (i = IsNotRational; i <= MaxCoord; i++) 
        Errs -> InitialValue[i] = (Srf -> Points)[i][0]; 
 
    /* Set weight initial value. */ 
    if (IsNotRational) 
        Errs -> InitialValue[0] = 1; 
    else 
        Errs -> InitialValue[0] = (Srf -> Points)[0][0]; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
* Irit parser error routine.                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   Error code                                                                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
void _IpcErrHandler(int code) 
{ 
    switch (code) { 
        case IPC_ERROR_OPEN_FILE: 
	    IRIT_WARNING_MSG("Failed to open output file.\n");
	    break; 
 
	case IPC_ERROR_ZLIB: 
	    IRIT_WARNING_MSG("Error in compression/uncompresion.\n");
	    break; 
 
        case IPC_ERROR_READ_FILE: 
            IRIT_WARNING_MSG("Failed to read from the compressed file.\n");
	    break; 
 
        case IPC_ERROR_NOT_IPC_FILE: 
	    IRIT_WARNING_MSG("The file is not in IPC Format.\n"); 
	    break; 
 
        case IPC_ERROR_FILE_CORRUPTED: 
	    IRIT_WARNING_MSG("The file is corrupted.\n"); 
	    break; 
 
        case IPC_ERROR_PREDICTOR_NOT_DEFINED: 
	    IRIT_WARNING_MSG("Predictor is not defined.\n"); 
	    break; 
 
        case IPC_ERROR_INVALID_QUANTIZER_VALUE: 
	    IRIT_WARNING_MSG("Quantization error value is invalid.\n"); 
	    break; 

        case IPC_ERROR_NO_TEMP_FILE: 
	    IRIT_WARNING_MSG("Can not create temp file.\n"); 
	    break; 

        case IPC_ERROR_WRITE_TO_STDOUT: 
	    IRIT_WARNING_MSG("Can not write to stdout.\n"); 
	    break; 
    
        case IPC_ERROR_PREDICTOR_INVALID_NORMAL: 
	    IRIT_WARNING_MSG("For current predictor normal is invalid.\n");
	    break;

        case IPC_ERROR_GENERAL: 
	    IRIT_WARNING_MSG("General error occured.\n");
	    break; 
    } 

    if (_IPLongJumpActive)
        longjmp(_IPLongJumpBuffer, code); 
} 

/*              Predictors for control points and weights.                   */
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Parallelogram predictor.                                                  *
*   The control points and weights predictor.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:  Arrays of quantized control points and weights.               *
*   Errs:       IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*   Srfs:       BSpline surfaces to encode.                                   *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsSrf(CagdRType *QntPoints[], 
                                IpcCtlPtStruct *Errs, 
                                CagdSrfStruct *Srf, 
                                IpcArgs *Args)
{
    int i, u, v, Digits,
        MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType); 
    CagdRType Errors, QntErrors, DRange,
        **Points =  Srf -> Points; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    float 
        QntError = Args -> QntError;
 
    /* The maximum overall dynamic range of the coordinates. */ 
    for (Digits = 0, DRange = Errs -> DRange;
	 (int) (DRange/10) > 0; 
	 DRange /= 10, Digits +=1); 
    
    Errs -> DRange = pow(10, Digits); 
     
    /* Initial minimum value. */ 
    Errs -> MaxIndex = 0; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) 
	for (v = 0; v < Srf -> VLength; v++)  
            for (u = 0; u < Srf -> ULength; u++)  { 
		/* E(0,0) = P(0,0) - qP(-1, -1) */     
	        if (u == 0 && v == 0) { 
		    Errors = Points[i][IPC_MESH_UV(Srf, 0, 0)]  
		                - QntPoints[i][IPC_QNT_MESH_UV(Srf, -1, -1)]; 
		}  
		/* E(0,j) = P(0,j) - qP(0, j-1) */     
		else if (u == 0) { 
		    Errors = Points[i][IPC_MESH_UV(Srf, 0, v)]  
		                - QntPoints[i][IPC_QNT_MESH_UV(Srf, 0, v-1)]; 
		}
		/* E(i,0) = P(i,0) - qP(i-1, 0) */     
		else if (v == 0) { 
		    Errors = Points[i][IPC_MESH_UV(Srf, u, 0)]  
                                - QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, 0)]; 
		}
		else {
		    /* E(i,j) = P(i,j)-qP(i-1,j)-qP(i,j-1)+qP(i-1,j-1) */ 
		    Errors = Points[i][IPC_MESH_UV(Srf, u, v)]  
	                    - QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, v)]  
	                    - QntPoints[i][IPC_QNT_MESH_UV(Srf, u, v-1)]  
	                    + QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, v-1)]; 
		} 
             
		if (!IPC_QUANTIZATION_USED(QntError)) { 
		    /* Not quantized errors. */ 
		    Errs -> PErrors[i][IPC_MESH_UV(Srf, u, v)] = Errors; 
                 
		    /* Quantize error. */ 
		    QntErrors = Errors; 
		}
		else { 
                /* Q(i,j) = <E(i,j)/(D*Delta)> */ 
		    Errs -> ErrIndexes[i][IPC_MESH_UV(Srf, u, v)] =  
		        IRIT_REAL_TO_INT(Errors / (Errs -> DRange * Errs -> Delta));
 
		    /* Quantize error. */ 
		    QntErrors = Errs -> ErrIndexes[i][IPC_MESH_UV(Srf, u, v)]  
				           * (Errs -> DRange * Errs -> Delta);
 
		    /* Set maximum index. */ 
		    if (i > 0) 
		        Errs -> MaxIndex = IRIT_MAX(
			    IRIT_ABS(Errs -> ErrIndexes[i][IPC_MESH_UV(Srf, u, v)]),
			    Errs -> MaxIndex); 
		}
 
		/* qP(i,j) = qE(i,j) + qP(i-1, j) + qP(i,j-1) - qP(i-1, j-1) */
		if (u == 0 && v == 0) {
		    QntPoints[i][IPC_QNT_MESH_UV(Srf, 0, 0)] =
                                                Errs -> InitialValue[i]; 
		}
		else if (u == 0) { 
		    QntPoints[i][IPC_QNT_MESH_UV(Srf, 0, v)] =      
		        QntErrors + QntPoints[i][IPC_QNT_MESH_UV(Srf, u, v-1)];
		}
		else if (v == 0) { 
		    QntPoints[i][IPC_QNT_MESH_UV(Srf, u, v)] =      
		        QntErrors + QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, v)];
		}
		else {
		    QntPoints[i][IPC_QNT_MESH_UV(Srf, u, v)] = QntErrors  
	                    + QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, v)]  
	                    + QntPoints[i][IPC_QNT_MESH_UV(Srf, u, v-1)]  
        	            - QntPoints[i][IPC_QNT_MESH_UV(Srf, u-1, v-1)]; 
		} 
	    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform predictor.                                                        *
*   The control points and weights predictor for Tri-Variate.                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:  Arrays of quantized control points and weights.               *
*   PtDiffs:    IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*   TV:         Tri-variate to encode.                                        *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsTrivTV(CagdRType *QntPoints[], 
                                   IpcCtlPtStruct *PtDiffs,  
                                   TrivTVStruct *TV, 
                                   float QntError) 
{ 
    int i, v, 
        MaxCoord = TRIV_NUM_OF_PT_COORD(TV),
        **IndexDiffs = PtDiffs -> ErrIndexes; 
    CagdBType 
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    CagdRType 
        Delta       = QntError, 
        DRange      = PtDiffs -> DRange,
        **QntDiffs  = PtDiffs -> PErrors, 
        **Points    = TV -> Points; 
 
    PtDiffs -> MaxIndex = 0; 
 
    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Init first point. */ 
        QntPoints[i][0] = PtDiffs -> InitialValue[i]; 
        QntDiffs[i][0] = 0; 
        IndexDiffs[i][0] = 0; 
 
        /* Find differences of all points. */         
        for (v = 1; v < PtDiffs -> NumPoints; v++) { 
            QntDiffs[i][v] = Points[i][v] - QntPoints[i][v-1]; 
 
            /* Quantize difference. */ 
            if (IPC_QUANTIZATION_USED(QntError)) 
            { 
              IndexDiffs[i][v] = IRIT_REAL_TO_INT(QntDiffs[i][v]/(Delta*DRange));
              QntDiffs[i][v]   = IndexDiffs[i][v]*Delta*DRange;
            }  
             
            QntPoints[i][v] = QntDiffs[i][v] + QntPoints[i][v-1]; 
 
            /* Calculate maxIndex. */ 
            if (i > 0) 
                PtDiffs -> MaxIndex = IRIT_MAX(IRIT_ABS(IndexDiffs[i][v]),
					       PtDiffs -> MaxIndex);
        } 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform predictor for Multi-variate.                                      *
*   The control points and weights predictor.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:         Multi-variate to encode.                                      *
*   PtDiffs:    IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsMultiVar(MvarMVStruct *MV, 
                                     IpcCtlPtStruct *PtDiffs, 
                                     float QntError) 
{ 
    int i, v, 
        Len = MVAR_CTL_MESH_LENGTH(MV), 
        MaxCoord     = CAGD_NUM_OF_PT_COORD(MV -> PType),
        **IndexDiffs = PtDiffs -> ErrIndexes; 
    CagdRType *QntPoints,
        Delta       = QntError, 
        DRange      = PtDiffs -> DRange, 
        **QntDiffs  = PtDiffs -> PErrors, 
        **Points    = MV -> Points;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(MV -> PType);

    PtDiffs -> MaxIndex = 0; 
 
    /* Allocate memory for quantized memory. */ 
    QntPoints = (CagdRType*)IritMalloc(sizeof(CagdRType) * Len); 
 
    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Init first point. */ 
        QntPoints[0] = PtDiffs -> InitialValue[i]; 
        QntDiffs[i][0] = 0; 
        IndexDiffs[i][0] = 0; 
 
        /* Find differences of all points. */         
        for (v = 1; v < PtDiffs -> NumPoints; v++) { 
            QntDiffs[i][v] = Points[i][v] - QntPoints[v-1]; 
 
            /* Quantize difference. */ 
            if (IPC_QUANTIZATION_USED(QntError))  
            { 
                IndexDiffs[i][v] =
			       IRIT_REAL_TO_INT(QntDiffs[i][v]/(Delta*DRange));
                QntDiffs[i][v]   = IndexDiffs[i][v] * Delta * DRange; 
            }  
             
            QntPoints[v] = QntDiffs[i][v] + QntPoints[v-1]; 
 
            /* Calculate maxIndex. */ 
            if (i > 0) 
                PtDiffs -> MaxIndex = IRIT_MAX(IRIT_ABS(IndexDiffs[i][v]),
					       PtDiffs -> MaxIndex);
        } 
    } 
 
    IritFree(QntPoints); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform predictor.                                                        *
*   The control points and weights predictor.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   PtDiffs:    Include information about encoded points.                     *
*   Points:     Control points to encode.                                     *
*   NumPoints:  Number of points in each direction.                           *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsUniform(IpcCtlPtStruct *PtDiffs, 
                                    CagdRType *Points[],  
                                    int NumPoints, 
                                    IpcArgs *Args)
{ 
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PtDiffs -> PType),
        **IndexDiffs  = PtDiffs -> ErrIndexes; 
    CagdRType DRange, *QntPoints, 
        Delta         = Args -> QntError, 
        DRangeInit    = IPC_ROUND10(PtDiffs -> DRange),
        **QntDiffs    = PtDiffs -> PErrors;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PtDiffs -> PType);
     
    /* Allocate memory for quantized memory. */ 
    QntPoints = (CagdRType*)IritMalloc(sizeof(CagdRType) * NumPoints); 

    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Set different range for weights and control points. */ 
        DRange = (i) ? DRangeInit : 1; 
 
        /* Init first point. */ 
        QntPoints[0] = PtDiffs -> InitialValue[i]; 
        QntDiffs[i][0] = 0; 
        IndexDiffs[i][0] = 0; 
 
        /* Find differences of all points. */         
        for (v = 0; v < PtDiffs -> NumPoints - 1; v++) { 
            /* Calculate new error between predicted point to exist one. */ 
            QntDiffs[i][v] = Points[i][v+1] - QntPoints[v]; 
 
            /* Quantize difference. */ 
            if (IPC_QUANTIZATION_USED(Args -> QntError))  
            { 
                IndexDiffs[i][v] = IRIT_REAL_TO_INT(QntDiffs[i][v]/(Delta*DRange));
                QntDiffs[i][v]   = IndexDiffs[i][v] * Delta * DRange;
            }  
             
            /* Build current quantized point. */ 
            QntPoints[v+1] = QntDiffs[i][v] + QntPoints[v]; 
 
            /* Calculate maxIndex. */ 
            if (i > 0) 
                PtDiffs -> MaxIndex = IRIT_MAX(IRIT_ABS(IndexDiffs[i][v]),
					       PtDiffs -> MaxIndex);
        } 
    } 
 
    IritFree(QntPoints); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   2D Plane predictor.                                                       *
*   The control points and weights predictor.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   PtDiffs:    Includes information about encoded points.                    *
*   Points:     Control points to encode.                                     *
*   NumPoints:  Number of points in each direction.                           *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPoints2D(IpcCtlPtStruct *PtDiffs, 
                               CagdRType *Points[], 
                               int NumPoints, 
                               IpcArgs *Args)
{
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PtDiffs -> PType), 
        **IndexDiffs  = PtDiffs -> ErrIndexes; 
    CagdRType DRange,
        Delta         = Args -> QntError,
        DRangeInit    = IPC_ROUND10(PtDiffs -> DRange),
        **QntDiffs    = PtDiffs -> PErrors; 
    IrtPtType Pt0, Pt1, Pt2, Pt, Pt2D, PrevPt2D, PtErr; 
    IrtHmgnMatType InvMat, TransMat;
    IrtVecType V01, V12, Normal; 
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PtDiffs -> PType);
 
    /* Set first three points. */ 
    for (i = 1; i <= MaxCoord; i++) { 
           Pt0[i-1] = Points[i][0]; 
           Pt1[i-1] = Points[i][1]; 
           Pt2[i-1] = Points[i][2]; 
    } 
 
    if (MaxCoord == 3) {
        /* Calculate Normal vector to plane of point0, point1, point2. */ 
        IRIT_PT_SUB(V01, Pt1, Pt0);
        IRIT_PT_SUB(V12, Pt2, Pt1);
        IRIT_CROSS_PROD(Normal, V01, V12); 
 
        IPC_ZERO_PT(Normal); 
    } else 
            IRIT_PT_RESET(Normal); 
 
    /* Save Normal. */ 
    IRIT_VEC_COPY(PtDiffs -> Normal, Normal); 
     
    /* Round point to zero if needed. */ 
    IPC_ZERO_PT(Normal); 
     
    if (!IRIT_PT_EQ_ZERO(Normal)) { 
        /* Create 2D - transformation inverse matrix. */ 
        GMGenTransMatrixZ2Dir(InvMat, Pt0, Normal, 1.0); 
         
        /* Create 2D - transformation matrix. */ 
        if (!MatInverseMatrix(InvMat, TransMat))  
                _IpcErrHandler(IPC_ERROR_GENERAL); 
 
        /* Translate the base points to 2D space, */
        /* first point will always be 0. */ 
        IRIT_PT_RESET(PrevPt2D); 
    }
    else 
        IRIT_PT_COPY(PrevPt2D, Pt0); 
 
    if (!IsNotRational) 
            QntDiffs[0][0] = 0.0; 
 
    /* Find differences of all points. */   
    /* Algorithm work: */
    /* transfer all points to 2D space and then save their differences. */
    for (v = 1; v < PtDiffs -> NumPoints; v++) { 
        for (i = 1; i <= MaxCoord; i++)  
            Pt[i-1] = Points[i][v]; 

        /* Translate the base points to 2D space. */ 
        if (!IRIT_PT_EQ_ZERO(Normal)) 
            MatMultPtby4by4(Pt2D, Pt, TransMat); 
        else 
            IRIT_PT_COPY(Pt2D, Pt);         

        IRIT_PT_SUB(PtErr, Pt2D, PrevPt2D); 
        IRIT_PT_COPY(PrevPt2D, Pt2D); 
          
        /* Round point to zero if needed. */ 
        IPC_ZERO_PT(PtErr); 

        for (i = IsNotRational; i <= MaxCoord; i++) { 
            /* Set different range for weights and control points. */
            DRange = (i) ? DRangeInit : 1; 
              
            /* If weights defined. */
            if (i == 0) { 
                QntDiffs[i][v] = (v >= 2) ? 
                    Points[0][v] - QntDiffs[0][v-2] : Points[0][v];
            }
	    else 
                /* Set error between predicted point to exist one. */ 
                QntDiffs[i][v] = PtErr[i-1]; 
              
            /* Quantize difference. */ 
            if (IPC_QUANTIZATION_USED(Args -> QntError)) { 
                IndexDiffs[i][v] =
			       IRIT_REAL_TO_INT(QntDiffs[i][v]/(Delta*DRange));
                QntDiffs[i][v]   = IndexDiffs[i][v] * Delta * DRange; 
            }  

            /* Calculate maxIndex. */ 
            if (i > 0) 
                PtDiffs -> MaxIndex = IRIT_MAX(IRIT_ABS(IndexDiffs[i][v]),
					       PtDiffs -> MaxIndex);
        } 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Predict a Hord between points 2 - 3.                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   V12: Vector between points 1-2.                                           *
*   V12: Vector between points 1-3.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static IrtRType IpcArcPredictHord(IrtVecType V12, IrtVecType V23) 
{ 
    IrtRType Hord12, Hord23, Hord34; 
     
    /* Calculate the hords length. */ 
    Hord12 = IRIT_VEC_LENGTH(V12); 
    Hord23 = IRIT_VEC_LENGTH(V23); 
         
    /* Predict a Hord between points 2 - 3 */ 
    Hord34 = (Hord12 * IRIT_MIN(Hord12, Hord23) + 
              Hord23 * IRIT_MAX(Hord12, Hord23)) / (Hord12 + Hord23); 
 
    return Hord34; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Arc predictor - auxilary function.                                        *
*  Find center of the arc and it's appropriate radius.                        *
*  (x1-x0)^2 + (y1-y0)^2 = R^2                                                *
*  (x2-x0)^2 + (y2-y0)^2 = R^2                                                *
*  (x3-x0)^2 + (y3-y0)^2 = R^2                                                *
* PARAMETERS:                                                                 *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcArcFindCenterAndRadius(IrtRType *R2,  
                                      IrtPtType Pt0, 
                                      IrtPtType Pt1, 
                                      IrtPtType Pt2, 
                                      IrtPtType Pt3)
{
    IrtHmgnMatType InvMat, Mat; 
    IrtVecType Vec; 
    IrtRType x0, y0, 
	x1 = Pt1[0],
        y1 = Pt1[1],  
        x2 = Pt2[0],
	y2 = Pt2[1],  
        x3 = Pt3[0],
	y3 = Pt3[1]; 
 
    /* Find center of the circle. */ 
    MatGenUnitMat(Mat); 
    Mat[0][0] =  2*(x1-x2); 
    Mat[0][1] =  2*(x1-x3); 
    Mat[1][0] =  2*(y1-y2); 
    Mat[1][1] =  2*(y1-y3); 
 
    Vec[0] = (IRIT_SQR(x1) - IRIT_SQR(x2)) + (IRIT_SQR(y1) - IRIT_SQR(y2));  
    Vec[1] = (IRIT_SQR(x1) - IRIT_SQR(x3)) + (IRIT_SQR(y1) - IRIT_SQR(y3));  
    Vec[2] = 0;                                                          
 
    if (!MatInverseMatrix(Mat, InvMat)) { 
        *R2 = 0; 
        return; 
    } 
     
    /* Find x0, y0. */ 
    MatMultPtby4by4(Pt0, Vec, InvMat); 
    x0 = Pt0[0];       
    y0 = Pt0[1];       
 
    /* Find radius*radius. */ 
    *R2 = IRIT_SQR(x1-x0) + IRIT_SQR(y1-y0); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Arc predictor - auxilary function.                                        *
*  Predict the fourth point.                                                  *
*  (x4-x3)^2+(y4-y3)^2 = Hord^2                                               *
*  (x4-x0)^2+(y4-y0)^2 = R^2                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcArcPredictPoint(IrtPtType Pt4, 
                               IrtPtType Pt0, 
                               IrtPtType Pt3, 
                               IrtRType R, 
                               IrtRType Hord34)
{
    IrtRType dx, dy, D, x0, y0, x3, y3, x4, y4, x4_1, x4_2, y4_1, y4_2, 
                 a, b, det, t, A, B, C; 
    IrtPtType Pt4_1, Pt4_2; 
 
    if (R == 0) { 
        IRIT_PT_RESET(Pt4); 
	return; 
    } 
 
    x0 = Pt0[0];       x3 = Pt3[0];       dx = 2*(x0-x3); 
    y0 = Pt0[1];       y3 = Pt3[1];       dy = 2*(y0-y3); 
 
    D  = Hord34 - IRIT_SQR(R) - IRIT_SQR(x3) - IRIT_SQR(y3) + 
			        IRIT_SQR(x0) + IRIT_SQR(y0); 
 
    /* Pt4 == Pt3 so differences between them 0. */ 
    if (IRIT_APX_EQ(dx, 0) && IRIT_APX_EQ(dy, 0)) { 
        IRIT_PT_COPY(Pt4, Pt3); 
	return; 
    }  
 
    if (IRIT_APX_EQ(dx, 0)) { 
        y4 = D/dy; 
	x4 = x3; 
    }
    else if (IRIT_APX_EQ(dy, 0)) { 
        x4 = D/dx; 
	y4 = y3; 
    }
    else { 
        /* y4 = D/dy - (dx/dy)*x4 */ 
        /* A*x4^2 + B*x4 + C = 0 */ 
        /* Set varaibles. */ 
        a = dx/dy; 
	t = D/dy; 
	b = y3-t; 
	A = 1 + IRIT_SQR(a); 
	B = -2*x3 + 2*a*b; 
	C = IRIT_SQR(x3)+IRIT_SQR(b)-Hord34; 
	det = IRIT_SQR(B)-4*A*C; 
                 
	/* Hord prediction failed. */ 
	if (det < 0) { 
	    IRIT_PT_COPY(Pt4, Pt3); 
	    return; 
	}
	else 
	    det = sqrt(det); 
 
	x4_1 = (-B+det)/(2*A); 
	x4_2 = (-B-det)/(2*A); 
	y4_1 = t - a*x4_1; 
	y4_2 = t - a*x4_2; 
 
	IRIT_PT_SET(Pt4_1, x4_1, y4_1, 0); 
	IRIT_PT_SET(Pt4_2, x4_2, y4_2, 0); 
                 
	/* Find best point. */ 
	if (IRIT_PT_PT_DIST_SQR(Pt3, Pt4_1) < IRIT_PT_PT_DIST_SQR(Pt3, Pt4_2))  
	    IRIT_PT_COPY(Pt4, Pt4_1); 
	else 
	    IRIT_PT_COPY(Pt4, Pt4_2); 
 
	return; 
    } 
    /* Set predicted point. */         
    IRIT_PT_SET(Pt4, x4, y4, 0); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Arc predictor - auxilary function.                                        *
*   The control points and weights predictor.                                 *
*   This function is used by both predictor and reconstructor.                *
*                                                                             *
* PARAMETERS:                                                                 *
*   PtDiffs:    Includes information about encoded points.                    *
*   QntPoints:  Quantized control points to encode.                           *
*   v:          control point's index                                         *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
void _IpcPredictPointsArcAux(CagdRType **QntPoints, int v) 
{ 
    IrtHmgnMatType InvMat, TransMat;
    IrtVecType V12, V23, Normal; 
    IrtRType Hord34, R; 
    IrtPtType Pt0, Pt1, Pt2, Pt3, Pt4; 
 
    if (v < 3) return; 
 
    IPC_IRIT_PT_SET(Pt1, QntPoints, v-3); 
    IPC_IRIT_PT_SET(Pt2, QntPoints, v-2); 
    IPC_IRIT_PT_SET(Pt3, QntPoints, v-1); 

    /* Calculate Normal vector to plane of point1, point2, point3. */ 
    IRIT_PT_SUB(V12, Pt2, Pt1);
    IRIT_PT_SUB(V23, Pt3, Pt2);
    IRIT_CROSS_PROD(Normal, V12, V23); 
 
    /* Predict a Hord between points 2 - 3 */ 
    Hord34 = IpcArcPredictHord(V12, V23); 
 
    /* Create 2D - transformation inverse matrix. */ 
    if (IRIT_PT_APX_EQ_ZERO_EPS(Normal, IRIT_EPS)) { 
        /* no Arc prediction can be done. */ 
        /* return Pt3 as predicted point. */ 
        IPC_VAL_SET(QntPoints, Pt3, v);   
        return; 
    } 
     
    GMGenTransMatrixZ2Dir(TransMat, Pt3, Normal, 1.0);
 
    /* Create 2D - transformation matrix. */ 
    if (!MatInverseMatrix(TransMat, InvMat)) 
    { 
        /* no Arc prediction can be done. */ 
        /* return Pt3 as predicted point. */ 
        IPC_VAL_SET(QntPoints, Pt3, v);   
        return; 
    } 
         
    /* Translate the base points to 2D space. */ 
    MatMultPtby4by4(Pt1, Pt1, TransMat); 
    MatMultPtby4by4(Pt2, Pt2, TransMat); 
    MatMultPtby4by4(Pt3, Pt3, TransMat); 
  
    /* Calculate errors between predicted and real points. */ 
     
    /* Find center of the arc and it's appropriate radius. */ 
    IpcArcFindCenterAndRadius(&R, Pt0, Pt1, Pt2, Pt3); 
     
    /* Predict the fourth point. */ 
    IpcArcPredictPoint(Pt4, Pt0, Pt3, R, Hord34); 
 
    /* Translate the base points to 3D space. */ 
    MatMultPtby4by4(Pt4, Pt4, InvMat); 
     
    /* return Pt4 as predicted point. */ 
    IPC_VAL_SET(QntPoints, Pt4, v);   
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Arc predictor.                                                            *
*   The control points and weights predictor.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   PtDiffs:    Includes information about encoded points.                    *
*   Points:     Control points to encode.                                     *
*   NumPoints:  Number of points in each direction.                           *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsArc(IpcCtlPtStruct *PtDiffs, 
                                CagdRType *Points[], 
                                int NumPoints, 
                                IpcArgs *Args)
{
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PtDiffs -> PType), 
        **IndexDiffs  = PtDiffs -> ErrIndexes; 
    CagdRType DRange, *QntPoints[CAGD_MAX_PT_SIZE],
        Delta         = Args -> QntError, 
        DRangeInit    = IPC_ROUND10(PtDiffs -> DRange),
        **QntDiffs    = PtDiffs -> PErrors;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PtDiffs -> PType);
 
    /* Allocate memory for quantized points. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        QntPoints[i] = (CagdRType*)IritMalloc(sizeof(CagdRType)*NumPoints);
 
    /* Set first quantized point. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
            QntPoints[i][0] = PtDiffs -> InitialValue[i]; 
 
    /* Find differences of all points. */         
    for (v = 0; v < PtDiffs -> NumPoints - 1; v++) { 
        /* Arc predictor needs 3 points in order to predict 4th. */ 
        _IpcPredictPointsArcAux(QntPoints, v+1); 
             
        for (i = IsNotRational; i <= MaxCoord; i++) { 
            /* Set different range for weights and control points. */ 
            DRange = (i) ? DRangeInit : 1; 

            /* First two points predicted uniformlly. */ 
            QntDiffs[i][v] = 
                Points[i][v+1] - QntPoints[i][IPC_ARC_INDX(v, i)];

            /* Quantize difference. */ 
            if (IPC_QUANTIZATION_USED(Args -> QntError)) 
            { 
                IndexDiffs[i][v] = IRIT_REAL_TO_INT(QntDiffs[i][v]/(Delta*DRange));
                QntDiffs[i][v]   = IndexDiffs[i][v] * Delta * DRange; 
            }  

            /* Build current quantized point. */ 
            /* First two quantized points reconstructed uniformly. */ 
            QntPoints[i][v+1] = 
                QntDiffs[i][v] + QntPoints[i][IPC_ARC_INDX(v, i)];

            /* Calculate maxIndex. */ 
            if (i > 0) 
	        PtDiffs -> MaxIndex = IRIT_MAX(IRIT_ABS(IndexDiffs[i][v]),
					       PtDiffs -> MaxIndex);
	} 
    } 
 
    /* Free quantized points memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
            IritFree(QntPoints[i]); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create normal to set of control points.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Nrml:       Normal to control points                                     *
*   Points:     Points.                                                      *
*   NumPoints:  Number of points.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:                                                                    *
*****************************************************************************/
static void IpcNormalAvg(IrtVecType Nrml, CagdRType *Points[], int NumPoints)
{
    int i, iter;
    IrtVecType V1, V2;
    IrtPtType P0, P1, P2;
    CagdRType 
        EPS = 10 * IPC_QUANTIZATION_DEFAULT;
    
    IRIT_VEC_RESET(Nrml);
            
    IRIT_PT_SET(P0, Points[1][0], Points[2][0], Points[3][0]);
    IRIT_PT_SET(P1, Points[1][1], Points[2][1], Points[3][1]);
    IRIT_PT_SET(P2, Points[1][2], Points[2][2], Points[3][2]);

    IRIT_PT_SUB(V1, P1, P0);
    IRIT_PT_SUB(V2, P2, P1);

    IRIT_CROSS_PROD(Nrml, V1, V2);
    
    if (IRIT_PT_APX_EQ_ZERO_EPS(Nrml, EPS))
    {
        IRIT_VEC_RESET(Nrml);
        Nrml[2] = 1.0;
        return;
    }

    for (iter = 0; iter < 2; iter++) {
        for (i = 0; i < 3; i++) {
            if (IRIT_APX_EQ_EPS(Nrml[i], 0.0, EPS))
                    Nrml[i] = 0.0;
        }

        IRIT_VEC_SAFE_NORMALIZE(Nrml);
    }

    if (Nrml[2] < 0)
        IRIT_VEC_SCALE(Nrml, -1);
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to set to geometric point from point.                  *
****************************************************************************/
void _IpcSetToPt(CagdRType *Points[], int v, IrtPtType Pt, int MaxCoord)
{
    int i;

    IRIT_PT_RESET(Pt);
    for (i = 1; i <= MaxCoord; i++)
        Pt[i-1] = Points[i][v];
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to set point from geometric point.                     *
****************************************************************************/
void _IpcSetFromPt(CagdRType *Points[], int v, IrtPtType Pt, int MaxCoord)
{
    int i;

    for (i = 1; i <= MaxCoord; i++)
        Points[i][v] = Pt[i-1];
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to copy point to point.                                *
****************************************************************************/
void _IpcPtCopy(CagdRType *Pts1[], int v1,
                CagdRType *Pts2[], int v2,
                int MaxCoord)
{
    int i;

    for (i = 1; i <= MaxCoord; i++)
        Pts1[i][v1] = Pts2[i][v2];
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to substruct points.                                   *
****************************************************************************/
void _IpcPtSub(CagdRType *PtsRes[], int vRes,
               CagdRType *Pts1[], int v1,
               CagdRType *Pts2[], int v2,
               int MaxCoord)
{
    int i;

    for (i = 1; i <= MaxCoord; i++)
        PtsRes[i][vRes] = Pts1[i][v1] - Pts2[i][v2];
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to add points.                         		    *
****************************************************************************/
void _IpcPtAdd(CagdRType *PtsRes[], int vRes,
               CagdRType *Pts1[], int v1,
               CagdRType *Pts2[], int v2,
               int MaxCoord)
{
    int i;

    for (i = 1; i <= MaxCoord; i++)
        PtsRes[i][vRes] = Pts1[i][v1] + Pts2[i][v2];
}

/****************************************************************************
* DESCRIPTION:                                                              *
* Auxiliary function to function IpcPredictPointsAnglesSrf    		    *
****************************************************************************/
static IrtBType IpcPredictPointsAnglesAux(IpcCtlPtStruct *Errs,
                                          CagdRType *QPts[],
                                          CagdRType *Pts[],
                                          int Block,
                                          int BlockLen,
                                          IpcArgs *Args)
{
    int v, i, *ErrsIndexes[CAGD_MAX_PT_SIZE], NextAngle,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Errs -> PType);
    CagdRType *PErrors[CAGD_MAX_PT_SIZE], CosTeta, SinTeta, EPS,
        Delta  = Errs -> Delta,
        DRange = Errs -> DRange;
    CagdBType
        IsAllSame = TRUE;
    IrtHmgnMatType RotMat, InvMat, Mat;
    IrtPtType QP0, QP1, QP2, P2, Res;
    IrtVecType Trans, Vec;

    Errs -> MaxIndex = 0;
    EPS = IPC_QUANTIZATION_USED(Args -> QntError) ? 
            100.0 * Args -> QntError : IPC_QUANTIZATION_DEFAULT *1e-2;

    /* Create new references in order to simplify code. */
    for (i = 1; i <= MaxCoord; i++) {
        ErrsIndexes[i] = &(Errs -> ErrIndexes[i][Block * BlockLen]);
        PErrors[i]     = &(Errs -> PErrors[i][Block * BlockLen]);
        
        IRIT_ZAP_MEM(ErrsIndexes[i], sizeof(int) * BlockLen);
        IRIT_ZAP_MEM(PErrors[i], sizeof(CagdRType) * BlockLen);
    }

    if (BlockLen == 2) {
        _IpcPtCopy(PErrors, 0, Pts, 0, MaxCoord);
        _IpcPtSub(PErrors, 1, Pts, 1, Pts, 0, MaxCoord);

        if (IPC_QUANTIZATION_USED(Args -> QntError))
          for (v = 0; v <= 1; v++)
           for (i = 1; i <= MaxCoord; i++) {
              ErrsIndexes[i][v] =
			     IRIT_REAL_TO_INT(PErrors[i][v]/ (Delta * DRange));
              Errs -> MaxIndex = IRIT_MAX(IRIT_ABS(ErrsIndexes[i][v]),
					  Errs -> MaxIndex);
           }
        
        return TRUE;
    }

    /*Calculate normal to point's plane. */
    IpcNormalAvg(Errs -> Normal, Pts, BlockLen);

    _IpcSetToPt(Pts, 0, Trans, MaxCoord);

    /* Generate new rotatation  matrix. */
    GMGenTransMatrixZ2Dir(Mat, Trans, Errs -> Normal, 1.0);

    if (!MatInverseMatrix(Mat, InvMat)) {
        IRIT_HMGN_MAT_COPY(InvMat, Mat);
    }

    /* Rotate all points using rotation matrix. */
    CagdMatTransform(Pts, BlockLen, MaxCoord, TRUE, InvMat);

    _IpcPtCopy(QPts, 0, Pts, 0, MaxCoord);
    _IpcPtCopy(QPts, 1, Pts, 1, MaxCoord);

    _IpcSetToPt(QPts, 0, QP0, MaxCoord);
    _IpcSetToPt(QPts, 1, QP1, MaxCoord);

    IRIT_PT_SUB(Vec, QP1, QP0);
    
    _IpcSetFromPt(PErrors, 0, Trans, MaxCoord);
    _IpcSetFromPt(PErrors, 1,   Vec, MaxCoord);

    if (IPC_QUANTIZATION_USED(Args -> QntError))
        for (i = 1; i <= MaxCoord; i++) 
            for (v = 0; v <= 1; v++) {
                ErrsIndexes[i][v] =
		    IRIT_REAL_TO_INT(PErrors[i][v]/ (Delta * DRange));
                Errs -> MaxIndex =
		    IRIT_MAX(IRIT_ABS(ErrsIndexes[i][v]), Errs -> MaxIndex);
	    }
    
    for (NextAngle = 0, v = 2; v < BlockLen-1; v++) {
        CosTeta = Errs -> Angles[Block][NextAngle++ % Errs -> NumAngles[Block]];
        SinTeta = sqrt(1 - IRIT_SQR(CosTeta));
        MatGenMatRotZ(CosTeta, SinTeta, RotMat);

        _IpcSetToPt(QPts, v-2, QP0, MaxCoord);
        _IpcSetToPt(QPts, v-1, QP1, MaxCoord);
        
        IRIT_PT_SUB(Vec, QP1, QP0);

        MatMultVecby4by4(Vec, Vec,  RotMat);

        IRIT_PT_ADD(QP2, QP1, Vec);

        _IpcSetToPt(Pts, v,   P2, MaxCoord);

        IRIT_PT_SUB(Res, P2, QP2);

        if (IPC_QUANTIZATION_USED(Args -> QntError)) {
            for (i = 1; i <= MaxCoord; i++) {
		ErrsIndexes[i][v] =
				 IRIT_REAL_TO_INT(Res[i-1] / (Delta * DRange));
		PErrors[i][v] = ErrsIndexes[i][v] * Delta * DRange;
		Res[i-1] = PErrors[i][v];
		Errs -> MaxIndex = IRIT_MAX(IRIT_ABS(ErrsIndexes[i][v]),
					    Errs -> MaxIndex);
            }
        } 
        else
            _IpcSetFromPt(PErrors, v, Res, MaxCoord);
        
        IRIT_PT_ADD(QP2, Res, QP2);
        _IpcSetFromPt(QPts, v, QP2, MaxCoord);

        if (!IRIT_PT_APX_EQ_ZERO_EPS(Res, EPS))
            IsAllSame = FALSE;
    }

    _IpcPtSub(PErrors, v, Pts, v, QPts, 0, MaxCoord);
    _IpcSetToPt(PErrors, v, Res, MaxCoord);
    if (!IRIT_PT_APX_EQ_ZERO_EPS(Res, EPS))
            IsAllSame = FALSE;
    
    if (IPC_QUANTIZATION_USED(Args -> QntError)) {
        for (i = 1; i <= MaxCoord; i++) {
            ErrsIndexes[i][v] =
			    IRIT_REAL_TO_INT(PErrors[i][v] / (Delta * DRange));
            PErrors[i][v] = ErrsIndexes[i][v] * Delta * DRange;
            Errs -> MaxIndex = IRIT_MAX(IRIT_ABS(ErrsIndexes[i][v]),
					Errs -> MaxIndex);
        }
    }

    /* Clear rest of the points if predictor match perfect. */
    if (IsAllSame) {
        for (i = 1; i <= MaxCoord; i++) {
            if (IPC_QUANTIZATION_USED(Args -> QntError))
                IRIT_ZAP_MEM(ErrsIndexes[i] + 2, sizeof(int) * (BlockLen - 2));
        
            IRIT_ZAP_MEM(PErrors[i] + 2, sizeof(CagdRType) * (BlockLen - 2));
        }
    }
        
    return IsAllSame;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Angles surface Predictor.                                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   QntPoints:  Quantized points.                                             *
*   Errs:       Includes information about encoded points.                    *
*   Srf:        Surface to predict.                                           *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsAnglesSrf(CagdRType *QntPoints[],
                                      IpcCtlPtStruct *Errs,
                                      CagdSrfStruct *Srf,
                                      IpcArgs *Args)
{
    int Block, i, v, 
        NumPoints = Srf -> ULength * Srf -> VLength,
        NumBlocks = Srf -> VLength,
        BlockLen  = Srf -> ULength,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Errs -> PType);
    CagdRType *Points[CAGD_MAX_PT_SIZE], 
              *Pts[CAGD_MAX_PT_SIZE], *QPts[CAGD_MAX_PT_SIZE];
    CagdBType IsAllSame,
        IsNotRational  = !CAGD_IS_RATIONAL_PT(Errs -> PType);

    Errs -> NumBlocks = NumBlocks;
    Errs -> BlockLen  = BlockLen;
    
    /*Allocate memory for normals and predictors for each block. */
    Errs -> Nrmls = 
        (IrtVecType *) IritMalloc(sizeof(IrtVecType) * NumBlocks);
    Errs -> Predicts = 
        (IpcPredictorType *) IritMalloc(sizeof(IpcPredictorType) * NumBlocks);
    
    /* Predict differences for weights. */
    if (!IsNotRational) {
        IpcPredictWeightsUniform(Errs, Srf -> Points[0], Args);
    }

    /* Copy source control points.  */
    for (i = 1; i <= MaxCoord; i++) {
        /* Copy surfaces points in order not to change original surface. */
        Points[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * NumPoints);
	CAGD_GEN_COPY(Points[i], Srf -> Points[i],
		      sizeof(CagdRType) * NumPoints);
      
	/* Allocate memory for quantized points. */
	QPts[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * BlockLen);
    }
    
    /* Normalize points by weights. */
    if (!IsNotRational) {
        for (i = 1; i <= MaxCoord; i++)
            for (v = 0; v < NumPoints; v++)
                if (Srf -> Points[0][v])
                    Points[i][v] /= Srf -> Points[0][v];
    }

    /* Find best angles for current points, and set appropriate predictor. */
    IpcPredictPointsSetAngles(Errs, Points, NumPoints);

    /* Predict each block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        /* Make index to new block of points. */
        for (i = 1; i <= MaxCoord; i++) 
            Pts[i] = &(Points[i][Block * BlockLen]);
            
        /* Make prediction. */
        IsAllSame = 
           IpcPredictPointsAnglesAux(Errs, QPts, Pts, Block, BlockLen, Args);

        /* Set specific predictor for current block. */
        IpcAngleSetBlockPredictor(Errs, Block, -1, IsAllSame);
            
        /* Copy block normal. */
        IRIT_VEC_COPY(Errs -> Nrmls[Block], Errs -> Normal);
    }

    /* Free allocated memory. */
    for (i = 1; i <= MaxCoord; i++) {
        IritFree(Points[i]);
        IritFree(QPts[i]);
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set specific predictor to best set of angles for specific block.          *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:       Includes information about encoded points.                    *
*   Block:      Block index.                                                  *
*   Predict:    Specific predictor.                                           *
*   Perfect:    Indicator if predictor is perfect.                            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcAngleSetBlockPredictor(IpcCtlPtStruct *Errs,
                                      int Block,
                                      int Predict,
                                      CagdBType Perfect)
{
    /* Set */
    switch (Predict) {
        case 0:  
            Errs -> Predicts[Block] =
            Perfect ? IPC_SRF_PREDICTOR_ANGLES_PERFECT1 :
                      IPC_SRF_PREDICTOR_ANGLES_REGULAR1 ;
            break;
        case 1:  
            Errs -> Predicts[Block] = 
            Perfect ? IPC_SRF_PREDICTOR_ANGLES_PERFECT2 :
                      IPC_SRF_PREDICTOR_ANGLES_REGULAR2 ;
            break;
    }

    /* Change to perfect. */
    if (Predict == -1 && Perfect) {
        if (Errs -> Predicts[Block] == IPC_SRF_PREDICTOR_ANGLES_REGULAR1)
            Errs -> Predicts[Block] = IPC_SRF_PREDICTOR_ANGLES_PERFECT1;
        else if (Errs -> Predicts[Block] == IPC_SRF_PREDICTOR_ANGLES_REGULAR2)
            Errs -> Predicts[Block] = IPC_SRF_PREDICTOR_ANGLES_PERFECT2;
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Set specific predictor to best set of angles for each block.              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:       Includes information about encoded points.                    *
*   Points:     Control points.                                               *
*   NumPoints:  Number of control points.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictPointsSetAngles(IpcCtlPtStruct *Errs,
                                      CagdRType *Points[],
                                      int NumPoints)
{
    IrtPtType P0, P1, P2;
    IrtVecType V1, V2;
    CagdRType AngleCos;
    int p, Block, i;

    /* Allocate memory for set of angles for each block and their length. */
    Errs -> Angles =
        (CagdRType **) IritMalloc(sizeof(CagdRType*) * Errs -> NumBlocks);
    Errs -> NumAngles = 
        (int *) IritMalloc(sizeof(int) * Errs -> NumBlocks);

    /* Set for each block it's best set of angles. */
    for (Block = 0; Block < Errs -> NumBlocks; Block++) {
        p = Block * Errs -> BlockLen;
        IRIT_PT_SET(P0, Points[1][p],   Points[2][p],   Points[3][p]);
        IRIT_PT_SET(P1, Points[1][p+1], Points[2][p+1], Points[3][p+1]);
        IRIT_PT_SET(P2, Points[1][p+2], Points[2][p+2], Points[3][p+2]);

        IRIT_PT_SUB(V1, P1, P0);
        IRIT_PT_SUB(V2, P2, P1);

        AngleCos = IRIT_DOT_PROD(V1, V2) /
	                           (IRIT_VEC_LENGTH(V1) * IRIT_VEC_LENGTH(V2));

        for (i = 0; i < IPC_ANGLES_SET; i++)
            if (IRIT_APX_EQ_EPS(AngleCos, _IpcAnglesCos[i][0], 0.2) || i == 0) {
                Errs -> Angles[Block]    = _IpcAnglesCos[i];
                Errs -> NumAngles[Block] = _IpcAnglesNum[i];
                
                IpcAngleSetBlockPredictor(Errs, Block, i, FALSE);
            }
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform weights predictor.                                                *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:       Includes information about encoded points.                    *
*   Weights:    Weights to encode.                                            *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcPredictWeightsUniform(IpcCtlPtStruct *Errs, 
                                     CagdRType *Weights,
                                     IpcArgs *Args)
{
    int v, 
        *IndexDiffs  = Errs -> ErrIndexes[0]; 
    CagdRType *QntWeights,
        Delta         = Args -> QntError,
        DRange        = IPC_ROUND10(Errs -> DRange),
        *QntDiffs     = Errs -> PErrors[0];

    /* Allocate memory for quantized memory. */ 
    QntWeights = (CagdRType *) IritMalloc(sizeof(CagdRType)*Errs -> NumPoints);

    /* Init first and second weights. */
    for (v = 0; v <= 1; v++) {
        if (IPC_QUANTIZATION_USED(Args -> QntError)) {
            IndexDiffs[v] = IRIT_REAL_TO_INT(Weights[v] / (Delta * DRange));
            QntDiffs[v]   = IndexDiffs[v] * Delta * DRange;
        }
	else
            QntDiffs[v]   = Weights[v];

        QntWeights[v] = Weights[v];

        if (v == 1) {
            QntDiffs[1] -=  QntDiffs[0];
            IndexDiffs[1] = IRIT_REAL_TO_INT(QntDiffs[1] / (Delta * DRange));
        }
    }
    
    /* Find differences of all weights. */         
    for (v = 2; v < Errs -> NumPoints; v++) { 
        /* Calculate new error between predicted weight to exist one. */
        QntDiffs[v] = Weights[v] - QntWeights[v-2];
 
        /* Quantize difference. */ 
        if (IPC_QUANTIZATION_USED(Args -> QntError))
        { 
            IndexDiffs[v] = IRIT_REAL_TO_INT(QntDiffs[v] / (Delta * DRange));
            QntDiffs[v]   = IndexDiffs[v] * Delta * DRange;
        }
             
        /* Build current quantized point. */
        QntWeights[v] = QntDiffs[v] + QntWeights[v-2];
    }

    IritFree(QntWeights);
}

#endif /* IPC_BIN_COMPRESSION */
