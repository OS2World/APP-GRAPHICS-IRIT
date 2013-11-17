/******************************************************************************
* Mdl_loc.h - header file for the Model library.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 96.					      *
******************************************************************************/

#ifndef MDL_LOC_H
#define MDL_LOC_H

#include "irit_sm.h"
#include "iritprsr.h"
#include "mdl_lib.h"

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call MdlFatalError, but you may want to reroute this   *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/

#define MDL_BOOL_NEAR_UV_EPS	1e-3

#define MDL_FATAL_ERROR(Msg)	MdlFatalError(Msg)

#define MDL_TSEG_NEW_TAG	0x01
#define MDL_TSEG_USED_TAG	0x02

#define	MDL_IS_TSEG_NEW(Seg)	((Seg) -> Tags & MDL_TSEG_NEW_TAG)
#define	MDL_SET_TSEG_NEW(Seg)	((Seg) -> Tags |= MDL_TSEG_NEW_TAG)
#define	MDL_RST_TSEG_NEW(Seg)	((Seg) -> Tags &= ~MDL_TSEG_NEW_TAG)

#define	MDL_IS_TSEG_USED(Seg)	((Seg) -> Tags & MDL_TSEG_USED_TAG)
#define	MDL_SET_TSEG_USED(Seg)	((Seg) -> Tags |= MDL_TSEG_USED_TAG)
#define	MDL_RST_TSEG_USED(Seg)	((Seg) -> Tags &= ~MDL_TSEG_USED_TAG)

IRIT_GLOBAL_DATA_HEADER jmp_buf
    _MdlBoolLongJumpBuf;			 /* Used in fatal Bool err. */
IRIT_GLOBAL_DATA_HEADER MdlFatalErrorType 
    _MdlBoolFatalErrorNum;

/* From mdl2bool.c. */
int MdlBoolClassifyTrimSrfLoops(MdlTrimSrfStruct *TSrf,
				CagdRType Tol,
				CagdBType InsideOtherModel);
int MdlBoolTrimSrfIntersects(const MdlTrimSrfStruct *TSrf);
void MdlBoolClassifyNonInterTrimSrfs(MdlModelStruct *Model);

#endif /* MDL_LOC_H */
