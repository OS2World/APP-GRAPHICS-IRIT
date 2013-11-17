/******************************************************************************
* Trim_lib.h - header file for the TRIMmed surfaces library.		      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 94.					      *
******************************************************************************/

#ifndef TRIM_LIB_H
#define TRIM_LIB_H

#include <stdio.h>
#include "irit_sm.h"
#include "miscattr.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"

typedef enum {
    TRIM_ERR_TRIM_CRV_E2 = 2000,
    TRIM_ERR_BSPLINE_EXPECT,
    TRIM_ERR_BZR_BSP_EXPECT,
    TRIM_ERR_DIR_NOT_CONST_UV,
    TRIM_ERR_ODD_NUM_OF_INTER,
    TRIM_ERR_TCRV_ORIENT,
    TRIM_ERR_INCONSISTENT_CNTRS,
    TRIM_ERR_FAIL_MERGE_TRIM_SEG,
    TRIM_ERR_INVALID_TRIM_SEG,
    TRIM_ERR_INCON_PLGN_CELL,
    TRIM_ERR_TRIM_TOO_COMPLEX,
    TRIM_ERR_TRIMS_NOT_LOOPS,
    TRIM_ERR_LINEAR_TRIM_EXPECT,
    TRIM_ERR_NO_INTERSECTION,
    TRIM_ERR_POWER_NO_SUPPORT,
    TRIM_ERR_UNDEF_SRF,
    TRIM_ERR_TRIM_OPEN_LOOP,

    TRIM_ERR_UNDEFINE_ERR
} TrimFatalErrorType;

/******************************************************************************
* A trimmed surface can have trimming curves that either form a closed loop   *
* or start and end on the boundary of the surface. A trimming curve will be   *
* defined using a list of TrimCrvSegStruct, creating a closed loop or a       *
* curve that starts and ends in the boundary of the surface.		      *
*   Orientation of TrimCrvSegStruct should be such that the trimming curve    *
* tangent direction crossed with the surface normal points into the inside.   *
*   EucCrv can be either NULL where the Euclidean location must the be        *
* computed on the fly from parametric information or, if exist, must be       *
* used to prevent from black holes with adjacent surfaces.		      *
*   The trimming curves have no order what so ever.			      *
*   An outmost loop will always be present even if the entire four boundary   *
* curves are untrimmed.						 	      *
******************************************************************************/
typedef struct TrimCrvSegStruct {
    struct TrimCrvSegStruct *Pnext;
    IPAttributeStruct *Attr;
    CagdCrvStruct *UVCrv;    /* Trimming crv segment in srf's param. domain. */
    CagdCrvStruct *EucCrv;       /* Trimming curve as an E3 Euclidean curve. */
} TrimCrvSegStruct;

typedef struct TrimCrvStruct {
    struct TrimCrvStruct *Pnext;
    IPAttributeStruct *Attr;
    TrimCrvSegStruct *TrimCrvSegList;    /* List of trimming curve segments. */
} TrimCrvStruct;

typedef struct TrimSrfStruct {
    struct TrimSrfStruct *Pnext;
    IPAttributeStruct *Attr;
    int Tags;
    CagdSrfStruct *Srf;			  /* Surface trimmed by TrimCrvList. */
    TrimCrvStruct *TrimCrvList;		         /* List of trimming curves. */
} TrimSrfStruct;

/* Subdivision of trimmed surfaces may result in only one surface returned   */
/* as the other is completely trimmed away. This macros should be used to    */
/* define and identify the two parts.					     */
#define TRIM_IS_FIRST_SRF(Srf)		(((Srf) -> Tags & 0x01) == 0)
#define TRIM_IS_SECOND_SRF(Srf)		(((Srf) -> Tags & 0x01) == 1)
#define TRIM_SET_FIRST_SRF(Srf)		((Srf) -> Tags &= ~0x01)
#define TRIM_SET_SECOND_SRF(Srf)	((Srf) -> Tags |= 0x01)


typedef struct TrimIsoInterStruct {      /* Holds intersections of iso curve */
    struct TrimIsoInterStruct *Pnext;		    /* with trimming curves. */
    CagdRType Param;
} TrimIsoInterStruct;

typedef void (*TrimSetErrorFuncType)(TrimFatalErrorType);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

TrimCrvSegStruct *TrimCrvSegNew(CagdCrvStruct *UVCrv, CagdCrvStruct *EucCrv);
TrimCrvSegStruct *TrimCrvSegCopy(TrimCrvSegStruct *TrimCrvSeg);
TrimCrvSegStruct *TrimCrvSegCopyList(TrimCrvSegStruct *TrimCrvSegList);
void TrimCrvSegFree(TrimCrvSegStruct *TrimCrvSeg);
void TrimCrvSegFreeList(TrimCrvSegStruct *TrimCrvSegList);

#ifdef DEBUG
#define TrimCrvSegFree(TrimCrvSeg)         { TrimCrvSegFree(TrimCrvSeg); \
					     TrimCrvSeg = NULL; }
#define TrimCrvSegFreeList(TrimCrvSegList) { TrimCrvSegFreeList(TrimCrvSegList); \
					     TrimCrvSegList = NULL; }
#endif /* DEBUG */

TrimCrvStruct *TrimCrvNew(TrimCrvSegStruct *TrimCrvSegList);
TrimCrvStruct *TrimCrvCopy(TrimCrvStruct *TrimCrv);
TrimCrvStruct *TrimCrvCopyList(TrimCrvStruct *TrimCrvList);
void TrimCrvFree(TrimCrvStruct *TrimCrv);
void TrimCrvFreeList(TrimCrvStruct *TrimCrvList);

#ifdef DEBUG
#define TrimCrvFree(TrimCrv)         { TrimCrvFree(TrimCrv); TrimCrv = NULL; }
#define TrimCrvFreeList(TrimCrvList) { TrimCrvFreeList(TrimCrvList); \
				       TrimCrvList = NULL; }
#endif /* DEBUG */

TrimSrfStruct *TrimSrfNew(CagdSrfStruct *Srf,
			  TrimCrvStruct *TrimCrvList,
			  CagdBType HasTopLvlTrim);
TrimSrfStruct *TrimSrfNew2(CagdSrfStruct *Srf,
			   CagdCrvStruct *TrimCrvList,
			   CagdBType HasTopLvlTrim);
int TrimSrfVerifyTrimCrvsValidity(TrimSrfStruct *TrimSrf);
TrimSrfStruct *TrimSrfCopy(const TrimSrfStruct *TrimSrf);
TrimSrfStruct *TrimSrfCopyList(const TrimSrfStruct *TrimSrfList);
void TrimSrfFree(TrimSrfStruct *TrimSrf);
void TrimSrfFreeList(TrimSrfStruct *TrimSrfList);

#ifdef DEBUG
#define TrimSrfFree(TrimSrf)         { TrimSrfFree(TrimSrf); TrimSrf = NULL; }
#define TrimSrfFreeList(TrimSrfList) { TrimSrfFreeList(TrimSrfList); \
				       TrimSrfList = NULL; }
#endif /* DEBUG */

void TrimSrfTransform(TrimSrfStruct *TrimSrf,
		      CagdRType *Translate,
		      CagdRType Scale);
void TrimSrfMatTransform(TrimSrfStruct *TrimSrf, CagdMType Mat);
CagdBType TrimSrfsSame(const TrimSrfStruct *TSrf1,
		       const TrimSrfStruct *TSrf2,
		       CagdRType Eps);
CagdCrvStruct *TrimGetTrimmingCurves(const TrimSrfStruct *TrimSrf,
				     CagdBType ParamSpace,
				     CagdBType EvalEuclid);
CagdCrvStruct *TrimGetTrimmingCurves2(const TrimCrvStruct *TrimCrvList,
				      const TrimSrfStruct *TrimSrf,
				      CagdBType ParamSpace,
				      CagdBType EvalEuclid);
TrimSrfStruct *TrimPiecewiseLinearTrimmingCurves(TrimSrfStruct *TrimSrf,
						 CagdBType EvalEuclid);
TrimCrvStruct *TrimChainTrimmingCurves2Loops(TrimCrvStruct *TrimCrvs);
CagdCrvStruct *TrimChainTrimmingCurves2Loops2(CagdCrvStruct *UVCrvs,
					      CagdRType Tol);
TrimCrvStruct *TrimHealTrimmingCurves(TrimCrvStruct *TrimCrvs);
void TrimAffineTransTrimCurves(TrimCrvStruct *TrimCrvList,
			       CagdRType OldUMin,
			       CagdRType OldUMax,
			       CagdRType OldVMin,
			       CagdRType OldVMax,
			       CagdRType NewUMin,
			       CagdRType NewUMax,
			       CagdRType NewVMin,
			       CagdRType NewVMax);
TrimSrfStruct *TrimAffineTransTrimSrf(const TrimSrfStruct *TrimSrf,
				      CagdRType NewUMin,
				      CagdRType NewUMax,
				      CagdRType NewVMin,
				      CagdRType NewVMax);
CagdPolylineStruct *TrimCrvs2Polylines(TrimSrfStruct *TrimSrf,
				       CagdBType ParamSpace,
				       CagdRType TolSamples,
				       SymbCrvApproxMethodType Method);
CagdPolylineStruct *TrimCrv2Polyline(const CagdCrvStruct *TrimCrv,
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method,
				     CagdBType OptiLin);
CagdCrvStruct *TrimEvalTrimCrvToEuclid(const TrimSrfStruct *TrimSrf,
				       const CagdCrvStruct *UVCrv);
CagdCrvStruct *TrimEvalTrimCrvToEuclid2(const CagdSrfStruct *Srf,
					const CagdCrvStruct *UVCrv);
int TrimSetEuclidComposedFromUV(int EuclidComposedFromUV);
CagdRType *TrimPointInsideTrimmedCrvs(TrimCrvStruct *TrimCrvList,
				      const TrimSrfStruct *TSrf);
CagdBType TrimSrfTrimCrvSquareDomain(const TrimCrvStruct *TrimCrvList,
				     CagdRType *UMin,
				     CagdRType *UMax,
				     CagdRType *VMin,
				     CagdRType *VMax);
CagdBType TrimSrfTrimCrvAllDomain(const TrimSrfStruct *TrimSrf);
TrimSrfStruct *TrimClipSrfToTrimCrvs(TrimSrfStruct *TrimSrf);
TrimSrfStruct *TrimSrfDegreeRaise(const TrimSrfStruct *TrimSrf,
				  CagdSrfDirType Dir);
TrimSrfStruct *TrimSrfSubdivAtParam(TrimSrfStruct *TrimSrf,
				    CagdRType t,
				    CagdSrfDirType Dir);
int TrimSrfSubdivTrimmingCrvs(TrimCrvStruct *TrimCrvs,
			      CagdRType t,
			      CagdSrfDirType Dir,
			      TrimCrvStruct **TrimCrvs1,
			      TrimCrvStruct **TrimCrvs2);
TrimSrfStruct *TrimSrfRegionFromTrimSrf(TrimSrfStruct *TrimSrf,
					CagdRType t1,
					CagdRType t2,
					CagdSrfDirType Dir);
TrimSrfStruct *TrimSrfRefineAtParams(const TrimSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdBType Replace,
				     CagdRType *t,
				     int n);
TrimSrfStruct *TrimSrfReverse(const TrimSrfStruct *TrimSrf);
TrimSrfStruct *TrimSrfReverse2(const TrimSrfStruct *TrimSrf);

int TrimRemoveCrvSegTrimCrvs(TrimCrvSegStruct *TrimCrvSeg,
			     TrimCrvStruct **TrimCrvs);
int TrimRemoveCrvSegTrimCrvSegs(TrimCrvSegStruct *TrimCrvSeg,
				TrimCrvSegStruct **TrimCrvSegs);

void TrimSrfDomain(const TrimSrfStruct *TrimSrf,
		   CagdRType *UMin,
		   CagdRType *UMax,
		   CagdRType *VMin,
		   CagdRType *VMax);
#define TrimSrfSetDomain TrimAffineTransTrimSrf
CagdRType *TrimSrfEval(const TrimSrfStruct *TrimSrf, CagdRType u, CagdRType v);

CagdCrvStruct *TrimSrf2Curves(TrimSrfStruct *TrimSrf, 
			      int NumOfIsocurves[2]);
CagdCrvStruct *TrimCrvTrimParamList(CagdCrvStruct *Crv,
				    TrimIsoInterStruct *InterList);
TrimIsoInterStruct **TrimIntersectTrimCrvIsoVals(const TrimSrfStruct *TrimSrf,
						 int Dir,
						 CagdRType *OrigIsoParams,
						 int NumOfIsocurves,
						 CagdBType Perturb);
TrimIsoInterStruct **TrimIntersectCrvsIsoVals(const CagdCrvStruct *UVCrvs,
					      int Dir,
					      CagdRType *IsoParams,
					      int NumOfIsocurves);
CagdCrvStruct *TrimCrvAgainstTrimCrvs(CagdCrvStruct *UVCrv,
				      const TrimSrfStruct *TrimSrf,
				      CagdRType Eps);
CagdPolylineStruct *TrimSrf2Polylines(TrimSrfStruct *TrimSrf,
				      int NumOfIsocurves[2],
				      CagdRType TolSamples,
				      SymbCrvApproxMethodType Method);
CagdPolygonStruct *TrimSrfAdap2Polygons(const TrimSrfStruct *TrimSrf,
					CagdRType Tolerance,
					CagdBType ComputeNormals,
					CagdBType ComputeUV);
CagdPolylineStruct *TrimCrvsHierarchy2Polys(TrimCrvStruct *TrimLoops);
int TrimClassifyTrimmingLoops(TrimCrvStruct **TrimLoops);
CagdBType TrimClassifyTrimCurveOrient(const CagdCrvStruct *UVCrv);
CagdPolygonStruct *TrimSrf2Polygons2(const TrimSrfStruct *Srf,
				     int FineNess, 
				     CagdBType ComputeNormals,
				     CagdBType ComputeUV);
int TrimSetNumTrimVrtcsInCell(int NumTrimVrtcsInCell);
SymbCrvApproxMethodType TrimSetTrimCrvLinearApprox(CagdRType UVTolSamples,
					   SymbCrvApproxMethodType UVMethod);
CagdRType TrimGetTrimCrvLinearApprox(void);

TrimSrfStruct *TrimSrfsFromContours(const CagdSrfStruct *Srf,
				    const struct IPPolygonStruct *Cntrs);
TrimSrfStruct *TrimSrfsFromTrimPlsHierarchy(struct IPPolygonStruct *TopLevel,
					    struct IPPolygonStruct *TrimPls,
					    const CagdSrfStruct *Srf);
TrimCrvStruct *TrimPolylines2LinTrimCrvs(const struct IPPolygonStruct *Polys);

CagdBType TrimIsPointInsideTrimSrf(const TrimSrfStruct *TrimSrf,
				   CagdUVType UV);
CagdBType TrimIsPointInsideTrimCrvs(const TrimCrvStruct *TrimCrvs,
				    CagdUVType UV);
int TrimIsPointInsideTrimUVCrv(const CagdCrvStruct *UVCrv,
			       CagdUVType UV);

TrimSetErrorFuncType TrimSetFatalErrorFunc(TrimSetErrorFuncType ErrorFunc);
const char *TrimDescribeError(TrimFatalErrorType ErrorNum);
void TrimFatalError(TrimFatalErrorType ErrID);

void TrimDbg(const void *Obj);
void TrimDbgPrintTrimCurves(const TrimCrvStruct *TrimCrv);

/******************************************************************************
* Routines to handle layout (prisa) of trimmed surfaces.		      *
******************************************************************************/
TrimSrfStruct *TrimAllPrisaSrfs(const TrimSrfStruct *TSrfs,
				int SamplesPerCurve,
			        CagdRType Epsilon,
				CagdSrfDirType Dir,
			        CagdVType Space);
TrimSrfStruct *TrimPiecewiseRuledSrfApprox(const TrimSrfStruct *TSrf,
					   CagdBType ConsistentDir,
					   CagdRType Epsilon,
					   CagdSrfDirType Dir);
TrimSrfStruct *TrimPrisaRuledSrf(const TrimSrfStruct *TSrf,
				 int SamplesPerCurve,
				 CagdRType Space,
				 CagdVType Offset,
				 CagdSrfDirType Dir);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* TRIM_LIB_H */
