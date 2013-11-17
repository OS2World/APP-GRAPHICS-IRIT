/*****************************************************************************
* Visibility maps creation, manipulation and access.                         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by: Eyal Posener, Nov 2009                                         *
*****************************************************************************/

#include "vis_maps.h"
#include "zbuffer.h"

IRIT_STATIC_DATA const char 
    *VIS_MAP_UV_VALUES = "uvvals";

const IrtImgPixelStruct
    RNDR_VISMAP_EMPTY_COLOR   = {255, 255, 255}, /* White. */
    RNDR_VISMAP_MAPPED_COLOR  = {255, 0, 0}, /* Red. */
    RNDR_VISMAP_VISIBLE_COLOR = {0, 125, 0}, /* Dark green. */
    RNDR_VISMAP_TANGENT_COLOR = {255, 255, 0}, /* Yellow. */
    RNDR_VISMAP_POOR_AR_COLOR = {255, 255, 0}, /* Yellow. */
    RNDR_VISMAP_DEGEN_COLOR   = {255, 255, 255}; /* White. As if it's empty. */

/* This value must be identical to GM_COLLINEAR_EPS in geom_bsc.c. */
#define VIS_MAP_COLLINEAR_EPS  1e-10

static int VisMapTriangleSet(IRndrTriangleStruct *RendTri,
                IPPolygonStruct *Triangle,
                IRndrObjectStruct *Obj,
                IRndrSceneStruct *Scene);
static int VisMapSwitchTriangleSpaces(IRndrVMStruct *VisMap,
                               IRndrSceneStruct *Scene,
                               IRndrObjectStruct *Obj,
                               IPPolygonStruct *Triangle,
                               int Reverse);
/* This was visibility map code. It was replaced by another algorithm. */
#ifdef VM_USE_OLD_CODE 
static int VisMapTriangleDZ(IRndrVMStruct *Vismap,
                                 IPPolygonStruct *Triangle,
                                 IRndrVisibleValidityType *Validity,
                                 IrtRType *dz);
#endif
static int VisMapCheckValidity(IRndrVMStruct *Vismap,
                               IPPolygonStruct *Triangle,
                               IRndrVisibleValidityType *Validity);
static void VisMapDilation(IRndrVMStruct *VisMap);
static void VisMapRestoreVector(IRndrVMStruct *VisMap, IrtPtType v);
static int VisMapIsPoorAR(IRndrVMStruct *VisMap, IrtPtType v1, IrtPtType v2);
static int VisMapIsTangentZAxis(IRndrVMStruct *VisMap, IrtPtType Norm);
/* Scene fitting. */
static void VisMapFindLimits(IRndrVMStruct *VisMap,
                                      IPObjectStruct *OList);
static void VisMapSetRefreshLimits(IRndrVMStruct *VisMap, 
                                     IPVertexStruct *Vertex);
static void VisMapMakeFittingMatrices(IRndrVMStruct *VisMap);
/* Functions for the relation between point and a triangle. */
static int VisMapIntersect(IrtPtType Pt1, 
                           IrtPtType V1, 
                           IrtPtType Pt2, 
                           IrtPtType V2,
                           IrtRType *S,
                           IrtRType *T,
                           IrtPtType Intersection);
static int VisMapVectorToEdge(IrtPtType E1, 
                              IrtPtType E2, 
                              IrtPtType Pt, 
                              IrtPtType Res);
static int VisMapIsPointCovered(IRndrVMStruct *VisMap,
                                IrtPtType Pt, 
                                IPPolygonStruct *Triangle, 
                                IRndrZBufferStruct *Buffer);
static void VisMapPrepareUVValuesOfGeoObjSort(int *Indexes, IrtRType *Area, 
                                              int Length);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize a newly created visibility map.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:     IN, OUT, Pointer to visibility map object.                   M
*   Scene:      IN, Pointer to the related scene object.                     M
*   SuperSize:  IN, Super sampling size.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  0 if successful.                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMInit, initialize, create visibility map.                          M
*****************************************************************************/
int IRndrVMInit(IRndrVMStruct *VisMap, IRndrSceneStruct *Scene, int SuperSize)
{
    int u, v;
    IRndrUVListStruct Dummy, **Vertex;

    if (!VisMap || !Scene)
        return -1;

    assert(Scene -> SizeX >= 1 && Scene -> SizeY >= 1);

    Dummy.Value = IRNDR_VISMAP_FILL_EMPTY;
    Dummy.BackFaced = FALSE;
    Dummy.Validity = IRNDR_VISMAP_VALID_OK;
    Dummy.Triangle = NULL;

    VisMap -> Scene = Scene;
    VisMap -> InvScreenMatValid = FALSE;
    VisMap -> SizeU = Scene -> SizeX;
    VisMap -> SizeV = Scene -> SizeY;
    VisMap -> TargetSizeU = Scene -> SizeX / SuperSize;
    VisMap -> TargetSizeV = Scene -> SizeY / SuperSize;

    Vertex = RNDR_MALLOC(IRndrUVListStruct *, VisMap -> SizeV);

    for (v = 0; v < VisMap -> SizeV; ++v) {
        Vertex[v] = RNDR_MALLOC(IRndrUVListStruct, VisMap -> SizeU);
        for (u = 0; u < VisMap -> SizeU; ++u) {
            memcpy(&Vertex[v][u], &Dummy, sizeof(IRndrUVListStruct));
        }
    }
    VisMap -> UVMap = Vertex;
    VisMap -> MinU =  (float)IRIT_MAX_INT;
    VisMap -> MinV =  (float)IRIT_MAX_INT;
    VisMap -> MaxU = -(float)IRIT_MAX_INT;
    VisMap -> MaxV = -(float)IRIT_MAX_INT;
    VisMap -> SuperSize = SuperSize;
    VisMap -> CosTanAng = RNDR_UV_CRITIC_TAN;
    VisMap -> CriticAR = RNDR_UV_CRITIC_AR;
    VisMap -> DilationItarations = RNDR_UV_DILATION_ITERATIONS;
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set scene and visibility map limits according to the XY and UV maximum   M 
* and minimum coordinates in all objects. Also, sets the scene's screen      M
* matrix accoridng to the XY limits so the entrie scene will be in the       M
* scene's window and centered. That means that the entire scene's x and y    M
* coordinates will be in the range [0,scene -> sizex/y] (z coordinate isn't  M
* changed).                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:     IN, OUT, Pointer to visibility map object.                   M
*   Objects:    IN, Rendered object list.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMSetLimits, set, limits, UV.                                       M
*****************************************************************************/
void IRndrVMSetLimits(IRndrVMStruct *VisMap, IPObjectStruct* Objects)
{
    if (!VisMap || !Objects) {
        return;
    }

    VisMapFindLimits(VisMap, Objects);
    VisMapMakeFittingMatrices(VisMap);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setting critic cosines value of angle between view point to pixel and    M
*   triangle normal.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to the visibility map.                       M
*   CosAng:    IN, value of critic cosines value of angle between normal and M
*              view vector.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMSetTanAngle                                                       M
*****************************************************************************/
void IRndrVMSetTanAngle(IRndrVMStruct *VisMap, IrtRType CosAng)
{
    if (!VisMap || !RNDR_IN(CosAng, 0.0, 1.0)) {
        return;
    }

    VisMap -> CosTanAng = CosAng;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setting critic AR value.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to the visibility map.                       M
*   CriticAR:  IN, value of critic aspect ratio value, which is ratio        M
*              between the largest and smallest edge of a triangle.          M
*              use value 0 for skeeping AR check.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMSetCriticAR                                                       M
*****************************************************************************/
void IRndrVMSetCriticAR(IRndrVMStruct *VisMap, IrtRType CriticAR)
{
    if (!VisMap || (CriticAR < 0)) {
        return;
    }

    VisMap -> CriticAR = CriticAR;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setting dilation iterations value, this will define how many times the   M
* dilation algorithm will be executed, expanding one pixel at a time.        M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to the visibility map.                       M
*   Dilation:  IN, the amount of iterations to do the dilation algorithm.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMSetDilation                                                       M
*****************************************************************************/
void IRndrVMSetDilation(IRndrVMStruct *VisMap, int Dilation)
{
    if (!VisMap || (Dilation < 0)) {
        return;
    }

    VisMap -> DilationItarations = Dilation;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the context of visibility map (Sets all UV information to initial M
*   values).                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, OUT, Pointer to visibility map.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMClear, clear visibility map.                                      M
*****************************************************************************/
void IRndrVMClear(IRndrVMStruct *VisMap)
{
    int u,v;
    IRndrUVListStruct Dummy;

    if (!VisMap) {
        return;
    }

    Dummy.Value = IRNDR_VISMAP_FILL_EMPTY;
    Dummy.BackFaced = FALSE;
    Dummy.Validity = IRNDR_VISMAP_VALID_OK;
    Dummy.Triangle = NULL;

    for (v = 0; v < VisMap -> SizeV; ++v) {
        for (u = 0; u < VisMap -> SizeU; ++u) {
            memcpy(&VisMap -> UVMap[v][u], &Dummy, sizeof(IRndrUVListStruct));
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release the memory taken by the visibility map.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN,OUT, pointer to the visibility map.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMRelease, memory free, release                                     M
*****************************************************************************/
void IRndrVMRelease(IRndrVMStruct *VisMap)
{
    int v;

    if (!VisMap) {
        return;
    }
   
    for (v = 0; v < VisMap -> SizeV; ++v) {
        RNDR_FREE(VisMap -> UVMap[v]);
    }
    RNDR_FREE(VisMap -> UVMap);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* The method gets a scanned triangle polygon. It generates a triangle having M
* u-v-z coordinates for each vertex and x-y coordinates stored in 'uvvals'   M
* attribute. Then scans it into visibility map using the regular scan        M
* convention                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:      IN,OUT, pointer to the visibility map.                      M
*   RendTri:     IN, renderer current rendered triangle.                     M
*   Scene:       IN, scene pointer.                                          M
*   Obj:         IN, current rendered object.                                M
*   Triangle:    IN, a polygon triangle represented which was scanned into   M
*                      Z-buffer, with UV values in 'uvvals' attribute, in    M
*                      each vertex. This object is changed during this       M
*                      function (So it can't use for other purposes after    M
*                      this function).                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Whether the method finished successfully                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMPutTriangle, VisMap, UV, Triangle                                 M
*****************************************************************************/
int IRndrVMPutTriangle(IRndrVMStruct *VisMap,
                      IRndrTriangleStruct *RendTri,
                      IRndrSceneStruct *Scene,
                      IRndrObjectStruct *Obj,
                      IPPolygonStruct *Triangle)
{
    int retval, IsBackFaced;
#ifdef VM_USE_OLD_CODE 
    IrtRType
	dz = 0;
#endif

    if (!VisMap || !RendTri || !Scene || !Obj || !Triangle) {
        return FALSE;
    }

    /* Switch Triangle spaces such that x and y will contain the u and v     */
    /* values. The x and y values are backup in attributes VIS_MAP_X_ATTRIB  */
    /* and VIS_MAP_Y_ATTRIB.                                                 */
    VisMapSwitchTriangleSpaces(VisMap, Scene, Obj, Triangle, FALSE);

    /* Remembering if XYZ spaced triangle is back faced. */
    IsBackFaced = RendTri -> IsBackFaced;

/* This was visibility map code. It was replaced by another algorithm. */
#ifdef VM_USE_OLD_CODE 
    /* Calculate dz and make validity check. */
    if (!VisMapTriangleDZ(VisMap, Triangle, &RendTri -> Validity, &dz)) {
#endif
    if (!VisMapCheckValidity(VisMap, Triangle, &RendTri -> Validity)) {        
        return FALSE;
    }

    /* TriangleSet the UVZ spaced triangle. */
    retval = VisMapTriangleSet(RendTri, Triangle, Obj, Scene);
    
/* This was visibility map code. It was replaced by another algorithm. */
#ifdef VM_USE_OLD_CODE 
    /* Saving z error value for passing it into vertex z value. */
    RendTri -> dz = dz;
#endif

    /* Restore XYZ space back facing property. */
    RendTri -> IsBackFaced = IsBackFaced;

    /* Switch back Triangle spaces. */
    VisMapSwitchTriangleSpaces(VisMap, Scene, Obj, Triangle, TRUE);

    return retval;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check whether Pt is inside Triangle. (The calculation are done using     M
* only the xy coordinates). z returns the z value of triangle at the xy      M
* coordinates given by Pt (it may return the z value at the continuation     M
* of Triangle if xy is outside Triangle). s and t are the parameters which   M
* create Pt in the plane formula:                                            M
*   Triangle[0] + s*(Triangle[2]-Triangle[0]) + t*(Triangle[1]-Triangle[0])  M
*                                                                            *
* PARAMETERS:                                                                M
*   Triangle: IN, The triangle.                                              M
*   Pt:       IN, The point.                                                 M
*   Perim:    IN, If TRUE, points on the perimeter are considered as being   M
*             in the triangle. If FALSE they are not in the triangle.        M
*   z:        OUT, The z value of Triangle at the xy values given by Pt.     M
*             Ignored if it's NULL.                                          M
*   s, t:     The paramters of Pt in the plane forumla. Ignored if NULL.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Whether Pt is inside Triangle                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMIsPointInTriangle                                                 M
*****************************************************************************/
int IRndrVMIsPointInTriangle(IPPolygonStruct *Triangle, 
                             IrtPtType Pt,
                             int Perim,
                             IrtRType *z,
                             IrtRType *s,
                             IrtRType *t) 
{
    IrtPtType u, v;
    IrtRType LocalS, LocalT, *V0, *V1, *V2;

    V0 = Triangle -> PVertex -> Coord;
    V1 = Triangle -> PVertex -> Pnext -> Coord;
    V2 = Triangle -> PVertex -> Pnext -> Pnext -> Coord;

    IRIT_PT_SUB(u, V2, V0);
    IRIT_PT_SUB(v, V1, V0);
    LocalS = (v[1] * (Pt[0] - V0[0]) - (Pt[1] - V0[1]) * v[0]) /
                                       (u[0] * v[1] - v[0] * u[1]);
    LocalT = (u[0] * (Pt[1] - V0[1]) - (Pt[0] - V0[0]) * u[1]) /
                                       (u[0] * v[1] - v[0] * u[1]);
    if (s != NULL)
        *s = LocalS;
    if (t != NULL)
        *t = LocalT;
    if (z != NULL)
        *z = V0[2] + LocalS * u[2] + LocalT * v[2];

    /* If Perim is false and the point is on the perimeter returns FALSE. */
    if (!Perim && (IRIT_APX_EQ(LocalS, 0) ||
		   IRIT_APX_EQ(LocalT, 0) || 
		   IRIT_APX_EQ(LocalS + LocalT, 1)))
        return FALSE;

    return (RNDR_IN(LocalS, 0, 1) &&
	    RNDR_IN(LocalT, 0, 1) &&
	    (LocalS + LocalT <= 1));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns the intersection between Pt1 + s*V1 and Pt2 + t*V2 (ignoring the *
*   z coordinate when calculating the intersection) and also returns s and t *
*   of the intersection.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, V1:      IN, The first line.                                        *
*   S:            OUT, The parameter of the intersection of the first line.  *
*   Pt2, V2:      The second line.                                           *
*   T:            OUT, The parameter of the intersection of the second line. *
*   Intersection: The intersection between the two lines.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if the lines are parallel (S, T and intersection are          *
*        undefiend in that case).                                            *
*                                                                            *
* KEYWORDS:                                                                  *
*   VisMapIntersect                                                          *
*****************************************************************************/
static int VisMapIntersect(IrtPtType Pt1, 
                           IrtPtType V1, 
                           IrtPtType Pt2, 
                           IrtPtType V2,
                           IrtRType *s,
                           IrtRType *t,
                           IrtPtType Intersection)
{
    IrtRType 
        Pt1xSubPt2x = Pt1[0] - Pt2[0],
        Pt2ySubPt1y = Pt2[1] - Pt1[1],
        V1yV2xSubV2yV1x = V1[1] * V2[0] - V2[1] * V1[0];

    if (IRIT_APX_EQ_EPS(V1yV2xSubV2yV1x, 0, IRIT_EPS * IRIT_EPS))
        return FALSE;
    *s = (V2[1] * Pt1xSubPt2x + V2[0] * Pt2ySubPt1y) / V1yV2xSubV2yV1x;
    *t = (V1[1] * Pt1xSubPt2x + V1[0] * Pt2ySubPt1y) / V1yV2xSubV2yV1x;
    IRIT_PT_SCALE_AND_ADD(Intersection, Pt1, V1, *s);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Return the vector from the edge (E1, E2) to Pt such that the projection  *
*   of the vector and the edge on the XY plane will be perpendicular.        *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2: IN, Vertices of an edge.                                         *
*   Pt:     IN, The given point.                                             *
*   Res:    OUT, The vector from the edge (E1, E2) to Pt.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if the intersection is on (E1, E2).                         *
*           FALSE in any of the followng cases:                              *
*             The intersection is on the continuation of (E1, E2).           *
*             E1 and E2 project to the same point on XY plan.                *
*                                                                            *
* KEYWORDS:                                                                  *
*   VisMapVectorToEdge                                                       *
*****************************************************************************/
static int VisMapVectorToEdge(IrtPtType E1, 
                              IrtPtType E2, 
                              IrtPtType Pt, 
                              IrtPtType Res)
{
    IrtPtType u, U2d, Norm, z, Intersection;
    IrtRType s, t;

    IRIT_PT_SUB(u, E2, E1);
    IRIT_PT_SET(U2d, u[0], u[1], 0);
    IRIT_PT_SET(z, 0, 0, 1);
    IRIT_CROSS_PROD(Norm, U2d, z);
    if ( IRIT_PT_LENGTH(Norm) < IRIT_PT_NORMALIZE_ZERO)
        return FALSE;
    IRIT_PT_NORMALIZE(Norm);

    /* By definition if E1 != E2 there must be intersection but I left the  */
    /* condition anyway just in case.                                       */
    if (!VisMapIntersect(E1, u, Pt, Norm, &s, &t, Intersection) ||
        (s < - IRIT_EPS*IRIT_EPS ) || (s > 1.0+IRIT_EPS*IRIT_EPS))
        return FALSE;
    IRIT_PT_SUB(Res, Pt, Intersection);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If Pt isn't inside Triangle, relcoation it to be the closest point       M
*  inside triangle (The calculation are done only with the xy coordinates).  M
*                                                                            *
* PARAMETERS:                                                                M
*   Triangle: IN, The triangle.                                              M
*   Pt:       IN, The point.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMRelocatePtIntoTriangle                                            M
*****************************************************************************/
void IRndrVMRelocatePtIntoTriangle(IPPolygonStruct *Triangle, IrtPtType Pt) 
{
    int i, longEnough;
    IrtRType *v[3],
        BestLength = IRIT_MAX_INT;
    IrtPtType Vector, BestVector, PtMoveVector, PtOnTriangle,
        PtOnCenterOfMass, VectorFromCOM, ThirdFactor;

    /*   We make sure the point is inside the triangle. Points on the       */
    /* perimeter of the target aren't good either (to prevent occlusion by  */
    /* adjacent triangle).                                                  */
    if (IRndrVMIsPointInTriangle(Triangle, Pt, FALSE, NULL, NULL, NULL))
        return;

    v[0] = Triangle -> PVertex -> Coord;
    v[1] = Triangle -> PVertex -> Pnext -> Coord;
    v[2] = Triangle -> PVertex -> Pnext -> Pnext -> Coord;

    /* Find the vector from the closest edge or vertex. */
    for (i = 0; i <= 2; i++) {
        if (VisMapVectorToEdge(v[i], v[(i + 1) % 3], Pt, Vector) && 
            (IRIT_PT_LENGTH(Vector) < BestLength)) {
            IRIT_PT_COPY(BestVector, Vector);
            BestLength = IRIT_PT_LENGTH(Vector);
        }
    }
    for (i = 0; i <= 2; i++) {
        IRIT_PT_SUB(Vector, Pt, v[i]);
        if (IRIT_PT_LENGTH(Vector) < BestLength) {
            IRIT_PT_COPY(BestVector, Vector);
            BestLength = IRIT_PT_LENGTH(Vector);
        }
    }

    /*   Find a point inside the triangle which is on the continuation of  */
    /* the line from Pt to <closest point on Triangle perimeter to Pt>.    */
    IRIT_PT_SUB(PtOnTriangle, Pt, BestVector);
    IRIT_PT_SCALE(BestVector, 0.1);
    IRIT_PT_COPY(PtMoveVector, BestVector);
    longEnough = 0;
    while ((IRIT_PT_LENGTH(PtMoveVector) > IRIT_EPS * IRIT_EPS) && 
            /*   If we still in after 15 loops there must be some           */
            /* computational problem such as infinite vector length.        */
	   (longEnough < 15)) { 
        IRIT_PT_SUB(Pt, PtOnTriangle, PtMoveVector);
        if (IRndrVMIsPointInTriangle(Triangle, Pt, FALSE, NULL, NULL, NULL))
            return;
        IRIT_PT_SCALE(PtMoveVector, 0.5);
        longEnough++;
    }

    /* If last method didn't work. Just find a point on the line from       */
    /* < closest point on Triangle perimeter to Pt > to the center of mass  */
    /* of Triangle.                                                         */
    IRIT_PT_SET(ThirdFactor, 1./3, 1./3, 1./3);
    IRIT_PT_BLEND_BARYCENTRIC(PtOnCenterOfMass, v[0], v[1], v[2], ThirdFactor);
    IRIT_PT_SUB(VectorFromCOM, PtOnTriangle, PtOnCenterOfMass);
    if (IRIT_PT_LENGTH(BestVector) < IRIT_PT_LENGTH(VectorFromCOM)) {
        /*   Notice that you can't use in IRIT_PT_SCALE a second parameter  */
        /* which depends on the vector itself.                              */
        IrtRType 
            Scale = IRIT_PT_LENGTH(BestVector)/IRIT_PT_LENGTH(VectorFromCOM);

        IRIT_PT_SCALE(VectorFromCOM, Scale);
    }
    IRIT_PT_SUB(Pt, PtOnTriangle, VectorFromCOM);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check whether the given point is covered by any other triangle.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VisMap:   IN, pointer to the visibility map.                             *
*   Pt:       IN, Point to check whether it covered by any other triangle.   *
*   Triangle: IN, The triangle which created Pt.                             *
*   Buffer:   IN, The zbuffer with all the triangles.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Whether this point is covered by other polygon.                *
*****************************************************************************/
static int VisMapIsPointCovered(IRndrVMStruct *VisMap,
                                IrtPtType Pt, 
                                IPPolygonStruct *Triangle, 
                                IRndrZBufferStruct *Buffer)
{
    int i, j,
        k = 1,    /* The radius of cells to look for the covering triangle. */
        x = IRIT_REAL_TO_INT(Pt[0]),
        y = IRIT_REAL_TO_INT(Pt[1]);
    /* There are two assumptions here:                                      */
    /* No picture has more than 2^32 pixels, so no overlap of point numbers.*/
    /* IAux3 is initialized to 0 in all triangles (as it is initialized in  */
    /* any creation of polygon).                                            */
    IRIT_STATIC_DATA int
        PointNum = 0;

    PointNum++;

    /* 0 is illegal point number since IAux3 is inialized to 0. */
    if (PointNum == 0)
        PointNum++;    

    for (i = y - k; i <= y + k; i++) {
        for (j = x - k; j <= x + k; j++) {
            IRndrZPointStruct *z;
            IrtRType TriangleZ;

            if (!RNDR_IN(i, 0, VisMap -> SizeV - 1) || 
                !RNDR_IN(j, 0, VisMap -> SizeU - 1))
                continue;
            z = &Buffer -> z[i][j].First;
            for (; z != NULL; z = z -> Next) {                
                if ((z -> Triangle == NULL) || (z -> Triangle == Triangle))
                    continue;

                /* Skipping triangles we have already checked. */
                if (z -> Triangle -> IAux3 == PointNum)
                    continue;
                if (IRndrVMIsPointInTriangle(z -> Triangle, Pt, TRUE, 
                                             &TriangleZ, NULL, NULL) &&
		    (TriangleZ > Pt[2])) 
                    return TRUE;

                z -> Triangle -> IAux3 = PointNum;
            }           
        }
    }
    return FALSE;
}

/*****************************************************************************
* AUXILIARY:								     *
*   Auxilary function to IRndrVMPrepareUVValuesOfGeoObj.                     *
*   Sort the indexes arrays by the areas from large to small.                *
*****************************************************************************/
static void VisMapPrepareUVValuesOfGeoObjSort(int *Indexes,
					      IrtRType *Area, 
                                              int Length)
{
    int i, j;

    for (i = Length - 2; i >= 0; i--)
        for (j = 0; j <= i; j++)
            if (Area[j] < Area[j+1]) {
                IRIT_SWAP(int, Indexes[j], Indexes[j+1]);
                IRIT_SWAP(IrtRType, Area[j], Area[j+1]);
            }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan over all visibility map pixels, for each uv values, contains xyz    M
*   values is compared to the triangles in the z-buffer in coordinate xy.    M
*   coordinate can be visible, hidden(mapped) or unmapped.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN,OUT, pointer to the visibility map.                         M
*   Buff:     IN, OUT, the zbuffer.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMScan, Visible, UV, Scan                                           M
*****************************************************************************/
void IRndrVMScan(IRndrVMStruct *VisMap, IRndrZBufferStruct *Buff)
{
    int u, v;

    if (!VisMap || !Buff) {
        return;
    }

    for (v = 0; v < VisMap -> SizeV; ++v) {
        for (u = 0; u < VisMap -> SizeU; ++u) {
            IRndrUVListStruct 
                *Pixel = &(VisMap -> UVMap[v][u]);

            /* Empty pixels are skipped and their value is left empty. */
            if ((Pixel -> Value == IRNDR_VISMAP_FILL_EMPTY) ||
                /* Backfaced triangles or invalid triangles are not seen.   */
                /* Therefore, their value is left mapped.                   */
                (Pixel -> BackFaced) ||
                (Pixel -> Validity != IRNDR_VISMAP_VALID_OK)) 
                continue;
            else {
/* This is old visibility map code. It was replaced by another algorithm.   */
#ifdef VM_USE_OLD_CODE 
                int x, y;
                IrtRType z, ZBufZ;
                x = IRIT_REAL_TO_INT(
                    Pixel -> Coord[RNDR_X_AXIS] / VisMap -> SuperSize);

                y = IRIT_REAL_TO_INT(
                    Pixel -> Coord[RNDR_Y_AXIS] / VisMap -> SuperSize);

                z = Pixel -> Coord[RNDR_Z_AXIS];

                RNDR_MAXM(x, 0);
                RNDR_MAXM(y, 0);
                RNDR_MINM(x, VisMap -> TargetSizeU - 1);
                RNDR_MINM(y, VisMap -> TargetSizeV - 1);

                ZBufferGetLineDepth(Buff, x, x, y, &ZBufZ);
                /* Check if UV pixel is in front of Z-buffer Z value. */

                Pixel -> Value =
                    (z >= ZBufZ - RNDR_ZBUF_SAME_EPS) ?
                    IRNDR_VISMAP_FILL_VISIBLE : IRNDR_VISMAP_FILL_MAPPED;
#else
                /* Check if the point is covered by any triangle. */
                int i, j, 
                    SumCovered = 0,
                    Total = 0;
                IrtPtType Coord;
                IrtRType x, y;

                x = Pixel -> Coord[RNDR_X_AXIS] - VisMap -> SuperSize/2;
                y = Pixel -> Coord[RNDR_Y_AXIS] - VisMap -> SuperSize/2;
                Coord[RNDR_Z_AXIS] = Pixel -> Coord[RNDR_Z_AXIS];
                for (i = 0; i <= VisMap -> SuperSize - 1; i++) {
                    Coord[RNDR_Y_AXIS] = y + i;
                    if (!RNDR_IN(Coord[RNDR_Y_AXIS], 0, 
                        VisMap -> SizeV - IRIT_EPS))
                        continue;
                    for (j = 0; j <= VisMap -> SuperSize - 1; j++) {
                        Coord[RNDR_X_AXIS] = x + j;
                        if (!RNDR_IN(Coord[RNDR_X_AXIS], 0, 
                            VisMap -> SizeU - IRIT_EPS))
                            continue;
                        if (VisMapIsPointCovered(VisMap, Coord, 
                            Pixel -> Triangle, Buff))
                            SumCovered++;
                        Total++;
                    }
                }
                Pixel -> Value = (SumCovered > Total / 2) ? 
                    IRNDR_VISMAP_FILL_MAPPED : IRNDR_VISMAP_FILL_VISIBLE;
#endif /* VM_USE_OLD_CODE */
            }
        }
    }
    VisMapDilation(VisMap);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Manually adds a single pixel to visibility map.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to visibility map.                           M
*   u:         IN, the column number.                                        M
*   v:         IN, the line number.                                          M
*   xyzVals:   IN, the pixel's view coordinates.                             M
*   Validity:  IN, validity of pixel.                                        M
*   BackFaced: IN, if back face culling is on and pixel belongs to back      M
*                  facing triangle.                                          M
*   Triangle:  IN, The triangle which created this UV point.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMPutPixel, put pixel                                               M
*****************************************************************************/
void IRndrVMPutPixel(IRndrVMStruct *VisMap,
		     int u,
		     int v,
		     IrtPtType xyzVals,
		     IRndrVisibleValidityType Validity,
		     int BackFaced,
                     IPPolygonStruct *Triangle)
{
    IRndrUVListStruct *Vertex;

    if (!VisMap)
        return;

    assert(RNDR_IN(u, 0, VisMap -> SizeU - 1) &&
           RNDR_IN(v, 0, VisMap -> SizeV - 1));

    Vertex = &VisMap -> UVMap[v][u];

    /*   The conditions in this section decide when to skip the current    */
    /* pixel in favour of previous pixel. When the previous pixel is back  */
    /* faced or degenreated, it's always in last priority and the new      */
    /* pixel should be accepted. Therefore, we don't need to check the     */
    /* conditions in this section.                                         */
    /*   If the previous pixel is back faced and the new is degenerated    */
    /* then we do get in because degenerated is in less priority than      */
    /*  backfaced.							   */
    if (!(Vertex -> BackFaced &&
	  Validity != IRNDR_VISMAP_VALID_DEGEN) &&
	(Vertex -> Validity != IRNDR_VISMAP_VALID_DEGEN)) {
        /*   If the triangle isn't valid and the pixel has already been    */
        /* claimed by other valid triangle (value isn't empty and validity */
        /* is ok) we leave the pixel as belonging to the valid triangle.   */
        if ((Validity != IRNDR_VISMAP_VALID_OK) &&
            (Vertex -> Value != IRNDR_VISMAP_FILL_EMPTY) && 
            (Vertex -> Validity == IRNDR_VISMAP_VALID_OK))
             return;

        /* If we have a closer vertex mapped already, we do nothing.       */
        if ((Vertex -> Value != IRNDR_VISMAP_FILL_EMPTY) &&
            (Vertex -> Coord[RNDR_Z_AXIS] > xyzVals[RNDR_Z_AXIS])) 
             return;
    }

    /* Put mapped vertex. */
    IRIT_PT_COPY(Vertex -> Coord, xyzVals);
    Vertex -> Validity = Validity;
    Vertex -> BackFaced = BackFaced;
    Vertex -> Value = IRNDR_VISMAP_FILL_MAPPED;
    Vertex -> Triangle = Triangle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieves visibility information of a specific line in UV space.         M
*   The line should be allocated by the caller.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:       IN, OUT, pointer to visibility map.                        M
*   u0:           IN, minimal U coordinate.                                  M
*   u1:           IN, maximal U coordinate.                                  M
*   v:            IN, line V number.                                         M
*   FilterCoeff:  IN, the filter to use in the super sampling. If NULL, uses M
*                 the same weight for all samples.                           M
*   Result:       OUT, the visibility values of the line. When super samplingM
*                 is disabled 0 For not visible and 1 for visible. When superM
*                 sampling is enable the values are in [0,1] and reflact the M
*                 amount of visible cell out of the total valid cells        M
*                 (invalid cells are ignored).                               M
*                 If empty, returns negative number.                         M
*                 If Validity isn't IRNDR_VISMAP_VALID_OK, Result is         M
*                 undefined.                                                 M
*   Validity:     The validity of the pixel.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMGetLine, Visible, access                                          M
*****************************************************************************/
int IRndrVMGetLine(IRndrVMStruct *VisMap,
		   int u0,
		   int u1,
		   int v,
		   IrtRType **FilterCoeff,
		   IrtRType *Result,
                   IRndrVisibleValidityType *Validity)
{
    int u, SizeU, SizeV;

    if (!VisMap || !Result || !Validity) {
        return FALSE;
    }

    assert(u0 >= 0 && u0 <= u1 && u1 < VisMap -> TargetSizeU &&
            v >= 0 && v < VisMap -> TargetSizeV);

    if (VisMap -> SuperSize == 1) {
        /* No filtering. */
        for (u = u0; u <= u1; ++u, ++Result) {
            switch(VisMap -> UVMap[v][u].Value) {
                case IRNDR_VISMAP_FILL_EMPTY:
                    *Result = -1;
                    break;
                case IRNDR_VISMAP_FILL_MAPPED:
                    *Result = 0;
                    break;
                case IRNDR_VISMAP_FILL_VISIBLE:
                    *Result = 1;
                    break;
	        default:
		    assert(0);
                    break;
            };
            *Validity = VisMap -> UVMap[v][u].Validity;
        }
    }
    else {
        /*   Compute average of super-sampled points. Normalizing it to the */
        /* weight of only the mapped samples.                               */
        /*   When considering invalid point, its enough that one point is   */
        /* valid in order to ignore all the other invalid points. If all    */
        /* the points are invalid (or empty), the validity will the type of */
        /* validity that appear the most times.                             */
        int SuperSize = VisMap -> SuperSize;

        for (u = u0; u <= u1; ++u, ++Result, ++Validity) {
            IrtRType
	        TotalWeight = 0.0;
            int ValidityCounts[IRNDR_VISMAP_VALID_COUNT] = {0};

            *Result = 0.0;
            *Validity = IRNDR_VISMAP_VALID_OK;
            for (SizeV = 0; SizeV < SuperSize; ++SizeV) {
                for (SizeU = 0; SizeU < SuperSize; ++SizeU) {
                    IRndrUVListStruct
		        *Pixel =  &VisMap -> 
                          UVMap[v * SuperSize + SizeV][u * SuperSize + SizeU];
                    IrtRType 
                        AddToResult,
                        Filter = (FilterCoeff == NULL) ? 1
                                                  : FilterCoeff[SizeV][SizeU];

                    switch (Pixel -> Value) {
                        case IRNDR_VISMAP_FILL_EMPTY:
                            continue;
                        case IRNDR_VISMAP_FILL_MAPPED:
                            AddToResult = 0;
                            ValidityCounts[Pixel -> Validity]++;
                            break;
                        case IRNDR_VISMAP_FILL_VISIBLE:
                            AddToResult = 1;
                            break;
		        default:
			    AddToResult = -IRIT_INFNTY,
			    assert(0);
			    break;
                    };
                    TotalWeight += Filter;
                    *Result += AddToResult * Filter;
                }
            }
            if (TotalWeight != 0)
                *Result /= TotalWeight;
            else if (ValidityCounts[IRNDR_VISMAP_VALID_OK] > 0)
	        *Result = 0;
            else {
                IRndrVisibleValidityType i;
                int Count = 0;

                for (i = IRNDR_VISMAP_VALID_OK+1; 
                     i <= IRNDR_VISMAP_VALID_COUNT - 1;
                     i++) {
                    if (ValidityCounts[i] > Count) {
                        Count = ValidityCounts[i];
                        *Validity = i;
                    }                       
                }
                *Result = -1; /* Return negative number if no invalid value */
                              /* was found which mean empty. Otherwise,     */
                              /* result should be ignored.                  */
            }
        }
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the domain of the given object (Object which aren't polygons are     M
* ignored.                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:                   IN - The object to get its domain. Objects which M
*                                aren't polygons are ignored.                M
*   UMin, UMax, VMin, VMax: OUT - The domain of the object.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: FALSE if no uv value was found.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMGetObjDomain                                                      M
*****************************************************************************/
int IRndrVMGetObjDomain(IPObjectStruct *PObj, 
                        IrtRType *UMin, 
                        IrtRType *UMax, 
                        IrtRType *VMin, 
                        IrtRType *VMax)
{
    IPPolygonStruct *Pl;
    IPVertexStruct *V;
    int Found = FALSE;

    if (!IP_IS_POLY_OBJ(PObj))
        return FALSE;

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
            float *Uv;

            Uv = AttrGetUVAttrib(V -> Attr, VIS_MAP_UV_VALUES);
            if (Uv != NULL) {
                if (!Found) {
                    Found = TRUE;
                    *UMin = Uv[0];
                    *UMax = Uv[0];
                    *VMin = Uv[1];
                    *VMax = Uv[1];
                }
                *UMin = IRIT_MIN(*UMin, Uv[0]);
                *UMax = IRIT_MAX(*UMax, Uv[0]);
                *VMin = IRIT_MIN(*VMin, Uv[1]);
                *VMax = IRIT_MAX(*VMax, Uv[1]);
            }
        }
    }
    return Found;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If there is more than one object in PObj (using PNext) arrange the UV    M
* values domains to a one continuous non overlapping domain.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:                           The geometric object.                    M
*   MapWidth, MapHeight:            The dimensions of the visibility map.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVMPrepareUVValuesOfGeoObj                                           M
*****************************************************************************/
void IRndrVMPrepareUVValuesOfGeoObj(IPObjectStruct *PObj, 
                                    int MapWidth, 
                                    int MapHeight)
{
    /* Variables for all sections. */
    int i, *Indexes, Length;
    const int
        MARGIN = 2;

    /* Variables for all sections. */
    IrtRType *UMin, *UMax, *VMin, *VMax, *SizeU, *SizeV, 
        *Area, SumArea, MinWidth, MaxWidth,
        /* Variables for section 2 (some for 3 as well). */        
        BestRatioDistance, Width,
        BestMaxWidth, BestMaxHeight, ScaleWidth, ScaleHeight,
        /* Variables for section 3. */
        SumPrevWidth, SumPrevHeight, MaxLineHeight;
    IPObjectStruct *PObj2, **Objects;

    /* 1. Calculates values for each object. */
    Length = IPObjListLen(PObj);
    Indexes = IritMalloc(Length * sizeof(int));
    UMin = IritMalloc(Length * sizeof(IrtRType));
    UMax = IritMalloc(Length * sizeof(IrtRType));
    VMin = IritMalloc(Length * sizeof(IrtRType));
    VMax = IritMalloc(Length * sizeof(IrtRType));
    SizeU = IritMalloc(Length * sizeof(IrtRType));
    SizeV = IritMalloc(Length * sizeof(IrtRType));
    Area = IritMalloc(Length * sizeof(IrtRType));
    Objects = IritMalloc(Length * sizeof(IPObjectStruct*));
    MinWidth = IRIT_INFNTY;
    MaxWidth = -IRIT_INFNTY;
    SumArea = 0;

    for (PObj2 = PObj, i = 0; PObj2 != NULL; PObj2 = PObj2 -> Pnext, i++){
        IrtRType Scale;
        GMBBBboxStruct *BBox;

        Indexes[i] = i;
        if (!IRndrVMGetObjDomain(PObj2, &UMin[i], &UMax[i], &VMin[i], 
				 &VMax[i])) {
	    Length--;
	    i--;
            if (Length == 0)
                return;
	    continue;
        }
        /* The area is used as a measure of how much of the uv space should */
        /* this object take. We add also the bounding box diagonal for      */
        /* cases such as rings when the actual area is small though it take */
        /* much more area if you consider the hole as well.                 */
        BBox = GMBBComputeBboxObject(PObj2);
        Area[i] = GMPolyObjectArea(PObj2) +
                                IRIT_PT_PT_DIST_SQR(BBox -> Max, BBox -> Min);
        SumArea += Area[i];
        Scale = sqrt(Area[i]/((UMax[i] - UMin[i])*(VMax[i] - VMin[i])));
        /* Here we give the uv domain the same area as the euclidean area.  */
        SizeU[i] = (UMax[i] - UMin[i])*Scale;
        SizeV[i] = (VMax[i] - VMin[i])*Scale;
        Objects[i] = PObj2;
        MinWidth = IRIT_MIN(MinWidth, SizeU[i]);
        MaxWidth = IRIT_MAX(MaxWidth, SizeU[i]);
    }
    VisMapPrepareUVValuesOfGeoObjSort(Indexes, Area, Length);

    /* 2. Finds the best way to arrange the objects. */

    /* We look for the best width to spread the uv maps of the objects      */
    /* while keeping the aspect ratio.                                      */
    Width = IRIT_MAX(sqrt((SumArea*MapWidth)/MapHeight), MaxWidth);
    BestRatioDistance = IRIT_INFNTY;   /* The distance from 1 of the best   */
                                       /* ratio between width to height.    */
    BestMaxHeight = 0;
    BestMaxWidth = 0;
    while(TRUE){
        IrtRType 
            CurMaxWidth = 0,    /* Maximum width of the current arrangment. */
                                 /* Each while loop checks one arrangement. */
            LineMaxHeight = 0,       /* Maximum height in the current line. */
            CurWidth = 0, 
            CurHeight = 0;

        for (i = 0; i <= Length - 1; i++) {
            if (CurWidth + SizeU[Indexes[i]] > Width) {
                CurMaxWidth = IRIT_MAX(CurMaxWidth, CurWidth);
                CurHeight += LineMaxHeight;
                LineMaxHeight = 0;
                CurWidth = 0;
            }
            LineMaxHeight = IRIT_MAX(LineMaxHeight, SizeV[Indexes[i]]);
            CurWidth += SizeU[Indexes[i]];
        }
        CurHeight += LineMaxHeight;
        CurMaxWidth = IRIT_MAX(CurMaxWidth, CurWidth);
        if (CurMaxWidth == 0)
            CurMaxWidth = CurWidth;
        if (BestRatioDistance - fabs(CurHeight/CurMaxWidth) > IRIT_EPS) {
            BestRatioDistance = fabs(CurHeight/CurMaxWidth);
            BestMaxHeight = CurHeight;
            BestMaxWidth = CurMaxWidth;
        }
        else if (BestRatioDistance - fabs(CurHeight/CurMaxWidth) < IRIT_EPS)
            /* Reached best width. */
            break;

        /* Looking for better spread. */
        if (CurMaxWidth/CurHeight > (1.0*MapWidth)/MapHeight) {
            Width -= MinWidth;
            if (Width < MaxWidth)
                /* Can't get Width any smaller. */
                break;
        }
        else
            Width += MinWidth;                
    };

    /* 3. Changes the UV values according to previous calculations. */

    ScaleWidth = MapWidth / BestMaxWidth;
    ScaleHeight = MapHeight / BestMaxHeight;

    /* SumPrevHeight - The real sum of the previous lines height.           */
    /* SumPrevWidth - The real sum of the previous objects width.           */
    /* MaxLineHeight - The real height of the current line. Calculated by   */
    /*                 the maximum real height of all the objects in the    */
    /*                 current line.                                        */
    /* CurHeightPixels - The integer height of the current section.         */
    /* CurWidthPixels - The integer width of the current section.           */
    SumPrevWidth = 0;
    SumPrevHeight = 0;
    MaxLineHeight = 0;
    for (i = 0; i <= Length - 1; i++) {
        int CurWidthPixels = 
                IRIT_REAL_TO_INT(SumPrevWidth + SizeU[Indexes[i]]*ScaleWidth) -
                IRIT_REAL_TO_INT(SumPrevWidth),
            CurHeightPixels = 
                IRIT_REAL_TO_INT(SumPrevHeight + SizeV[Indexes[i]]*ScaleHeight) -
                IRIT_REAL_TO_INT(SumPrevHeight);
        IrtRType Scale[2], Trans[2];

        /* Next line. */
        if (SumPrevWidth +  SizeU[Indexes[i]]*ScaleWidth - MapWidth > IRIT_EPS) {
            SumPrevHeight += MaxLineHeight;
            CurWidthPixels = IRIT_REAL_TO_INT(SizeU[Indexes[i]]*ScaleWidth);
            CurHeightPixels =
                IRIT_REAL_TO_INT(SumPrevHeight + SizeV[Indexes[i]]*ScaleHeight) 
                - IRIT_REAL_TO_INT(SumPrevHeight);
            SumPrevWidth = 0;
            MaxLineHeight = 0;
        }
        /* I avoid margin at the end and begin of the image because it will */
        /* probably be scaled out by anyone using those uv coordinates.     */
        /* This will spread piece y on [x+MARGIN, x+y-MARGIN].              */
        Scale[0] = (CurWidthPixels - (SumPrevWidth == 0 ? 1 : 2)*MARGIN)/
            (UMax[Indexes[i]] - UMin[Indexes[i]]);
        Scale[1] = (CurHeightPixels - (SumPrevHeight == 0 ? 1 : 2))
            /(VMax[Indexes[i]] - VMin[Indexes[i]]);
        /* Translate to origin and then translate to the begining of the    */
        /* calculated section (SumPrevWidth).                               */
        Trans[0] = -UMin[Indexes[i]]*Scale[0] + 
            (IRIT_REAL_TO_INT(SumPrevWidth) + (SumPrevWidth == 0 ? 0 : MARGIN) 
            + IRIT_UEPS);
        Trans[1] = -VMin[Indexes[i]]*Scale[1] +
            (IRIT_REAL_TO_INT(SumPrevHeight) + (SumPrevHeight == 0 ? 0 : MARGIN)
            + IRIT_UEPS);
        /* Doing scaling and then translation. */
        GMAffineTransUVVals(Objects[Indexes[i]], Scale, Trans);

        /* Updating data as result of new section. */
        MaxLineHeight = IRIT_MAX(MaxLineHeight, SizeV[Indexes[i]]*ScaleHeight);
        SumPrevWidth += SizeU[Indexes[i]]*ScaleWidth;
    }

    IritFree(Indexes);
    IritFree(UMin);
    IritFree(UMax);
    IritFree(VMin);
    IritFree(VMax);
    IritFree(SizeU);
    IritFree(SizeV);
    IritFree(Area);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Sets a uv triangle using DoVisMapCalcs field in Object struct             M
*  turned on while calling TriangleSet method.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   RendTri:  OUT, pointer to the Triangle object.                           M
*   Triangle: IN, pointer to Irit polygon object.                            M
*   Obj:      IN, pointer to Object which contains a Triangle and stores     M
*             various characteristics common to every polygon in the object. M
*   Scene:    IN, pointer to the scene context.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE when successful.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapTriangleSet                                                        M
*****************************************************************************/
static int VisMapTriangleSet(IRndrTriangleStruct *RendTri,
			     IPPolygonStruct *Triangle,
			     IRndrObjectStruct *Obj,
			     IRndrSceneStruct *Scene)
{
    int retval;

    if (!RendTri || !Triangle || !Obj || !Scene) {
        return FALSE;
    }

    Obj -> DoVisMapCalcs = TRUE;
    retval = TriangleSet(RendTri, Triangle, Obj, Scene);
    Obj -> DoVisMapCalcs = FALSE;
    return retval;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Switch Polygon coordinates so it could be scanned with the z buffer      M
* scan convention into the visibility map. Simply set the coord field to     M
* contain the uv values while the x and y values are backuped in attributes  M
* VIS_MAP_X_ATTRIB and VIS_MAP_Y_ATTRIB.                                     M
* At the same time, change the uv space to be [0..size of vismap - 1] (both  M
* in coord and in the uv attribute).                                         M
* If Reverse is TRUE, do the opposite operation. The coord values set to     M
* the x and y values which are restored from the attributes VIS_MAP_X_ATTRIB M
* and VIS_MAP_Y_ATTRIB (the uv values which were in coord are dimissed).     M
* If Reverse is TRUE, no change of uv space is done.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, Pointer to the visibility map.                             M
*   Scene:    IN, a pointer to the scene struct.                             M
*   Obj:      IN, a pointer to the current rendered object.                  M
*   Triangle: IN, OUT, triangle polygon object.                              M
*   Reverse:  IN, Whether to switch uv into xy or do the reverse operation.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE if successful.                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapSwitchTriangleSpaces, space, switch, triangle, scan convention.    M
*****************************************************************************/
static int VisMapSwitchTriangleSpaces(IRndrVMStruct *VisMap,
				      IRndrSceneStruct *Scene,
				      IRndrObjectStruct *Obj,
				      IPPolygonStruct *Triangle,
                                      int Reverse)
{
    IPVertexStruct *v;

    if (!VisMap || !Scene || !Obj || !Triangle) {
        return FALSE;
    }
    if (!Reverse) {
        for (v = Triangle -> PVertex; v != NULL; v = v -> Pnext) {
            float *Uvvals;

            /* Get transformed vertices. */
            if (Obj -> Transformed != TRUE) {
                VertexTransform(v, &Scene -> Matrices, Obj, v -> Coord);
            }

            /* Backup x and y values. */
            AttrSetRealAttrib(&v -> Attr, VIS_MAP_X_ATTRIB, 
			      v -> Coord[RNDR_X_AXIS]);
            AttrSetRealAttrib(&v -> Attr, VIS_MAP_Y_ATTRIB, 
			      v -> Coord[RNDR_Y_AXIS]);

            /* X coordinate is U value, relative to min and max U values.   */
            /* Same for Y and V.                                            */
            Uvvals = AttrGetUVAttrib(v -> Attr, VIS_MAP_UV_VALUES);
            if (Uvvals == NULL) {
                return FALSE;
            }

            /* The addition (- 0.5 - IRIT_EPS) will make sure that after    */
            /* rounding the values will be inside [0, Max?].                */
            v -> Coord[RNDR_X_AXIS] = 
                (Uvvals[0] - VisMap -> MinU) /
                (VisMap -> MaxU - VisMap -> MinU) * 
                (VisMap -> SizeU - 0.5 - IRIT_EPS);
            v -> Coord[RNDR_Y_AXIS] = 
                (Uvvals[1] - VisMap -> MinV) /
                (VisMap -> MaxV - VisMap -> MinV) * 
                (VisMap -> SizeV - 0.5 - IRIT_EPS);

            Uvvals[0] = (float) v -> Coord[RNDR_X_AXIS];
            Uvvals[1] = (float) v -> Coord[RNDR_Y_AXIS];
        }
    }
    else {
        for (v = Triangle -> PVertex; v != NULL; v = v -> Pnext) {
            v -> Coord[RNDR_X_AXIS] = AttrGetRealAttrib(v -> Attr,
							VIS_MAP_X_ATTRIB);
            v -> Coord[RNDR_Y_AXIS] = AttrGetRealAttrib(v -> Attr,
							VIS_MAP_Y_ATTRIB);
            if ((v -> Coord[RNDR_X_AXIS] == IP_ATTR_BAD_REAL) ||
                (v -> Coord[RNDR_Y_AXIS] == IP_ATTR_BAD_REAL))
                return FALSE;
            AttrFreeOneAttribute(&v -> Attr, VIS_MAP_X_ATTRIB);
            AttrFreeOneAttribute(&v -> Attr, VIS_MAP_Y_ATTRIB);
        }
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check the validity of the triangle and set Validity accordingly.         M
*   Assumes that XY values and UVZ values were switched in the triangle.     M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, pointer to the visibility map.                             M
*   Triangle: IN, triangle polygon object.                                   M
*   Validity: OUT, validity of triangle.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE if successful (All the given parameters weren't NULL, and the  M
*        triangle has UV values).                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapCheckValidity, error                                               M
*****************************************************************************/
static int VisMapCheckValidity(IRndrVMStruct *VisMap,
                               IPPolygonStruct *Triangle,
                               IRndrVisibleValidityType *Validity)
{
    int i;
    IrtRType uv[3][2];
    IrtPtType Pt[3], OrigPt[3], uvxVec[2], uvyVec[2], xyzOrigVec[2];
    IPVertexStruct *v;

    if (!VisMap || !Triangle || !Validity) {
        return FALSE;
    }

    *Validity = IRNDR_VISMAP_VALID_OK;

    if (AttrGetIntAttrib(Triangle -> Attr, IP_ATTRIB_DEGEN_POLY) != 
        IP_ATTR_BAD_INT){     
        *Validity = IRNDR_VISMAP_VALID_DEGEN;
        return TRUE;
    }

    for (v = Triangle -> PVertex, i = 0;
            v != NULL; 
            v = v -> Pnext, ++i) {


        Pt[i][RNDR_X_AXIS] = AttrGetRealAttrib(v -> Attr, VIS_MAP_X_ATTRIB);
        Pt[i][RNDR_Y_AXIS] = AttrGetRealAttrib(v -> Attr, VIS_MAP_Y_ATTRIB);
        if ((Pt[i][RNDR_X_AXIS] == IP_ATTR_BAD_REAL) ||
            (Pt[i][RNDR_Y_AXIS] == IP_ATTR_BAD_REAL))
            return FALSE;
        Pt[i][RNDR_Z_AXIS] = v -> Coord[RNDR_Z_AXIS];        
        IRIT_PT_COPY(OrigPt[i], Pt[i]);
        VisMapRestoreVector(VisMap, OrigPt[i]);
        uv[i][0] = v -> Coord[RNDR_X_AXIS];
        uv[i][1] = v -> Coord[RNDR_Y_AXIS];
    }

    /* Making vectors of triangle's edges                                  */
    /* vector 0: from vertex 2 to vertex 0                                 */
    /* vector 1: from vertex 2 to vertex 1                                 */
    /* abcVec means vectors of triangle in space of a b and c              */
    /* xyzOrigVec means triangle in x,y,z space before transofrming to     */
    /* window space.                                                       */
    for (i = 0; i < 2; i++) {
        xyzOrigVec[i][0] = OrigPt[i][RNDR_X_AXIS] - OrigPt[2][RNDR_X_AXIS];
        uvxVec[i][2] = Pt[i][RNDR_X_AXIS] - Pt[2][RNDR_X_AXIS];

        xyzOrigVec[i][1] = OrigPt[i][RNDR_Y_AXIS] - OrigPt[2][RNDR_Y_AXIS];
        uvyVec[i][2] = Pt[i][RNDR_Y_AXIS] - Pt[2][RNDR_Y_AXIS];

        uvxVec[i][0] = uvyVec[i][0] = uv[i][0] - uv[2][0];
        uvxVec[i][1] = uvyVec[i][1] = uv[i][1] - uv[2][1];
        xyzOrigVec[i][2] = Pt[i][RNDR_Z_AXIS] - Pt[2][RNDR_Z_AXIS];        
    }
    
    /* Check AR of triangles. */
    if (VisMapIsPoorAR(VisMap, xyzOrigVec[0], xyzOrigVec[1]) ||
	VisMapIsPoorAR(VisMap, uvxVec[0], uvxVec[1]) ||
	VisMapIsPoorAR(VisMap, uvyVec[0], uvyVec[1]) ) {
        
        *Validity = IRNDR_VISMAP_VALID_POOR_AR;
    }
    else {
        IrtPtType uvxNorm, uvyNorm, xyzOrigNorm;

        IRIT_CROSS_PROD(xyzOrigNorm, xyzOrigVec[0], xyzOrigVec[1]);
        IRIT_CROSS_PROD(uvxNorm, uvxVec[0], uvxVec[1]);
        IRIT_CROSS_PROD(uvyNorm, uvyVec[0], uvyVec[1]);

        /* Check if triangle is close to tangent view point. */
        if (VisMapIsTangentZAxis(VisMap, xyzOrigNorm) ||
	    VisMapIsTangentZAxis(VisMap, uvxNorm) ||
	    VisMapIsTangentZAxis(VisMap, uvyNorm)) 
            *Validity = IRNDR_VISMAP_VALID_TANGENT;
    }
    return TRUE;
}

/* This is old visibility map code. It was replaced by another algorithm. */
#ifdef VM_USE_OLD_CODE
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimating z error value in visibility map pixel. to make up on          M
*   discretization errors.                                                   M
*   Estimating is made by the partial differentiating of z(x,y) ,x(u,v),     M
*   and y(u,v) as follow:                                                    M
*       Dz = (dz/dx)Dx + (dz/dy)Dy                                           M
*       Dy = (dy/du)Du + (dy/dv)Dv                                           M
*       Dx = (dx/du)Du + (dx/dv)Dv                                           M
*       Du = Dv = 1                                                          M
*   Where d is partial differentiation, and D means delta value.             M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, pointer to the visibility map.                             M
*   Triangle: IN, triangle polygon object.                                   M
*   Validity: OUT, validity of triangle.                                     M
*   dz:       OUT, z error estimation.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE if successful.                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapTriangleDZ, error, estimation.                                     M
*****************************************************************************/
static int VisMapTriangleDZ(IRndrVMStruct *VisMap,
			    IPPolygonStruct *Triangle,
			    IRndrVisibleValidityType *Validity,
			    IrtRType *dz)
{
    int i;
    IrtRType uv[3][2];
    IrtPtType Pt[3], OrigPt[3], uvxVec[2], uvyVec[2], xyzVec[2], xyzOrigVec[2];
    IPVertexStruct *v;

    if (!VisMap || !Triangle) {
        return FALSE;
    }

    *Validity = IRNDR_VISMAP_VALID_OK;

    for (v = Triangle -> PVertex, i = 0;
            v != NULL; 
            v = v -> Pnext, ++i) {

        float 
            *Uvvals = AttrGetUVAttrib(v -> Attr, VIS_MAP_UV_VALUES);

        if (!Uvvals) {
            return FALSE;
        }

        Pt[i][RNDR_X_AXIS] = Uvvals[0];
        Pt[i][RNDR_Y_AXIS] = Uvvals[1];
        Pt[i][RNDR_Z_AXIS] = v -> Coord[RNDR_Z_AXIS];
        IRIT_PT_COPY(OrigPt[i], Pt[i]);
        VisMapRestoreVector(VisMap, OrigPt[i]);
        uv[i][0] = v -> Coord[RNDR_X_AXIS];
        uv[i][1] = v -> Coord[RNDR_Y_AXIS];
    }

    /* Making vectors of triangle's edges                                  */
    /* vector 0: from vertex 2 to vertex 0                                 */
    /* vector 1: from vertex 2 to vertex 1                                 */
    /* abcVec means vectors of triangle in space of a b and c              */
    /* abcOrigVec means the same but with original values                  */
    for (i = 0; i < 2; i++) {
        xyzOrigVec[i][0] = OrigPt[i][RNDR_X_AXIS] - OrigPt[2][RNDR_X_AXIS];
        uvxVec[i][2] = xyzVec[i][0] = 
            Pt[i][RNDR_X_AXIS] - Pt[2][RNDR_X_AXIS];

        xyzOrigVec[i][1] = OrigPt[i][RNDR_Y_AXIS] - OrigPt[2][RNDR_Y_AXIS];
        uvyVec[i][2] = xyzVec[i][1] = 
            Pt[i][RNDR_Y_AXIS] - Pt[2][RNDR_Y_AXIS];

        uvxVec[i][0] = uvyVec[i][0] = uv[i][0] - uv[2][0];
        uvxVec[i][1] = uvyVec[i][1] = uv[i][1] - uv[2][1];
        xyzVec[i][2] = xyzOrigVec[i][2] =
           Pt[i][RNDR_Z_AXIS] - Pt[2][RNDR_Z_AXIS];        
    }
    
    /* Check AR of triangles. */
    if (VisMapIsPoorAR(VisMap, xyzOrigVec[0], xyzOrigVec[1]) ||
            VisMapIsPoorAR(VisMap, uvxVec[0], uvxVec[1]) ||
            VisMapIsPoorAR(VisMap, uvyVec[0], uvyVec[1]) ) {
        
        *Validity = IRNDR_VISMAP_VALID_POOR_AR;
        *dz = -IRIT_MAX_INT;
    }
    else {
        IrtRType dzdx, dzdy, dxdu, dxdv, dydu, dydv;
        IrtPtType uvxNorm, uvyNorm, xyzNorm, xyzOrigNorm;

        IRIT_CROSS_PROD(xyzOrigNorm, xyzOrigVec[0], xyzOrigVec[1]);

        IRIT_CROSS_PROD(uvxNorm, uvxVec[0], uvxVec[1]);
        IRIT_CROSS_PROD(uvyNorm, uvyVec[0], uvyVec[1]);
        IRIT_CROSS_PROD(xyzNorm, xyzVec[0], xyzVec[1]);

        /* Check if triangle is close to tangent view point. */
        if (VisMapIsTangentZAxis(VisMap, xyzOrigNorm) ||
                VisMapIsTangentZAxis(VisMap, uvxNorm) ||
                VisMapIsTangentZAxis(VisMap, uvyNorm)) {

            *Validity = IRNDR_VISMAP_VALID_TANGENT;
        }

        dxdu = IRIT_ABS(uvxNorm[0] / uvxNorm[2]);
        dxdv = IRIT_ABS(uvxNorm[1] / uvxNorm[2]);
       
        dydu = IRIT_ABS(uvyNorm[0] / uvyNorm[2]);
        dydv = IRIT_ABS(uvyNorm[1] / uvyNorm[2]);
        
        dzdx = IRIT_ABS(xyzNorm[0] / xyzNorm[2]);
        dzdy = IRIT_ABS(xyzNorm[1] / xyzNorm[2]);

        *dz = dzdx * (dxdu + dxdv) + dzdy * (dydu + dydv);
    }

    return TRUE;
}
#endif /* VM_USE_OLD_CODE */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Dilate the surrounding colors into the white pixels in the map.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VisMap:   IN, OUT, pointer to visibility map.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void VisMapDilation(IRndrVMStruct *VisMap) 
{
    int u, v, Times;
    IRIT_STATIC_DATA const int
        Picker[8][2] = { {  1,  0 },
                         { -1,  0 },
                         {  0,  1 },
                         {  0, -1 },
                         {  1,  1 },
                         { -1,  1 },
                         {  1, -1 },
                         { -1, -1 } };

    if (!VisMap) {
        return;
    }

    for (Times = 0; Times < VisMap -> DilationItarations; ++Times) {
        for (v = 0; v < VisMap -> SizeV; ++v) {
	    for (u = 0; u < VisMap -> SizeU; ++u) {
	        int k;
		IRndrUVListStruct
		    *Pixel = &VisMap -> UVMap[v][u];

		for (k = 0; 
		     (Pixel -> Value == IRNDR_VISMAP_FILL_EMPTY) && (k < 8); 
		     ++k) {
		    int i = IRIT_BOUND(u + Picker[k][0], 0, VisMap -> SizeU - 1),
		        j = IRIT_BOUND(v + Picker[k][1], 0, VisMap -> SizeV - 1);
		    IRndrUVListStruct
		        *Neighbour = &VisMap -> UVMap[j][i];

		    switch (Neighbour -> Value) {
                        case IRNDR_VISMAP_FILL_MAPPED:
			    Pixel -> Value = IRNDR_VISMAP_FILL_DILATE_MAPPED;
			    break;
                        case IRNDR_VISMAP_FILL_VISIBLE:
			    Pixel -> Value = IRNDR_VISMAP_FILL_DILATE_VISIBLE;
			    break;
		        default:
			    break;
		    }
		}
	    }
	}

	for (v = 0; v < VisMap -> SizeV; ++v) {
	    for (u = 0; u < VisMap -> SizeU; ++u) {
	        IRndrUVListStruct
		    *Pixel = &VisMap -> UVMap[v][u];

		switch (Pixel -> Value) {
                    case IRNDR_VISMAP_FILL_DILATE_MAPPED:
		        Pixel -> Value = IRNDR_VISMAP_FILL_MAPPED;
			break;
                    case IRNDR_VISMAP_FILL_DILATE_VISIBLE:
		        Pixel -> Value = IRNDR_VISMAP_FILL_VISIBLE;
			break;
		    default:
		        break;
		}
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Restore vector's world coordinate from view coordinate system.           M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, pointer to the visibility map.                             M
*   v:        IN, OUT: point object to be restore.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapRestoreVector, restore, vector, point.                             M
*****************************************************************************/
static void VisMapRestoreVector(IRndrVMStruct *VisMap, IrtPtType v)
{
    IRIT_STATIC_DATA IrtHmgnMatType Inv;
    IRndrSceneStruct *Scene;

    if (!VisMap || !VisMap -> Scene)
        return;

    Scene = VisMap -> Scene;

    /* Invert matrix only once, since we always have the same matrix. */
    if (!VisMap -> InvScreenMatValid) {
        MatInverseMatrix(Scene -> Matrices.ScreenMat, Inv);
        VisMap -> InvScreenMatValid = TRUE;
    }

    /* Undo screen translations */
    MatMultPtby4by4(v, v, Inv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Checks whether triangle has poor aspect ratio: the ratio between largest M
*   and smallest edge.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, pointer to the visibility map.                             M
*   v1:       IN, vector of one edge of triangle.                            M
*   v2:       IN, vector of second edge of triangle.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE if aspect ratio is poor.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapIsPoorAR, aspect ratio, triangle, AR.                              M
*****************************************************************************/
static int VisMapIsPoorAR(IRndrVMStruct *VisMap, IrtPtType v1, IrtPtType v2)
{
    int i;
    IrtPtType v3;
    IrtRType len[3],
        MaxL = 0,
        MinL = (IrtRType) IRIT_MAX_INT;

    if (!VisMap) {
        return TRUE;
    }
	
    if (VisMap -> CriticAR == 0) {
        return FALSE;
    }

    IRIT_VEC_SUB(v3, v1, v2);
    len[0] = IRIT_VEC_LENGTH(v1);
    len[1] = IRIT_VEC_LENGTH(v2);
    len[2] = IRIT_VEC_LENGTH(v3);

    for (i = 0; i < 3; ++i) {
        RNDR_MAXM(MaxL, len[i]);
        RNDR_MINM(MinL, len[i]);
    }

    return (MaxL / MinL > VisMap -> CriticAR);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Checks whether triangle is close to tangent view vector.                 M
*   uses RNDR_UV_CRITIC_TAN, as cos of angle between triangle normal and     M
*   z axis.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:   IN, pointer to the visibility map.                             M
*   Norm:     IN, vector normal of triangle.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE triangle is close to tangent view.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapIsTangentZAxis, tangent, triangle.                                 M
*****************************************************************************/
static int VisMapIsTangentZAxis(IRndrVMStruct *VisMap, IrtPtType Norm)
{
    IrtPtType
        z = {0.0, 0.0, 1.0};

    if (!VisMap) {
        return TRUE;
    }

    return (IRIT_ABS(IRIT_DOT_PROD(Norm, z) / IRIT_VEC_LENGTH(Norm)) < 
        VisMap -> CosTanAng);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Enabling scene fitting for current rendering by setting the scene's XY   M
* limits and VisMap's UV limits accordig to all of OList's vertices (The     M
* scene is stored in VisMap).                                                M
*   This function is recursive.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to the visibility map.                       M
*   OList:     IN, list of all object to fit in scene.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapFindLimits, scene fitting.                                         M
*****************************************************************************/
static void VisMapFindLimits(IRndrVMStruct *VisMap, IPObjectStruct *OList)
{
    IPPolygonStruct *Poly;
    IPVertexStruct *Vertex;
	
    if (!VisMap || !OList) {
        return;
    }

    for ( ; OList != NULL; OList = OList -> Pnext) {
        if (IP_IS_POLY_OBJ(OList) && IP_IS_POLYGON_OBJ(OList)) {
            for (Poly = OList -> U.Pl; Poly != NULL; Poly = Poly -> Pnext) {
                for (Vertex = Poly -> PVertex;
		                Vertex != NULL;
		                Vertex = Vertex -> Pnext) {

                    VisMapSetRefreshLimits(VisMap, Vertex);
                }
            }       
	}
	else if (IP_IS_OLST_OBJ(OList)) {
	    IPObjectStruct *PTmp;
	    int j = 0;

	    while ((PTmp = IPListObjectGet(OList, j++)) != NULL)
	        VisMapFindLimits(VisMap, PTmp);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adding to the scene the contribution of Vertex by updating the scene's   M
* XY limits values and VisMap's UV limits (The scene is stored in            M
* VisMap).                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:  IN, OUT, pointer to the visibility map.                         M
*   Vertex:  IN, vertex to update limits with.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void.                                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapSetRefreshLimits                                                   M
*****************************************************************************/
static void VisMapSetRefreshLimits(IRndrVMStruct *VisMap, 
                                   IPVertexStruct *Vertex)
{
    IrtRType Result[4];
    IRndrSceneStruct *Scene;
    float *uv;
	
    if (!VisMap || !VisMap -> Scene) {
        return;
    }
	
    Scene = VisMap -> Scene;

    uv =  AttrGetUVAttrib(Vertex -> Attr, VIS_MAP_UV_VALUES);
    if (uv) {
        if (VisMap -> MinU > uv[0])
            VisMap -> MinU = uv[0];
        if (VisMap -> MinV > uv[1])
            VisMap -> MinV = uv[1];
        if (VisMap -> MaxU < uv[0])
            VisMap -> MaxU = uv[0];
        if (VisMap -> MaxV < uv[1])
            VisMap -> MaxV = uv[1];
    }

    VertexTransform(Vertex, &Scene -> Matrices, NULL, Result);

    RNDR_MAXM(Scene -> XMax, Result[RNDR_X_AXIS]);
    RNDR_MAXM(Scene -> YMax, Result[RNDR_Y_AXIS]);
    RNDR_MINM(Scene -> XMin, Result[RNDR_X_AXIS]);
    RNDR_MINM(Scene -> YMin, Result[RNDR_Y_AXIS]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Setting scene fitting parameters after enabling.                         M
*   Uses the scene's XY limits in order to change the screen matrix so the   M
* entire scene will be in the scene's window and centered. That means that   M
* the entire scene's x and y coordinates will be in the range                M
* [0,scene -> sizex/y] (z coordinate isn't changed).                         M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMap:    IN, OUT, pointer to the visibility map.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisMapMakeFittingMatrices, scene fitting.                                M
*****************************************************************************/
static void VisMapMakeFittingMatrices(IRndrVMStruct *VisMap)
{
    IRndrSceneStruct *Scene;
    IrtRType Tx1, Ty1, Sx, Sy, Tx2, Ty2;
    IrtHmgnMatType Translate1, Scale, Translate2, *Mat;

    if (!VisMap || !VisMap -> Scene) {
        return;
    }

    Scene = VisMap -> Scene;

    Tx1 =  -0.5 * (Scene -> XMax + Scene -> XMin);
    Ty1 =  -0.5 * (Scene -> YMax + Scene -> YMin);
    /* The addition - 0.5 - IRIT_EPS will make sure that after rounding the */
    /* values will be inside [0, ?MAX].                                     */
    Sx = ((IrtRType) Scene -> SizeX - 0.5 - IRIT_EPS) /
                                               (Scene -> XMax - Scene -> XMin);
    Sy = ((IrtRType) Scene -> SizeY - 0.5 - IRIT_EPS) /
                                               (Scene -> YMax - Scene -> YMin);
    Tx2 = 0.5 * (Scene -> SizeX - 0.5 - IRIT_EPS);
    Ty2 = 0.5 * (Scene -> SizeY - 0.5 - IRIT_EPS);

    Mat = &Scene -> Matrices.ScreenMat;


    MatGenMatTrans(Tx1, Ty1, 0.0, Translate1);
    MatGenMatScale(Sx, Sy, 1.0, Scale);
    MatGenMatTrans(Tx2, Ty2, 0.0, Translate2);

    /* Applying changes on scene matrix. */
    MatMultTwo4by4(*Mat, *Mat, Translate1);
    MatMultTwo4by4(*Mat, *Mat, Scale);
    MatMultTwo4by4(*Mat, *Mat, Translate2);
}
