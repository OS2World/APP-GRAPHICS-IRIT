/*****************************************************************************
* Header Files ForOpen GL using CG (programmable hardware) drawing functions.*
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, January 2005.  *
*****************************************************************************/

#ifndef OPNGL_CG_H
#define OPNGL_CG_H

#include <windows.h>
#include <gl/gl.h>

#define CG_GET_NAMED_PARAMETER(VP, Name, Var) \
    Var = cgGetNamedParameter(VP, Name); \
    if (CheckCgError())	\
	return FALSE;
#define CG_GL_SET_PARAMETER_1F(Var, P1) \
    cgGLSetParameter1f(Var, P1); \
    CheckCgError();
#define CG_GL_SET_PARAMETER_2F(Var, P1, P2) \
    cgGLSetParameter2f(Var, P1, P2); \
    CheckCgError();
#define CG_GL_SET_PARAMETER_2D(Var, P1, P2) \
    cgGLSetParameter2d(Var, P1, P2); \
    CheckCgError();
#define CG_GL_SET_PARAMETER_3F(Var, P1, P2, P3) \
    cgGLSetParameter3f(Var, P1, P2, P3); \
    CheckCgError();
#define CG_GL_SET_PARAMETER_4F(Var, P1, P2, P3, P4) \
    cgGLSetParameter4f(Var, P1, P2, P3, P4); \
    CheckCgError();
#define CG_GL_SET_PARAMETER_4D(Var, P1, P2, P3, P4) \
    cgGLSetParameter4d(Var, P1, P2, P3, P4); \
    CheckCgError();
#define CG_GL_SET_STATE_MATRIX_PARAMETER(Var, StateMatrix, Transform); \
    cgGLSetStateMatrixParameter(Var, StateMatrix, Transform); \
    CheckCgError();

#ifndef GL_RGBA_FLOAT32_ATI
#define GL_RGBA_FLOAT32_ATI 0x8814
#endif

#ifndef GL_RGB_FLOAT32_ATI
#define GL_RGB_FLOAT32_ATI 0x8815
#endif


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER int GlblTextureActive;
IRIT_GLOBAL_DATA_HEADER IrtRType IGGlblOpacity;

int CheckCgError(void);

int InitPBuffer(int PbIdx,int XLen,int YLen);
void ErasePBuffer(int PbIdx);
int MakePBufferCurrent(int PbIdx);
int MakeCurrent(int PbIdx);
void ClosePBuffer(int PbIdx);
GLfloat * IGGetPBufferToHost(int PbIdx,int XLen,int YLen);
void IGDisplayObject(IPObjectStruct *PObj);
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* OPNGL_CG_H */

