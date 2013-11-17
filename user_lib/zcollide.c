/******************************************************************************
* ZCollide.c - handle the detection of maximal Z motion with no collision.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jul. 2000.					      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "user_loc.h"
#include "geom_lib.h"
#include "bool_lib.h"
#include "cagd_lib.h"

static int UserObjBboxIntersect(IPObjectStruct *PObj1,
				IPObjectStruct *PPolyObj2,
				IrtRType FineNess);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the maximal Z motion to move PObj2 down (-Z direction) so that  M
* it does not intersect PObj1.  Second object is converted to its Bbox.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:     First static object to place PObj2 on.                        M
*   PObj2:     Second dynamic object that is to be moved down (-Z direction) M
*	       until it is tangent to PObj1.				     M
*   FineNess:  Of polygonal approximation of PObj1.                          M
*   NumIters:  In the bisectioning of collision's test. 10 is a good start.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Maximal Z motion possible, or IRIT_INFNTY if no collision   M
*		 or error.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserTwoObjMaxZRelMotion                                                  M
*****************************************************************************/
IrtRType UserTwoObjMaxZRelMotion(IPObjectStruct *PObj1,
				 IPObjectStruct *PObj2,
				 IrtRType FineNess,
				 int NumIters)
{
    int i,
	OldInterCurve = BoolSetOutputInterCurve(TRUE),
      	OldCirc = IPSetPolyListCirc(TRUE);
    IrtRType MinZStep, MaxZStep,
	NewZ = 0.0;
    GMBBBboxStruct
	BBox1 = *GMBBComputeBboxObject(PObj1),
	BBox2 = *GMBBComputeBboxObject(PObj2);

    /* No geometry in either/both objects or no intersection in xy. */
    if (BBox1.Max[0] <= BBox2.Min[0] ||
	BBox2.Max[0] <= BBox1.Min[0] ||
	BBox1.Max[1] <= BBox2.Min[1] ||
	BBox2.Max[1] <= BBox1.Min[1])
	return IRIT_INFNTY;

    MinZStep = BBox1.Max[2] - BBox2.Min[2];
    MaxZStep = BBox1.Min[2] - BBox2.Max[2];

    /* Check collisions against the (moved) BBox2 of PObj2. */
    for (i = 0; i < NumIters; i++) {
	IPObjectStruct *PBBox;

	BBox2.Min[2] += (NewZ = (MinZStep + MaxZStep) * 0.5);
	PBBox = PrimGenBOXObject(BBox2.Min,
				 BBox2.Max[0] - BBox2.Min[0],
				 BBox2.Max[1] - BBox2.Min[1],
				 BBox2.Max[2] - BBox2.Min[2]);

	if (UserObjBboxIntersect(PObj1, PBBox, FineNess))
	    MaxZStep = NewZ;
	else
	    MinZStep = NewZ;
	IPFreeObject(PBBox);

	BBox2.Min[2] -= NewZ;
    }

    BoolSetOutputInterCurve(OldInterCurve);
    IPSetPolyListCirc(OldCirc);

    return NewZ;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Intersects an object that can be a whole hierarchy (PObj1) against a     *
* single bbox (PPolyObj2) object.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1:     First object to check for intersection.  Can be a whole       *
*	       hierarchy of objects.					     *
*   PPolyObj2: Second, single polygonal object to check PObj against for     *
*	       intersections.                                                *
*   Fineness:  Of polygonal approximation of PObj1.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if intersect, FALSE otherwise.                            *
*****************************************************************************/
static int UserObjBboxIntersect(IPObjectStruct *PObj1,
				IPObjectStruct *PPolyObj2,
				IrtRType FineNess)
{
    if (IP_IS_OLST_OBJ(PObj1)) {
	int i;
	IPObjectStruct *PObj1Tmp;

	for (i = 0; (PObj1Tmp = IPListObjectGet(PObj1, i)) != NULL; i++)
	    if (UserObjBboxIntersect(PObj1Tmp, PPolyObj2, FineNess))
	        return TRUE;

	return FALSE;
    }
    else {
	IPObjectStruct *InterObj, *PPolyObj1;
	IPPolygonStruct *Polys;

	switch (PObj1 -> ObjType) {
	    case IP_OBJ_POLY:
		PPolyObj1 = PObj1;
		break;
	    case IP_OBJ_SURFACE:
		if ((PPolyObj1 = AttrGetObjectObjAttrib(PObj1, "_ZCollidePls"))
								    == NULL) {
		    Polys = IPSurface2Polygons(PObj1 -> U.Srfs, FALSE,
					       FineNess, FALSE, FALSE, FALSE);
		    PPolyObj1 = IPGenPOLYObject(Polys);
		    AttrSetObjectObjAttrib(PObj1, "_ZCollidePls",
					   PPolyObj1, FALSE);
		}
		break;
	    case IP_OBJ_TRIMSRF:
		if ((PPolyObj1 = AttrGetObjectObjAttrib(PObj1, "_ZCollidePls"))
								    == NULL) {
		    Polys = IPTrimSrf2Polygons(PObj1 -> U.TrimSrfs, FALSE,
					       FineNess, FALSE, FALSE, FALSE);
		    PPolyObj1 = IPGenPOLYObject(Polys);
		    AttrSetObjectObjAttrib(PObj1, "_ZCollidePls",
					   PPolyObj1, FALSE);
		}
		break;
	    default:
		return FALSE;
	}

	if ((InterObj = BooleanAND(PPolyObj1, PPolyObj2)) != NULL) {
	    int RetVal = InterObj -> U.Pl != NULL;

	    IPFreeObject(InterObj);
	    return RetVal;
	}
	else
	    return FALSE;
    }
}

