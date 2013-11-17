/*****************************************************************************
* Definitions for the IHidden program:                                      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef IHIDDEN_H
#define IHIDDEN_H

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "grap_lib.h"
#include "attribut.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "symb_lib.h"

#define IHID_DEF_IHID_TOLERANCE		1e-3 /* Tolerance of CCI operations. */
#define IHID_DEF_SCRN_RSI_FINENESS    1000 /* Scrn Zbuf fineness in RSI ops. */
#define IHID_DEF_HIDDEN_WIDTH		1e-3 /* Line width of hidden curves. */
#define IHID_BNDRY_EPS		1e-5	  /* Move inside from real boundary. */
#define IHID_TRIM_EDGES_TOLERANCE	128 /* Tolerance of trimming curves. */
 
#define IHID_MARK_CTYPE(Crv, PObjOrig, CType)  \
		      AttrSetIntAttrib(&(Crv) -> Attr, "ctype", (CType)); \
		      AttrSetPtrAttrib(&(Crv) -> Attr, "_OrigObj", (PObjOrig));

typedef enum {
    IHID_STAGE_CRV_EXTRACT = 1,
    IHID_STAGE_CRV_CRV_INTER,
    IHID_STAGE_RAY_SRF_INTER
} IHidStageType;

typedef enum {
    IHID_CURVE_INDEPNDNT,
    IHID_CURVE_BOUNDARY,
    IHID_CURVE_ISOPARAM,
    IHID_CURVE_SILHOUETTE,
    IHID_CURVE_DISCONT
} IHidCrvType;

/* The following are global setable variables (via config file ihidden.cfg) */
IRIT_GLOBAL_DATA_HEADER int
    GlblMore,
    GlblQuiet,
    GlblOutputInvisible,
    GlblMonotoneCrvs,
    GlblNumIsoCurves[3],
    GlblBackFacing,
    GlblStopStage,
    GlblScrnRSIFineness;

IRIT_GLOBAL_DATA_HEADER IrtRType
    GlblIHidTolerance;

/* Global transformation matrix: */
IRIT_GLOBAL_DATA_HEADER IrtHmgnMatType
    GlblViewMat;			   /* Current view/persp. of object. */

/* Data structures used by the hidden curves modules: */

/* The prototypes of the ihidden.c module: */
void IHiddenExit(int ExitCode);

/* The prototypes of the rsi.c module: */
void RaySrfIntersections(IPObjectStruct *OrigGeom, CagdCrvStruct *Crvs);

/* The prototypes of the cci.c module: */
CagdCrvStruct *CrvCrvIntersections(CagdCrvStruct *Crvs, CagdRType CCITol);
int CCIUpdateSubdivCrvs(CagdCrvStruct *Crv,
			CagdRType t,
			CagdCrvStruct **Crv1,
			CagdCrvStruct **Crv2);

#endif /* IHIDDEN_H */
