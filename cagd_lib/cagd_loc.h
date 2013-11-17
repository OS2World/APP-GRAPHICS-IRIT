/******************************************************************************
* Cagd_loc.h - header file for the CAGD library.			      *
* This header is local to the library - it is NOT external.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#ifndef CAGD_LOC_H
#define CAGD_LOC_H

#include <math.h>

#define W 0	 /* Positions of points in Points array (see structs below). */
#define X 1
#define Y 2
#define Z 3

#define CAGD_MAX_FINENESS		10000

#define CAGD_CACHE_CRV_MAX_LEN		100
#define CAGD_CACHE_SRF_MAX_LEN		30

#define CAGD2PLY_MAX_SUBDIV_INDEX	16384/* Inc. as 2^n for fine approx. */

#include "iritprsr.h"
#include "cagd_lib.h"		     /* Include the extrenal header as well. */

typedef struct CagdSrfEvalCacheStruct {
    CagdCrvStruct *IsoSubCrv;
    IrtRType *VBasisFunc, v;
    int UIndexFirst, VIndexFirst;
} CagdSrfEvalCacheStruct;

/* Declaration of extrenal variables local to the cagd library only. */
IRIT_GLOBAL_DATA_HEADER CagdLin2PolyType _CagdLin2Poly;/* Lin srf conv. to polys. */
IRIT_GLOBAL_DATA_HEADER int _CagdSrf2PolygonStrips;  /* Should build poly strips? */
IRIT_GLOBAL_DATA_HEADER int _CagdSrf2PolygonFast;		   /* Fast/approx.?. */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Declarations of functions local to the Cagd library only. */
struct CagdA2PGridStruct *CagdSrfA2PGridInit(const CagdSrfStruct *Srf);
void CagdSrfA2PGridFree(struct CagdA2PGridStruct *A2PGrid);
void CagdSrfA2PGridInsertUV(struct CagdA2PGridStruct *A2PGrid,
			    int UIndex,
			    int VIndex,
			    CagdRType u,
			    CagdRType v);
void CagdSrfA2PGridProcessUV(struct CagdA2PGridStruct *A2PGrid);
CagdSrfPtStruct *CagdSrfA2PGridFetchRect(struct CagdA2PGridStruct *A2PGrid,
					 int UIndex1,
					 int VIndex1,
					 int UIndex2,
					 int VIndex2);
CagdSrfPtStruct *CagdSrfA2PGridFetchPts(struct CagdA2PGridStruct *A2PGrid,
					CagdSrfDirType Dir,
					int StartIndex,
					int EndIndex,
					int OtherDirIndex,
					CagdSrfPtStruct **LastPt,
					CagdBType Reversed);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* CAGD_LOC_H */
