/******************************************************************************
* DDM_text.c - real geometric detailing (displacement Deformation maps).      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 2001.					      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "bool_lib.h"
#include "allocate.h"
#include "geom_lib.h"
#include "user_loc.h"

#define USER_DDM_NUM_U_BUCKETS	1000
#define USER_DDM_NORMAL_EPS	1e-8

typedef struct UserDDMUBucketStruct {
    struct UserDDMUBucketStruct *Pnext;
    struct IPPolygonStruct *Pl;
    IrtRType *UVMinMax;
} UserDDMUBucketStruct;

IRIT_STATIC_DATA int GlblLclUV, GlblSrfUClosed, GlblSrfVClosed, GlblShiftDirPower;
IRIT_STATIC_DATA CagdVType GlblShiftDir;
IRIT_STATIC_DATA CagdRType GlblU, GlblV, GlblUMin, GlblUSize, GlblDu, GlblDv;
IRIT_STATIC_DATA const CagdSrfStruct *GlblSrf;
IRIT_STATIC_DATA CagdSrfStruct *GlblDuSrf, *GlblDvSrf;
IRIT_STATIC_DATA IPObjectStruct *GlblPlSrf, *GlblRetObjs;
IRIT_STATIC_DATA UserDDMUBucketStruct **GlblUBuckets;

static void UserDDMUVW2XYZ(CagdRType UVW[3],
			   CagdRType UMin,
			   CagdRType UMax,
			   CagdRType VMin,
			   CagdRType VMax,
			   CagdRType Vuv[2],
			   CagdRType XYZ[3]);
static void UserOneDDMOverSrf(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void UserOneDDMOverPls(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static int UserOneVrtxDDMOverPls(IPVertexStruct *V);
static int UserOneVrtxDDMOverPlsAux(const UserDDMUBucketStruct *Buckets,
				    IPVertexStruct *V);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Maps the given DDM texture defined in Texture over the surface Srf,      M
* duplicating it (UDup x VDup) times over the parametric domain of Srf.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to derive a DDM mapping for.                            M
*   Texture: Geometry defining the single tile of the DDM texture.           M
*	     The geometry is assumed to span [0..1] in both x and y.	     M
*   UDup, VDup: The U and V duplication factors.			     M
*   LclUV:   TRUE to keep local UV coordinates for each tile, FALSE to       M
*	     employ the UV coordinates of the surface Srf.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A set of objects that derive the DDMy surface         M
*	above Srf.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserDDMPolysOverPolys                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserDDMPolysOverSrf                                                      M
*****************************************************************************/
IPObjectStruct *UserDDMPolysOverSrf(const CagdSrfStruct *Srf,
				    const IPObjectStruct *Texture,
				    IrtRType UDup,
				    IrtRType VDup,
				    int LclUV)
{
    CagdRType UMin, UMax, VMin, VMax;

    GlblShiftDirPower = AttrGetObjectIntAttrib(Texture, "DispTextDirPower");

    if (!IP_ATTR_IS_BAD_INT(GlblShiftDirPower)) {
	const char
	    *p = AttrGetObjectStrAttrib(Texture, "DispTextDir");

	if (sscanf(p, "%lf,%lf,%lf",
		   &GlblShiftDir[0],
		   &GlblShiftDir[1],
		   &GlblShiftDir[2]) != 3 &&
	    sscanf(p, "%lf %lf %lf",
		   &GlblShiftDir[0],
		   &GlblShiftDir[1],
		   &GlblShiftDir[2]) != 3) {
	    GlblShiftDir[0] = GlblShiftDir[1] = 0.0;
	    GlblShiftDir[2] = -1.0;
	}
    }

    GlblRetObjs = IPGenLISTObject(NULL);

    GlblLclUV = LclUV;

    GlblSrf = Srf;
    GlblDuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
    GlblDvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

    GlblSrfUClosed = CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR),
    GlblSrfVClosed = CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    GlblDu = (UMax - UMin) / UDup;
    GlblDv = (VMax - VMin) / VDup;

    for (GlblU = UMin; GlblU < UMax; GlblU += GlblDu) {
	for (GlblV = VMin; GlblV < VMax; GlblV += GlblDv) {
	    IrtHmgnMatType Mat;

	    MatGenUnitMat(Mat);

	    /* Texture is still const-safe as UserOneDDMOverSrf is. */
	    IPTraverseObjListHierarchy((IPObjectStruct *) Texture, Mat, 
				       UserOneDDMOverSrf);
	}
    }

    CagdSrfFree(GlblDuSrf);
    CagdSrfFree(GlblDvSrf);

    return GlblRetObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Maps the given UVW location to XYZ above base surface space.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   UVW:		     Input UVW vertex location.                      *
*   UMin, UMax, VMin, VMax:  Surface domain boundaries.		             *
*   Vuv:		     Surface output parametric space coordinates.    *
*   XYZ,		     Output located XYZ new location.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserDDMUVW2XYZ(CagdRType UVW[3],
			   CagdRType UMin,
			   CagdRType UMax,
			   CagdRType VMin,
			   CagdRType VMax,
			   CagdRType Vuv[2],
			   CagdRType XYZ[3])
{
    CagdRType *R;
    CagdVType Pos, T1, T2, VTmp;

    Vuv[0] = UVW[0] * GlblDu + GlblU;
    Vuv[1] = UVW[1] * GlblDv + GlblV;

    /* Make sure inside surface domain. */
    while (TRUE) {
        if (GlblSrfUClosed) {
	    if (Vuv[0] > UMax)
	        Vuv[0] = UMin + (Vuv[0] - UMax);
	    else if (Vuv[0] < UMin)
	        Vuv[0] = UMax + (UMin - Vuv[0]);
	    else
	        break;
	}
	else {
	    if (Vuv[0] > UMax)
	        Vuv[0] = UMax;
	    else if (Vuv[0] < UMin)
	        Vuv[0] = UMin;
	    else
	        break;
	}
    }
    while (TRUE) {
        if (GlblSrfVClosed) {
	    if (Vuv[1] > VMax)
	        Vuv[1] = VMin + (Vuv[1] - VMax);
	    else if (Vuv[1] < VMin)
	        Vuv[1] = VMax + (VMin - Vuv[1]);
	    else
	        break;
	}
	else {
	    if (Vuv[1] > VMax)
	        Vuv[1] = VMax;
	    else if (Vuv[1] < VMin)
	        Vuv[1] = VMin;
	    else
	        break;
	}
    }

    /* Evaluate local frame in Euclidean space. */
    R = CagdSrfEval(GlblSrf, Vuv[0], Vuv[1]);
    CagdCoerceToE3(Pos, &R, -1, GlblSrf -> PType);
		    
    /* Compute the elevtion in base surface normal direction. */
    R = CagdSrfEval(GlblDuSrf, Vuv[0], Vuv[1]);
    CagdCoerceToE3(T1, &R, -1, GlblDuSrf -> PType);

    R = CagdSrfEval(GlblDvSrf, Vuv[0], Vuv[1]);
    CagdCoerceToE3(T2, &R, -1, GlblDvSrf -> PType);

    IRIT_CROSS_PROD(VTmp, T1, T2);
    IRIT_VEC_NORMALIZE(VTmp);
    IRIT_VEC_SCALE(VTmp, UVW[2]);
    IRIT_VEC_ADD(XYZ, Pos, VTmp);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process a single leaf in the DDM texture geometry.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to process.  A leaf in the geometry tree.                 *
*   Mat:    Transformation matrix in the tree hierarchy.  Ignored here.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserOneDDMOverSrf(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(GlblSrf, &UMin, &UMax, &VMin, &VMax);

    if (IP_IS_POLY_OBJ(PObj)) {
        int l;
	IPPolygonStruct *Pl;
        IPObjectStruct
	    *CpObj = IPCopyObject(NULL, PObj, TRUE);

	/* Filter invalid polygons. */
	GMCleanUpPolygonList(&CpObj -> U.Pl, IRIT_EPS);

	for (Pl = CpObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *V;

	    V = Pl -> PVertex;
	    assert(IPVrtxListLen(V) > 2);
	    do {
		CagdRType Vuv[2];
		CagdPType SrfPos;
	        IrtVecType T1, T2, VTmp;

		if (IRIT_APX_EQ_EPS(V -> Coord[0], 0.0, USER_DDM_NORMAL_EPS))
		    V -> Coord[0] = USER_DDM_NORMAL_EPS;
		if (IRIT_APX_EQ_EPS(V -> Coord[0], 1.0, USER_DDM_NORMAL_EPS))
		    V -> Coord[0] = 1.0 - USER_DDM_NORMAL_EPS;

		if (IRIT_APX_EQ_EPS(V -> Coord[1], 0.0, USER_DDM_NORMAL_EPS))
		    V -> Coord[1] = USER_DDM_NORMAL_EPS;
		if (IRIT_APX_EQ_EPS(V -> Coord[1], 1.0, USER_DDM_NORMAL_EPS))
		    V -> Coord[1] = 1.0 - USER_DDM_NORMAL_EPS;

	        UserDDMUVW2XYZ(V -> Coord, UMin, UMax, VMin, VMax,
			       Vuv, SrfPos);

		if (!GlblLclUV)
		    AttrSetUVAttrib(&V -> Attr, "uvvals", Vuv[0], Vuv[1]);

		if (IP_HAS_NORMAL_VRTX(V)) {
		    /* Find two vectors orthogonal to the normal at V. */
		    GMOrthogonalVector(V -> Normal, T1, TRUE);
		    IRIT_CROSS_PROD(T2, V -> Normal, T1);
		    IRIT_CROSS_PROD(VTmp, T1, T2);
		    if (IRIT_DOT_PROD(VTmp, V -> Normal) < 0.0) {
		        IRIT_VEC_SWAP(T1, T2);
		    }
		    IRIT_VEC_NORMALIZE(T1);
		    IRIT_VEC_NORMALIZE(T2);

		    IRIT_VEC_SCALE(T1, USER_DDM_NORMAL_EPS);
		    IRIT_PT_ADD(VTmp, V -> Coord, T1);
		    UserDDMUVW2XYZ(VTmp, UMin, UMax, VMin, VMax, Vuv, VTmp);
		    IRIT_VEC_SUB(T1, SrfPos, VTmp);

		    IRIT_VEC_SCALE(T2, USER_DDM_NORMAL_EPS);
		    IRIT_PT_ADD(VTmp, V -> Coord, T2);
		    UserDDMUVW2XYZ(VTmp, UMin, UMax, VMin, VMax, Vuv, VTmp);
		    IRIT_VEC_SUB(T2, SrfPos, VTmp);

		    IRIT_CROSS_PROD(V -> Normal, T1, T2);
		    IRIT_VEC_NORMALIZE(V -> Normal);
		}

		IRIT_PT_COPY(V -> Coord, SrfPos);
		V = V -> Pnext;
	    }
	    while (V != Pl -> PVertex && V != NULL);

	    assert(IPVrtxListLen(Pl -> PVertex) > 2);
	    IPUpdatePolyPlane(Pl); /* Plane equations is incorrect - fix it. */
	}

	l = IPListObjectLength(GlblRetObjs);
	IPListObjectInsert(GlblRetObjs, l++, CpObj);
	IPListObjectInsert(GlblRetObjs, l, NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Maps the given DDM texture defined in Texture over mesh surface PlSrf,   M
* duplicating it (UDup x VDup) times over the parametric domain of mesh.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlSrf:   Polygonal mesh surface to derive a DDM mapping for.             M
*	     This mesh must have a parametrization as UV coordinates as well M
*	     as normals at the vertices	of the polygons.		     M
*   Texture: Geometry defining the single tile of the DDM texture.           M
*	     The geometry is assumed to span [0..1] in both x and y.	     M
*   UDup, VDup: The U and V duplication factors.			     M
*   LclUV:   TRUE to keep local UV coordinates for each tile, FALSE to       M
*	     employ the UV coordinates of the surface Srf.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A set of objects that derive the DDM surface          M
*	above Srf.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserDDMPolysOverSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserDDMPolysOverPolys                                                    M
*****************************************************************************/
IPObjectStruct *UserDDMPolysOverPolys(IPObjectStruct *PlSrf,
				      const IPObjectStruct *Texture,
				      IrtRType UDup,
				      IrtRType VDup,
				      int LclUV)
{
    int i,
	TriOnly = TRUE;
    IrtRType UMin, UMax, VMin, VMax;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if (!IP_IS_POLY_OBJ(PlSrf) || !IP_IS_POLY_OBJ(Texture)) {
	USER_FATAL_ERROR(USER_ERR_EXPCT_POLY_OBJ);
	return NULL;
    }

    /* Make sure PlSrf has both UV parametrization and normals at vertices. */
    UMin = VMin = IRIT_INFNTY;
    UMax = VMax = -IRIT_INFNTY;
    for (Pl = PlSrf -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        int n = 0;

	V = Pl -> PVertex;
	do {
	    float
		*UV = AttrGetUVAttrib(V -> Attr, "uvvals");

	    n++;		 /* Count number of vertices in a polygons. */
	    if (!IP_HAS_NORMAL_VRTX(V)) {
	        USER_FATAL_ERROR(USER_ERR_EXPCT_VRTX_NRMLS);
		return NULL;
	    }

	    if (UV == NULL) {
	        USER_FATAL_ERROR(USER_ERR_EXPCT_VRTX_UVS);
		return NULL;
	    }
	    else {
		if (UMin > UV[0])
		    UMin = UV[0];
		if (UMax < UV[0])
		    UMax = UV[0];
		if (VMin > UV[1])
		    VMin = UV[1];
		if (VMax < UV[1])
		    VMax = UV[1];
	    }

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);

	if (n != 3)
	    TriOnly = FALSE;
    }

    if (!TriOnly) {
	IPObjectStruct 
	    *PTmp1 = GMConvertPolysToTriangles(PlSrf);

	PlSrf = GMRegularizePolyModel(PTmp1, TRUE);
	IPFreeObject(PTmp1);
    }

    /* Now process one tile at a time. */
    GlblUMin = UMin;
    GlblUSize = UMax - UMin;
    GlblDu = GlblUSize / UDup;
    GlblDv = (VMax - VMin) / VDup;

    GlblRetObjs = IPGenLISTObject(NULL);

    GlblLclUV = LclUV;

    GlblPlSrf = PlSrf;

    /* Preprocess all the polygons by computing UV bbox for each one and    */
    /* classifying the polygons into U buckets for faster access.	    */
    GlblUBuckets = (UserDDMUBucketStruct **)
	IritMalloc(sizeof(UserDDMUBucketStruct *) * USER_DDM_NUM_U_BUCKETS);
    for (i = 0; i < USER_DDM_NUM_U_BUCKETS; i++)
	GlblUBuckets[i] = NULL;

    for (Pl = GlblPlSrf -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	int i1, i2;
        IrtRType
	    *UVMinMax = (IrtRType *) IritMalloc(sizeof(IrtRType) * 4);
	IPVertexStruct
	    *V1 = Pl -> PVertex,
	    *V2 = V1 -> Pnext,
	    *V3 = V2 -> Pnext;
	float
	    *UV1 = AttrGetUVAttrib(V1 -> Attr, "uvvals"),
	    *UV2 = AttrGetUVAttrib(V2 -> Attr, "uvvals"),
	    *UV3 = AttrGetUVAttrib(V3 -> Attr, "uvvals");

	UVMinMax[0] = IRIT_MIN(IRIT_MIN(UV1[0], UV2[0]), UV3[0]);  /* UMin. */
	UVMinMax[1] = IRIT_MIN(IRIT_MIN(UV1[1], UV2[1]), UV3[1]);  /* VMin. */
	UVMinMax[2] = IRIT_MAX(IRIT_MAX(UV1[0], UV2[0]), UV3[0]);  /* UMax. */
	UVMinMax[3] = IRIT_MAX(IRIT_MAX(UV1[1], UV2[1]), UV3[1]);  /* VMax. */
	AttrSetPtrAttrib(&Pl -> Attr, "_uvExtrm", UVMinMax);

	i1 = (int) floor(USER_DDM_NUM_U_BUCKETS * (UVMinMax[0] - GlblUMin)
			                                         / GlblUSize);
	i1 = IRIT_BOUND(i1, 0, USER_DDM_NUM_U_BUCKETS - 1);
	i2 = (int) ceil(USER_DDM_NUM_U_BUCKETS * (UVMinMax[2] - GlblUMin)
			                                         / GlblUSize);
	i2 = IRIT_BOUND(i2, 0, USER_DDM_NUM_U_BUCKETS - 1);
	for (i = i1; i <= i2; i++) {
	    UserDDMUBucketStruct
	        *Bucket = (UserDDMUBucketStruct *)
		    IritMalloc(sizeof(UserDDMUBucketStruct));

	    Bucket -> Pl = Pl;
	    Bucket -> UVMinMax = UVMinMax;
	    IRIT_LIST_PUSH(Bucket, GlblUBuckets[i]);
	}
    }

    /* Synthesize all the texture tiles. */
    for (GlblU = UMin; GlblU < UMax; GlblU += GlblDu) {
	for (GlblV = VMin; GlblV < VMax; GlblV += GlblDv) {
	    IrtHmgnMatType Mat;

	    MatGenUnitMat(Mat);

	    /* Texture is const-safe as UserOneDDMOverPls is. */
	    IPTraverseObjListHierarchy((IPObjectStruct *) Texture, Mat,
				       UserOneDDMOverPls);
	}
    }

    /* Free all auxiliary data. */

    for (Pl = GlblPlSrf -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        IrtRType
	    *UVMinMax =	AttrGetPtrAttrib(Pl -> Attr, "_uvExtrm");

	IritFree(UVMinMax);
	AttrFreeOneAttribute(&Pl -> Attr, "_uvExtrm");
    }

    for (i = 0; i < USER_DDM_NUM_U_BUCKETS; i++) {
	while (GlblUBuckets[i] != NULL) {
	    UserDDMUBucketStruct
	        *Bucket = GlblUBuckets[i];

	    GlblUBuckets[i] = GlblUBuckets[i] -> Pnext;
	    IritFree(Bucket);
	}
    }
    IritFree(GlblUBuckets);

    if (!TriOnly)
	IPFreeObject(PlSrf);

    return GlblRetObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process a single leaf in the DDM texture geometry.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to process.  A leaf in the geometry tree.                 *
*   Mat:    Transformation matrix in the tree hierarchy.  Ignored here.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserOneDDMOverPls(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    if (IP_IS_POLY_OBJ(PObj)) {
        int l;
	IPPolygonStruct *Pl,
	    *MappedPls = NULL;
        IPObjectStruct
	    *CpObj = IPCopyObject(NULL, PObj, TRUE);

	while (CpObj -> U.Pl != NULL) {
	    IPVertexStruct *V;
	    int AllMapped = TRUE;

	    Pl = CpObj -> U.Pl;
	    CpObj -> U.Pl = Pl -> Pnext;
	    Pl -> Pnext = NULL;

	    V = Pl -> PVertex;
	    do {
		AllMapped &= UserOneVrtxDDMOverPls(V);

		V = V -> Pnext;
	    }
	    while (V != Pl -> PVertex && V != NULL);

	    if (AllMapped) {
		IRIT_LIST_PUSH(Pl, MappedPls);
	    }
	    else
		IPFreePolygon(Pl);
	}

	/* If we have any polygon(s) in valid UV domain, keep them. */
	if (MappedPls != NULL) {
	    CpObj -> U.Pl = MappedPls;
	    l = IPListObjectLength(GlblRetObjs);
	    IPListObjectInsert(GlblRetObjs, l++, CpObj);
	    IPListObjectInsert(GlblRetObjs, l, NULL);
	}
	else {
	    IPFreeObject(CpObj);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the triangle in the mesh surface that vertex V is above it and map  *
* V to its Euclidean position.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   V:   Vertex to DDM map over the given polygonal mesh.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if the vertex is in valid UV domain and was succesfully       *
*	  mapped, FALSE otherwise.					     *
*****************************************************************************/
static int UserOneVrtxDDMOverPls(IPVertexStruct *V)
{
    int Idx;
    IrtRType U;

    U = V -> Coord[0] * GlblDu + GlblU;

    Idx = (int) floor(USER_DDM_NUM_U_BUCKETS * (U - GlblUMin) / GlblUSize);
    Idx = IRIT_BOUND(Idx, 0, USER_DDM_NUM_U_BUCKETS - 1);

    /* The try computed bucket and then its neighbors which might work out */
    /* due to numeric errors.						   */
    if (UserOneVrtxDDMOverPlsAux(GlblUBuckets[Idx], V))
	return TRUE;
    else if (Idx > 0 &&
	     UserOneVrtxDDMOverPlsAux(GlblUBuckets[Idx - 1], V))
        return TRUE;
    else if (Idx < USER_DDM_NUM_U_BUCKETS - 1 &&
	     UserOneVrtxDDMOverPlsAux(GlblUBuckets[Idx + 1], V))
        return TRUE;
    else
	return FALSE;
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function to function UserOneVrtxDDMOverPls	         	     *
*****************************************************************************/
static int UserOneVrtxDDMOverPlsAux(const UserDDMUBucketStruct *Buckets,
				    IPVertexStruct *V)
{
    const UserDDMUBucketStruct *Bucket;
    IrtPtType UVVrtx;

    UVVrtx[0] = V -> Coord[0] * GlblDu + GlblU;
    UVVrtx[1] = V -> Coord[1] * GlblDv + GlblV;

    for (Bucket = Buckets; Bucket != NULL; Bucket = Bucket -> Pnext) {
        IrtRType
	    *UVMinMax = Bucket -> UVMinMax;
	IPPolygonStruct
	     *Pl = Bucket -> Pl;

	if (UVVrtx[0] < UVMinMax[0] ||
	    UVVrtx[0] > UVMinMax[2] ||
	    UVVrtx[1] < UVMinMax[1] ||
	    UVVrtx[1] > UVMinMax[3]) {
	    /* Given vertex is out of this triangle's bbox. */
	}
	else {     /* Cannot trivially reject this triangle - check it out. */
	    IPVertexStruct
		*V1 = Pl -> PVertex,
		*V2 = V1 -> Pnext,
		*V3 = V2 -> Pnext;
	    float
		*UV1 = AttrGetUVAttrib(V1 -> Attr, "uvvals"),
		*UV2 = AttrGetUVAttrib(V2 -> Attr, "uvvals"),
		*UV3 = AttrGetUVAttrib(V3 -> Attr, "uvvals");
	    IrtRType *Bary;
	    IrtPtType PUV1, PUV2, PUV3;

	    PUV1[0] = UV1[0];
	    PUV1[1] = UV1[1];
	    PUV2[0] = UV2[0];
	    PUV2[1] = UV2[1];
	    PUV3[0] = UV3[0];
	    PUV3[1] = UV3[1];

	    Bary = GMBaryCentric3Pts2D(PUV1, PUV2, PUV3, UVVrtx);

	    if (Bary != NULL) {
	        int i;
	        IrtRType
		    X = V -> Coord[0],
		    Y = V -> Coord[1],
		    Z = V -> Coord[2];
		IrtVecType Nrml;

		/* Found the triangle, V is above - map V. */
	        for (i = 0; i < 3; i++) {
		    V -> Coord[i] = V1 -> Coord[i] * Bary[0] +
		                    V2 -> Coord[i] * Bary[1] +
		                    V3 -> Coord[i] * Bary[2];
		    Nrml[i] = V1 -> Normal[i] * Bary[0] +
		              V2 -> Normal[i] * Bary[1] +
		              V3 -> Normal[i] * Bary[2];
		}

		IRIT_VEC_NORMALIZE(Nrml);
		IRIT_VEC_SCALE(Nrml, Z);
		IRIT_PT_ADD(V -> Coord, V -> Coord, Nrml);

		if (!GlblLclUV)
		    AttrSetUVAttrib(&V -> Attr, "uvvals", X, Y);

		return TRUE;
	    }
	}	        
    }

    return FALSE;
}
