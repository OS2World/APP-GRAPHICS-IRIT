/******************************************************************************
* Mvar_loc.h - header file for the MVAR library.			      *
* This header is local to the library - it is NOT external.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#ifndef MVAR_LOC_H
#define MVAR_LOC_H

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "mvar_lib.h"		     /* Include the external header as well. */

#define MVAR_BSCT_NUMER_TOL	1e-10

#define MVAR_NUMER_ZERO_NUM_STEPS	100
#define MVAR_ZERO_PARAM_REL_PERTURB	0.0000301060
#define MVAR_ZERO_PRECOND_SCALE		10

#define MVAR_EXPR_TREE_GET_COMMON_EXPR(ET) ( \
    assert(ET -> NodeType == MVAR_ET_NODE_COMMON_EXPR && \
	   ET -> Left != NULL),			 \
    ET -> Left)

IRIT_GLOBAL_DATA_HEADER CagdRType 
    MvarBsctSubdivTol, 
    MvarBsctNumerTol,
    MvarBsctUVTol;

typedef enum {
    MV_CV_CV = 1,
    MV_CV_PT,
    MV_NONE
} MvarBsctType;

typedef struct MvarVoronoiCrvStruct {
    struct MvarVoronoiCrvStruct *Pnext;
    MvarBsctType Type;
    CagdSrfStruct *F3;
    CagdCrvStruct *Crv1;
    CagdCrvStruct *Crv2; /* Crv2 will hold the second curve if Type is       */
                       /* CV_CV, and the rational bisector if Type is CV_PT. */
    CagdPType Pt;
} MvarVoronoiCrvStruct;

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call MvarFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define MVAR_FATAL_ERROR(Msg)	MvarFatalError(Msg)

/* Global vars defined in mvarzeros.c. */
IRIT_GLOBAL_DATA_HEADER int
    _MVGlblZeroApplyDomainReduction,
    _MVGlblZeroApplyGradPreconditioning,
    _MVGlblZeroApplyParallelHyperPlaneTest,
    _MVGlblZeroApplyNormalConeTest;
IRIT_GLOBAL_DATA_HEADER CagdRType
    _MVGlblZeroParamPerturb;

/* Global variables defined in mvarzer2.c. */
IRIT_GLOBAL_DATA_HEADER MvarMVsZerosSubdivCallBackFunc
    _MVGlblZeroETSubdivCallBackFunc;
IRIT_GLOBAL_DATA_HEADER CagdBType 
    _MVGlblETUseCommonExpr,
    _MVGlblETCagdCnvrtBezier2MVs;

/******************************************************************************
* Voronoi cell computation.						      *
******************************************************************************/
int MvarBsctIsCurveLL(MvarVoronoiCrvStruct *Cv);
MvarVoronoiCrvStruct *MvarBsctApplyLL(MvarVoronoiCrvStruct *Cv);
int MvarBsctApplyCC(MvarVoronoiCrvStruct *Cv1, 
		    MvarVoronoiCrvStruct **CCFreeCrvs);
MvarVoronoiCrvStruct *MvarBsctPurgeAwayLLAndCCConstraints(MvarVoronoiCrvStruct 
							          *InputCrvs);
void MvarBsctTrimCurveBetween(MvarVoronoiCrvStruct *Cv, 
			      MvarPtStruct *Pt1, 
			      MvarPtStruct *Pt2, 
			      MvarVoronoiCrvStruct **TrimmedCurve);
void MvarBsctComputeLowerEnvelope(MvarVoronoiCrvStruct *inputCurves, 
				  MvarVoronoiCrvStruct **lowerEnvelope);
MvarPtStruct *MvarBsctImplicitCrvExtremeAliter(CagdSrfStruct *Srf,
					       CagdSrfDirType Dir,
					       CagdRType MvarBsctSubdivTol,
					       CagdRType MvarBsctNumerTol);
CagdSrfStruct *MvarBsctTrimSurfaceByUVBbox(CagdSrfStruct *Srf, 
					   CagdBBoxStruct UVBbox);
MvarPtStruct *MvarBsctNewFindZeroSetOfSrfAtParam(CagdSrfStruct *Srf, 
						 CagdRType Param,
						 CagdSrfDirType Dir, 
						 CagdRType MvarBsctSubdivTol,
						 CagdRType MvarBsctNumerTol,
						 CagdBType ShouldCheckEndPoints);
void MvarBsctSplitImplicitCrvToMonotonePieces(CagdSrfStruct *Srf,
					      CagdSrfStruct **OutLst,
					      CagdRType MvarBsctSubdivTol,
					      CagdRType MvarBsctNumerTol);
void MvarBsctComputeDenomOfP(CagdCrvStruct *Crv1Inp,	
			     CagdCrvStruct *Crv2Inp,	
			     CagdSrfStruct **DenomOut);
CagdRType *MvarComputeInterMidPoint(CagdCrvStruct *Crv1,
				    CagdRType t1,
				    CagdCrvStruct *Crv2,
				    CagdRType t2);
void MvarBsctComputeF3(CagdCrvStruct *Crv1Inp,                       
		       CagdCrvStruct *Crv2Inp,			           
		       CagdCrvStruct **Crv1Coerced,
		       CagdCrvStruct **Crv2Coerced,	
		       CagdSrfStruct **F3,
		       CagdSrfStruct **L1,			           
		       CagdSrfStruct **L2,			           
		       CagdSrfStruct **CC1,			           
		       CagdSrfStruct **CC2);
CagdRType *MvarBsctComputeXYFromBisTR(CagdCrvStruct *Crv1,
				      CagdRType t,
				      CagdCrvStruct *Crv2,
				      CagdRType r);
MvarPtStruct *MvarBsctSkel2DEqPts3Crvs(CagdCrvStruct *Crv1,
				       CagdCrvStruct *Crv2,
				       CagdCrvStruct *Crv3);
MvarVoronoiCrvStruct *MvarVoronoiCrvNew(void);
MvarVoronoiCrvStruct *MvarVoronoiCrvCopy(MvarVoronoiCrvStruct *Crv);
void MvarVoronoiCrvFree(MvarVoronoiCrvStruct *Crv);
void MvarVoronoiCrvFreeList(MvarVoronoiCrvStruct *CrvList);
MvarVoronoiCrvStruct *MvarVoronoiCrvReverse(MvarVoronoiCrvStruct *Crv);
int MvarBsctIsXSmaller(MvarPtStruct *P1, MvarPtStruct *P2);
void MvarBsctCurveLeft(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res);
void MvarBsctCurveRight(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res);
int MvarBsctCv1IsYSmallerAt(MvarVoronoiCrvStruct *Cv1, 
			    MvarVoronoiCrvStruct *Cv2, 
			    MvarPtStruct *MidPoint);
void MvarBsctGetAllIntersectionPoints(MvarVoronoiCrvStruct *Cv1, 
				      MvarVoronoiCrvStruct *Cv2, 
				      MvarPtStruct **Points);
void MvarBsctSplitCurve(MvarVoronoiCrvStruct *Cv, 
			MvarPtStruct *SplitPt, 
			MvarVoronoiCrvStruct **CvLeft,
			MvarVoronoiCrvStruct **CvRight);
CagdCrvStruct *MvarBsctTrimCrvPt(CagdCrvStruct *Crv, 
				 CagdRType *Pt, 
				 CagdRType Alpha,
				 CagdCrvStruct *BaseCrv);

/******************************************************************************
* Minimal spanning circle/sphere computation.				      *
******************************************************************************/
int MvarMSConstraints(CagdPtStruct *PointList,
		      struct IPObjectStruct **MVObjs,
		      int LenOfMVs,
		      CagdRType *Center,
		      CagdRType *Radius,
		      CagdRType SubdivTol,
		      CagdRType NumerTol);
MvarMVStruct **MvarMSConstraintsOfAPair(CagdPtStruct *PointList,
					struct IPObjectStruct **MVObjs,
					int LenOfMVs,
					int Dim);
MvarMVStruct **MvarMSConstraintsOfATriplet(CagdPtStruct *PointList,
					   struct IPObjectStruct **MVObjs,
					   int LenOfMVs,
					   MvarMVStruct **MVPDenom1, 
					   MvarMVStruct **MVPNumer1,
					   int Dim);

/******************************************************************************
* Zero set computation.							      *
******************************************************************************/
MvarPtStruct *MvarZeroGenPtMidMvar(const MvarMVStruct *MV, int SingleSol);
MvarPtStruct *MvarZeroFilterIdenticalSet(MvarPtStruct *ZeroSet,
					 MvarMVStruct * const *MVs,
					 const MvarConstraintType *Constraints,
					 int NumOfMVs,
					 int NumOfZeroMVs,
					 CagdRType Tol);
MvarPtStruct *MvarZeroMVsSubdiv(MvarMVStruct **MVs,
				MvarConstraintType *Constraints,
				int NumOfMVs,
				int NumOfZeroMVs,
				int ApplyNormalConeTest,
				CagdRType SubdivTol,
				int Depth);
MvarPtStruct *MvarZeroGetRootsByKantorovich(MvarMVStruct **MVs,
                                            MvarConstraintType *Constraints,
					    int NumOfMVs,
					    int NumOfZeroMVs,
					    int ApplyNormalConeTest,
					    CagdRType SubdivTol,
					    int Depth);

/******************************************************************************
* Zero set ET computation.						      *
******************************************************************************/
/* Expression trees' equations handling/building routine. */
MvarExprTreeEqnsStruct *MvarExprTreeEqnsMalloc(int NumEqns,
					       int MaxNumCommonExprs);
void MvarExprTreeEqnsFree(MvarExprTreeEqnsStruct *Eqns);
void MvarExprTreeEqnsReallocCommonExprs(MvarExprTreeEqnsStruct *Eqns,
					int NewSize);
void MvarExprTreeEqnsUpdateCommonExprIdcs(MvarExprTreeStruct *ET,
					  MvarExprTreeStruct **CommonExprs,
					  int NumOfCommonExpr);
int MvarExprTreeEqnsSubdivAtParam(const MvarExprTreeEqnsStruct *Eqns,
				  CagdRType t,
				  MvarMVDirType Dir,
				  MvarExprTreeEqnsStruct **Eqns1,
				  MvarExprTreeEqnsStruct **Eqns2);
MvarExprTreeEqnsStruct *MvarExprTreeEqnsBuild(MvarExprTreeStruct **MVETs,
					      const MvarConstraintType
					                   *ConstraintsTypes,
					      int NumOfMVETs);
MvarExprTreeEqnsStruct *MvarExprTreeEqnsBuild2(MvarMVStruct * const *MVs,
					       const MvarConstraintType
					                   *ConstraintTypes,
					       int NumOfMVs);
int MvarExprTreeToVector(MvarExprTreeStruct *ET,
			 MvarExprTreeStruct **Vec,
			 int Idx);
int MvarExprTreeRemoveFromVector(MvarExprTreeStruct **Vec, int Idx);

/* Expression trees' solving aux. routines. */
MvarPtStruct *MvarEqnsZeroNumeric(MvarPtStruct *ZeroSet,
				  const MvarExprTreeEqnsStruct *Eqns,
				  MvarMVStruct * const *MVs,
				  int NumMVs,
				  CagdRType NumericTol);
MvarPtStruct *MvarExprTreeEqnsZeroFilterIdenticalSet(
					   MvarPtStruct *ZeroSet,
					   const MvarExprTreeEqnsStruct *Eqns,
					   CagdRType Tol);
MvarPtStruct *MvarExprTreeEqnsMidPt(MvarExprTreeStruct * const *MVETs,
				    int NumOfMVETs,
				    int SingleSol);

#endif /* MVAR_LOC_H */
