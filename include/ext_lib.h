/******************************************************************************
* Ext_lib.h - header file for the extensions library.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug 2009.					      *
******************************************************************************/

#ifndef EXTEN_LIB_H
#define EXTEN_LIB_H


#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* FUnctions related to rectangular region tiling from curves. */
CagdCrvStruct *IrtExtC2SOrderCurves(CagdCrvStruct *CrvList, 
				    int IsBoolSumOrder);
IPObjectStruct *IrtExtC2SGeneral(CagdCrvStruct **CrvList,
				 IrtRType AngularDeviation,
				 IrtRType ConcaveCorners,
				 int CurveOutputType,
				 IrtRType SizeRectangle,
				 int NumSmoothingSteps,
				 const char **ErrorMsg,
				 const char *Name);

void IrtExtExampleFunction(IrtRType *R, IrtVecType V);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* EXTEN_LIB_H */
