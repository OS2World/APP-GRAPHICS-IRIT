/*****************************************************************************
*  Open GL using CG (programmable hardware) drawing functions.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, May 2005.	     *
*****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "iritprsr.h"
#include "allocate.h"
#include "grap_loc.h"
#include "triv_lib.h"
#include "geom_lib.h"
#include "wntdrvs.h"
#include "editmanp.h"

#ifdef IRIT_HAVE_OGL_CG_LIB

#include <cg/cg.h>
#include <cg/cgGL.h>
/* This needs to be include into the CG SDK where the NV extensions reside. */
#include <gl/glext.h>
#include <gl/wglext.h>

#include "opngl_cg.h"

/* Compile with USE_PBUFFER only if GPU isn't supporting FrameBuffer Object.*/
/* FrameBuffer Object is Supported from OpenGl 2.0.			    */
/* PBuffer may be slower than FBO due to context switch.		    */

#ifndef USE_PBUFFER
#include "ogl_fbo.h"
#define FRAMEBUFFER_SIZE (1024)
IRIT_STATIC_DATA FrameBuffer GlblFBuffer;
#endif

#define PBUFFER_SIZE (1024)
#define POINT_SIZE (1)

enum { FIRST = 1, LAST = 2, OTHER = 4};
enum { NONE, NORMALOFFSET, NORMALTRIG };

typedef struct CGparams {
    CGparameter  ModelViewProjMatrix; 
    CGparameter  ModelViewMatrix; 
    CGparameter  Position;
    CGparameter  Color;
    CGparameter  Normal;
    CGparameter  TexCoord;
    CGparameter  TexDim; 
    CGparameter  TrivDim;
    CGparameter  UBases;
    CGparameter  VBases;
    CGparameter  WBases;
    CGparameter  GlblTvSpan;
    CGparameter  Range;
    CGparameter  MinLimit;
    CGparameter  GlblAnimVal;
    CGparameter  TexObject; 
    CGparameter  AmbientFactor;
    CGparameter  DiffuseFactor; 
    CGparameter  SpecularFactor; 
    CGparameter  Shininess;
    CGparameter  LightPos0;
    CGparameter  LightPos1;
    CGparameter  FlippedNormals;
    CGparameter  NormalMethod;
    CGparameter  Texture;
} CGparamsStruct;

typedef struct {
    CGparameter TargetCoord;
    CGparameter SourceCoord;
    CGparameter SourcePdCoord;
    CGparameter TexPosObj;
    CGparameter TexIdxObj;
} CGDmParamStruct; 

typedef struct PBuffer {
    HPBUFFERARB HBuffer;
    HGLRC PBufferRC;
    HDC PBufferDC;
    HGLRC OrigRC;
    HDC OrigDC;
    int Width;
    int Height;
    GLuint PBufferTexId;
} PBufferStruct;

/* Static data for CG shaders. */
IRIT_STATIC_DATA CGcontext 
    Context = NULL;
IRIT_STATIC_DATA CGprogram
    GlblFfdVertexProgram = NULL,
    GlblFfdFragmentProgram = NULL,
    GlblFfdVertexProgramPhase1 = NULL,
    GlblFfdVertexProgramPhase2 = NULL,
    GlblFfdFragmentProgramPhase2 = NULL;

IRIT_STATIC_DATA PBufferStruct GlblPBuffer[2];

IRIT_STATIC_DATA CGparamsStruct
    GlblCGParamSinglePhase,
    GlblCGParamDoublePhase1,
    GlblCGParamDoublePhase2,
    GlblCGParamDoublePhase2Fragment;

IRIT_STATIC_DATA CGprofile 
    GlblVertexProfile, 
    GlblFragmentProfile;

IRIT_STATIC_DATA int 
    GlblRenderTrivState,
    GlblVertexCount,
    GlblFfdShaderType,
    GlblFfdTexDimU,
    GlblFfdTexDimV,
    GlblMinParam[3],
    GlblRangeParam[3],
    GlblNrmlDir = 1,
    GlblCounterX = 0,
    GlblCounterY = 1, 
    GlblNx = PBUFFER_SIZE - POINT_SIZE,
    GlblNy = PBUFFER_SIZE - POINT_SIZE,
    GlblNormalCalcMethod = 0,
    GlblVertexPhase1TextureSize = 4096,
    GlblPBuffCurrent = -1,
    GlblObjColor[4];

IRIT_STATIC_DATA GLfloat
    *GlblVertexPhase1Texture = NULL;
IRIT_STATIC_DATA GLuint GlblVertexPhase1TextureHandle;

IRIT_STATIC_DATA IrtBboxType GlblObjBBox; 
IRIT_STATIC_DATA TrivTVStruct *GlblTv;
IRIT_STATIC_DATA IrtRType
    GlblAnimVal[3],
    GlblTvSpan[3];

/* Pointers to OpenGl functions. */
IRIT_STATIC_DATA PFNGLACTIVETEXTUREARBPROC
	glActiveTextureARB = NULL;
IRIT_STATIC_DATA PFNWGLGETPIXELFORMATATTRIBIVARBPROC 
	wglGetPixelFormatAttribivARB = NULL;
IRIT_STATIC_DATA PFNWGLGETPIXELFORMATATTRIBFVARBPROC 
	wglGetPixelFormatAttribfvARB = NULL;
IRIT_STATIC_DATA PFNWGLCHOOSEPIXELFORMATARBPROC 
	wglChoosePixelFormatARB = NULL;
IRIT_STATIC_DATA PFNWGLCREATEPBUFFERARBPROC 
	wglCreatePbufferARB = NULL;
IRIT_STATIC_DATA PFNWGLGETPBUFFERDCARBPROC 
	wglGetPbufferDCARB = NULL;
IRIT_STATIC_DATA PFNWGLRELEASEPBUFFERDCARBPROC 
	wglReleasePbufferDCARB = NULL;
IRIT_STATIC_DATA PFNWGLDESTROYPBUFFERARBPROC
	wglDestroyPbufferARB = NULL;
IRIT_STATIC_DATA PFNWGLQUERYPBUFFERARBPROC 
	wglQueryPbufferARB = NULL;
IRIT_STATIC_DATA PFNWGLBINDTEXIMAGEARBPROC 
	wglBindTexImageARB = NULL;
IRIT_STATIC_DATA PFNWGLRELEASETEXIMAGEARBPROC 
	wglReleaseTexImageARB = NULL;
IRIT_STATIC_DATA PFNWGLSETPBUFFERATTRIBARBPROC
	wglSetPbufferAttribARB = NULL;
IRIT_STATIC_DATA PFNGLMULTITEXCOORD3FARBPROC 
	glMultiTexCoord3f = NULL;
IRIT_STATIC_DATA PFNGLMULTITEXCOORD2FARBPROC 
	glMultiTexCoord2f = NULL;

static int IGLoadShaders(void);
static int IrtParseFfdTextureString(const char *DTexture,
				    char *FName,
				    int* ShaderType,
				    IrtRType GlblTvSpan[3],
				    int * TrivRenderState,
				    int * AnimSamples,
				    IrtPtType * ConstOffset,
				    int *NormalMethod);

static int ApplyDeformedObjectRender(IPObjectStruct * PObj);
static void IGGetAnimationFactors(IPObjectStruct *PObj, IrtRType t);
static int LoadFFDShadersSinglePhase(void);
static int LoadFFDShadersDoublePhase(void);
static int RenderShaderSinglePhase(IPObjectStruct *PObj);
static int RenderShaderDoublePhase(IPObjectStruct *PObj);
static void DrawModelAsGrid(IPObjectStruct * PObj);
static int IsArbExtensionSupported(const char *extension);
static void DrawTexturedQuad();
static void SetGlbGlblObjColor(IPObjectStruct *PObj);
static void UpdateShadingParameters(int algorithm);
static int IGCheckSysStatus(IPObjectStruct *PObj,int ForceRecompute);
static void RecomputeLists(IPObjectStruct *PObj);
static void SetControlPoint(IPObjectStruct *PObj);
static int PreProcessAnimation(IPObjectStruct * PObj);
static void RecomputeCacheGeom(IPObjectStruct * PObj,
			       int FreePolygons,
			       int FreeIsolines,
			       int FreeSketches,
			       int FreeCtlMesh);
static IPObjectStruct * SingleStepAnimation(IPObjectStruct * PObj);
static void IncrementInstanceCounter(IPObjectStruct * PObj);
static void ApplyDeformedVertices(IPObjectStruct *PObj,
				  int XLen, 
				  int YLen);

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Draw to the proper CG interface instead of the usual opengl             * 
*    Used for the single phase algorithm                                     *
*									     *
* PARAMETERS:                                                                *
*   Vertex:      Trivariate from which to create texture.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if the a texture object is created, FALSE otherwise.     *
*****************************************************************************/
static int IGCgVertexHanlingSinglePhase(IPVertexStruct *V)
{
    const char *Str;
    IrtRType TColor[4], Vx, Vy, Vz, T1, T2, T3;

    if (V -> Coord[0] < 0 || V -> Coord[0] > 1 ||
	V -> Coord[1] < 0 || V -> Coord[1] > 1 ||
	V -> Coord[2] < 0 || V -> Coord[2] > 1) {
	    if (IGGlblMore)
		printf("Error: Model not in unit box\n");
	    return FALSE;
	}

    if ((Str = AttrGetStrAttrib(V -> Attr, "RGB")) != NULL &&
	sscanf(Str, "%lf,%lf,%lf",
	    &TColor[0], &TColor[1], &TColor[2]) == 3) {
        TColor[0] /= 255.0;
        TColor[1] /= 255.0;
        TColor[2] /= 255.0;
	TColor[3] = IGGlblOpacity;
	glColor4f((GLfloat) TColor[0],
		  (GLfloat) TColor[1],
		  (GLfloat) TColor[2],
		  (GLfloat) TColor[3]);
    }
    
    Vx = (V -> Coord[0] + GlblAnimVal[0]) * 
	(GlblRangeParam[0] * GlblTvSpan[0]) + GlblMinParam[0];
    Vy = (V -> Coord[1] + GlblAnimVal[1]) * 
	(GlblRangeParam[1] * GlblTvSpan[1]) + GlblMinParam[1];
    Vz = (V -> Coord[2] + GlblAnimVal[2]) * 
	(GlblRangeParam[2] * GlblTvSpan[2]) + GlblMinParam[2];
    
    T1 = Vx - floor(Vx);
    glMultiTexCoord3f(0, (GLfloat) (0.5 - T1 + 0.5 * T1 * T1),
		         (GLfloat)(0.5 + T1 - T1 * T1),
		         (GLfloat)(0.5 * T1 * T1));

    T2 = Vy - floor(Vy);
    glMultiTexCoord3f(1, (GLfloat) (0.5 - T2 + 0.5 * T2 * T2),
		         (GLfloat)(0.5 + T2 - T2 * T2),
		         (GLfloat)(0.5 * T2 * T2));

    T3 = Vz - floor(Vz);
    glMultiTexCoord3f(2, (GLfloat) (0.5 - T3 + 0.5 * T3 * T3),
		         (GLfloat)(0.5 + T3 - T3 * T3),
		         (GLfloat)(0.5 * T3 * T3));

    glNormal3f(GlblNrmlDir * (GLfloat) V -> Normal[0],
	       GlblNrmlDir * (GLfloat) V -> Normal[1],
	       GlblNrmlDir * (GLfloat) V -> Normal[2]);

    glVertex4f((GLfloat) floor(Vx),
	       (GLfloat) floor(Vy),
	       (GLfloat) floor(Vz), 1);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Draw to the proper CG interface instead of the usual opengl             * 
*    Used for the first pass in the  double phase algorithm                  *
*	    								     *
* PARAMETERS:                                                                *
*   Vertex:      Trivariate from which to create texture.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if the a texture object is created, FALSE otherwise.     *
*****************************************************************************/
static int IGCgVertexHanlingFirstPass(IPVertexStruct * V)
{
    IrtPtType VP1, VP2, T1, T2, T3, P1, P2,
	X = { 1, 0, 0 },
	Y = { 0, 1, 0 },
	Z = { 0, 0, 1 };
    GLfloat Px = ((GLfloat) (GlblCounterX)) / ((GLfloat) (GlblNx)),
	    Py = ((GLfloat) (GlblCounterY)) / ((GLfloat) (GlblNy));

    if (V->Coord[0] < 0 || V -> Coord[0] > 1 ||
	V -> Coord[1] < 0 || V -> Coord[1] > 1 ||
	V -> Coord[2] < 0 || V -> Coord[2] > 1) {
	if (IGGlblMore)
	    printf("Error: Model not in unit box\n");
	return FALSE;
    }

    glTexCoord3f((GLfloat) (V -> Coord[0]), 
		 (GLfloat) (V -> Coord[1]),
		 (GLfloat) (V -> Coord[2]));
    glVertex4f(Px, Py, 0, 1);
    AttrSetUVAttrib(&V -> Attr, "xyvals", Px, Py);

    /* Update the global counters of the grid. */
    GlblCounterX += POINT_SIZE;
    if (GlblCounterX >= GlblNx) {
	GlblCounterY += POINT_SIZE;
	GlblCounterX = 0;
    }
    GlblVertexCount++;

    if (GlblNormalCalcMethod == 1) {
	GLfloat Offset = 0.5f;
	float epsilon = 0.0001f;
	IRIT_PT_COPY (P1, V -> Normal);
	IRIT_PT_SCALE(P1, epsilon);
	IRIT_PT_ADD(VP1, P1, V -> Coord);
	glTexCoord3f((GLfloat) VP1[0], (GLfloat) VP1[1], (GLfloat) VP1[2]);
	glVertex4f(Px, Py + Offset, 0, 1);
    }    

    /* Calculate Orthogonal Vectors. */
    else if (GlblNormalCalcMethod == 2) {
	GLfloat Offset = 0.333f;
	IrtRType epsilon = 0.001;
	IRIT_CROSS_PROD(T1, X, V -> Normal);
	IRIT_CROSS_PROD(T2, Y, V -> Normal);
	IRIT_CROSS_PROD(T3, Z, V -> Normal);
	if (IRIT_PT_LENGTH(T1) > IRIT_PT_LENGTH(T2)) 
	    IRIT_PT_COPY (P1, T1);
	else IRIT_PT_COPY (P1, T2);
	if (IRIT_PT_LENGTH(T3) > IRIT_PT_LENGTH(P1))
	    IRIT_PT_COPY (P1, T3);

	IRIT_PT_NORMALIZE(P1);
	IRIT_CROSS_PROD(P2, P1, V -> Normal);
	IRIT_PT_NORMALIZE(P2);
	IRIT_PT_SCALE(P1, epsilon);
	IRIT_PT_SCALE(P2, epsilon);
	IRIT_PT_ADD(VP1, (V -> Coord), P1);
	IRIT_PT_ADD(VP2, (V -> Coord), P2);

	glTexCoord3f((GLfloat) VP1[0], (GLfloat) VP1[1], (GLfloat) VP1[2]);
	glVertex4f(Px, Py + Offset, 0, 1);
	glTexCoord3f((GLfloat) VP2[0], (GLfloat) VP2[1], (GLfloat) VP2[2]);
	glVertex4f(Px, Py + 2 * Offset, 0, 1);
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Draw to the proper CG interface instead of the usual opengl             * 
*    Used for the single phase algorithm                                     *
*									     *
* PARAMETERS:                                                                *
*   Vertex:      Trivariate from which to create texture.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if the a texture object is created, FALSE otherwise.     *
*****************************************************************************/
static int IGCgVertexHandlingSecondPass(IPVertexStruct *V)
{
    const char *Str;
    IrtRType TColor[4];
    float *XY;
    	
    if ((Str = AttrGetStrAttrib(V -> Attr, "RGB")) != NULL &&
		sscanf(Str, "%lf,%lf,%lf",
		&TColor[0], &TColor[1], &TColor[2]) == 3) {
            TColor[0] /= 255.0;
            TColor[1] /= 255.0;
            TColor[2] /= 255.0;
	    TColor[3] = IGGlblOpacity;
	    glColor4f((GLfloat) TColor[0],
	      (GLfloat) TColor[1],
	      (GLfloat) TColor[2],
	      (GLfloat) TColor[3]);
	}

    glNormal3f(GlblNrmlDir* (GLfloat) V -> Normal[0],
                GlblNrmlDir* (GLfloat) V -> Normal[1],
                GlblNrmlDir* (GLfloat) V -> Normal[2]);
    XY = AttrGetUVAttrib(V -> Attr, "xyvals");    
    if (XY)
	glVertex4f(XY[0], XY[1], 0, 1);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* a trivariate B-spline - assumed to have a uniform knot                     * 
* Stores its control points in a texture. The texture is rounded up to the   *
* nearest power of two sizes - a limitation of the vertex texture HW ?       *
* PARAMETERS:                                                                *
*   PObj:      Trivariate from which to create texture.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if the a texture object is created, FALSE otherwise.     *
*****************************************************************************/
static int IGCreateFFDTexture(IPObjectStruct *DefObj, IPObjectStruct *TvObj)
{
    int k, j, i, 
	TexIdx = 0, 
	RowIdx = 0;
    
    GLfloat* ffdImg, *ffdIdxImg;
    GLuint ffdTexture, ffdIdxTexture;
    TrivTVStruct *tv = TvObj -> U.Trivars;
   
    GlblFfdTexDimU = tv -> WLength * tv -> ULength;
    GlblFfdTexDimV = tv -> VLength;
    for (k = 2; k < GlblFfdTexDimU; k *= 2);
	GlblFfdTexDimU = k;
    for (k = 2; k < GlblFfdTexDimV; k *= 2);
	GlblFfdTexDimV = k;
    
    ffdImg = 
	(GLfloat*) IritMalloc(GlblFfdTexDimV * GlblFfdTexDimU * sizeof(GLfloat) * 4);
    ffdIdxImg = 
	(GLfloat*) IritMalloc(GlblFfdTexDimV * GlblFfdTexDimU * sizeof(GLfloat) * 4);
    GlblVertexPhase1Texture = 
	(GLfloat*) IritMalloc(GlblVertexPhase1TextureSize * sizeof(GLfloat) * 4);

    IRIT_ZAP_MEM(ffdImg, sizeof(ffdImg));
    IRIT_ZAP_MEM(ffdIdxImg, sizeof(ffdIdxImg));
    
    for (k = 0; k < tv->WLength; k++) {
        for (j = 0; j < tv -> ULength; j++) {
            for (i = 0; i < tv -> VLength; i++) {
                int idx = TRIV_MESH_UVW(tv, j, i, k);

                if (TexIdx >= GlblFfdTexDimU * GlblFfdTexDimV)
		    break;
              
                *(ffdImg + 4 * TexIdx) = (GLfloat) tv -> Points[1][idx];
                *(ffdImg + 4 * TexIdx+1) = (GLfloat) tv -> Points[2][idx];
                *(ffdImg + 4 * TexIdx+2) = (GLfloat) tv -> Points[3][idx];
                *(ffdImg + 4 * TexIdx+3) = (GLfloat) 1;

		*(ffdIdxImg + 4 * TexIdx) = (GLfloat) j;
                *(ffdIdxImg + 4 * TexIdx + 1) = (GLfloat) i;
                *(ffdIdxImg + 4 * TexIdx + 2) = (GLfloat) k;
                *(ffdIdxImg + 4 * TexIdx + 3) = (GLfloat) 1;
                
                TexIdx++;
                RowIdx++;
                if (RowIdx % tv -> VLength == 0) {
                    TexIdx += (GlblFfdTexDimV - tv -> VLength);
                    RowIdx =0;
                }
               
            }
        }
    }

    for (i = 0; i < GlblVertexPhase1TextureSize*4; i++)
	GlblVertexPhase1Texture[i] = 0;

    for (i = 0; i < GlblVertexPhase1TextureSize; i++) {
	IrtRType
	    t1 = (IrtRType) i / (GlblVertexPhase1TextureSize - 1);

	GlblVertexPhase1Texture[4 * i] = (GLfloat) (0.5 - t1 + 0.5 * t1 * t1);
	GlblVertexPhase1Texture[4 * i + 1] = (GLfloat) (0.5 + t1 - t1 * t1);
	GlblVertexPhase1Texture[4 * i + 2] = (GLfloat) (0.5 * t1 * t1);
	GlblVertexPhase1Texture[4 * i + 3] = 0;

    }

    glGenTextures(1, &ffdTexture);
    glBindTexture(GL_TEXTURE_2D, ffdTexture);
    /* We must use GL_RGBA_FLOAT32_ATI for texture in vertex shaders. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI, 
		 GlblFfdTexDimV,GlblFfdTexDimU, 0, GL_RGBA, GL_FLOAT, ffdImg);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    IritFree(ffdImg);

   
    glGenTextures(1, &ffdIdxTexture);
    glBindTexture(GL_TEXTURE_2D, ffdIdxTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI, 
		 GlblFfdTexDimV,GlblFfdTexDimU, 0, GL_RGBA, GL_FLOAT, ffdIdxImg);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    IritFree(ffdIdxImg);

    glGenTextures(1, &GlblVertexPhase1TextureHandle);
    glBindTexture(GL_TEXTURE_2D, GlblVertexPhase1TextureHandle);
    /* We must use GL_RGBA_FLOAT32_ATI for texture in vertex shaders. */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI,
		 GlblVertexPhase1TextureSize, 1,
		 0, GL_RGBA, GL_FLOAT, GlblVertexPhase1Texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		    GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		    GL_NEAREST);
    IritFree(GlblVertexPhase1Texture);

    AttrSetObjectIntAttrib(TvObj, "_ffd_tex_id", ffdTexture);
    AttrSetObjectIntAttrib(TvObj, "_ffd_idx_tex_id", ffdIdxTexture);
    AttrSetObjectPtrAttrib(TvObj, "_ffd_def_obj", DefObj); 

   return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Checks if machine hardware supports VP40 & FP40 cg profiles.               * 
* Loading the shaders only if there is support.                              * 
*                                                                            *
* PARAMETERS:                                                                *
*   void                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if loading was successful, FALSE otherwise.              *
*****************************************************************************/
static int IGLoadShaders(void)
{   
    switch (GlblFfdShaderType) {
        case 0:
            return LoadFFDShadersSinglePhase();
        case 1:
            return LoadFFDShadersDoublePhase();     
        default:
            return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Load shaders and assign parameter handles for the single phase algorithm   * 
* Loading the shaders only if there is support.                              * 
*                                                                            *
* PARAMETERS:                                                                *
*   void                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if loading was successful, FALSE otherwise.              *
*****************************************************************************/
static int LoadFFDShadersSinglePhase(void)
{
    const char *ShaderPath;

    if (GlblFfdVertexProgram != NULL) 
    	return TRUE; /* Valid shaders are already loaded. */

    /* CG_PROFILE_VP40 supports the NV_vertex_program3 extension. */
    
    if (cgGLIsProfileSupported(CG_PROFILE_VP40) &&
    	cgGLIsProfileSupported(CG_PROFILE_FP40)) {
		GlblVertexProfile = CG_PROFILE_VP40;
		cgGLSetOptimalOptions(GlblVertexProfile);
		GlblFragmentProfile = CG_PROFILE_FP40;
		cgGLSetOptimalOptions(GlblFragmentProfile);
    }
    else
        return FALSE; 

    /* Test cgContext creation. */
    Context = cgCreateContext();
    if (CheckCgError())
        return FALSE;
    
    ShaderPath = searchpath("ffd_shdr.cg");

    GlblFfdVertexProgram = cgCreateProgramFromFile(Context, 
						   CG_SOURCE, ShaderPath,
						   GlblVertexProfile,
						   "FFDSinglePhase", NULL);
    if (CheckCgError() && 
        GlblFfdVertexProgram != NULL)
	return FALSE;

    if (!cgIsProgramCompiled(GlblFfdVertexProgram))
        cgCompileProgram(GlblFfdVertexProgram);

    cgGLLoadProgram(GlblFfdVertexProgram);
    if (CheckCgError())
        return FALSE;

    /* Bind the handles to semantics. */
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.position",
			   GlblCGParamSinglePhase.Position);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.normal",
			   GlblCGParamSinglePhase.Normal);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.color",
			   GlblCGParamSinglePhase.Color);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "ModelViewProj",
			   GlblCGParamSinglePhase.ModelViewProjMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "ModelView", 
			   GlblCGParamSinglePhase.ModelViewMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "TrivDim",
			   GlblCGParamSinglePhase.TrivDim);    
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "TextureDim",
			   GlblCGParamSinglePhase.TexDim);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.UBases",
			   GlblCGParamSinglePhase.UBases);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.VBases", 
			   GlblCGParamSinglePhase.VBases);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "vIn.WBases",
			   GlblCGParamSinglePhase.WBases);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "texture",
			   GlblCGParamSinglePhase.TexObject);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "AmbientFactor", 
			   GlblCGParamSinglePhase.AmbientFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "DiffuseFactor",
			   GlblCGParamSinglePhase.DiffuseFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "SpecularFactor",
			   GlblCGParamSinglePhase.SpecularFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "Shininess",
			   GlblCGParamSinglePhase.Shininess);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "LightPos0",
			   GlblCGParamSinglePhase.LightPos0);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgram, "LightPos1",
			   GlblCGParamSinglePhase.LightPos1);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Load shaders and assign parameter handles for the single phase algorithm   * 
* Loading the shaders only if there is support.                              * 
*                                                                            *
* PARAMETERS:                                                                *
*   void                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if loading was successful, FALSE otherwise.              *
*****************************************************************************/
static int LoadFFDShadersDoublePhase(void)
{
    const char *ShaderPath;

    if (GlblFfdVertexProgramPhase1 != NULL &&
	    GlblFfdVertexProgramPhase2 &&
	    GlblFfdFragmentProgram != NULL &&
	    GlblFfdFragmentProgramPhase2 != NULL) 
        return TRUE; /* Valid shaders are already loaded. */

    /* CG_PROFILE_VP40 supports the NV_vertex_program3 extension */
    if (cgGLIsProfileSupported(CG_PROFILE_VP40) &&
    	    cgGLIsProfileSupported(CG_PROFILE_FP40)) {
	GlblVertexProfile = CG_PROFILE_VP40;
	cgGLSetOptimalOptions(GlblVertexProfile);
	GlblFragmentProfile = CG_PROFILE_FP40;
	cgGLSetOptimalOptions(GlblFragmentProfile);
    }
    else
        return FALSE; 

    Context = cgCreateContext();
    if (CheckCgError())
        return FALSE;

    ShaderPath = searchpath("ffd_shdr.cg");

    /* Phase 1 vertex shader. */
    GlblFfdVertexProgramPhase1 = cgCreateProgramFromFile(Context, 
                             CG_SOURCE, ShaderPath,
	                     GlblVertexProfile, "FFDDoublePhaseVertex1", NULL);
    if (CheckCgError() || 
        GlblFfdVertexProgramPhase1 == NULL)
	    return FALSE;
    
    if (!cgIsProgramCompiled(GlblFfdVertexProgramPhase1))
        cgCompileProgram(GlblFfdVertexProgramPhase1);
    
    cgGLLoadProgram(GlblFfdVertexProgramPhase1);
    if (CheckCgError())
        return FALSE;

    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"vIn.position", GlblCGParamDoublePhase1.Position);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"TvSpan", GlblCGParamDoublePhase1.GlblTvSpan);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"Range", GlblCGParamDoublePhase1.Range);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"MinLimit", GlblCGParamDoublePhase1.MinLimit);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"AnimVal", GlblCGParamDoublePhase1.GlblAnimVal);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, 
	"vIn.texCoord", GlblCGParamDoublePhase1.TexCoord);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"ModelViewProj", GlblCGParamDoublePhase1.ModelViewProjMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1,
	"Texture", GlblCGParamDoublePhase1.Texture);
    
    /* Phase 1 fragment shader. */
    GlblFfdFragmentProgram = cgCreateProgramFromFile(Context, 
                             CG_SOURCE, ShaderPath,
	                     GlblFragmentProfile, "FFDDoublePhasePixel", NULL);
    if (CheckCgError() || GlblFfdFragmentProgram == NULL)
	return FALSE;
    
    if (!cgIsProgramCompiled(GlblFfdFragmentProgram))
        cgCompileProgram(GlblFfdFragmentProgram);

    cgGLLoadProgram(GlblFfdFragmentProgram);
    if (CheckCgError())
	return FALSE;
      
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgram, 
	"TrivDim", GlblCGParamDoublePhase1.TrivDim);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgram,
	"TextureDim", GlblCGParamDoublePhase1.TexDim);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgram, 
	"texture", GlblCGParamDoublePhase1.TexObject);

    /* Phase 2 vertex shader. */
    GlblFfdVertexProgramPhase2 = cgCreateProgramFromFile(Context, 
                            CG_SOURCE, ShaderPath,
		            GlblVertexProfile, "FFDDoublePhaseVertex2", NULL);
    if (CheckCgError() || GlblFfdVertexProgramPhase2 == NULL)
	return FALSE;

    if (!cgIsProgramCompiled(GlblFfdVertexProgramPhase2))
        cgCompileProgram(GlblFfdVertexProgramPhase2);    
 
    cgGLLoadProgram(GlblFfdVertexProgramPhase2);
    if (CheckCgError())
	return FALSE;

    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "vIn.position",
			   GlblCGParamDoublePhase2.Position);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "vIn.color", 
			   GlblCGParamDoublePhase2.Color);    
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "AmbientFactor", 
			   GlblCGParamDoublePhase2.AmbientFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "DiffuseFactor",
			   GlblCGParamDoublePhase2.DiffuseFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "SpecularFactor",
			   GlblCGParamDoublePhase2.SpecularFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "Shininess",
			   GlblCGParamDoublePhase2.Shininess);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "LightPos0",
			   GlblCGParamDoublePhase2.LightPos0);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "LightPos1",
			   GlblCGParamDoublePhase2.LightPos1);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "ModelView",
			   GlblCGParamDoublePhase2.ModelViewMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "ModelViewProj",
			   GlblCGParamDoublePhase2.ModelViewProjMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "texture",
			   GlblCGParamDoublePhase2.TexObject);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "FlippedNormals",
			   GlblCGParamDoublePhase2.FlippedNormals);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase2, "NormalMethod",
			   GlblCGParamDoublePhase2.NormalMethod);

    /* Phase 2 fragment shader. */
    GlblFfdFragmentProgramPhase2 = cgCreateProgramFromFile(Context, 
				CG_SOURCE, ShaderPath, GlblFragmentProfile,
				"FFDDoublePhaseFragment2", NULL);
    if (CheckCgError() || GlblFfdVertexProgramPhase2 == NULL)
	return FALSE;

    if (!cgIsProgramCompiled(GlblFfdFragmentProgramPhase2))
        cgCompileProgram(GlblFfdFragmentProgramPhase2);    
 
    cgGLLoadProgram(GlblFfdFragmentProgramPhase2);
    if (CheckCgError())
	return FALSE;

    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "AmbientFactor", 
			   GlblCGParamDoublePhase2Fragment.AmbientFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "DiffuseFactor",
			   GlblCGParamDoublePhase2Fragment.DiffuseFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "SpecularFactor",
			   GlblCGParamDoublePhase2Fragment.SpecularFactor);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "Shininess",
			   GlblCGParamDoublePhase2Fragment.Shininess);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "LightPos0",
			   GlblCGParamDoublePhase2Fragment.LightPos0);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "LightPos1", 
			   GlblCGParamDoublePhase2Fragment.LightPos1);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "ModelView",
			   GlblCGParamDoublePhase2Fragment.ModelViewMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "FlippedNormals",
			   GlblCGParamDoublePhase2Fragment.FlippedNormals);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgramPhase2, "NormalMethod", 
			   GlblCGParamDoublePhase2Fragment.NormalMethod);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Parses the string of the "ffd_texture" attribute	      	             *
*                                                                            *
* PARAMETERS:                                                                *
*   DTexture:       The string of the "dtexture" attribute.                  *
*   FName:          The texture file name will be placed here.               *
*   ShaderType:     The algorithm to use                                     *
*   NormalMethod:   Normals Calculation method                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:         TRUE if parsed succesfully, FALSE otherwise.                *
*****************************************************************************/ 
static int IrtParseFfdTextureString(const char *DTexture,
				    char *FName,
				    int *ShaderType,
				    IrtRType GlblTvSpan[3],
				    int *TrivRenderState,
				    int *AnimSamples,
				    IrtPtType *ConstOffset,
				    int *NormalMethod)
{
    char *p;

    if (DTexture == NULL)
    	return FALSE;

    GlblTvSpan[0] = 1.0;
    GlblTvSpan[1] = 1.0;
    GlblTvSpan[2] = 1.0;

    strncpy(FName, DTexture, IRIT_LINE_LEN_LONG - 1);
    if ((p = strchr(FName, ',')) != NULL) {
	*p++ = 0; /* Mark the end of the regular string. */
    	if (sscanf(p, "%d,%d,%lf,%lf,%lf,%d,%lf,%lf,%lf,%d", ShaderType, 
		   TrivRenderState, &GlblTvSpan[0], &GlblTvSpan[1], 
		   &GlblTvSpan[2], AnimSamples,	&((*ConstOffset)[0]),
		   &((*ConstOffset)[1]), &((*ConstOffset)[2]),
		   NormalMethod) != 10)
	return FALSE;
    }
    SearchPath(NULL, FName, NULL, IRIT_LINE_LEN_LONG - 1, FName, NULL);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluate scl_z animation curve, if aGlblNy, to set the scaling factor.   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   The DDM texture object.                                          *
*   t:      Current animation time.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    Scale factor computed, one by default.                      *
*****************************************************************************/
static void IGGetAnimationFactors(IPObjectStruct *PObj, IrtRType t)
{
    int i;
    IrtRType
        Factor = 1.0;
    IPObjectStruct *TCrv,
	*AnimCrv = AttrGetObjectObjAttrib(PObj, "ffd_anim"),
	*Crvs[3] = {NULL};

    if (AnimCrv != NULL) {
        if (IP_IS_OLST_OBJ(AnimCrv)) {
	    int ListLen = IPListObjectLength(AnimCrv);
	    for (i = 0; i < ListLen; i++) {
		TCrv = IPListObjectGet(AnimCrv, i);
		if (IP_IS_CRV_OBJ(TCrv)) {
		    if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_u", 7) == 0)
			Crvs[0] = TCrv;
		    else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_v", 7) == 0)
			Crvs[1] = TCrv;
		    else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_w", 7) == 0)
			Crvs[2] = TCrv;
		}
	    }
    
	}
	else {
	    TCrv = AnimCrv;
	    if (IP_IS_CRV_OBJ(TCrv)) {
		if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_u", 7) == 0)
		    Crvs[0] = TCrv;
		else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_v", 7) == 0)
		    Crvs[1] = TCrv;
		else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "trans_w", 7) == 0)
		    Crvs[2] = TCrv;
	    }
	}
    
	for (i = 0; i < 3; i++) {
	    IrtRType *R, TMin, TMax;
	    CagdCrvStruct *Crv;
	    
	    if (Crvs[i] == NULL)
		continue;
	    Crv = Crvs[i] -> U.Crvs;
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (t < TMin)
		t = TMin;
	    else if (t > TMax)
		t = TMax;
	    R = CagdCrvEval(Crv, t);
	    GlblAnimVal[i] = CAGD_IS_RATIONAL_CRV(Crv) ? R[1] / R[0] : R[1];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read the trivariate file name form the model definition                  *
* set up the texture for further processing.                                 *
*                                                                            *
* PARA*ETERS:                                                                *
*   PObj:      Object to extract its texture mapping function if has one.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if object has ddm texture and process was successful     *
*****************************************************************************/
static int IGInitFfdDraw(IPObjectStruct *PObj)
{
    IPObjectStruct* DefObj;
    int NormalMethod,
	AnimSamples = 1;
    IrtPtType * ConstOffset;
    char FfdTextureFile[IRIT_LINE_LEN_LONG], *ModelFileName;
    const char *Texture;

    /* Store the BBox for use in the vertex callback */
    if (glMultiTexCoord3f == NULL)
        glMultiTexCoord3f = 
	    (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3f");
    if (glMultiTexCoord2f == NULL)
        glMultiTexCoord2f = 
	    (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2f");
    if (glActiveTextureARB == NULL)
        glActiveTextureARB = 
	    (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
    
    if (!IP_IS_TRIVAR_OBJ(PObj))
	return FALSE;

    /* Texture already created. */
    if (AttrGetObjectIntAttrib(PObj, "_ffd_tex_id") != IP_ATTR_BAD_INT)
        return TRUE;

    Texture = AttrGetObjectStrAttrib(PObj, "ffd_texture");
    if (Texture == NULL)
    	return FALSE;
    ConstOffset = (IrtPtType*) (IritMalloc(sizeof(IrtPtType)));
    (*ConstOffset)[0] = (*ConstOffset)[1] = (*ConstOffset)[2] = 0;
    if (!IrtParseFfdTextureString(Texture, FfdTextureFile, &GlblFfdShaderType,
				  GlblTvSpan, &GlblRenderTrivState,
				  &AnimSamples,	ConstOffset, &NormalMethod)) 
        return FALSE;
    ModelFileName = FfdTextureFile;
    if ((DefObj = IPGetDataFiles(&ModelFileName, 1, TRUE, TRUE)) == NULL)
	return FALSE;
    
    if (IGCreateFFDTexture(DefObj,PObj) == FALSE)
	return FALSE;
    
    if (GlblFfdShaderType == 1)
#ifdef USE_PBUFFER
	if (InitPBuffer(0, PBUFFER_SIZE, PBUFFER_SIZE) == FALSE) {
#else
	if ((GlblFBuffer = FrameBufferCreate(FRAMEBUFFER_SIZE,
					     FRAMEBUFFER_SIZE, 
					  GL_COLOR_ATTACHMENT0_EXT)) == NULL) {
#endif
	    if (IGGlblMore) {
		printf("Failed To init PBuffer\n");
	    }
	    return FALSE;
	}

    AttrSetObjectIntAttrib(PObj, "_anim_samples", AnimSamples);
    AttrSetObjectIntAttrib(PObj, "_normal_method", NormalMethod);
    AttrSetObjectPtrAttrib(PObj, "_const_samples", ConstOffset);
#ifdef DEBUG_CG_FFD
    printf("const sample [%lf,%lf,%lf]\n",
	(*ConstOffset)[0], (*ConstOffset)[1], (*ConstOffset)[2]);
#endif

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Init a single PBuffer as rendering surface. of type float32 RGBA         M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer Index							     M
*   XLen: X Size of the buffer						     M
*   YLen: Y Size of the buffer						     M 
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if no go.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InitPBuffer                                                              M
*****************************************************************************/
int InitPBuffer(int PbIdx, int XLen, int YLen)
{
    GLuint NumFormats;
    GLint PixelFormat;
    const int
	PBufferFlags[] = {
	    WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,
	    WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
	    0, 0, 0
	};
    const int
	IAttribList[]={
	    WGL_BIND_TO_TEXTURE_RGBA_ARB, 1, 
	    WGL_DRAW_TO_PBUFFER_ARB, 1,
	    WGL_SUPPORT_OPENGL_ARB, 1,
	    WGL_PIXEL_TYPE_ARB,WGL_TYPE_RGBA_FLOAT_ATI,
	    WGL_RED_BITS_ARB, 32,
	    WGL_GREEN_BITS_ARB, 32,
	    WGL_BLUE_BITS_ARB, 32,
	    WGL_ALPHA_BITS_ARB, 32,
	    0
	};
    const float
	FAttribList[] = { 0 };

    if (GlblPBuffer[PbIdx].HBuffer) /* PBuffer already exists. */
        return TRUE;
    
    if (!IsArbExtensionSupported("WGL_ARB_extensions_string") ||
	!IsArbExtensionSupported("WGL_ARB_pixel_format") ||
	!IsArbExtensionSupported("WGL_ARB_pbuffer"))
        return FALSE;
	
    glGenTextures(1, &GlblPBuffer[PbIdx].PBufferTexId);
    glBindTexture(GL_TEXTURE_2D, GlblPBuffer[PbIdx].PBufferTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GlblPBuffer[PbIdx].OrigDC = wglGetCurrentDC();
    GlblPBuffer[PbIdx].OrigRC = wglGetCurrentContext();
    
    wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
        wglGetProcAddress("wglGetPixelFormatAttribivARB");
    wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)
        wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
        wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)
        wglGetProcAddress("wglCreatePbufferARB");
    wglGetPbufferDCARB=(PFNWGLGETPBUFFERDCARBPROC)
	wglGetProcAddress("wglGetPbufferDCARB");
    wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)
    	wglGetProcAddress("wglReleasePbufferDCARB");
    wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)
	wglGetProcAddress("wglDestroyPbufferARB");
    wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)
	wglGetProcAddress("wglQueryPbufferARB");
    wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)
        wglGetProcAddress("wglBindTexImageARB");
    wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)
        wglGetProcAddress("wglReleaseTexImageARB");
    wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)
        wglGetProcAddress("wglSetPbufferAttribARB");
    
    if (!wglChoosePixelFormatARB(GlblPBuffer[PbIdx].OrigDC, IAttribList, 
				 FAttribList, 1, &PixelFormat, &NumFormats))
        return FALSE;
    
    GlblPBuffer[PbIdx].Width =  XLen;
    GlblPBuffer[PbIdx].Height = YLen;
    GlblPBuffer[PbIdx].HBuffer = 
	wglCreatePbufferARB(GlblPBuffer[PbIdx].OrigDC, PixelFormat, 
			    GlblPBuffer[PbIdx].Width, 
			    GlblPBuffer[PbIdx].Height, PBufferFlags);
    if (!GlblPBuffer[PbIdx].HBuffer)
	return FALSE;
    GlblPBuffer[PbIdx].PBufferDC = 
	wglGetPbufferDCARB(GlblPBuffer[PbIdx].HBuffer);
    if (!GlblPBuffer[PbIdx].PBufferDC)
	return FALSE;

    GlblPBuffer[PbIdx].PBufferRC = 
	wglCreateContext(GlblPBuffer[PbIdx].PBufferDC);
    if (!GlblPBuffer[PbIdx].PBufferRC)
        return FALSE;
    wglQueryPbufferARB(GlblPBuffer[PbIdx].HBuffer,
		       WGL_PBUFFER_WIDTH_ARB, &GlblPBuffer[PbIdx].Width);
    wglQueryPbufferARB(GlblPBuffer[PbIdx].HBuffer, 
		       WGL_PBUFFER_HEIGHT_ARB, &GlblPBuffer[PbIdx].Height);
    wglShareLists(GlblPBuffer[PbIdx].OrigRC, GlblPBuffer[PbIdx].PBufferRC);

#ifdef DEBUG_CG_FFD    
    printf("Create pbuffer size = (%dX%d)\n", 
	   GlblPBuffer[PbIdx].Width, GlblPBuffer[PbIdx].Height);
#endif
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Close the PBuffer.						             M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer Index							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void.				    				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ClosePBuffer, InitPBuffer                                                M
*****************************************************************************/
void ClosePBuffer(int PbIdx)
{
    if (GlblPBuffer[PbIdx].HBuffer == NULL)
	return;
    if (GlblPBuffer[PbIdx].PBufferRC) {
	if (!wglDeleteContext(GlblPBuffer[PbIdx].PBufferRC)) {
#ifdef DEBUG_CG_FFD
	    printf("Fail to release Pbuffer Rendering Context.\n");
#endif
       }
       GlblPBuffer[PbIdx].PBufferRC = NULL;
    }
    if (GlblPBuffer[PbIdx].PBufferDC &&
	!wglReleasePbufferDCARB(GlblPBuffer[PbIdx].HBuffer, 
				GlblPBuffer[PbIdx].PBufferDC)) {
#ifdef DEBUG_CG_FFD
        printf("Fail to release Pbuffer DC\n"); 
#endif
	GlblPBuffer[PbIdx].PBufferDC = NULL;
    }
	
    if (!wglDestroyPbufferARB(GlblPBuffer[PbIdx].HBuffer)) {
#ifdef DEBUG_CG_FFD
	printf("Fail to destroy pbuffer\n");
#endif
    }
    GlblPBuffer[PbIdx].HBuffer = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a Black rectangle to the PBuffer to make sure it is in init state  M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer to close                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void.                                        			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ErasePBuffer                                                             M
*****************************************************************************/
void ErasePBuffer(int PbIdx)
{
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBegin(GL_POLYGON);
    glColor4f(0, 0, 0, 1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glVertex3f(1, 1, 0);
    glVertex3f(1, 0, 0);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    wglReleaseTexImageARB(GlblPBuffer[PbIdx].HBuffer, WGL_FRONT_LEFT_ARB);
    glDisable(GL_TEXTURE_2D);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Changes the valid pbuffer index					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer index to make current                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE for success, FALSE otherwise.                      	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MakePBufferCurrent                                                       M
*****************************************************************************/
int MakePBufferCurrent(int PbIdx)
{
    if (GlblPBuffCurrent == 1)
	return TRUE;
    if (!wglMakeCurrent(GlblPBuffer[PbIdx].PBufferDC,
			GlblPBuffer[PbIdx].PBufferRC))
        return FALSE;
    GlblPBuffCurrent = 1;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Changes the valid pbuffer index					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer index to make current                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE for success, FALSE otherwise.                            	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MakeCurrent                                                              M
*****************************************************************************/
int MakeCurrent(int PbIdx)
{
    if (GlblPBuffCurrent == 0)
	return TRUE;

    if (!wglMakeCurrent(GlblPBuffer[PbIdx].OrigDC, GlblPBuffer[PbIdx].OrigRC))
        return FALSE;

    GlblPBuffCurrent = 0;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates and allocate an array from pbuffer data			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PbIdx: Buffer index			                                     M
*   XLen:  X length of the buffer					     M
*   YLen:  Y length of the buffer					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   GLfloat *: pointer to the allocated array                         	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGetPBufferToHost                                                       M
*****************************************************************************/
GLfloat *IGGetPBufferToHost(int PbIdx, int XLen, int YLen)
{
    GLfloat *Image;

    MakePBufferCurrent(PbIdx);
    Image = (GLfloat*) IritMalloc(XLen * YLen * 4 * sizeof(GLfloat));
    glReadPixels(0, 0, XLen, YLen, GL_RGBA, GL_FLOAT, Image);
    MakeCurrent(PbIdx);
    return Image;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a single object with DTexture attributes using current modes       M
*   and transformations.	                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if no go.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGFfdDraw                                                              M
*****************************************************************************/
int IGCGFfdDraw(IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA int LocInList = 0;
    int Ret = FALSE;

    if (!IGInitFfdDraw(PObj) ||
	!IGLoadShaders())
        return FALSE;
    
    glPushAttrib(GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    switch (GlblFfdShaderType) {
	case 0:
	    Ret = RenderShaderSinglePhase(PObj);
	    break;
	case 1:
	    Ret = RenderShaderDoublePhase(PObj);
	    break;
    }
    glPopAttrib();
    if (GlblRenderTrivState) {
	return FALSE; /* Force the triv to be rendered. */
    }
    else
	return TRUE; /* Finish. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Draw FFD using single pass rendering algorithm on the vertex processor   *
*   assume to load the proper shaders                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*                                                                            *
*****************************************************************************/
static int RenderShaderSinglePhase(IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA int NumPolys;
    int TexId;
    IGVertexHandleExtraFuncType OldFunc;
    TrivTVStruct *Tv;
    IPObjectStruct *DefObj;
    GLuint ModelDisplayList;

    SetGlbGlblObjColor(PObj);
    IGCheckSysStatus(PObj,0);
    OldFunc = IGSetHandleVertexProcessingFunc(IGCgVertexHanlingSinglePhase); 
    
    if (IGGlblAnimation) {
        IGGetAnimationFactors(PObj,IGAnimation.RunTime);
	RecomputeLists(PObj);			      
    }
    else{
        GlblAnimVal[0] = 0.0;
        GlblAnimVal[1] = 0.0;
        GlblAnimVal[2] = 0.0;
    }
    DefObj = SingleStepAnimation(PObj);
    if (DefObj == NULL)
	DefObj = (IPObjectStruct*)AttrGetObjectPtrAttrib(PObj, "_ffd_def_obj");
    
    Tv = PObj->U.Trivars;
    GlblMinParam[0] = Tv -> UOrder - 1;
    GlblMinParam[1] = Tv -> VOrder - 1;
    GlblMinParam[2] = Tv -> WOrder - 1;
    GlblRangeParam[0] =  Tv -> ULength - GlblMinParam[0];
    GlblRangeParam[1] =  Tv -> VLength - GlblMinParam[1];
    GlblRangeParam[2] =  Tv -> WLength - GlblMinParam[2];
    
    memcpy(GlblObjBBox, DefObj -> BBox, sizeof(GlblObjBBox)); 
    
     /* Enabling shaders & Textures. */
    cgGLEnableProfile(GlblVertexProfile);
    CheckCgError();

    cgGLBindProgram(GlblFfdVertexProgram);
    CheckCgError();

    CG_GL_SET_PARAMETER_2F(GlblCGParamSinglePhase.TexDim, 
	(GLfloat) 1.0 / GlblFfdTexDimV, (GLfloat) 1.0 / GlblFfdTexDimU);
    CG_GL_SET_PARAMETER_3F(GlblCGParamSinglePhase.TrivDim,
        (GLfloat) Tv -> ULength, 
        (GLfloat) Tv -> VLength,
        (GLfloat) Tv -> WLength);
    
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamSinglePhase.ModelViewProjMatrix,
				     CG_GL_MODELVIEW_PROJECTION_MATRIX,
				     CG_GL_MATRIX_IDENTITY);
    
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamSinglePhase.ModelViewMatrix,
				     CG_GL_MODELVIEW_MATRIX,
				     CG_GL_MATRIX_IDENTITY);
    
    UpdateShadingParameters(1);

    TexId = AttrGetObjectIntAttrib(PObj, "_ffd_tex_id");
    glBindTexture(GL_TEXTURE_2D, TexId);
    cgGLSetTextureParameter(GlblCGParamSinglePhase.TexObject, TexId);	
    CheckCgError();
    cgGLEnableTextureParameter(GlblCGParamSinglePhase.TexObject);
	
    /* Render loop - for this version we have to traverse here.	 Recursive   */
    /* call after attr set - force normal render accept for attr setting.    */
    if ((ModelDisplayList = AttrGetObjectIntAttrib(PObj,
				  "_ffd_model_disp_lst")) != IP_ATTR_BAD_INT) {
        glCallList(ModelDisplayList);
	IGGlblNumPolys = NumPolys;
    }
    else {
	NumPolys = IGGlblNumPolys;
	ModelDisplayList  = glGenLists(1);
        AttrSetObjectIntAttrib(PObj, "_ffd_model_disp_lst", ModelDisplayList);
        glNewList(ModelDisplayList, GL_COMPILE);
	IGDisplayObject(DefObj);
	glEndList();
	NumPolys = IGGlblNumPolys - NumPolys; 
	glCallList(ModelDisplayList);
    }
    
    cgGLDisableTextureParameter(GlblCGParamSinglePhase.TexObject);
    cgGLDisableProfile(GlblVertexProfile);

    IGSetHandleVertexProcessingFunc(OldFunc);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Draw FFD using double pass rendering algorithm on the pixel and vertex   *
* processos. First pass will compute deformation on the vertex and pixel     *
* processors and output the result to a pbuffer. The second pass will        *
* apply the deformation and compute the shading                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static int RenderShaderDoublePhase(IPObjectStruct *PObj)
{
    int err, TexId, 
	ViewPort[4] = { 0, 0, 0, 0 };
    IrtPtType *ConstOffset, GlblAnimValLocal;
    IPObjectStruct *DefObj = SingleStepAnimation(PObj);
    if (DefObj == NULL) {
	DefObj = (IPObjectStruct*) AttrGetObjectPtrAttrib(PObj, "_ffd_def_obj");
    }

    IGCheckSysStatus(DefObj, 0);

    if (IGGlblAnimation) {
	IGGetAnimationFactors(PObj, IGAnimation.RunTime);
	if (AttrGetObjectIntAttrib(PObj, "_anim_samples") > 0)
	    PreProcessAnimation(PObj);
    }

    GlblAnimValLocal[0] = GlblAnimVal[0];
    GlblAnimValLocal[1] = GlblAnimVal[1];
    GlblAnimValLocal[2] = GlblAnimVal[2];
    
    ConstOffset = AttrGetObjectPtrAttrib(PObj, "_const_samples");
    if (ConstOffset) {
	GlblAnimValLocal[0] += (*ConstOffset)[0];
	GlblAnimValLocal[1] += (*ConstOffset)[1];
	GlblAnimValLocal[2] += (*ConstOffset)[2];
    }

    /* Start render to off screen buffer. */
#ifdef USE_PBUFFER
    MakePBufferCurrent(0);
#else
    FrameBufferBeginRender(GlblFBuffer);
    /* If we are using FBO we need to change the viewport to the FBO size. */
    glGetIntegerv(GL_VIEWPORT, ViewPort);
    glViewport(0, 0, FRAMEBUFFER_SIZE, FRAMEBUFFER_SIZE);
#endif

    glClearColor(0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* Enable phase1 vertex shader and pixel shader. */
    cgGLEnableProfile(GlblVertexProfile);
    CheckCgError();
    cgGLBindProgram(GlblFfdVertexProgramPhase1);
    CheckCgError();

    cgGLEnableProfile(GlblFragmentProfile);
    CheckCgError();
    cgGLBindProgram(GlblFfdFragmentProgram);
    CheckCgError();

    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "vIn.position",
			   GlblCGParamDoublePhase1.Position);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "TvSpan",
			   GlblCGParamDoublePhase1.GlblTvSpan);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "Range",
			   GlblCGParamDoublePhase1.Range);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "MinLimit",
			   GlblCGParamDoublePhase1.MinLimit);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "AnimVal",
			   GlblCGParamDoublePhase1.GlblAnimVal);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "ModelViewProj",
			   GlblCGParamDoublePhase1.ModelViewProjMatrix);
    CG_GET_NAMED_PARAMETER(GlblFfdVertexProgramPhase1, "Texture",
			   GlblCGParamDoublePhase1.Texture);
    CG_GET_NAMED_PARAMETER(GlblFfdFragmentProgram, "texture",
			   GlblCGParamDoublePhase1.TexObject);

    TexId = AttrGetObjectIntAttrib(PObj, "_ffd_tex_id");

    glActiveTextureARB(GL_TEXTURE0_ARB);  
    glBindTexture(GL_TEXTURE_2D, TexId);
    cgGLSetTextureParameter(GlblCGParamDoublePhase1.TexObject, TexId);	
    CheckCgError();
    cgGLEnableTextureParameter(GlblCGParamDoublePhase1.TexObject);
    CG_GL_SET_PARAMETER_2D(GlblCGParamDoublePhase1.TexDim,
			   1.0 / GlblFfdTexDimV, 1.0 / GlblFfdTexDimU);

    glActiveTextureARB(GL_TEXTURE1_ARB); 
    glBindTexture(GL_TEXTURE_2D, GlblVertexPhase1TextureHandle);
    cgGLSetTextureParameter(GlblCGParamDoublePhase1.Texture,
			    GlblVertexPhase1TextureHandle);
    CheckCgError();
    cgGLEnableTextureParameter(GlblCGParamDoublePhase1.Texture);
    CheckCgError();

    /* Set globals for this model. */  
    GlblTv = PObj->U.Trivars;
    GlblMinParam[0] = GlblTv -> UOrder - 1;
    GlblMinParam[1] = GlblTv -> VOrder - 1;
    GlblMinParam[2] = GlblTv -> WOrder - 1;
    GlblRangeParam[0] =  GlblTv -> ULength - GlblMinParam[0];
    GlblRangeParam[1] =  GlblTv -> VLength - GlblMinParam[1];
    GlblRangeParam[2] =  GlblTv -> WLength - GlblMinParam[2];
   
    CG_GL_SET_PARAMETER_3F(GlblCGParamDoublePhase1.GlblTvSpan,
			   (GLfloat)GlblTvSpan[0],
			   (GLfloat)GlblTvSpan[1],
			   (GLfloat)GlblTvSpan[2]);
    
    CG_GL_SET_PARAMETER_3F(GlblCGParamDoublePhase1.Range,
			   (GLfloat)GlblRangeParam[0],
			   (GLfloat)GlblRangeParam[1],
			   (GLfloat)GlblRangeParam[2]);

    CG_GL_SET_PARAMETER_3F(GlblCGParamDoublePhase1.MinLimit,
			   (GLfloat)GlblMinParam[0],
			   (GLfloat)GlblMinParam[1],
			   (GLfloat)GlblMinParam[2]);

    CG_GL_SET_PARAMETER_3F(GlblCGParamDoublePhase1.GlblAnimVal,
			   (GLfloat)GlblAnimValLocal[0],
			   (GLfloat)GlblAnimValLocal[1],
			   (GLfloat)GlblAnimValLocal[2]);
#ifdef DEBUG_CG_FFD
    printf("Anim val = [%lf %lf %lf]\n", GlblAnimValLocal[0], 
	GlblAnimValLocal[1], GlblAnimValLocal[2]);
#endif
    CG_GL_SET_PARAMETER_3F(GlblCGParamDoublePhase1.TrivDim,
			   (GLfloat)GlblTv -> ULength,
			   (GLfloat)GlblTv -> VLength,
			   (GLfloat)GlblTv -> WLength);

    GlblNormalCalcMethod = AttrGetObjectIntAttrib(PObj, "_normal_method");
    err = glGetError();
    DrawModelAsGrid(DefObj);
    err = glGetError();
    cgGLDisableTextureParameter(GlblCGParamDoublePhase1.TexObject);
    cgGLDisableTextureParameter(GlblCGParamDoublePhase1.Texture);
    glDisable(GL_TEXTURE_2D);
    cgGLDisableProfile(GlblVertexProfile);
    CheckCgError();
    cgGLDisableProfile(GlblFragmentProfile);
    CheckCgError();

    /* Finish 1st phase render. */
#ifdef USE_PBUFFER
    MakeCurrent(0); 
#else
    FrameBufferEndRender(GlblFBuffer);
    /* Change back to the old Viewport if using FBO. */
    glViewport(ViewPort[0], ViewPort[1], ViewPort[2], ViewPort[3]);
#endif

    ApplyDeformedObjectRender(DefObj);
    err = glGetError();
    IncrementInstanceCounter(PObj);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Apply the deformed vertex positions that are stored.		     *
*   The second pass will						     *
*   apply the deformation and compute the shading                            *
*   The function is called after the whole scene finished drawing            *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static int ApplyDeformedObjectRender(IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA int NumPolys;
    GLuint ModelDisplayList;
    IGVertexHandleExtraFuncType OldFunc;

    /* Bind the texture of phase 1. */
#ifdef USE_PBUFFER
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, PBuffer[0].PBufferTexId);
    wglBindTexImageARB(PBuffer[0].HBuffer, WGL_FRONT_LEFT_ARB);
    glEnable(GL_TEXTURE_2D);
#else
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, FrameBufferGetTexture(GlblFBuffer));
    glEnable(GL_TEXTURE_2D);
#endif

    cgGLEnableProfile(GlblVertexProfile);
    CheckCgError();
    cgGLBindProgram(GlblFfdVertexProgramPhase2);
    CheckCgError();

    cgGLEnableProfile(GlblFragmentProfile);
    CheckCgError();
    cgGLBindProgram(GlblFfdFragmentProgramPhase2);
    CheckCgError();

    UpdateShadingParameters(2);
    
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamDoublePhase2.ModelViewProjMatrix,
    CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamDoublePhase2.ModelViewMatrix,
				     CG_GL_MODELVIEW_MATRIX,
				     CG_GL_MATRIX_IDENTITY);

    CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.FlippedNormals,
			   IGGlblFlipNormalOrient ? 1.0f : -1.0f);
    CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.NormalMethod,
			   (float) GlblNormalCalcMethod);
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamDoublePhase2Fragment.ModelViewMatrix,
				     CG_GL_MODELVIEW_MATRIX,
				     CG_GL_MATRIX_IDENTITY);
    CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.FlippedNormals,
			   IGGlblFlipNormalOrient ? 1.0f : -1.0f);
    CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.NormalMethod,
			   (float) GlblNormalCalcMethod);

    OldFunc = IGSetHandleVertexProcessingFunc(IGCgVertexHandlingSecondPass); 
    GlblVertexCount = 0;
    
    SetGlbGlblObjColor(PObj);
    /* Render loop - for this version we have to traverse here. Recursive    */
    /* call after attr set - force normal render accept for attr setting.    */
    if ((ModelDisplayList = 
	AttrGetObjectIntAttrib(PObj,
			       "_ffd_model_disp_lst")) != IP_ATTR_BAD_INT) {
        glCallList(ModelDisplayList);
	IGGlblNumPolys += AttrGetObjectIntAttrib(PObj, "_num_polys");
    }
    else {
	GLuint
	    texId = 0;

	NumPolys = IGGlblNumPolys;
	ModelDisplayList  = glGenLists(1);
	AttrSetObjectIntAttrib(PObj, "_ffd_model_disp_lst", ModelDisplayList);
	glNewList(ModelDisplayList, GL_COMPILE);
	IGDisplayObject(PObj);
	glEndList();
	NumPolys = IGGlblNumPolys - NumPolys;
	AttrSetObjectIntAttrib(PObj, "_num_polys", NumPolys);    
    }

    /* Unbind the texture. */
#ifdef USE_PBUFFER
    wglReleaseTexImageARB(PBuffer[0].HBuffer, WGL_FRONT_LEFT_ARB);
#else
    glActiveTextureARB(GL_TEXTURE0_ARB); 
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
#endif

    cgGLDisableProfile(GlblVertexProfile);
    cgGLDisableProfile(GlblFragmentProfile);

    /* Reset the parameters for the second pass. */  
    GlblCounterX = 0;
    GlblCounterY = 1;

    IGSetHandleVertexProcessingFunc(NULL);
    return TRUE;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   In the case where an object has animation-steps > 0 this mean that there *
* is an active irit animation on the object, the animation is processed and  *
* the objects are stored as a list of display lists			     *
* PARAMETERS:                                                                *
*   Dummy object for rendering                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   TRUE for success, FALSE otherwise					     *
*****************************************************************************/
static int PreProcessAnimation(IPObjectStruct * PObj)
{
    IPObjectStruct *DefObj, *AnimObj,
	*AnimObjList = NULL;
    IrtPtType CenterFirst, CenterLast, *AnimOffset;
    GMAnimationStruct Animation;
    IrtRType AnimTime;
    int i, 
	AnimSteps = 0,
	NumInstances = 1;
    
    AnimSteps = AttrGetObjectIntAttrib(PObj, "_anim_samples");
    DefObj = AttrGetObjectPtrAttrib(PObj, "_ffd_def_obj");
    if (DefObj == NULL)
	return FALSE;
    
    if ((AnimObjList = (IPObjectStruct*) 
	AttrGetPtrAttrib(PObj -> Attr, "_anim_object_list")) == NULL) {
	
	GMAnimFindAnimationTime(&Animation, DefObj);
	AnimTime = Animation.FinalT - Animation.StartT;
	if (IRIT_APX_UEQ(AnimTime, 0))
	    return FALSE;
	AnimOffset = IritMalloc(sizeof(IrtPtType));
	(*AnimOffset)[0] = (*AnimOffset)[1] = (*AnimOffset)[2] = 0;
	for (i = 0; i < AnimSteps; i++) {
	    if (AnimSteps > 0) {
    		GMBBBboxStruct *BBox;
		AnimObj = GMAnimEvalObjAtTime((float) i * AnimTime / 
					                  AnimSteps, DefObj);
#ifdef DEBUG_CG_FFD
		printf("load animation sample - %d\n", i);
#endif
		if (i == 0) {
		    BBox = GMBBComputeBboxObject(AnimObj);
		    CenterFirst[0] = (BBox -> Max[0] + BBox -> Min[0]) / 2;
		    CenterFirst[1] = (BBox -> Max[1] + BBox -> Min[1]) / 2;
		    CenterFirst[2] = (BBox -> Max[2] + BBox -> Min[2]) / 2;
		}
		else if (i == AnimSteps - 1) {
		    BBox = GMBBComputeBboxObject(AnimObj);
		    CenterLast[0] = (BBox -> Max[0] + BBox -> Min[0]) / 2;
		    CenterLast[1] = (BBox -> Max[1] + BBox -> Min[1]) / 2;
		    CenterLast[2] = (BBox -> Max[2] + BBox -> Min[2]) / 2;
		    (*AnimOffset)[0] =  CenterLast[0] - CenterFirst[0];
		    (*AnimOffset)[1] =  CenterLast[1] - CenterFirst[1];
		    (*AnimOffset)[2] =  CenterLast[2] - CenterFirst[2];
		}
		AnimObj -> Pnext = NULL;
		AnimObjList = IPAppendObjLists(AnimObjList, AnimObj);
	    }
	}
	if ((*AnimOffset)[0] > (*AnimOffset)[1] && 
	    (*AnimOffset)[0] > (*AnimOffset)[2]) {
	    (*AnimOffset)[1] = 0; (*AnimOffset)[2] =0;
	}
	else if ((*AnimOffset)[1] > (*AnimOffset)[0] && 
	    (*AnimOffset)[1] > (*AnimOffset)[2]) {
	    (*AnimOffset)[0] = 0; (*AnimOffset)[2] =0;
	}
	else if ((*AnimOffset)[2] > (*AnimOffset)[0] &&
	    (*AnimOffset)[2] > (*AnimOffset)[1]) {
	    (*AnimOffset)[0] = 0; (*AnimOffset)[1] =0;
	}
	AttrSetPtrAttrib(&PObj -> Attr, "_anim_object_list", AnimObjList);
	AttrSetPtrAttrib(&PObj -> Attr, "_anim_offset", AnimOffset);

    }
    return TRUE;	
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given an object get the next instance from the animation series.         *
*						   			     *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct: 			    				     *
*****************************************************************************/
static IPObjectStruct *SingleStepAnimation(IPObjectStruct * PObj)
{
    IPObjectStruct *AnimObjList;
    IrtPtType *AnimOffset;
    int StepDiv, StepMod, StepCounter, i, AnimSteps, NumInstances;
    
    if ((AnimSteps = AttrGetObjectIntAttrib(PObj, "_anim_samples"))
	                             == IP_ATTR_BAD_INT || AnimSteps <= 0) {
	return NULL;
    }

    if ((AnimObjList = AttrGetObjectPtrAttrib(PObj,
					      "_anim_object_list")) == NULL) {
	return NULL;
    }

    if ((AnimOffset = AttrGetObjectPtrAttrib(PObj, "_anim_offset")) == NULL) {
	return NULL;
    }
    
    StepCounter = (int) (((IGAnimation.RunTime )) / IGAnimation.Dt);
    StepMod = StepCounter % AnimSteps;
    StepDiv = StepCounter / AnimSteps;
    NumInstances = (int) (((IGAnimation.FinalT - IGAnimation.StartT)
					       / IGAnimation.Dt) / AnimSteps);
    i = 0;
    while (AnimObjList) {
	if (i++ == StepMod) {   
	    GlblAnimVal[0] = ((*AnimOffset)[0]) * (StepDiv);
	    GlblAnimVal[1] = ((*AnimOffset)[1]) * (StepDiv);
	    GlblAnimVal[2] = ((*AnimOffset)[2]) * (StepDiv);
	    
	    return AnimObjList;
	}
	AnimObjList = AnimObjList -> Pnext;
    }
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Increment the animation instnace counter. If an object has stored series *
*   of animation objects, the function will increment th counter	     *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	    			    				     *
*****************************************************************************/
static void IncrementInstanceCounter(IPObjectStruct *PObj)
{
    int AnimSteps, InstanceCounter, NumInstances;
    IPObjectStruct *DefObj;
    IrtRType RunTime;
    DefObj = SingleStepAnimation(PObj);

    if (DefObj == NULL)
	return;

    if ((AnimSteps = 
	AttrGetObjectIntAttrib(PObj, "_anim_samples")) == IP_ATTR_BAD_INT)
	return;

    if ((RunTime = 
	 AttrGetObjectRealAttrib(DefObj, "_anim_runtime")) == IP_ATTR_BAD_REAL)
	AttrSetObjectRealAttrib(DefObj, "_anim_runtime", IGAnimation.RunTime);
    else if (IGAnimation.RunTime == 
	AttrGetObjectRealAttrib(DefObj, "_anim_runtime"))
	return;
    else
        AttrSetObjectRealAttrib(DefObj, "_anim_runtime", IGAnimation.RunTime);

    NumInstances = (int) (((IGAnimation.FinalT - IGAnimation.StartT)
	                                        / IGAnimation.Dt) / AnimSteps);
    if ((InstanceCounter = 
	    AttrGetObjectIntAttrib(DefObj, "_anim_phase")) == IP_ATTR_BAD_INT)
        AttrSetObjectIntAttrib(DefObj, "_anim_phase", 0);
    else if (InstanceCounter < NumInstances)
	AttrSetObjectIntAttrib(DefObj, "_anim_phase", InstanceCounter + 1);
    else
	AttrSetObjectIntAttrib(DefObj, "_anim_phase",0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calls display list for the second phase of the FFD algorithm	     *
*						   			     *
* PARAMETERS:                                                                *
*   PObj: Object to Draw						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	    			    				     *
*****************************************************************************/
static void DrawModelAsGrid(IPObjectStruct *PObj)
{
    int Err;
    GLuint DisplayListHandle;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    gluLookAt(0, 0, 0,
              0, 0, -10,
              0, 1, 0);
	
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblCGParamDoublePhase1.ModelViewProjMatrix,
				     CG_GL_MODELVIEW_PROJECTION_MATRIX,
				     CG_GL_MATRIX_IDENTITY);
    glPointSize(POINT_SIZE);
    
    if ((DisplayListHandle = AttrGetObjectIntAttrib(PObj,
						    "_ffd_grid_disp_lst"))
							 != IP_ATTR_BAD_INT) {
	glCallList(DisplayListHandle);
    }
    else {
        IGVertexHandleExtraFuncType OldFunc;
	int OldDrawStyle = IGGlblDrawStyle;

	IGGlblDrawStyle = IG_STATE_DRAW_STYLE_POINTS;
	OldFunc = IGSetHandleVertexProcessingFunc(IGCgVertexHanlingFirstPass); 

	DisplayListHandle  = glGenLists(1);
	AttrSetObjectIntAttrib(PObj, "_ffd_grid_disp_lst", DisplayListHandle);
	glNewList(DisplayListHandle, GL_COMPILE);
    	IGDisplayObject(PObj);
	glEndList();
	
	/* Restore state. */
	IGGlblDrawStyle = OldDrawStyle;
	IGSetHandleVertexProcessingFunc(OldFunc); 
    }
	
    glPopMatrix();
    Err = glGetError();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    Err = glGetError();
    glMatrixMode(GL_MODELVIEW);
    Err = glGetError();
    glPointSize(1);
    Err = glGetError();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if ARB is supported on Gpu					     *
* PARAMETERS:                                                                *
*   expresion: ARB expresion to check				    	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TRUE if supported, FALSE otherwise.					     *
*****************************************************************************/
static int IsArbExtensionSupported(const char *Extension)
{
    HDC hdc = wglGetCurrentDC();
    const char
	*Extensions = NULL;
    char *Start, *Where, *Terminator;

    /* Get function pointer. */
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB=
	(PFNWGLGETEXTENSIONSSTRINGARBPROC)
	wglGetProcAddress("wglGetExtensionsStringARB");
    if (wglGetExtensionsStringARB == NULL)
        return FALSE;
    Extensions = wglGetExtensionsStringARB(hdc);
    Start = (char*) Extensions;
    for ( ; ; ) {
        Where = strstr((const char *) Start, Extension);
        if (!Where)
	    break;
        Terminator = Where + strlen(Extension);
        if (Where == Start || *(Where - 1) == ' ')
	    if (*Terminator == ' ' || *Terminator == '\0')
	        return TRUE;
        Start = Terminator;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get the color for index on a local context.	   			     *
*						   			     *
* PARAMETERS:                                                                *
*  color:  color index,							     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	 			    				     *
*****************************************************************************/
static void SetColorIndex(int color)
{
    IRIT_STATIC_DATA short Colors[IG_MAX_COLOR + 1][3] =
    {
	{ 0,   0,   0   },  /* 0. BLACK. */
	{ 0,   0,   170 },  /* 1. BLUE. */
	{ 0,   170, 0   },  /* 2. GREEN. */
	{ 0,   170, 170 },  /* 3. GlblCounterYAN. */
	{ 170, 0,   0   },  /* 4. RED. */
	{ 170, 0,   170 },  /* 5. MAGENTA. */
	{ 170, 170, 0   },  /* 6. BROWN. */
	{ 170, 170, 170 },  /* 7. LIGHTGREY. */
	{ 85,  85,  85  },  /* 8. DARKGRAY. */
	{ 85,  85,  255 },  /* 9. LIGHTBLUE. */
	{ 85,  255, 85  },  /* 10. LIGHTGREEN. */
	{ 85,  255, 255 },  /* 11. LIGHTGlblCounterYAN. */
	{ 255, 85,  85  },  /* 12. LIGHTRED. */
	{ 255, 85,  255 },  /* 13. LIGHTMAGENTA. */
	{ 255, 255, 85  },  /* 14. YELLOW. */
	{ 255, 255, 255 }   /* 15. WHITE. */
    };

    if (color < 0 || color > IG_MAX_COLOR)
        color = IG_IRIT_WHITE;

    GlblObjColor[0] = Colors[color][0];
    GlblObjColor[1] = Colors[color][1];
    GlblObjColor[2] = Colors[color][2];
    GlblObjColor[3] = 255;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the global obj color in a local context.  			     *
*						   			     *
* PARAMETERS:                                                                *
*   PObj:  Object to set its color.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.	 			    				     *
*****************************************************************************/
static void SetGlbGlblObjColor(IPObjectStruct *PObj)
{
    int c, 
	Color[4] = { 255, 255, 255, 255 };

    if (AttrGetObjectRGBColor(PObj, &Color[0], &Color[1], &Color[2])) {
	 memcpy(GlblObjColor, Color, sizeof(GlblObjColor));
    }
    else if ((c = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR) {
	SetColorIndex(c);
    }
    else {
	/* Use white as default color. */
        SetColorIndex(IG_IRIT_WHITE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update parameters for the shader.					     *
*						   			     *
* PARAMETERS:                                                                *
*   Algorithm: Which algorithm to use.				    	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void UpdateShadingParameters(int Algorithm)
{
    GlblNrmlDir =IGGlblFlipNormalOrient == TRUE ? -1 : 1;

    if (Algorithm == 1) {
	CG_GL_SET_PARAMETER_1F(GlblCGParamSinglePhase.AmbientFactor,
	    IGShadeParam.LightAmbient[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamSinglePhase.DiffuseFactor, 
	    IGShadeParam.LightDiffuse[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamSinglePhase.SpecularFactor, 
	    IGShadeParam.LightSpecular[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamSinglePhase.Shininess,
	    IGShadeParam.Shininess);
	CG_GL_SET_PARAMETER_4F(GlblCGParamSinglePhase.LightPos0 ,
					IGShadeParam.LightPos[0][0],
					IGShadeParam.LightPos[0][1],
					IGShadeParam.LightPos[0][2],
					IGShadeParam.LightPos[0][3]);
	CG_GL_SET_PARAMETER_4F(GlblCGParamSinglePhase.LightPos1 ,
					IGShadeParam.LightPos[1][0],
					IGShadeParam.LightPos[1][1],
					IGShadeParam.LightPos[1][2],
					IGShadeParam.LightPos[1][3]);
    }
    else if (Algorithm == 2) {
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.AmbientFactor, 
	    IGShadeParam.LightAmbient[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.DiffuseFactor, 
	    IGShadeParam.LightDiffuse[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.SpecularFactor,
	    IGShadeParam.LightSpecular[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2.Shininess,
	    IGShadeParam.Shininess); 
	CG_GL_SET_PARAMETER_4F(GlblCGParamDoublePhase2.LightPos0 ,
					IGShadeParam.LightPos[0][0],
					IGShadeParam.LightPos[0][1],
					IGShadeParam.LightPos[0][2],
					IGShadeParam.LightPos[0][3]); 
	CG_GL_SET_PARAMETER_4F(GlblCGParamDoublePhase2.LightPos1 ,
					IGShadeParam.LightPos[1][0],
					IGShadeParam.LightPos[1][1],
					IGShadeParam.LightPos[1][2],
					IGShadeParam.LightPos[1][3]);

	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.AmbientFactor,
	    IGShadeParam.LightAmbient[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.DiffuseFactor,
	    IGShadeParam.LightDiffuse[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.SpecularFactor,
	    IGShadeParam.LightSpecular[0]);
	CG_GL_SET_PARAMETER_1F(GlblCGParamDoublePhase2Fragment.Shininess,
	    IGShadeParam.Shininess); 
	CG_GL_SET_PARAMETER_4F(GlblCGParamDoublePhase2Fragment.LightPos0 ,
					IGShadeParam.LightPos[0][0],
					IGShadeParam.LightPos[0][1],
					IGShadeParam.LightPos[0][2],
					IGShadeParam.LightPos[0][3]); 
	CG_GL_SET_PARAMETER_4F(GlblCGParamDoublePhase2Fragment.LightPos1 ,
					IGShadeParam.LightPos[1][0],
					IGShadeParam.LightPos[1][1],
					IGShadeParam.LightPos[1][2],
					IGShadeParam.LightPos[1][3]);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if the system global variables have changed, and update Open GL    *
*   tiles display lists if nesseccery.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: Objects needs to recompute.	                                     *
*   ForceRecompute: Force Recompute without checking the status		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: 	                                                             *
*****************************************************************************/
static int IGCheckSysStatus(IPObjectStruct *PObj, int ForceRecompute)
{
    IRIT_STATIC_DATA int
	DrawStyle, DrawSurfacePoly, DrawSurfaceWire, ShadingModel, FourPerFlat,
	FlipNormalOrient, PolygonOptiApprox, CountNumPolys, CurrState,
        State = 0,
        Init = 1;
    IRIT_STATIC_DATA IrtRType PlgnFineness;
    IPObjectStruct *DefObj;

    if ((CurrState = AttrGetObjectIntAttrib(PObj, "State")) != State ||
	(CountNumPolys != IGGlblCountNumPolys) ||
	(DrawStyle != IGGlblDrawStyle) ||
	(DrawSurfacePoly != IGGlblDrawSurfacePoly) ||
	(DrawSurfaceWire != IGGlblDrawSurfaceWire) ||
	(ShadingModel != IGGlblShadingModel) ||
	(FourPerFlat != IGGlblFourPerFlat) ||
	(FlipNormalOrient != IGGlblFlipNormalOrient) ||
	(PolygonOptiApprox != IGGlblPolygonOptiApprox) ||
	(PlgnFineness != IGGlblPlgnFineness) ||
	ForceRecompute) {
	if (CurrState == State || Init) {
	    State ^= 1;
	    Init = 0;
	    if ((PolygonOptiApprox != IGGlblPolygonOptiApprox) || 
		(PlgnFineness != IGGlblPlgnFineness)) {
		RecomputeCacheGeom(PObj, 1, 1, 1, 1);
	    }
	    IGActiveFreeNamedAttribute(PObj, "State");
	    AttrSetObjectIntAttrib(PObj, "State", State);
	    CountNumPolys = IGGlblCountNumPolys;
	    DrawStyle = IGGlblDrawStyle;
	    DrawSurfacePoly = IGGlblDrawSurfacePoly;
	    DrawSurfaceWire = IGGlblDrawSurfaceWire;
	    ShadingModel = IGGlblShadingModel;
	    FourPerFlat = IGGlblFourPerFlat;
	    FlipNormalOrient = IGGlblFlipNormalOrient;
	    PolygonOptiApprox = IGGlblPolygonOptiApprox;
	    PlgnFineness = IGGlblPlgnFineness;
	    RecomputeLists(PObj);	
	    return TRUE;
	}
	else {
	    IGActiveFreeNamedAttribute(PObj, "State");
	    AttrSetObjectIntAttrib(PObj, "State", State);
	    RecomputeLists(PObj);
	    if ((DefObj = AttrGetObjectPtrAttrib(PObj,
						 "_ffd_def_obj")) != NULL)
		RecomputeCacheGeom(DefObj, 1, 1, 1, 1);
	}
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deletes FFD attributes, frees open gl lists.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: Objects needs to recompute.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void				   	                             *
*****************************************************************************/
static void RecomputeLists(IPObjectStruct *PObj)
{
    GLuint DisplayListHandle;

    if ((DisplayListHandle = AttrGetObjectIntAttrib(PObj,
						    "_ffd_model_disp_lst"))
							!= IP_ATTR_BAD_INT) {
	glDeleteLists(DisplayListHandle, 1);
	AttrFreeOneAttribute(&PObj->Attr, "_ffd_model_disp_lst");
    }

    if ((DisplayListHandle = AttrGetObjectIntAttrib(PObj,
						    "_ffd_grid_disp_lst"))
							!= IP_ATTR_BAD_INT) {
	glDeleteLists(DisplayListHandle, 1);
	AttrFreeOneAttribute(&PObj -> Attr, "_ffd_grid_disp_lst");
    }	    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   recompute a tesselation for a freeform object on state changes	     *
* PARAMETERS:                                                                *
*   PObj: Objects needs to recompute.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void RecomputeCacheGeom(IPObjectStruct * PObj,
			       int FreePolygons,
			       int FreeIsolines,
			       int FreeSketches,
			       int FreeCtlMesh)
{
    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int i = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    RecomputeCacheGeom(PTmp, FreePolygons,
			       FreeIsolines, FreeSketches, FreeCtlMesh);
    }
    else if (IP_IS_FFGEOM_OBJ(PObj))
	IGActiveFreePolyIsoAttribute(PObj, FreePolygons,
				     FreeIsolines, FreeSketches, FreeCtlMesh);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   An interface to manipulate the position of a selected group of control   *
*   points. The control points are visually represented as point objects     *
*   with index attributes.						     *
* PARAMETERS:                                                                *
*   PObj: Object to manipulate		                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.								     *
*****************************************************************************/
static void SetControlPoint(IPObjectStruct *PObj)
{
    int TexId, ObjCounter;
    GLfloat
	NewPoint[4] = { 0 };
    TrivTVStruct
	*Tv = NULL;
    IPObjectStruct *TvObj, *TObj;

    TexId = AttrGetObjectIntAttrib(PObj, "_ffd_tex_id");
    TvObj = (IPObjectStruct*)AttrGetObjectPtrAttrib(PObj, "_ffd_triv");
    Tv = TvObj -> U.Trivars;
    if (Tv == NULL || IP_ATTR_BAD_INT == TexId)
	return;

    for (ObjCounter = 0; ObjCounter < IGObjManipNumActiveObjs; ObjCounter++) {
	int i, j, k, x, y;

	TObj = IGObjManipCurrentObjs[ObjCounter];
	i = AttrGetObjectIntAttrib(TObj, "TV_I");
	j = AttrGetObjectIntAttrib(TObj, "TV_J");
	k = AttrGetObjectIntAttrib(TObj, "TV_K");
	if (i == IP_ATTR_BAD_INT || 
	    j == IP_ATTR_BAD_INT || 
	    k == IP_ATTR_BAD_INT)
	    continue;
	x = j;
	y = k * Tv -> ULength + i;
	NewPoint[0] = (GLfloat)TObj -> U.Pt[0];
	NewPoint[1] = (GLfloat)TObj -> U.Pt[1];
	NewPoint[2] = (GLfloat)TObj -> U.Pt[2];
	glBindTexture(GL_TEXTURE_2D, TexId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, 
	    GL_FLOAT, NewPoint);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Copy the values of deformed positions from the PBuffer to an array	     *
*   Compute the bounding box and regenerate the trivariate		     *
* PARAMETERS:                                                                *
*   XLen, YLen the number of pixels in the x,y-direction		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static void ApplyDeformedVertices(IPObjectStruct *PObj,
				  int XLen, 
				  int YLen)
{
    int i, j;
    GLfloat *DefPositions;

    DefPositions = IGGetPBufferToHost(0, XLen, YLen);
    GlblObjBBox[1][0] = GlblObjBBox[0][0] = DefPositions[0];
    GlblObjBBox[1][1] = GlblObjBBox[0][1] = DefPositions[1];
    GlblObjBBox[1][2] = GlblObjBBox[0][2] = DefPositions[2];
    for (i = 0; i < YLen; i++) {
	for (j = 0; j < XLen; j++) {
	    int idx = 4 * (XLen * i + j);

	    if (GlblObjBBox[0][0] > DefPositions[idx])
		GlblObjBBox[0][0] = DefPositions[idx];
	    if (GlblObjBBox[1][0] < DefPositions[idx])
		GlblObjBBox[1][0] = DefPositions[idx];

	    if (GlblObjBBox[0][1] > DefPositions[idx + 1])
		GlblObjBBox[0][1] = DefPositions[idx + 1];
	    if (GlblObjBBox[1][1] < DefPositions[idx + 1])
		GlblObjBBox[1][1] = DefPositions[idx + 1];

	    if (GlblObjBBox[0][2] > DefPositions[idx + 2])
		GlblObjBBox[0][2] = DefPositions[idx + 2];
	    if (GlblObjBBox[1][2] < DefPositions[idx + 2])
		GlblObjBBox[1][2] = DefPositions[idx + 2];
	}
    }
    IritFree(DefPositions);
    /* Store the new bbox of the deformed object. */
    memcpy(PObj -> BBox, GlblObjBBox, sizeof(GlblObjBBox));
}

#else

/* Dummy functions to link to if no Open GL CG graphics is available. */
int IGFfdDraw(IPObjectStruct *PObj)
{
    return FALSE;
}

#endif /* IRIT_HAVE_OGL_CG_LIB */
