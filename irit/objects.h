/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   General, visible to others, definitions of Objects module.		     *
*****************************************************************************/

/*****************************************************************************
*   Prototype for visible function in the Objects module.		     *
*****************************************************************************/

#ifndef	OBJECTS_H
#define	OBJECTS_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void SetUpPredefObjects(void);
void SetObjectAttrColor(IPObjectStruct *PObj, IrtRType *RColor);
void SetObjectAttrWidth(IPObjectStruct *PObj, IrtRType *RWidth);
void SetObjectAttrDWidth(IPObjectStruct *PObj, IrtRType *RDWidth);
void SetObjectAttrib(IPObjectStruct *PObj,
		     const char *Name,
		     IPObjectStruct *Data);
void SetObjectAttribProp(IPObjectStruct *PObj,
			 const char *Name,
			 IPObjectStruct *Data);
void SetObject2VrtcsAttribProp(IPObjectStruct *PObj, const char *Name);
IPObjectStruct *GetObjectAttrib(IPObjectStruct *PObj, const char *Name);
void CopyObjectAllAttribs(IPObjectStruct *PDest, IPObjectStruct *PSrc);
void RemoveObjectAttrib(IPObjectStruct *PObj, const char *Name);
IPObjectStruct *SetPolyVrtxNormal(IPObjectStruct *PObj,
				  IrtRType *RVrtxID,
				  IPObjectStruct *Normal);
IPObjectStruct *SetGetPolyVrtxAttrib(IPObjectStruct *PObj,
				     IrtRType *RVrtxID,
				     const char *Name,
				     IPObjectStruct *Data);
int SetDumpLevel(int DumpLvl);
IPObjectStruct *MakeTextGeometry(const char *Str,
				 IrtVecType Spacing,
				 IrtRType *Scaling);
double ThisObjectIs(const char *Name);
IPObjectStruct *ReverseListHierarchy(IPObjectStruct *PObj);
void IritObjectPrintf(const char *CtlStr, IPObjectStruct *PObjLst);
void IritObjectPrintfSetFile(const char *FileName);
IPObjectStruct *EvalAnimationTime(IrtRType *RTime,
				  IPObjectStruct *Obj,
				  IrtRType *EvalMats);
double GetObjectSize(IPObjectStruct *ListObj);
double GetMeshSize(IPObjectStruct *ListObj, IrtRType *RDir);
IrtRType PolyCountPolys(IPObjectStruct *PObj);
IPObjectStruct *GetNilList(void);
IPObjectStruct *GetNthList(IPObjectStruct *ListObj, IrtRType *Rn);
IPObjectStruct *RefNthList(IPObjectStruct *ListObj, IrtRType *Rn);
IPObjectStruct *NthList(IPObjectStruct *ListObj, int n, int Ref);
void SetSubObjectName(IPObjectStruct *PListObj,
		      const IrtRType *RIndex,
		      const char *NewName);
IPObjectStruct *GetSubObjectName(IPObjectStruct *PListObj, IrtRType *RIndex);
void SnocList(IPObjectStruct *PObj, IPObjectStruct *ListObj);
IPObjectStruct *GetObjectCoord(IPObjectStruct *PObj, IrtRType *RIndex);
IPObjectStruct *ComputeBBOXObject(IPObjectStruct *PObj);
IPObjectStruct *GenerateInstance(const char *InstanceName,
				 IPObjectStruct *InstMat);
IPObjectStruct *ConvertPolysTriangles(IPObjectStruct *PObj, IrtRType *RRegular);
IPObjectStruct *LimitTrianglesEdgeLen(IPObjectStruct *PTris, IrtRType *RMaxLen);
IPObjectStruct *ConvertPointsToPolys(IPObjectStruct *PObj, IrtRType *MaxTol);
void FreeIritObject(IPObjectStruct *PObj);
void DeleteIritObject(IPObjectStruct *PObj, int Free);
void InsertIritObject(IPObjectStruct *PObj, int DelOld);
void InsertIritObjectLast(IPObjectStruct *PObj, int DelOld);
void PrintIritObject(IPObjectStruct *PObj);
char *GetObjectTypeAsString(IPObjectStruct *PObj);
void PrintIritGlblObjectList(void);
void PrintIritObjectList(IPObjectStruct *PObj);
IPObjectStruct *CoerceIritObjectTo(IPObjectStruct *PObj, IrtRType *RNewType);
void SaveObjectInFile(const char *FileName, IPObjectStruct *PObj);
IPObjectStruct *LoadObjectFromFile(const char *FileName);
int LoadSaveObjectParseError(const char **ErrorMsg);
IPObjectStruct *GenMatObjectPosDir(IrtPtType Pos,
				   IrtVecType Dir,
				   IrtVecType UpDir);
IPObjectStruct *GetMatTransDecomp(IPObjectStruct *Mat);
IPObjectStruct *GetMatTransDecomp2(IPObjectStruct *Mat);
IPObjectStruct *GetMatTransRecomp(IPObjectStruct *MatFactorObj);
IPObjectStruct *GenMatObjectGeneric(IPObjectStruct *LstObjList);
IPObjectStruct *GenMatProjectionMat(IrtPlnType Plane,
				    IrtVecType EyeDir,
				    IrtRType *EyeInfinity);
IPObjectStruct *GenMatReflectionMat(IrtPlnType Plane);

IPObjectStruct *GenBOXObject(IrtVecType Pt,
			     IrtRType *WidthX,
			     IrtRType *WidthY, IrtRType *WidthZ);
IPObjectStruct *GenGBOXObject(IrtVecType Pt,
			      IrtVecType Dir1,
			      IrtVecType Dir2,
			      IrtVecType Dir3);
IPObjectStruct *GenCONEObject(IrtVecType Pt,
			      IrtVecType Dir,
			      IrtRType *R,
			      IrtRType *Bases);
IPObjectStruct *GenCONE2Object(IrtVecType Pt,
			       IrtVecType Dir,
			       IrtRType *R1,
			       IrtRType *R2,
			       IrtRType *Bases);
IPObjectStruct *GenCYLINObject(IrtVecType Pt,
			       IrtVecType Dir,
			       IrtRType *R,
			       IrtRType *Bases);
IPObjectStruct *GenSPHEREObject(IrtVecType Center, IrtRType *R);
IPObjectStruct *GenTORUSObject(IrtVecType Center,
			       IrtVecType Normal,
			       IrtRType *Rmajor,
			       IrtRType *Rminor);
IPObjectStruct *GenPOLYDISKObject(IrtVecType N, IrtVecType T, IrtRType *R);
IPObjectStruct *GenPOLYGONObject(IPObjectStruct *PObjList,
				 IrtRType * RIsPolyline);
IPObjectStruct *GenObjectFromPolyList(IPObjectStruct *PObjList);
IPObjectStruct *GenObjectFromPolylineList(IPObjectStruct *PObjList,
					  IrtRType *Eps);
int InsertPolyToPoly(IPObjectStruct *PPoly, IPObjectStruct *PPolys);
IPObjectStruct *GenCROSSECObject(IPObjectStruct *PObj);
IPObjectStruct *GenSURFREVObject(IPObjectStruct *Cross);
IPObjectStruct *GenSURFREVAxisObject(IPObjectStruct *Cross,
				     IrtVecType Axis);
IPObjectStruct *GenSURFREV2Object(IPObjectStruct *Cross,
				  IrtRType *StartAngle,
				  IrtRType *EndAngle);
IPObjectStruct *GenSURFREV2AxisObject(IPObjectStruct *Cross,
				      IrtRType *StartAngle,
				      IrtRType *EndAngle,
				      IrtVecType Axis);
IPObjectStruct *GenEXTRUDEObject(IPObjectStruct *Cross,
				 IrtVecType Dir,
				 IrtRType *RBases);
IPObjectStruct *GenDecimatedObject(IPObjectStruct *PPolyObj,
				   IrtRType *RDecimType,
				   IrtRType *RThreshold);
IPObjectStruct *PolyCurvatureApprox(IPObjectStruct *PPolyObj,
				    IrtRType *RNumOfRings,
				    IrtRType *RCubicFit);
IPObjectStruct *PolyImportanceApprox(IPObjectStruct *PPolyObj,
				     IrtRType *RGetRange);
IPObjectStruct *TwoPolysMorphing(IPObjectStruct *Pl1,
				 IPObjectStruct *Pl2,
				 IrtRType *t);
IPObjectStruct *PolyPropFetch(IPObjectStruct *PObj,
			      IrtRType *RPropType,
			      IPObjectStruct *PropParam);
IPObjectStruct *FitPrim2PolyModel(IPObjectStruct *PObj,
				  IrtRType *FitType,
				  IrtRType *Tol,
				  IrtRType *NumIters);
IPObjectStruct *SetUVsToPolys(IPObjectStruct *PlObj,
			      IPObjectStruct *Scales,
			      IPObjectStruct *Trans);
IPObjectStruct *PolygonHolesObject(IPObjectStruct *RootObj,
				   IPObjectStruct *IslandObjs);
IPObjectStruct *SplitLnkList2ObjList(IPObjectStruct *LnkListObj);
IPObjectStruct *MinSetCover(IPObjectStruct *Set, IrtRType *Tol);
IPObjectStruct *UniformVectorsonSphere(IrtRType *n);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* OBJECTS_H */

