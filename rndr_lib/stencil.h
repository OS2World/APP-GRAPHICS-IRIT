/******************************************************************************
* Stencil.h - header file for the stencil in the RNDR library.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_STENCIL_H_
#define _IRNDR_STENCIL_H_

#include "rndr_loc.h"

typedef struct IRndrStencilCfgStruct {
    IRndrStencilCmpType SCmp;
    int Ref;
    unsigned Mask;
    IRndrStencilOpType OpFail;
    IRndrStencilOpType OpZFail;
    IRndrStencilOpType OpZPass;
} IRndrStencilCfgStruct;

IrtBType StencilTest(IRndrStencilCfgStruct *StencilCfg, int Stencil);

void StencilOpFail(IRndrStencilCfgStruct *StencilCfg, int *StencilPtr);

void StencilOpZFail(IRndrStencilCfgStruct *StencilCfg, int *StencilPtr);

void StencilOpZPass(IRndrStencilCfgStruct *StencilCfg, int *StencilPtr);

#endif /* _IRNDR_STENCIL_H_ */
