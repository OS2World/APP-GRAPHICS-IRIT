/*****************************************************************************
* Initializes the Triangle structure based on the original irit polygon.     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "triangle.h"
#include "zbuffer.h"
#include "vis_maps.h"

static void PolyAveragePoint(IPPolygonStruct *Poly, IrtPtType p);
static int IsPolyBackfaced(IPPolygonStruct *Poly,
			   IrtPtType Viewer,
			   int Parallel);
int VertexGetUVAttrAux(IPVertexStruct *Vertex,
		       IrtRType *u,
		       IrtRType *v);
static void CalcInterpol(IRndrEdgeStruct *Edge,
			 IPVertexStruct *v,
			 IPVertexStruct *vMid,
			 IrtRType *TVertex,
			 IRndrObjectStruct *o,
			 IRndrSceneStruct *Scene,
                         IPPolygonStruct *Triangle);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calulate the transformed vertex coordinate.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Vertex:       IN, pointer to the Vertex.                                 M
*   Matrices:     IN, pointer to matrices context.                           M
*   o:            IN, pointer to Object which contains the Triangle that     M
*                     contains the vertex. If o is NULL it's being ignored.  M
*   Result:       OUT, the result transformed homogenous coordinate.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   VertexTransform                                                          M
*****************************************************************************/
void VertexTransform(IPVertexStruct *Vertex,
                     IRndrMatrixContextStruct *Matrices,
                     IRndrObjectStruct *o,
                     IrtRType *Result)
{
    IrtRType w;

    IRIT_PT_COPY(Result, Vertex -> Coord);
    Result[3] = 1.0;
    if  ((o != NULL) && (o -> Animated == TRUE)) {
        MatMultWVecby4by4(Result, Result, o -> AnimationMatrix);
    }
    MatMultWVecby4by4(Result, Result, Matrices -> TransMat);
    w = 1 / Result[3];
    IRIT_PT_SCALE(Result, w);

    /* This function expects Result to be with w == 1 and the function's    */
    /* result is with w == 1 as well.                                       */
    MatMultPtby4by4(Result, Result, Matrices -> ScreenMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes average of the vertices points of the triangle polygon.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:    IN, pointer to the Irit polygon object.                         *
*   p:       OUT, result average point.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PolyAveragePoint(IPPolygonStruct *Poly, IrtPtType p)
{
    IPVertexStruct *v;

    IRIT_PT_RESET(p);
    for (v = Poly -> PVertex; v; v = v -> Pnext)
        IRIT_PT_ADD(p, p, v -> Coord);
    IRIT_PT_SCALE(p, 1.0 / 3.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Determines if viewer sees the backface side of the polygon, uses plane   *
*   equation to obtain normal value.                                         *
*   Assumes that normal is directed into the object normaly and both viewer  *
*   and Plane equation are in object space.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:      Pointer to Irit polygon object.                               *
*   Viewer:    Viewing location (perspective) or direction (orthographics).  *
*   Parallel:  TRUE for parallel (orthographic) projects,		     *
*              FALSE for persective.					     *
* RETURN VALUE:                                                              *
*   int:    Boolean value, zero if viewer sees front side of the polygon.    *
*****************************************************************************/
static int IsPolyBackfaced(IPPolygonStruct *Poly,
			   IrtPtType Viewer,
			   int Parallel)
{
    /* Assume that normal to the sphere is directed into sphere. */
    return !Parallel ?
        RNDR_PLANE_EQ_EVAL(Poly -> Plane, Viewer) > -IRIT_EPS :
        IRIT_DOT_PROD(Poly -> Plane, Viewer) > -IRIT_EPS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new Triangle object and allocates memory for it's fields.      M
*   Should be called before the first time the object is used.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Tri:       IN, pointer to the Triangle object.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TriangleInit                                                             M
*****************************************************************************/
void TriangleInit(IRndrTriangleStruct *Tri)
{
    int i;

    /* Allocate space to store interpolation data for every edge + one      */
    /* temporary val because light's number can change over time, we        */
    /* allocate the maximal number.					    */
    Tri -> Vals = RNDR_MALLOC(IRndrIntensivityStruct*, 4);
    Tri -> dVals = RNDR_MALLOC(IRndrIntensivityStruct*, 4);

    for (i = 0; i < 4; i++) {
        Tri -> Vals[i] = RNDR_MALLOC(IRndrIntensivityStruct,
				     RNDR_MAX_LIGHTS_NUM);
        Tri -> dVals[i] = RNDR_MALLOC(IRndrIntensivityStruct,
				      RNDR_MAX_LIGHTS_NUM);
    }
    Tri -> Validity = IRNDR_VISMAP_VALID_OK;
    Tri -> IsBackFaced = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the memory of a the Triangle.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Tri:       IN, pointer to the Triangle object.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TriangleRelease                                                          M
*****************************************************************************/
void TriangleRelease(IRndrTriangleStruct *Tri)
{
    int i;

    for (i = 0; i < 4; i++) {
        RNDR_FREE(Tri -> Vals[i]);
        RNDR_FREE(Tri -> dVals[i]);
    }

    RNDR_FREE(Tri -> Vals);
    RNDR_FREE(Tri -> dVals);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Caculates iterpolation values for an edge.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Edge:     Pointer to the edge.                                           *
*   v:        Pointer to the first vertex of the edge.                       *
*   vMid:     Pointer to the first middle vertex of the triangle.            *
*   TVertex:  The transformed coordinates of the vertex.                     *
*   o:        The object the triangle belongs to.                            *
*   Scene:    The scene context.                                             *
*   Triangle: The triangle which contain this edge.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CalcInterpol(IRndrEdgeStruct *Edge,
			 IPVertexStruct *v,
			 IPVertexStruct *vMid,
			 IrtRType *TVertex,
			 IRndrObjectStruct *o,
			 IRndrSceneStruct *Scene,
                         IPPolygonStruct *Triangle)
{
    int j, VRed, VGreen, VBlue,
        ShadeModal = Scene -> ShadeModel;
    double VRedFP, VGreenFP, VBlueFP; 
    IrtPtType Coord;
    IRndrInterpolStruct
        *Value = &Edge -> Value;
    Value -> IntensSize = Scene -> Lights.n;

    Edge -> dValue.IntensSize = Scene -> Lights.n;
    Edge -> x = IRIT_REAL_TO_INT(TVertex[RNDR_X_AXIS]);
    Edge -> YMin = IRIT_REAL_TO_INT(TVertex[RNDR_Y_AXIS]);
    Edge -> Value.z = TVertex[RNDR_Z_AXIS];
    Edge -> Value.w = 1 / TVertex[3];

    if (o -> DoVisMapCalcs) {
        IrtRType s, t;
        IrtPtType Pt, Uvz[4], VecS, VecT;
        IPVertexStruct *Vertex;
        int i;

        /* Relocate the point to be at the middle of the uv pixel. */
        IRIT_PT_SET(Pt, Edge -> x + 0.5, Edge -> YMin + 0.5, 0);
        IRndrVMIsPointInTriangle(Triangle, Pt, TRUE, NULL, &s, &t);
        for (Vertex = Triangle -> PVertex, i = 0; i <= 2; 
            i++, Vertex = Vertex -> Pnext) {
            Uvz[i][0] = AttrGetRealAttrib(v -> Attr, VIS_MAP_X_ATTRIB);
            Uvz[i][1] = AttrGetRealAttrib(v -> Attr, VIS_MAP_Y_ATTRIB);
            if ((Uvz[i][0] == IP_ATTR_BAD_REAL) ||
                (Uvz[i][1] == IP_ATTR_BAD_REAL)) {
                Uvz[i][0] = Uvz[i][1] = 0.0;
            }
            Uvz[i][2] = Vertex -> Coord[2];
        }
        IRIT_PT_SUB(VecS, Uvz[2], Uvz[0]);
        IRIT_PT_SUB(VecT, Uvz[1], Uvz[0]);
        IRIT_PT_SCALE_AND_ADD(Uvz[3], Uvz[0], VecS, s);
        IRIT_PT_SCALE_AND_ADD(Uvz[3], Uvz[3], VecT, t);
        /* Set XY to be the uv of value, since value's xy are actually UV. */ 
        Value -> u = Uvz[3][0];
        Value -> v = Uvz[3][1];
        Value -> z = Uvz[3][2];
    } 
    else if (o -> Txtr.Type == TEXTURE_TYPE_RSTR ||
        o -> Txtr.Type == TEXTURE_TYPE_PROC ||
        o -> Txtr.Type == TEXTURE_TYPE_SRF) {
        if (VertexGetUVAttrAux(v, &Value -> u, &Value -> v)) {
            Value -> u = (Value -> u - o -> Txtr.PrmUMin) /
                (o -> Txtr.PrmUMax - o -> Txtr.PrmUMin);
            Value -> v = (Value -> v - o -> Txtr.PrmVMin) /
                (o -> Txtr.PrmVMax - o -> Txtr.PrmVMin);
        }
        else {
            /* No UV Values.  Use the XY positions instead. */
            Value -> u = v -> Coord[0];
            Value -> v = v -> Coord[1];
        }
        Value -> u *= Value -> w;
        Value -> v *= Value -> w;
    }
    else {
        Value -> u = Value -> v = 0.0;
    }

    /* Handle normal. */
    IRIT_PT_COPY(Value -> n, v -> Normal);
    IRIT_PT_SCALE(Value -> n, Value -> w);
    if ((Value -> HasColor = AttrGetRGBDoubleColor(v -> Attr,
			            &VRedFP, &VGreenFP, &VBlueFP)) != FALSE) {
        Value -> c[0] = VRedFP;
        Value -> c[1] = VGreenFP;
        Value -> c[2] = VBlueFP;
    }     
    else if ((Value -> HasColor = AttrGetRGBColor(v -> Attr,
				    &VRed, &VGreen, &VBlue)) != FALSE) {
        Value -> c[0] = VRed / 255.0;
        Value -> c[1] = VGreen / 255.0;
        Value -> c[2] = VBlue / 255.0;
    }
    else
	IRIT_PT_RESET(Value -> c);
    Edge -> dValue.HasColor = Value -> HasColor;

    /* Handle interpol values. */
    switch (ShadeModal) {
        case RNDR_SHADING_GOURAUD:
            if (o -> Transformed )
		MatMultPtby4by4(Coord, v -> Coord,
				Scene -> Matrices.ViewInvMat);
	    else
		IRIT_PT_COPY(Coord, v -> Coord);

	    for (j = 0; j < Scene -> Lights.n; ++j) {
		LightIntensivity(&Scene -> Lights.Src[j], Coord, v -> Normal,
				 o, Scene, &Value -> i[j]);
		Value -> i[j].Diff *= Value -> w;
		Value -> i[j].Spec *= Value -> w;
	    }
	    break;
	case RNDR_SHADING_FLAT:
	    Edge -> dValue.i = NULL;
	    if (o -> Transformed )
		MatMultPtby4by4(Coord, vMid -> Coord,
				Scene -> Matrices.ViewInvMat);
	    else
		IRIT_PT_COPY(Coord, vMid -> Coord);

	    for (j = 0; j < Scene -> Lights.n ; ++j)
		LightIntensivity(&Scene -> Lights.Src[j], Coord,
				 vMid -> Normal, o, Scene, &Value -> i[j]);
	    break;
	default:
	    /* For Phong don't use interpolation values. */
	    Value -> i = NULL;
	    Edge -> dValue.i = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Wraps an Irit polygon and initialize the Triangle structure              M
* from polygon and object data.                                              M
*   That includes scan line and interpolation algorithm data initialization. M
*                                                                            *
* PARAMETERS:                                                                M
*   Tri:     OUT, pointer to the Triangle object.                            M
*   Poly:    IN, pointer to Irit polygon object.                             M
*   o:       IN, pointer to Object which contains a Triangle and stores      M
*            various charactarisitics common to every polygon in the object. M
*   Scene:   IN, pointer to the scene context.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: 1 in successful, 0 if polygon is not OK.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TriangleSet                                                              M
*****************************************************************************/
int TriangleSet(IRndrTriangleStruct *Tri,
                IPPolygonStruct *Poly,
                IRndrObjectStruct *o,
                IRndrSceneStruct *Scene)
{
    int e, i, XMax = 0, XMin = 0, YMin, YMinXMin;
    IRndrEdgeStruct *PEdge, First;
    IPVertexStruct *v, VertexMid, *Vertex[3];
    IrtRType TVertex[3][4];
    IrtPtType Viewer;

    if (!Poly || AttrGetRealAttrib(Poly -> Attr, "_INVIS") == 1)
        return 0;
    IRIT_PT_COPY(&Viewer, Scene -> Matrices.Viewer);
    for (i = 0; i < 3; i++) {
        Tri -> Edge[i].Value.i = Tri -> Vals[i];
        Tri -> Edge[i].dValue.i = Tri -> dVals[i];
    }

    /* Simplify access to vertices. */
    for (v = Poly -> PVertex, i = 0; v != NULL; v = v -> Pnext, i++) {
        Vertex[i] = v;
    }

    /* In flat model we evalute everything with respect to the average point.*/
    if (Scene -> ShadeModel == RNDR_SHADING_FLAT) {
        PolyAveragePoint(Poly, VertexMid.Coord);

        IRIT_PT_COPY(VertexMid.Normal, Poly -> Plane);
        IRIT_PT_NORMALIZE(VertexMid.Normal);
    }

    /* Check edges length. */
    if (!o -> DoVisMapCalcs) {
        IrtPtType edge;

        for (i = 0; i < 3; i++) {
            IRIT_VEC_SUB(edge, Vertex[i] -> Coord, Vertex[(i+1)%3] -> Coord);
            if (IRIT_VEC_LENGTH(edge) < RNDR_MIN_TRI_EDGE_LENGTH) {
                return 0;
            }
        }
    }

    /* Transform vertices. */
    if (o -> Transformed != TRUE && o -> DoVisMapCalcs != TRUE) {
        for (i = 0; i < 3; i++) {
            VertexTransform(Vertex[i], &Scene -> Matrices, o, TVertex[i]);
        }
    }
    else {
        for (i = 0; i < 3; i++) {
            IRIT_PT_COPY(TVertex[i], Vertex[i] -> Coord);
            TVertex[i][3] = AttrGetRealAttrib(Vertex[i] -> Attr, "_1/W");
        }
    }
    for (i = 0; i < 3; i++) {
        if (TVertex[i][RNDR_W_AXIS] < 0) {
	    static int
		PrintIt = FALSE;

	    if (!PrintIt) {
		_IRndrReportWarning(IRIT_EXP_STR("Negative w coorinate\n"));
	        PrintIt = TRUE;
	    }
	    return 0;
	}
    }

    /* Degenerated triangle doesn't care about normals. */
    if ((Tri -> Validity != IRNDR_VISMAP_VALID_DEGEN) &&
        IRIT_APX_EQ(Poly -> Plane[RNDR_X_AXIS], 0) && /* Do not create       */
        IRIT_APX_EQ(Poly -> Plane[RNDR_Y_AXIS], 0) && /* a triangle for a    */
        IRIT_APX_EQ(Poly -> Plane[RNDR_Z_AXIS], 0))   /* poly with zero      */
        return 0;				      /* length normal.      */

    Tri -> IsBackFaced = FALSE;
    if (Scene -> BackFace &&
	IsPolyBackfaced(Poly, Viewer,
			Scene -> Matrices.ParallelProjection)) {
        Tri -> IsBackFaced = TRUE;

        /* Dont skip back side polygons, when scanning UV coordinates. */
        if (o -> DoVisMapCalcs == FALSE) {
            /* In XYZ scanning for visiblity map the backside polygon can    */
            /* occlude so they shouldn't have been removed. However, any     */
            /* backside polygon should have faceside polygon that occlude it,*/
            /* so it's safe to remove it.                                    */
            return 0;               /* Skip back side polygons if user asks. */
        }
    }

    Tri -> Object = o;                        /* Connect to container object */
    Tri -> Poly = Poly;                        /* and to the source polygon. */

    /* Count edges and min-max dimensions for the polygon. */
    YMinXMin = XMin = Tri -> YMin = IRIT_MAX_INT;
    XMax = Tri -> YMax = -IRIT_MAX_INT;
    Tri -> ZMin = IRIT_INFNTY;
    Tri -> ZMax = -IRIT_INFNTY;
    for (i = 0; i < 3; i++) {
        YMin = IRIT_REAL_TO_INT(TVertex[i][RNDR_Y_AXIS]);
        if (YMin < Tri -> YMin) {
            Tri -> YMin = YMin;
            YMinXMin = IRIT_REAL_TO_INT(TVertex[i][RNDR_X_AXIS]);
        }
        else if (YMin == Tri -> YMin) {
            RNDR_MINM(YMinXMin, IRIT_REAL_TO_INT(TVertex[i][RNDR_X_AXIS]));
        }
	if (Tri -> ZMin > TVertex[i][RNDR_Z_AXIS])
	    Tri -> ZMin = TVertex[i][RNDR_Z_AXIS];
	if (Tri -> ZMax < TVertex[i][RNDR_Z_AXIS])
	    Tri -> ZMax = TVertex[i][RNDR_Z_AXIS];
        RNDR_MAXM(Tri -> YMax, YMin);
        RNDR_MINM(XMin, IRIT_REAL_TO_INT(TVertex[i][RNDR_X_AXIS]));
        RNDR_MAXM(XMax, IRIT_REAL_TO_INT(TVertex[i][RNDR_X_AXIS]));
    }

    /* We have no deal with Triangles out of image rectangle. */
    if ((Tri -> YMax < 0) ||
        (Tri -> YMin >= Scene -> SizeY) ||
        (XMax < 0) ||
        (XMin >= Scene -> SizeX)) {
        return 0;
    }
    YMin = Tri -> YMin;

    /* Obtain and initialize Triangle's vertices current (initial)           */
    /* characteristics which are scan line algorithm values, and             */
    /* interpolants such as normal, intensivity, homogeneous coordinate...   */
    for (i = 0; i < 3; i++) {
        CalcInterpol(&Tri -> Edge[i], Vertex[i], &VertexMid, TVertex[i],
		     o, Scene, Poly);
    }

    /* Create and initalize interpolation values data structure. */
    First = Tri -> Edge[0];

    if (Scene -> ShadeModel == RNDR_SHADING_PHONG)
        First.Value.i = NULL;
    else
        First.Value.i = Tri -> Vals[3];                /* The temporary val. */

    InterpolCopy(&First.Value, &Tri -> Edge -> Value);
    for (e = 0; e < 3; ++e) {
        IrtRType dy;
        IRndrEdgeStruct *Next;

        PEdge = &Tri -> Edge[e];
        Next = (e + 1 == 3) ? &First : PEdge + 1;
        dy = PEdge -> dy = Next -> YMin - PEdge -> YMin;

        InterpolDelta(&PEdge -> dValue, &Next -> Value, &PEdge -> Value, dy);
        if (PEdge -> dy < 0) {
            PEdge -> dx = PEdge -> x - Next -> x;
            PEdge -> x = Next -> x;
            PEdge -> YMin = Next -> YMin;
            InterpolCopy(&PEdge -> Value, &Next -> Value);
        }
        else
            PEdge -> dx = Next -> x - PEdge -> x;
        PEdge -> Inc = PEdge -> dy = IRIT_ABS(PEdge -> dy);
    }

    /* Sort edges, keep sorted in SortedEdge array.                         */
    /* SortedEdge[0] is the left one, SortedEdge[1] the right one and       */
    /* SortedEdge[2] the last. We ignore horizonal edges.                   */
    Tri -> SortedEdge[0] = Tri -> SortedEdge[1] = Tri -> SortedEdge[2] = NULL;

    for (i = 0; i < 3; ++i) {
        if ( (Tri -> Edge[i].dy != 0) && (Tri -> Edge[i].YMin == YMin)) {
            if ((Tri -> Edge[i].x == YMinXMin) &&
                (!Tri -> SortedEdge[0] ||
		 ((IrtRType) Tri -> Edge[i].dx / Tri -> Edge[i].dy) <
		     ((IrtRType) Tri -> SortedEdge[0] -> dx /
		                                Tri -> SortedEdge[0] -> dy))) {
                /* Found the left edge. */
                if (!Tri -> SortedEdge[1])
                    Tri -> SortedEdge[1] = Tri -> SortedEdge[0];
                Tri -> SortedEdge[0] = &Tri -> Edge[i];
            }

            /* Found the right edge. */
            else
                Tri -> SortedEdge[1] = &Tri -> Edge[i];
        }
        else if (Tri -> Edge[i].dy != 0) {
            Tri -> SortedEdge[2] = &Tri -> Edge[i];
        }
    }

    if (!Tri -> SortedEdge[0]) {
	for (i = 0; i < 3; i++) {
            if (Tri -> Edge[i].x == XMin )
                Tri -> SortedEdge[0] = &Tri -> Edge[i];
            if (Tri -> Edge[i].x == XMax)
                Tri -> SortedEdge[1] = &Tri -> Edge[i];
        }
    }

    return 1;
}
