/*****************************************************************************
*   A NUL device driver. Prints the objects it recieves from server.         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_loc.h"

IRIT_GLOBAL_DATA int
    IGCrvEditActive = FALSE,
    IGSrfEditActive = FALSE,
    IGObjManipNumActiveObjs = 0;
IRIT_GLOBAL_DATA CagdCrvStruct
    *IGCrvEditCurrentCrv = NULL;
IRIT_GLOBAL_DATA CagdSrfStruct
    *IGSrfEditCurrentSrf = NULL;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGCrvEditCurrentObj = NULL,
    *IGSrfEditCurrentObj = NULL,
    *IGCrvEditPreloadEditCurveObj = NULL,
    *IGSrfEditPreloadEditSurfaceObj = NULL,
    **IGObjManipCurrentObjs = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of nuldrvs - NUL graphics driver of IRIT.             	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    IPObjectStruct
	*PObjects = NULL;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("nuldrvs", argc, argv);
    IGProcessCommandMessages(FALSE);

    if (IGGlblStandAlone) {
	fprintf(stderr, "NULDrvs has no meaning running stand alone.\n");
	exit(1);
    }

    if (!IGGlblDebugObjectsFlag && !IGGlblDebugEchoInputFlag) {
	fprintf(stderr, "NULDrvs is pretty useless with neither -d nor -D.\n");
	exit(1);
    }

    while (TRUE) {
	IGReadObjectsFromSocket(IGGlblViewMode, &PObjects);

	if (PObjects != NULL && IP_IS_STR_OBJ(PObjects)) {
	    if (strcmp(PObjects -> U.Str, "EXIT") == 0)
		break;
	}

	IPFreeObjectList(PObjects);
	PObjects = NULL;

	IritSleep(10);
    }

    exit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handles the events of the pop up window.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       Event to handle.                                            M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Do we need to refresh the screen according to what we know  M
*		 on entry.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if we need to refresh the screen.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleState                                                            M
*****************************************************************************/
int IGHandleState(int State, int StateStatus, int Refresh)
{
     return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make some sound.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritBeep                                                               M
*****************************************************************************/
void IGIritBeep(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single object using current modes and transformations.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawObject                                                             M
*****************************************************************************/
void IGDrawObject(IPObjectStruct *PObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Optionally construct a state pop up menu for the driver, if has one.       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCreateStateMenu                                                        M
*****************************************************************************/
void IGCreateStateMenu(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the curve editor, if not already up and hook CrvObj to it.         M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to edit.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupCrvEditor                                                         M
*****************************************************************************/
void IGPopupCrvEditor(IPObjectStruct *CrvObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the surface editor, if not already up and hook SrfObj to it.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to edit.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupSrfEditor                                                         M
*****************************************************************************/
void IGPopupSrfEditor(IPObjectStruct *SrfObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the object editor, if not already up and hook PObj to it.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to edit.                                                 M
*   CloneIt: If TRUE make a copy of given object fist and edit the clone.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupObjEditor                                                         M
*****************************************************************************/
void IGPopupObjEditor(IPObjectStruct *PObj, int CloneIt)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Poly object using current modes and transformations.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to draw.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPoly                                                               M
*****************************************************************************/
void IGDrawPoly(IPObjectStruct *PObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handle picking of objects.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PickEntity:  Type of entity to pick (object, cursor etc.).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickObject                                                       M
*****************************************************************************/
void IGHandlePickObject(IGPickEntityType PickEntity)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Preprocess polygonal model for fast silhouette dependent polygonization    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  Polygonal model to preprocess.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A pointer to the object data structure.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGVGGenDataStruct                                                        M
*****************************************************************************/
IPObjectStruct *IGVGGenDataStruct(IPObjectStruct *PObj)
{
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract a polygonal model from current view, that is silhouette dependent. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  Object to extract the view dependent polygonization.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A pointer to the newly generated object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGVGSelect                                                               M
*****************************************************************************/
IPObjectStruct *IGVGSelect(IPObjectStruct *PObj)
{
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the current matrix in a selected file name.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:   Perspective or orthographics current view mode.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSaveCurrentMatInFile                                                   M
*****************************************************************************/
void IGSaveCurrentMatInFile(int ViewMode)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the current constructed curve.  Invoked when screen is redrawn.     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditRedrawCrv                                                           M
*****************************************************************************/
void CEditRedrawCrv(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently editted curve.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditDetachCurve                                                         M
*****************************************************************************/
void CEditDetachCurve(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attach to a curve object for further editing.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:     Curve object to attach to.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CEditAttachOldDirectly                                                   M
*****************************************************************************/
void CEditAttachOldDirectly(IPObjectStruct *CrvObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw the current constructed surface.  Invoked when screen is redrawn.   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditRedrawSrf                                                           M
*****************************************************************************/
void SEditRedrawSrf(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently editted surface.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditDetachSurface                                                       M
*****************************************************************************/
void SEditDetachSurface(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attach to a surface object for further editing.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface object to attach to.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SEditAttachOldDirectly                                                   M
*****************************************************************************/
void SEditAttachOldDirectly(IPObjectStruct *SrfObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach for the currently transformed object.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipDetachObj                                                      M
*****************************************************************************/
void IGObjManipDetachObj(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Make error message box in printf style.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   Error message.	                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
    fprintf(stderr, "\n%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make yes/no message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Title message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if yes was selected, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritYesNoQuestion                                                      M
*****************************************************************************/
int IGIritYesNoQuestion(char *Msg)
{
    fprintf(stderr,
	    "Yes/No message box not implemented:\n%s\nreturning yes...", Msg);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redirects stdin/out/err to a second console window.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedirectIOToConsole                                                    M
*****************************************************************************/
void IGRedirectIOToConsole(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function initilized the subview mat to front, side, top & Isometry  M
* views.                                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGInitializeSubViewMat                                                   M
*****************************************************************************/
void IGInitializeSubViewMat(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function to enable/disable 4views mode.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Display4Views:  TRUE for 4 views mode.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if a change in views' style occured.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetDisplay4Views                                                       M
*****************************************************************************/
int IGSetDisplay4Views(int Display4Views)
{
    return 0;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves one iteration of the animation sequence as an image.		     M
*									     *
* PARAMETERS:								     M
*   ImageFileName:  File name where to save the current display as an image. M
*									     *
* RETURN VALUE:								     M
*   void								     M
*									     *
* KEYWORDS:								     M
*   IGSaveDisplayAsImage						     M
*****************************************************************************/
void IGSaveDisplayAsImage(char *ImageFileName)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing of the polygonal object, on the fly if   M
* needed, and display it.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSketches:     A sketches object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchDrawPolygons                                                     M
*****************************************************************************/
void IGSketchDrawPolygons(IPObjectStruct *PObjSketches)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing for the given polygonal object.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PlObj:      The polygonal object to process.                             M
*   FineNess:   Relative fineness to approximate the sketchs of PlObj with.  M
*   Importance: If TRUE, we should also compute importance for each point.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The sketch data.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchGenPolySketches                                                  M
*****************************************************************************/
IPObjectStruct *IGSketchGenPolySketches(IPObjectStruct *PlObj,
					IrtRType FineNess,
					int Importance)
{
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Precompute the strokes that emphasize improtant features in the geometry M
* using an importance measure that looks at the diherdal angle of adjacent   M
* polygons.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         Polygonal model to process for importance.	             M
*   SketchParams: NPR sketch parameters' info structure.		     M
*   FineNess:     Of marching steps on the surface.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Traced strokes as piecewise linear approximations.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSketchGenPolySketches                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchGenPolyImportanceSketches                                        M
*****************************************************************************/
IPObjectStruct *IGSketchGenPolyImportanceSketches(IPObjectStruct *PObj,
					    IGSketchParamStruct *SketchParams,
					    IrtRType FineNess)
{
    return NULL;
}

#ifdef IRIT_HAVE_OGL_CG_LIB

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
    return FALSE;
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
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a single object with ffd_texture attributes using current modes    M
* and transformations.		                                             M
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
    return FALSE;
}

#endif /* IRIT_HAVE_OGL_CG_LIB */
