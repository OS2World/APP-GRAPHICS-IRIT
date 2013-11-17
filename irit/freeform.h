/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to provide the required interfact for the cagd library for the    *
* free form surfaces and curves.					     *
*****************************************************************************/

#ifndef FREEFORM_H
#define FREEFORM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MIN_FREE_FORM_RES 2

CagdRType *GetKnotVector(IPObjectStruct *KntObjList,
			 int Order,
			 int *Length,
			 char **ErrStr,
			 int Validate);
int GetDrawCtlPt(void);
int GetFourPerFlat(void);
int GetPolyApproxOptimal(void);
int GetPolyApproxUV(void);
int GetPolyApproxTri(void);
int GetPolyMergeCoplanar(void);
IrtRType GetPolyApproxTol(void);
IPObjectStruct *GenPowerSurfaceObject(IPObjectStruct *LstObjList);
IPObjectStruct *GenPowerCurveObject(IPObjectStruct *PtObjList);
IPObjectStruct *GenBezierSurfaceObject(IPObjectStruct *LstObjList);
IPObjectStruct *GenBezierCurveObject(IPObjectStruct *PtObjList);
IPObjectStruct *GenBsplineSurfaceObject(IrtRType *UOrder,
					IrtRType *VOrder,
					IPObjectStruct *LstObjList,
					IPObjectStruct *KntObjList);
IPObjectStruct *GenBsplineCurveObject(IrtRType *ROrder,
				      IPObjectStruct *PtObjList,
				      IPObjectStruct *KntObjList);
IPObjectStruct *GenSURFPREVObject(IPObjectStruct *Cross);
IPObjectStruct *DivideSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *Dir,
				    IrtRType *ParamVal);
IPObjectStruct *RegionFromSurfaceObject(IPObjectStruct *SrfObj,
					IrtRType *RDir,
					IrtRType *ParamVal1,
					IrtRType *ParamVal2);
IPObjectStruct *DivideCurveObject(IPObjectStruct *CrvObj, IrtRType *ParamVal);
IPObjectStruct *RegionFromCurveObject(IPObjectStruct *CrvObj,
				      IrtRType *ParamVal1,
				      IrtRType *ParamVal2);
IPObjectStruct *RefineSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *Dir,
				    IrtRType *Replace,
				    IPObjectStruct *KnotsObj);
IPObjectStruct *RefineCurveObject(IPObjectStruct *CrvObj,
				  IrtRType *Replace,
				  IPObjectStruct *KnotsObj);
IPObjectStruct *EvalSurfaceObject(IPObjectStruct *SrfObj,
				  IrtRType *u,
				  IrtRType *v);
IPObjectStruct *EvalCurveObject(IPObjectStruct *CrvObj, IrtRType *t);
IPObjectStruct *DeriveSurfaceObject(IPObjectStruct *SrfObj, IrtRType *Dir);
IPObjectStruct *IntegrateSurfaceObject(IPObjectStruct *SrfObj, IrtRType *Dir);
IPObjectStruct *DeriveCurveObject(IPObjectStruct *CrvObj);
IPObjectStruct *IntegrateCurveObject(IPObjectStruct *CrvObj);
IPObjectStruct *MoebiusCurveTrans(IPObjectStruct *CrvObj, IrtRType *C);
IPObjectStruct *MoebiusSurfaceTrans(IPObjectStruct *SrfObj,
				    IrtRType *C,
				    IrtRType *RDir);
IPObjectStruct *SurfaceNormalObject(IPObjectStruct *SrfObj);
IPObjectStruct *CurveNormalObject(IPObjectStruct *CrvObj);
IPObjectStruct *NormalSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *u,
				    IrtRType *v);
IPObjectStruct *NormalCurveObject(IPObjectStruct *CrvObj, IrtRType *t);
IPObjectStruct *TangentSurfaceObject(IPObjectStruct *SrfObj,
				     IrtRType *Dir,
				     IrtRType *u,
				     IrtRType *v,
				     IrtRType *Normalize);
IPObjectStruct *TangentCurveObject(IPObjectStruct *CrvObj,
				   IrtRType *t,
				   IrtRType *Normalize);
IPObjectStruct *CurveFromSrfMesh(IPObjectStruct *SrfObj,
				 IrtRType *Dir,
				 IrtRType *Index);
IPObjectStruct *CurveFromSurface(IPObjectStruct *SrfObj,
				 IrtRType *Dir,
				 IrtRType *ParamVal);
IPObjectStruct *MeshGeometry2Polylines(IPObjectStruct *Obj);
IPObjectStruct *SurfaceReverse(IPObjectStruct *SrfObj);
void ComputeSurfacePolygons(IPObjectStruct *PObj, int Normals);
void ComputeTrimSrfPolygons(IPObjectStruct *PObj, int Normals);
void ComputeTriSrfPolygons(IPObjectStruct *PObj, int Normals);
void ComputeTrivarPolygons(IPObjectStruct *PObj, int Normals);
IPObjectStruct *Geometry2Polygons(IPObjectStruct *Obj, IrtRType *RNormals);
IPObjectStruct *Geometry2Polylines(IPObjectStruct *Obj, IrtRType *Optimal);
IPObjectStruct *Geometry2PointList(IPObjectStruct *Obj,
				   IrtRType *Optimal,
				   IrtRType *Merge);
IPObjectStruct *ExtremumControlPointVals(IPObjectStruct *Obj, CagdRType *Min);
IPObjectStruct *GenCircleCurveObject(IrtVecType Position, IrtRType *Radius);
IPObjectStruct *GenPCircleCurveObject(IrtVecType Position, IrtRType *Radius);
IPObjectStruct *GenArcCurveObject(IrtVecType Start,
				  IrtVecType Center,
				  IrtVecType End);
IPObjectStruct *GenSpiralCurveObject(CagdRType *NumOfLoops,
				     CagdRType *Pitch,
				     CagdRType *RSampling,
				     CagdRType *RCtlPtsPerLoop);
IPObjectStruct *GenHelixCurveObject(CagdRType *NumOfLoops,
				    CagdRType *Pitch,
				    CagdRType *Radius,
				    CagdRType *RSampling,
				    CagdRType *RCtlPtsPerLoop);
IPObjectStruct *GenSineCurveObject(CagdRType *NumOfCycles,
				   CagdRType *RSampling,
				   CagdRType *RCtlPtsPerCycle);
IPObjectStruct *GenArc2CurveObject(IrtVecType Center,
				   IrtRType *Radius,
				   IrtRType *StartAngle,
				   IrtRType *EndAngle);
IPObjectStruct *GenRuledSrfObject(IPObjectStruct *Obj1, IPObjectStruct *Obj2);
IPObjectStruct *GenRuledTVObject(IPObjectStruct *Obj1, IPObjectStruct *Obj2);
IPObjectStruct *GenBoolSumSrfObject(IPObjectStruct *Crv1,
				    IPObjectStruct *Crv2,
				    IPObjectStruct *Crv3,
				    IPObjectStruct *Crv4);
IPObjectStruct *GenBoolOneSrfObject(IPObjectStruct *BndryCrv);
IPObjectStruct *GenTrivBoolSumSrfObject(IPObjectStruct *Srf1,
					IPObjectStruct *Srf2,
					IPObjectStruct *Srf3,
					IPObjectStruct *Srf4,
					IPObjectStruct *Srf5,
					IPObjectStruct *Srf6);
  IPObjectStruct *GenTrivBoolOneSrfObject(IPObjectStruct *BndrySrf);
IPObjectStruct *GenSrfFromCrvsObject(IPObjectStruct *CrvList,
				     IrtRType *OtherOrder,
				     IrtRType *OtherEC);
IPObjectStruct *GenSrfInterpolateCrvsObject(IPObjectStruct *CrvList,
					    IrtRType *OtherOrder,
					    IrtRType *OtherEC,
					    IrtRType *OtherParam);
IPObjectStruct *GenSweepScaleSrfObject(IPObjectStruct *CrossSection,
				       IPObjectStruct *Axis,
				       IPObjectStruct *Scale,
				       IPObjectStruct *Frame,
				       IrtRType *RRefine);
IPObjectStruct *GenSweepSrfObject(IPObjectStruct *CrossSection,
				  IPObjectStruct *Axis, 
				  IPObjectStruct *Frame);
IPObjectStruct *GenOffsetObject(IPObjectStruct *Obj,
				IPObjectStruct *Offset,
				IrtRType *Tolerance,
				IrtRType *BezInterp);
IPObjectStruct *GenAOffsetObject(IPObjectStruct *Obj,
				 IPObjectStruct *Offset,
				 IrtRType *Epsilon,
				 IrtRType *Trim,
				 IrtRType *BezInterp);
IPObjectStruct *HasLclSelfInterOffsetObject(IPObjectStruct *Obj,
					    IPObjectStruct *OffsetObj);
IPObjectStruct *GenLeastSqrOffsetObject(IPObjectStruct *Obj,
					IrtRType *Offset,
					IrtRType *NumOfSamples,
					IrtRType *NumOfDOF,
					IrtRType *Order);
IPObjectStruct *GenMatchingOffsetObject(IPObjectStruct *Obj,
					IrtRType *Offset,
					IrtRType *Tolerance);
IPObjectStruct *TrimOffsetObject(IPObjectStruct *Crv,
				 IPObjectStruct *OffCrv,
				 IPObjectStruct *ParamsObj);
IPObjectStruct *MergeSrfSrf(IPObjectStruct *Srf1,
			    IPObjectStruct *Srf2,
			    IrtRType *Dir,
			    IrtRType *SameEdge);
IPObjectStruct *MergeCurvesAndCtlPoints(IPObjectStruct *PObj1,
					IPObjectStruct *PObj2);
IPObjectStruct *EditCrvControlPoint(IPObjectStruct *PObjCrv,
				    IPObjectStruct *CtlPt,
				    IrtRType *Index);
IPObjectStruct *EditSrfControlPoint(IPObjectStruct *PObjSrf,
				    IPObjectStruct *CtlPt,
				    IrtRType *UIndex,
				    IrtRType *VIndex);
IPObjectStruct *EditTVControlPoint(IPObjectStruct *PObjTV,
				   IPObjectStruct *CtlPt,
				   IrtRType *UIndex,
				   IrtRType *VIndex,
				   IrtRType *WIndex);
IPObjectStruct *RaiseCurveObject(IPObjectStruct *PObjCrv, IrtRType *Order);
IPObjectStruct *ReduceCurveObject(IPObjectStruct *PObjCrv, IrtRType *RNewOrder);
IPObjectStruct *RaiseSurfaceObject(IPObjectStruct *PObjSrf,
				   IrtRType *RDir,
				   IrtRType *RNewOrder);
IPObjectStruct *RaiseTrivarObject(IPObjectStruct *PObjTV,
				  IrtRType *RDir,
				  IrtRType *RNewOrder);
IPObjectStruct *RaiseMultivarObject(IPObjectStruct *PObjMV,
				    IrtRType *RDir,
				    IrtRType *RNewOrder);
void MakeFreeFormCompatible(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *TwoCrvsMorphing(IPObjectStruct *PObjCrv1,
				IPObjectStruct *PObjCrv2,
				IrtRType *Method,
				IrtRType *Blend);
IPObjectStruct *TwoSrfsMorphing(IPObjectStruct *PObjSrf1,
				IPObjectStruct *PObjSrf2,
				IrtRType *Blend);
IPObjectStruct *TwoTVsMorphing(IPObjectStruct *PObjTV1,
			       IPObjectStruct *PObjTV2,
			       IrtRType *Blend);
IPObjectStruct *CnvrtBezierToPower(IPObjectStruct *PObj);
IPObjectStruct *CnvrtGregoryToBezier(IPObjectStruct *PObj);
IPObjectStruct *CnvrtPowerToBezier(IPObjectStruct *PObj);
IPObjectStruct *CnvrtBezierToBspline(IPObjectStruct *PObj);
IPObjectStruct *CnvrtBsplineToBezier(IPObjectStruct *PObj);
IPObjectStruct *TwoFreeformsProduct(IPObjectStruct *PObj1,
				    IPObjectStruct *PObj2);
IPObjectStruct *TwoFreeformsDotProduct(IPObjectStruct *PObj1,
				       IPObjectStruct *PObj2);
IPObjectStruct *TwoFreeformsCrossProduct(IPObjectStruct *PObj1,
					 IPObjectStruct *PObj2);
IPObjectStruct *TwoBasisInnerProduct(IPObjectStruct *PCrv,
				     IrtRType *RInt1,
				     IrtRType *RInt2);
IPObjectStruct *TwoFreeformsSum(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *TwoFreeformsDiff(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *ParallelSrfs2Shell(IPObjectStruct *Srf1Obj,
				   IPObjectStruct *Srf2Obj);
IPObjectStruct *CrvReparametrization(IPObjectStruct *Obj,
				     IrtRType *TMin,
				     IrtRType *TMax);
IPObjectStruct *SrfReparametrization(IPObjectStruct *Obj,
				     IrtRType *RDir,
				     IrtRType *TMin,
				     IrtRType *TMax);
IPObjectStruct *TrivReparametrization(IPObjectStruct *Obj,
				      IrtRType *RDir,
				      IrtRType *TMin,
				      IrtRType *TMax);
IPObjectStruct *MvarReparametrization(IPObjectStruct *Obj,
				      IrtRType *RDir,
				      IrtRType *TMin,
				      IrtRType *TMax);
IPObjectStruct *CrvCurvaturePts(IPObjectStruct *PObj,
				IrtRType *Eps,
				 IrtRType *Operation);
IPObjectStruct *CrvInflectionPts(IPObjectStruct *PObj,
				 IrtRType *Eps,
				 IrtRType *Operation);
IPObjectStruct *CrvCurvatureFunction(IPObjectStruct *PObj,
				     IrtRType *RSamples,
				     IrtRType *ROrder,
				     IrtRType *RArcLen);
IPObjectStruct *SrfRadialCurvature(IPObjectStruct *PObj,
				   IrtVecType ViewDir,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol,
				   IrtRType *MergeTol);
IPObjectStruct *ImplicitCrvExtreme(IPObjectStruct *PSrf,
				   IrtRType *Dir,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol);
IPObjectStruct *SrfGaussCurvature(IPObjectStruct *PObj, IrtRType *NumerOnly);
IPObjectStruct *SrfMeanCurvature(IPObjectStruct *PObj, IrtRType *NumerOnly);
IPObjectStruct *SrfCurvatureBounds(IPObjectStruct *PObj,
				   IrtRType *RPtType,
				   IrtRType *RDir);
IPObjectStruct *FreeformEvolute(IPObjectStruct *PObj);
IPObjectStruct *SrfIsoFocalSrf(IPObjectStruct *PObj, IrtRType *RDir);
IPObjectStruct *CrvZeros(IPObjectStruct *PObj, IrtRType *Eps, IrtRType *Axis);
IPObjectStruct *CrvExtremes(IPObjectStruct *PObj,
			    IrtRType *Eps,
			    IrtRType *Axis);
IPObjectStruct *CrvCrvInter(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *Eps,
			    IrtRType *SelfInter);
IPObjectStruct *CrvCrvInterArrangment(IPObjectStruct *PObj,
				      IrtRType *Eps,
				      IrtRType *Operation,
				      IPObjectStruct *PtObj);
IPObjectStruct *CrvCrvInterArrangment2(IPObjectStruct *PCrvs,
				       IrtRType *CAOperation,
				       IPObjectStruct *Params);
IPObjectStruct *ComputeBeltCurve(IPObjectStruct *Pulleys,
				 IrtRType *BeltThickness,
				 IrtRType *BoundingArcs,
				 IrtRType *ReturnCrvs);
IPObjectStruct *SrfSrfInter(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *Euclidean,
			    IrtRType *Eps,
			    IrtRType *AlignSrfs);
IPObjectStruct *MultiVarsInter(IPObjectStruct *PGeom,
			       IrtRType *SubdviTol,
			       IrtRType *NumerTol,
			       IrtRType *UserExprTree);
IPObjectStruct *MultiVarsContact(IPObjectStruct *PGeom1,
				 IPObjectStruct *PGeom2,
				 IPObjectStruct *MotionCrvs,
				 IrtRType *SubdivTol,
				 IrtRType *NumerTol,
				 IrtRType *UseExprTree);
IPObjectStruct *CrvPointDist(IPObjectStruct *PCrv,
			     IrtPtType Point,
			     IrtRType *MinDist,
			     IrtRType *Eps);
IPObjectStruct *CrvLineDist(IPObjectStruct *PCrv,
			    IrtPtType Point,
			    IrtVecType Vec,
			    IrtRType *MinDist,
			    IrtRType *Eps);
IPObjectStruct *SrfPointDist(IPObjectStruct *PSrf,
			     IrtPtType Point,
			     IrtRType *MinDist,
			     IrtRType *SubdivTol,
			     IrtRType *NumerTol);
IPObjectStruct *SrfLineDist(IPObjectStruct *PSrf,
			    IrtPtType LnPt,
			    IrtVecType LnDir,
			    IrtRType *MinDist,
			    IrtRType *SubdivTol,
			    IrtRType *NumerTol);
IPObjectStruct *CrvComposition(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *CrvDecomposition(IPObjectStruct *PObj);
IPObjectStruct *SrfsPrisa(IPObjectStruct *Srfs,
			  CagdRType *SamplesPerCurve,
			  CagdRType *Epsilon,
			  CagdRType *Dir,
			  CagdVType Space,
			  CagdRType *RDoCrossSecs);
IPObjectStruct *SrfAdapIsoCurves(IPObjectStruct *Srf,
				 CagdRType *GenIsos,
				 CagdRType *Dir,
				 CagdRType *Eps,
				 CagdRType *FullIsoOutputType,
				 CagdRType *SinglePathSmoothing,
				 IPObjectStruct *WeightPtScl);
IPObjectStruct *GetFreefromParamDomain(IPObjectStruct *FreeformObj);
IPObjectStruct *LineLeastSquarePtData(IPObjectStruct *PtObjList);
IPObjectStruct *PlaneLeastSquarePtData(IPObjectStruct *PtObjList);
IPObjectStruct *CrvLeastSquarePtData(IPObjectStruct *PtObjList,
				     CagdRType *Order,
				     CagdRType *CrvSize,
				     IPObjectStruct *Params,
				     CagdRType *RPeriodic);
IPObjectStruct *SrfLeastSquarePtData(IPObjectStruct *LstObjList,
				     CagdRType *UOrder,
				     CagdRType *VOrder,
				     CagdRType *USize,
				     CagdRType *VSize,
				     CagdRType *ParamType);
IrtRType GetResolutionReal(void);
int GetResolution(int ClipToMin);
IPObjectStruct *CurveMultiResDecomp(IPObjectStruct *CrvObj,
				    IrtRType *Discont,
				    IrtRType *LeastSqr);
IPObjectStruct *GenTrimmedSurface(IPObjectStruct *SrfObj,
				  IPObjectStruct *TrimmedCrvsObj,
				  IrtRType *RHasTopLvlTrim);
IPObjectStruct *GenTrimmedSurfaces2(IPObjectStruct *SrfObj,
				    IPObjectStruct *Cntrs);
IPObjectStruct *GetSrfFromTrimmedSrf(IPObjectStruct *TrimmedSrfObj);
IPObjectStruct *GetTrimCrvsFromTrimmedSrf(IPObjectStruct *TrimmedSrfObj,
					  IrtRType *RParamSpace);
IPObjectStruct *GenBezierTrivarObject(IPObjectStruct *LstLstObjList);
IPObjectStruct *GenBsplineTrivarObject(IrtRType *RUOrder,
				       IrtRType *RVOrder,
				       IrtRType *RWOrder,
				       IPObjectStruct *LstLstObjList,
				       IPObjectStruct *KntObjList);
IPObjectStruct *EvalTrivarObject(IPObjectStruct *TVObj,
				 IrtRType *u,
				 IrtRType *v,
				 IrtRType *w);
IPObjectStruct *SurfaceFromTrivar(IPObjectStruct *TVObj,
				  IrtRType *RDir,
				  IrtRType *ParamVal);
IPObjectStruct *SurfaceFromTrivMesh(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *RIndex);
IPObjectStruct *DivideTrivarObject(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *ParamVal);
IPObjectStruct *RegionFromTrivarObject(IPObjectStruct *TVObj, 
					IrtRType *RDir,
					IrtRType *ParamVal1,
					IrtRType *ParamVal2);
IPObjectStruct *RefineTrivarObject(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *RReplace,
				    IPObjectStruct *KnotsObj);
IPObjectStruct *DeriveTrivarObject(IPObjectStruct *TVObj, IrtRType *Dir);
IPObjectStruct *InterpolateTrivar(IPObjectStruct *PObj,
				  IrtRType *DULength,
				  IrtRType *DVLength,
				  IrtRType *DWLength,
				  IrtRType *DUOrder,
				  IrtRType *DVOrder,
				  IrtRType *DWOrder);
IPObjectStruct *ComputeCrvMoments(IPObjectStruct *CrvObj, IrtRType *RMoment);
IPObjectStruct *GenTVFromSrfsObject(IPObjectStruct *SrfList,
				    IrtRType *OtherOrder,
				    IrtRType *OtherEC);
IPObjectStruct *GenTVInterpolateSrfsObject(IPObjectStruct *SrfList,
					   IrtRType *OtherOrder,
					   IrtRType *OtherEC,
					   IrtRType *OtherParam);
IPObjectStruct *GenTVREVObject(IPObjectStruct *Cross);
IPObjectStruct *GenTVPREVObject(IPObjectStruct *Cross);
IPObjectStruct *TrivZeroJacobian(IPObjectStruct *Tv,
				 IrtRType *REuclid,
				 IrtRType *RSkipRate,
				 IPObjectStruct *RFineness);
IPObjectStruct *CrvKernel(IPObjectStruct *Crv,
			  IrtRType *RGamma,
			  IrtRType *REuclid,
			  IPObjectStruct *RFineness,
			  IrtRType *RSilh);
IPObjectStruct *SrfKernel(IPObjectStruct *Srf,
			  IrtRType *Fineness,
			  IrtRType *RSkipRate);
IPObjectStruct *CrvDiameter(IPObjectStruct *Crv,
			    IrtRType *RSubEps,
			    IrtRType *RNumEps,
			    IrtRType *RMinMaxAll);
IPObjectStruct *GenHermiteObject(IPObjectStruct *Pos1,
				 IPObjectStruct *Pos2,
				 IPObjectStruct *Dir1,
				 IPObjectStruct *Dir);
IPObjectStruct *GenBlendHermiteObject(IPObjectStruct *Pos1,
				      IPObjectStruct *Pos2,
				      IPObjectStruct *Dir1,
				      IPObjectStruct *Dir2,
				      IPObjectStruct *CrossSec,
				      IPObjectStruct *Normal);
IPObjectStruct *GenBlendHermiteOnSrfObject(IPObjectStruct *Srf,
					   IPObjectStruct *UVCrv,
					   IPObjectStruct *CrossSecShape,
					   IrtRType *TanScale,
					   IPObjectStruct *Width,
					   IPObjectStruct *Height);
IPObjectStruct *MatchTwoCurves(IPObjectStruct *PCrv1,
			       IPObjectStruct *PCrv2,
			       IrtRType *RReduce,
			       IrtRType *RSampleSet,
			       IrtRType *RReparamOrder,
			       IrtRType *RRotate,
			       IrtRType *RNorm);
IPObjectStruct *ContourFreeform(IPObjectStruct *PSrfObj,
				IrtPlnType Plane,
				IPObjectStruct *PSrfEvalObj,
				IPObjectStruct *PValidationObj);
IPObjectStruct *SurfaceRayIntersect(IPObjectStruct *PSrfObj,
				    IrtPtType ReyPos,
				    IrtVecType RayDir,
				    IrtRType *Tolerance);
IPObjectStruct *PolygonPtInclusion(IPObjectStruct *PObj, IrtPtType Pt);
IPObjectStruct *CurvePtInclusion(IPObjectStruct *PObj,
				 IrtPtType Pt,
				 IrtRType *Epsilon);
IPObjectStruct *PolygonRayIntersect(IPObjectStruct *PObj,
				    IrtPtType RayOrigin,
				    IrtRType *RayDir);
IPObjectStruct *FreeFormSplitScalar(IPObjectStruct *FreeForm);
IPObjectStruct *FreeFormMergeScalar(IPObjectStruct *ScalarFreeFormList,
				    IrtRType *RPType);
IPObjectStruct *FreeFormIrtPtType(IPObjectStruct *FreeForm);
IPObjectStruct *FreeFormGeomType(IPObjectStruct *FreeForm);
IPObjectStruct *ComputeConvexHull(IPObjectStruct *Geom, IrtRType *RFineNess);
IPObjectStruct *ComputeMinSpanCirc(IPObjectStruct *Geom,
				   IPObjectStruct *Tols);
IPObjectStruct *ComputeMinSpanSphere(IPObjectStruct *Geom);
IPObjectStruct *ComputeMinSpanCone(IPObjectStruct *Geom);
IPObjectStruct *CrvPointTangents(IPObjectStruct *Crv,
				 IrtPtType Pt,
				 IrtRType *RFineNess);
IPObjectStruct *CrvTwoTangents(IPObjectStruct *Crv, IrtRType *RFineNess);
IPObjectStruct *CircTangentsTwoCrvs(IPObjectStruct *Crv1,
				    IPObjectStruct *Crv2,
				    IrtRType *Radius,
				    IrtRType *Tol);
IPObjectStruct *SrfThreeTangents(IPObjectStruct *SrfList,
				 IrtRType *Orientation,
				 IrtRType *SubdivTol,
				 IrtRType *NumericTol);
IPObjectStruct *FreeFormOrder(IPObjectStruct *FreeForm);
IPObjectStruct *FreeFormMeshSize(IPObjectStruct *FreeForm);
IPObjectStruct *FreeFormKnotVector(IPObjectStruct *FreeForm);
IPObjectStruct *FreeFormControlPoints(IPObjectStruct *FreeForm);
IPObjectStruct *CurveVisibility(IPObjectStruct *CrvObj,
				IrtRType *Tol,
				IrtVecType View);
IPObjectStruct *VisibConeDecomposition(IPObjectStruct *SrfObj,
				       IrtRType *Resolution,
				       IrtRType *ConeSize);
IPObjectStruct *CrvAngleMap(IPObjectStruct *CrvObj,
			    IrtRType *Tolerance,
			    IrtRType *Angle,
			    IrtRType *RDiagExtreme);
IPObjectStruct *CrvViewMap(IPObjectStruct *CrvObj,
			   IPObjectStruct *ViewCrvObj,
			   IrtRType *SubTol,
			   IrtRType *NumTol,
			   IrtRType *TrimInvisible);
IPObjectStruct *PointCoverOfHemiSphere(IrtRType *Size);
IPObjectStruct *FreeformPointDistrib(IPObjectStruct *FreeFormObj,
				     IrtRType *RParamUniform,
				     IrtRType *RNumOfPts);
IPObjectStruct *GenBezierTriSrfObject(IrtRType *RLength,
				      IPObjectStruct *PtObjList);
IPObjectStruct *GenBsplineTriSrfObject(IrtRType *RLength,
				       IrtRType *ROrder,
				       IPObjectStruct *PtObjList,
				       IPObjectStruct *KntObjList);
IPObjectStruct *GenGregoryTriSrfObject(IrtRType *RLength,
				       IPObjectStruct *PtObjList);
IPObjectStruct *EvalTriSrfObject(IPObjectStruct *TriSrfObj,
				 IrtRType *u,
				 IrtRType *v,
				 IrtRType *w);
IPObjectStruct *NormalTriSrfObject(IPObjectStruct *TriSrfObj,
				   IrtRType *u,
				   IrtRType *v,
				   IrtRType *w);
IPObjectStruct *DeriveTriSrfObject(IPObjectStruct *TriSrfObj, IrtRType *Dir);
IPObjectStruct *CurveEnvelopeOffset(IPObjectStruct *CrvObj,
				    IrtRType *Height,
				    IrtRType *Tolerance);
IPObjectStruct *SphericalBisector(IPObjectStruct *Objs, IrtRType *Tolerance);
IPObjectStruct *PlanePointBisector(IrtPtType Pt, IrtRType *Size);
IPObjectStruct *CylinPointBisector(IrtPtType CylPt,
				   IrtVecType CylDir,
				   IrtRType *CylRad,
				   IrtPtType Pt,
				   IrtRType *Size);
IPObjectStruct *ConePointBisector(IrtPtType ConeApex,
				  IrtVecType ConeDir,
				  IrtRType *ConeAngle,
				  IrtPtType Pt,
				  IrtRType *Size);
IPObjectStruct *SpherePointBisector(IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtPtType Pt);
IPObjectStruct *TorusPointBisector(IrtPtType TrsCntr,
				   IrtVecType TrsDir,
				   IrtRType *TrsMajorRad,
				   IrtRType *TrsMinorRad,
				   IrtPtType Pt);
IPObjectStruct *PlaneLineBisector(IrtVecType LineDir, IrtRType *Size);
IPObjectStruct *ConeLineBisector(IrtVecType ConeDir,
				 IrtRType *ConeAngle,
				 IrtVecType LineDir,
				 IrtRType *Size);
IPObjectStruct *SphereLineBisector(IrtPtType SprCntr,
				   IrtRType *SprRad,
				   IrtRType *Size);
IPObjectStruct *SpherePlaneBisector(IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtRType *Size);
IPObjectStruct *CylinPlaneBisector(IrtPtType CylPt,
				   IrtVecType CylDir,
				   IrtRType *CylRad,
				   IrtRType *Size);
IPObjectStruct *ConePlaneBisector(IrtPtType ConeApex,
				  IrtVecType ConeDir,
				  IrtRType *ConeAngle,
				  IrtRType *Size);
IPObjectStruct *SphereSphereBisector(IrtPtType SprCntr1,
				     IrtRType *SprRad1,
				     IrtPtType SprCntr2,
				     IrtRType *SprRad2);
IPObjectStruct *CylinSphereBisector(IrtPtType CylPt,
				    IrtVecType CylDir,
				    IrtRType *CylRad,
				    IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtRType *Size);
IPObjectStruct *ConeConeBisector(IrtVecType Cone1Dir,
				 IrtRType *Cone1Angle,
				 IrtVecType Cone2Dir,
				 IrtRType *Cone2Angle,
				 IrtRType *Size);
IPObjectStruct *ConeConeBisector2(IrtPtType Cone1Pt,
				  IrtVecType Cone1Dir,
				  IrtRType *Cone1Angle,
				  IrtPtType Cyl2Pt,
				  IrtVecType Cyl2Dir,
				  IrtRType *Cyl2Rad);
IPObjectStruct *ConeSphereBisector(IrtPtType ConeApex,
				   IrtVecType ConeDir,
				   IrtRType *ConeAngle,
				   IrtPtType SprCntr,
				   IrtRType *SprRad,
				   IrtRType *Size);
IPObjectStruct *TorusSphereBisector(IrtPtType TrsCntr,
				    IrtVecType TrsDir,
				    IrtRType *TrsMajorRad,
				    IrtRType *TrsMinorRad,
				    IrtPtType SprCntr,
				    IrtRType *SprRad);
IPObjectStruct *CylinCylinBisector(IrtPtType Cyl1Pt,
				   IrtVecType Cyl1Dir,
				   IrtRType *Cyl1Rad,
				   IrtPtType Cyl2Pt,
				   IrtVecType Cyl2Dir,
				   IrtRType *Cyl2Rad);
IPObjectStruct *ConeCylinBisector(IrtPtType Cone1Pt,
				  IrtVecType Cone1Dir,
				  IrtRType *Cone1Angle,
				  IrtPtType Cyl2Pt,
				  IrtVecType Cyl2Dir,
				  IrtRType *Cyl2Rad);
IPObjectStruct *CurveBisectorSkel2D(IPObjectStruct *CrvObj,
				    IrtRType *RZeroSet,
				    IrtRType *RBisectFun,
				    IrtRType *Tolerance,
				    IrtRType *RNumerImprove,
				    IrtRType *RSameNormal);
IPObjectStruct *CurveBisectorSkel3D(IPObjectStruct *CrvObj,
				    IrtRType *RBisectFun);
IPObjectStruct *CurveAlphaSector(IPObjectStruct *CrvObj, IrtRType *Alpha);
IPObjectStruct *SurfaceBisectorSkel(IPObjectStruct *CrvObj, IrtPtType Pt);
IPObjectStruct *Skel2D2PrimsInter(IPObjectStruct *Obj1,
				  IPObjectStruct *Obj2,
				  IPObjectStruct *Obj3,
				  IrtRType *OutExtent,
				  IrtRType *Eps,
				  IrtRType *FineNess,
				  IPObjectStruct *MZeroTols);
IPObjectStruct *SkelNDPrimsInter(IPObjectStruct *ListMVs,
				 IrtRType *SubdivTol,
				 IrtRType *NumericTol,
				 IrtRType *UseExprTree);
IPObjectStruct *EvalSrfNormalsCone(IPObjectStruct *PSrf);
IPObjectStruct *FreeFormOrthotomic(IPObjectStruct *FreeForm,
				   IrtPtType Pt,
				   IrtRType *K);
IPObjectStruct *FreeFormSilhouette(IPObjectStruct *Srf,
				   IrtVecType VDir,
				   IrtRType *REuclidean);
IPObjectStruct *FreeFormBoundary(IPObjectStruct *PObj);
IPObjectStruct *FreeFormTopoAspectGraph(IPObjectStruct *PSrf);
IPObjectStruct *FreeFormSilhInflections(IPObjectStruct *PSrf,
					IrtVecType ViewDir,
					IrtRType *SubdivTol,
					IrtRType *NumerTol);
IPObjectStruct *FreeFormPolarSilhouette(IPObjectStruct *PSrf,
					IrtVecType VDir,
					IrtRType *REuclidean);
IPObjectStruct *FreeFormIsocline(IPObjectStruct *Srf,
				 IrtVecType VDir,
				 IrtRType *Theta,
				 IrtRType *REuclidean,
				 IrtRType *RMoreLessRgn);
IPObjectStruct *FreeFormPoles(IPObjectStruct *FreeForm);
IPObjectStruct *TrivIsoContourCurvature(IPObjectStruct *TrivObj,
					IrtPtType Pos,
					IrtRType *RCompute);
IPObjectStruct *MarchCubeVolume(IPObjectStruct *VolumeSpec,
				IrtPtType CubeDim,
				IrtRType *RSkipFactor,
				IrtRType *RIsoVal);
IPObjectStruct *PointCoverPolyObj(IPObjectStruct *PolyObj,
				  IrtRType *Rn,
				  IrtRType *Dir);
IPObjectStruct *IsoCoverTVObj(IPObjectStruct *TVObj,
			      IrtRType *RNumStrokes,
			      IrtRType *RStrokeType,
			      IrtVecType MinMaxPwrLen,
			      IrtRType *RStepSize,
			      IrtRType *RIsoVal,
			      IrtVecType ViewDir);
IPObjectStruct *LoadVolumeIntoTV(const char *FileName,
				 const IrtRType *RDataType,
				 IrtVecType VolSize,
				 IrtVecType Orders);
IPObjectStruct *DivideMultivarObject(IPObjectStruct *MVObj,
				     IrtRType *RDir,
				     IrtRType *ParamVal);
IPObjectStruct *RegionFromMultivarObject(IPObjectStruct *MVObj, 
					 IrtRType *RDir,
					 IrtRType *ParamVal1,
					 IrtRType *ParamVal2);
IPObjectStruct *RefineMultivarObject(IPObjectStruct *MVObj,
				     IrtRType *RDir,
				     IrtRType *RReplace,
				     IPObjectStruct *KnotsObj);
IPObjectStruct *DeriveMultivarObject(IPObjectStruct *MVObj, IrtRType *Dir);
IPObjectStruct *FreeFormDuality(IPObjectStruct *Obj);
IPObjectStruct *TwoCrvsAlgebraicSum(IPObjectStruct *Crv1Obj,
				    IPObjectStruct *Crv2Obj);
IPObjectStruct *TwoCrvsSwungAlgSum(IPObjectStruct *Crv1Obj,
				   IPObjectStruct *Crv2Obj);
IPObjectStruct *PromoteMultiVar(IPObjectStruct *MVObj, IPObjectStruct *PrmList);
IPObjectStruct *ReverseMultiVar(IPObjectStruct *MVObj,
				IrtRType *Axis1,
				IrtRType *Axis2);
IPObjectStruct *MultiVarFromMultiVar(IPObjectStruct *MVObj,
				     IrtRType *Dir,
				     IrtRType *t);
IPObjectStruct *MultiVarFromMVMesh(IPObjectStruct *MVObj,
				   IrtRType *Dir,
				   IrtRType *Index);
IPObjectStruct *EvalMultiVarObject(IPObjectStruct *MVObj,
				   IPObjectStruct *ParamObjs);
IPObjectStruct *EvalMultiVarZeros(IPObjectStruct *MVListObj,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol);
IPObjectStruct *EvalMultiVarUnivarZeros(IPObjectStruct *MVListObj,
					IrtRType *Step,
					IrtRType *SubdivTol,
					IrtRType *NumerTol);
IPObjectStruct *MergeMVMV(IPObjectStruct *MV1Obj,
			  IPObjectStruct *MV2Obj,
			  IrtRType *Dir,
			  IrtRType *RDiscont);
IPObjectStruct *MultivarBisector(IPObjectStruct *MV1Obj,
				 IPObjectStruct *MV2Obj,
				 IrtRType *ROutputType,
				 IrtRType *SubdivTol,
				 IrtRType *NumerTol);
IPObjectStruct *MultivarTrisector(IPObjectStruct *FF1Obj,
				  IPObjectStruct *FF2Obj,
				  IPObjectStruct *FF3Obj,
				  IrtRType *RStep,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtVecType BBoxMin,
				  IrtVecType BBoxMax);
IPObjectStruct *MultiVarPack3CircsInTri(const IrtRType *Pt1,
					const IrtRType *Pt2,
					const IrtRType *Pt3,
					const IrtRType *InteriorSolsOnly,
					const IrtRType *SubdivTol,
					const IrtRType *NumerTol);
IPObjectStruct *MultiVarPack6CircsInTri(const IrtRType *Pt1,
					const IrtRType *Pt2,
					const IrtRType *Pt3,
					const IrtRType *InteriorSolsOnly,
					const IrtRType *SubdivTol,
					const IrtRType *NumerTol);
IPObjectStruct *MultivarRayTrap(IPObjectStruct *ObjList,
				IrtRType *ROrient,
				IrtRType *SubdivTol,
				IrtRType *NumerTol,
				IrtRType *UseExprTree);
IPObjectStruct *SurfaceAccessibility(IPObjectStruct *PosSrf,
				     IPObjectStruct *OrientSrf,
				     IPObjectStruct *CheckSrf,
				     IPObjectStruct *AccessDir,
				     IrtRType *SubdivTol,
				     IrtRType *NumerTol);
IPObjectStruct *SurfaceFlecnodals(IPObjectStruct *Srf,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *MergeTol,
				  IrtRType *RContactOrder,
				  IrtRType *RUnivarMVSolver);
IPObjectStruct *SurfaceBiTangents(IPObjectStruct *Srf,
				  IrtRType *Orientation,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *MergeTol);
IPObjectStruct *ParabolicCurvesOnSrf(IPObjectStruct *PObjSrf,
				     IrtRType *Euclidean);
IPObjectStruct *SrfFundamentalForms(IPObjectStruct *PObjSrf,
				    IrtRType *FormNum);
IPObjectStruct *RuledRuledInter(IPObjectStruct *Srf1C1,
				IPObjectStruct *Srf1C2,
				IPObjectStruct *Srf2C1,
				IPObjectStruct *Srf2C2,
				IrtRType *Tolerance,
				IrtRType *RZeroSetFunc);
IPObjectStruct *RingRingInter(IPObjectStruct *Srf1C,
			      IPObjectStruct *Srf1r,
			      IPObjectStruct *Srf2C,
			      IPObjectStruct *Srf2r,
			      IrtRType *Tolerance,
			      IrtRType *RZeroSetFunc);
IPObjectStruct *CurvesCompare(IPObjectStruct *Crv1Obj,
			      IPObjectStruct *Crv2Obj,
			      IrtRType *Eps);
IPObjectStruct *GenConicSection(IPObjectStruct *ABCDEFObj,
				IrtRType *ZLevel,
				IrtRType *Distance,
				IrtRType *RCrvEval);
IPObjectStruct *GenEllipse3Points(IrtPtType Pt1,
				  IrtPtType Pt2,
				  IrtPtType Pt3,
				  IrtRType *Offset);
IPObjectStruct *GenQuadric(IPObjectStruct *ABCDEFGHIJObj);
IPObjectStruct *PromoteConicToQuadric(IPObjectStruct *ABCDEFObj, IrtRType *Z);
IPObjectStruct *TransformImplicit(IrtRType *Oper,
				  IPObjectStruct *ABCDEFGHIJObj,
				  IPObjectStruct *TransMat);
IPObjectStruct *WhatIsGeometryType(const IPObjectStruct *Geom,
				   IrtRType *RIsType,
				   const IrtRType *Eps);
IPObjectStruct *ComputeSmoothPolyNormals(IPObjectStruct *PObj,
					 IrtRType *MaxAngle);
IPObjectStruct *FixPolyNormals(IPObjectStruct *PObj, IrtRType *TrustPlNrml);
IPObjectStruct *FixPolyGeometry(IPObjectStruct *PObj,
				IrtRType *Op,
				IrtRType *Eps);
IPObjectStruct *BlossomEvaluation(IPObjectStruct *PObj,
				  IPObjectStruct *BlsmVals);
IPObjectStruct *CleanRefinedKnots(IPObjectStruct *CrvObj);
IPObjectStruct *RemoveKnots(IPObjectStruct *CrvObj,
			    IrtRType *Tolerance);
IPObjectStruct *ReflectionLines(IPObjectStruct *SrfObj,
				IrtVecType ViewDir,
				IPObjectStruct *Lines,
				IrtRType *Euclidean);
IPObjectStruct *CrvAreaIntergal(IPObjectStruct *CrvObj);
IPObjectStruct *SrfVolumeIntergal(IPObjectStruct *SrfObj,
				  IrtRType *Method,
				  IrtRType *Eval);
IPObjectStruct *SrfMomentIntergal(IPObjectStruct *SrfObj,
				  IrtRType *Moment,
				  IrtRType *Axis1,
				  IrtRType *Axis2,
				  IrtRType *Eval);
IPObjectStruct *RegisterPointSet(IPObjectStruct *PointsSet1,
				 IPObjectStruct *PointsSetSrf2,
				 IrtRType *AlphaConverge,
				 IrtRType *Tolerance);
IPObjectStruct *EvalDDMForSrf(IPObjectStruct *Srf,
			      IPObjectStruct *Texture,
			      IrtRType *DuDup,
			      IrtRType *DvDup,
			      IrtRType *LclUVs);
IPObjectStruct *AnalyticSrfFit(IPObjectStruct *UVPts,
			       IPObjectStruct *EucPts,
			       IrtRType *RFirstAtOrigin,
			       IrtRType *RFitDegree);
IPObjectStruct *PolyPlaneClipping(IPObjectStruct *Poly, IrtPlnType Plane);
IPObjectStruct *EvalCurveCurvature(IPObjectStruct *PCrv, IrtRType *t);
IPObjectStruct *EvalSurfaceCurvature(IPObjectStruct *PSrf,
				     IrtRType *U,
				     IrtRType *V,
				     IrtRType *REuclidean);
IPObjectStruct *EvalSurfaceAsympDir(IPObjectStruct *PSrf,
				    IrtRType *U,
				    IrtRType *V,
				    IrtRType *REuclidean);
IPObjectStruct *CrvArcLenApprox(IPObjectStruct *PCrv,
				IrtRType *Fineness,
				IrtRType *ROrder);
IPObjectStruct *TextWarpThroughSrf(const IPObjectStruct *PSrf,
				   const char *Txt,
				   const IrtRType *HSpace,
				   const IrtRType *VBase,
				   const IrtRType *VTop,
				   const IrtRType *RLigatures);
IPObjectStruct *BezierRayClipping(IrtPtType RayPt,
				  IrtVecType RayDir,
				  IPObjectStruct *Srf);
IPObjectStruct *GenPowerMVObject(IPObjectStruct *LensLstObj,
				 IPObjectStruct *PtLstObj);
IPObjectStruct *GenBezierMVObject(IPObjectStruct *LensLstObj,
				  IPObjectStruct *PtLstObj);
IPObjectStruct *GenBsplineMVObject(IPObjectStruct *LensLstObj,
				   IPObjectStruct *OrdersLstObj,
				   IPObjectStruct *PtLstObj,
				   IPObjectStruct *KVLstObj);
IPObjectStruct *SrfUmbilicPts(IPObjectStruct *PSrfObj,
			      IrtRType *SubTol,
			      IrtRType *NumTol);
IPObjectStruct *CrvBiArcApprox(IPObjectStruct *PCrv,
			       IrtRType *Tolerance,
			       IrtRType *MaxAngle);
IPObjectStruct *DistanceTwoFreeforms(IPObjectStruct *PObj1,
				     IPObjectStruct *PObj2,
				     IrtRType *RDistType);
IPObjectStruct *FreeformCompareUptoRigidScale2D(IPObjectStruct *PObj1,
						IPObjectStruct *PObj2,
						CagdRType *Eps);
IPObjectStruct *ApproxCrvAsQuadratics(IPObjectStruct *Crv,
				      IrtRType *Tol,
				      IrtRType *MaxLen);
IPObjectStruct *ApproxCrvAsCubics(IPObjectStruct *Crv,
				  IrtRType *Tol,
				  IrtRType *MaxLen);
IPObjectStruct *ComputeVoronoiCell(IPObjectStruct *CrvObj);
IPObjectStruct *BspCrvFitting(IPObjectStruct *PtObjList, 
                              IPObjectStruct *InitInfo,
                              IrtRType *FitType,
                              IPObjectStruct *Constants);
IPObjectStruct *FreeformAntipodalPoints(IPObjectStruct *FF,
					IrtRType *SubdivTol,
					IrtRType *NumerTol);
IPObjectStruct *FreeformSelfInter(IPObjectStruct *FF,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *NrmlDeviation,
				  IrtRType *Euclidean);
IPObjectStruct *PointsToPolys(IPObjectStruct *PtsList, IrtRType *MergeTol);
IPObjectStruct *CrvCrvtrByOneCtlPt(IPObjectStruct *PCrv,
				   IrtRType *CtlPtIdx,
				   IrtRType *Min,
				   IrtRType *Max,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol,
				   IrtRType *Operation);
IPObjectStruct *MinimalDistance(IPObjectStruct *PObj1,
				IPObjectStruct *PObj2,
				IrtRType *Eps);
IPObjectStruct *HausdorffDistance(IPObjectStruct *PObj1,
				  IPObjectStruct *PObj2,
				  IrtRType *Eps,
				  IrtRType *ROneSided);
IPObjectStruct *SrfSrfInter2(IPObjectStruct *PObjSrf1,
			     IPObjectStruct *PObjSrf2,
			     IrtRType *Step,
			     IrtRType *SubdivTol,
			     IrtRType *NumerTol,
			     IrtRType *Euclidean);
IPObjectStruct *NCContourPath(IPObjectStruct *PObj,
			      IrtRType *Offset,
			      IrtRType *ZBaseLevel,
			      IrtRType *TPathSpace,
			      IrtRType *RUnits);
IPObjectStruct *NCPocketPath(IPObjectStruct *PObj,
			     IrtRType *ToolRadius,
			     IrtRType *RoughOffset,
			     IrtRType *TPathSpace,
			     IrtRType *TPathJoin,
			     IrtRType *RUnits,
			     IrtRType *TrimSelfInters);

IPObjectStruct *Model3DFrom2Images(const char *Image1Name,
				   const char *Image2Name,
				   IrtRType *RDoTexture,
				   const IPObjectStruct *Blob,
				   IrtRType *RBlobSpread,
				   IrtRType *RBlobColor,
				   IrtRType *RResolution,
				   IrtRType *RNegative,
				   IrtRType *Intensity,
				   IrtRType *MinIntensity,
				   IrtRType *RMergePolys);
IPObjectStruct *Model3DFrom3Images(const char *Image1Name,
				   const char *Image2Name,
				   const char *Image3Name,
				   IrtRType *RDoTexture,
				   const IPObjectStruct *Blob,
				   IrtRType *RBlobSpread,
				   IrtRType *RBlobColor,
				   IrtRType *RResolution,
				   IrtRType *RNegative,
				   IrtRType *Intensity,
				   IrtRType *MinIntensity,
				   IrtRType *RMergePolys);
IPObjectStruct *MicroBlobsDither3DFrom2Images(const char *Image1Name,
					      const char *Image2Name,
					      IrtRType *RDitherSize,
					      IrtRType *RMatchWidth,
					      IrtRType *RNegate,
					      IrtRType *RAugmentContrast,
					      IrtRType *RSpreadMethod,
					      IrtRType *SphereRad);
IPObjectStruct *MicroBlobsDither3DFrom3Images(const char *Image1Name,
					      const char *Image2Name,
					      const char *Image3Name,
					      IrtRType *RDitherSize,
					      IrtRType *RMatchWidth,
					      IrtRType *RNegate,
					      IrtRType *RAugmentContrast,
					      IrtRType *RSpreadMethod,
					      IrtRType *SphereRad);
IPObjectStruct *RuledSrfFit(IPObjectStruct *SrfObj,
			    IrtRType *RDir,
			    IrtRType *ExtndDmn,
			    IrtRType *Samples);
IPObjectStruct *CrvOrthoProjOnSrf(IPObjectStruct *CrvObj,
				  IPObjectStruct *SrfObj,
				  IrtRType *Tol,
				  IrtRType *Euclid);
IPObjectStruct *Crvs2RectRegionGeneral(IPObjectStruct *CrvList,
				       IrtRType *RAngularDeviations,
				       IrtRType *RCurveOutputType,
				       IrtRType *RSizeRectangle,
				       IrtRType *RNumSmoothingSteps);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* FREEFORM_H */
