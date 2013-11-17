/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Definitions, visible to others, of Boolean operation modules:	     *
*****************************************************************************/

#ifndef BOOL_LIB_H
#define BOOL_LIB_H

/* Boolean operations types: */
typedef enum {
    BOOL_OPER_OR = 1,
    BOOL_OPER_AND,
    BOOL_OPER_SUB,
    BOOL_OPER_NEG,
    BOOL_OPER_CUT,
    BOOL_OPER_ICUT,
    BOOL_OPER_MERGE,
    BOOL_OPER_SELF,
    BOOL_OPER_CONTOUR
} BoolOperType;

typedef enum {
    BOOL_ERR_NO_POLY_OBJ,
    BOOL_ERR_NO_BOOL_OP_SUPPORT,
    BOOL_ERR_NO_MATCH_POINT,
    BOOL_ERR_NO_ELMNT_TO_DEL,
    BOOL_ERR_SORT_INTER_LIST,
    BOOL_ERR_FIND_VERTEX_FAILED,
    BOOL_ERR_NO_COPLANAR_VRTX,
    BOOL_ERR_NO_OPEN_LOOP,
    BOOL_ERR_NO_NEWER_VERTEX,
    BOOL_ERR_NO_INTERSECTION,
    BOOL_ERR_LOOP_LESS_3_VRTCS,
    BOOL_ERR_NO_INVERSE_MAT,
    BOOL_ERR_ADJ_STACK_OF,
    BOOL_ERR_CIRC_VRTX_LST,
    BOOL_ERR_NO_2D_OP_SUPPORT,
    BOOL_ERR_NO_PLLN_MATCH,
    BOOL_ERR_DISJ_PROP_ERR,
    BOOL_ERR_EMPTY_POLY_OBJ,

    BOOL_ERR_UNDEFINE_ERR
} BoolFatalErrorType;

typedef struct Bool2DInterStruct {  /* Holds info. on 2D intersetion points. */
    struct Bool2DInterStruct *Pnext;
    IPVertexStruct *Poly1Vrtx, *Poly2Vrtx;  /* Pointer to Pl1/2 inter. vrtx. */
    IPVertexStruct *Poly1Vrtx2, *Poly2Vrtx2; /* In share corners - 2 inters! */
    int DualInter;   /* If two intersections at the same location (corners). */
    IrtRType Param1, Param2;     /* Parametrization along the poly vertices. */
    IrtPtType InterPt;				/* Location of intersection. */
    IrtVecType Normal;			/* Estimated normal at intersection. */
} Bool2DInterStruct;

#define BOOL_DISJ_GET_INDEX(Pl)		Pl -> IAux2
#define BOOL_DISJ_SET_INDEX(Pl, Index)	Pl -> IAux2 = Index
#define BOOL_DISJ_RESET(Pl)		BOOL_DISJ_SET_INDEX(Pl, 0)

typedef void (*BoolFatalErrorFuncType)(BoolFatalErrorType ErrID);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Prototypes of the global routines in adjacency.c module: */
int BoolGenAdjacencies(IPObjectStruct *PObj);
int BoolGenAdjSetSrfBoundaries(IrtRType UMin,
			       IrtRType VMin,
			       IrtRType UMax,
			       IrtRType VMax);
void BoolClnAdjacencies(IPObjectStruct *PObj);
int BoolMarkDisjointParts(IPObjectStruct *PObj);
IPPolygonStruct *BoolGetDisjointPart(IPObjectStruct *PObj, int Index);
IPVertexStruct *BoolGetAdjEdge(IPVertexStruct *V);

/* Prototypes of global functions in bool-2d.c module: */
IPPolygonStruct *Boolean2D(IPPolygonStruct *Pl1,
			   IPPolygonStruct *Pl2,
			   BoolOperType BoolOper);
Bool2DInterStruct *Boolean2DComputeInters(IPPolygonStruct *Pl1,
					  IPPolygonStruct *Pl2,
					  int HandlePolygons,
					  int DetectIntr);
int BoolFilterCollinearities(IPPolygonStruct *Pl);

/* Prototype of the global functions in the Boolean operations module: */
IPObjectStruct *BooleanOR(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanAND(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanSUB(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanNEG(IPObjectStruct *PObj);
IPObjectStruct *BooleanCUT(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanICUT(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanMERGE(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanSELF(IPObjectStruct *PObj);
IPObjectStruct *BooleanCONTOUR(IPObjectStruct *PObj, IrtPlnType Pln);
IPObjectStruct *BooleanMultiCONTOUR(IPObjectStruct *PObj,
				    IrtRType CntrLevel,
				    int Axis,
				    int Init,
				    int Done);

IPPolygonStruct *BoolInterPolyPoly(IPPolygonStruct *Pl1, IPPolygonStruct *Pl2);

void BoolDfltFatalError(BoolFatalErrorType ErrID);
BoolFatalErrorFuncType BoolSetFatalErrorFunc(BoolFatalErrorFuncType ErrFunc);
const char *BoolDescribeError(BoolFatalErrorType ErrorNum);

int BoolSetOutputInterCurve(int OutputInterCurve);
IrtRType BoolSetPerturbAmount(IrtRType PerturbAmount);
int BoolSetHandleCoplanarPoly(int HandleCoplanarPoly);
int BoolSetParamSurfaceUVVals(int HandleBoolParamSrfUVVals);
int BoolSetPolySortAxis(int PolySortAxis);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* BOOL_LIB_H */
