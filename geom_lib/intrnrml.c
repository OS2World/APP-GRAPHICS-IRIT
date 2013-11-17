/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Aug. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to interplate internal vertices using vertices on the given       *
* convex boundary OriginalVList. All internal vertices are assumed to be in  *
* the interior of the convex region defined by OriginalVList or on its       *
* boundary.								     *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "geom_loc.h"

typedef struct GMVertexInfoStruct {
    IrtVecType Normal;
    IrtRType CosMaxDeviate;
    int Count;
} GMVertexInfoStruct;

IRIT_STATIC_DATA GMVertexInfoStruct
    *GlblVertices = NULL;
IRIT_STATIC_DATA IPPolygonStruct
    *GlblCrntPlBlendNormal = NULL;

#define NRML_SAME_VRTX_EPS	1e-5   /* Eps to consider two vertices same. */
#define IHT_VERTEX_KEY(Pt)	(Pt[0] * 0.301060 + \
				 Pt[1] * 0.280791 + \
				 Pt[2] * 0.190886)

#define GM_REORIENT_STACK_SIZE	131071       /* Reorien polygons stack size. */
#define GM_POLY_REORIENTED_TAG	0x40   /* A tag for processed Reoriented pl. */

#define	GM_IS_REORIENTED_POLY(Poly)  ((Poly) -> Tags & GM_POLY_REORIENTED_TAG)
#define	GM_SET_REORIENTED_POLY(Poly) ((Poly) -> Tags |= GM_POLY_REORIENTED_TAG)
#define	GM_RST_REORIENTED_POLY(Poly) ((Poly) -> Tags &= ~GM_POLY_REORIENTED_TAG)
static void UpdateOneVertexByInterp(IPVertexStruct *VUpdate,
				    const IPPolygonStruct *OriginalPl,
				    int DoRgb,
				    int DoUV,
				    int DoNrml);
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2);

#ifdef DEBUG 
static void GMPrintPolygon(IPPolygonStruct *Pl);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
* For each polygon in PlList update any vertex with a proper normal, uv      M
* uv coord, rgb color, etc. if available in the Original polygon vertex list M
* OriginalPl.								     M
*   All the new vertices are enclosed within the original polygon which      M
* must be convex as well.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlList:       List of polygons to update normal for.                     M
*   OriginalPl:   Original polygons PlList was derived from, probably using  M
*                 Boolean operations.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBlendNormalsToVertices, GMInterpVrtxNrmlBetweenTwo,                    M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl,                     M
*   GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,                        M
*   GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMUpdateVerticesByInterp, normals, uv coords, rgb color                  M
*****************************************************************************/
void GMUpdateVerticesByInterp(IPPolygonStruct *PlList,
			      const IPPolygonStruct *OriginalPl)
{
    int R, G, B,
        ID = AttrGetIntAttrib(OriginalPl -> Attr, "ID"),
	DoRgb = AttrGetRGBColor(OriginalPl -> PVertex -> Attr, &R, &G, &B),
	DoUV = AttrGetUVAttrib(OriginalPl -> PVertex -> Attr, "uvvals") != NULL;
    IPVertexStruct *V, *VHead;

    while (PlList) {
	V = VHead = PlList -> PVertex;
	do {
	    UpdateOneVertexByInterp(V, OriginalPl, DoRgb, DoUV,
				    IRIT_PT_APX_EQ_ZERO_EPS(V -> Normal,
						       IRIT_EPS));

	    V = V -> Pnext;
	}
	while (V != NULL && V != VHead);

	if (!IP_ATTR_IS_BAD_INT(ID))
	    AttrSetIntAttrib(&PlList -> Attr, "ID", ID);

	PlList = PlList -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update attributes of one vertex by a convex blend of the attributes of   *
* (V, VNext). VUpdate is assumed to be on the edge (V, VNext).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   VUpdate:      Vertex to update attributes for.                           *
*   V, VNext:	  Edge (V, VNext) that is assumed to contain VUpdate.	     *
*   DoRgb:        Handle "rgb" attribute interpolation on vertices.	     *
*   DoUV:         Handle "uv" attribute interpolation on vertices.	     *
*   DoNrml:       Handle Normals interpolation on vertices.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void GMUpdateVertexByInterp(IPVertexStruct *VUpdate,
			    const IPVertexStruct *V,
			    const IPVertexStruct *VNext,
			    int DoRgb,
			    int DoUV,
			    int DoNrml)
{
    if (GMCollinear3Vertices(V, VUpdate, VNext)) {
	if (DoNrml)
	    GMInterpVrtxNrmlBetweenTwo(VUpdate, V, VNext);
	if (DoRgb)
	    GMInterpVrtxRGBBetweenTwo(VUpdate, V, VNext);
	if (DoUV)
	    GMInterpVrtxUVBetweenTwo(VUpdate, V, VNext);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update attributes of one vertex by a convex blend of the attributes of   *
* boundary vertices.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   VUpdate:      Vertex to update attributes for.                           *
*   OriginalPl:   Using a convex blend of attrs from the original polygon.   *
*   DoRgb:        Handle "rgb" attribute interpolation on vertices.	     *
*   DoUV:         Handle "uv" attribute interpolation on vertices.	     *
*   DoNrml:       Handle Normals interpolation on vertices.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateOneVertexByInterp(IPVertexStruct *VUpdate,
				    const IPPolygonStruct *OriginalPl,
				    int DoRgb,
				    int DoUV,
				    int DoNrml)
{
    IPVertexStruct *V,
	*OriginalVList = OriginalPl -> PVertex;

    V = OriginalVList;
    do {
	if (GMCollinear3Vertices(V, VUpdate, V -> Pnext)) {
	    if (DoNrml)
	        GMInterpVrtxNrmlBetweenTwo(VUpdate, V, V -> Pnext);
	    if (DoRgb)
	        GMInterpVrtxRGBBetweenTwo(VUpdate, V, V -> Pnext);
	    if (DoUV)
	        GMInterpVrtxUVBetweenTwo(VUpdate, V, V -> Pnext);
	    return;
	}
	V = V -> Pnext;
    }
    while (V != NULL && V != OriginalVList);

    /* If we are here then the point is not on the polygon boundary and in   */
    /* that case blend all vertices based on the distance-weight.            */
    GMInterpVrtxNrmlFromPl(VUpdate, OriginalPl);
    GMInterpVrtxRGBFromPl(VUpdate, OriginalPl);
    GMInterpVrtxUVFromPl(VUpdate, OriginalPl);

    IP_SET_NORMAL_VRTX(VUpdate);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Verify the collinearity of the given three vertices.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2, V3: Vertices to test for collinearity.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if collinear, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMCollinear3Vertices, collinearity                                       M
*****************************************************************************/
int GMCollinear3Vertices(const IPVertexStruct *V1,
			 const IPVertexStruct *V2,
			 const IPVertexStruct *V3)
{
    IrtRType l;
    IrtVecType V12, V23, V;

    if (IRIT_PT_APX_EQ(V1 -> Coord, V2 -> Coord) ||
	IRIT_PT_APX_EQ(V2 -> Coord, V3 -> Coord))
	return TRUE;

    IRIT_PT_SUB(V12, V1 -> Coord, V2 -> Coord);
    IRIT_PT_SUB(V23, V2 -> Coord, V3 -> Coord);

    /* Make sure the middle point is in fact in the middle. */
    if (V12[0] * V23[0] < -IRIT_UEPS ||
        V12[1] * V23[1] < -IRIT_UEPS ||
	V12[2] * V23[2] < -IRIT_UEPS)
	return FALSE;

    GMVecCrossProd(V, V12, V23);

    l = IRIT_PT_LENGTH(V);

    return IRIT_APX_EQ(l, 0.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compues blending weights for a vertex inside a polygon.  Computes the    M
* barycentric coordinates of the triangle in Pl, V is in.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Coord:  Of vertex that its weights we seek.		                     M
*   Pl:     Polygon including V.				             M
*   Wgt:    Vector to update with two weights.  Must be of length larger or  M
*           equal to the number of vertices in Pl.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE if failed (V outside Pl).               M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl,		     M
*   GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,			     M
*   GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMEvalWeightsVFromPl, barycentric coordinates                            M
*****************************************************************************/
int GMEvalWeightsVFromPl(const IrtRType *Coord,
			 const IPPolygonStruct *Pl,
			 IrtRType *Wgt)
{
    int i;
    IPVertexStruct *V,
	*VFirst = Pl -> PVertex;

    /* Find out the triangle inside Pl that holds V. */
    assert(VFirst != NULL);
    for (i = 1, V = VFirst -> Pnext;
	 V -> Pnext != NULL && V -> Pnext != VFirst;
	 i++, V = V -> Pnext) {
	IrtRType
	    *W3 = GMBaryCentric3Pts(VFirst -> Coord, V -> Coord,
				    V -> Pnext -> Coord, Coord);

        if (W3 != NULL) {			    /* Found our triangle. */
	    int n = IPVrtxListLen(VFirst);

	    IRIT_ZAP_MEM(Wgt, sizeof(IrtRType) * n);
	    Wgt[0] = W3[0];
	    Wgt[i] = W3[1];
	    Wgt[i + 1] = W3[2];

	    return TRUE;
	}
    }

    return FALSE; /* Point is outsize Pl. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update Normal of the middle vertex V, assumed to be between V1 and V2.   M
*                                                                            *
* PARAMETERS:                                                                M
*   V:          Vertex that its normal is to be updated.                     M
*   V1, V2:     Edge V is assumed to be on so that the two normals of V1     M
*               and V2 can be blended to form the normal of V.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo2,                   M
*   GMInterpVrtxNrmlFromPl, GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,M
*   GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl 			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxNrmlBetweenTwo, normals                                      M
*****************************************************************************/
void GMInterpVrtxNrmlBetweenTwo(IPVertexStruct *V,
				const IPVertexStruct *V1,
				const IPVertexStruct *V2)
{
    IrtRType t1, t2;
    IrtVecType Vec1, Vec2;

    IRIT_PT_SUB(Vec1, V -> Coord, V1 -> Coord);
    IRIT_PT_SUB(Vec2, V -> Coord, V2 -> Coord);
    t1 = IRIT_PT_LENGTH(Vec1);
    t2 = IRIT_PT_LENGTH(Vec2);

    IRIT_PT_COPY(Vec1, V1 -> Normal);
    IRIT_PT_COPY(Vec2, V2 -> Normal);
    IRIT_PT_SCALE(Vec1, t2);
    IRIT_PT_SCALE(Vec2, t1);
    IRIT_PT_ADD(V -> Normal, Vec1, Vec2);

    if (!IRIT_PT_APX_EQ_ZERO_EPS(V -> Normal, IRIT_UEPS)) {
        IRIT_PT_NORMALIZE(V -> Normal);
	IP_SET_NORMAL_VRTX(V);
    }
    else {
        IP_RST_NORMAL_VRTX(V);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update normal of middle position Pt, assumed to be between V1 and V2.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Middle position at which a normal is to be computed.         M
*   Normal:     Where resulting vector is to be placed.                      M
*   V1, V2:     Edge V is assumed to be on so that the two normals of V1     M
*               and V2 can be blended to form the normal of V.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,                    M
*   GMInterpVrtxNrmlFromPl, GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,M
*   GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl 			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxNrmlBetweenTwo2, normals                                     M
*****************************************************************************/
void GMInterpVrtxNrmlBetweenTwo2(IrtPtType Pt,
				 IrtVecType Normal,
				 const IPVertexStruct *V1,
				 const IPVertexStruct *V2)
{
    IrtRType t1, t2;
    IrtVecType Vec1, Vec2;

    IRIT_PT_SUB(Vec1, Pt, V1 -> Coord);
    IRIT_PT_SUB(Vec2, Pt, V2 -> Coord);
    t1 = IRIT_PT_LENGTH(Vec1);
    t2 = IRIT_PT_LENGTH(Vec2);

    if (IRIT_APX_EQ(t1, 0.0) && IRIT_APX_EQ(t2, 0.0)) {
	IRIT_PT_COPY(Normal, V1 -> Normal);
    }
    else {
	IRIT_PT_COPY(Vec1, V1 -> Normal);
	IRIT_PT_COPY(Vec2, V2 -> Normal);
	IRIT_PT_SCALE(Vec1, t2);
	IRIT_PT_SCALE(Vec2, t1);
	IRIT_PT_ADD(Normal, Vec1, Vec2);

	IRIT_PT_NORMALIZE(Normal);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update Normal of vertex V, based on surrounding polygon Pl.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vrtx:   Vertex that its normal is to be updated.                         M
*   Pl:     Polygon surrounding V to interpolate normal from.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if point is inside polygon, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxRGBBetweenTwo,                  M
*   GMInterpVrtxRGBFromPl, GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl    M 
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxNrmlFromPl, normals                                          M
*****************************************************************************/
int GMInterpVrtxNrmlFromPl(IPVertexStruct *Vrtx, const IPPolygonStruct *Pl)
{
    int i, j,
        n = IPVrtxListLen(Pl -> PVertex);
    IrtRType
        *Wgt = (IrtRType *) IritMalloc(sizeof(IrtRType) * n);
    IPVertexStruct *V;

    if (GMEvalWeightsVFromPl(Vrtx -> Coord, Pl, Wgt)) {
        IRIT_VEC_RESET(Vrtx -> Normal);
        for (i = 0, V = Pl -> PVertex; i < n; i++, V = V -> Pnext) {
	    for (j = 0; j < 3; j++)
	        Vrtx -> Normal[j] += V -> Normal[j] * Wgt[i];
	}
	IritFree(Wgt);
    }
    else {
        IritFree(Wgt);
        return FALSE;
    }

    if (!IRIT_PT_APX_EQ_ZERO_EPS(V -> Normal, IRIT_UEPS)) {
        IRIT_PT_NORMALIZE(Vrtx -> Normal);
	IP_SET_NORMAL_VRTX(Vrtx);
    }
    else {
        IP_RST_NORMAL_VRTX(Vrtx);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update RGB of the middle vertex V, assumed to be between V1 and V2.      M
*                                                                            *
* PARAMETERS:                                                                M
*   V:          Vertex that its rgb color is to be updated.                  M
*   V1, V2:     Edge V is assumed to be on so that the two rgb colors of V1  M
*               and V2 can be blended to form the rgb color of V.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE if failed.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl,		     M
*   GMInterpVrtxRGBFromPl, GMInterpVrtxUVBetweenTwo, GMInterpVrtxUVFromPl    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxRGBBetweenTwo, rgb color                                     M
*****************************************************************************/
int GMInterpVrtxRGBBetweenTwo(IPVertexStruct *V,
			      const IPVertexStruct *V1,
			      const IPVertexStruct *V2)
{
    int R1, G1, B1, R2, G2, B2, R, G, B;

    if (AttrGetRGBColor(V1 -> Attr, &R1, &G1, &B1) &&
	AttrGetRGBColor(V2 -> Attr, &R2, &G2, &B2)) {
        IrtRType t1, t2;
	IrtVecType Vec1, Vec2;

	IRIT_PT_SUB(Vec1, V -> Coord, V1 -> Coord);
	IRIT_PT_SUB(Vec2, V -> Coord, V2 -> Coord);
	t1 = IRIT_PT_LENGTH(Vec1);
	t2 = IRIT_PT_LENGTH(Vec2);
	t2 /= (t1 + t2 + IRIT_UEPS);
	t1 = 1.0 - t2;

	R = (int) (t1 * R2 + t2 * R1);
	G = (int) (t1 * G2 + t2 * G1);
	B = (int) (t1 * B2 + t2 * B1);
	AttrSetRGBColor(&V -> Attr, R, G, B);
	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update rgb color of vertex V, based on surrounding polygon Pl.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vrtx:   Vertex that its rgb color is to be updated.                      M
*   Pl:     Polygon surrounding V to interpolate rgb color from.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if point is inside polygon, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl, 		     M
*   GMInterpVrtxRGBBetweenTwo, GMInterpVrtxUVBetweenTwo, 		     M
*   GMInterpVrtxUVFromPl						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxRGBFromPl, rgb color                                         M
*****************************************************************************/
int GMInterpVrtxRGBFromPl(IPVertexStruct *Vrtx, const IPPolygonStruct *Pl)
{
    int i, IR, IG, IB,
        n = IPVrtxListLen(Pl -> PVertex);
    IrtRType R, G, B,
        *Wgt = (IrtRType *) IritMalloc(sizeof(IrtRType) * n);
    IPVertexStruct *V;

    if (GMEvalWeightsVFromPl(Vrtx -> Coord, Pl, Wgt)) {
        R = G = B = 0.0;
        for (i = 0, V = Pl -> PVertex; i < n; i++, V = V -> Pnext) {
	    int RTmp, GTmp, BTmp;

	    if (!AttrGetRGBColor(V -> Attr, &RTmp, &GTmp, &BTmp)) {
	        IritFree(Wgt);
	        return FALSE;
	    }

	    R += RTmp * Wgt[i];
	    G += GTmp * Wgt[i];
	    B += BTmp * Wgt[i];
	}
	IritFree(Wgt);
    }
    else {
        IritFree(Wgt);
        return FALSE;
    }

    IR = (int) (0.5 + R);
    IR = IRIT_BOUND(IR, 0, 255);

    IG = (int) (0.5 + G);
    IG = IRIT_BOUND(IG, 0, 255);

    IB = (int) (0.5 + B);
    IB = IRIT_BOUND(IB, 0, 255);

    AttrSetRGBColor(&Vrtx -> Attr, IR, IG, IB);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update UV of the middle vertex V, assumed to be between V1 and V2.       M
*                                                                            *
* PARAMETERS:                                                                M
*   V:          Vertex that its UV coordinates are to be updated.            M
*   V1, V2:     Edge V is assumed to be on so that the two UV coords of V1   M
*               and V2 can be blended to form the UV coords of V.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE if failed.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl,		     M
*   GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,			     M
*   GMInterpVrtxUVFromPl						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxUVBetweenTwo, uv coordinates                                 M
*****************************************************************************/
int GMInterpVrtxUVBetweenTwo(IPVertexStruct *V,
			     const IPVertexStruct *V1,
			     const IPVertexStruct *V2)
{
    float *UV1, *UV2, UV[2];

    if ((UV1 = AttrGetUVAttrib(V1 -> Attr, "uvvals")) != NULL &&
	(UV2 = AttrGetUVAttrib(V2 -> Attr, "uvvals")) != NULL) {
        IrtRType t1, t2;
	IrtVecType Vec1, Vec2;

	IRIT_PT_SUB(Vec1, V -> Coord, V1 -> Coord);
	IRIT_PT_SUB(Vec2, V -> Coord, V2 -> Coord);
	t1 = IRIT_PT_LENGTH(Vec1);
	t2 = IRIT_PT_LENGTH(Vec2);
	t2 /= (t1 + t2 + IRIT_UEPS);

	IRIT_PT2D_BLEND(UV, UV1, UV2, (float) t2);

	AttrSetUVAttrib(&V -> Attr, "uvvals", UV[0], UV[1]);
	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update UV coordinate of vertex V, based on surrounding polygon Pl.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vrtx:   Vertex that its UV coordinate is to be updated.                  M
*   Pl:     Polygon surrounding V to interpolate UV coordinate from.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE if failed.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMInterpVrtxNrmlBetweenTwo,		     M
*   GMInterpVrtxNrmlBetweenTwo2, GMInterpVrtxNrmlFromPl,		     M
*   GMInterpVrtxRGBBetweenTwo, GMInterpVrtxRGBFromPl,			     M
*   GMInterpVrtxUVBetweenTwo						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMInterpVrtxUVFromPl, uv coordinates                                     M
*****************************************************************************/
int GMInterpVrtxUVFromPl(IPVertexStruct *Vrtx, const IPPolygonStruct *Pl)
{
    int i,
        n = IPVrtxListLen(Pl -> PVertex);
    IrtRType UV[2],
        *Wgt = (IrtRType *) IritMalloc(sizeof(IrtRType) * n);
    IPVertexStruct *V;

    if (GMEvalWeightsVFromPl(Vrtx -> Coord, Pl, Wgt)) {
        UV[0] = UV[1] = 0.0;
        for (i = 0, V = Pl -> PVertex; i < n; i++, V = V -> Pnext) {
	    float *UVTmp;

	    if ((UVTmp = AttrGetUVAttrib(V -> Attr, "uvvals")) == NULL) {
	        IritFree(Wgt);
	        return FALSE;
	    }

	    UV[0] += UVTmp[0] * Wgt[i];
	    UV[1] += UVTmp[1] * Wgt[i];
	}
	IritFree(Wgt);
    }
    else {
        IritFree(Wgt);
        return FALSE;
    }

    AttrSetUVAttrib(&Vrtx -> Attr, "uvvals", UV[0], UV[1]);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare two vertices if same or not.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   VP1, VP2:   Two vertices to compare.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0, +1 if V1 less, equal, greater than V2.                  *
*****************************************************************************/
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2)
{
    IPVertexStruct
	*V1 = (IPVertexStruct *) VP1,
	*V2 = (IPVertexStruct *) VP2;
    IrtRType
	*Coord1 = V1 -> Coord,
	*Coord2 = V2 -> Coord;

    if (IRIT_PT_APX_EQ_EPS(Coord1, Coord2, NRML_SAME_VRTX_EPS)) {
	/* Update cross references so we could recover the averaged normal. */
        int Idx,
	    Idx1 = AttrGetIntAttrib(V1 -> Attr, "_vrtxIdx"),
	    Idx2 = AttrGetIntAttrib(V2 -> Attr, "_vrtxIdx");

	if (!IP_ATTR_IS_BAD_INT(Idx1)) {
	    AttrSetIntAttrib(&V2 -> Attr, "_vrtxIdx", Idx1);
	    Idx = Idx1;
	}
	else if (!IP_ATTR_IS_BAD_INT(Idx2)) {
	    AttrSetIntAttrib(&V1 -> Attr, "_vrtxIdx", Idx2);
	    Idx = Idx2;
	}
	else {
	    GEOM_FATAL_ERROR(GEOM_ERR_VRTX_MTCH_FAILED);
	    Idx = 0;
	}

	IRIT_VEC_ADD(GlblVertices[Idx].Normal, GlblVertices[Idx].Normal,
		GlblCrntPlBlendNormal -> Plane);
	GlblVertices[Idx].Count++;

	return 0;
    }
    else {
        int i;

        for (i = 0; i < 3; i++) {
	    if (Coord1[i] < Coord2[i])
	        return -1;
	    else if (Coord1[i] > Coord2[i])
	        return 1;
	}

	return 0; /* Should never get here. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Approximate normals to all vertices of the given geometry by blending    M
* the normals of the faces that share the vertex.  Assumes polygons are      M
* properly oriented.  Places on each vertex an "_CosNrmlMaxDeviation"        M
* attribute with the maximal deviation of this normal from an adjacent       M
* polygon plane (cosine of the maximal angle).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlList:   List of polygons to blend the normals of their vertices        M
*   MaxAngle: Between approximated normal at vertex and polygon normal of    M
*	      the vertex to allow averaging. In degrees.		     M
*               If Negative, all vertices normals are cleared and all        M
*	      polygon normals reevaluated.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMUpdateVerticesByInterp, GMFixNormalsOfPolyModel                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBlendNormalsToVertices                                                 M
*****************************************************************************/
void GMBlendNormalsToVertices(IPPolygonStruct *PlList,
			      IrtRType MaxAngle)
{
    int i, VIndex,
	TotalVertices = 0;
    IrtRType Min, Max, R,
	CosMaxAngle = cos(IRIT_DEG2RAD(MaxAngle));
    IPVertexStruct *V;
    IPPolygonStruct *Pl;
    IritHashTableStruct *IHT;
    GMBBBboxStruct 
	*BBox = GMBBComputePolyListBbox(PlList);

    /* Clear all vertices normals and reevaluate polygons' normals. */
    if (MaxAngle < 0.0) {
        for (Pl = PlList; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct
		*V = Pl -> PVertex;

	    do {
	        IP_RST_NORMAL_VRTX(V);

		V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);

	    IPUpdatePolyPlane(Pl);
	}

        return;
    }

    /* Create a hash table to hold vertices and detect identities. */
    Min = IRIT_MIN(IRIT_MIN(BBox -> Min[0], BBox -> Min[1]), BBox -> Min[2]);
    Max = IRIT_MAX(IRIT_MAX(BBox -> Max[0], BBox -> Max[1]), BBox -> Max[2]);
    IHT = IritHashTableCreate(Min, Max, NRML_SAME_VRTX_EPS,
			      IPPolyListLen(PlList));

    for (Pl = PlList; Pl != NULL; Pl = Pl -> Pnext)
	TotalVertices += IPVrtxListLen(Pl -> PVertex);

    if (TotalVertices == 0)
	return;

    /* Allocate data structures to hold all vertices, matching similar ones. */
    GlblVertices = IritMalloc(sizeof(GMVertexInfoStruct) * TotalVertices);

    for (Pl = PlList, VIndex = 0; Pl != NULL; Pl = Pl -> Pnext) {
        V = Pl -> PVertex;
	GlblCrntPlBlendNormal = Pl;
        do {
	    if (!IritHashTableInsert(IHT, V, CmpTwoVertices,
				     IHT_VERTEX_KEY(V -> Coord), FALSE)) {
	        /* It is new data to hash table - prepare new info for V. */
	        IRIT_VEC_COPY(GlblVertices[VIndex].Normal, Pl -> Plane);
		GlblVertices[VIndex].CosMaxDeviate = 0.0;
		GlblVertices[VIndex].Count = 1;
	        AttrSetIntAttrib(&V -> Attr, "_vrtxIdx", VIndex++);
	    }

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    IritHashTableFree(IHT);

    /* Compute the average normals of the vertices and normalize. */
    for (i = 0; i < VIndex; i++) {
	if (IRIT_VEC_SQR_LENGTH(GlblVertices[i].Normal) > IRIT_SQR(IRIT_UEPS))
	    IRIT_VEC_NORMALIZE(GlblVertices[i].Normal);
    }

    /* Compute maximal deviation of normal from polygon planes. */
    for (Pl = PlList; Pl != NULL; Pl = Pl -> Pnext) {
        V = Pl -> PVertex;
        do {
	    i = AttrGetIntAttrib(V -> Attr, "_vrtxIdx");
	    R = IRIT_DOT_PROD(Pl -> Plane, GlblVertices[i].Normal);

	    if (GlblVertices[i].CosMaxDeviate < R)
	        GlblVertices[i].CosMaxDeviate = R;

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    /* Update original data structure. */
    for (Pl = PlList; Pl != NULL; Pl = Pl -> Pnext) {
        V = Pl -> PVertex;
        do {
	    i = AttrGetIntAttrib(V -> Attr, "_vrtxIdx");
	    AttrFreeOneAttribute(&V -> Attr, "_vrtxIdx");

	    if (IRIT_DOT_PROD(Pl -> Plane, GlblVertices[i].Normal) > CosMaxAngle) {
	        IRIT_VEC_COPY(V -> Normal, GlblVertices[i].Normal);
	    }
	    else {
	        IRIT_VEC_COPY(V -> Normal, Pl -> Plane);
	    }
	    IP_SET_NORMAL_VRTX(V);

	    AttrSetRealAttrib(&V -> Attr, "_CosNrmlMaxDeviation",
			      GlblVertices[i].CosMaxDeviate);

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    IritFree(GlblVertices);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Using computed adjacency information, propagate the orientation of first M
* polygon in the given poly object until all polygons are processed.         M
*   Disjoint poly-meshes will be marked with an "_OrientDisjoint" attribute  M
* on the first polygon of each disjoit part.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:   Polygons to reorient them all based on first polygon.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*    GMFixNormalsOfPolyModel                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFixOrientationOfPolyModel                                              M
*****************************************************************************/
void GMFixOrientationOfPolyModel(IPPolygonStruct *Pls)
{
    IRIT_STATIC_DATA IPPolygonStruct
        *GlblReorientStack[GM_REORIENT_STACK_SIZE];
    int i, j,
	OldCirc = IPSetPolyListCirc(TRUE),
	StackPointer = 0;
    IPObjectStruct
	*PDummy = IPGenPOLYObject(Pls);
    IPPolygonStruct *Pl;

    if (!OldCirc)
        GMVrtxListToCircOrLin(Pls, TRUE);
    BoolGenAdjacencies(PDummy);

    for (Pl = Pls, i = j = 0; Pl != NULL; Pl = Pl -> Pnext, i++)
        GM_RST_REORIENTED_POLY(Pl);

    GlblReorientStack[StackPointer++] = Pls;
    do {
        IPVertexStruct *V;

        Pl = GlblReorientStack[--StackPointer];
	GM_SET_REORIENTED_POLY(Pl);
	j++;

	/* Scan the neighbors of polygon Pl. */
	V = Pl -> PVertex;
	do {
	    IrtVecType Dir, AdjDir;
	    IPPolygonStruct
		*PAdj = V -> PAdj;
	    IPVertexStruct *VAdj;

	    if (PAdj != NULL &&
		!GM_IS_REORIENTED_POLY(PAdj) &&
		(VAdj = BoolGetAdjEdge(V)) != NULL) {
	        IRIT_VEC_SUB(Dir, V -> Pnext -> Coord, V -> Coord);
		IRIT_VEC_SUB(AdjDir, VAdj -> Pnext -> Coord, VAdj -> Coord);

		/* Should be opposite directions.  If not - reverse poly. */
	        if (IRIT_DOT_PROD(Dir, AdjDir) > 0.0) {
		    IRIT_PLANE_SCALE(PAdj -> Plane, -1);
		    IPReverseVrtxList(PAdj);
		}

	        if (StackPointer >= GM_REORIENT_STACK_SIZE)
		    GEOM_FATAL_ERROR(GEOM_ERR_REORIENT_STACK_OF);

	        GlblReorientStack[StackPointer++] = V -> PAdj;
	    }

	    V = V -> Pnext;
	}
	while (V != Pl -> PVertex);
    }
    while (StackPointer > 0);

    if (!OldCirc)
        GMVrtxListToCircOrLin(Pls, FALSE);
    IPSetPolyListCirc(OldCirc);
    PDummy -> U.Pl = NULL;
    IPFreeObject(PDummy);

    /* Make sure we visited all the polygons.  Could happen we did not if   */
    /* the object has several disjoint parts.				    */
    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	if (!GM_IS_REORIENTED_POLY(Pl)) {
	    /* separate the marked and unmarked polys and reinvoke on the   */
	    /* unmarked polygons, recursively.                              */
	    IPPolygonStruct
	        *PlHead = Pls,
		*MarkedPls = NULL,
		*UnMarkedPls = NULL;

	    Pls = Pls -> Pnext;

	    while (Pls) {
	        IRIT_LIST_POP(Pl, Pls);

		if (GM_IS_REORIENTED_POLY(Pl)) {
		    IRIT_LIST_PUSH(Pl, MarkedPls);
		}
		else {
		    IRIT_LIST_PUSH(Pl, UnMarkedPls);
		}
	    }

	    PlHead -> Pnext = MarkedPls;

	    AttrSetIntAttrib(&UnMarkedPls -> Attr, "_OrientDisjoint", TRUE),

	    IPGetLastPoly(PlHead) -> Pnext = UnMarkedPls;
	    GMFixOrientationOfPolyModel(UnMarkedPls);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fix orientation discrepancy between polygon normals and vertices normals.M
*                                                                            *
* PARAMETERS:                                                                M
*   PlList:         Polygonal object to correct normals.		     M
*   TrustFixedPt:   0 to trust the vertices' normal,			     M
*		    1 to trust the orientation of polygons' normals,	     M
*		    2 to reorient the polygons so all plane normals point    M
*		      outside or all inside (based on first poly).	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBlendNormalsToVertices, GMFixOrientationOfPolyModel                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFixNormalsOfPolyModel                                                  M
*****************************************************************************/
void GMFixNormalsOfPolyModel(IPPolygonStruct *PlList, int TrustFixedPt)
{
    IPPolygonStruct *Pl;

    if (TrustFixedPt == 2) {
        GMFixOrientationOfPolyModel(PlList);
	return;
    }

    /* TrustFixedPt == 0 or 1. */
    for (Pl = PlList; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct
	    *V = Pl -> PVertex;

	if (!IP_HAS_PLANE_POLY(Pl)) {
	    GEOM_FATAL_ERROR(GEOM_ERR_NO_POLY_PLANE);
	    continue;
	}

        do {
	    if (IP_HAS_NORMAL_VRTX(V)) {
		if (IRIT_DOT_PROD(Pl -> Plane, V -> Normal) < 0) {
		    /* We have a discrepancy. */
		    if (TrustFixedPt)
		        IRIT_VEC_SCALE(V -> Normal, -1)
		    else {
		        IRIT_PLANE_SCALE(Pl -> Plane, -1);
			IPReverseVrtxList(Pl);
			break;
		    }
		}
	    }
	    else
	        GEOM_FATAL_ERROR(GEOM_ERR_NO_VRTX_NRML);

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }
}

#ifdef DEBUG 

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the content of the given polygon, to standard output.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:         Polygon to print to stdout.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMPrintPolygon(IPPolygonStruct *Pl)
{
    IPVertexStruct
	*V = Pl -> PVertex,
	*VHead = V;

    do {
	printf("    %10lg %10lg %10lg (%10lg %10lg %10lg)",
	       V -> Coord[0], V -> Coord[1], V -> Coord[2],
	       V -> Normal[0], V -> Normal[1], V -> Normal[2]);
	if (IP_IS_INTERNAL_VRTX(V))
	    printf(" (Internal)\n");
	else
	    printf("\n");
	V = V -> Pnext;
    }
    while (V!= NULL && V != VHead);
}

#endif /* DEBUG */

