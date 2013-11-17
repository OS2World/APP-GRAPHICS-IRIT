/*****************************************************************************
*  Open GL using CG (programmable hardware) drawing functions.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, January 2005.  *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "iritprsr.h"

#ifdef IRIT_HAVE_OGL_CG_LIB

#include <windows.h>
#include <gl/gl.h>
#include <cg/cg.h>
#include <cg/cgGL.h>
#include <gl/glu.h>
/* This needs to be included into the CG SDK where the NV extensions reside. */
#include <gl/glext.h>
#include "grap_loc.h"
#include "opngl_cg.h"
#include "time.h"

/* Static data for CG shaders. */
IRIT_STATIC_DATA CGcontext
    GlblContext = NULL;
IRIT_STATIC_DATA CGprogram
    GlblVertexProgram = NULL,
    GlblFragmentProgram = NULL;
IRIT_STATIC_DATA CGparameter
    GlblModelViewProjMatrix, GlblModelViewMatrix, GlblMiscMatrix,
    GlblPosition, GlblNormal, GlblTileUV, GlblTexDim, GlblScale,
    GlblConstScale, GlblMorph, GlblOCoord, GlblONormal, GlblSampling,
    GlblAmbient, GlblDiffuse, GlblSpecular, GlblShininess,
    GlblLightSource0, GlblLightSource1, GlblVCompatiblePosition,
    GlblVCompatibleNormal, GlblVCompatibleColor, GlblVP1, GlblVP2,
    GlblCalcNormalsMethod, GlblShaded, GlblVShaded;
IRIT_STATIC_DATA IrtBboxType GlblPosBBox, GlblUvBBox, GlblNrmlBBox;
IRIT_STATIC_DATA CGprofile VertexProfile, FragmentProfile;
IRIT_STATIC_DATA PFNGLACTIVETEXTUREARBPROC
    glActiveTextureARB = NULL;

void IGDisplayObject(IPObjectStruct *PObj);
static int IGCreateDdmDisplayList(IPObjectStruct *PObj);
static void IGCheckSysStatus();
static int IGSetVertexTexture(IPObjectStruct *PObj, IPObjectStruct *PObj2);
static int IGSetTextureCoord(IPVertexStruct *V);
static int IGLoadShaders(IPObjectStruct *PObj);
static void IGCreateTextureFromObject(IPObjectStruct *PObj, 
			       int NumUSamples, 
			       int NumVSamples);
static void PowerOfTwoTexture (int* x);
static int IrtParseDTextureString(const char *DTexture,
				  char *FName,
				  char *Shader,
				  float *NumTilesX,
				  float *NumTilesY,
				  int *NumSamplesX,
				  int *NumSamplesY,
				  int *ClampX,
				  int *ClampY,
				  float *Scale,
				  int *DrawOriginalObject,
				  int *DdmAnimation,
				  float *CalcNormalsMethod,
				  int *UseMultiTiles);
static int IGInitDTexture(IPObjectStruct *PObj);
static void IGGetAnimFactors(IPObjectStruct *PObj,
			     IrtRType t,
			     IrtRType AnimFactors[3]);
static int IGVertexBoundsCompute(IPVertexStruct *V);
static int IGVertexPosAsColor(IPVertexStruct *V);
static int IGVertexNrmlAsColor(IPVertexStruct *V);
static int ConstructTexturesFromPolyModel(IPObjectStruct * PObj,
				    int NumUSamples,
				    int NumVSamples);
static void ApplyDiffusion(GLfloat *image,
		    int XLen,
		    int YLen);
static void DecideNonTiles(IPObjectStruct *PObj,
		    float *map,
		    int XLen,
		    int YLen);

#define RECT_TILE	0
#define HEX_TILE	1
#define TRIG_TILE	2
#define TRIG_TILE_REV	3

/*****************************************************************************
* DESCRIPTION:                                                               *
* Search Irit object list for objects and call IGDisplayObject for real      *
* non list objects.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object (List) to draw.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.								     *
*****************************************************************************/
void IGDisplayObject(IPObjectStruct *PObj)
{
    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int i = 0;
	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
		IGDisplayObject(PTmp);
    }
    else 
	IGDrawObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Create open gl display list from a single object and add the handle for    * 
* the display list to the object's attribute                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if the display list is created, FALSE otherwise.         *
*****************************************************************************/
static int IGCreateDdmDisplayList(IPObjectStruct *PObj)
{
    int i, j, DdmAnimation,
	NumFiles,
	n = 0;
    GLuint Ddm,
	**DdmHandleList = NULL;
    const char *DTextureFile;
    char DTexture[IRIT_LINE_LEN_LONG];
    IPObjectStruct **ObjPtrList = NULL;
    GMAnimationStruct Animation;
    IrtRType AnimTime;
    
    if ((DdmHandleList = AttrGetObjectPtrAttrib(PObj, "_DdmHandleList")) != NULL
	    && DdmHandleList[0][0] > 0)
	return TRUE;
    if ((DTextureFile = AttrGetObjectStrAttrib(PObj, "DTextureFile")) == NULL) 
	return FALSE;
    else {
	GMBBBboxStruct *bbox;
	IGSetHandleVertexProcessingFunc(IGSetTextureCoord);
	if ((NumFiles = AttrGetObjectIntAttrib(PObj, "NumFiles")) 
	    == IP_ATTR_BAD_INT){
	    NumFiles = 0;
	    AttrSetObjectIntAttrib(PObj, "NumFiles", 0);
	}
	ObjPtrList = AttrGetObjectPtrAttrib(PObj, "_ObjPtrList");
	if (ObjPtrList == NULL){
	    ObjPtrList = IritMalloc(NumFiles + 1 * sizeof(IPObjectStruct*));
	    for (i = 0; i < NumFiles + 1; i++)
		ObjPtrList[i] = NULL;
	    AttrSetObjectPtrAttrib(PObj, "_ObjPtrList", ObjPtrList);
	}

	if (ObjPtrList[0] == NULL){
	    ObjPtrList[0] = IPGetDataFiles(&DTextureFile, 1, TRUE, TRUE);
	    for (i = 1; i < NumFiles + 1; i++){
	        const char *File;
		char TextureFile[14] = "DTextureFile ";

		TextureFile[12] = i + '0';
		File = AttrGetObjectStrAttrib(PObj, TextureFile);
		if (File != NULL){
		    ObjPtrList[i] = IPGetDataFiles(&File, 1, TRUE, TRUE);
		}
		else
		    return FALSE;
		}
	}
	if (ObjPtrList[0] != NULL) {
	    IPObjectStruct *PTemp;

	    if (AttrGetObjectIntAttrib(ObjPtrList[0], "HEX_TILE")
						    != IP_ATTR_BAD_INT)
		AttrSetObjectIntAttrib(PObj, "TILE_TYPE", 1);
	    else if (AttrGetObjectIntAttrib(ObjPtrList[0], "TRIG_TILE") 
						    != IP_ATTR_BAD_INT)
		AttrSetObjectIntAttrib(PObj, "TILE_TYPE", 2);
	    else if (AttrGetObjectIntAttrib(ObjPtrList[0], "TRIG_TILE_REV")
						    != IP_ATTR_BAD_INT)
		AttrSetObjectIntAttrib(PObj, "TILE_TYPE", 3);

	    if (ObjPtrList[1] != NULL && NumFiles > 0)
		IGSetVertexTexture(ObjPtrList[0], ObjPtrList[1]);

	    DdmAnimation = AttrGetObjectIntAttrib(PObj, "DdmAnimation");
	    if (DdmHandleList == NULL){
		DdmHandleList = IritMalloc((NumFiles + 1) * sizeof(int));
		AttrSetObjectPtrAttrib(PObj, "_DdmHandleList", DdmHandleList);
		for (i = 0; i < (NumFiles + 1); i++)
		    DdmHandleList[i] = IritMalloc(DdmAnimation * sizeof(int));
	    }
	
	    strcpy (DTexture, AttrGetObjectStrAttrib(PObj, "DTexture"));
	    IGActiveFreeNamedAttribute(ObjPtrList[0], "_PolygonsHiRes");
	    PTemp = IGGlblDisplayList;
	    IGGlblDisplayList = ObjPtrList[0];
	    GMAnimFindAnimationTime(&Animation, ObjPtrList[0]);
	    AnimTime = Animation.FinalT - Animation.StartT;

	    for (i = 0; i < NumFiles + 1; i++){
		for (j = 0; j < DdmAnimation; j++) {
		    Ddm = glGenLists(1);
		    DdmHandleList[i][j] = Ddm;
		    glNewList(Ddm, GL_COMPILE);
		    if (DdmAnimation > 1) {
			IPObjectStruct *AnimObj = 
				GMAnimEvalObjAtTime((float) j * AnimTime / 
				DdmAnimation, ObjPtrList[0]);
			IGDisplayObject(AnimObj);
			IPFreeObject(AnimObj);
		    }
		    else {
			IGDisplayObject(ObjPtrList[i]);
		    }
		    glEndList();
		}
	    }
	    IGGlblDisplayList = PTemp;
	    if (AttrGetObjectIntAttrib(ObjPtrList[0], "_ImageTexID") 
		    != IP_ATTR_BAD_INT)
		AttrSetObjectIntAttrib(PObj, "TextureUV",
		    AttrGetObjectIntAttrib(ObjPtrList[0], "_ImageTexID"));
	    if (IGGlblMore){
		if (!IP_HAS_BBOX_OBJ(ObjPtrList[0])){
		    IP_SET_BBOX_OBJ(ObjPtrList[0]);
		    bbox = GMBBComputeBboxObject(ObjPtrList[0]);
		    IRIT_GEN_COPY(&ObjPtrList[0] -> BBox[0][0] ,&(bbox -> Min[0]),
				  sizeof(ObjPtrList[0] -> BBox[0]));
		    IRIT_GEN_COPY(&ObjPtrList[0] -> BBox[1][0] ,&(bbox -> Max[0]),
				  sizeof(ObjPtrList[0] -> BBox[1]));
		    if (bbox->Min[0] < 0 || bbox->Min[1] < 0 ||
			    bbox->Max[0] > 1 || bbox->Max[1] > 1)
		    printf ("Warning: Tile's domain exceeds [0,1]\n");
		}
	    }

	    if (IP_IS_POLY_OBJ(ObjPtrList[0]))
		n = IPPolyListLen(ObjPtrList[0] -> U.Pl);
	    else {
		IPObjectStruct
    		*PTmp = AttrGetObjectObjAttrib(ObjPtrList[0], "_PolygonsHiRes");
		if (PTmp == NULL)
		    PTmp = AttrGetObjectObjAttrib(ObjPtrList[0], "_IsolinesHiRes");
		if (PTmp != NULL)
		    n = IPPolyListLen(PTmp -> U.Pl);
	    }
	    AttrSetObjectIntAttrib(PObj, "DdmNumPolys", n);
	}
	else {
	    IGActiveFreeNamedAttribute(PObj, "DTexture");
	    IGSetHandleVertexProcessingFunc(NULL);
	    return FALSE;
	}
	IGSetHandleVertexProcessingFunc(NULL);
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Recreate Open GL display list from a single object and add the handle    M 
* for the display list to the object's attribute.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Object to update.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGCGDrawDTexture                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGFreeDTexture                                                         M
*****************************************************************************/
void IGCGFreeDTexture(IPObjectStruct *PObj)
{
    int i, j;
    GLuint **DdmHandleList;
    	
    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int i = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    IGCGFreeDTexture(PTmp);
    }

    DdmHandleList = AttrGetObjectPtrAttrib(PObj, "_DdmHandleList");
    if (DdmHandleList != NULL && DdmHandleList[0][0]> 0) {
	for (i = 0; i < AttrGetObjectIntAttrib(PObj, "NumFiles") + 1; i++){
	    for (j = 0; j < AttrGetObjectIntAttrib(PObj, "DdmAnimation"); j++){
		if (DdmHandleList[i][j] > 0) 
		    glDeleteLists(DdmHandleList[i][j], 1); 
		    DdmHandleList[i][j] = 0;
		}
	}
	IGActiveFreeNamedAttribute(PObj, "DdmNumPolys");
	IGCreateDdmDisplayList(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Check if the system global variables have changed, and update Open GL	     *
* tiles display lists if nesseccery.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   None	                                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void					                             *
*****************************************************************************/
static void IGCheckSysStatus(void)
{
    static int
	DrawStyle = -1,
        DrawSurfacePoly = -1,
        DrawSurfaceWire = -1,
        DrawSolid = -1,
        ShadingModel = -1,
        FourPerFlat = -1,
        FlipNormalOrient = -1,
        Init = FALSE;
    IPObjectStruct *PObj;

    if ((DrawStyle != IGGlblDrawStyle) ||
	(DrawSurfacePoly != IGGlblDrawSurfacePoly) ||
	(DrawSurfaceWire != IGGlblDrawSurfaceWire) ||
	(ShadingModel != IGGlblShadingModel) ||
	(FourPerFlat != IGGlblFourPerFlat) ||
	(FlipNormalOrient != IGGlblFlipNormalOrient)) {
        DrawStyle = IGGlblDrawStyle;
	    DrawSurfacePoly = IGGlblDrawSurfacePoly;
	    DrawSurfaceWire = IGGlblDrawSurfaceWire;
	    ShadingModel = IGGlblShadingModel;
	    FourPerFlat = IGGlblFourPerFlat;
	    FlipNormalOrient = IGGlblFlipNormalOrient;

	    PObj = IGGlblDisplayList;
	
	    if (Init) {
		for ( ; PObj != NULL; PObj = PObj -> Pnext) {
		    IGCGFreeDTexture(PObj);
		}
	    }
	Init = TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prepare object's vertex atribute for objects with dtexture2 attribute      *
* the display list to the object's attribute                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Target object - PObj2 Vertex will be added to PObj Vertex Attr*
*   PObj2:     Source object                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE for success, FALSE otherwise.                            *
*****************************************************************************/
static int IGSetVertexTexture(IPObjectStruct *PObj, IPObjectStruct *PObj2)
{
    IPPolygonStruct
	*P1 = PObj -> U.Pl,
	*P2 = PObj2 -> U.Pl;
    
    if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLY_OBJ(PObj2))
	return FALSE;

    for ( ; P1 != NULL, P2 != NULL; P1 = P1 -> Pnext, P2 = P2 -> Pnext) {
	IPVertexStruct
	    *V1 = P1 ->PVertex,
	    *V2 = P2 -> PVertex;

	for ( ; V1 != NULL, V2 != NULL; V1 = V1 -> Pnext, V2 = V2 -> Pnext) {
	    if (IP_HAS_NORMAL_VRTX(V2) ==  FALSE)
	    IRIT_GEN_COPY(V2 -> Normal, P2 -> Plane, 3 * sizeof(IrtRType));
	    AttrSetPtrAttrib(&V1 -> Attr, "_CompatibleVertex", V2);
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Assign compatible vertex to shader's input texture (for polymorph          * 
* animation)								     *
*                                                                            *
* PARAMETERS:                                                                *
*   V: Vertex to handle							     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       FALSE.							     *
*****************************************************************************/
static int IGSetTextureCoord(IPVertexStruct *V)
{
    IPVertexStruct *V2;
    const  char *Str;
    int Color[3], FlipNormal = 1;
    double epsilon = 0.01;
    IrtPtType VP1, VP2, T1, T2, P1, P2,
	X = { 1, 0, 0 },
        Y = { 0, 1, 0 };
     
    if ((V2 = (IPVertexStruct *) AttrGetPtrAttrib(V -> Attr,
					       "_CompatibleVertex")) != NULL) {
	if (!IGGlblFlipNormalOrient)
	    FlipNormal = -1;
	CG_GL_SET_PARAMETER_4F(GlblVCompatiblePosition,
			       (GLfloat) V2 -> Coord[0],
			       (GLfloat) V2 -> Coord[1],
			       (GLfloat) V2 -> Coord[2], 1);

	CG_GL_SET_PARAMETER_4F(GlblVCompatibleNormal,
			       (GLfloat) V2 -> Normal[0] * FlipNormal,
			       (GLfloat) V2 -> Normal[1] * FlipNormal,
			       (GLfloat) V2 -> Normal[2] * FlipNormal, 0);
	if ((Str = AttrGetStrAttrib(V2 -> Attr, "RGB")) != NULL &&
	    sscanf(Str, "%d,%d,%d", &Color[0], &Color[1], &Color[2]) == 3) {
		CG_GL_SET_PARAMETER_4F(GlblVCompatibleColor,
			       (GLubyte) Color[0] / (float)255,
			       (GLubyte) Color[1] / (float)255,
			       (GLubyte) Color[2] / (float)255, 0);
	}
	else 
	    CG_GL_SET_PARAMETER_4F(GlblVCompatibleColor, -1, -1, -1, -1);
    }
    else 
	CG_GL_SET_PARAMETER_4F(GlblVCompatibleColor, -1, -1, -1, -1);

    IRIT_CROSS_PROD(T1, X, V -> Normal);
    IRIT_CROSS_PROD(T2, Y, V -> Normal);
    if (IRIT_PT_LENGTH(T1) > IRIT_PT_LENGTH(T2)) 
	IRIT_PT_COPY (P1, T1);
    else 
	IRIT_PT_COPY (P1, T2);
  
    IRIT_PT_NORMALIZE(P1);
    IRIT_CROSS_PROD(P2, P1, V -> Normal);
    IRIT_PT_NORMALIZE(P2);
    IRIT_PT_SCALE(P1, epsilon);
    IRIT_PT_SCALE(P2, epsilon);
    IRIT_PT_ADD(VP1, V -> Coord, P1);
    IRIT_PT_ADD(VP2, V -> Coord, P2);

    if (!IGGlblFlipNormalOrient){
	IRIT_PT_SCALE(VP1, -1.0);
	IRIT_PT_SCALE(VP2, -1.0);
    }

    CG_GL_SET_PARAMETER_4F(GlblVP1, (GLfloat) VP1[0], (GLfloat) VP1[1], 
		(GLfloat) VP1[2], 1);
    CG_GL_SET_PARAMETER_4F(GlblVP2, (GLfloat) VP2[0], (GLfloat) VP2[1], 
		(GLfloat) VP2[2], 1);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Checks if a cg error occured.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   void                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   True if there was cg error                                               *
*****************************************************************************/
int CheckCgError(void)
{
    CGerror
	Err = cgGetError();

	if (Err != CG_NO_ERROR) { 
	    if (IGGlblMore) {
		printf("%s\n\n", cgGetErrorString(Err));
		printf("%s\n", cgGetLastListing(GlblContext));
		printf("Cg error!\n");
	    }
	return TRUE;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Checks if machine hardware supports VP40 & FP40 cg profiles.               * 
* Loading the shaders if there is support.                                   * 
*                                                                            *
* PARAMETERS:                                                                *
*   void                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if loading was successful, FALSE otherwise.              *
*****************************************************************************/
static int IGLoadShaders(IPObjectStruct *PObj)
{
    static int
	Init = TRUE;
    char ShaderPath[IRIT_LINE_LEN_LONG];
    const char *Shader;

    if (GlblVertexProgram != NULL) 
        return TRUE; /* Valid shaders are already loaded. */
	
    if (!Init)
	return FALSE;

    /* CG_PROFILE_VP40 supports the NV_vertex_program3 extension, */
    if (cgGLIsProfileSupported(CG_PROFILE_VP40) &&
    	    cgGLIsProfileSupported(CG_PROFILE_FP40)) {
	VertexProfile = CG_PROFILE_VP40;
	cgGLSetOptimalOptions(VertexProfile);
	FragmentProfile = CG_PROFILE_FP40;
	cgGLSetOptimalOptions(FragmentProfile);
    }
    else {
        if (IGGlblMore)
	    IGIritError("Hardware Doesn't Support VP40 Profiles");
	Init = FALSE;
	return FALSE; 
    }

    /* Test GlblCGcontext creation. */
    GlblContext = cgCreateContext();
    if (CheckCgError()) {
	Init = FALSE;
        return FALSE;
    }
    
    Shader = AttrGetObjectStrAttrib(PObj, "Shader");
    if (Shader == NULL)
        SearchPath(NULL, "ddm_vshd.cg", NULL, 255, ShaderPath, NULL);
    else
        SearchPath(NULL, Shader, NULL, 255, ShaderPath, NULL);

    /* Test adding source text to context. */
    GlblVertexProgram = cgCreateProgramFromFile(GlblContext, CG_SOURCE,
						ShaderPath, VertexProfile,
						"DdmVertexShader", NULL);
    if (CheckCgError()) {
	Init = FALSE;
        return FALSE;
    }
    if (IGGlblMore)
	printf("Using Vertex Shader: %s\n", ShaderPath);

    if (!cgIsProgramCompiled(GlblVertexProgram))
        cgCompileProgram(GlblVertexProgram);

    if (GlblVertexProgram != NULL) {
	cgGLLoadProgram(GlblVertexProgram);
	if (CheckCgError()) {
	    Init = FALSE;
	    return FALSE;
	}
    }
    /* Fragment Shader section */
/*   
    GlblFragmentProgram = cgCreateProgramFromFile(GlblContext, CG_SOURCE,
						  ShaderPath, FragmentProfile,
						  "PixelShader", NULL);
    if (CheckCgError()) {
	Init = FALSE;
        return FALSE;
    }
    if (!cgIsProgramCompiled(GlblFragmentProgram))
        cgCompileProgram(GlblFragmentProgram);
	    if (GlblFragmentProgram != NULL) {
		cgGLLoadProgram(GlblFragmentProgram);
		if (CheckCgError()) {
		    Init = FALSE;
		    return FALSE;
	}
    }
*/

    /* Setup the parameters for the vertex shader */
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "TileUV", GlblTileUV);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "TexDim", GlblTexDim);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Sampling", GlblSampling);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "OCoord", GlblOCoord);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "ONormal", GlblONormal);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Scale", GlblScale);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "ConstScale", GlblConstScale);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Morph", GlblMorph);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "ModelViewProjMatrix",
			   GlblModelViewProjMatrix);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "ModelViewMatrix", 
		GlblModelViewMatrix);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "MiscMatrix", GlblMiscMatrix);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Ambient", GlblAmbient);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Diffuse", GlblDiffuse);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Specular", GlblSpecular);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Shininess", GlblShininess);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "LightSource0", GlblLightSource0);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "LightSource1", GlblLightSource1);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "Shaded", GlblVShaded);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "In.VCompatiblePosition", 
		GlblVCompatiblePosition);    
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "In.VCompatibleNormal", 
		GlblVCompatibleNormal);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "In.VCompatibleColor", 
		GlblVCompatibleColor);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "In.VP1", GlblVP1);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "In.VP2", GlblVP2);
    CG_GET_NAMED_PARAMETER(GlblVertexProgram, "CalcNormalsMethod", 
		GlblCalcNormalsMethod);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Create textures for vertex positions and normal for the shaders.           *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Object to draw.                                             *
*   NumUSamples: Number of samples in U.                                     *
*   NumVSamples: Number of samples in V.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGCreateTextureFromObject(IPObjectStruct *PObj, 
				      int NumUSamples, 
				      int NumVSamples)
{
    int uIdx, vIdx,
        Rational = 0;
    double uStep, vStep, u, v, MinU, MaxU, MinV, MaxV;
    CagdRType *P;
    CagdVecStruct *N;
    GLuint TxtrPos, TxtrNrml;
    GLfloat *PTexture, *NTexture;
    CagdBBoxStruct BBox;
    clock_t start;
    if (AttrGetObjectIntAttrib(PObj, "_PTexture") != IP_ATTR_BAD_INT ||
    	AttrGetObjectIntAttrib(PObj, "_NTexture") != IP_ATTR_BAD_INT)
    	return;
    start = clock();

    if (IP_IS_POLY_OBJ(PObj)) {	
	if (ConstructTexturesFromPolyModel(PObj ,NumUSamples,
					   NumVSamples) == FALSE) {
	    printf("Fail to create DDM textures from poly model");
	}
#ifdef RENDER_INFO
	printf("preprocessing = %lf\n",
	    (double) (clock() - start) / CLOCKS_PER_SEC);
#endif
	return;
    }
    else if (!IP_IS_SRF_OBJ(PObj))
    	return;	
    
    CagdSrfBBox(PObj -> U.Srfs,&BBox);
    CagdSrfDomain(PObj -> U.Srfs, &MinU, &MaxU, &MinV, &MaxV);
    uStep = (MaxU - MinU) / (NumUSamples);
    vStep = (MaxV - MinV) / (NumVSamples);
    PTexture = (GLfloat *) IritMalloc(NumUSamples * NumVSamples *
				      sizeof(GLfloat) * 4);
    NTexture = (GLfloat *) IritMalloc(NumUSamples * NumVSamples *
				      sizeof(GLfloat) * 4); 
    if (CAGD_IS_RATIONAL_SRF(PObj -> U.Srfs))
	Rational = 1;
	
    /* Sample the surface on rectangular grid. */
    for (uIdx = 0; uIdx < NumUSamples; uIdx++) {
	for (vIdx = 0; vIdx < NumVSamples; vIdx++) {
	    int Idx;
	    IrtRType w;

	    u = uIdx * uStep;
	    v = vIdx * vStep;
	    Idx = (vIdx * NumUSamples + uIdx) * 4;

	    P = CagdSrfEval(PObj -> U.Srfs, u, v);
	    N = CagdSrfNormal(PObj -> U.Srfs, u, v, TRUE);
	    if (Rational) 
		 w = P[0];
	    else 
		 w = 1;
	    PTexture[Idx] = (GLfloat) P[1] / (GLfloat) w;
	    PTexture[Idx + 1] = (GLfloat) P[2] / (GLfloat) w;
	    PTexture[Idx + 2] = (GLfloat) P[3] / (GLfloat) w;
	    PTexture[Idx + 3] = 1;
	
	    NTexture[Idx] = (GLfloat) N -> Vec[0];
	    NTexture[Idx + 1] = (GLfloat) N -> Vec[1];
	    NTexture[Idx + 2] = (GLfloat) N -> Vec[2];
	    NTexture[Idx + 3] = 0;
	}
    }
#ifdef RENDER_INFO
    printf("preprocessing = %lf\n", (double) (clock() - start) / CLOCKS_PER_SEC);
#endif
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &TxtrPos);
    glBindTexture(GL_TEXTURE_2D, TxtrPos);
    if (AttrGetObjectIntAttrib(PObj, "ClampX") != 0)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (AttrGetObjectIntAttrib(PObj, "ClampY") != 0)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	    

    /* We must use GL_RGBA_FLOAT32_ATI for texture in shaders */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI,
		 NumUSamples, NumVSamples, 
		 0, GL_RGBA, GL_FLOAT, PTexture);

    glGenTextures(1, &TxtrNrml);
    glBindTexture(GL_TEXTURE_2D,TxtrNrml);
    if (AttrGetObjectIntAttrib(PObj, "ClampX") != 0)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (AttrGetObjectIntAttrib(PObj, "ClampY") != 0)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* We must use GL_RGBA_FLOAT32_ATI for texture in shaders */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI,
		 NumUSamples, NumVSamples,
		 0, GL_RGBA, GL_FLOAT, NTexture);

    glDisable(GL_TEXTURE_2D);
    AttrSetObjectIntAttrib(PObj, "_PTexture", TxtrPos);
    AttrSetObjectIntAttrib(PObj, "_NTexture", TxtrNrml);
		
    IritFree(PTexture);
    IritFree(NTexture);

}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if an integer is power of 2.					     *
*   If not input will be changed to the ceil power of two number.            *
*   This function should be used to correct wrong input sampling parameters  *
*   of texture creating functions                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   x:       input/output integer.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.								     *
*****************************************************************************/ 
static void PowerOfTwoTexture(int *x)
{
    int i;

    for (i = 1; i < 32768; i *= 2)
	if (*x <= i)
	break;
	if (*x < i) {
	    *x = i;
	    if (IGGlblMore)
		printf ("Sampling paramater is not power of 2. Rounding up to: %d\n", i);
    }
    return;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Parses the string of the "DTexture" attribute	      		     *
*                                                                            *
* PARAMETERS:                                                                *
*   DTexture:       The string of the "dtexture" attribute.                  *
*   FName:          The texture file name will be placed here.               *
*   Du:             Number of texture objects to place in U axis.            *
*   Dv:             Number of texture objects to place in V axis.            *
*   SamplingFactor: Number of samples to take in each axis (effective        *
*                   sampling = (Du,Dv) * SamplingFactor                      *
*   Scale:          The scaling factor in W axis.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:         TRUE if parsed succesfully, FALSE otherwise.                *
*****************************************************************************/ 
static int IrtParseDTextureString(const char *DTexture,
				  char *FName,
				  char *Shader,
				  float *NumTilesX,
				  float *NumTilesY,
				  int *NumSamplesX,
				  int *NumSamplesY,
				  int *ClampX,
				  int *ClampY,
				  float *Scale,
				  int *DrawOriginalObject,
				  int *DdmAnimation,
				  float *CalcNormalsMethod,
				  int *UseMultiTiles)
{
    char *p, *q;
    char t;
    int BoolTiles = 0,
	BoolSamples = 0;
    
    if (DTexture == NULL)
    	return FALSE;

    *ClampX = 0;
    *ClampY = 0;
    *Scale = 1;
    *DrawOriginalObject = 0;
    strcpy(Shader, "");
    *DdmAnimation = 1;
    *CalcNormalsMethod = 0;
	*UseMultiTiles = 0;

    strncpy(FName, DTexture, IRIT_LINE_LEN_LONG - 1);

    if ((p = strchr(FName, ',')) != NULL) {
	*p++ = 0;		      /* Mark the end of the regular string. */
	
       while (1){	   
	    q = strchr(p, ',');
	    if (q != NULL) 
		*q++ = 0;
	    while (*p == ' ') 
		p++;
	    if (*p == 'S'){
		sscanf(p, "%c %d %d", &t, NumSamplesX, NumSamplesY);
		BoolSamples = 1;
	    }
	    else if (*p == 'T'){
		sscanf(p, "%c %f %f", &t, NumTilesX, NumTilesY);
		BoolTiles = 1;
	    }
	    else if (*p == 'Z') 
		sscanf(p, "%c %f", &t, Scale);
	    else if (!strcmp(p, "OB"))
		*DrawOriginalObject = 1;
	    else if (!strcmp(p, "OA"))
		*DrawOriginalObject = 2;
	    else if (*p == 'H') 
		sscanf(p, "%c %s", &t, Shader);
	    else if (*p == 'M')
		*UseMultiTiles = 1;
	    else if (!strcmp(p, "RU"))
		*ClampX = 0;
	    else if (!strcmp(p, "RV"))
		*ClampY = 0;
	    else if (!strcmp(p, "CU"))
		*ClampX = 1;
	    else if (!strcmp(p, "CV"))
		*ClampY = 1;
	    else if (!strcmp(p, "CRU"))
		*ClampX = 2;
	    else if (!strcmp(p, "CRV"))
		*ClampY = 2;
	    else if (*p == 'A') 
		sscanf(p, "%c %d", &t, DdmAnimation);
	    else if (!strcmp(p, "NT"))
		*CalcNormalsMethod = 1;
	    else if (!strcmp(p, "NO"))
		*CalcNormalsMethod = 0;
	    if (q == NULL)
		break;
	    p = q;
       }
    }
    if (!BoolTiles){
	if (IGGlblMore)
	    printf("Error: cant initialize dtexture. Missing number of tiles\n");
	    return FALSE;
    }
    if (!BoolSamples){
	*NumSamplesX = (int) *NumTilesX;
	*NumSamplesY = (int) *NumTilesY;
	if (IGGlblMore)
	    printf("Warning: missing number of samples. Using number of tiles\n");
    }
	return TRUE;
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Parses the string of the "dtexturefiles" attribute	      	             *
*                                                                            *
* PARAMETERS:                                                                *
*									     *
*   TextureFiles: The string of the "dtexturefiles" attribute.               *
*   PObj:         Main object to add file names attributes                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of proccessed files.				     *
*****************************************************************************/ 
static int IrtParseDTextureFiles(const char *TextureFiles, IPObjectStruct* PObj)
{
	int i = 1;
	char File[IRIT_LINE_LEN_LONG], Parser[IRIT_LINE_LEN_LONG], *q, *p,
	    Attrib[14] = "DTextureFile ";

	if (TextureFiles == NULL)
	    return FALSE;

	strcpy(Parser, TextureFiles);
	p = Parser;

	while (i < 10){	   
	    q = strchr(p, ' ');
	    if (q != NULL) 
		*q++ = 0;
	    sscanf (p, "%s", &File);
	    Attrib[12] = i + '0';
	    AttrSetObjectStrAttrib(PObj, Attrib, File);
	    if (q == NULL)
		break;
	    p = q;
	    i++;
	}
	AttrSetObjectIntAttrib(PObj, "NumFiles", i);
	return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads in a ddm texture details if object has "ptexture" attribute and    *
* set up the texture for further processing.                                 *
*                                                                            *
* PARA*ETERS:                                                                *
*   PObj:      Object to extract its texture mapping function if has one.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if object has ddm texture and process was successful     *
*****************************************************************************/
static int IGInitDTexture(IPObjectStruct *PObj)
{
    int	**DdmHandleList, NumSamplesX, NumSamplesY, ClampX, ClampY,
	DrawOriginalObject, DdmAnimation, UseMultiTiles;
    float NumTilesX, NumTilesY, Scale, CalcNormalsMethod;
    char DTextureFile[IRIT_LINE_LEN_LONG], Shader[IRIT_LINE_LEN_LONG];
    const char
	*Texture = AttrGetObjectStrAttrib(PObj, "DTexture"),
	*TextureFiles = AttrGetObjectStrAttrib(PObj, "DTextureFiles");

    if (Texture == NULL)
    	return FALSE;

    if ((DdmHandleList =
	   AttrGetObjectPtrAttrib(PObj, "_DdmHandleList")) != NULL &&
	   DdmHandleList[0][0] > 0) 
	return TRUE;
    
    if (!IrtParseDTextureString(Texture, DTextureFile, Shader, &NumTilesX,
				&NumTilesY, &NumSamplesX, &NumSamplesY, 
				&ClampX, &ClampY, &Scale, &DrawOriginalObject, 
				&DdmAnimation, &CalcNormalsMethod, &UseMultiTiles)) {
	IGActiveFreeNamedAttribute(PObj, "DTexture");
    	return FALSE;
    }
    IrtParseDTextureFiles(TextureFiles, PObj);

    PowerOfTwoTexture(&NumSamplesX);
    PowerOfTwoTexture(&NumSamplesY);

    AttrSetObjectStrAttrib(PObj, "DTextureFile", DTextureFile);
    if (strcmp(Shader, ""))
	AttrSetObjectStrAttrib(PObj, "Shader", Shader);
    AttrSetObjectIntAttrib(PObj, "NumSamplesX", NumSamplesX);
    AttrSetObjectIntAttrib(PObj, "NumSamplesY", NumSamplesY);
    AttrSetObjectRealAttrib(PObj, "NumTilesX", NumTilesX);
    AttrSetObjectRealAttrib(PObj, "NumTilesY", NumTilesY);
    AttrSetObjectIntAttrib(PObj, "ClampX", ClampX);
    AttrSetObjectIntAttrib(PObj, "ClampY", ClampY);
    AttrSetObjectRealAttrib(PObj, "Scale", Scale);
    AttrSetObjectIntAttrib(PObj, "DrawOriginalObject", DrawOriginalObject);
    AttrSetObjectIntAttrib(PObj, "DdmAnimation", DdmAnimation);
    AttrSetObjectRealAttrib(PObj, "CalcNormalsMethod", CalcNormalsMethod);
    AttrSetObjectRealAttrib(PObj, "UseMultiTiles", UseMultiTiles);


    if (glActiveTextureARB == NULL)
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
	    wglGetProcAddress("glActiveTextureARB");

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculates animation factors for Z Scale, Mov U, Mov V                   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:	     Object to draw.					     *
*   t:               Animation run time.                                     *
*   AnimFactors[3]:  return factors: [ZScale, Mov U, Mov V]                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.				   				     *
*****************************************************************************/
static void IGGetAnimFactors(IPObjectStruct *PObj,
			     IrtRType t,
			     IrtRType AnimFactors[4])
{
    int i;
    IrtRType
        Factor = 1.0;
    IPObjectStruct
	*AnimCrv,
	*Crvs[4] = {NULL}, *TCrv;

    while (IP_IS_OLST_OBJ(PObj))
	PObj = IPListObjectGet(PObj, 0);
	AnimCrv = AttrGetObjectObjAttrib(PObj, "animation");
	AnimFactors[0] = AnimFactors[3] = 1;
	AnimFactors[1] = AnimFactors[2] = 0;
	if (AnimCrv != NULL) {
	    if (IP_IS_OLST_OBJ(AnimCrv)) {
		int ListLen = IPListObjectLength(AnimCrv);
		for (i = 0; i < ListLen; i++) {
		TCrv = IPListObjectGet(AnimCrv, i);
		if (IP_IS_CRV_OBJ(TCrv)) {
		    if (strnicmp(IP_GET_OBJ_NAME(TCrv), "SCL_Z", 5) == 0)
		    Crvs[0] = TCrv;
		    else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MOV_U", 5) == 0)
		    Crvs[1] = TCrv;
		    else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MOV_V", 5) == 0)
		    Crvs[2] = TCrv;
		    else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MORPH", 5) == 0)
		    Crvs[3] = TCrv;
		}
	    }      
	}
	else {
	    TCrv = AnimCrv;
    	    if (IP_IS_CRV_OBJ(TCrv)) {
    		if (strnicmp(IP_GET_OBJ_NAME(TCrv), "SCL_Z", 5) == 0)
		    Crvs[0] = TCrv;
		else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MOV_U", 5) == 0)
		    Crvs[1] = TCrv;
		else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MOV_V", 5) == 0)
		    Crvs[2] = TCrv;
		else if (strnicmp(IP_GET_OBJ_NAME(TCrv), "MORPH", 5) == 0)
		    Crvs[3] = TCrv;
	    }	    
	}
	for (i = 0; i < 4; i++) {
	    IrtRType *R, TMin, TMax,
		tt = t;

	    CagdCrvStruct *Crv;				
	    if (Crvs[i] == NULL)
	    continue;
	    Crv = Crvs[i] -> U.Crvs;
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (t < TMin)
	    tt = TMin;
	    else if (t > TMax)
	    tt = TMax;
	    R = CagdCrvEval(Crv, tt);
	    AnimFactors[i] = CAGD_IS_RATIONAL_CRV(Crv) ? R[1] / R[0] : R[1];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a single object with DTexture attributes using current modes       M
* and transformations.		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if no go.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGCGFreeDTexture                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGDrawDTexture                                                         M
*****************************************************************************/
int IGCGDrawDTexture(IPObjectStruct *PObj)
{
    char *TileMap;
    GLuint TxtrPos, TxtrNrml, **DdmHandleList;
    int n, NumSamplesX, NumSamplesY, u, v, DdmAnimation, NumFiles,
		UseMultiTiles,
		TileCount = 0,
		CurrentAnim = 0,
		CurrentObj = 0,
		TileType = RECT_TILE;
    float SamplingFactorX, SamplingFactorY, NumTilesX, NumTilesY, x, y, Shaded,
		CScale = 1.0f,
		ZScaleFactor = 1.0f,
		Morph = 1.0f,
		CalcNormalsMethod = 0.0f;
    static float 
	TransU = 0.0,
        TransV = 0.0;
    IrtRType AnimFactors[4],
	Transp = AttrGetObjectRealAttrib(PObj, "Transp");
    IrtHmgnMatType Mat;
    IrtPtType LightSource0, LightSource1;  

    if (!IGInitDTexture(PObj) ||
	!IGLoadShaders(PObj) ||
	!IGCreateDdmDisplayList(PObj))
        return FALSE;

    IGCheckSysStatus();
        
    glPushAttrib(GL_CURRENT_BIT | GL_BLEND | GL_TEXTURE_2D);
    if (!IP_ATTR_IS_BAD_REAL(Transp))
	IGSetTranspObj(0.0);
    
    CalcNormalsMethod = (float) AttrGetObjectRealAttrib(PObj,
							"CalcNormalsMethod");
    DdmHandleList = AttrGetObjectPtrAttrib(PObj, "_DdmHandleList");
    DdmAnimation = AttrGetObjectIntAttrib(PObj, "DdmAnimation");
    if (AttrGetObjectIntAttrib(PObj, "LastHandle") != IP_ATTR_BAD_INT)
	CurrentAnim = AttrGetObjectIntAttrib(PObj, "LastHandle");
    if ((NumFiles = AttrGetObjectIntAttrib(PObj, "NumFiles")) == IP_ATTR_BAD_INT)
	NumFiles = 0;
    if ((UseMultiTiles = 
	    AttrGetObjectIntAttrib(PObj, "UseMultiTiles")) == IP_ATTR_BAD_INT)
	UseMultiTiles = 0;

    
    if (IGGlblAnimation) {
	IPObjectStruct** ObjPtrList = 
	    AttrGetObjectPtrAttrib(PObj, "_ObjPtrList");
	
	CurrentAnim = (int) (IGAnimation.RunTime * 100) % DdmAnimation;

	IGGetAnimFactors(ObjPtrList[0],
			IGAnimation.RunTime, AnimFactors);
	ZScaleFactor = (float) AnimFactors[0],
	TransU = (float) AnimFactors[1],
	TransV = (float) AnimFactors[2];
	Morph = (float) AnimFactors[3];
	IGActiveFreeNamedAttribute(PObj, "ZScaleFactor");
	IGActiveFreeNamedAttribute(PObj, "TransU");
	IGActiveFreeNamedAttribute(PObj, "TransV");
	IGActiveFreeNamedAttribute(PObj, "Morph");
	AttrSetObjectRealAttrib(PObj, "ZScaleFactor", (float) AnimFactors[0]);
	AttrSetObjectRealAttrib(PObj, "TransU", (float) AnimFactors[1]);
	AttrSetObjectRealAttrib(PObj, "TransV", (float) AnimFactors[2]);
	AttrSetObjectRealAttrib(PObj, "Morph", (float) AnimFactors[3]);
    }

	{
	    IrtRType
		LastZScaleFactor = AttrGetObjectRealAttrib(PObj, "ZScaleFactor"),
		LastMorph = AttrGetObjectRealAttrib(PObj, "Morph");

	    if (LastZScaleFactor != IP_ATTR_BAD_REAL)
		ZScaleFactor = (float) LastZScaleFactor;
	    if (LastMorph != IP_ATTR_BAD_REAL)
		Morph = (float) LastMorph;
	}

    if ((TileType = AttrGetObjectIntAttrib(PObj,
					"TILE_TYPE")) == IP_ATTR_BAD_INT)
	TileType = RECT_TILE;
	
    if (AttrGetObjectIntAttrib(PObj, "DrawOriginalObject") == 1) {
        char DTexture[255];
	const char
	    *p = AttrGetObjectStrAttrib(PObj, "DTexture");

	strncpy(DTexture, p, 255);
	AttrFreeObjectAttribute(PObj, "DTexture");
	IGDisplayObject(PObj);
	AttrSetObjectStrAttrib(PObj, "DTexture", DTexture);
    }

    NumTilesX = (float) AttrGetObjectRealAttrib(PObj, "NumTilesX");
    NumTilesY = (float) AttrGetObjectRealAttrib(PObj, "NumTilesY");
    NumSamplesX = AttrGetObjectIntAttrib(PObj, "NumSamplesX");
    NumSamplesY = AttrGetObjectIntAttrib(PObj, "NumSamplesY");
    CScale *= (float) AttrGetObjectRealAttrib(PObj, "Scale");
    SamplingFactorX = (float) NumSamplesX / (float) NumTilesX;
    SamplingFactorY = (float) NumSamplesY / (float) NumTilesY;
    
    IGCreateTextureFromObject(PObj, NumSamplesX, NumSamplesY);

    LightSource0[0] = IGShadeParam.LightPos[0][0];
    LightSource0[1] = IGShadeParam.LightPos[0][1];
    LightSource0[2] = IGShadeParam.LightPos[0][2];
    LightSource1[0] = IGShadeParam.LightPos[1][0];
    LightSource1[1] = IGShadeParam.LightPos[1][1];
    LightSource1[2] = IGShadeParam.LightPos[1][2];
    IRIT_PT_NORMALIZE(LightSource0);
    IRIT_PT_NORMALIZE(LightSource1);
    
    MatGenUnitMat(Mat);

    CG_GL_SET_PARAMETER_2F(GlblTexDim, (GLfloat) NumSamplesX,
			   (GLfloat) NumSamplesY); 
    CG_GL_SET_PARAMETER_2F(GlblSampling, (GLfloat) SamplingFactorX,
			   (GLfloat) SamplingFactorY);
    CG_GL_SET_PARAMETER_1F(GlblScale, ZScaleFactor);
    CG_GL_SET_PARAMETER_1F(GlblConstScale, CScale);
    CG_GL_SET_PARAMETER_1F(GlblMorph, Morph);

    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblModelViewProjMatrix,
				     CG_GL_MODELVIEW_PROJECTION_MATRIX, 
	                             CG_GL_MATRIX_IDENTITY);
    CG_GL_SET_STATE_MATRIX_PARAMETER(GlblModelViewMatrix,
		CG_GL_MODELVIEW_MATRIX, CG_GL_MATRIX_IDENTITY);
    
    cgGLSetMatrixParameterdr(GlblMiscMatrix, &Mat[0][0]);
    CheckCgError();

    CG_GL_SET_PARAMETER_3F(GlblAmbient,
			   IGShadeParam.LightAmbient[0],
			   IGShadeParam.LightAmbient[1],
			   IGShadeParam.LightAmbient[2]);
    CG_GL_SET_PARAMETER_3F(GlblDiffuse,
			   IGShadeParam.LightDiffuse[0],
			   IGShadeParam.LightDiffuse[1],
			   IGShadeParam.LightDiffuse[2]);
    CG_GL_SET_PARAMETER_3F(GlblSpecular,
			   IGShadeParam.LightSpecular[0],
			   IGShadeParam.LightSpecular[1],
			   IGShadeParam.LightSpecular[2]);
    CG_GL_SET_PARAMETER_3F(GlblLightSource0,
			   (float) LightSource0[0],
			   (float) LightSource0[1],
			   (float) LightSource0[2]);
    CG_GL_SET_PARAMETER_3F(GlblLightSource1,
			   (float) LightSource1[0],
			   (float) LightSource1[1],
			   (float) LightSource1[2]);
    CG_GL_SET_PARAMETER_1F(GlblShininess, IGShadeParam.Shininess);
    CG_GL_SET_PARAMETER_1F(GlblCalcNormalsMethod, CalcNormalsMethod);

    Shaded = (float) (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID);

    CG_GL_SET_PARAMETER_1F(GlblVShaded, Shaded);
       
    TxtrPos = AttrGetObjectIntAttrib(PObj, "_PTexture");
    glBindTexture(GL_TEXTURE_2D, TxtrPos);
    cgGLSetTextureParameter(GlblOCoord, TxtrPos);	
    CheckCgError();
    
    TxtrNrml = AttrGetObjectIntAttrib(PObj, "_NTexture");
    glBindTexture(GL_TEXTURE_2D, TxtrNrml);
    cgGLSetTextureParameter(GlblONormal, TxtrNrml);	
    CheckCgError();

    cgGLEnableTextureParameter(GlblOCoord);
    cgGLEnableTextureParameter(GlblONormal);

    CheckCgError();
    cgGLEnableProfile(VertexProfile);
    CheckCgError();
    cgGLBindProgram(GlblVertexProgram);
    CheckCgError();

    n = AttrGetObjectIntAttrib(PObj, "DdmNumPolys");
    if (IGGlblCountNumPolys && !IP_ATTR_IS_BAD_INT(n))
	IGGlblNumPolys += (int) NumTilesX * (int) NumTilesY * n;

    TileMap = (char *) AttrGetPtrAttrib(PObj -> Attr, "_TileDecisionMap");
    
    IritRandomInit(1);

    if (TileType == RECT_TILE) {
	int InitU = 0, 
	    InitV = 0;

	if ((AttrGetObjectIntAttrib(PObj, "ClampX") == 2)){
	    TransU = ((TransU * NumTilesX) - 
		    (int) (TransU * NumTilesX)) / NumTilesX;
	    InitU = -1;
	}
	if ((AttrGetObjectIntAttrib(PObj, "ClampY") == 2)){
	    TransV = ((TransV * NumTilesY) - 
		    (int) (TransV * NumTilesY)) / NumTilesY;
	    InitV = -1;
	}
	for (u = InitU; u < NumTilesX; u++) {
    	    for (v = InitV; v < NumTilesY; v++) {
		if (TileMap && 
		    TileMap[(int) (u * NumTilesY + v)] == 0) {
		    continue;
		}
		TileCount++; 
		CG_GL_SET_PARAMETER_2F(GlblTileUV,
			       x = (GLfloat) u / (GLfloat) NumTilesX + TransU, 
			       y = (GLfloat) v / (GLfloat) NumTilesY + TransV);
		CheckCgError();
		glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		if (UseMultiTiles) {
		    CurrentObj = (int) (IritRandom(0, NumFiles + 1));
		}
	    }
	}
    }
    else if (TileType == HEX_TILE) {
	float
	    rx = 1 / (2 * NumTilesX),
	    ry = 1 / (2 * NumTilesY),
	    StepX = (2 - (float) sin(30 * M_PI / 180)) * rx;

	for (u = 0; u < NumTilesX * 3 / 2; u++) {
	    for (v = 0; v < NumTilesY; v++) {
		x = StepX * u;
		y = v / NumTilesY;
		if (u % 2 == 1) {
		    y += ry;
		}
		x += TransU;
		y += TransV;
		CG_GL_SET_PARAMETER_2F(GlblTileUV, x, y);	
		glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		TileCount++; 
	    }
	}
    }
    else if (TileType == TRIG_TILE || TileType == TRIG_TILE_REV) {
	float
	    Sin60 = (float) sin(60 * M_PI / 180),
	    StepX = 1 / NumTilesX,
	    StepY = 1 / NumTilesY;

	for (u = 0; u < NumTilesX; u++) {
	    for (v = 0; v < NumTilesY; v++) {
		x = u * StepX;   
		y = v * StepY;
		x += TransU;
		y += TransV;

		if (v % 2 == 0 || TileType == TRIG_TILE_REV){
		    MatGenUnitMat(Mat);
		    CG_GL_SET_PARAMETER_2F(GlblTileUV, x, y);	
		    cgGLSetMatrixParameterdr(GlblMiscMatrix, &Mat[0][0]);
		    glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		    x += StepX * 1.5f;
		    y += StepY;
		    CG_GL_SET_PARAMETER_2F(GlblTileUV, x, y);	
		    MatGenMatRotX1(M_PI, Mat);
		    cgGLSetMatrixParameterdr(GlblMiscMatrix, &Mat[0][0]);
		    glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		}
		else {
		    MatGenUnitMat(Mat);
		    x += StepX * 0.5f;
		    CG_GL_SET_PARAMETER_2F(GlblTileUV, x, y);	
		    cgGLSetMatrixParameterdr(GlblMiscMatrix, &Mat[0][0]);
		    glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		    x += StepX * 0.5f;
		    y += StepY;
		    MatGenMatRotX1(M_PI, Mat);
		    CG_GL_SET_PARAMETER_2F(GlblTileUV, x, y);	
		    cgGLSetMatrixParameterdr(GlblMiscMatrix, &Mat[0][0]);
		    glCallList(DdmHandleList[CurrentObj][CurrentAnim]);
		}
		TileCount += 2;
	    }
	}
    }
#ifdef RENDER_INFO
    printf("total tiles %lf rendered tiles = %d\n",
	    NumTilesX * NumTilesY, TileCount); 
#endif

    TileCount = 0;
    glPopAttrib();
    if (!IP_ATTR_IS_BAD_REAL(Transp))
	IGSetTranspObj(Transp);

    cgGLDisableTextureParameter(GlblOCoord);
    cgGLDisableTextureParameter(GlblONormal);

    glDisable(GL_TEXTURE_2D);
    cgGLDisableProfile(VertexProfile);
/*  cgGLDisableProfile(FragmentProfile); */
    if (AttrGetObjectIntAttrib(PObj, "DrawOriginalObject") != 2)
	return TRUE;
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The function is a vertex callback for rendering model dimensions         *
* The texture and model BBoxes are computed				     * 
* PARAMETERS:                                                                *
*   PObj:      Object to draw.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static int IGVertexBoundsCompute(IPVertexStruct *V)
{
    float *Uv;

    if ((Uv = AttrGetUVAttrib(V -> Attr, "uvvals")) != NULL) {
	int i;

	for (i = 0; i < 3; i++) {
	    if (V -> Coord[i] < GlblPosBBox[0][i])
		GlblPosBBox[0][i] = V -> Coord[i];
	    if (V -> Coord[i] > GlblPosBBox[1][i])
		GlblPosBBox[1][i] = V -> Coord[i];
	}
	
	for (i=0; i < 2; i++) {
	    if (Uv[i] < GlblUvBBox[0][i])
		GlblUvBBox[0][i] = Uv[i];
	    if (Uv[i] > GlblUvBBox[1][i])
		GlblUvBBox[1][i] = Uv[i];
	}
	
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The function is a vertex callback for rendering the texture space        *
* Position is encoded as color						     * 
* PARAMETERS:                                                                *
*   V:     Vertex to draw.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static int IGVertexPosAsColor(IPVertexStruct *V)
{
    float *Uv;

    if ((Uv = AttrGetUVAttrib(V -> Attr, "uvvals")) != NULL) {
	glColor4f((GLfloat) ((V -> Coord[0] - GlblPosBBox[0][0]) /
			     (GlblPosBBox[1][0] - GlblPosBBox[0][0])),
		  (GLfloat) ((V -> Coord[1] - GlblPosBBox[0][1]) /
			     (GlblPosBBox[1][1] - GlblPosBBox[0][1])),
		  (GLfloat) ((V -> Coord[2] - GlblPosBBox[0][2]) /
			     (GlblPosBBox[1][2] - GlblPosBBox[0][2])), 1.0);
	glVertex3f((GLfloat) ((Uv[0] - GlblUvBBox[0][0]) /
			      (GlblUvBBox[1][0]- GlblUvBBox[0][0])), 
	           (GLfloat) ((Uv[1] - GlblUvBBox[0][1]) /
			      (GlblUvBBox[1][1]- GlblUvBBox[0][1])), 0);
    }
    else
	return FALSE;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    The function is a vertex callback for rendering the texture space       *
*	 Normal is encoded as color					     * 
* PARAMETERS:                                                                *
*   V:     Vertex to draw.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE if no go.				     *
*****************************************************************************/
static int IGVertexNrmlAsColor(IPVertexStruct *V)
{
    float *Uv;

    if ((Uv = AttrGetUVAttrib(V -> Attr, "uvvals")) != NULL) {
	glColor4f((GLfloat)((V -> Normal[0] + 1) / 2),
		  (GLfloat)((V -> Normal[1] + 1) / 2),
		  (GLfloat)((V -> Normal[2] + 1) / 2), 1.0);
	glVertex3f((GLfloat) ((Uv[0] - GlblUvBBox[0][0]) /
			      (GlblUvBBox[1][0] - GlblUvBBox[0][0])), 
	           (GLfloat) ((Uv[1] - GlblUvBBox[0][1]) /
			      (GlblUvBBox[1][1] - GlblUvBBox[0][1])), 0);
    }
    else
	return FALSE;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a polygonal model with proper texture coordinates		     *
*   and proper normals, construct two DDM textures  to the model	     *		
* PARAMETERS:                                                                *
*   PObj : the input object						     *
*   NumUSamples,NumVSamples - texture dimensions			     *
* RETURN VALUE:                                                              *
*	TRUE if success, False otherwise				     *
*****************************************************************************/
static int ConstructTexturesFromPolyModel(IPObjectStruct *PObj,
				    int NumUSamples,
				    int NumVSamples)
{
    GLuint TxtrPos,TxtrNrml;
    GLfloat *Storage;
    char DTextureStr[256], DTexture2Str[256];
    const char *TStr;
    int i, j,
	DrawMode = IGGlblDrawStyle,
	HasDTexture2 = FALSE;
    float Scale[3], Bias[3], *Uv;
    IGVertexHandleExtraFuncType OldFunc;

    /* Check that we actually have texture coord. */
    if ((Uv = AttrGetUVAttrib(PObj -> U.Pl -> PVertex -> Attr,
			      "uvvals")) == NULL)
        return FALSE;
    
    TStr = AttrGetStrAttrib(PObj -> Attr, "dtexture");
    if (TStr)
	strcpy(DTextureStr, TStr);
    else 
	return FALSE;
    AttrFreeOneAttribute(&PObj -> Attr, "dtexture");
    TStr = AttrGetStrAttrib(PObj -> Attr, "dtexture2");
    if (TStr) {
        strcpy(DTexture2Str, TStr);
	AttrFreeOneAttribute(&PObj -> Attr, "dtexture2");
	HasDTexture2 = TRUE;
    }

    /* Render once to compute bounding boxes */
    IGVertexBoundsCompute(PObj -> U.Pl -> PVertex);
    OldFunc = IGSetHandleVertexProcessingFunc(IGVertexBoundsCompute);
    IGDrawObject(PObj);

    InitPBuffer(0,NumUSamples, NumVSamples);
    MakePBufferCurrent(0);
    /* Set up the viewing parameters */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    gluLookAt(0, 0, 0,
              0, 0, -1,
              0, 1, 0);
	
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_SOLID;

    /* Render the position */
    IGSetHandleVertexProcessingFunc(IGVertexPosAsColor);	
    IGDrawObject(PObj);

    Storage = IGGetPBufferToHost(0, NumUSamples, NumVSamples);
    DecideNonTiles(PObj, Storage, NumUSamples, NumVSamples);

    MakeCurrent(0);
    /* Apply Bias and scale to the model */
    for (j = 0; j < 3; j++) {
	Scale[j] = (float) (GlblPosBBox[1][j] - GlblPosBBox[0][j]);
	Bias[j] = (float) GlblPosBBox[0][j];
    }
    for (i = 0; i < NumUSamples * NumVSamples * 4; i += 4) {
	for (j = 0; j < 3; j++) {
	    Storage[i + j] = Storage[i + j] * Scale[j] + Bias[j];
	}
    }
    ApplyDiffusion(Storage, NumUSamples, NumVSamples);
    glGenTextures(1, &TxtrPos);
    glBindTexture(GL_TEXTURE_2D, TxtrPos);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI,
		 NumUSamples, NumVSamples, 0, GL_RGBA, GL_FLOAT, Storage);
    AttrSetObjectIntAttrib(PObj, "_PTexture", TxtrPos);
    IritFree(Storage);
    ClosePBuffer(0);
    InitPBuffer(0, NumUSamples, NumVSamples);
    /* Render the normals. */
    MakePBufferCurrent(0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 
    gluLookAt(0, 0, 0,
              0, 0, -1,
              0, 1, 0);
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    IGSetHandleVertexProcessingFunc(IGVertexNrmlAsColor);
    IGDrawObject(PObj);
    Storage = IGGetPBufferToHost(0, NumUSamples, NumVSamples);
    for (i = 0; i < NumUSamples * NumVSamples * 4; i += 4) {
	for (j = 0; j < 3; j++) {
	    Storage[i + j] = Storage[i + j] * 2 - 1;
	}
    }
	
    ApplyDiffusion(Storage, NumUSamples, NumVSamples);
    MakeCurrent(0);
    glGenTextures(1, &TxtrNrml);
    glBindTexture(GL_TEXTURE_2D, TxtrNrml);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    	
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI,
		 NumUSamples, NumVSamples, 0, GL_RGBA, GL_FLOAT, Storage);
    if (glGetError() != GL_NO_ERROR)
        printf("error in glTexImage2D\n");
    AttrSetObjectIntAttrib(PObj, "_NTexture", TxtrNrml);
    IritFree(Storage);	
	
    /* Restore state. */
    ClosePBuffer(0);
    IGSetHandleVertexProcessingFunc(OldFunc);
    AttrSetStrAttrib(&PObj -> Attr, "dtexture", (char *) (DTextureStr));
    if (HasDTexture2)
	AttrSetStrAttrib(&PObj -> Attr, "dtexture2", (char *) (DTexture2Str));
    IGGlblDrawStyle = DrawMode;
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   given an initial map - propogate the values on the boundary to the edges *
* This will assure that patches that are o*utside the original map are       *
*  clamped. Use an 8-neighbor diffusion kernel				     *
* PARAMETERS:                                                                *
*   image:  2D RGBA texture of size 4 * XLen * YLen                          *
*                                                                            *
* RETURN VALUE:                                                              *
*	void.								     *
*****************************************************************************/
static void ApplyDiffusion(GLfloat *Image, int XLen, int YLen)
{
    int i, j, k,
        XIdx[8] = { -1, 1, 0, 0, -1, -1, 1, 1 },
	YIdx[8] = { 0 ,0, -1, 1, -1, 1, -1, -1 },
	MaxLen = XLen > YLen ? XLen : YLen; 
    float
	Weight[8] = {1, 1, 1, 1, 0.707f, 0.707f, 0.707f, 0.707f};

    GLfloat *TImg, *SourceImage, *TargetImage;
   
    TImg = (GLfloat*) IritMalloc(XLen * YLen * sizeof(GLfloat) * 4);
    IRIT_GEN_COPY(TImg, Image, XLen * YLen * sizeof(GLfloat) * 4);

    SourceImage = Image, TargetImage = TImg;
    for (k = 0; k < MaxLen; k++) {
        for (j = 0; j < YLen; j++) {
	    for (i = 0; i < XLen; i++) {    
		int s;
		GLfloat
		    avg[3] = { 0 },
		    TotalWeight = 0;

		if (SourceImage[4 * (j * XLen + i) + 3] == 1.0)
			continue;
		for (s = 0; s < 8; s++) {
		    if ((i + XIdx[s]) < 0 || (i + XIdx[s]) >= XLen ||
			(j + YIdx[s]) < 0 || (j + YIdx[s]) >= YLen)
			continue;
		    if (SourceImage[4 * ((j + YIdx[s]) * XLen +
			    (i + XIdx[s])) + 3] == 0.0)
			continue;
		    else {
			TotalWeight += Weight[s];
			avg[0] += Weight[s] * 
				SourceImage[4 * ((j + YIdx[s]) * XLen +
					(i + XIdx[s]))];
			avg[1] += Weight[s] *
				SourceImage[4 * ((j + YIdx[s]) * XLen + 
					(i + XIdx[s])) + 1];
			avg[2] += Weight[s] *
				SourceImage[4 * ((j + YIdx[s]) * XLen +
					(i + XIdx[s])) + 2];
		    }	
		}
		if (TotalWeight > 0) {
		    TargetImage[4 * (j * XLen + i)] = avg[0] / TotalWeight;
		    TargetImage[4 * (j * XLen + i) + 1] = avg[1] / TotalWeight;
		    TargetImage[4 * (j * XLen + i) + 2] = avg[2] / TotalWeight;
		    TargetImage[4 * (j * XLen + i) + 3] = 1.0;
		}
	    }
	}
	IRIT_GEN_COPY(SourceImage, TargetImage,
		      XLen * YLen * sizeof(GLfloat) * 4);
    }
    IRIT_GEN_COPY(Image, TargetImage, XLen * YLen * sizeof(GLfloat) * 4);

    IritFree(TImg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Used to contruct the polygonal object texture				     *
*                                                                            *
* PARAMETERS:                                                                *
*	XLen: Size of X							     *
*	YLen: Size of Y							     *
*									     *
* RETURN VALUE:                                                              *
*	void.								     *
*****************************************************************************/
static void DecideNonTiles(IPObjectStruct *PObj,
			   float *Map,
			   int XLen,
			   int YLen)
{
    int i, j,
        NumTilesX = (int) AttrGetObjectRealAttrib(PObj, "NumTilesX"),
        NumTilesY = (int) AttrGetObjectRealAttrib(PObj, "NumTilesY");
    char *TileMap;
    
    TileMap = (char *) IritMalloc(NumTilesY * NumTilesX);
    IRIT_ZAP_MEM(TileMap, NumTilesY * NumTilesX);
    
    for (j = 0; j < YLen; j++) {
	for (i = 0; i < XLen; i++) {    
	    int xIdx = (int) ((double) (i) / XLen * NumTilesX),
		yIdx = (int) ((double) (j) / YLen * NumTilesY);
	
	    if (Map[4 * (j * XLen + i) + 3] == 1.0) {
		TileMap[xIdx * NumTilesX + yIdx] = 1;
	    }	
	}
    }
    AttrSetPtrAttrib(&PObj -> Attr, "_TileDecisionMap", TileMap);
}

#else

/* Dummy functions to link to if no Open GL CG graphics is available. */
int IGCGDrawDTexture(IPObjectStruct *PObj)
{
    return FALSE;
}

void IGCGFreeDTexture(IPObjectStruct *PObj)
{
}

#endif /* IRIT_HAVE_OGL_CG_LIB */
