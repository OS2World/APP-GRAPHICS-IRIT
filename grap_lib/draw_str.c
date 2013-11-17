/*****************************************************************************
*   Default string drawing routine common to graphics drivers.	             *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "geom_lib.h"
#include "grap_loc.h"

#define DEFAULT_SCALE 0.1

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single string object using current modes and transformations.       M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A string object to draw.   	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawString                                                             M
*****************************************************************************/
void IGDrawString(IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA IrtVecType
        DefSpace = { DEFAULT_SCALE, 0.0, 0.0 };
    IrtRType
        Scale = AttrGetObjectRealAttrib(PObj, "StrScale");
    IrtVecType *Pos, *Space;
    IPObjectStruct *PObjGeometry,
        *PosObj = AttrGetObjectObjAttrib(PObj, "StrPos"),
        *SpaceObj = AttrGetObjectObjAttrib(PObj, "StrSpace");

    if (PosObj == NULL || !IP_IS_VEC_OBJ(PosObj))
        Pos = NULL;
    else
	Pos = &PosObj -> U.Vec;

    if (SpaceObj == NULL || !IP_IS_VEC_OBJ(SpaceObj)) {
        DefSpace[0] = Scale == IP_ATTR_BAD_REAL ? DEFAULT_SCALE : Scale;
	Space = &DefSpace;
    }
    else
	Space = &SpaceObj -> U.Vec;

    if (Scale == IP_ATTR_BAD_REAL)
        Scale = DEFAULT_SCALE;

    /* Convert the object to a polyline object. */
    if ((PObjGeometry = AttrGetObjectObjAttrib(PObj, "_geometry")) == NULL) {
        IrtHmgnMatType Mat;
        IPObjectStruct
	    *PTmp = GMMakeTextGeometry(PObj -> U.Str, *Space, &Scale);

	if (Pos != NULL) {
	    MatGenMatTrans((*Pos)[0], (*Pos)[1], (*Pos)[2], Mat);
	    PObjGeometry = IPFlattenTree(GMTransformObject(PTmp, Mat));
	    IPFreeObject(PTmp);
	}
	else
	    PObjGeometry = IPFlattenTree(PTmp);

	/* Collect all polylines to a single object. */
	if (PObjGeometry != NULL) {
	    for (PTmp = PObjGeometry -> Pnext;
		 PTmp != NULL;
		 PTmp = PTmp -> Pnext) {
	        PObjGeometry -> U.Pl =
		    IPAppendPolyLists(PObjGeometry -> U.Pl, PTmp -> U.Pl);

		PTmp -> U.Pl = NULL;
	    }

	    IPFreeObjectList(PObjGeometry -> Pnext);
	    PObjGeometry -> Pnext = NULL;
	}

	if (IGGlblCacheGeom && PObjGeometry != NULL) {
	    GMBBBboxStruct
	        *BBox = GMBBComputeBboxObject(PObjGeometry);

	    IRIT_PT_COPY(PObjGeometry -> BBox[0], BBox -> Min);
	    IRIT_PT_COPY(PObjGeometry -> BBox[1], BBox -> Max);
	    IP_SET_BBOX_OBJ(PObjGeometry);

	    IRIT_PT_COPY(PObj -> BBox[0], BBox -> Min);
	    IRIT_PT_COPY(PObj -> BBox[1], BBox -> Max);
	    IP_SET_BBOX_OBJ(PObj);

	    IP_SET_OBJ_NAME2(PObjGeometry, IP_GET_OBJ_NAME(PObj));

	    AttrSetObjectObjAttrib(PObj, "_geometry", PObjGeometry, FALSE);
	}
    }

    if (PObjGeometry != NULL) {
        IGDrawPolyFuncPtr(PObjGeometry);

	if (!IGGlblCacheGeom)
	    IPFreeObject(PObjGeometry);
    }
}
