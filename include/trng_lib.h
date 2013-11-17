/******************************************************************************
* Trng_lib.h - header file for the TRNG library.			      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#ifndef TRNG_LIB_H
#define TRNG_LIB_H

#include <stdio.h>
#include "irit_sm.h"
#include "miscattr.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"

typedef enum {
    TRNG_ERR_DIR_NOT_VALID,
    TRNG_ERR_UNDEF_GEOM,
    TRNG_ERR_WRONG_DOMAIN,
    TRNG_ERR_WRONG_ORDER,
    TRNG_ERR_BSPLINE_NO_SUPPORT,
    TRNG_ERR_GREGORY_NO_SUPPORT,

    TRNG_ERR_UNDEFINE_ERR
} TrngFatalErrorType;

typedef enum {
    TRNG_UNDEF_TYPE = 1260,
    TRNG_TRISRF_BEZIER_TYPE,
    TRNG_TRISRF_BSPLINE_TYPE,
    TRNG_TRISRF_GREGORY_TYPE
} TrngGeomType;

typedef enum {
    TRNG_NO_DIR = CAGD_NO_DIR,
    TRNG_CONST_U_DIR = CAGD_CONST_U_DIR,
    TRNG_CONST_V_DIR = CAGD_CONST_V_DIR,
    TRNG_CONST_W_DIR,
    TRNG_END_DIR
} TrngTriSrfDirType;

typedef struct TrngTriangleStruct {
    struct TrngTriangleStruct *Pnext;
    struct IPAttributeStruct *Attr;
    struct {
	CagdPType Pt;
	CagdVType Nrml;
	CagdUVType UV;
    } T[3];
} TrngTriangleStruct;

typedef struct TrngTriangSrfStruct {
    struct TrngTriangSrfStruct *Pnext;
    struct IPAttributeStruct *Attr;
    TrngGeomType GType;
    CagdPointType PType;
    int Length;		    /* Mesh size (length of edge of triangular mesh. */
    int Order;		      /* Order of triangular surface (Bspline only). */
    CagdRType *Points[CAGD_MAX_PT_SIZE];     /* Pointer on each axis vector. */
    CagdRType *KnotVector;
} TrngTriangSrfStruct;

typedef void (*TrngSetErrorFuncType)(TrngFatalErrorType);

#define TRNG_IS_BEZIER_TRISRF(TriSrf)	((TriSrf) -> GType == TRNG_TRISRF_BEZIER_TYPE)
#define TRNG_IS_BSPLINE_TRISRF(TriSrf)	((TriSrf) -> GType == TRNG_TRISRF_BSPLINE_TYPE)
#define TRNG_IS_GREGORY_TRISRF(TriSrf)	((TriSrf) -> GType == TRNG_TRISRF_GREGORY_TYPE)

#define TRNG_IS_RATIONAL_TRISRF(TriSrf)	CAGD_IS_RATIONAL_PT((TriSrf) -> PType)

/******************************************************************************
*                   + P00k         Assuming a domain of 0 <= U,V,W <= 1, and  *
*                  / \W = 1        an edge of k control points, the drawing   *
*                 /   \            on the left shows the way the mesh is      *
*           P10k +     \           defined.				      *
*               /       \						      *
*              /         \						      *
*             /           \	   Rows are order in the Points slot of	      *
*            /             \	   TrngTriangSrfStruct staring from Pk00      *
*           /               \	   along the horizontal line to P0k0, the     *
*          /        P0,k-1,1 +	   second line from Pk-1,0,k-1 to P0,k-1,1,   *
*         /                   \		   up to the last row of size one of  *
*  U = 1 /                     \  V = 1    P00k.			      *
*  Pk00 +-----------------------+ P0k0					      *
******************************************************************************/

#define TRNG_TRISRF_MESH_SIZE(TriSrf)  (((TriSrf) -> Length + 1) * \
					(TriSrf) -> Length / 2 + \
					(TRNG_IS_GREGORY_TRISRF(TriSrf) ? 3 : 0))
#define TRNG_LENGTH_MESH_SIZE(Length)	(((Length) + 1) * (Length) / 2)

#define TRNG_TRI_GRG_SRF_MESH_SIZE(TriSrf)  (TRNG_TRISRF_MESH_SIZE(TriSrf) + 3)

/* Index of point Pijk, where k is in fact equal to 'Length - (i+j)' into    */
/* the Points vector of TrngTriangSrfStruct.				     */
/*   Provided k > 0 equal:						     */
/* j + ((Length - (k - 1)) + Length) * (k / 2) =			     */
/* j + (2 * Length - k + 1) * (k / 2)					     */

#define TRNG_MESH_JK(TriSrf, j, k)	((j) + (2 * (TriSrf) -> Length - \
						(k) + 1) * (k) / 2)

#define TRNG_MESH_IJK(TriSrf, i, j, k)	TRNG_MESH_JK(TriSrf, j, k)	

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/******************************************************************************
* General routines of the triangular surface library:			      *
******************************************************************************/
CagdRType TrngIJChooseN(int i, int j, int N);
TrngTriangSrfStruct *TrngTriSrfNew(TrngGeomType GType,
				   CagdPointType PType,
				   int Length);
TrngTriangSrfStruct *TrngBspTriSrfNew(int Length,
				      int Order,
				      CagdPointType PType);
TrngTriangSrfStruct *TrngBzrTriSrfNew(int Length, CagdPointType PType);
TrngTriangSrfStruct *TrngGrgTriSrfNew(int Length, CagdPointType PType);
TrngTriangSrfStruct *TrngTriSrfCopy(const TrngTriangSrfStruct *TriSrf);
TrngTriangSrfStruct *TrngTriSrfCopyList(const TrngTriangSrfStruct *TriSrfList);
void TrngTriSrfFree(TrngTriangSrfStruct *TriSrf);
void TrngTriSrfFreeList(TrngTriangSrfStruct *TriSrfList);

TrngTriangSrfStruct *TrngCnvrtBzr2BspTriSrf(const TrngTriangSrfStruct
						                      *TriSrf);
TrngTriangSrfStruct *TrngCnvrtGregory2BzrTriSrf(TrngTriangSrfStruct *TriSrf);

void TrngTriSrfTransform(TrngTriangSrfStruct *TriSrf,
			 CagdRType *Translate,
			 CagdRType Scale);
void TrngTriSrfMatTransform(TrngTriangSrfStruct *TriSrf,
			    CagdMType Mat);

TrngTriangSrfStruct *TrngCoerceTriSrfsTo(const TrngTriangSrfStruct *TriSrf,
					 CagdPointType PType);
TrngTriangSrfStruct *TrngCoerceTriSrfTo(const TrngTriangSrfStruct *TriSrf,
					CagdPointType PType);

void TrngTriSrfDomain(const TrngTriangSrfStruct *TriSrf,
		      CagdRType *UMin,
		      CagdRType *UMax,
		      CagdRType *VMin,
		      CagdRType *VMax,
		      CagdRType *WMin,
		      CagdRType *WMax);
CagdBType TrngParamsInDomain(const TrngTriangSrfStruct *TriSrf,
			     CagdRType u,
			     CagdRType v,
			     CagdRType w);
CagdRType *TrngTriSrfEval(const TrngTriangSrfStruct *TriSrf,
			  CagdRType u,
			  CagdRType v,
			  CagdRType w);
CagdRType *TrngTriSrfEval2(const TrngTriangSrfStruct *TriSrf,
			   CagdRType u,
			   CagdRType v);
CagdVecStruct *TrngTriSrfNrml(const TrngTriangSrfStruct *TriSrf,
			      CagdRType u,
			      CagdRType v);
void TrngTriSrfBBox(const TrngTriangSrfStruct *TriSrf, CagdBBoxStruct *BBox);
void TrngTriSrfListBBox(const TrngTriangSrfStruct *TriSrfs,
			CagdBBoxStruct *BBox);
CagdPolylineStruct *TrngTriSrf2CtrlMesh(const TrngTriangSrfStruct *TriSrf);
CagdBType TrngBspTriSrfHasOpenEC(const TrngTriangSrfStruct *TriSrf);
TrngTriangSrfStruct *TrngBspTriSrfOpenEnd(const TrngTriangSrfStruct *TriSrf);
CagdPolygonStruct *TrngTriSrf2Polygons(const TrngTriangSrfStruct *TriSrf,
				       int FineNess,
				       CagdBType ComputeNormals,
				       CagdBType ComputeUV);
CagdPolylineStruct *TrngTriSrf2Polylines(const TrngTriangSrfStruct *TriSrf,
					 int NumOfIsocurves[3],
					 CagdRType TolSamples,
					 SymbCrvApproxMethodType Method);
CagdCrvStruct *TrngTriSrf2Curves(const TrngTriangSrfStruct *TriSrf,
				 int NumOfIsocurves[2]);
TrngTriangSrfStruct *TrngBzrTriSrfDirecDerive(const TrngTriangSrfStruct *TriSrf,
					      CagdVType DirecDeriv);
TrngTriangSrfStruct *TrngTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
				      TrngTriSrfDirType Dir);
TrngTriangSrfStruct *TrngBzrTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
					 TrngTriSrfDirType Dir);
TrngTriangSrfStruct *TrngBspTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
					 TrngTriSrfDirType Dir);
CagdCrvStruct *TrngCrvFromTriSrf(const TrngTriangSrfStruct *TriSrf,
				 CagdRType t,
				 TrngTriSrfDirType Dir);
CagdBType TrngTriSrfsSame(const TrngTriangSrfStruct *Srf1,
			  const TrngTriangSrfStruct *Srf2,
			  CagdRType Eps);

void TrngDbg(void *Obj);

void TrngGregory2Bezier4(CagdRType **Qt, CagdRType **Pt);
void TrngGregory2Bezier5(CagdRType **Qt, CagdRType **Pt);
void TrngGregory2Bezier6(CagdRType **Qt, CagdRType **Pt);

/******************************************************************************
* Error handling.							      *
******************************************************************************/
TrngSetErrorFuncType TrngSetFatalErrorFunc(TrngSetErrorFuncType ErrorFunc);
void TrngFatalError(TrngFatalErrorType ErrID);
const char *TrngDescribeError(TrngFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* TRNG_LIB_H */
