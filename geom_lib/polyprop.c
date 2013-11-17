/*****************************************************************************
* PolyProp.c -  computes contant values of features over polygonal meshes.   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Modified by: Gershon Elber				Ver 1.1, Mar 20023   *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "irit_sm.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "grap_lib.h"
#include "geom_loc.h"

#define GM_POLY_PROP_EPS	1e-4

IRIT_STATIC_DATA const char
    *GlblPropertyAttr = NULL;
IRIT_STATIC_DATA int
    GlblCurvatureProperty = 0;
IRIT_STATIC_DATA IrtRType
    GlblIsophotesInclinationCosAngle = 0.0;
IRIT_STATIC_DATA IrtVecType
    GlblIsophotesLightDir = { 1.0, 0.0, 0.0 };

static IrtRType VertexPropertyAttribute(IPVertexStruct *V,
					IPPolygonStruct *Pl);
static IrtRType VertexPropertyIsophotes(IPVertexStruct *V,
					IPPolygonStruct *Pl);
static IrtRType VertexPropertyCurvature(IPVertexStruct *V,
					IPPolygonStruct *Pl);
static IrtRType *GMFetchBlendedMidPoint(IrtRType *Coord1,
					IrtRType *Coord2,
					IrtRType Val1,
					IrtRType Val2,
					IrtRType ConstVal);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function to fetch an attribute value at the given vertex V of  *
* polygon Pl.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:    Current vertex to process.                                         *
*   Pl:   Current polygon to process, where V is.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:                                                                *
*****************************************************************************/
static IrtRType VertexPropertyAttribute(IPVertexStruct *V,
					IPPolygonStruct *Pl)
{
    IrtRType
	R = AttrGetRealAttrib(V -> Attr, GlblPropertyAttr);

    if (IP_ATTR_IS_BAD_REAL(R))
	return 0.0;
    else
        return R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to derive curves over a polygonal model, Pls, based on given  M
* prescribed attribute PropAttr. 				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:          The polygons to process the constant value property        M
*		  for.  Assumed to hold triangles only.		             M
*   PropAttr:     Name of attribute to extract.				     M
*   Value:        Value of property to seek.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  List of polylines on the polygons along the	     M
*			requested property value.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyPropFetchIsophotes, GMPolyPropFetchCurvature                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyPropFetchAttribute                                                 M
*****************************************************************************/
IPPolygonStruct *GMPolyPropFetchAttribute(IPPolygonStruct *Pls,
					  const char *PropAttr,
					  IrtRType Value)
{
    GlblPropertyAttr = PropAttr;

    return GMPolyPropFetch(Pls, VertexPropertyAttribute, Value);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function to derive the isophote value at the given vertex V of *
* polygon Pl.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:    Current vertex to process.                                         *
*   Pl:   Currebt polygonm to process, where V is.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:                                                                *
*****************************************************************************/
static IrtRType VertexPropertyIsophotes(IPVertexStruct *V,
					IPPolygonStruct *Pl)
{
    if (IP_HAS_NORMAL_VRTX(V)) {
        return IRIT_DOT_PROD(V -> Normal, GlblIsophotesLightDir)
		                           - GlblIsophotesInclinationCosAngle;
    }
    else {
        return 0.0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to derive isophotes for a polygonal model, Pls.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:               The polygons to process the constant value property   M
*		       for. Assumed to hold triangles only.		     M
*   ViewDir:           Direction of view.				     M
*   InclinationAngle:  In degrees to consider the isophotes for.  90 degrees M
*		       would yield regular silhouettes viewed from LightDir. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  List of polylines on the polygons along the	     M
*			requested isophotes.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyPropFetchCurvature, GMPolyPropFetchAttribute                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyPropFetchIsophotes                                                 M
*****************************************************************************/
IPPolygonStruct *GMPolyPropFetchIsophotes(IPPolygonStruct *Pls,
					  const IrtVecType ViewDir,
					  IrtRType InclinationAngle)
{
    GlblIsophotesInclinationCosAngle = cos(IRIT_DEG2RAD(InclinationAngle));
    IRIT_VEC_COPY(GlblIsophotesLightDir, ViewDir);

    return GMPolyPropFetch(Pls, VertexPropertyIsophotes, 0.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function to derive the curvature value at the given vertex V   *
* of polygon Pl.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:    Current vertex to process.                                         *
*   Pl:   Currebt polygonm to process, where V is.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:                                                                *
*****************************************************************************/
static IrtRType VertexPropertyCurvature(IPVertexStruct *V,
					IPPolygonStruct *Pl)
{
    IrtRType R;

    switch (GlblCurvatureProperty) {
	case 0:
	default:
	    R = AttrGetRealAttrib(V -> Attr, "KCurv");
	    break;
	case 1:
	    R = AttrGetRealAttrib(V -> Attr, "HCurv");
	    break;
    }

    if (IP_ATTR_IS_BAD_REAL(R))
	return 0.0;
    else
        return R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to derive isophotes for a polygonal model, Pls.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:               The polygons to process the constant value property   M
*		       for. Assumed to hold triangles only.		     M
*   CurvatureProperty: 0 for Gaussian curvature, 1 for Mean curvature.       M
*   NumOfRings:        In the paraboloid fit estimation, usually 1-2.        M
*   CrvtrVal:          Value of curvature property to seek.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  List of polylines on the polygons along the	     M
*			requested curvature lines.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyPropFetchIsophotes, GMPolyPropFetchAttribute                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyPropFetchCurvature                                                 M
*****************************************************************************/
IPPolygonStruct *GMPolyPropFetchCurvature(IPPolygonStruct *Pls,
					  int CurvatureProperty,
					  int NumOfRings,
					  IrtRType CrvtrVal)
{
    GlblCurvatureProperty = CurvatureProperty;

    GMPlCrvtrSetCurvatureAttr(Pls, NumOfRings, TRUE);  /* Set K and H props. */

    return GMPolyPropFetch(Pls, VertexPropertyCurvature, CrvtrVal);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Derive the exact middle point along the edges between two vertices based *
* on the property values at the end and ConstVal.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Coord1, Coord2: The two coordinates of the two adjacent vertices.        *
*   Val1, Val2:     The values of the property at the two end points.        *
*   ConstVal:       Constant value of property sought.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:  A pointer to static location holding newly derived point,   *
*		 or NULL if no intersection of porperty value with this      *
*		 edge.							     *
*****************************************************************************/
static IrtRType *GMFetchBlendedMidPoint(IrtRType *Coord1,
					IrtRType *Coord2,
					IrtRType Val1,
					IrtRType Val2,
					IrtRType ConstVal)
{
    static IrtPtType MidPt;
    IrtRType t;

    if ((Val1 <= ConstVal && Val2 >= ConstVal) ||
	(Val2 <= ConstVal && Val1 >= ConstVal)) {
        /* Edge has an intersection point. */
        if (Val1 == Val2)
	    t = 0.5;
	else
	    t = (Val1 - ConstVal) / (Val1 - Val2);
	IRIT_PT_BLEND(MidPt, Coord2, Coord1, t);
	return MidPt;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the constant value over a polygonal model by processing each    M
* polygons for edges that the values of the property are begin crossed.      M
*   The input polygonal model is assumed to hold triangles only.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:            The polygons to process the constant value property for. M
*		    Assumed to hold triangles only.			     M
*   VertexProperty: A call back function to return the desired property for  M
*		    the invoked vertex.					     M
*   ConstVal:       Constant value of property sought.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  List of polylines on the polygons for which the      M
*			property assumes the value ConstVal.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyPropFetchIsophotes, GMPolyPropFetchCurvature,			     M
*   GMFetchVertexPropertyFuncType                      	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyPropFetch                                                          M
*****************************************************************************/
IPPolygonStruct *GMPolyPropFetch(IPPolygonStruct *Pls,
				 GMFetchVertexPropertyFuncType VertexProperty,
				 IrtRType ConstVal)
{
    int InformNonTriangle = FALSE;
    IPPolygonStruct *Pl;
    IPPolygonStruct
        *PropPl = NULL;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
        int i,
	    NumInters = 0;
	IrtPtType Inters[3];
        IPVertexStruct
	    *V1 = Pl -> PVertex,
	    *V2 = V1 -> Pnext,
	    *V3 = V2 -> Pnext;
	IrtRType *MidPt,
	    Val1 = VertexProperty(V1, Pl),
	    Val2 = VertexProperty(V2, Pl),
	    Val3 = VertexProperty(V3, Pl);

	if (V3 -> Pnext != NULL && V3 -> Pnext != V1 && !InformNonTriangle) {
	    IRIT_WARNING_MSG("Extracting property from non triangles.");
	    InformNonTriangle = TRUE;
	}

	if ((MidPt = GMFetchBlendedMidPoint(V1 -> Coord, V2 -> Coord,
					    Val1, Val2, ConstVal)) != NULL) {
	    /* We have a new intersection along edge V1V2. */
	    IRIT_PT_COPY(Inters[NumInters], MidPt);
	    NumInters++;
	}

	if ((MidPt = GMFetchBlendedMidPoint(V1 -> Coord, V3 -> Coord,
					    Val1, Val3, ConstVal)) != NULL) {
	    for (i = 0; i < NumInters; i++) {
		if (IRIT_PT_APX_EQ(Inters[i], MidPt))
		    break;
	    }

	    if (i >= NumInters) {
	        /* We have a new intersection along edge V1V3. */
	        IRIT_PT_COPY(Inters[NumInters], MidPt);
		NumInters++;
	    }
	}

	if ((MidPt = GMFetchBlendedMidPoint(V2 -> Coord, V3 -> Coord,
					    Val2, Val3, ConstVal)) != NULL) {
	    for (i = 0; i < NumInters; i++) {
		if (IRIT_PT_APX_EQ(Inters[i], MidPt))
		    break;
	    }

	    if (i >= NumInters) {
	        /* We have a new intersection along edge V1V3. */
	        IRIT_PT_COPY(Inters[NumInters], MidPt);
		NumInters++;
	    }
	}

	if (NumInters >= 2) {
	    /* Construct an edge and push into PropPl list. */
	    PropPl = GMGenPolyline2Vrtx(Inters[0], Inters[1], PropPl);
	}
    }

    /* Merge all these little edges into long polygons. */
    return GMMergePolylines(PropPl, GM_POLY_PROP_EPS);
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a polyline out of a list of 2 vertices V1/2.	     M
*   No test is made to make sure the 2 points are not the same...	     M
*   The points are placed in order.                      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2:        Two vertices of the constructed polyline.                 M
*   Pnext:         Next is chain of polylines, in linked list.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The constructed polyline.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenPolygon3Vrtx, PrimGenPolygon4Vrtx                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenPolyline2Vrtx                                                       M
*****************************************************************************/
IPPolygonStruct *GMGenPolyline2Vrtx(IrtVecType V1,
				    IrtVecType V2,
				    IPPolygonStruct *Pnext)
{
    IPPolygonStruct *PPoly;
    IPVertexStruct *V;

    PPoly = IPAllocPolygon(0, V = IPAllocVertex2(NULL), Pnext);
    IRIT_PT_COPY(V -> Coord, V1);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V2);

    return PPoly;
}
