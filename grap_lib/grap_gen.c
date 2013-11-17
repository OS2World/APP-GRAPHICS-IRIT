/*****************************************************************************
*   General routines common to graphics library.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "ip_cnvrt.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "grap_loc.h"

#define IG_SILH_GRID_SIZE		20
#define IG_DISCONT_MIN_ANGLE		(M_PI / 4) /* 45 degrees dihedral. */

IRIT_GLOBAL_DATA int
    IGGlblBackGroundColor[3] = { 0,   0,   0 },
    IGGlblAbortKeyPressed = FALSE,
    IGGlblAdapIsoDir = CAGD_CONST_U_DIR,
    IGGlblBackFaceCull = FALSE,
    IGGlblCacheGeom = TRUE,
    IGGlblCountNumPolys = FALSE,
    IGGlblDepthCue = TRUE,
    IGGlblDrawInternal = FALSE,
    IGGlblDrawPNormal = FALSE,
    IGGlblDrawPolygons = TRUE,
    IGGlblDrawSurfaceMesh = FALSE,
    IGGlblDrawSurfacePoly = FALSE,
    IGGlblDrawSurfaceWire = TRUE,
    IGGlblDrawSurfaceBndry = FALSE,
    IGGlblDrawSurfaceSilh = FALSE,
    IGGlblDrawSurfaceDiscont = FALSE,
    IGGlblDrawSurfaceContours = FALSE,
    IGGlblDrawSurfaceIsophotes = FALSE,
    IGGlblDrawSurfaceSketch = FALSE,
    IGGlblDrawSurfaceRflctLns = FALSE,
    IGGlblDrawStyle = 0,
    IGGlblDrawVNormal = FALSE,
    IGGlblFlipNormalOrient = FALSE,
    IGGlblPolygonStrips = FALSE,
    IGGlblFourPerFlat = TRUE,
    IGGlblHighlight1Color[3] = { 255, 0, 255 },
    IGGlblHighlight2Color[3] = { 255, 0,   0 },
    IGGlblIntensityHighState = TRUE,
    IGGlblLastLowResDraw = FALSE,
    IGGlblLineWidth = 1,
    IGGlblManipulationActive = FALSE,
    IGGlblMore = FALSE,
    IGGlblNumOfIsolines = IG_DEFAULT_NUM_OF_ISOLINES,
    IGGlblNumPolys = 0,
    IGGlblPolygonOptiApprox = TRUE,
    IGGlblQuickLoad = FALSE,
    IGGlblContinuousMotion = FALSE,
    IGGlblViewMode = IG_VIEW_PERSPECTIVE,
    IGGlblAntiAliasing = 0,
    IGGlblShadingModel = IG_SHADING_PHONG,
    IGGlblDoDoubleBuffer = TRUE,
    IGGlblAnimation = FALSE,
    IGGlblTransformMode = IG_TRANS_SCREEN,
    IGGlblCountFramePerSec = 0,
    IGGlblAlwaysGenUV = FALSE;
IRIT_GLOBAL_DATA IrtRType
    IGGlblMinPickDist = 0.2,
    IGGlblNormalSize = 0.2,
    IGGlblPointSize = 0.02,
    IGGlblPlgnFineness = IG_DEFAULT_POLYGON_FINENESS,
    IGGlblPllnFineness = IG_DEFAULT_PLLN_OPTI_FINENESS,
    IGGlblRelLowresFineNess = 0.3,
    IGGlblFramePerSec = 0.0,
    IGGlblChangeFactor = 1.0,
    IGGlblZMinClip = -2.0,
    IGGlblZMaxClip = 2.0,
    IGGlblEyeDistance = 0.03;
IRIT_GLOBAL_DATA SymbCrvApproxMethodType
    IGGlblPolylineOptiApprox = SYMB_CRV_APPROX_TOLERANCE;
IRIT_GLOBAL_DATA IrtHmgnMatType IGGlblCrntViewMat, IGGlblInvCrntViewMat,
    IGGlblLastProcessMat, IGGlblPushViewMat, IGGlblPushPrspMat,
    IGGlblIsometryViewMat = {		     /* Isometric view, by default. */
	{ -0.707107, -0.408248, 0.577350, 0.000000 },
	{  0.707107, -0.408248, 0.577350, 0.000000 },
	{  0.000000,  0.816496, 0.577350, 0.000000 },
	{  0.000000,  0.000000, 0.000000, 1.000000 }
    };
IRIT_GLOBAL_DATA GMAnimationStruct IGAnimation = {
    0.0,	/* StartT */
    1.0,	/* FinalT */
    0.01,	/* Dt */
    0.0,	/* RunTime */
    FALSE,	/* TwoWaysAnimation */
    FALSE,	/* SaveAnimationGeom */
    FALSE,	/* SaveAnimationImage */
    FALSE,	/* BackToOrigin */
    1,		/* NumOfRepeat */
    FALSE,	/* StopAnim */
    FALSE,	/* SingleStep */
    FALSE,	/* TextInterface */
    30,		/* MiliSecSleep */
    0,		/* _Count; */
    NULL,	/* ExecEachStep */
    { 0 },	/* BaseFileName */
};
IRIT_GLOBAL_DATA IGShadeParamStruct IGShadeParam = {
    2,					/* Two light sources. */
    {
	{  1.0f,  2.0f,  -5.0f, 0.0f },   /* Position of first light source. */
        {  3.0f, -1.0f, -10.0f, 0.0f }   /* Position of second light source. */
    },
    { 0.1f, 0.1f, 0.1f, 1.0f },	                     /* Ambient RGB factors. */
    { 0.6f, 0.6f, 0.6f, 1.0f },                      /* Diffuse RGB factors. */
    { 0.5f, 0.5f, 0.5f, 1.0f },                     /* Specular RGB factors. */
    { 0.0f, 0.0f, 0.0f, 1.0f },                     /* Emissive RGB factors. */
    32.0f					        /* Shininess factor. */
};
IRIT_GLOBAL_DATA IGSketchParamStruct IGSketchParam = {
    0.9,				   /* Effect of shading on sketches. */
    0.1,			       /* Effect of silhouettes on sketches. */
    IG_SKETCHING_ISO_PARAM,	     /* Type of sketched silhouette strokes. */
    IG_SKETCHING_ISO_PARAM,		/* Type of sketched shading strokes. */
    IG_SKETCHING_ISOCLINES,	     /* Type of sketched importance strokes. */
    TRUE,			      /* Do importance sketching be default. */
    TRUE,					    /* Inverse shading flag. */
    2.5,				        /* Neutral importance decay. */
    90.0				/* Neutral importance frontal decay. */
};
IRIT_GLOBAL_DATA IGDrawUpdateFuncType
    IGDrawIsoCrvsPreFunc = NULL,
    IGDrawIsoCrvsPostFunc = NULL,
    IGDrawSrfPolysPreFunc = NULL,
    IGDrawSrfPolysPostFunc = NULL,
    IGDrawSrfWirePreFunc = NULL,
    IGDrawSrfWirePostFunc = NULL,
    IGDrawSketchPreFunc = NULL,
    IGDrawSketchPostFunc = NULL,
    IGDrawCtlMeshPreFunc = NULL,
    IGDrawCtlMeshPostFunc = NULL;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGGlblDisplayList = NULL;
IRIT_GLOBAL_DATA IGDrawUpdateFuncType
    _IGDrawSrfPolysPreFunc = NULL,
    _IGDrawSrfPolysPostFunc = NULL,
    _IGDrawSrfWirePreFunc = NULL,
    _IGDrawSrfWirePostFunc = NULL,
    _IGDrawSketchPreFunc = NULL,
    _IGDrawSketchPostFunc = NULL,
    _IGDrawCtlMeshPreFunc = NULL,
    _IGDrawCtlMeshPostFunc = NULL;

IRIT_STATIC_DATA int
    IGGlblNumOfIsophotes = 10,
    IGGlblNumOfContours = 10;
IRIT_STATIC_DATA IrtVecType
    IGGlblIsophotesDir = { 0.0, 0.0, 1.0 },
    IGGlblContoursDir = { 0.0, 0.0, 1.0 };


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Make sure all polygons are convex.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Objects to make sure all its polygons are convex.              M
*   Depth:	Of object hierarchy.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGConfirmConvexPolys                                                     M
*****************************************************************************/
void IGConfirmConvexPolys(IPObjectStruct *PObj, int Depth)
{
    for (; PObj != NULL; PObj = PObj -> Pnext) {
	if (IP_IS_OLST_OBJ(PObj)) {
	    IPObjectStruct *PTmp;
	    int i = 0;

	    /* Search in its list. */
	    while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	        IGConfirmConvexPolys(PTmp, Depth + 1);
	}
	else if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj)) {
	    int OldCirc;

	    /* Make sure all polygons are convex. */
	    if ((OldCirc = IPSetPolyListCirc(TRUE)) != TRUE)
	        IPOpenPolysToClosed(PObj -> U.Pl);

	    GMConvexPolyObject(PObj);

	    IPSetPolyListCirc(OldCirc);
	    if (!OldCirc)
	        IPClosedPolysToOpen(PObj -> U.Pl);
	}

	/* No linked list in inner levels. */
	if (Depth > 0)
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get data.   This function also updates the following global variables:   M
* + IPViewMat, IPPrspMat are update with the respective matrices, if found.  M
* + IPWasViewMat, IPWasPrspMat are set to TRUE if above matrices were found. M
* + IGGlblLastProcessMat is updated with the most current matrix value.      M
* + IGGlblViewMode is updated with view mode - perspective or orthographic.  M
*                                                                            *
* PARAMETERS:                                                                M
*   FileNames:   Names of files from which to read the geometry.             M
*   NumFiles:    Number of files in array of names FileNames.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Read geometry, NULL if error or no geometry.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetDataFiles                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGLoadGeometry                                                           M
*****************************************************************************/
IPObjectStruct *IGLoadGeometry(const char **FileNames, int NumFiles)
{
    IPObjectStruct *PObj,
	*PObjs = NULL;

    MatGenUnitMat(IGGlblLastProcessMat);

    IPWasPrspMat = IPWasViewMat = FALSE;

    if (NumFiles > 0) {
	if ((PObjs = IPGetDataFiles(FileNames,
				    NumFiles, TRUE, TRUE)) != NULL) {
	    if (!IGGlblQuickLoad)
		IGConfirmConvexPolys(PObjs, 0);

	    /* If we found a continuous motion matrix - fetch it out. */
	    if ((PObj = IPGetObjectByName("CONT_MAT", PObjs,
					  FALSE)) != NULL &&
		IP_IS_MAT_OBJ(PObj)) {
	        IRIT_HMGN_MAT_COPY(IGGlblLastProcessMat, PObj -> U.Mat);
	        IGGlblContinuousMotion = TRUE;
	    }
	}
	else {
	    return NULL;
	}

	IGGlblViewMode = IPWasPrspMat ? IG_VIEW_PERSPECTIVE
				      : IG_VIEW_ORTHOGRAPHIC;
    }

    return PObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Saves the current viewing matrices.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:     Either perspective or orthographic.                        M
*   Name:         File name to save current view at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSubmitCurrentMat                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSaveCurrentMat                                                         M
*****************************************************************************/
void IGSaveCurrentMat(int ViewMode, char *Name)
{
    char *p, FullName[IRIT_LINE_LEN_LONG];
    int	i, j;
    FILE *f;

    if (Name == NULL)
	Name = IG_DEFAULT_IRIT_MAT;

    strcpy(FullName, Name);
    if ((p = strrchr(FullName, '.')) != NULL &&
	(stricmp(p + 1, IRIT_TEXT_DATA_FILE) != 0 ||
	 stricmp(p + 1, IRIT_BINARY_DATA_FILE) != 0 ||
	 stricmp(p + 1, IRIT_MATRIX_DATA_FILE) != 0))
        *p = 0;
    strcat(FullName, ".");
    strcat(FullName, IRIT_MATRIX_DATA_FILE);

#if defined(AMIGA) && !defined(__SASC)
    if (IRT_STR_ZERO_LEN(Name) || (f = fopen(Name, "w")) == NULL) {
#else
    if (IRT_STR_ZERO_LEN(Name) || (f = fopen(Name, "wt")) == NULL) {
#endif /* defined(AMIGA) && !defined(__SASC) */
	return;
    }

    fprintf(f, "[OBJECT MATRICES\n    [OBJECT VIEW_MAT\n\t[MATRIX");
    for (i = 0; i < 4; i++) {
	fprintf(f, "\n\t    ");
	for (j = 0; j < 4; j++)
	    fprintf(f, "%12.9f ", IPViewMat[i][j]);
    }
    fprintf(f, "\n\t]\n    ]\n");

    if (ViewMode == IG_VIEW_PERSPECTIVE) {
	fprintf(f, "    [OBJECT PRSP_MAT\n\t[MATRIX");
	for (i = 0; i < 4; i++) {
	    fprintf(f, "\n\t    ");
	    for (j = 0; j < 4; j++)
		fprintf(f, "%12.9f ", IPPrspMat[i][j]);
	}
	fprintf(f, "\n\t]\n    ]\n");
    }

    fprintf(f, "]\n");

    fclose(f);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free attribute named Name from all objects in PObjs object's hierarchy and M
* linked list.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjs:        Objects to remove Named attribute from.                    M
*   FreePolygons: If TRUE free all polygonal approximations of freeforms.    M
*   FreeIsolines: If TRUE free all isocurve approximations of freeforms.     M
*   FreeSketches: If TRUE free all sketching strokes of freeforms.           M
*   FreeCtlMesh:  If TRUE free all control meshes of freeforms.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGActiveListFreePolyIsoAttribute                                         M
*****************************************************************************/
void IGActiveListFreePolyIsoAttribute(IPObjectStruct *PObjs,
				      int FreePolygons,
				      int FreeIsolines,
				      int FreeSketches,
				      int FreeCtlMesh)
{
    IPObjectStruct
	*PObj = PObjs;

    for (; PObj != NULL; PObj = PObj -> Pnext) {
	IGActiveFreePolyIsoAttribute(PObj, FreePolygons, FreeIsolines,
				     FreeSketches, FreeCtlMesh);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free attribute named Name from all objects in PObjs object's hierarchy.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjs:     Objects to remove Named attribute from.                       M
*   Name:      Name of attribute to remove.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGActiveListFreeNamedAttribute                                           M
*****************************************************************************/
void IGActiveListFreeNamedAttribute(IPObjectStruct *PObjs, char *Name)
{
    IPObjectStruct
	*PObj = PObjs;

    for (; PObj != NULL; PObj = PObj -> Pnext) {
	IGActiveFreeNamedAttribute(PObj, Name);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free attribute named Name from all objects in PObj object's hierarchy.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         Object to remove Named attribute from.                     M
*   FreePolygons: If TRUE free all polygonal approximations of freeforms.    M
*   FreeIsolines: If TRUE free all isocurve approximations of freeforms.     M
*   FreeSketches: If TRUE free all sketching strokes of freeforms.           M
*   FreeCtlMesh:  If TRUE free all control meshes of freeforms.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGActiveFreePolyIsoAttribute                                             M
*****************************************************************************/
void IGActiveFreePolyIsoAttribute(IPObjectStruct *PObj,
				  int FreePolygons,
				  int FreeIsolines,
				  int FreeSketches,
				  int FreeCtlMesh)
{
    VoidPtr PrepSils;
    IPObjectStruct *PObjPl;

    if ((PrepSils = AttrGetObjectPtrAttrib(PObj, "_silh")) != NULL) {
        GMSilProprocessFree(PrepSils);
	IGActiveFreeNamedAttribute(PObj, "_silh");
    }

    IGActiveFreeNamedAttribute(PObj, "_Disconts");
    IGActiveFreeNamedAttribute(PObj, "_Contours");
    IGActiveFreeNamedAttribute(PObj, "_Isophotes");

#   ifdef IRIT_HAVE_OGL_CG_LIB
	IGCGFreeDTexture(PObj);
#   endif /* IRIT_HAVE_OGL_CG_LIB */

    if (FreePolygons) {
	IGActiveFreeNamedAttribute(PObj, "_polygons");
	IGActiveFreeNamedAttribute(PObj, "_PolygonsHiRes");
	IGActiveFreeNamedAttribute(PObj, "_PolygonsLoRes");
    }
    if (FreeIsolines) {
	IGActiveFreeNamedAttribute(PObj, "_adap_iso");
	IGActiveFreeNamedAttribute(PObj, "_isolines");
	IGActiveFreeNamedAttribute(PObj, "_IsolinesHiRes");
	IGActiveFreeNamedAttribute(PObj, "_IsolinesLoRes");
    }
    if (FreeSketches) {
	IGActiveFreeNamedAttribute(PObj, "_sketches");
    }
    if (FreeCtlMesh) {
	IGActiveFreeNamedAttribute(PObj, "_ctlmesh");
    }
    
    if ((PObjPl = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes")) != NULL)
	IGActiveFreePolyIsoAttribute(PObjPl, FreePolygons, FreeIsolines,
				     FreeSketches, FreeCtlMesh);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Free attribute named Name from Pobj in all object's hierarchy.             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Objects to remove Named attribute from.                       M
*   Name:      Name of attribute to remove.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGActiveFreeNamedAttribute                                               M
*****************************************************************************/
void IGActiveFreeNamedAttribute(IPObjectStruct *PObj, char *Name)
{
    VoidPtr SphCones;
    IPObjectStruct *PObjAttr, *PObjTmp;

    if (strcmp(Name, "_sketches") == 0 &&
	(PObjAttr = AttrGetObjectObjAttrib(PObj, "_sketches")) != NULL &&
	(SphCones = AttrGetObjectPtrAttrib(PObjAttr, "_SphCones")) != NULL)
	GMSphConeQueryFree(SphCones);

    AttrFreeOneAttribute(&PObj -> Attr, Name);

    if ((PObjTmp = AttrGetObjectObjAttrib(PObj, "_Coerced")) != NULL)
        IGActiveFreeNamedAttribute(PObjTmp, Name);

    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int i = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    IGActiveFreeNamedAttribute(PTmp, Name);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Update the BBox of the given object if has none.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Objects to update its BBOX.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGUpdateObjectBBox                                                       M
*****************************************************************************/
void IGUpdateObjectBBox(IPObjectStruct *PObj)
{
    if (IP_IS_GEOM_OBJ(PObj) && !IP_HAS_BBOX_OBJ(PObj)) {
	GMBBBboxStruct
	    *BBox = GMBBComputeBboxObject(PObj);

	IRIT_PT_COPY(PObj -> BBox[0], BBox -> Min);
	IRIT_PT_COPY(PObj -> BBox[1], BBox -> Max);

	IP_SET_BBOX_OBJ(PObj);
    }

    if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PTmp;
	int i = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    IGUpdateObjectBBox(PTmp);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the global view matrix IPViewMat with given matrix Mat           M
* while preserving the scaling factor in the original global view.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat: N.S.F.I.                                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGUpdateViewConsideringScale                                             M
*****************************************************************************/
void IGUpdateViewConsideringScale(IrtHmgnMatType Mat)
{
    int i, j;
    IrtRType
	Scale = MatScaleFactorMatrix(IPViewMat) / MatScaleFactorMatrix(Mat);
    IrtHmgnMatType TmpMat;

    MatGenMatUnifScale(Scale, TmpMat);
    MatMultTwo4by4(Mat, Mat, TmpMat);

    /* Copy the 3 by 3 block of rotation/scale, leaving translation intact. */
    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	    IPViewMat[i][j] = Mat[i][j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the minimal distance in PObj to the line defined by LinePos and    M
* LineDir.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         List of objects to search for a minimal distance.          M
*   MinPl:	  Poly with the minimal distance in PObj.		     M
*   MinPt:        Closest point on the picked object.			     M
*   MinPlIsPolyline:  TRUE if MinPl is a polyline, FALSE if a polygon.       M
*   LinePos:      A point on the line to test against.                       M
*   LineDir:      The direction of the line.                                 M
*   HitDepth:	  In case of zero distance (the ray hits a polygon, update   M
*                 the closest depth.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:     The minimal distance found to the line.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGFindMinimalDist                                                        M
*****************************************************************************/
IrtRType IGFindMinimalDist(IPObjectStruct *PObj,
			   IPPolygonStruct **MinPl,
			   IrtPtType MinPt,
			   int *MinPlIsPolyline,
			   IrtPtType LinePos,
			   IrtVecType LineDir,
			   IrtRType *HitDepth)
{
    IrtRType d1, d2, IndexFrac,
	Dist = IRIT_INFNTY;
    IPObjectStruct *PObjTmp;

    *MinPlIsPolyline = TRUE;
    *MinPl = NULL;
    *HitDepth = IRIT_INFNTY;

    if (!IP_HAS_BBOX_OBJ(PObj))
	return IRIT_INFNTY;

    if (UserMinDistLineBBox(LinePos, LineDir, PObj -> BBox) <
							  IGGlblMinPickDist) {
	IrtPtType Pt;
	CagdRType *R;

	switch (PObj -> ObjType) {
	    case IP_OBJ_POINT:
	    case IP_OBJ_VECTOR:
	    case IP_OBJ_CTLPT:
	        switch (PObj -> ObjType) {
		    case IP_OBJ_POINT:
		        IRIT_PT_COPY(Pt, PObj -> U.Pt);
			break;
		    case IP_OBJ_VECTOR:
			IRIT_PT_COPY(Pt, PObj -> U.Vec);
			break;
		    case IP_OBJ_CTLPT:
			R = PObj -> U.CtlPt.Coords;
			CagdCoercePointTo(Pt, CAGD_PT_E3_TYPE,
					(CagdRType **) &R,
					-1, PObj -> U.CtlPt.PtType);
			break;
		    default:
			break;
		}
		Dist = GMDistPointLine(Pt, LinePos, LineDir);
		break;
	    case IP_OBJ_POLY:
		if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
		    Dist = UserMinDistLinePolygonList(LinePos, LineDir,
						      PObj -> U.Pl, MinPl,
						      MinPt, HitDepth,
						      &IndexFrac);
		else if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME ||
			 IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS)
		    Dist = UserMinDistLinePolylineList(LinePos, LineDir,
						       PObj -> U.Pl, TRUE,
						       MinPl, MinPt,
						       &IndexFrac);
		*MinPlIsPolyline = IP_IS_POLYLINE_OBJ(PObj);
		break;
	    case IP_OBJ_CURVE:
		if ((PObjTmp = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								     != NULL) {
		    Dist = UserMinDistLinePolylineList(LinePos, LineDir,
						       PObjTmp -> U.Pl, FALSE,
						       MinPl, MinPt, 
						       &IndexFrac);
		    *MinPlIsPolyline = TRUE;
		}
		break;
	    case IP_OBJ_SURFACE:
	    case IP_OBJ_TRIMSRF:
	    case IP_OBJ_TRIVAR:
	    case IP_OBJ_TRISRF:
	    case IP_OBJ_MODEL:
		d1 = d2 = IRIT_INFNTY;
		if (IGGlblDrawSurfacePoly &&
		    (PObjTmp = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								     != NULL) {
		    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
		        d1 = UserMinDistLinePolygonList(LinePos, LineDir,
							PObjTmp -> U.Pl,
							MinPl,	MinPt,
							HitDepth,
							&IndexFrac);
		    else if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME ||
			     IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS)
		        d1 = UserMinDistLinePolylineList(LinePos, LineDir,
							 PObjTmp -> U.Pl,
							 TRUE, MinPl, MinPt,
							 &IndexFrac);
		    *MinPlIsPolyline = IP_IS_POLYLINE_OBJ(PObjTmp);
		}

		if (IGGlblDrawSurfaceWire &&
		    (PObjTmp = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								     != NULL) {
		    d2 = UserMinDistLinePolylineList(LinePos, LineDir,
						     PObjTmp -> U.Pl, FALSE,
						     MinPl, MinPt,
						     &IndexFrac);
		    *MinPlIsPolyline = TRUE;
		}
		Dist = IRIT_MIN(d1, d2);
		break;
	    case IP_OBJ_MULTIVAR:
		if ((PObjTmp = AttrGetObjectObjAttrib(PObj,
						      "_Coerced")) != NULL) {
		    /* Is a curve/surface/trivar - look at the coerced ver. */
		    return IGFindMinimalDist(PObjTmp, MinPl, MinPt,
					     MinPlIsPolyline,
					     LinePos, LineDir, HitDepth);
		}
	    default:
		break;
	}
    }

    return Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing of the surface on the fly if needed      M
* and display it.                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A surface(s) object.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolygonSketches                                                    M
*****************************************************************************/
void IGDrawPolygonSketches(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjSketches;

    if ((PObjSketches = AttrGetObjectObjAttrib(PObj, "_sketches")) == NULL) {
	PObjSketches = IGGenPolygonSketches(PObj, 1.0);

	if (IGGlblCacheGeom)
	    AttrSetObjectObjAttrib(PObj, "_sketches", PObjSketches, FALSE);
    }

    IGSketchDrawPolygons(PObjSketches);

    if (!IGGlblCacheGeom)
	IPFreeObject(PObjSketches);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing for the given object with the prescribed M
* fineness.    		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A surface(s) object.                                          M
*   FineNess:  Relative fineness to approximate PObj with.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sketch data.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGenPolygonSketches                                                     M
*****************************************************************************/
IPObjectStruct *IGGenPolygonSketches(IPObjectStruct *PObj, IrtRType FineNess)
{
    IrtRType
        RelativeFineNess = AttrGetObjectRealAttrib(PObj, "res_sketch");
    IPObjectStruct *PPtsObj1, *PPtsObj2, *PPtsObj;

    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;

    PPtsObj1 = IGSketchGenPolySketches(PObj, RelativeFineNess * FineNess,
				       FALSE);
    if (PPtsObj1 -> U.Pl -> PVertex != NULL) {
        PPtsObj1 -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	AttrSetObjectPtrAttrib(PPtsObj1, "_SphCones",
			       GMSphConeQueryInit(PPtsObj1));
    }

    PPtsObj2 = IGSketchGenPolyImportanceSketches(PObj,
						 &IGSketchParam,
						 FineNess);

    PPtsObj = IPGenLISTObject(PPtsObj1);
    IPListObjectInsert(PPtsObj, 1, PPtsObj2);
    IPListObjectInsert(PPtsObj, 2, NULL);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the boundary and silhouette edges of a polygonal object.            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal object.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolySilhBndry                                                      M
*****************************************************************************/
void IGDrawPolySilhBndry(IPObjectStruct *PObj)
{
    IGDrawPolySilhouette(PObj);
    IGDrawPolyBoundary(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the silhouette edges of a polygonal object.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal object.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolySilhouette                                                     M
*****************************************************************************/
void IGDrawPolySilhouette(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceSilh) {
        VoidPtr PrepSils;
	IPObjectStruct *PObjSilh;

	if ((PrepSils = AttrGetObjectPtrAttrib(PObj, "_silh")) == NULL) {
	    GMVrtxListToCircOrLin(PObj -> U.Pl, TRUE);
	    BoolGenAdjacencies(PObj);
	    PrepSils = GMSilPreprocessPolys(PObj, IG_SILH_GRID_SIZE);
	    GMVrtxListToCircOrLin(PObj -> U.Pl, FALSE);

	    /* The following is actually a memory leak as when this PObj is  */
	    /* freed this pointer will not be released!.		     */
	    if (IGGlblCacheGeom)
	        AttrSetObjectPtrAttrib(PObj, "_silh", PrepSils);
	}
	PObjSilh = GMSilExtractSil(PrepSils, IGGlblCrntViewMat);
	IGDrawPolyFuncPtr(PObjSilh);
	IPFreeObject(PObjSilh);

	if (!IGGlblCacheGeom)
	    GMSilProprocessFree(PrepSils);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the boundary edges of a polygonal object.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal object.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyBoundary                                                       M
*****************************************************************************/
void IGDrawPolyBoundary(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceBndry) {
        IPObjectStruct *PObjBndry;

	if ((PObjBndry = AttrGetObjectObjAttrib(PObj, "_bndry")) == NULL) {
	    GMVrtxListToCircOrLin(PObj -> U.Pl, TRUE);
	    BoolGenAdjacencies(PObj);
	    PObjBndry = GMSilExtractBndry(PObj);
	    GMVrtxListToCircOrLin(PObj -> U.Pl, FALSE);

	    if (IGGlblCacheGeom)
	        AttrSetObjectObjAttrib(PObj, "_bndry", PObjBndry, FALSE);
	}
	IGDrawPolyFuncPtr(PObjBndry);

	if (!IGGlblCacheGeom)
	    IPFreeObject(PObjBndry);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setup information for contouring.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y, z:    Direction of contouring.                                     M
*   n:		Number of desired contours.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old number of contours.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyContoursSetup                                                  M
*****************************************************************************/
int IGDrawPolyContoursSetup(IrtRType x, IrtRType y, IrtRType z, int n)
{
   int OldN = IGGlblNumOfContours;

   IGGlblNumOfContours = n;
   IGGlblContoursDir[0] = x;
   IGGlblContoursDir[1] = y;
   IGGlblContoursDir[2] = z;

   IRIT_VEC_NORMALIZE(IGGlblContoursDir);

   return OldN;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw XY parallel contours to the given model.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal object.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyContours                                                       M
*****************************************************************************/
void IGDrawPolyContours(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjContours, *PTmp;

    if (IGGlblDrawSurfaceContours) {
        if ((PObjContours = AttrGetObjectObjAttrib(PObj,
						   "_Contours")) == NULL) {
	    int NumZContours;
	    IrtRType Dz, Z, ZMax;
	    GMBBBboxStruct *BBox;
	    IPObjectStruct
	        *PObjPl = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes");
	    IrtHmgnMatType Mat, InvMat;
	
	    if (PObjPl == NULL)
	        PObjPl = PObj;
	    if (!IP_IS_POLY_OBJ(PObjPl))
	        return;
	    if ((PObjContours = AttrGetObjectObjAttrib(PObjPl,
						      "_Contours")) == NULL) {
	        NumZContours = AttrGetObjectIntAttrib(PObj, "NumZContours");
		if (IP_ATTR_IS_BAD_INT(NumZContours))
		    NumZContours = IGGlblNumOfContours;

		/* Rotate the Poly object to the proper orientation and     */
		/* make sure polygons have circular vertex lists.	    */
		GMGenMatrixZ2Dir(Mat, IGGlblContoursDir);
		MatInverseMatrix(Mat, InvMat);
		PObjPl = GMTransformObject(PObjPl, InvMat);

		GMVrtxListToCircOrLin(PObjPl -> U.Pl, TRUE);
		BBox = GMBBComputeBboxObject(PObjPl);

		Dz = (BBox -> Max[2] - BBox -> Min[2]) / NumZContours;
		Z = BBox -> Min[2] + Dz * 0.5;
		ZMax = BBox -> Max[2];

		BooleanMultiCONTOUR(PObjPl, Z, 2, TRUE, FALSE);	   /* Init. */
		for ( ; Z < ZMax; Z += Dz) {
		    PTmp = BooleanMultiCONTOUR(PObjPl, Z, 2, FALSE, FALSE);

		    if (PTmp != NULL) {
		        assert(IP_IS_POLY_OBJ(PTmp));

			if (PObjContours == NULL)
			    PObjContours = PTmp;
			else {
			    IPPolygonStruct
				*Pl = PTmp -> U.Pl;

			    PTmp -> U.Pl = NULL;
			    IPFreeObject(PTmp);

			    if (Pl != NULL) {
			        IPGetLastPoly(Pl) -> Pnext = PObjContours -> U.Pl;
				PObjContours -> U.Pl = Pl;
			    }
			}
		    }
		}
		BooleanMultiCONTOUR(PObjPl, Z, 2, FALSE, TRUE);	   /* Done. */
		IPFreeObject(PObjPl);

		if (PObjContours != NULL) {
		    /* Map the contours back to the model's location. */
		  PObjPl = GMTransformObject(PObjContours, Mat);
		  IPFreeObject(PObjContours);
		  PObjContours = PObjPl;

		  PObjContours -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

		  if (IGGlblCacheGeom)
		      AttrSetObjectObjAttrib(PObj, "_Contours",
					     PObjContours, FALSE);
		}
	    }
	}

	if (PObjContours != NULL) {
	    IGDrawPolyFuncPtr(PObjContours);

	    if (!IGGlblCacheGeom)
		IPFreeObject(PObjContours);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setup information for isophotes' drawings.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y, z:    Direction of isophotes' view.                                M
*   n:		Number of desired isophotes.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old number of isophotes.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyIsophotesSetup                                                 M
*****************************************************************************/
int IGDrawPolyIsophotesSetup(IrtRType x, IrtRType y, IrtRType z, int n)
{
   int OldN = IGGlblNumOfIsophotes;

   IGGlblNumOfIsophotes = n;
   IGGlblIsophotesDir[0] = x;
   IGGlblIsophotesDir[1] = y;
   IGGlblIsophotesDir[2] = z;

   IRIT_VEC_NORMALIZE(IGGlblIsophotesDir);

   return OldN;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw Isophotes from the Z direction to the given model.	     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal object.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyIsophotes                                                      M
*****************************************************************************/
void IGDrawPolyIsophotes(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjIsophotes;

    if (IGGlblDrawSurfaceIsophotes) {
        if ((PObjIsophotes = AttrGetObjectObjAttrib(PObj,
						    "_Isophotes")) == NULL) {
	    int NumZIsophotes;
	    IrtRType DAlpha, Alpha;
	    IPObjectStruct
	        *PObjPl = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes");
				
	    if (PObjPl == NULL)
	        PObjPl = PObj;
	    if (!IP_IS_POLY_OBJ(PObjPl))
	        return;

	    PObjPl = GMConvertPolysToTriangles(PObjPl);

	    NumZIsophotes = AttrGetObjectIntAttrib(PObj, "NumZIsophotes");
	    if (IP_ATTR_IS_BAD_INT(NumZIsophotes))
	        NumZIsophotes = IGGlblNumOfIsophotes;

	    DAlpha = 180.0 / NumZIsophotes;
	    Alpha = DAlpha * 0.5;

	    for ( ; Alpha < 180.0; Alpha += DAlpha) {
	        IPPolygonStruct
		    *Pls = GMPolyPropFetchIsophotes(PObjPl -> U.Pl,
						    IGGlblIsophotesDir, Alpha);
		if (Pls != NULL) {
		    if (PObjIsophotes == NULL)
			PObjIsophotes = IPGenPOLYLINEObject(Pls);
		    else {
			IPGetLastPoly(Pls) -> Pnext = PObjIsophotes -> U.Pl;
			PObjIsophotes -> U.Pl = Pls;
		    }
		}
	    }

	    IPFreeObject(PObjPl);

	    if (PObjIsophotes != NULL) {
		PObjIsophotes -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

		if (IGGlblCacheGeom)
		    AttrSetObjectObjAttrib(PObj, "_Isophotes",
					PObjIsophotes, FALSE);
	    }
	}

	if (PObjIsophotes != NULL) {
	    IGDrawPolyFuncPtr(PObjIsophotes);

	    if (!IGGlblCacheGeom)
		IPFreeObject(PObjIsophotes);
	}
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the discont edges of an object, if any.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polyline object.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolyDiscontinuities                                                M
*****************************************************************************/
void IGDrawPolyDiscontinuities(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceDiscont) {
        IPObjectStruct *PObjDiscont;

	if ((PObjDiscont = AttrGetObjectObjAttrib(PObj,
						  "_Disconts")) == NULL) {
	    PObjDiscont = GMSilExtractDiscont(PObj, IG_DISCONT_MIN_ANGLE);

	    if (IGGlblCacheGeom)
	        AttrSetObjectObjAttrib(PObj, "_Disconts", PObjDiscont, FALSE);
	}
	IGDrawPolyFuncPtr(PObjDiscont);

	if (!IGGlblCacheGeom)
	    IPFreeObject(PObjDiscont);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the proper isoparametric curve approximation of the object.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To get the iso curves' approximation                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The iso curve's approximation.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGetObjIsoLines                                                         M
*****************************************************************************/
IPObjectStruct *IGGetObjIsoLines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if (IGGlblManipulationActive) {
	if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesLoRes"))
								      == NULL)
	    PObjPolylines = (IPObjectStruct *)
		AttrGetObjectPtrAttrib(PObj, "_IsolinesLoRes");
    }
    else
	PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes");

    return PObjPolylines;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the proper polygonal approximation of the object.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     To get the polygonal approximation                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The polygonal approximation.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGetObjPolygons                                                         M
*****************************************************************************/
IPObjectStruct *IGGetObjPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons;

    if (IGGlblManipulationActive) {
	if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsLoRes"))
								       == NULL)
	    PObjPolygons = (IPObjectStruct *)
		AttrGetObjectPtrAttrib(PObj, "_PolygonsLoRes");
    }
    else
	PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes");

    return PObjPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads in a texture mapping image if object has "ptexture" attribute and  M
* set up the texture for further processing.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to extract its texture mapping function if has one.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if object has texture mapping, FALSE otherwise.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGInitSrfTexture                                                         M
*****************************************************************************/
int IGInitSrfTexture(IPObjectStruct *PObj)
{
    int MaxX, MaxY,
	Alpha = TRUE,				    /* Get alpha if you can. */
	FlipXYImage = FALSE,
	HasXYZScale = FALSE;
    char TextureFName[IRIT_LINE_LEN_LONG];
    const char
        *Texture = AttrGetObjectStrAttrib(PObj, "ptexture");
    IrtImgPixelStruct *Image;
    IrtRType ScaleXYZ[3];

    if (Texture == NULL)
	return FALSE;

    if ((Image = AttrGetObjectPtrAttrib(PObj, "_ImageTexture")) != NULL)
	return TRUE;

    if (!IrtImgParsePTextureString(Texture, TextureFName,
				   ScaleXYZ, &FlipXYImage)) {
        IRIT_WARNING_MSG_PRINTF("Failed to parse texture mapping image file \"%s\".  Texture ignored\n",
				Texture);
        return FALSE;
    }
    HasXYZScale = !IRIT_APX_EQ(ScaleXYZ[2], IRIT_INFNTY);

    IrtImgReadImageXAlign(4);            /* OGL requires 4-bytes words in X. */

    if ((Image = IrtImgReadImage2(TextureFName, &MaxX, &MaxY,
				  &Alpha)) != NULL) {
	int Width = MaxX + 1,
	    Height = MaxY + 1;

	if (FlipXYImage) {
	    Image = IrtImgFlipXYImage(Image, MaxX, MaxY, Alpha);
	    IRIT_SWAP(int, Width, Height);
	}

	AttrSetObjectPtrAttrib(PObj, "_ImageTexture", Image);
	AttrSetObjectIntAttrib(PObj, "_ImageWidth", Width);
	AttrSetObjectIntAttrib(PObj, "_ImageHeight", Height);
	AttrSetObjectIntAttrib(PObj, "_ImageAlpha", Alpha);
	AttrSetObjectRealAttrib(PObj, "_ImageScaleX", ScaleXYZ[0]);
	AttrSetObjectRealAttrib(PObj, "_ImageScaleY", ScaleXYZ[1]);

	if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj))
	    GMGenUVValsForPolys(PObj, ScaleXYZ[0], ScaleXYZ[1], ScaleXYZ[2],
				HasXYZScale);

	return TRUE;
    }
    else {
        IRIT_WARNING_MSG_PRINTF("Failed to read texture mapping image file \"%s\".  Texture ignored\n",
				Texture);

	/* Remove the "ptexture" attribute so we would not try again... */
        AttrFreeObjectAttribute(PObj, "ptexture");

	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Processes the given event. Returns TRUE if redraw of view window is needed.M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:          Event to process.                                        M
*   ChangeFactor:   A continuous scale between -1 and 1 to quantify the      M
*                   change to apply according to the event type.	     M
*		    For composed operation contains both X and Y information.M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE if refresh is needed.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleContinuousMotion                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDefaultProcessEvent                                                    M
*****************************************************************************/
int IGDefaultProcessEvent(IGGraphicEventType Event, IrtRType *ChangeFactor)
{
    int UpdateView = TRUE;
    IrtHmgnMatType Mat, TMat;

    MatGenUnitMat(Mat);

    switch (Event) {
	case IG_EVENT_SCR_OBJ_TGL:    /* Its Coordinate system - toggle it. */
	    UpdateView = FALSE;
	    break;
	case IG_EVENT_PERS_ORTHO_TGL:	      /* Its View mode - toggle it. */
	    break;
	case IG_EVENT_PERS_ORTHO_Z: /* Its Perspective Z focal point modif. */
	    if (IGGlblViewMode != IG_VIEW_PERSPECTIVE) {
		UpdateView = FALSE;
		break;
	    }
	    /* Make it between 0.5 and 1.5: */
	    *ChangeFactor = *ChangeFactor / 2.0 + 1.0;
	    IPPrspMat[2][2] *= *ChangeFactor;
	    IPPrspMat[2][3] *= *ChangeFactor;
	    IPPrspMat[3][2] *= *ChangeFactor;
	    break;
	case IG_EVENT_ROTATE:		    /* Its rotation in both X and Y. */
	    /* Doing it seperatly for X and Y is not the right thing, but it */
	    /* does work for us, in interactive use.			     */
	    MatGenMatRotY1(IRIT_DEG2RAD(ChangeFactor[0] * IG_MAX_ROTATE_ANGLE / 150),
									TMat);
	    MatGenMatRotX1(IRIT_DEG2RAD(-ChangeFactor[1] * IG_MAX_ROTATE_ANGLE / 150),
									Mat);
	    MatMultTwo4by4(Mat, TMat, Mat);
	    break;
	case IG_EVENT_ROTATE_X:		   /* Its rotation along the X axis. */
	    MatGenMatRotX1(IRIT_DEG2RAD(*ChangeFactor * IG_MAX_ROTATE_ANGLE), Mat);
	    break;
	case IG_EVENT_ROTATE_Y:		   /* Its rotation along the Y axis. */
	    MatGenMatRotY1(IRIT_DEG2RAD(*ChangeFactor * IG_MAX_ROTATE_ANGLE), Mat);
	    break;
	case IG_EVENT_ROTATE_Z:		   /* Its rotation along the Z axis. */
	    MatGenMatRotZ1(IRIT_DEG2RAD(*ChangeFactor * IG_MAX_ROTATE_ANGLE), Mat);
	    break;
	case IG_EVENT_TRANSLATE:	 /* Its translation in both X and Y. */
	    MatGenMatTrans(ChangeFactor[0] / 300.0, ChangeFactor[1] / 300.0,
			   0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_X:	/* Its translation along the X axis. */
	    MatGenMatTrans(*ChangeFactor * IG_MAX_TRANSLATE_FACTOR,
			   0.0, 0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_Y:	/* Its translation along the Y axis. */
	    MatGenMatTrans(0.0, *ChangeFactor * IG_MAX_TRANSLATE_FACTOR,
			   0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_Z:	/* Its translation along the Z axis. */
	    MatGenMatTrans(0.0, 0.0,
			   *ChangeFactor * IG_MAX_TRANSLATE_FACTOR, Mat);
	    break;
	case IG_EVENT_SCALE:		      /* Its scaling along all axes. */
	    if (*ChangeFactor >= 0.0)		      /* Make it around 1... */
	        *ChangeFactor = *ChangeFactor * IG_MAX_SCALE_FACTOR + 1.0;
	    else
	        *ChangeFactor =
			    1.0 / (-*ChangeFactor * IG_MAX_SCALE_FACTOR + 1.0);
	    MatGenMatUnifScale(*ChangeFactor, Mat);
	    break;
	case IG_EVENT_NEAR_CLIP:		     /* Near plane clipping. */
	    IGGlblZMinClip += *ChangeFactor * IG_MAX_CLIP_FACTOR;
	    IGGlblEyeDistance =
			    1.0 / (IRIT_SQR(IRIT_SQR(IGGlblZMinClip)) + 0.001);
	    break;
	case IG_EVENT_FAR_CLIP:		              /* Far plane clipping. */
	    IGGlblZMaxClip += *ChangeFactor * IG_MAX_CLIP_FACTOR;
	    break;
        case IG_EVENT_ANIMATION:
	    IGGlblAnimation = TRUE;
	    GMAnimGetAnimInfoText(&IGAnimation);
	    GMAnimDoAnimation(&IGAnimation, IGGlblDisplayList);
	    IGGlblAnimation = FALSE;
	    break;
	case IG_EVENT_DEPTH_CUE:
	    break;
	case IG_EVENT_PUSH_MATRIX:
	    IRIT_GEN_COPY(IGGlblPushViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    IRIT_GEN_COPY(IGGlblPushPrspMat, IPPrspMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_EVENT_POP_MATRIX:
	    IRIT_GEN_COPY(IPViewMat, IGGlblPushViewMat, sizeof(IrtHmgnMatType));
	    IRIT_GEN_COPY(IPPrspMat, IGGlblPushPrspMat, sizeof(IrtHmgnMatType));
	    break;
	default:
	    UpdateView = FALSE;
    }

    if (UpdateView) {
	switch (IGGlblTransformMode) {/* Udpate the global viewing matrix. */
	    case IG_TRANS_SCREEN:
	        MatMultTwo4by4(IPViewMat, IPViewMat, Mat);
		break;
	    case IG_TRANS_OBJECT:
		MatMultTwo4by4(IPViewMat, Mat, IPViewMat);
		break;
	}

        IRIT_HMGN_MAT_COPY(IGGlblLastProcessMat, Mat);
    }
    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handle the event of a pop up window. This is the default handler which can M
* be invoked by other specific handlers for event they do not care about.    M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       State event type to handle.                                 M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Not used.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if needs to refresh.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDefaultStateHandler                                                    M
*****************************************************************************/
int IGDefaultStateHandler(int State, int StateStatus, int Refresh)
{
    int UpdateView = TRUE;
    IrtHmgnMatType Mat;

    switch (State) {
	case IG_STATE_MOUSE_SENSITIVE:
	    if (StateStatus == IG_STATE_INC)
	        IGGlblChangeFactor *= 2.0;
	    else
	        IGGlblChangeFactor *= 0.5;
	    break;
	case IG_STATE_SCR_OBJ_TGL:
	    if (StateStatus == IG_STATE_TGL) {
		IGGlblTransformMode =
		    IGGlblTransformMode == IG_TRANS_OBJECT ?
					   IG_TRANS_SCREEN :
					   IG_TRANS_OBJECT;
	    }
	    else {
	        IGGlblTransformMode =
		    StateStatus == IG_STATE_ON ? IG_TRANS_SCREEN
					       : IG_TRANS_OBJECT;
	    }
	    UpdateView = FALSE;
	    break;
	case IG_STATE_CONT_MOTION:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblContinuousMotion = !IGGlblContinuousMotion;
	    else 
		IGGlblContinuousMotion = StateStatus == IG_STATE_ON;
	    UpdateView = FALSE;
	    break;
	case IG_STATE_NRML_ORIENT:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblFlipNormalOrient = !IGGlblFlipNormalOrient;
	    else 
		IGGlblFlipNormalOrient = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_PERS_ORTHO_TGL:
	    if (StateStatus == IG_STATE_TGL) {
		IGGlblViewMode = IGGlblViewMode == IG_VIEW_ORTHOGRAPHIC ?
					           IG_VIEW_PERSPECTIVE :
					           IG_VIEW_ORTHOGRAPHIC;
	    }
	    else {
	        IGGlblViewMode =
		    StateStatus == IG_STATE_ON ? IG_VIEW_PERSPECTIVE
					       : IG_VIEW_ORTHOGRAPHIC;
	    }
	    break;
	case IG_STATE_BACK_FACE_CULL:
	    if (StateStatus == IG_STATE_TGL)
		IGGlblBackFaceCull = !IGGlblBackFaceCull;
	    else 
		IGGlblBackFaceCull = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_SHADING_MODEL:
	    switch (IGGlblShadingModel) {
	        case IG_SHADING_NONE:
	            IGGlblShadingModel = IG_SHADING_BACKGROUND;
		    break;
		case IG_SHADING_BACKGROUND:
		    IGGlblShadingModel = IG_SHADING_FLAT;
		    break;
		case IG_SHADING_FLAT:
		    IGGlblShadingModel = IG_SHADING_GOURAUD;
		    break;
		case IG_SHADING_GOURAUD:
		    IGGlblShadingModel = IG_SHADING_PHONG;
		    break;
		case IG_SHADING_PHONG:
		    IGGlblShadingModel = IG_SHADING_NONE;
		    break;
	    }
	    break;
	case IG_STATE_DEPTH_CUE:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDepthCue = !IGGlblDepthCue;
	    else 
		IGGlblDepthCue = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_INTERNAL:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawInternal = !IGGlblDrawInternal;
	    else 
		IGGlblDrawInternal = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_VNORMAL:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawVNormal = !IGGlblDrawVNormal;
	    else 
		IGGlblDrawVNormal = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_PNORMAL:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawPNormal = !IGGlblDrawPNormal;
	    else 
		IGGlblDrawPNormal = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_MESH:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceMesh = !IGGlblDrawSurfaceMesh;
	    else 
		IGGlblDrawSurfaceMesh = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_WIRE:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceWire = !IGGlblDrawSurfaceWire;
	    else 
		IGGlblDrawSurfaceWire = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_BNDRY:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceBndry = !IGGlblDrawSurfaceBndry;
	    else 
		IGGlblDrawSurfaceBndry = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_SILH:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceSilh = !IGGlblDrawSurfaceSilh;
	    else 
		IGGlblDrawSurfaceSilh = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_POLY:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfacePoly = !IGGlblDrawSurfacePoly;
	    else 
		IGGlblDrawSurfacePoly = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_POLYGONS:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawPolygons = !IGGlblDrawPolygons;
	    else 
		IGGlblDrawPolygons = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_SKTCH:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceSketch = !IGGlblDrawSurfaceSketch;
	    else 
		IGGlblDrawSurfaceSketch = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_SRF_RFLCT_LNS:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDrawSurfaceRflctLns = !IGGlblDrawSurfaceRflctLns;
	    else 
		IGGlblDrawSurfaceRflctLns = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_DRAW_STYLE:
	    switch (IGGlblDrawStyle) {
	        case IG_STATE_DRAW_STYLE_WIREFRAME:
		    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_SOLID;
		    break;
	        case IG_STATE_DRAW_STYLE_SOLID:
		    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_POINTS;
		    break;
	        case IG_STATE_DRAW_STYLE_POINTS:
	        default:
		    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_WIREFRAME;
		    break;
	    }
	    break;
	case IG_STATE_DOUBLE_BUFFER:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblDoDoubleBuffer = !IGGlblDoDoubleBuffer;
	    else 
		IGGlblDoDoubleBuffer = StateStatus == IG_STATE_ON;
	    break;
	case IG_STATE_ANTI_ALIASING:
	    switch (IGGlblAntiAliasing) {
	        case IG_STATE_ANTI_ALIAS_OFF:
		    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_ON;
		    break;
	        case IG_STATE_ANTI_ALIAS_ON:
		    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_BLEND;
		    break;
	        case IG_STATE_ANTI_ALIAS_BLEND:
	        default:
		    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_OFF;
		    break;
	    }
	    break;
	case IG_STATE_FOUR_PER_FLAT:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblFourPerFlat = !IGGlblFourPerFlat;
	    else 
		IGGlblFourPerFlat = StateStatus == IG_STATE_ON;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, FALSE, FALSE, FALSE);
	    break;
	case IG_STATE_NUM_ISOLINES:
	    if (StateStatus == IG_STATE_INC) {
	        if (IGGlblNumOfIsolines == 0)
		    IGGlblNumOfIsolines = 1;
		else
		    IGGlblNumOfIsolines *= 2;
	    }
	    else if (StateStatus == IG_STATE_DEC)	        
	        IGGlblNumOfIsolines /= 2;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, FALSE, FALSE);
	    break;
	case IG_STATE_LENGTH_VECTORS:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblNormalSize *= 2.0;
		IGGlblPointSize *= 2.0;
	    }
	    else if (StateStatus == IG_STATE_DEC) {        
	        IGGlblNormalSize /= 2.0;
		IGGlblPointSize /= 2.0;
	    }
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    break;
	case IG_STATE_POLYGON_APPROX:
	    if (StateStatus == IG_STATE_INC) {
	        if (IGGlblPolygonOptiApprox == 0) {
		    IGGlblPlgnFineness *= 2.0;
		}
		else {
		    IGGlblPlgnFineness /= 2.0;
		}
	    }
	    else if (StateStatus == IG_STATE_DEC) {        
	        if (IGGlblPolygonOptiApprox == 0) {
		    IGGlblPlgnFineness /= 2.0;
		    if (IGGlblPolygonOptiApprox == 0 &&
			IGGlblPlgnFineness < 2.0)
		        IGGlblPlgnFineness = 2.0;
		}
		else {
		    IGGlblPlgnFineness *= 2.0;
		}
	    }
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, TRUE, TRUE, FALSE);
	    break;
	case IG_STATE_SAMP_PER_CRV_APPROX:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblPllnFineness *=
		    IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM ? 2
									: 0.5;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblPllnFineness *=
		    IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM ? 0.5
									: 2;

		if (IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM &&
		    IGGlblPllnFineness < 2)
		    IGGlblPllnFineness = 2;
	    }
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, TRUE, FALSE);
	    break;
	case IG_STATE_POLY_APPROX:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblPlgnFineness *= IGGlblPolygonOptiApprox == 0 ? 2.0 : 0.5;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblPlgnFineness *= IGGlblPolygonOptiApprox == 0 ? 0.5 : 2.0;

		if (IGGlblPolygonOptiApprox == 0 && IGGlblPlgnFineness < 2.0)
		    IGGlblPlgnFineness = 2.0;
	    }
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, FALSE, FALSE, FALSE);
	    break;
	case IG_STATE_VIEW_FRONT:
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotZ1(IRIT_DEG2RAD(0.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_SIDE:
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotY1(IRIT_DEG2RAD(90.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_TOP:
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotX1(IRIT_DEG2RAD(90.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_ISOMETRY:
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    IGUpdateViewConsideringScale(IGGlblIsometryViewMat);
	    break;
	case IG_STATE_VIEW_4:
	    break;
	case IG_STATE_CLEAR_VIEW:
	    IPFreeObjectList(IGGlblDisplayList);
	    IGGlblDisplayList = NULL;
	    IrtImgReadClrCache();             /* Free all used texture maps. */
	    break;
	case IG_STATE_WIDTH_LINES:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblLineWidth *= 2;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblLineWidth /= 2;
		if (IGGlblLineWidth < 1)
		    IGGlblLineWidth = 1;
	    }
	    break;
	case IG_STATE_WIDTH_POINTS:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblPointSize *= 2;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblPointSize /= 2;
	    }
	    break;
	case IG_STATE_NUM_POLY_COUNT:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblCountNumPolys = !IGGlblCountNumPolys;
	    else 
		IGGlblCountNumPolys = StateStatus == IG_STATE_ON;
	    break;	    
	case IG_STATE_FRAME_PER_SEC:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblCountFramePerSec = !IGGlblCountFramePerSec;
	    else 
		IGGlblCountFramePerSec = StateStatus == IG_STATE_ON;
	    break;	    
	case IG_STATE_POLYGON_OPTI:
	    if (StateStatus == IG_STATE_TGL)
	        IGGlblPolygonOptiApprox = !IGGlblPolygonOptiApprox;
	    else
	        IGGlblPolygonOptiApprox = StateStatus == IG_STATE_ON;

	    IGGlblPlgnFineness = IGGlblPolygonOptiApprox ? 0.02 : 20;

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, FALSE, FALSE, FALSE);
	    break;	    
	case IG_STATE_POLYLINE_OPTI:
	    if (StateStatus == IG_STATE_TGL) {
	        if (IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM)
		    IGGlblPolylineOptiApprox = SYMB_CRV_APPROX_TOLERANCE;
		else
		    IGGlblPolylineOptiApprox = SYMB_CRV_APPROX_UNIFORM;
	    }
	    else
	        IGGlblPolylineOptiApprox = 
		    StateStatus == IG_STATE_ON ? SYMB_CRV_APPROX_TOLERANCE
					       : SYMB_CRV_APPROX_UNIFORM;

	    IGGlblPllnFineness =
	        IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_TOLERANCE ?
		    IG_DEFAULT_PLLN_OPTI_FINENESS :
		    IG_DEFAULT_SAMPLES_PER_CURVE;

	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, TRUE, FALSE, FALSE);
	    break;	    
	case IG_STATE_LOWRES_RATIO:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblRelLowresFineNess *= 2;
		if (IGGlblRelLowresFineNess > 1.0)
		    IGGlblRelLowresFineNess = 1.0;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblRelLowresFineNess /= 2;
	    }
	    break;
        case IG_STATE_ANIMATION:
	    IGGlblAnimation = TRUE;
	    GMAnimGetAnimInfoText(&IGAnimation);
	    GMAnimDoAnimation(&IGAnimation, IGGlblDisplayList);
	    IGGlblAnimation = FALSE;
	    break;
	default:
	    UpdateView = FALSE;
	    break;
    }

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the current frame per second.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Start:  TRUE to start timing, FALSE to end it.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGUpdateFPS                                                              M
*****************************************************************************/
void IGUpdateFPS(int Start)
{
    IRIT_STATIC_DATA IrtRType
	LastT = -1;
    IrtRType t;

    if (Start) {
        LastT = IritCPUTime(FALSE);
	return;
    }

    t = IritCPUTime(FALSE);
    if (LastT >= t) {
        IGGlblFramePerSec = 100.0; /* Too fast for us... */
    }
    else {
        /* Average last 4 frame times. */
        IGGlblFramePerSec = (IGGlblFramePerSec * 3 + 1.0 / (t - LastT)) * 0.25;
	if (IGGlblFramePerSec > 100.0)
	    IGGlblFramePerSec = 100.0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting before polygons are drawn.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSrfPolysPreFunc                                                     M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSrfPolysPreFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSrfPolysPreFunc;


    _IGDrawSrfPolysPreFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting after polygons are drawn.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSrfPolysPostFunc                                                    M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSrfPolysPostFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSrfPolysPostFunc;

    _IGDrawSrfPolysPostFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting before wireframe draw.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSrfWirePreFunc                                                      M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSrfWirePreFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSrfWirePreFunc;

    _IGDrawSrfWirePreFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting after wireframe draw.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSrfWirePostFunc                                                     M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSrfWirePostFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSrfWirePostFunc;

    _IGDrawSrfWirePostFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting before sketch style draw.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSketchPreFunc                                                       M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSketchPreFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSketchPreFunc;

    _IGDrawSketchPreFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting after sketch style draw.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetSketchPostFunc                                                      M
*****************************************************************************/
IGDrawUpdateFuncType IGSetSketchPostFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawSketchPostFunc;

    _IGDrawSketchPostFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting before control mesh draw.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetCtlMeshPreFunc                                                      M
*****************************************************************************/
IGDrawUpdateFuncType IGSetCtlMeshPreFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawCtlMeshPreFunc;

    _IGDrawCtlMeshPreFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a call back function to handle setting after control mesh draw.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func: New callback function to use, or NULL to disable.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGDrawUpdateFuncType:   Old callback function used.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetCtlMeshPostFunc                                                     M
*****************************************************************************/
IGDrawUpdateFuncType IGSetCtlMeshPostFunc(IGDrawUpdateFuncType Func)
{
    IGDrawUpdateFuncType
        OldFunc = _IGDrawCtlMeshPostFunc;

    _IGDrawCtlMeshPostFunc = Func;

    return OldFunc;
}
