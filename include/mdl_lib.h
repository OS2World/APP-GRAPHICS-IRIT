/******************************************************************************
* Mdl_lib.h - header file for the Model library.			      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber and Bogdanov Alexander   July, 1996		      *
******************************************************************************/

#ifndef MDL_LIB_H
#define MDL_LIB_H

#include "cagd_lib.h" 
#include "trim_lib.h" 

typedef enum {
    MDL_BOOL_UNION = 6000,
    MDL_BOOL_INTERSECTION,
    MDL_BOOL_SUBTRACTION,
    MDL_BOOL_CUT,
    MDL_BOOL_INTER_CRVS
} MdlBooleanType;

typedef enum {
    MDL_ERR_NO_ERROR = 0,

    MDL_ERR_PTR_REF = 1000,
    MDL_ERR_TSEG_NO_SRF,
    MDL_ERR_BOOL_MERGE_FAIL,
    MDL_ERR_TSEG_NOT_FOUND,
    MDL_ERR_TSRF_NOT_FOUND,
    MDL_ERR_FP_ERROR,
    MDL_ERR_BOOL_DISJOINT,
    MDL_ERR_BOOL_GET_REF,
    MDL_ERR_BOOL_CLASSIFY_FAIL,
    MDL_ERR_BOOL_UVMATCH_FAIL,

    MDL_ERR_UNDEFINE_ERR
} MdlFatalErrorType;

typedef struct MdlTrimSegStruct {
    struct MdlTrimSegStruct *Pnext;
    struct IPAttributeStruct *Attr;
    struct MdlTrimSrfStruct *SrfFirst;
    struct MdlTrimSrfStruct *SrfSecond;
    CagdCrvStruct *UVCrvFirst;   /* Trim crv segment in srf's param. domain. */
    CagdCrvStruct *UVCrvSecond;  /* Trim crv segment in srf's param. domain. */
    CagdCrvStruct *EucCrv;       /* Trimming curve as an E3 Euclidean curve. */
    IrtBType Tags;
} MdlTrimSegStruct;

typedef struct MdlTrimSegRefStruct {
    struct MdlTrimSegRefStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MdlTrimSegStruct *TrimSeg;
    IrtBType Reversed;
    IrtBType Tags;
} MdlTrimSegRefStruct;

typedef struct MdlLoopStruct {
    struct MdlLoopStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MdlTrimSegRefStruct *SegRefList;
} MdlLoopStruct;

typedef struct MdlTrimSrfStruct {
    struct MdlTrimSrfStruct *Pnext;
    struct IPAttributeStruct *Attr;
    CagdSrfStruct *Srf;                /* Surface trimmed by MdlTrimSegList. */
    MdlLoopStruct *LoopList;
} MdlTrimSrfStruct;

typedef struct MdlModelStruct {
    struct MdlModelStruct *Pnext;
    struct IPAttributeStruct *Attr;
    MdlTrimSrfStruct *TrimSrfList;
    MdlTrimSegStruct *TrimSegList;       /* List of trimming curve segments. */
} MdlModelStruct;

typedef void (*MdlSetErrorFuncType)(MdlFatalErrorType);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void MdlTrimSegFree(MdlTrimSegStruct *MTSeg);
void MdlTrimSegFreeList(MdlTrimSegStruct *MTSegList);
void MdlTrimSegRefFree(MdlTrimSegRefStruct *MTSegRef);
void MdlLoopFree(MdlLoopStruct *MdlLoop);
void MdlLoopFreeList(MdlLoopStruct *MdlLoopList);
void MdlTrimSrfFree(MdlTrimSrfStruct *TrimSrf);
void MdlTrimSrfFreeList(MdlTrimSrfStruct *MdlTrimSrfList);
void MdlModelFree(MdlModelStruct *Model);
void MdlModelFreeList(MdlModelStruct *Model);


#ifdef DEBUG
#define MdlTrimSegFree(MTSeg)         { MdlTrimSegFree(MTSeg); MTSeg = NULL; }
#define MdlTrimSegFreeList(MTSegList) { MdlTrimSegFreeList(MTSegList); \
					MTSegList = NULL; }
#define MdlTrimSegRefFree(MTSegRef)   { MdlTrimSegRefFree(MTSegRef); \
					MTSegRef = NULL; }
#define MdlLoopFree(MdlLoop)          { MdlLoopFree(MdlLoop); MdlLoop = NULL; }
#define MdlTrimSrfFree(TrimSrf)       { MdlTrimSrfFree(TrimSrf); \
					TrimSrf = NULL; }
#define MdlTrimSrfFreeList(MdlTrimSrfList) { MdlTrimSrfFreeList(MdlTrimSrfList); \
					     MdlTrimSrfList = NULL; }
#define MdlModelFree(Model)           { MdlModelFree(Model); Model = NULL; }
#define MdlModelFreeList(Model)       { MdlModelFreeList(Model); Model = NULL; }
#endif /* DEBUG */

MdlTrimSegStruct *MdlTrimSegCopy(const MdlTrimSegStruct *MdlTrimSeg,
				 const MdlTrimSrfStruct *TrimSrfList);
MdlTrimSegStruct *MdlTrimSegCopyList(const MdlTrimSegStruct *MdlTrimSegList,
				     const MdlTrimSrfStruct *TrimSrfList);
MdlTrimSegRefStruct *MdlTrimSegRefCopy(const MdlTrimSegRefStruct *SegRefList,
				       const MdlTrimSegStruct *TrimSegList);
MdlTrimSegRefStruct *MdlTrimSegRefCopyList(const MdlTrimSegRefStruct *SegRefList,
					   const MdlTrimSegStruct *TrimSegList);
MdlLoopStruct *MdlLoopCopy(const MdlLoopStruct *MdlLoop, 
			   const MdlTrimSegStruct *TrimSegList);
MdlLoopStruct *MdlLoopCopyList(const MdlLoopStruct *MdlLoopList, 
			       const MdlTrimSegStruct *TrimSegList);
MdlTrimSrfStruct *MdlTrimSrfCopy(const MdlTrimSrfStruct *MdlTrimSrf, 
				 const MdlTrimSegStruct *TrimSegList);
MdlTrimSrfStruct *MdlTrimSrfCopyList(const MdlTrimSrfStruct *MdlTrimSrfList, 
				     const MdlTrimSegStruct *TrimSegList);
MdlModelStruct *MdlModelCopy(const MdlModelStruct *Model);
MdlModelStruct *MdlModelCopyList(const MdlModelStruct *ModelList);
MdlTrimSegStruct *MdlTrimSegNew(CagdCrvStruct *UVCrv1,
				CagdCrvStruct *UVCrv2,
                                CagdCrvStruct *EucCrv1,
                                MdlTrimSrfStruct *SrfFirst,
                                MdlTrimSrfStruct *SrfSecond);
MdlTrimSegRefStruct *MdlTrimSegRefNew(MdlTrimSegStruct *MdlTrimSeg);
MdlLoopStruct *MdlLoopNew(MdlTrimSegRefStruct *MdlTrimSegRefList);
MdlTrimSrfStruct *MdlTrimSrfNew(CagdSrfStruct *Srf,
				MdlLoopStruct *LoopList, 
				CagdBType HasTopLvlTrim,
				CagdBType UpdateBackTSrfPtrs);
MdlTrimSrfStruct *MdlTrimSrfNew2(CagdSrfStruct *Srf,
	 		         CagdCrvStruct *LoopList, 
			         CagdBType HasTopLvlTrim);
MdlModelStruct *MdlModelNew(CagdSrfStruct *Srf,
		            CagdCrvStruct *LoopList,
		            CagdBType HasTopLvlTrim);
MdlModelStruct *MdlModelNew2(MdlTrimSrfStruct *TrimSrfs,
			     MdlTrimSegStruct *TrimSegs);
void MdlPatchTrimmingSegPointers(MdlModelStruct *Model);
IritIntPtrSizeType MdlGetLoopSegIndex(const MdlTrimSegRefStruct *TrimSeg,
				      const MdlTrimSegStruct *TrimSegList);
IritIntPtrSizeType MdlGetSrfIndex(const MdlTrimSrfStruct *Srf,
				  const MdlTrimSrfStruct *TrimSrfList);

void MdlModelTransform(MdlModelStruct *Model,
		       const CagdRType *Translate,
		       CagdRType Scale);
void MdlModelMatTransform(MdlModelStruct *Model, CagdMType Mat);

/******************************************************************************
* Bounding boxes routines.						      *
******************************************************************************/

void MdlModelBBox(const MdlModelStruct *Mdl, CagdBBoxStruct *BBox);
void MdlModelListBBox(const MdlModelStruct *Mdls, CagdBBoxStruct *BBox);
void MdlModelTSrfTCrvsBBox(const MdlTrimSrfStruct *TSrf, CagdBBoxStruct *BBox);

/******************************************************************************
* Primitives routines.							      *
******************************************************************************/

int MdlTwoTrimSegsSameEndPts(MdlTrimSegStruct *TSeg1,
			     MdlTrimSegStruct *TSeg2,
			     CagdRType Tol);
MdlTrimSegRefStruct *MdlGetSrfTrimSegRef(MdlTrimSrfStruct *TSrf,
					 MdlTrimSegStruct *TSeg);
int MdlStitchModel(MdlModelStruct *Mdl, CagdRType StitchTol);
MdlModelStruct *MdlPrimPlane(CagdRType MinX,
			     CagdRType MinY,
			     CagdRType MaxX,
			     CagdRType MaxY,
			     CagdRType ZLevel);
MdlModelStruct *MdlPrimPlaneSrfOrderLen(CagdRType MinX,
					CagdRType MinY,
					CagdRType MaxX,
					CagdRType MaxY,
					CagdRType ZLevel,
					int Order,
					int Len);
MdlModelStruct *MdlPrimBox(CagdRType MinX,
			   CagdRType MinY,
			   CagdRType MinZ,
			   CagdRType MaxX,
			   CagdRType MaxY,
			   CagdRType MaxZ);
MdlModelStruct *MdlPrimSphere(const CagdVType Center,
			      CagdRType Radius,
			      CagdBType Rational);
MdlModelStruct *MdlPrimTorus(const CagdVType Center,
			     CagdRType MajorRadius,
			     CagdRType MinorRadius,
			     CagdBType Rational);
MdlModelStruct *MdlPrimCone2(const CagdVType Center,
			     CagdRType MajorRadius,
			     CagdRType MinorRadius,
			     CagdRType Height,
			     CagdBType Rational,
			     CagdPrimCapsType Caps);
MdlModelStruct *MdlPrimCone(const CagdVType Center,
			    CagdRType Radius,
			    CagdRType Height,
			    CagdBType Rational,
			    CagdPrimCapsType Caps);
MdlModelStruct *MdlPrimCylinder(const CagdVType Center,
				CagdRType Radius,
				CagdRType Height,
				CagdBType Rational,
				CagdPrimCapsType Caps);
int MdlStitchSelfSrfPrims(int Stitch);
int MdlCreateCubeSpherePrim(int CubeTopoSphere);

/******************************************************************************
* Boolean operations.							      *
******************************************************************************/

CagdCrvStruct *MdlExtructReversUVCrv(const MdlTrimSrfStruct *MdlSrf, 
				     const MdlTrimSegStruct *MdlSeg);

void MdlBooleanSetTolerances(CagdRType SubdivTol,
			     CagdRType NumerTol,
			     CagdRType TraceTol);

struct IPObjectStruct *MdlBooleanUnion(const MdlModelStruct *Model1, 
				       const MdlModelStruct *Model2);
struct IPObjectStruct *MdlBooleanIntersection(const MdlModelStruct *Model1, 
					      const MdlModelStruct *Model2);
struct IPObjectStruct *MdlBooleanSubtraction(const MdlModelStruct *Model1, 
					     const MdlModelStruct *Model2);
struct IPObjectStruct *MdlBooleanCut(const MdlModelStruct *Model1,
				     const MdlModelStruct *Model2);
struct IPObjectStruct *MdlBooleanMerge(const MdlModelStruct *Model1,
				       const MdlModelStruct *Model2,
				       CagdBType StitchBndries);
CagdCrvStruct *MdlBooleanInterCrv(const MdlModelStruct *Model1,
				  const MdlModelStruct *Model2,
				  int InterType);
int MdlBoolSetOutputInterCrv(int OutputInterCurve);
int MdlBoolSetOutputInterCrvType(int OutputInterCurveType);
MdlModelStruct *MdlModelNegate(const MdlModelStruct *Model);

int MdlBoolCleanUnusedTrimCrvsSrfs(MdlModelStruct *Model);
void MdlBoolClipTSrfs2TrimDomain(MdlModelStruct *Model);

/******************************************************************************
* Model maintenance routines.						      *
******************************************************************************/

int MdlSplitTrimCrv(MdlTrimSegStruct *Seg,
		    const CagdPtStruct *Pts,
		    int Idx,
		    CagdRType Eps,
		    int *Proximity);
MdlTrimSegStruct *MdlDivideTrimCrv(MdlTrimSegStruct *Seg,
				   const CagdPtStruct *Pts,
				   int Idx,
				   CagdRType Eps,
				   int *Proximity);
MdlTrimSegStruct *MdlFilterOutCrvs(MdlTrimSegStruct *TSegs,
				   const MdlTrimSrfStruct *TSrf);
CagdBType MdlIsPointInsideTrimSrf(const MdlTrimSrfStruct *TSrf,
				  CagdUVType UV);
void MdlEnsureMdlTrimCrvsPrecision(MdlModelStruct *Mdl);
void MdlEnsureTSrfTrimCrvsPrecision(MdlTrimSrfStruct *MdlTrimSrf);

/******************************************************************************
* Conversion routines.							      *
******************************************************************************/

TrimSrfStruct *MdlCnvrtMdl2TrimmedSrfs(const MdlModelStruct *Model);
MdlModelStruct *MdlCnvrtSrf2Mdl(const CagdSrfStruct *Srf);
MdlModelStruct *MdlCnvrtTrimmedSrf2Mdl(const TrimSrfStruct *TSrf);
CagdCrvStruct *MdlExtractUVCrv(const MdlTrimSrfStruct *Srf,
			       const MdlTrimSegStruct *Seg);

/******************************************************************************
* Error handling.							      *
******************************************************************************/

MdlSetErrorFuncType MdlSetFatalErrorFunc(MdlSetErrorFuncType ErrorFunc);
void MdlFatalError(MdlFatalErrorType ErrID);
const char *MdlDescribeError(MdlFatalErrorType ErrID);

void MdlDbg(void *Obj);

#ifdef DEBUG
int MdlDbgMC(const MdlModelStruct *Mdl, int Format);
int MdlDbgSC(const MdlTrimSrfStruct *TSrf, int Format);
int MdlDbgRC(const MdlTrimSegRefStruct *Refs, int Format);
int MdlDbgRC2(const MdlTrimSegRefStruct *Refs,
	      const MdlTrimSrfStruct *TSrf,
	      int Format);

int MdlDebugHandleTCrvLoops(const MdlTrimSrfStruct *TSrf,
			    const MdlLoopStruct *Loops,
			    const CagdPType Trans,
			    int Display,
			    int TrimEndPts);
int MdlDebugHandleTSrfCrvs(const MdlTrimSegStruct *TCrvs,
			   const MdlTrimSrfStruct *TSrf,
			   const CagdPType Trans,
			   int Display,
			   int TrimEndPts);
int MdlDebugHandleTSrfRefCrvs(const MdlTrimSegRefStruct *Refs,
			      const MdlTrimSrfStruct *TSrf,
			      const CagdPType Trans,
			      int Loop,
			      int Display,
			      int TrimEndPts);
int MdlDebugWriteTrimSegs(const MdlTrimSegStruct *TSegs,
			  const MdlTrimSrfStruct *TSrf,
			  const CagdPType Trans);
  int MdlDebugVerify(const MdlModelStruct *Model, int TestLoops);
#endif /* DEBUG */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* MDL_LIB_H */
