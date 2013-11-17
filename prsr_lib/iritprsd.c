/******************************************************************************
* Irtprscd.c - Provide a routine to decompress Irit objects.                  *
*              Reference paper: "Coding of 3D virtual objects with NURBS"     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Yura Zharkovsky 2003                                             *
******************************************************************************/
#include <stdio.h> 
#include <math.h> 
#include "prsr_loc.h"
#include "obj_dpnd.h" 
#include "allocate.h" 
#include "geom_lib.h" 

#ifdef IPC_BIN_COMPRESSION

#ifdef __WINNT__
#include <io.h> 
#endif /* __WINNT__ */

#define IPC_STRING_LEN 256 

extern CagdRType _IpcAnglesCos[IPC_ANGLES_SET_AUX][IPC_ANGLES_MAX];
extern int _IpcAnglesNum[IPC_ANGLES_SET_AUX];

/* private declarations of reconstructors. */ 
static void IpcReconstructPointsTrivTV(TrivTVStruct *TV, IpcSrfStruct *IpcSrf);
static void IpcReconstructPointsMultiVar(MvarMVStruct *MV, 
                                         IpcSrfStruct  
                                         *IpcSrf); 
static void IpcReconstructPointsSrf(CagdSrfStruct *Srf, IpcSrfStruct *IpcSrfs);
static void IpcReconstructPointsSrfAngles(CagdSrfStruct *Srf, 
                                          IpcSrfStruct  *IpcSrfs);
static void IpcReconstructPointsUniform(CagdRType *Points[], 
                                        IpcCtlPtStruct *PointErrs, 
                                        IpcArgs *Args); 
static void IpcReconstructPoints2D(CagdRType *Points[], 
                                   IpcCtlPtStruct *PointErrs, 
                                   IpcArgs *Args);
static void IpcReconstructPointsArc(CagdRType *Points[], 
                                    IpcCtlPtStruct *PointErrs, 
                                    IpcArgs *Args);
static void IpcReconstructPointsGetAngles(IpcCtlPtStruct *Errs);
static void IpcReconstructPointsAnglesAux(IpcCtlPtStruct *Errs,
                                          CagdRType *QPts[],
                                          CagdRType *Pts[],
                                          int Block,
                                          int BlockLen,
                                          IpcArgs *Args);
static void IpcReconstructWeightsUniform(IpcCtlPtStruct *Errs, 
                                         CagdRType *Weights, 
                                         IpcArgs *Args);
static void IpcUpdateNormal(IrtVecType Normal);
static IPObjectStruct* IpcDecompressObj2(IpcFile *f, IpcArgs *Args);
static IPObjectStruct* IpcDecompressObjFromFileAux(const char *FileName,
                                                   IpcArgs *PArgs);
static int  IpcDecompressContinue(IpcFile *f, IpcArgs *Args); 
static void EndianSwapBuffer(void *Data,
                             long TypeSize,
                             long Count,
                             IpcArgs *Args);
static int IpcDecompressAsInt(IpcFile *f, void *Data, long Count, IpcArgs *Args);
static int IpcDecompressIntAsDouble(IpcFile *f, 
                                 void *Data, 
                                 long Count, 
                                 IpcArgs *Args);
static void IpcDecompress(IpcFile *f,
                          void *Data,
                          long TypeSize,
                          long Count,
                          IpcArgs *Args);
static void IpcDecompressAux(IpcFile *f, 
                             IrtBType *DestBuffer, 
                             long *DestSize,
                             IpcArgs *Args);
static void IpcSetObjPredictors(IpcFile *f, IpcArgs *Args); 
static void IpcDecompressDynamicObj(IpcFile *f, 
                                    IPObjectStruct *PObj, 
                                    IrtBType Map, 
                                    IpcArgs *Args);
static void IpcDecompressDependencies(IpcFile *f, 
                                      IPODObjectDpndncyStruct **HeadDpnds,
                                      IpcArgs *Args);
static void IpcDecompressUnion(IpcFile *f, 
                               IPObjectStruct *PObj, 
                               IpcArgs *Args);
static void IpcDecompressCtlPt(IpcFile *f, 
                               CagdCtlPtStruct *CtlPt, 
                               IpcArgs *Args);
static void IpcDecompressKV(IpcFile *f,
			    IpcKnotVectorStruct *KV, 
			    IpcArgs *Args);
static void IpcDecompressPoints(IpcFile *f, 
                                IpcCtlPtStruct *Points, 
                                IpcArgs *Args);
static void IpcDecompressPointsAngles(IpcFile *f, 
                                      IpcCtlPtStruct *Points, 
                                      IpcArgs *Args);
static void IpcDecompressAttributes(IpcFile *f, 
                                    IPAttributeStruct **HeadAttr, 
                                    IpcArgs *Args);
static void IpcDecompressStrings(IpcFile *f,
                                 IpcStrings **HeadStrs,
                                 IpcArgs *Args);
static char *IpcDecompressString(IpcFile *f, char *String, IpcArgs *Args);
static void IpcLimitPoints(CagdRType *Points[], 
                           CagdPointType PType, 
                           int NumPoints, 
                           CagdRType QntError);
static void IpcLimitPoint(IrtPtType Pt, IpcArgs *Args); 
static IPObjectStruct* IpcDecompressObjAux(IpcFile *f, IpcArgs *Args); 
static TrivTVStruct* IpcDecompressTrivTV(IpcFile *f, IpcArgs *Args); 
static MvarMVStruct* IpcDecompressMultiVar(IpcFile *f, IpcArgs *Args); 
static void IpcDecompressInstances(IpcFile *f, 
                                   IPInstanceStruct *Inst, 
                                   IpcArgs *Args);
static TrngTriangSrfStruct* IpcDecompressTriSrfs(IpcFile *f, IpcArgs *Args); 
static CagdSrfStruct* IpcDecompressSrf(IpcFile *f, IpcArgs *Args); 
static TrimSrfStruct* IpcDecompressTrimSrf(IpcFile *f, IpcArgs *Args); 
static TrimCrvStruct* IpcDecompressTrimCrvList(IpcFile *f, IpcArgs *Args); 
static TrimCrvSegStruct* IpcDecompressTrimCrvSegList(IpcFile *f, 
						     IpcArgs *Args);
static CagdCrvStruct* IpcDecompressCrvs(IpcFile *f, IpcArgs *Args); 
static IPPolygonStruct* IpcDecompressPolygons(IpcFile *f, IpcArgs *Args); 
static IPVertexStruct* IpcDecompressVertex(IpcFile *f, 
                                           IPPolygonStruct *PAdj, 
                                           IpcArgs *Args);
static void IpcDecompressPointFloat(IpcFile *f, IrtPtType Pt, IpcArgs *Args); 
static MdlModelStruct* IpcDecompressModels(IpcFile *f, IpcArgs *Args); 
static MdlTrimSrfStruct* IpcDecompressTrimSrfList(IpcFile *f, IpcArgs *Args); 
static MdlLoopStruct* IpcDecompressLoopList(IpcFile *f, IpcArgs *Args); 
static MdlTrimSegStruct* IpcDecompressTrimSegList(IpcFile *f, IpcArgs *Args); 
static void IpcDecompressObjList(IpcFile *f, 
                                 IPObjectStruct *PObj, 
                                 IpcArgs *Args);
 
static void IpcDecodeTrivTV(TrivTVStruct *TV, IpcSrfStruct  *IpcSrf); 
static void IpcDecodePointsTrivTV(TrivTVStruct *TV, IpcSrfStruct  *IpcSrf); 
static void IpcDecodeMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf); 
static void IpcDecodePointsMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf); 
static void IpcDecodeTriSrf(TrngTriangSrfStruct *TriSrf, 
                            IpcTriSrfStruct *IpcTriSrf); 
static void IpcDecodePointsTriSrf(TrngTriangSrfStruct *TriSrf, 
                                  IpcTriSrfStruct *IpcTriSrf); 
static void IpcDecodeSrf(CagdSrfStruct *Srf,  IpcSrfStruct  *IpcSrf); 
static void IpcDecodeCrvPoints(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv); 
static void IpcDecodeKnotVector(CagdRType *KnotVector, 
                                int Order, 
                                int Length, 
                                CagdBType Periodic, 
                                IpcKnotVectorStruct  *KnotVectorStruct, 
                                float QntError);
static void IpcDecodeCrv(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv); 
  
static void IpcReadHeader(IpcFile *f, IpcArgs *Args); 
static CagdRType* IpcReconstructBreakVector(IpcKnotVectorStruct *KV, 
                                            float QntError);
static void IpcReconstructKnotVector(CagdRType *KnotVector, 
                                     CagdRType *BreakValues,  
                                     int NumBreakValues, 
                                     int * MultiplicityMap, 
                                     int KnotVectorLength, 
                                     CagdRType Scale,
                                     CagdRType InitBreakValue);
static int IpcUniqeDecodeKnotVector(CagdRType *KnotVector,  
                                    int Len, 
                                    int Order, 
                                    int Length, 
                                    int Periodic, 
                                    IpcKnotVectorStruct  *KnotVectorStruct); 
static void IpcGetControlPoint(CagdCtlPtStruct *CtlPt, 
                               CagdSrfStruct *Srf, 
                               int UIndex, 
                               int VIndex); 
static void IpcGetControlPointWeight(CagdRType *Weight, 
                                     CagdSrfStruct *Srf, 
                                     int UIndex, 
                                     int VIndex); 
 static void IpcDecodePointsSrf(CagdSrfStruct *Srfs, IpcSrfStruct  *IpcSrfs); 

/******************************************************************************
* DESCRIPTION:                                                                M
*  Set quantization error for spesific Handler.  			      M
*                                                                             *
* PARAMETERS:                                                                 M
*   Handler:     A handler to the open stream.				      M
*   QntError:    Quantization error. Safe range is [0.00001 - 0.0000001].     M
*                IPC_QUANTIZATION_NONE = Quantization is not used.            M
*                                                                             *
* RETURN VALUE:                                                               M
*   void                                                                      M
*                                                                             *
* SEE ALSO:                                                                   M
*   IpcDecompressObjFromFile.                                                 M
*                                                                             *
* KEYWORDS:                                                                   M
*   IpcSetQuantization, Quantization, Handle.                                 M
******************************************************************************/
void IpcSetQuantization(int Handler, float QntError)
{
    _IPStream[Handler].QntError = QntError;
} 

/******************************************************************************
* DESCRIPTION:                                                                M
*  Decompress the Irit objects from a compressed file using Handler           M
*  to an IPObjectStructs list.       					      M
*                                                                             *
* PARAMETERS:                                                                 M
*   FileName:   A compression file to load from compressed data.              M
*                                                                             *
* RETURN VALUE:                                                               M
*   IPObjectStruct *: A list of the Irit objects.                             M
*                                                                             *
* SEE ALSO:                                                                   M
*   IpcDecompressObjFromFile.                                                 M
*                                                                             *
* KEYWORDS:                                                                   M
*   IpcDecompressObjFromFile, files, parser, uncompress                       M
******************************************************************************/
IPObjectStruct *IpcDecompressObjFromFile(const char *FileName)
{
    IpcArgs Args;

    return IpcDecompressObjFromFileAux(FileName, &Args);
} 

/******************************************************************************
* DESCRIPTION:                                                                M
*  Decompress the Irit objects from a compressed file using Handler           M
*  to an IPObjectStructs list.       					      M
*                                                                             *
* PARAMETERS:                                                                 M
*   Handler:     A handler to the open stream.				      M
*                                                                             *
* RETURN VALUE:                                                               M
*   IPObjectStruct *: A list of the Irit objects.                             M
*                                                                             *
* SEE ALSO:                                                                   M
*   IpcDecompressObjFromFile.                                                 M
*                                                                             *
* KEYWORDS:                                                                   M
*   IpcDecompressObj, files, parser, uncompress                               M
******************************************************************************/
IPObjectStruct *IpcDecompressObj(int Handler)
{
    IpcArgs Args;
    IPObjectStruct* PObjs;
    
    PObjs = IpcDecompressObjFromFileAux(_IPStream[Handler].FileName, &Args);
    _IPStream[Handler].QntError = Args.QntError;

    return PObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcDecompressObjFromFile		     *
*****************************************************************************/
static IPObjectStruct *IpcDecompressObjFromFileAux(const char *FileName, 
                                                   IpcArgs *Args)
{
    IPObjectStruct* PObjs;
    IpcFile *f; 

    /* Decompress archive file if extern compressor is used. */
    FileName = _IpcDecompressArchiveFile(FileName);

    if ((f = _IPC_OPEN_FILE(FileName, IP_READ_BIN_MODE)) == NULL)
        return NULL; 

    PObjs = IpcDecompressObj2(f, Args);
    
    _IPC_CLOSE_FILE(f);

    /* Remove temporary file. */
    _IpcRemoveTempFile(FileName);

    return PObjs;
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in order to pass/accept parameters.       			      *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   A compression file to load from compressed data.              *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   IPObjectStruct: A list of the Irit objects.                               *
******************************************************************************/
static IPObjectStruct *IpcDecompressObj2(IpcFile *f, IpcArgs *Args)
{ 
    IPObjectStruct *HeadObj;

    /* Its the setjmp itself call! */
    if (setjmp(_IPLongJumpBuffer) != 0) {
        _IPLongJumpActive = FALSE;
        _IPC_CLOSE_FILE(f);
        return NULL;
    } 
     _IPLongJumpActive = TRUE;

    /* Read header of a compressed file and compression arguments. */ 
    IpcReadHeader(f, Args);

    /* Set quantizer according to quantization step. */
    Args -> Quantizer = IPC_CAT_VALUE(Args -> QntError);
 
    /* Decompress all objects. */ 
    HeadObj = IpcDecompressObjAux(f, Args); 

    _IPLongJumpActive = FALSE;

    return HeadObj; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress archive file using extern compressor to temporary _file.       *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name of a extern file to decompress.                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   char* : Temporary archive name.                                           *
******************************************************************************/
const char *_IpcDecompressArchiveFile(const char *FileName)
{
#if !defined(IRIT_HAVE_GZIP_LIB) && defined(__WINNT__)
    /* Alternative compressor is defined. */
    char Command[IPC_COMMAND_LINE];
    const char
        *ArchiveName = _IpcGetFileNameNoExt(FileName, TRUE);
    
   /* Decompress mapping file. */
    sprintf(Command, _IPC_DECOMPRESS_COMMAND_LINE, ArchiveName);
    system(Command);

    rewind(stdout);

    FileName = IritStrdup(IPC_TEMP_FILENAME);
#endif /* !IRIT_HAVE_GZIP_LIB && __WINNT__ */

    return FileName;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Remove temporary file.                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   FileName:   File name of a extern file to decompress.                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
void _IpcRemoveTempFile(const char *TempFileName)
{
#if !defined(IRIT_HAVE_GZIP_LIB) && defined(__WINNT__)
    /* Remove temporary file. */
    remove(TempFileName);
#endif /* !IRIT_HAVE_GZIP_LIB && __WINNT__ */
}

/******************************************************************************
* DESCRIPTION:                                                                *
*  Read from the file an indicator if stream of same values is ended or not.  *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:        A handler to compressed file.                                   *
*   Args:     User defined arguments.                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static int IpcDecompressContinue(IpcFile *f, IpcArgs *Args)
{ 
    IrtBType StreamContinue;
 
    IpcDecompress(f, (IrtBType*)&StreamContinue, sizeof(IrtBType), 1, Args);
    return StreamContinue;
} 

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n type numbers, in place.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:       Input Buffer.                                                *
*   TypeSize:   Each variable size in input buffer.                          *
*   Count:      Number of elements in input buffer.                          *
*   Args:       User defined arguments.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void*: Swapped buffer.                                                   *
*****************************************************************************/
static void EndianSwapBuffer(void *Data,
                             long TypeSize,
                             long Count,
                             IpcArgs *Args)
{
    int i, j, k;
    char
	*c = (char *)Data;

    for (i = 0; i < Count; i++) {
	for (j = 0, k = TypeSize - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += TypeSize;
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress and convert each output Byte to int.                           *
*   									      *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Input Buffer of integers.                                     *
*   Count:      Number of elements in input buffer.                           *
*   Args:       User defined arguments.                                       *
*   									      *
*   RETURN VALUE:                                                             *
*   int:        Error code.                                                   *
******************************************************************************/
static int IpcDecompressAsInt(IpcFile *f, void *Data, long Count, IpcArgs *Args)
{
    int i;
    IrtBType Byte;

    for (i = 0; i < Count; ++i) {
        IpcDecompress(f, &Byte, sizeof(IrtBType), 1, Args);
        ((int*)Data)[i] = (int) Byte;
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress and convert each output float to double.                       *
*   									      *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Input Buffer of integers.                                     *
*   Count:      Number of elements in input buffer.                           *
*   Args:       User defined arguments.                                       *
*   									      *
*   RETURN VALUE:                                                             *
*   int:        Error code.                                                   *
******************************************************************************/
static int IpcDecompressIntAsDouble(IpcFile *f, 
                                 void *Data, 
                                 long Count, 
                                 IpcArgs *Args)
{
    int i, Quant;

    for (i = 0; i < Count; ++i) {
        IpcDecompress(f, &Quant, sizeof(int), 1, Args);
        ((CagdRType *)Data)[i] = Quant * IPC_QUANTIZATION_DEFAULT;
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Read compressed data from file and uncompress it.                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Data:       Input Buffer.                                                 *
*   TypeSize:   Each variable size in input buffer.                           *
*   Count:      Number of elements in input buffer.                           *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompress(IpcFile *f,
                          void *Data,
                          long TypeSize,
                          long Count,
                          IpcArgs *Args)
{ 
    int j;
    IRIT_STATIC_DATA IrtBType Buffer[IPC_OUTPUT_BUFFER_LEN];
    IRIT_STATIC_DATA long 
        Size = 0,
        DestSize = 0;
    long 
        DataLen = TypeSize * Count;

    for (j = 0; j < DataLen; j++) { 
        if (Size == DestSize) {  
            IpcDecompressAux(f, Buffer, &DestSize, Args); 
            Size = 0; 
        } 
 
        ((IrtBType*)Data)[j] = Buffer[Size++];
    }

    if (Args -> SwapEndian && TypeSize > 1)
        EndianSwapBuffer(Data, TypeSize, Count, Args);
} 

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcDecompress          		     *
*****************************************************************************/
static void IpcDecompressAux(IpcFile *f, 
                             IrtBType *DestBuffer, 
                             long *DestSize,
                             IpcArgs *Args)
{ 
    /* Read block length. */ 
    if (_IPC_READ(f, DestSize, sizeof(int)) <= 0) { 
        DestBuffer = NULL; 
        *DestSize = 0; 
    }

    if (Args -> SwapEndian)
        EndianSwapBuffer(DestSize, sizeof(long), 1, Args);
     
    if (_IPC_READ(f, DestBuffer, *DestSize) <= 0) 
        _IpcErrHandler(IPC_ERROR_READ_FILE); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Read a header from a compressed file..                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReadHeader(IpcFile *f, IpcArgs *Args) 
{ 
    int Data; 
    char Buff[64]; 

    _IPC_READ(f, &(Args -> SwapEndian), sizeof(Args -> SwapEndian));
    /* Set indicator if swap endian operation is needed. */
    Args -> SwapEndian = 
        Args -> SwapEndian == _IPThisLittleEndianHardware() ? FALSE : TRUE;
 
    /* Indicator that a file of compress type. */  
    _IPC_READ(f, Buff, sizeof(char)*2);
    if (Args -> SwapEndian)
        EndianSwapBuffer(Buff, sizeof(char), 2, Args);
    if (strncmp(Buff, "IC", 2))
        _IpcErrHandler(IPC_ERROR_NOT_IPC_FILE); 
 
    /* Read a Version of the compressed file. */ 
    if (_IPC_READ(f, &Data, sizeof(int)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED);
 
    if (_IPC_READ(f, &(Args -> QntError), sizeof(float)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED);
 
    if (_IPC_READ(f, &(Args -> SrfPredictor), sizeof(IpcPredictorType)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED);
 
    if (_IPC_READ(f, &(Args -> CrvPredictor), sizeof(IpcPredictorType)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED);
 
    if (_IPC_READ(f, &(Args -> TriSrfPredictor), sizeof(IpcPredictorType))<= 0)
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED); 
 
    if (_IPC_READ(f, &(Args -> TVPredictor), sizeof(IpcPredictorType)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED); 
 
    if (_IPC_READ(f, &(Args -> MVPredictor), sizeof(IpcPredictorType)) <= 0) 
        _IpcErrHandler(IPC_ERROR_FILE_CORRUPTED);

    if (Args -> SwapEndian) {
        EndianSwapBuffer(&Data, sizeof(int), 1, Args);
        EndianSwapBuffer(&(Args -> QntError), sizeof(float), 1, Args);
        EndianSwapBuffer(&(Args -> SrfPredictor), sizeof(float), 1, Args);
        EndianSwapBuffer(&(Args -> CrvPredictor), sizeof(float), 1, Args);
        EndianSwapBuffer(&(Args -> TriSrfPredictor), sizeof(float), 1, Args);
        EndianSwapBuffer(&(Args -> TVPredictor), sizeof(float), 1, Args);
        EndianSwapBuffer(&(Args -> MVPredictor), sizeof(float), 1, Args);
    }
} 
 
/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcDecompressObj       		     *
*****************************************************************************/
static IPObjectStruct* IpcDecompressObjAux(IpcFile *f, IpcArgs *Args)  
{ 
    IrtBType ObjMap; 
    IPObjStructType ObjType; 
    IPObjectStruct *PObj;

    /* Decompress and set in Args the object's predictors. */ 
    IpcSetObjPredictors(f, Args); 

    /* Decompress object's internal map. */ 
    IpcDecompress(f, &ObjMap, sizeof(IrtBType), 1, Args);
            
    IpcDecompressAsInt(f, &ObjType, 1, Args);

    /* Allocate object. */ 
    PObj = IPAllocObject("", ObjType, NULL); 

    /* Read Union dynamic part. */ 
    IpcDecompressUnion(f, PObj, Args); 

    /* Write object's dynamic part. */ 
    IpcDecompressDynamicObj(f, PObj, ObjMap, Args); 
    
   return PObj; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress and set the object's predictors.                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:        A handler to compressed file.                                   *
*   Args:     User defined arguments.                                         *
*             Specifies different parameters of decompression.                *
*             In decompression those arguments are read from compressed file. *
*                                                                             *
* RETURN VALUE:                                                               *
*       void                                                                  *
******************************************************************************/
static void IpcSetObjPredictors(IpcFile *f, IpcArgs *Args) 
{ 
    IrtBType ObjPredictMap; 
 
    /* Decompress and set object's predictors map. */ 
    IpcDecompress(f, &ObjPredictMap, sizeof(IrtBType), 1, Args); 
 
    /* All predictors besides curve and surface predictors */
    /* are default and not coded.                          */ 
    Args -> CrvPredictor = ObjPredictMap & 0x0F;
    ObjPredictMap >>= 4;
    Args -> SrfPredictor = ObjPredictMap & 0x0F;
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress object's dynamic part.                                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:        A handler to compressed file.                                   *
*   PObj:     Irit format objects list.                                       *
*   Map:      Mapping of the object.                                          *
*   Args:     User defined arguments.                                         *
*             Specifies different parameters of decompression.                *
*             In decompression those arguments are read from compressed file. *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressDynamicObj(IpcFile *f, 
                                    IPObjectStruct *PObj, 
                                    IrtBType Map, 
                                    IpcArgs *Args)
{
    char ObjName[IRIT_LINE_LEN_VLONG];

    if (Map & IPC_BITMAP_BBOX) 
        if (IpcDecompressContinue(f, Args))
            IpcDecompress(f, &(PObj -> BBox), sizeof(IrtRType), 
                             sizeof(IrtBboxType)/sizeof(IrtRType), Args);
    else 
        memset(&(PObj -> BBox), 0, sizeof(IrtBboxType));  
 
    if (Map & IPC_BITMAP_COUNT) 
        IpcDecompress(f, &(PObj -> Count), sizeof(int), 1, Args);
    else 
        PObj -> Count = 0; 
 
    if (Map & IPC_BITMAP_TAGS) 
        IpcDecompress(f, &(PObj -> Tags), sizeof(int), 1, Args);
    else 
        PObj -> Tags = 0; 
 
    if (Map & IPC_BITMAP_NAME)  
        IpcDecompressString(f, ObjName, Args); 
    else 
        ObjName[0] = 0;

    IP_SET_OBJ_NAME2(PObj, ObjName);
     
    if (Map & IPC_BITMAP_ATTR) 
        IpcDecompressAttributes(f, &(PObj -> Attr), Args); 
    else 
        PObj -> Attr = NULL; 
 
    if (Map & IPC_BITMAP_DPNDS) 
        IpcDecompressDependencies(f, &(PObj -> Dpnds), Args); 
    else 
        PObj -> Dpnds = NULL; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress objects attributes.                                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:        A handler to compressed file.                                   *
*   HeadAttr: Pointer to decompressed object's attributes.                    *
*   Args:     User defined arguments.                                         *
*             Specifies different parameters of decompression.                *
*             In decompression those arguments are read from compressed file. *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressAttributes(IpcFile *f, 
                                    IPAttributeStruct **HeadAttr, 
                                    IpcArgs *Args)
{
    IPAttributeStruct *Attr, *TmpAttr; 
     
    *HeadAttr = NULL; 
 
    while (IpcDecompressContinue(f, Args)) {
	char AttrName[IRIT_LINE_LEN_LONG];

	/* Create a new attribute. */
	IpcDecompressString(f, (char *) AttrName, Args);

	TmpAttr = _AttrMallocAttribute(AttrName, IP_ATTR_NONE); 
	Attr = !(*HeadAttr) ? (*HeadAttr = TmpAttr) : (Attr -> Pnext = TmpAttr);

	IpcDecompressAsInt(f, &(Attr -> Type), 1, Args);
 
	switch (Attr -> Type) { 
	    case IP_ATTR_NONE:  
	        break; 
 
	    case IP_ATTR_INT:  
		IpcDecompress(f, &(Attr -> U.I), sizeof(int), 1, Args); 
		break; 
 
	    case IP_ATTR_REAL:  
		IpcDecompress(f, &(Attr -> U.R), sizeof(IrtRType), 1, Args); 
		break; 
 
	    case IP_ATTR_UV:  
		IpcDecompress(f, &(Attr -> U.UV), sizeof(float), 2, Args); 
		break; 
 
	    case IP_ATTR_STR:  
		Attr -> U.Str = IpcDecompressString(f, NULL, Args);
		break; 
 
	    case IP_ATTR_OBJ: 
		Attr -> U.PObj = IpcDecompressObjAux(f, Args); 
		break; 
 
	    case IP_ATTR_PTR:  
	    case IP_ATTR_REFPTR: 
		IpcDecompress(f, &(Attr -> U.Ptr), sizeof(VoidPtr), 1, Args);
		break; 
        } 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress object's dependecies.                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   HeadDpnds:  Pointer to the object's decompressed dependencies.            *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of decompression.              *
*               Those arguments are read from compressed file.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressDependencies(IpcFile *f, 
                                      IPODObjectDpndncyStruct **HeadDpnds, 
                                      IpcArgs *Args) 
{ 
    IPODObjectDpndncyStruct *Dpnds, *PrevDpnds; 
 
    /* Initial Dependencies. */ 
    *HeadDpnds = 
        (IPODObjectDpndncyStruct*)IritMalloc(sizeof(IPODObjectDpndncyStruct));
    Dpnds = PrevDpnds = *HeadDpnds; 
 
    while (IpcDecompressContinue(f, Args)) { 
        IpcDecompressAttributes(f, &(Dpnds -> Attr), Args); 
        IpcDecompressStrings(f, (IpcStrings**)&(Dpnds  -> ObjParams), Args); 
        IpcDecompressStrings(f, (IpcStrings**)&(Dpnds  -> ObjDepends), Args); 
        Dpnds -> EvalExpr = IpcDecompressString(f, NULL, Args);
        IpcDecompress(f, &(Dpnds -> EvalIndex), sizeof(int), 1, Args);
        IpcDecompress(f, &(Dpnds -> NumVisits), sizeof(int), 1, Args);
        IpcDecompress(f, &(Dpnds -> NumParams), sizeof(int), 1, Args);
         
        PrevDpnds = Dpnds; 
        Dpnds -> Pnext = 
         (IPODObjectDpndncyStruct*)IritMalloc(sizeof(IPODObjectDpndncyStruct));
        Dpnds = Dpnds -> Pnext; 
    } 
 
    /* Free allocted memory. */  
    if (*HeadDpnds == Dpnds) { 
        *HeadDpnds = NULL; 
        return; 
    } 
 
    IritFree(Dpnds); 
    PrevDpnds -> Pnext = NULL; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress string from compressed file.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   String:     String to decompress to.                                      *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static char* IpcDecompressString(IpcFile *f, char *String, IpcArgs *Args)
{ 
    int Size; 
    char Str[IPC_STRING_LEN]; 
 
    IpcDecompress(f, &Size, sizeof(int), 1, Args);
    if (Size == 0)  
        return NULL; 
    else  
    if (Size == 1)  
        return IritStrdup(""); 
    else 
        Size--; 
 
    IpcDecompress(f, Str, sizeof(char), Size, Args); 
    Str[Size] = '\0';  
 
    if (String == NULL) 
        String = IritStrdup(Str); 
    else 
        strcpy(String, Str); 
 
    return String; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress strings from compressed file.                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   HeadStrs:   Strings to decompress from compressed file.                   *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressStrings(IpcFile *f,
                                 IpcStrings **HeadStrs,
                                 IpcArgs *Args)
{
    IpcStrings *Str, *PrevStr; 
 
    /* Initial String. */ 
    *HeadStrs = Str = PrevStr = (IpcStrings*)IritMalloc(sizeof(IpcStrings)); 
 
    while (IpcDecompressContinue(f, Args)) { 
        IpcDecompressString(f, Str -> Name, Args);
 
        PrevStr = Str; 
        Str -> Pnext = (IpcStrings*)IritMalloc(sizeof(IpcStrings)); 
        Str = Str -> Pnext; 
    } 
     
    /* Free allocated memory. */ 
    if (*HeadStrs == PrevStr) {  
        *HeadStrs = NULL; 
        return ; 
    } 
 
    IritFree(Str); 
    PrevStr -> Pnext = NULL; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress object's union (dynamic part).                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PObj:       Irit format objects list.                                     *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of decompression.              *
*               Those arguments are read from compressed file.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressUnion(IpcFile *f, IPObjectStruct *PObj, IpcArgs *Args)
{ 
    switch (PObj -> ObjType) { 
        case IP_OBJ_SURFACE:  
            PObj -> U.Srfs = IpcDecompressSrf(f, Args); 
            break; 
        case IP_OBJ_TRIVAR:  
            PObj -> U.Trivars = IpcDecompressTrivTV(f, Args); 
            break; 
        case IP_OBJ_MULTIVAR:  
            PObj -> U.MultiVars = IpcDecompressMultiVar(f, Args); 
            break; 
        case IP_OBJ_TRIMSRF:  
            PObj -> U.TrimSrfs = IpcDecompressTrimSrf(f, Args); 
            break; 
        case IP_OBJ_TRISRF:  
            PObj -> U.TriSrfs = IpcDecompressTriSrfs(f, Args); 
            break; 
        case IP_OBJ_POLY:  
            PObj -> U.Pl = IpcDecompressPolygons(f, Args); 
            break; 
        case IP_OBJ_CURVE:  
            PObj -> U.Crvs = IpcDecompressCrvs(f, Args); 
            break; 
        case IP_OBJ_MATRIX: 
            IpcDecompress(f, PObj -> U.Mat, sizeof(IrtRType), 
                         sizeof(IrtHmgnMatType)/sizeof(IrtRType), Args);
            break; 
        case IP_OBJ_NUMERIC: 
            IpcDecompress(f, &(PObj -> U.R), sizeof(IrtRType), 1, Args);
            break; 
        case IP_OBJ_POINT: 
            IpcDecompress(f, PObj -> U.Pt, sizeof(IrtRType),
                         sizeof(IrtPtType)/sizeof(IrtRType), Args);
            break; 
        case IP_OBJ_VECTOR: 
            IpcDecompress(f, PObj -> U.Vec, sizeof(IrtRType),
                         sizeof(IrtVecType)/sizeof(IrtRType), Args);
            break;
        case IP_OBJ_PLANE: 
            IpcDecompress(f, PObj -> U.Plane, sizeof(IrtRType),
                            sizeof(IrtPlnType)/sizeof(IrtRType), Args);
            break; 
        case IP_OBJ_CTLPT: 
            IpcDecompressCtlPt(f, &(PObj -> U.CtlPt), Args);  
            break; 
        case IP_OBJ_STRING: 
            PObj -> U.Str = IpcDecompressString(f, NULL, Args);
            break; 
        case IP_OBJ_INSTANCE: 
            IpcDecompressInstances(f, PObj -> U.Instance, Args);  
            break; 
        case IP_OBJ_MODEL: 
            PObj -> U.Mdls = IpcDecompressModels(f, Args); 
            break; 
        case IP_OBJ_LIST_OBJ: 
            IpcDecompressObjList(f, PObj, Args); 
            break; 
        default: 
            PObj -> U.VPtr = NULL; 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of objects from file.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   PObj:         Irit format objects list.                                   *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*       void                                                                  *
******************************************************************************/
static void IpcDecompressObjList(IpcFile *f, 
                                 IPObjectStruct *PObj, 
                                 IpcArgs *Args)
{
    IPObjectStruct *PObjects;     
    int i = 0; 

    /* Enlarge list of objects if needed. */
    while (IpcDecompressContinue(f, Args)) {
        PObjects = IpcDecompressObjAux(f, Args);
        IPListObjectInsert(PObj, i++, PObjects);
    }

    IPListObjectInsert(PObj, i, NULL);
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Bspline surface from file.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   PObj:         Irit format objects list.                                   *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*       void                                                                  *
******************************************************************************/
static CagdSrfStruct *IpcDecompressSrf(IpcFile *f, IpcArgs *Args) 
{ 
    IpcSrfStruct IpcSrf; 
    CagdSrfStruct *Srfs, *HeadSrfs, *TmpSrf; 
 
    HeadSrfs = NULL;     
 
    /* Read an indicator if a stream of surfaces is ended. */ 
    while (IpcDecompressContinue(f, Args)) { 
        /* Read GType. */ 
        IpcDecompress(f, &(IpcSrf.GType), sizeof(CagdGeomType), 1, Args);
         
        /* Read PType. */ 
        IpcDecompress(f, &(IpcSrf.PType), sizeof(CagdPointType), 1, Args);
 
        /* Read maximum error and index. */  
        IpcDecompress(f, &(IpcSrf.PointErrs.MaxIndex), sizeof(int), 1, Args);
 
        IpcDecompress(f, &(IpcSrf.ULength), sizeof(int), 1, Args);
        IpcDecompressAsInt(f, &(IpcSrf.UOrder), 1, Args);
         
        IpcDecompress(f, &(IpcSrf.VLength), sizeof(int), 1, Args);
        IpcDecompressAsInt(f, &(IpcSrf.VOrder), 1, Args);
 
        /* Set arguments to ipc surface. */ 
        IpcSrf.Args = Args; 
 
        /* Read only relevant for BSpline information. */ 
        if ( IpcSrf.GType == CAGD_SBSPLINE_TYPE ) { 
            IpcDecompress(f, &(IpcSrf.UPeriodic), sizeof(CagdBType), 1, Args);
            IpcDecompress(f, &(IpcSrf.VPeriodic), sizeof(CagdBType), 1, Args);
 
            /* Read and decompress U knot vector. */ 
            IpcDecompressKV(f, &(IpcSrf.UKV), Args);
     
            /* Read and decompress V knot vector. */ 
            IpcDecompressKV(f, &(IpcSrf.VKV), Args);
 
            /* Create new BSpline surface. */ 
            TmpSrf = BspPeriodicSrfNew(IpcSrf.ULength, 
                                       IpcSrf.VLength,  
                                       IpcSrf.UOrder, 
                                       IpcSrf.VOrder,  
                                       IpcSrf.UPeriodic, 
                                       IpcSrf.VPeriodic,  
                                       IpcSrf.PType); 
 
            TmpSrf -> UPeriodic = IpcSrf.UPeriodic; 
            TmpSrf -> VPeriodic = IpcSrf.VPeriodic; 
 
        } else { 
            /* Create new Bezier surface. */ 
            TmpSrf = BzrSrfNew(IpcSrf.ULength, IpcSrf.VLength, IpcSrf.PType);
        } 
 
        Srfs = (!HeadSrfs) ? (HeadSrfs = TmpSrf) : (Srfs -> Pnext = TmpSrf);
     
        /* Initialize surface. */ 
        Srfs -> GType   = IpcSrf.GType; 
         
        /* Set number of points in mesh. */ 
        IpcSrf.PointErrs.NumPoints = Srfs -> VLength * Srfs -> ULength;
        IpcSrf.PointErrs.NumBlocks = Srfs -> VLength;
        IpcSrf.PointErrs.BlockLen  = Srfs -> ULength;
        IpcSrf.PointErrs.PType     = Srfs -> PType;

        /* Read and decompress control points and weights. */
        if (Args -> SrfPredictor == IPC_SRF_PREDICTOR_ANGLES)
            IpcDecompressPointsAngles(f, &IpcSrf.PointErrs, Args);
        else
            IpcDecompressPoints(f, &IpcSrf.PointErrs, Args);

        IpcDecompressAttributes(f, &(Srfs -> Attr), Args);
  
        /* Decode */ 
        IpcDecodeSrf(Srfs, &IpcSrf); 
    } 
 
    return HeadSrfs; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Tri-variate from file.                                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of decompression.              *
*               Those arguments are read from compressed file.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static TrivTVStruct *IpcDecompressTrivTV(IpcFile *f, IpcArgs *Args) 
{ 
    IpcSrfStruct IpcSrf; 
    TrivTVStruct *TVs, *HeadTVs, *TmpTV; 
 
    HeadTVs = NULL;     
 
    /* Read an indicator if a stream of tri-variates is ended. */ 
    while (IpcDecompressContinue(f, Args)) { 
        /* Read GType and PType. */ 
        IpcDecompress(f, &(IpcSrf.GType), sizeof(TrivGeomType), 1, Args);
        IpcDecompress(f, &(IpcSrf.PType), sizeof(CagdPointType), 1, Args); 
 
        /* Read maximum error and index. */  
        IpcDecompress(f, &(IpcSrf.PointErrs.MaxIndex), sizeof(int), 1, Args);
 
        IpcDecompress(f, &(IpcSrf.ULength), sizeof(int), 1, Args);
        IpcDecompressAsInt(f, &(IpcSrf.UOrder), 1, Args); 
         
        IpcDecompress(f, &(IpcSrf.VLength), sizeof(int), 1, Args);     
        IpcDecompressAsInt(f, &(IpcSrf.VOrder), 1, Args); 
 
        IpcDecompress(f, &(IpcSrf.WLength), sizeof(int), 1, Args);     
        IpcDecompressAsInt(f, &(IpcSrf.WOrder), 1, Args);
 
        /* Set arguments to ipc surface. */ 
        IpcSrf.Args = Args; 
 
        /* Read only relevant for BSpline information. */ 
        if (TRIV_IS_BSPLINE_TV((&IpcSrf))) { 
            IpcDecompress(f, &(IpcSrf.UPeriodic), sizeof(CagdBType), 1, Args);
            IpcDecompress(f, &(IpcSrf.VPeriodic), sizeof(CagdBType), 1, Args);
            IpcDecompress(f, &(IpcSrf.WPeriodic), sizeof(CagdBType), 1, Args);
 
            /* Read and decompress U knot vector. */ 
            IpcDecompressKV(f, &(IpcSrf.UKV), Args);
     
            /* Read and decompress V knot vector. */ 
            IpcDecompressKV(f, &(IpcSrf.VKV), Args);
 
            /* Read and decompress V knot vector. */ 
            IpcDecompressKV(f, &(IpcSrf.WKV), Args);
 
            /* Create new BSpline tri-variate. */ 
            TmpTV = TrivBspTVNew(IpcSrf.ULength, 
                                 IpcSrf.VLength, 
                                 IpcSrf.WLength,  
                                 IpcSrf.UOrder,  
                                 IpcSrf.VOrder,  
                                 IpcSrf.WOrder, 
                                 IpcSrf.PType); 
 
            TmpTV -> UPeriodic = IpcSrf.UPeriodic; 
            TmpTV -> VPeriodic = IpcSrf.VPeriodic; 
            TmpTV -> WPeriodic = IpcSrf.WPeriodic; 
 
        } else { 
            /* Create new Bezier tri-variate. */ 
            TmpTV = TrivBzrTVNew(IpcSrf.ULength, 
                                 IpcSrf.VLength, 
                                 IpcSrf.WLength, 
                                 IpcSrf.PType); 
        } 
 
        TVs = (!HeadTVs) ? (HeadTVs = TmpTV) : (TVs -> Pnext = TmpTV);
     
        /* Initialize tri-variate. */ 
        TVs -> GType   = IpcSrf.GType; 
 
        /* Set number of points in mesh. */ 
        IpcSrf.PointErrs.NumPoints = 
                        TVs -> VLength * TVs -> ULength * TVs -> WLength;
        IpcSrf.PointErrs.PType = TVs -> PType; 
 
        /* Read and decompress control points and weights. */ 
        IpcDecompressPoints(f, &IpcSrf.PointErrs, Args); 
 
        IpcDecompressAttributes(f, &(TVs -> Attr), Args); 
  
        /* Decode */ 
        IpcDecodeTrivTV(TVs, &IpcSrf); 
    } 
 
    return HeadTVs; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Trimmed surfaces from file.                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   TrimSrfs*:   Decompressed BSpline trimmed surfaces.                       *
******************************************************************************/
static TrimSrfStruct* IpcDecompressTrimSrf(IpcFile *f, IpcArgs *Args) 
{ 
    TrimSrfStruct *TrimSrfs, *TmpTrimSrfs, *HeadTrimSrfs; 
 
    HeadTrimSrfs = NULL; 
 
    while (IpcDecompressContinue(f, Args)) {
        /* Allocate memory for next trimmed surface in list. */  
        TmpTrimSrfs = TrimSrfNew(NULL, NULL, TRUE); 
 
        TrimSrfs = (!HeadTrimSrfs) ? (HeadTrimSrfs = TmpTrimSrfs):
                                     (TrimSrfs -> Pnext = TmpTrimSrfs); 
        TrimSrfs -> Pnext = NULL; 
 
        /* Decompress Tags. */ 
        IpcDecompress(f, &(TrimSrfs -> Tags), sizeof(int), 1, Args);
 
        /* Decompress trimmed surface. */ 
        TrimSrfs -> Srf = IpcDecompressSrf(f, Args);             
         
        /* Compress attributes. */ 
        IpcDecompressAttributes(f, &(TrimSrfs -> Attr), Args); 
 
        /* Decompress  list of trimming curves. */ 
        TrimSrfs -> TrimCrvList = IpcDecompressTrimCrvList(f, Args);  
    } 
 
    return HeadTrimSrfs; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of trimming curve from file.                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   Crvs*:       Decompressed Trimmed curves.                                 *
******************************************************************************/
static TrimCrvStruct* IpcDecompressTrimCrvList(IpcFile *f, IpcArgs *Args) 
{ 
    TrimCrvStruct *Crvs, *TmpCrvs, *HeadCrvs; 
 
    HeadCrvs = NULL; 
 
    while (IpcDecompressContinue(f, Args)) { 
        /* Allocate memory for next curve in list. */  
        TmpCrvs = TrimCrvNew(NULL); 
         
        Crvs = (!HeadCrvs) ? (HeadCrvs = TmpCrvs) : (Crvs -> Pnext = TmpCrvs);
        Crvs -> Pnext = NULL; 
         
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(Crvs -> Attr), Args); 
 
        /* Decompress list of curve segments. */ 
        Crvs -> TrimCrvSegList = IpcDecompressTrimCrvSegList(f, Args); 
    } 

    return HeadCrvs; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of trimming curve segments from file.                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   TrimCrvSegs*: Decompressed Trimmed curves segments.                       *
******************************************************************************/
static TrimCrvSegStruct* IpcDecompressTrimCrvSegList(IpcFile *f, IpcArgs *Args)
{ 
    TrimCrvSegStruct *CrvSegs, *HeadCrvSegs, *TmpCrvSegs; 
 
    HeadCrvSegs = NULL;    
 
    while (IpcDecompressContinue(f, Args)) {
        /* Allocate memory for next curve segment in list. */  
        TmpCrvSegs = TrimCrvSegNew(NULL, NULL); 
 
        CrvSegs = (!HeadCrvSegs) ? (HeadCrvSegs = TmpCrvSegs) : 
                                   (CrvSegs -> Pnext = TmpCrvSegs); 
        CrvSegs -> Pnext = NULL; 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(CrvSegs -> Attr), Args); 
 
        /* Decompress curves. */ 
        CrvSegs -> EucCrv = IpcDecompressCrvs(f, Args); 
        CrvSegs -> UVCrv  = IpcDecompressCrvs(f, Args); 
    } 
 
    return HeadCrvSegs; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Models from compressed file.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   MdlModelStruct*: Decompressed models segments.                            *
******************************************************************************/
static MdlModelStruct* IpcDecompressModels(IpcFile *f, IpcArgs *Args) 
{ 
    MdlModelStruct *Mdls, *HeadMdls, *TmpMdls;    
     
    HeadMdls = NULL; 
 
    while (IpcDecompressContinue(f, Args)) {  
        /* Allocate memory for model. */ 
        TmpMdls = (MdlModelStruct*)IritMalloc(sizeof(MdlModelStruct)); 
        Mdls    = (!HeadMdls) ? (HeadMdls = TmpMdls) : 
                                (Mdls -> Pnext = TmpMdls); 
        Mdls -> Pnext = NULL; 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(Mdls -> Attr), Args); 
 
        Mdls -> TrimSrfList = IpcDecompressTrimSrfList(f, Args); 
        Mdls -> TrimSegList = IpcDecompressTrimSegList(f, Args); 
    } 
 
    return HeadMdls;  
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress trimmed surfaces list from file.                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   MdlTrimSrfStruct*:  Trimmed surfaces to decompress.                       *
******************************************************************************/
static MdlTrimSrfStruct* IpcDecompressTrimSrfList(IpcFile *f, IpcArgs *Args) 
{ 
    MdlTrimSrfStruct *TrimSrfs, *HeadTrimSrfs, *TmpTrimSrfs;    
     
    HeadTrimSrfs = NULL; 
 
    while (IpcDecompressContinue(f, Args)) {      
        /* Allocate memory for trimmed surface list. */ 
        TmpTrimSrfs = (MdlTrimSrfStruct*)IritMalloc(sizeof(MdlTrimSrfStruct)); 
        TrimSrfs    = (!HeadTrimSrfs) ? (HeadTrimSrfs = TmpTrimSrfs) : 
                                        (TrimSrfs -> Pnext = TmpTrimSrfs); 
        TrimSrfs -> Pnext = NULL; 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(TrimSrfs -> Attr), Args); 
 
        TrimSrfs -> Srf      = IpcDecompressSrf(f, Args); 
        TrimSrfs -> LoopList = IpcDecompressLoopList(f, Args); 
    } 
 
    return HeadTrimSrfs;  
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress trimmed surfaces loop list from file.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   MdlLoopStruct*:     Decompressed Trimmed surfaces loop list.              *
******************************************************************************/
static MdlLoopStruct* IpcDecompressLoopList(IpcFile *f, IpcArgs *Args) 
{ 
    MdlLoopStruct *LLs, *HeadLLs, *TmpLLs;    
    MdlTrimSegRefStruct *SRList, *HeadSRList, *TmpSRList; 
 
    HeadLLs = NULL; 
     
    while (IpcDecompressContinue(f, Args)) {      
        /* Allocate memory.  */ 
        TmpLLs = (MdlLoopStruct*)IritMalloc(sizeof(MdlLoopStruct)); 
        LLs    = (!HeadLLs) ? (HeadLLs = TmpLLs) : (LLs -> Pnext = TmpLLs); 
        LLs -> Pnext = NULL; 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(LLs -> Attr), Args); 
         
        HeadSRList = NULL; 
        while (IpcDecompressContinue(f, Args)) { 
            /* Allocate memory.  */ 
            TmpSRList = 
                (MdlTrimSegRefStruct*)IritMalloc(sizeof(MdlTrimSegRefStruct));
            SRList = (!HeadSRList) ? (HeadSRList = TmpSRList) : 
                                        (SRList -> Pnext = TmpSRList);
            SRList -> Pnext = NULL;             
             
            /* Decompress attributes. */ 
            IpcDecompressAttributes(f, &(SRList -> Attr), Args); 
            SRList -> TrimSeg = IpcDecompressTrimSegList(f, Args); 
 
            IpcDecompress(f, &(SRList -> Reversed), sizeof(int), 1, Args);
        } 
         
        LLs -> SegRefList = HeadSRList;  
    } 
 
    return HeadLLs; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress trimmed segments to file.                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:            A handler to compressed file.                               *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   MdlTrimSegStruct*:     Decompressed Trimmed segments.                     *
******************************************************************************/
static MdlTrimSegStruct* IpcDecompressTrimSegList(IpcFile *f, IpcArgs *Args) 
{ 
    MdlTrimSegStruct *Segs, *HeadSegs, *TmpSegs;       
 
    HeadSegs = NULL; 
     
    while (IpcDecompressContinue(f, Args)) { 
        /* Allocate memory.  */ 
        TmpSegs = (MdlTrimSegStruct*)IritMalloc(sizeof(MdlTrimSegStruct)); 
        Segs    = (!HeadSegs) ? (HeadSegs = TmpSegs) : 
                                (Segs -> Pnext = TmpSegs); 
        Segs -> Pnext = NULL; 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(Segs -> Attr), Args); 
 
        Segs -> SrfFirst  = IpcDecompressTrimSrfList(f, Args); 
        Segs -> SrfSecond = IpcDecompressTrimSrfList(f, Args); 
 
        Segs -> UVCrvFirst   = IpcDecompressCrvs(f, Args);  
        Segs -> UVCrvSecond  = IpcDecompressCrvs(f, Args);  
        Segs -> EucCrv       = IpcDecompressCrvs(f, Args); 
    } 
 
    return HeadSegs; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of curves from file.                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   CagdCrvStruct*:   Decompressed List of curves.                            *
******************************************************************************/
static CagdCrvStruct* IpcDecompressCrvs(IpcFile *f, IpcArgs *Args) 
{ 
    CagdCrvStruct *Crvs, *HeadCrvs, *TmpCrvs; 
    IpcCrvStruct IpcCrv; 
    IPAttributeStruct *Attr; 
 
    HeadCrvs = NULL; 
     
    while (IpcDecompressContinue(f, Args)) { 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &Attr, Args); 
 
        /* Decompress PType. */ 
        IpcDecompress(f, &IpcCrv.PType, sizeof(CagdPointType), 1, Args);
 
        /* Decompress GType. */ 
        IpcDecompress(f, &IpcCrv.GType, sizeof(CagdGeomType), 1, Args);
         
        IpcDecompress(f, &(IpcCrv.Length), sizeof(int), 1, Args);
        IpcDecompressAsInt(f, &(IpcCrv.Order), 1, Args);
 
        if ( IpcCrv.GType == CAGD_CBSPLINE_TYPE ) { 
            /* Decompress Periodic. */ 
            IpcDecompress(f, &IpcCrv.Periodic, sizeof(CagdBType), 1, Args);
 
            /* Write and compress knot vector. */ 
            IpcDecompressKV(f, &IpcCrv.KV, Args);
 
            /* Allocate new BSpline Curve. */ 
            TmpCrvs = BspPeriodicCrvNew(IpcCrv.Length, 
                                        IpcCrv.Order, 
                                        IpcCrv.Periodic, 
                                        IpcCrv.PType); 
        } 
        else { 
            /* Allocate new Bezier Curve. */ 
            TmpCrvs = BzrCrvNew(IpcCrv.Length, IpcCrv.PType); 
        } 
 
        Crvs = (!HeadCrvs) ? (HeadCrvs = TmpCrvs) : 
                                (Crvs -> Pnext = TmpCrvs);
 
        Crvs -> Attr     = Attr;  
        Crvs -> Order    = IpcCrv.Order; 
        Crvs -> Periodic = IpcCrv.Periodic; 
 
        IpcCrv.Args  = Args; 
        IpcCrv.PointErrs.NumPoints = Crvs -> Length; 
        IpcCrv.PointErrs.PType     = Crvs -> PType; 
 
        if (Args -> CrvPredictor == IPC_CRV_PREDICTOR_2D) { 
            if (IpcDecompressContinue(f, Args)) 
                IpcDecompress(f, IpcCrv.PointErrs.Normal, sizeof(IrtRType), 
                                sizeof(IrtVecType)/sizeof(IrtRType), Args);
            else 
                IRIT_PT_RESET(IpcCrv.PointErrs.Normal); 
        }
	else 
            /* Between n points there are n-1 differences. */ 
	    IpcCrv.PointErrs.NumPoints = Crvs -> Length - 1; 
         
        /* Read and decompress control points and weights. */ 
        IpcDecompressPoints(f, &IpcCrv.PointErrs, Args); 
 
        /* Between n points there are n-1 differences. */ 
        IpcCrv.PointErrs.NumPoints = Crvs -> Length; 
 
        /* Decode curve. */ 
        IpcDecodeCrv(Crvs, &IpcCrv); 
    } 
 
    return HeadCrvs;   
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Triangular surfaces from file.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   TrngTriangSrfStruct*:   Decompressed triangular surfaces.                 *
******************************************************************************/
static TrngTriangSrfStruct* IpcDecompressTriSrfs(IpcFile *f, IpcArgs *Args) 
{ 
    TrngTriangSrfStruct *TriSrfs, *HeadTriSrfs, *TmpTriSrfs; 
    IpcTriSrfStruct IpcTriSrf; 
    IPAttributeStruct *Attr; 
 
    HeadTriSrfs = NULL; 
     
    while (IpcDecompressContinue(f, Args)) { 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &Attr, Args); 
 
        /* Decompress GType, PType. */ 
        IpcDecompress(f, &IpcTriSrf.GType, sizeof(TrngGeomType), 1, Args);
        IpcDecompress(f, &IpcTriSrf.PType, sizeof(CagdPointType), 1, Args);
         
        IpcDecompress(f, &(IpcTriSrf.Length), sizeof(int), 1, Args);
        IpcDecompressAsInt(f, &(IpcTriSrf.Order), 1, Args);
 
        if (IpcTriSrf.GType == TRNG_TRISRF_BSPLINE_TYPE) { 
            /* Write and compress knot vector. */ 
            IpcDecompressKV(f, &IpcTriSrf.KV, Args);
 
            /* Allocate new BSpline Triangular. */ 
            TmpTriSrfs = TrngBspTriSrfNew(IpcTriSrf.Length, 
                                          IpcTriSrf.Order, 
                                          IpcTriSrf.PType); 
        } 
        else { 
            /* Allocate new Bezier Triangular. */ 
            TmpTriSrfs = TrngBzrTriSrfNew(IpcTriSrf.Length, IpcTriSrf.PType);
        } 
 
        TriSrfs = (!HeadTriSrfs) ? (HeadTriSrfs = TmpTriSrfs) : 
                                   (TriSrfs -> Pnext = TmpTriSrfs); 
 
        TriSrfs -> Attr        = Attr;  
        TriSrfs -> Order    = IpcTriSrf.Order; 
 
        IpcTriSrf.Args  = Args; 
        IpcTriSrf.PointErrs.NumPoints = TRNG_TRISRF_MESH_SIZE(TriSrfs);
        IpcTriSrf.PointErrs.PType     = TriSrfs -> PType; 
 
        /* Read and decompress control points and weights. */ 
        IpcDecompressPoints(f, &IpcTriSrf.PointErrs, Args); 
 
        /* Decode Triangular. */ 
        IpcDecodeTriSrf(TriSrfs, &IpcTriSrf); 
    } 
 
    return HeadTriSrfs;   
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress Multi-variate from file.                                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   MvarMVStruct*:   Decompressed Multi-variates.                             *
******************************************************************************/
static MvarMVStruct* IpcDecompressMultiVar(IpcFile *f, IpcArgs *Args) 
{ 
    MvarMVStruct *MVs, *HeadMVs, *TmpMVs; 
    IpcSrfStruct IpcSrf; 
    IPAttributeStruct *Attr; 
    int i; 
 
    HeadMVs = NULL; 
  
    while (IpcDecompressContinue(f, Args)) { 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &Attr, Args); 
 
        /* Decompress Gtype PType and Dim. */ 
        IpcDecompress(f, &IpcSrf.GType, sizeof(MvarGeomType), 1, Args);
        IpcDecompress(f, &IpcSrf.PType, sizeof(CagdPointType), 1, Args); 
        IpcDecompress(f, &IpcSrf.Dim  , sizeof(int), 1, Args);
 
        /* Decompress Lenghts and Orders. */ 
        IpcDecompress(f, IpcSrf.Lengths, sizeof(int), IpcSrf.Dim, Args);
        IpcDecompressAsInt(f, IpcSrf.Orders, IpcSrf.Dim, Args);
 
        if ( IpcSrf.GType == MVAR_BSPLINE_TYPE ) { 
          /* Allocate new BSpline multi-variate. */ 
          TmpMVs = MvarBspMVNew(IpcSrf.Dim, 
                                  IpcSrf.Lengths, 
                                  IpcSrf.Orders, 
                                  IpcSrf.PType);
             
          /* Decompress Periodic. */ 
          IpcDecompressAsInt(f, TmpMVs -> Periodic, IpcSrf.Dim, Args);

          /* Read and decompress knot vector. */ 
          for (i = 0; i < IpcSrf.Dim; i++) 
                IpcDecompressKV(f, &IpcSrf.KVs[i], Args);
        } else { 
            /* Allocate memory for Bezier Multi-variate. */ 
            TmpMVs = MvarBzrMVNew(IpcSrf.Dim, IpcSrf.Lengths, IpcSrf.PType); 
        } 
 
        MVs    = (!HeadMVs) ? (HeadMVs = TmpMVs) : (MVs -> Pnext = TmpMVs);
        MVs -> Attr     = Attr; 
 
        IpcSrf.PointErrs.PType     = MVs -> PType; 
        IpcSrf.PointErrs.NumPoints = MVAR_CTL_MESH_LENGTH(MVs); 
 
        /* Read and decompress control points and weights. */ 
        IpcDecompressPoints(f, &IpcSrf.PointErrs, Args); 
 
        /* Decode curve. */ 
        IpcSrf.Args  = Args; 
 
        IpcDecodeMultiVar(MVs, &IpcSrf); 
    } 
 
    return HeadMVs;   
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of polygons from file.                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:      A handler to compressed file.                                     *
*   Args:   User defined arguments.                                           *
*           Specifies different parameters of compression.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   IPPolygonStruct*:    Decompressed polygons.                               *
******************************************************************************/
static IPPolygonStruct* IpcDecompressPolygons(IpcFile *f, IpcArgs *Args) 
{ 
    IPPolygonStruct *Pls, *HeadPls, *TmpPls; 
 
    HeadPls = NULL; 
 
    while (IpcDecompressContinue(f, Args)) { 
        /* Allocate memory. */ 
        TmpPls = IPAllocPolygon(0, NULL, NULL); 
        Pls = (!HeadPls) ? (HeadPls = TmpPls) : (Pls -> Pnext = TmpPls); 
         
        /* Decompress Tags. */ 
        IpcDecompress(f, &(Pls -> Tags), sizeof(IrtBType), 1, Args);
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(Pls -> Attr), Args); 
 
        /* Decompress varaibles. */ 
        if (IpcDecompressContinue(f, Args)) { 
                IpcDecompress(f, &(Pls -> PAux), sizeof(VoidPtr), 1, Args);
                IpcDecompress(f, &(Pls -> IAux), sizeof(int), 1, Args); 
                IpcDecompress(f, &(Pls -> IAux2), sizeof(int), 1, Args); 
        } 
 
        /* Decompress  BBox of polygons. */ 
        if (IP_HAS_BBOX_POLY(Pls)) 
                IpcDecompress(f, &(Pls -> BBox), sizeof(IrtRType), 
                                sizeof(IrtBboxType)/sizeof(IrtRType), Args);
 
        Pls -> PVertex = IpcDecompressVertex(f, Pls, Args); 
 
        /* Set a polygon plane. */ 
        if (IP_HAS_PLANE_POLY(Pls)) 
                IPUpdatePolyPlane(Pls); 
 
        if (IRIT_PT_EQ_ZERO(Pls -> PVertex -> Normal)) 
              IPUpdateVrtxNrml(Pls, Pls -> Plane);      
    } 
 
    return HeadPls;     
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress intances from file.                                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Inst:       Pointer to decompressed instance.                             *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
******************************************************************************/
static void IpcDecompressInstances(IpcFile *f, 
                                   IPInstanceStruct *Inst, 
                                   IpcArgs *Args)
{ 
    /* Clear instance's structure. */ 
    memset(Inst, 0, sizeof(IPInstanceStruct)); 
 
    Inst -> Name = IpcDecompressString(f, NULL, Args);
    IpcDecompress(f, Inst -> Mat, sizeof(IrtRType), 
		  sizeof(IrtHmgnMatType) / sizeof(IrtRType), Args);
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress point from file if defined as float.                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Pt:         Point to decompress.                                          *
*                                                                             *
* RETURN VALUE:                                                               *
*   IPVertexStruct*:    Decompressed vertices.                                *
******************************************************************************/
static void IpcDecompressPointFloat(IpcFile *f, IrtPtType Pt, IpcArgs *Args) 
{ 
    int i, QntValueIndx;

#ifdef IPC_VERTEX_FLOAT 
    float TmpValue; 
    /* Decompress as float number. */
    for (i = 0; i < 3; i++) { 
        IpcDecompress(f, &TmpValue, sizeof(float), 1, Args);
        Pt[i] = TmpValue;    
    } 
#else 
    /* Use standard decompression. */
    if (IPC_QUANTIZATION_USED(Args -> QntError)) { 
        for (i = 0; i < 3; i++) { 
            IpcDecompress(f, &QntValueIndx, sizeof(int), 1, Args);
            Pt[i] = QntValueIndx * Args -> QntError; 
        }   
        
        /* Limit point precision by quantization value. */ 
        IpcLimitPoint(Pt, Args); 
  
    } 
    else {       
        IpcDecompress(f, Pt , sizeof(IrtRType),
		      sizeof(IrtPtType) / sizeof(IrtRType), Args);
    } 
#endif 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress list of vertecies from file.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   PAdj:       Adjacent polygon's reference.                                 *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   IPVertexStruct*:    Decompressed vertices.                                *
******************************************************************************/
static IPVertexStruct* IpcDecompressVertex(IpcFile *f, 
                                           IPPolygonStruct *PAdj, 
                                           IpcArgs *Args)
{
    IPVertexStruct *PVertex, *HeadVertex, *TmpVertex; 
 
    HeadVertex = NULL; 
 
    Args -> Quantizer = IPC_CAT_VALUE(Args -> QntError); 
 
    while (IpcDecompressContinue(f, Args)) { 
        /* Allocate memory for vertex. */ 
        TmpVertex = IPAllocVertex(0, NULL, NULL); 
        PVertex = (!HeadVertex) ? (HeadVertex = TmpVertex) : 
                                  (PVertex -> Pnext = TmpVertex); 
 
        /* Decompress attributes. */ 
        IpcDecompressAttributes(f, &(PVertex -> Attr), Args); 
 
        /* Set adjacent polygon's reference. */ 
        PVertex -> PAdj = PAdj; 
 
        /* Decompress varaibles. */ 
        IpcDecompress(f, &(PVertex -> Tags)  , sizeof(IrtBType), 1, Args);
 
        IpcDecompressPointFloat(f, PVertex -> Coord, Args); 
 
        /* Decompress vertex normal. */  
        if (IpcDecompressContinue(f, Args)) 
	    IpcDecompressPointFloat(f, PVertex -> Normal, Args); 
    } 
 
    return HeadVertex;    
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decompress control point from file.                                       *
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
static void IpcDecompressCtlPt(IpcFile *f, 
                               CagdCtlPtStruct *CtlPt, 
                               IpcArgs *Args)
{
    int i, MaxCoord;  
    CagdBType IsNotRational; 
 
    /* Compress attributes. */ 
    IpcDecompressAttributes(f, &(CtlPt -> Attr), Args);     
    /* Decompress type of a point. */ 
    IpcDecompress(f, &(CtlPt -> PtType), sizeof(CagdPointType), 1, Args);
 
    MaxCoord = CAGD_NUM_OF_PT_COORD(CtlPt -> PtType); 
    IsNotRational = !CAGD_IS_RATIONAL_PT(CtlPt -> PtType); 
 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        IpcDecompress(f, &(CtlPt -> Coords[i]), sizeof(CagdRType), 1, Args);
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*  Decode to list of curves from IPC curve's structure.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   Crv:    Curve to decode.                                                  *
*   IpcCrv: IPC curve structure includes entire information about curve.      *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeCrv(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv) 
{ 
    if (CAGD_IS_BSPLINE_CRV(Crv)) { 
        /* Decode knot vector. */ 
        IpcDecodeKnotVector(Crv -> KnotVector, 
                            Crv -> Order, 
                            Crv -> Length, 
                            Crv -> Periodic, 
                            &(IpcCrv -> KV), 
                            IpcCrv -> Args -> QntError); 
    } 
 
    /* Decode control points. */ 
    IpcDecodeCrvPoints(Crv, IpcCrv); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*  Decode to triangular surfaces from IPC structure.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   TriSrf:    Triangular surface to compress.                                *
*   IpcTriSrf: IPC structure includes entire information about Triangular.    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeTriSrf(TrngTriangSrfStruct *TriSrf, 
                            IpcTriSrfStruct *IpcTriSrf)
{
    if (TRNG_IS_BSPLINE_TRISRF(TriSrf)) { 
        /* Decode knot vector. */ 
        IpcDecodeKnotVector(TriSrf -> KnotVector, 
                            TriSrf -> Order, 
                            TriSrf -> Length, 
                            FALSE, 
                            &(IpcTriSrf -> KV), 
                            IpcTriSrf -> Args -> QntError); 
    } 
 
    /* Decode control points. */ 
    IpcDecodePointsTriSrf(TriSrf, IpcTriSrf); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*  Decode to multi-variate from IPC structure.                                *
*                                                                             *
* PARAMETERS:                                                                 *
*   MV:     Multi-variate to decode.                                          *
*   IpcSrf: IPC structure includes entire information about multi-variate.    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf) 
{ 
    int i; 
 
    if (MV -> GType == MVAR_BSPLINE_TYPE ) { 
        /* Decode knot vectors. */ 
        for (i = 0; i < MV -> Dim; i++) { 
            IpcDecodeKnotVector(MV -> KnotVectors[i],  
                                MV -> Orders[i], 
                                MV -> Lengths[i], 
                                MV -> Periodic[i], 
                                &(IpcSrf -> KVs[i]), 
                                IpcSrf -> Args -> QntError);
        } 
    } 
 
    /* Decode control points. */ 
    IpcDecodePointsMultiVar(MV, IpcSrf); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode curves points from IPC curve points structure.                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Crv:       Curve to encode.                                               *
*   IpcCrv:    Includes entire information about encoded curve.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeCrvPoints(CagdCrvStruct *Crv, IpcCrvStruct *IpcCrv) 
{ 
    int i,  
        NumPoints = IpcCrv -> PointErrs.NumPoints,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(Crv -> PType);     

    /* Reconstruct control points */ 
    switch (IpcCrv -> Args -> CrvPredictor) {  
        case IPC_CRV_PREDICTOR_UNIFORM: 
             IpcReconstructPointsUniform(Crv -> Points, 
                                         &(IpcCrv -> PointErrs), 
                                         IpcCrv -> Args); 
             break; 

        case IPC_CRV_PREDICTOR_2D: 
             IpcReconstructPoints2D(Crv -> Points, 
                                    &(IpcCrv -> PointErrs), 
                                    IpcCrv -> Args); 
             break; 

        case IPC_CRV_PREDICTOR_ARC: 
             IpcReconstructPointsArc(Crv -> Points, 
                                     &(IpcCrv -> PointErrs), 
                                     IpcCrv -> Args); 
             break; 

        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
     
    /* Free allocated memory for encoded curves. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        IritFree(IpcCrv -> PointErrs.PErrors[i]); 
        IritFree(IpcCrv -> PointErrs.ErrIndexes[i]); 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode triangular surface points from IPC points structure.               *
*                                                                             *
* PARAMETERS:                                                                 *
*   TriSrf:    Triangular surface to decode to.                               *
*   IpcTriSrf: Includes entire information about Triangular to compress.      *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodePointsTriSrf(TrngTriangSrfStruct *TriSrf, 
                                  IpcTriSrfStruct *IpcTriSrf)
{
    int  
        NumPoints = IpcTriSrf -> PointErrs.NumPoints,
        MaxCoord  = CAGD_NUM_OF_PT_COORD(TriSrf -> PType); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(TriSrf -> PType);     

    /* Reconstruct control points */ 
    switch (IpcTriSrf -> Args -> TriSrfPredictor) {  
	case IPC_TRISRF_PREDICTOR_UNIFORM: 
	    IpcReconstructPointsUniform(TriSrf -> Points, 
					&IpcTriSrf -> PointErrs, 
					IpcTriSrf -> Args); 
	    break; 

	default: 
	    _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode Multi-variate points from IPC Multi-variate points structure.      *
*                                                                             *
* PARAMETERS:                                                                 *
*   MV:     Multi-variate to decode.                                          *
*   IpcSrf: Includes entire information about multi-variate to decompress.    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodePointsMultiVar(MvarMVStruct *MV, IpcSrfStruct *IpcSrf) 
{ 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(MV -> PType); 
    int  
        MaxCoord = CAGD_NUM_OF_PT_COORD(MV -> PType); 
 
    /* Reconstruct control points */ 
    switch (IpcSrf -> Args -> MVPredictor) {  
        case IPC_MV_PREDICTOR_UNIFORM: 
	    IpcReconstructPointsMultiVar(MV, IpcSrf); 
	    break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Read and decompress knot vector.                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   KV:         IPC knot vector structure which includes                      *
*               entire information about knot vector.                         *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressKV(IpcFile *f,
			    IpcKnotVectorStruct *KV, 
			    IpcArgs *Args)
{
    int Size; 
    float 
        QntError = Args -> QntError;
 
    IpcDecompressAsInt(f, &(KV -> KnotVectorMap), 1, Args);
    IpcDecompress(f, &(KV -> Scale), sizeof(CagdRType), 1, Args);
    IpcDecompress(f, &(KV -> InitBreakValue), sizeof(CagdRType), 1, Args);
 
    if ((KV -> KnotVectorMap) != IPC_KNOT_VECTOR_GENERAL)  
        return; 
 
    IpcDecompress(f, &(KV -> NumBreakValues), sizeof(int), 1, Args);
     
    /* Read and decompress Multiplicity Map. */ 
    Size = KV -> NumBreakValues * sizeof(int); 
    KV -> MultiplicityMap = (int*)IritMalloc(Size); 
    IpcDecompressAsInt(f, KV -> MultiplicityMap, KV -> NumBreakValues, Args);
 
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcDecompress(f, &(KV -> DeltaK), sizeof(CagdRType), 1, Args);
         
        /* Read and decompress Error Indexes. */ 
        Size = (KV -> NumBreakValues - 1) * sizeof(int); 
        KV -> ErrIndexes = (int*)IritMalloc(Size); 
        IpcDecompress(f, (KV -> ErrIndexes), sizeof(int),
		      Size / sizeof(int), Args);
    }
    else {         
        Size = (KV -> NumBreakValues - 1) * sizeof(CagdRType); 
        KV -> KVErrors = (CagdRType*)IritMalloc(Size); 
        IpcDecompress(f, (KV -> KVErrors), sizeof(CagdRType), 
                                      Size / sizeof(CagdRType), Args);
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Read and decompress control points and weights.                           *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Points:     Structure which holds information about control points.       *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressPoints(IpcFile *f, 
                                IpcCtlPtStruct *Points, 
                                IpcArgs *Args)
{
    signed char TmpIndex; 
    int i, j, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(Points -> PType);
    long 
        NumPoints     = Points -> NumPoints;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(Points -> PType);
    float 
        QntError = Args -> QntError;
 
    /* Allocate memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        Points -> ErrIndexes[i] = 
            (int*)IritMalloc(sizeof(int) * NumPoints); 

        Points -> PErrors[i] = 
            (CagdRType*)IritMalloc(sizeof(CagdRType)*NumPoints); 
    } 
  
    IpcDecompress(f, &(Points -> PType), sizeof(CagdPointType), 1, Args);
 
    /* Read and decompress initial values of control points and weights. */ 
    IpcDecompress(f, 
                  (Points -> InitialValue + IsNotRational),
                  sizeof(CagdRType),
                  (MaxCoord + !IsNotRational),
                  Args);
 
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcDecompress(f, &(Points -> Delta), sizeof(CagdRType), 1, Args);
        IpcDecompress(f, &(Points -> DRange), sizeof(CagdRType), 1, Args); 
        IpcDecompress(f, &(Points -> MaxIndex), sizeof(int), 1, Args); 
 
        /* Read and decompress error indexes of control points and weights. */ 
        for (i = IsNotRational; i <= MaxCoord; i++) { 
          if (Points -> MaxIndex != (signed char)Points -> MaxIndex) 
              IpcDecompress(f, (Points -> ErrIndexes[i]), 
			    sizeof(int), NumPoints, Args);
          else  
              /* Resize. */ 
              for (j = 0; j < NumPoints; j++) { 
                  IpcDecompress(f, &TmpIndex, sizeof(signed char), 1, Args); 
		  Points -> ErrIndexes[i][j] = TmpIndex; 
            } 
        } 
    }
    else { 
	/* Read and decompress errors of control points and weights. */ 
        for (i = IsNotRational; i <= MaxCoord; i++)  
	    IpcDecompress(f, (Points -> PErrors[i]), 
			  sizeof(CagdRType), NumPoints, Args);
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Read and decompress control points and weights                            *
*   (angles predictor was used).                                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   f:          A handler to compressed file.                                 *
*   Points:     Structure which holds information about control points.       *
*   Args:       User defined arguments.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecompressPointsAngles(IpcFile *f, 
                                      IpcCtlPtStruct *Points, 
                                      IpcArgs *Args)
{
    int Block, i, TypeSize,
        MaxCoord  = CAGD_NUM_OF_PT_COORD(Points -> PType), 
        MaxIndex  = Points -> MaxIndex,
        NumBlocks = Points -> NumBlocks,
        BlockLen = Points -> BlockLen,
        NumPoints = Points -> NumPoints;
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_PT(Points -> PType);
    CagdRType Delta_DRange,
        QntError = Args -> QntError;
    void *BlockPtr;

    /* Allocate memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        Points -> ErrIndexes[i] = (int *) IritMalloc(sizeof(int) * NumPoints);
        Points -> PErrors[i] = 
            (CagdRType *) IritMalloc(sizeof(CagdRType) * NumPoints);
    }

    /*Allocate memory for normals and predictors for each block. */ 
    Points -> Nrmls = (IrtVecType *) IritMalloc(sizeof(IrtVecType)*NumBlocks);
    Points -> Predicts = 
        (IpcPredictorType *) IritMalloc(sizeof(IpcPredictorType)*NumBlocks);

    IpcDecompress(f, &(Points -> PType), sizeof(CagdPointType), 1, Args);

    if (IPC_QUANTIZATION_USED(QntError)) { 
        IpcDecompress(f, &(Points -> Delta), sizeof(CagdRType), 1, Args);
        IpcDecompress(f, &(Points -> DRange), sizeof(CagdRType), 1, Args); 
        IpcDecompress(f, &(Points -> MaxIndex), sizeof(int), 1, Args);

        Delta_DRange  = Points -> Delta * Points -> DRange;
        TypeSize = sizeof(int);
    }
    else {
        TypeSize = sizeof(IrtRType);
    }

    /* Number of points in block > 2, decompress normal and predictors. */
    if (BlockLen > 2) {
        /* Decompress x, y of normals (normalized). */
        for (Block = 0; Block < NumBlocks; Block++) {
            IpcDecompressIntAsDouble(f, Points -> Nrmls[Block], 2, Args);
            IpcUpdateNormal(Points -> Nrmls[Block]);
        }
    
        /* Decompress predictors for each block. */
        IpcDecompressAsInt(f, Points -> Predicts, NumBlocks, Args);
    }
    
    /* Decompress P0 and P1 in each block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        for (i = 1; i <= MaxCoord; i++) {
            if (IPC_QUANTIZATION_USED(QntError)) 
                BlockPtr = &(Points -> ErrIndexes[i][Block * BlockLen]);
            else
                BlockPtr = &(Points -> PErrors[i][Block * BlockLen]);

            if (IPC_QUANTIZATION_USED(QntError)) {
                if (MaxIndex == (signed char)MaxIndex)
                    IpcDecompressAsInt(f, BlockPtr, 2, Args);
            }
            
            IpcDecompress(f, BlockPtr, TypeSize, 2, Args);
        }
    }
    
    /* If predictor not perfect, decompress rest of the points in block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        for (i = 1; i <= MaxCoord; i++) {
            if (IPC_QUANTIZATION_USED(QntError)) 
                BlockPtr = &(Points -> ErrIndexes[i][Block * BlockLen + 2]);
            else
                BlockPtr = &(Points -> PErrors[i][Block * BlockLen + 2]);

            if (!IPC_IS_PREDICTOR_PERFECT(Points -> Predicts[Block])) {
                if (IPC_QUANTIZATION_USED(QntError) && 
                                         (MaxIndex == (signed char)MaxIndex)) {
		    IpcDecompressAsInt(f, BlockPtr, BlockLen - 2, Args);
                }

                IpcDecompress(f, BlockPtr, TypeSize, BlockLen - 2, Args);
            }
            else { /* IPC_SRF_PREDICTOR_ANGLES_PERFECT */
                IRIT_ZAP_MEM(BlockPtr, TypeSize * (BlockLen - 2));
            }
        }
    }

    /* Decompress errors of control weights. */
    if(!IsNotRational) {
        if (IPC_QUANTIZATION_USED(QntError))
            BlockPtr = Points -> ErrIndexes[0];
        else
            BlockPtr = Points -> PErrors[0];

        IpcDecompress(f, BlockPtr, TypeSize, NumPoints, Args);
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode surface from encoded irit format to regular irit format.           *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:       BSpline surfaces to decode into.                              *
*   IpcSrf:     Includes entire information about surface.                    *
*               Used in order to decode surface effectively.                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeSrf(CagdSrfStruct *Srf, IpcSrfStruct  *IpcSrf) 
{ 
    Srf ->  PType = IpcSrf -> PType; 
     
    if CAGD_IS_BSPLINE_SRF(Srf) { 
        /* Decode U knot vector. */         
        IpcDecodeKnotVector(Srf -> UKnotVector, 
                            Srf -> UOrder, 
                            Srf -> ULength, 
                            Srf -> UPeriodic, 
                            &(IpcSrf -> UKV), 
                            IpcSrf -> Args -> QntError);
      
        /* Decode V knot vector. */         
        IpcDecodeKnotVector(Srf -> VKnotVector, 
                            Srf -> VOrder, 
                            Srf -> VLength, 
                            Srf -> VPeriodic,
                            &(IpcSrf -> VKV), 
                            IpcSrf -> Args -> QntError);
    } 
 
    /* Decode points. */ 
    IpcDecodePointsSrf(Srf, IpcSrf); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode tri-variate from encoded irit format to regular irit format.       *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:         Tri-variate to decode into.                                   *
*   IpcSrf:     Includes entire information about surface.                    *
*               Used in order to decode surface effectively.                  *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodeTrivTV(TrivTVStruct *TV, IpcSrfStruct  *IpcSrf) 
{ 
    TV -> PType = IpcSrf -> PType; 
     
    if (TRIV_IS_BSPLINE_TV(TV)) { 
        /* Decode U knot vector. */         
        IpcDecodeKnotVector(TV -> UKnotVector, 
                            TV -> UOrder, 
                            TV -> ULength, 
                            TV -> UPeriodic, 
                            &(IpcSrf -> UKV), 
                            IpcSrf -> Args -> QntError); 
      
        /* Decode V knot vector. */         
        IpcDecodeKnotVector(TV -> VKnotVector, 
                            TV -> VOrder, 
                            TV -> VLength, 
                            TV -> VPeriodic, 
                            &(IpcSrf -> VKV), 
                            IpcSrf -> Args -> QntError); 
 
        /* Decode W knot vector. */         
        IpcDecodeKnotVector(TV -> WKnotVector, 
                            TV -> WOrder, 
                            TV -> WLength, 
                            TV -> WPeriodic, 
                            &(IpcSrf -> WKV),
                            IpcSrf -> Args -> QntError); 
    } 
 
    /* Decode points. */ 
    IpcDecodePointsTrivTV(TV, IpcSrf); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*  Decode knot vector from encoded Irit format to regular Irit format.        *
*                                                                             *
* PARAMETERS:                                                                 *
*   KnotVector:         Knot vector values.                                   *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*   Periodic:           Valid only for Bspline curves.                        *
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
static void IpcDecodeKnotVector(CagdRType *KnotVector, 
                                int Order, 
                                int Length, 
                                CagdBType Periodic, 
                                IpcKnotVectorStruct  *KnotVectorStruct, 
                                float QntError)
{ 
    CagdRType *BreakValues; 
    int Len = Order + Length + (Periodic ? Order - 1 : 0); 
 
    /* Checks if KnotVector is uniqe, if is uniqe, reconstruct it. */ 
    if (IpcUniqeDecodeKnotVector(KnotVector, Len, Order, Length, Periodic, 
                                 KnotVectorStruct) != IPC_KNOT_VECTOR_GENERAL)
            return; 
 
    /* reconstruct break vector */ 
    BreakValues = IpcReconstructBreakVector(KnotVectorStruct, QntError);
 
    /* reconstruct Knot vector */ 
    IpcReconstructKnotVector(KnotVector, BreakValues,  
                             KnotVectorStruct -> NumBreakValues, 
                             KnotVectorStruct -> MultiplicityMap,  
                             Len,  
                             KnotVectorStruct -> Scale, 
                             KnotVectorStruct -> InitBreakValue); 
 
    /* Free alocated memory. */  
    IritFree(BreakValues); 
 
    if (IPC_QUANTIZATION_USED(QntError)) { 
        IritFree(KnotVectorStruct -> ErrIndexes); 
        IritFree(KnotVectorStruct -> MultiplicityMap); 
    } 
    else 
        IritFree(KnotVectorStruct -> KVErrors);     
}
 
/******************************************************************************
* DESCRIPTION:                                                                *
*  Reconstruct Break Vector                                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   KV:         IPC knot vector structure which includes                      *
*               entire information about knot vector.                         *
*   QntError:   Quantization step between(0..1).                              *
*               Specifies maximum error for values.                           *
*               In addition can accept next values:                           *
*               IPC_QUANTIZATION_NONE - no quanization is used.               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static CagdRType* IpcReconstructBreakVector(IpcKnotVectorStruct *KV,
                                            float QntError) 
{ 
    int i; 
    CagdRType *qntBreakValues, BKInit, Error; 
     
    qntBreakValues =  
        (CagdRType*) IritMalloc(sizeof(CagdRType) * (KV -> NumBreakValues)); 
 
    /* Loop initialization. */
    qntBreakValues[0] = 0; 
 
    /* In order to minimize Error(0) in common case. */ 
    BKInit = -1.0 / (KV -> NumBreakValues - 1); 
 
    if (IPC_QUANTIZATION_USED(QntError)) 
        Error = KV -> ErrIndexes[0] * KV -> DeltaK; 
    else 
        Error = KV -> KVErrors[0]; 
 
    qntBreakValues[1] = Error + 2 * qntBreakValues[0] - BKInit; 
 
    /* calculate break vector */ 
    for (i = 1; i < KV -> NumBreakValues - 1; i++) { 
	if (IPC_QUANTIZATION_USED(QntError)) 
	    Error = KV -> ErrIndexes[i] * KV -> DeltaK; 
	else 
	    Error = KV -> KVErrors[i]; 
             
	qntBreakValues[i+1] =
	    Error + 2*qntBreakValues[i] - qntBreakValues[i-1];
    } 
 
    return qntBreakValues; 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Reconstruct knot vector                                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   KnotVector:         Reconstructed knot vector.                            *
*   BreakValues:        Knot vector's different values.                       *
*   NumBreakValues:     Number of different knot vector values.               *
*   MultiplicityMap:    Array of different numbers from KnotVector.           *
*   KnotVectorLength:   Length of knot vector.                                *
*   Scale:              Scaling factor of knot vector.                        *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructKnotVector(CagdRType *KnotVector,  
                                     CagdRType *BreakValues,  
                                     int NumBreakValues, 
                                     int * MultiplicityMap,  
                                     int KnotVectorLength,  
                                     CagdRType Scale, 
                                     CagdRType InitBreakValue) 
{ 
    int i, j, left, right; 
 
    for (i = left = right = 0; i < NumBreakValues; i++) { 
        left   = right; 
        right += MultiplicityMap[i] + 1; 
        for ( j = left; j < right; j++)  
            KnotVector[j] = BreakValues[i] / Scale + InitBreakValue; 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Check if knot vector is uniqe(common used), if it is uniqe,               *
*   reconstruct knot vector.                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   KnotVector:         Knot vector values.                                   *
*   Len:                Size of knot vector.                                  *
*   Order:              Order in tensor product surface (Bspline only).       *
*   Length:             Mesh size in the tensor product surface.              *
*   Periodic:           Defines knot vector size.                             *
*   KnotVectorStruct:   Includes entire information about knot vector.        *
*                                                                             *
* RETURN VALUE:                                                               *
*   int:                Knot vector type.                                     *
******************************************************************************/
/*  */ 
static int IpcUniqeDecodeKnotVector(CagdRType *KnotVector,  
                                    int Len, 
                                    int Order, 
                                    int Length, 
                                    int Periodic, 
                                    IpcKnotVectorStruct  *KnotVectorStruct)
{     
    int i; 
    CagdRType InitBreakValue; 
 
    switch (KnotVectorStruct -> KnotVectorMap) { 
        case IPC_KNOT_VECTOR_UNIFORM_FLOAT:  
            if (Periodic) 
                BspKnotUniformPeriodic(Length, Order, KnotVector); 
            else 
                BspKnotUniformFloat(Length, Order, KnotVector); 
            break; 
 
        case IPC_KNOT_VECTOR_UNIFORM_OPEN:  
            BspKnotUniformOpen(Length, Order, KnotVector); 
            break; 
 
    } 
 
    if (KnotVectorStruct -> KnotVectorMap != IPC_KNOT_VECTOR_GENERAL) { 
        /* Scale knot vector. */     
        for (i = 0; i < Len; i++)  
            KnotVector[i] *=  KnotVectorStruct -> Scale; 
 
        /* Shift knot vector. */     
        InitBreakValue = KnotVectorStruct -> InitBreakValue - KnotVector[0];
        for (i = 0; i < Len; i++)  
            KnotVector[i] +=  InitBreakValue; 
    } 
 
 
    return KnotVectorStruct -> KnotVectorMap; 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode control points and weights from encoded                            *
*   Irit surface to regular Irit surface.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:       BSpline surfaces to encode.                                   *
*   Errs:       IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodePointsSrf(CagdSrfStruct *Srf, IpcSrfStruct  *IpcSrfs) 
{ 
    int i,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType); 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf); 

 
    /* Reconstruct control points */ 
    switch (IpcSrfs -> Args -> SrfPredictor) {  
        case IPC_SRF_PREDICTOR_ANGLES:
	    IpcReconstructPointsSrfAngles(Srf, IpcSrfs);
	    break;
        case IPC_SRF_PREDICTOR_PARALLELOGRAM: 
	    IpcReconstructPointsSrf(Srf, IpcSrfs); 
	    break;
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Free allocated memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        IritFree(IpcSrfs -> PointErrs.ErrIndexes[i]); 
        IritFree(IpcSrfs -> PointErrs.PErrors[i]);    
    } 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decode control points and weights from encoded                            *
*   Irit surface to regular Irit surface.                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:         Tri-variate to decode into.                                   *
*   Errs:       IPC control points and weights structure. Used in order       *
*               to encode control points and weights more effectively.        *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcDecodePointsTrivTV(TrivTVStruct *TV, IpcSrfStruct  *IpcSrf) 
{ 
    int i, 
        MaxCoord = TRIV_NUM_OF_PT_COORD(TV); 
    CagdBType  
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV); 
 
    /* Reconstruct control points */ 
    switch (IpcSrf -> Args -> TVPredictor) {  
        case IPC_TV_PREDICTOR_UNIFORM:
	    IpcReconstructPointsTrivTV(TV, IpcSrf); 
	    break; 
        default: 
            _IpcErrHandler(IPC_ERROR_PREDICTOR_NOT_DEFINED); 
    } 
 
    /* Free allocated memory. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        IritFree(IpcSrf -> PointErrs.ErrIndexes[i]); 
        IritFree(IpcSrf -> PointErrs.PErrors[i]);     
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Limit points precision.                                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   Points:       Reconstructed control points.                               *
*   PType:        Points type.                                                *
*   NumPoints:    Number of points on each axis.                              *
*   QntError:     Quantization step between(0..1).                            *
*                 Specifies maximum error for values.                         *
*                 In addition can accept next values:                         *
*                 IPC_QUANTIZATION_NONE - no quanization is used.             *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcLimitPoints(CagdRType *Points[], 
                           CagdPointType PType, 
                           int NumPoints, 
                           CagdRType QntError) 
{ 
    int Quantizer, i, v,
        MaxCoord = CAGD_NUM_OF_PT_COORD(PType); 
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PType);  
 
    /* Round reconstructed points to maximum quantization value. */ 
    if (IPC_QUANTIZATION_USED(QntError)) { 
	Quantizer = IPC_CAT_VALUE(QntError); 
	for (i = IsNotRational; i <= MaxCoord; i++)  
	    for (v = 0; v < NumPoints; v++) { 
	        Points[i][v] = floor((float) (Points[i][v] * Quantizer + 0.5))
								/ Quantizer;
		/* If values is around zero set the value to 0. */ 
		Points[i][v] = IPC_LIMIT_ZERO(Points[i][v]); 
		Points[i][v] = IPC_LIMIT_ONE(Points[i][v]); 
 
		/* If first and last pts differs by eps, make them equal. */
		if (IRIT_APX_UEQ(Points[i][NumPoints-1], Points[i][0])) 
		    Points[i][NumPoints-1], Points[i][0]; 
	    } 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Limit point precision.                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   Pt:           Point to normalize.                                         *
*   QntError:     Quantization step between(0..1).                            *
*                 Specifies maximum error for values.                         *
*                 In addition can accept next values:                         *
*                 IPC_QUANTIZATION_NONE - no quanization is used.             *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcLimitPoint(IrtPtType Pt, IpcArgs *Args) 
{ 
    int i, 
        Quantizer = Args -> Quantizer; 
 
    if (IPC_QUANTIZATION_USED(Args -> QntError)) { 
        for (i = 0; i < sizeof(IrtPtType) / sizeof(IrtRType); i++) { 
            Pt[i] =  floor((float)(Pt[i] * Quantizer + 0.5)) / Quantizer;  
            Pt[i] = IPC_LIMIT_ZERO(Pt[i]); 
            Pt[i] = IPC_LIMIT_ONE(Pt[i]); 
        } 
    } 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Parallelogram reconstructor.                                              *
*   Reconstruct control points and weights from encoded                       *
*   control points and weights.                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:       BSpline surfaces to reconstruct into.                         *
*   IpcSrfs:    Includes entire information about surfaces.                   *
*               Used in order to reconstruct surfaces effectively.            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsSrf(CagdSrfStruct *Srf, IpcSrfStruct  *IpcSrfs)
{ 
    int i, u, v, NumPoints, 
	MaxCoord =  CAGD_NUM_OF_PT_COORD(Srf -> PType),
        **ErrIndexes = IpcSrfs -> PointErrs.ErrIndexes; 
    CagdBType  
        IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf); 
    CagdRType  
        **Points = Srf -> Points, 
        **Errors = IpcSrfs -> PointErrs.PErrors, 
        QntError = IpcSrfs -> Args -> QntError; 
  
    NumPoints = Srf -> VLength * Srf -> ULength; 
 
    for (i = IsNotRational; i <= MaxCoord; i++) 
	for (v = 0; v < Srf -> VLength; v++)  
	    for (u = 0; u < Srf -> ULength; u++) { 
         
	        /* P(i,j) = E(i,j)  + P(i-1,j) + P(i,j-1) - P(i-1,j-1) */ 
	        if (IPC_QUANTIZATION_USED(QntError)) { 
		    Points[i][IPC_MESH_UV(Srf, u, v)] =  
		        ErrIndexes[i][IPC_MESH_UV(Srf, u, v)]  
		            * IpcSrfs -> PointErrs.Delta
		            * IpcSrfs -> PointErrs.DRange; 
		} 
		else { 
		    Points[i][IPC_MESH_UV(Srf, u, v)] = 
                                        Errors[i][IPC_MESH_UV(Srf, u, v)];
		}
 
		/* Set initial values of control points and weights. */ 
		if (u == 0 && v == 0) { 
		    Points[i][IPC_MESH_UV(Srf, 0, 0)] += 
                                    IpcSrfs -> PointErrs.InitialValue[i];
		}
		else if (u == 0) { 
		    Points[i][CAGD_MESH_UV(Srf, 0, v)] += 
                                    Points[i][IPC_MESH_UV(Srf, 0 , v - 1)];
		}
		else if (v == 0) { 
		    Points[i][CAGD_MESH_UV(Srf, u, 0)] += 
                                    Points[i][IPC_MESH_UV(Srf, u - 1, 0)];
		}
		else { 
		    Points[i][CAGD_MESH_UV(Srf, u, v)] +=      
		        Points[i][IPC_MESH_UV(Srf, u - 1, v)]  
			      + Points[i][IPC_MESH_UV(Srf, u , v - 1)]  
		              - Points[i][IPC_MESH_UV(Srf, u - 1, v - 1)]; 
		}
	    }
 
    /* Round reconstructed points to maximum quantization value. */ 
    IpcLimitPoints(Points, Srf -> PType, NumPoints, QntError); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform Tri-variate reconstructor.                                        *
*   Reconstruct control points and weights from encoded                       *
*   control points and weights.                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   TV:         Tri-variate to reconstruct into.                              *
*   IpcSrfs:    Includes entire information about surfaces.                   *
*               Used in order to reconstruct surfaces effectively.            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsTrivTV(TrivTVStruct *TV, IpcSrfStruct  *IpcSrf)
{ 
    int i, v, 
        MaxCoord      = TRIV_NUM_OF_PT_COORD(TV),
        NumPoints     = IpcSrf -> PointErrs.NumPoints,
        **IndexDiffs  = IpcSrf -> PointErrs.ErrIndexes;
    CagdRType 
        Delta         = IpcSrf -> PointErrs.Delta, 
        DRange        = IpcSrf -> PointErrs.DRange,
        **QntDiffs    = IpcSrf -> PointErrs.PErrors,
        **Points      = TV -> Points;
    float 
        QntError      = IpcSrf -> Args -> QntError;
    CagdBType 
        IsNotRational = !TRIV_IS_RATIONAL_TV(TV);

    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Set first point. */ 
        Points[i][0] = IpcSrf -> PointErrs.InitialValue[i]; 
 
        /* Find points from differences. */         
        for (v = 1; v < NumPoints; v++) { 
            if (IPC_QUANTIZATION_USED(QntError))  
                QntDiffs[i][v] = IndexDiffs[i][v] * Delta * DRange; 
 
            /* Reconstruct point. */ 
            Points[i][v] = QntDiffs[i][v] + Points[i][v-1]; 
        } 
    } 
     
    /* Round reconstructed points to maximum quantization value. */ 
    IpcLimitPoints(Points, TV -> PType, NumPoints, QntError); 
} 
 
/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform Multi-variate reconstructor.                                      *
*   Reconstruct control points and weights from encoded                       *
*   control points and weights.                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   MV:         Multi-variate to reconstruct into.                            *
*   IpcSrf:     Includes entire information about surfaces.                   *
*               Used in order to reconstruct surfaces effectively.            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsMultiVar(MvarMVStruct *MV, 
                                         IpcSrfStruct  *IpcSrf)
{
    int i, v, 
        MaxCoord     = CAGD_NUM_OF_PT_COORD(MV -> PType),
        **IndexDiffs = IpcSrf -> PointErrs.ErrIndexes, 
        NumPoints    = IpcSrf -> PointErrs.NumPoints; 
    CagdRType 
        Delta        = IpcSrf -> PointErrs.Delta, 
        DRange       = IpcSrf -> PointErrs.DRange, 
        **QntDiffs   = IpcSrf -> PointErrs.PErrors,
        **Points     = MV -> Points; 
    float 
        QntError     = IpcSrf -> Args -> QntError; 
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(MV -> PType);

    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Set first point. */ 
        Points[i][0] = IpcSrf -> PointErrs.InitialValue[i]; 
 
        /* Find points from differences. */         
        for (v = 1; v < NumPoints; v++) { 
            if (IPC_QUANTIZATION_USED(QntError))  
                QntDiffs[i][v] = IndexDiffs[i][v] * Delta * DRange; 
 
            /* Reconstruct point. */ 
            Points[i][v] = QntDiffs[i][v] + Points[i][v-1]; 
        } 
    } 
     
    /* Round reconstructed points to maximum quantization value. */ 
    IpcLimitPoints(Points, MV -> PType, NumPoints, QntError); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform reconstructor.                                                    *
*   Reconstruct control points and weights using unoform prediction.          *
*                                                                             *
* PARAMETERS:                                                                 *
*   Points:       Reconstructed control points.                               *
*   PointErrs:    Includes entire information about encoded points.           *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsUniform(CagdRType *Points[], 
                                        IpcCtlPtStruct *PointErrs, 
                                        IpcArgs *Args) 
{ 
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PointErrs -> PType), 
        NumPoints     = PointErrs -> NumPoints,
        **IndexDiffs  = PointErrs -> ErrIndexes;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PointErrs -> PType);
    CagdRType DRange,
        Delta         = PointErrs -> Delta,
        DRangeInit    = IPC_ROUND10(PointErrs -> DRange),
        **QntDiffs    = PointErrs -> PErrors;
    float 
        QntError      = Args -> QntError; 

    /* Init first point difference. */ 
    for (i = IsNotRational; i <= MaxCoord; i++) { 
        /* Set different range for weights and control points. */ 
        DRange = (i) ? DRangeInit : 1; 
 
        /* Set first point. */ 
        Points[i][0] = PointErrs -> InitialValue[i]; 
 
        /* Find points from differences. */         
        for (v = 0; v < NumPoints - 1; v++) { 
            if (IPC_QUANTIZATION_USED(QntError))  
                QntDiffs[i][v] = IndexDiffs[i][v] * Delta * DRange; 
 
            /* Reconstruct point. */ 
            Points[i][v+1] = QntDiffs[i][v] + Points[i][v]; 
        } 
    } 
     
    /* Round reconstructed points to maximum quantization value. */ 
    IpcLimitPoints(Points, PointErrs -> PType, NumPoints, QntError); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   2D reconstructor.                                                         *
*   Reconstruct control points and weights using unoform prediction.          *
*                                                                             *
* PARAMETERS:                                                                 *
*   Points:       Reconstructed control points.                               *
*   PointErrs:    Includes entire information about encoded points.           *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPoints2D(CagdRType *Points[], 
                                   IpcCtlPtStruct *PointErrs, 
                                   IpcArgs *Args)
{
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PointErrs -> PType),
        NumPoints     = PointErrs -> NumPoints,
        **IndexDiffs  = PointErrs -> ErrIndexes;
    CagdRType DRange,
        Delta         = PointErrs -> Delta, 
        DRangeInit    = IPC_ROUND10(PointErrs -> DRange), 
        **QntDiffs   = PointErrs -> PErrors; 
    float 
        QntError      = Args -> QntError;
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(PointErrs -> PType); 
    IrtHmgnMatType InvMat, TransMat; 
    IrtPtType Pt0, Pt, Pt2D, PrevPt2D, PtErr; 
    IrtVecType Normal; 

    IRIT_VEC_COPY(Normal, PointErrs -> Normal); 
    IRIT_PT_RESET(PrevPt2D); 
 
    /* Set first point. */ 
    for (i = 1; i <= MaxCoord; i++)  
           Pt0[i-1] = PointErrs -> InitialValue[i]; 
 
    /* Set initial point. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
            Points[i][0] = PointErrs -> InitialValue[i]; 
 
    if (!IRIT_PT_EQ_ZERO(Normal)) { 
        /* Create 2D - transformation inverse matrix. */ 
        GMGenTransMatrixZ2Dir(InvMat, Pt0, Normal, 1.0); 
         
        /* Create 2D - transformation matrix. */ 
        if (!MatInverseMatrix(InvMat, TransMat))  
	    _IpcErrHandler(IPC_ERROR_GENERAL); 
 
        /* Translate the base points to 2D space,   */
        /* first point will always be 0.            */ 
        IRIT_PT_RESET(PrevPt2D); 
    }
    else 
        IRIT_PT_COPY(PrevPt2D, Pt0); 
 
    if (!IsNotRational) 
        QntDiffs[0][0] = 0.0; 
 
    /* Find differences of all points.                  */    
    /* Algorithm work: Calculate 2D points and then     */
    /* transfer them to 3D space.                       */ 
    for (v = 1; v < NumPoints; v++) { 
	for (i = IsNotRational; i <= MaxCoord; i++) {   
	    /* Set different range for weights and control points. */ 
	    DRange = (i) ? DRangeInit : 1; 
 
	    /* Get error between predicted point to exist one. */ 
	    if (IPC_QUANTIZATION_USED(QntError))  
	        QntDiffs[i][v] = IndexDiffs[i][v] * Delta * DRange;
	}
 
	/* If weights defined. */ 
	if (!IsNotRational)  
            Points[0][v] = (v >= 2) ? (QntDiffs[0][v] + QntDiffs[0][v-2])
                                    : QntDiffs[0][v]; 

	for (i = 1; i <= MaxCoord; i++)
	    PtErr[i-1] = QntDiffs[i][v]; 
           
	/* Translate the base points to 3D space. */ 
	IRIT_PT_ADD(Pt2D, PtErr, PrevPt2D); 
	IRIT_PT_COPY(PrevPt2D, Pt2D); 
 
	if (!IRIT_PT_EQ_ZERO(Normal)) 
	    MatMultPtby4by4(Pt, Pt2D, InvMat); 
	else  
	    IRIT_PT_COPY(Pt, Pt2D);         
 
	/* Set reconstructed points. */ 
	for (i = 1; i <= MaxCoord; i++)  
	    Points[i][v] = Pt[i-1]; 
    } 
     
    /* Round reconstructed points to maximum quantization value. */
    IpcLimitPoints(Points, PointErrs -> PType, NumPoints, QntError);
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   Arc reconstructor.                                                        *
*   Reconstruct control points and weights using arc prediction.              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Points:       Reconstructed control points.                               *
*   PointErrs:    Includes entire information about encoded points.           *
*   Args:         User defined arguments.                                     *
*                 Specifies different parameters of compression.              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsArc(CagdRType *Points[], 
                                    IpcCtlPtStruct *PointErrs, 
                                    IpcArgs *Args)
{
    int i, v, 
        MaxCoord      = CAGD_NUM_OF_PT_COORD(PointErrs -> PType),
        NumPoints     = PointErrs -> NumPoints,
        **IndexDiffs  = PointErrs -> ErrIndexes;
    CagdBType
        IsNotRational = !CAGD_IS_RATIONAL_PT(PointErrs -> PType);
    CagdRType DRange,
        Delta         = PointErrs -> Delta, 
        DRangeInit    = IPC_ROUND10(PointErrs -> DRange), 
        **QntDiffs    = PointErrs -> PErrors; 
    float 
        QntError      = Args -> QntError; 
 
    /* Set first quantized point. */ 
    for (i = IsNotRational; i <= MaxCoord; i++)  
        Points[i][0] = PointErrs -> InitialValue[i]; 
 
    /* Find points from differences. */         
    for (v = 0; v < NumPoints - 1; v++) { 
        /* Arc predictor needs 3 points in order to predict 4th. */ 
        _IpcPredictPointsArcAux(Points, v+1);         
 
        for (i = IsNotRational; i <= MaxCoord; i++) { 
           /* Set different range for weights and control points. */ 
           DRange = (i) ? DRangeInit : 1; 
 
           if (IPC_QUANTIZATION_USED(QntError))  
	       QntDiffs[i][v] = IndexDiffs[i][v] * Delta * DRange; 
 
           /* First two quantized points reconstructed uniformlly. */ 
           Points[i][v+1] = QntDiffs[i][v] + Points[i][IPC_ARC_INDX(v, i)];
        } 
    } 
     
    /* Round reconstructed points to maximum quantization value. */ 
    IpcLimitPoints(Points, PointErrs -> PType, NumPoints, QntError); 
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   By angles reconstructor.                                                  *
*   Reconstruct control points and weights from encoded                       *
*   control points and weights.                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   Srfs:       BSpline surfaces to reconstruct into.                         *
*   IpcSrfs:    Includes entire information about surfaces.                   *
*               Used in order to reconstruct surfaces effectively.            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsSrfAngles(CagdSrfStruct *Srf, 
                                          IpcSrfStruct  *IpcSrfs)
{
    int Block, i, v,
        NumPoints = Srf -> ULength * Srf -> VLength,
        NumBlocks = Srf -> VLength,
        BlockLen  = Srf -> ULength,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *QPts[CAGD_MAX_PT_SIZE], *Pts[CAGD_MAX_PT_SIZE];
    CagdBType 
        IsNotRational  = !CAGD_IS_RATIONAL_PT(Srf -> PType);
    IpcCtlPtStruct 
        *Errs = &(IpcSrfs -> PointErrs);
    IpcArgs 
        *Args = IpcSrfs -> Args;

    /*Allocate memory for quantized points. */ 
    for (i = 1; i <= MaxCoord; i++) 
        QPts[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * BlockLen);
    
    /* Reconstruct differences for weights. */
    if (!IsNotRational)
        IpcReconstructWeightsUniform(Errs, Srf -> Points[0], Args);

    /* Get angles which were used to predict points. */
    IpcReconstructPointsGetAngles(&IpcSrfs -> PointErrs);

    /* Reconstruct each block. */
    for (Block = 0; Block < NumBlocks; Block++) {
        /* Make index to new block of points. */
        for (i = 1; i <= MaxCoord; i++)
            Pts[i] = &(Srf -> Points[i][Block * BlockLen]);
            
        /* Reconstruct each block. */
        IpcReconstructPointsAnglesAux(Errs, QPts, Pts, Block, BlockLen, Args);
    }
    
    /* Normalize points by weights. */ 
    if (!IsNotRational) {
        for (v = 0; v < Srf -> ULength * Srf -> VLength; v++)
            for (i = 1; i <= MaxCoord; i++)
                Srf -> Points[i][v] *= Srf -> Points[0][v];
    }

    /* Round reconstructed points to maximum quantization value. */
    IpcLimitPoints(Srf -> Points, Errs -> PType, NumPoints, Args -> QntError);

    /* Free allocated memory. */
    for (i = 1; i <= MaxCoord; i++)
        IritFree(QPts[i]);

    IritFree(Errs -> Nrmls);
    IritFree(Errs -> Predicts);
    IritFree(Errs -> NumAngles);
    IritFree(Errs -> Angles);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Update normal to set of control points.                                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   Normal:       Normal to control points                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcUpdateNormal(IrtVecType Normal)
{
    CagdRType X;
    
    if (IRIT_FABS(Normal[0]) >= 1.0) {
        Normal[0] = 1.0 * IRIT_SIGN(Normal[0]);
	Normal[1] = 0.0;
	Normal[2] = 0.0;
    }

    if (IRIT_FABS(Normal[1]) >= 1.0){
        Normal[0] = 0.0;
	Normal[1] = 1.0 * IRIT_SIGN(Normal[1]);
	Normal[2] = 0.0;
    }

    X = IRIT_SQR(Normal[0]) + IRIT_SQR(Normal[1]);
    
    Normal[2] = (X >= 1.0) ? 0.0 : sqrt(1 - X);

    IRIT_VEC_SAFE_NORMALIZE(Normal);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Get specific set of angles from according predictor.                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:       Includes information about encoded points.                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructPointsGetAngles(IpcCtlPtStruct *Errs)
{
    int Block;

    Errs -> Angles = 
        (CagdRType **) IritMalloc(sizeof(CagdRType*) * Errs -> NumBlocks);
    Errs -> NumAngles = 
        (int *) IritMalloc(sizeof(int) * Errs -> NumBlocks);

    for (Block = 0; Block < Errs -> NumBlocks; Block++) {
        switch (Errs -> Predicts[Block]) {
            case IPC_SRF_PREDICTOR_ANGLES_PERFECT1:
            case IPC_SRF_PREDICTOR_ANGLES_REGULAR1:
                Errs -> Angles[Block]    = _IpcAnglesCos[0];
                Errs -> NumAngles[Block] = _IpcAnglesNum[0];
                break;
            case IPC_SRF_PREDICTOR_ANGLES_PERFECT2:
            case IPC_SRF_PREDICTOR_ANGLES_REGULAR2:
                Errs -> Angles[Block]    = _IpcAnglesCos[1];
                Errs -> NumAngles[Block] = _IpcAnglesNum[1];
                break;
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function IpcReconstructPointsAngles    		     *
*****************************************************************************/
static void IpcReconstructPointsAnglesAux(IpcCtlPtStruct *Errs,
                                          CagdRType *QPts[],
                                          CagdRType *Pts[],
                                          int Block,
                                          int BlockLen,
                                          IpcArgs *Args)
{
    int v, i, *ErrsIndexes[CAGD_MAX_PT_SIZE], NextAngle,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Errs -> PType);
    CagdRType *PErrors[CAGD_MAX_PT_SIZE],
        Delta  = Errs -> Delta,
        DRange = Errs -> DRange,
        CosTeta, SinTeta;
    CagdBType
        isAllSame = TRUE;
    IrtHmgnMatType RotMat, Mat;
    IrtPtType QP0, QP1, QP2, Res;
    IrtVecType Trans, Vec;

    /* Create new references in order to simplify code. */
    for (i = 1; i <= MaxCoord; i++) {
        ErrsIndexes[i] = &(Errs -> ErrIndexes[i][Block * BlockLen]);
        PErrors[i]     = &(Errs -> PErrors[i][Block * BlockLen]);
        
        if (IPC_QUANTIZATION_USED(Args -> QntError))
            for (v = 0; v < BlockLen ; v++)
                PErrors[i][v] = ErrsIndexes[i][v] * Delta * DRange;
    }

    if (BlockLen == 2) {
        _IpcPtCopy(Pts, 0, PErrors, 0, MaxCoord);
        _IpcPtAdd(Pts, 1, PErrors, 0, PErrors, 1, MaxCoord);
        
        return;
    }

    _IpcSetToPt(PErrors, 0, Trans, MaxCoord);
    _IpcSetToPt(PErrors, 1,   Vec, MaxCoord);

    IRIT_PT_RESET(QP0);
    IRIT_PT_ADD(QP1, QP0, Vec);

    _IpcSetFromPt(QPts, 0, QP0, MaxCoord);
    _IpcSetFromPt(QPts, 1, QP1, MaxCoord);

    _IpcPtCopy(Pts, 0, QPts, 0, MaxCoord);
    _IpcPtCopy(Pts, 1, QPts, 1, MaxCoord);

    for (NextAngle = 0, v = 2; v < BlockLen-1; v++) {
        CosTeta = Errs -> Angles[Block][NextAngle++ % Errs -> NumAngles[Block]];
        SinTeta = sqrt(1 - IRIT_SQR(CosTeta));
        MatGenMatRotZ(CosTeta, SinTeta, RotMat);

        _IpcSetToPt(QPts, v-2, QP0, MaxCoord);
        _IpcSetToPt(QPts, v-1, QP1, MaxCoord);
        
        IRIT_PT_SUB(Vec, QP1, QP0);

        MatMultVecby4by4(Vec, Vec,  RotMat);

        IRIT_PT_ADD(QP2, QP1, Vec);
        
        _IpcSetToPt(PErrors, v, Res, MaxCoord);

        IRIT_PT_ADD(QP2, QP2, Res);

        _IpcSetFromPt(QPts, v, QP2, MaxCoord);
        _IpcPtCopy(Pts, v, QPts, v, MaxCoord);
    }

    _IpcPtCopy(QPts, v, QPts, 0, MaxCoord);
    _IpcPtAdd(Pts, v, QPts, v, PErrors, v, MaxCoord);
    
    /* Generate new rotatation  matrix. */
    GMGenTransMatrixZ2Dir(Mat, Trans, Errs -> Nrmls[Block], 1.0);

    /* Rotate all points using rotation matrix. */
    CagdMatTransform(Pts, BlockLen, MaxCoord, TRUE, Mat);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Uniform weights reconstructor.                                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   Errs:       Includes information about encoded points.                    *
*   Weights:    Weights to decode.                                            *
*   Args:       User defined arguments.                                       *
*               Specifies different parameters of compression.                *
*                                                                             *
* RETURN VALUE:                                                               *
*   void:                                                                     *
******************************************************************************/
static void IpcReconstructWeightsUniform(IpcCtlPtStruct *Errs, 
                                         CagdRType *Weights, 
                                         IpcArgs *Args)
{
    int v,
        MaxCoord      = CAGD_NUM_OF_PT_COORD(Errs -> PType), 
        NumWeights    = Errs -> NumPoints,
        *QDiffsIndx   = Errs -> ErrIndexes[0];
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_PT(Errs -> PType);
    CagdRType 
        Delta         = Errs -> Delta,
        DRange        = IPC_ROUND10(Errs -> DRange),
        *QDiffs       = Errs -> PErrors[0];
    float 
        QntError      = Args -> QntError; 

    /* Set first and second weight. */
    for (v = 0; v <= 1; v++) {
        if (IPC_QUANTIZATION_USED(QntError))
            Weights[v] = QDiffsIndx[v] * Delta * DRange;
        else
            Weights[v] = QDiffs[v];

        if (v == 1)
            Weights[1] += Weights[0];
    }
     
    /* Find weights from differences. */         
    for (v = 2; v < NumWeights; v++) { 
        if (IPC_QUANTIZATION_USED(QntError))
            QDiffs[v] = QDiffsIndx[v] * Delta * DRange;
 
	/* Reconstruct weight. */ 
	Weights[v] = QDiffs[v] + Weights[v-2]; 
    } 
}

#endif /* IPC_BIN_COMPRESSION */
