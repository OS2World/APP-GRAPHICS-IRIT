/*****************************************************************************
* Implementation of stencil test and stencil operations, OpenGL-style.       *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/
#include "stencil.h"

static void StencilOp(IRndrStencilCfgStruct *SCfg,
                      int *SPtr,
                      IRndrStencilOpType Op);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs stencil tests.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   StencilCfg:     IN, stencil configuration to be used.                    M
*   Stencil:        IN, stecil buffer value to test.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtBType: comparison result.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   StencilTest, stencil, stencil test, opengl                               M
*****************************************************************************/
IrtBType StencilTest(IRndrStencilCfgStruct *StencilCfg, int Stencil)
{
    int A = (StencilCfg -> Ref) & (StencilCfg -> Mask),
        B = Stencil & (StencilCfg -> Mask);

    switch (StencilCfg -> SCmp) {
        case IRNDR_STENCIL_ALWAYS:
            return TRUE;
	case IRNDR_STENCIL_NEVER:
	    return FALSE;
	case IRNDR_STENCIL_LESS:
	    return A < B;
	case IRNDR_STENCIL_LEQUAL:
	    return A <= B;
	case IRNDR_STENCIL_GREATER:
	    return A > B;
	case IRNDR_STENCIL_GEQUAL:
	    return A >= B;
	case IRNDR_STENCIL_EQUAL:
	    return A == B;
	case IRNDR_STENCIL_NOTEQUAL:
	    return A != B;
    }
    return FALSE;   /* Undefined Func value. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs stencil operation in case of stencil test failure.              M
*                                                                            *
* PARAMETERS:                                                                M
*   StencilCfg:     IN, stencil configuration to be used.                    M
*   StencilPtr:     IN, pointer to target stencil buffer value.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   StencilOpFail, stencil, stencil operation, opengl                        M
*****************************************************************************/
void StencilOpFail(IRndrStencilCfgStruct *StencilCfg, int *StencilPtr)
{
    StencilOp(StencilCfg, StencilPtr, StencilCfg -> OpFail);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Performs stencil operation in case of stencil test success and Z test   M
*  failure.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   StencilCfg:     IN, stencil configuration to be used.                    M
*   StencilPtr:     IN, pointer to target stencil buffer value.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   StencilOpZFail, stencil, stencil operation, opengl                       M
*****************************************************************************/
void StencilOpZFail(IRndrStencilCfgStruct* StencilCfg, int *StencilPtr)
{
    StencilOp(StencilCfg, StencilPtr, StencilCfg -> OpZFail);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Performs stencil operation in case of stencil test pass and Z test      M
*    pass.                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   StencilCfg:    IN, stencil configuration to be used.                     M
*   StencilPtr:    IN, pointer to target stencil buffer value.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   StencilOpZPass,stencil, stencil operation, opengl                        M
*****************************************************************************/
void StencilOpZPass(IRndrStencilCfgStruct* StencilCfg, int *StencilPtr)
{
    StencilOp(StencilCfg, StencilPtr, StencilCfg -> OpZPass);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Performs stencil operation.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   SCfg:     IN, stencil configuration to be used.                          M
*   SPtr:     IN, pointer to target stencil buffer value.                    M
*   Op:       IN  stencil operation to perform.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   StencilOp, stencil, stencil operation, opengl                            M
*****************************************************************************/
void StencilOp(IRndrStencilCfgStruct *SCfg, int *SPtr, IRndrStencilOpType Op)
{
    switch (Op) {
        case IRNDR_STENCIL_INCR:
            (*SPtr)++;
	    return;
	case IRNDR_STENCIL_KEEP:
	    return;
	case IRNDR_STENCIL_ZERO:
	    (*SPtr) = 0;
	    return;
	case IRNDR_STENCIL_REPLACE:
	    (*SPtr) = SCfg -> Ref;
	    return;
	case IRNDR_STENCIL_DECR:
	    (*SPtr)--;
	    return;
	case IRNDR_STENCIL_INVERT:
	    (*SPtr) = ~(*SPtr);
	    return;
    }
}
