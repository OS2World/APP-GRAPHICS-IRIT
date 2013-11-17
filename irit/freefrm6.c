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

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "bool_lib.h"
#include "objects.h"
#include "mrchcube.h"
#include "mdl_lib.h"
#include "freeform.h"

#define CONIC_CRVS_ERROR	0.1       /* Error above 10% is purged away. */

static CagdRType *ComputeInterMidPoint(CagdCrvStruct *Crv1,
				       CagdRType t1,
				       CagdCrvStruct *Crv2,
				       CagdRType t2,
				       CagdRType *Dists);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the K-orthotomics of freeform curves and surfaces.              M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm: A curve or a surface to compute its K-orthotomic.              M
*   Pt:	      The points to which the K-orthotomic is computed for FreeForm. M
*   K:        The magnitude of the orthotomic function.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The K-orthotomic of the freeform relative to point P  M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormOrthotomic                                                       M
*****************************************************************************/
IPObjectStruct *FreeFormOrthotomic(IPObjectStruct *FreeForm,
				   IrtPtType Pt,
				   IrtRType *K)
{
    IPObjectStruct
	*PObj = NULL;

    if (IP_IS_CRV_OBJ(FreeForm)) {
	PObj = IPGenCRVObject(SymbCrvOrthotomic(FreeForm -> U.Crvs, Pt, *K));
    }
    else if (IP_IS_SRF_OBJ(FreeForm)) {
	PObj = IPGenSRFObject(SymbSrfOrthotomic(FreeForm -> U.Srfs, Pt, *K));
    }
    else {
	IRIT_FATAL_ERROR("FFPoles: Invalid freeform to check for poles!");
	return NULL;
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the silhouettes of a freeform surface or a polygonal	     M
* approximation of such.					             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         An object to compute its silhouettes.	                     M
*   VDir:         The viewing direction (orthographic).		             M
*   REuclidean:   If TRUE, returns the silhouettes in Euclidean space.       M
*		  Otherwise, the silhouette edges are returned in the        M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The silhouettes of surface Srf from VDir.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormSilhouette                                                       M
*****************************************************************************/
IPObjectStruct *FreeFormSilhouette(IPObjectStruct *PObj,
				   IrtVecType VDir,
				   IrtRType *REuclidean)
{
    int Euclidean = IRIT_REAL_PTR_TO_INT(REuclidean),
	Resolution = GetResolution(FALSE);
    IrtRType
        RelResolution = AttrGetObjectRealAttrib(PObj, "resolution");

    if (Resolution < MIN_FREE_FORM_RES)
        Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
        Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

    if (IP_IS_SRF_OBJ(PObj)) {
        IPPolygonStruct
	    *PPoly = SymbSrfSilhouette(PObj -> U.Srfs, VDir,
				       Resolution, Euclidean);

	if (PPoly != NULL)
	    return IPGenPOLYLINEObject(PPoly);
    }
    else if (IP_IS_POLY_OBJ(PObj)) {
	IrtHmgnMatType VMat, VMatInv;

	GMGenMatrixZ2Dir(VMat, VDir);

	/* Compute the inverse. */
	MatTranspMatrix(VMat, VMatInv);

        BoolGenAdjacencies(PObj);

        return GMSilExtractSilDirect(PObj, VMatInv);
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts the boundary of the given geometry.  If a polygonal model,      M
* extracts the boundary (polygonswith no neighbor). If a surface, returns    M
* the boundary curves of the (trimmed) surface.  If a trivar, returns the    M
* six faces of the trivar.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         An object to compute its boundary.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The boundary.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormBoundary                                                         M
*****************************************************************************/
IPObjectStruct *FreeFormBoundary(IPObjectStruct *PObj)
{
    if (IP_IS_SRF_OBJ(PObj)) {
        int i;
        CagdCrvStruct
	    **Bnrdies = CagdBndryCrvsFromSrf(PObj -> U.Srfs);
	IPObjectStruct
	    *ListObj = IPGenLISTObject(NULL);

	for (i = 0; i < 4; i++)
	    IPListObjectInsert(ListObj, i, IPGenCRVObject(Bnrdies[i]));

	IPListObjectInsert(ListObj, i, NULL);
	return ListObj;
    }
    else if (IP_IS_TRIMSRF_OBJ(PObj)) {
        IrtRType
	    RParamSpace = 0.0;

        return GetTrimCrvsFromTrimmedSrf(PObj, &RParamSpace);
    }
    else if (IP_IS_POLY_OBJ(PObj)) {
	const char *Str;
	IrtRType UMin, VMin, UMax, VMax;

	if ((Str = AttrGetObjectStrAttrib(PObj, "SrfBoundary")) &&
	    (sscanf(Str, "%lf %lf %lf %lf", &UMin, &VMin, &UMax, &VMax) == 4 ||
	     sscanf(Str, "%lf,%lf,%lf,%lf", &UMin, &VMin, &UMax, &VMax) == 4))
	    BoolGenAdjSetSrfBoundaries(UMin, VMin, UMax, VMax);

        BoolGenAdjacencies(PObj);

	BoolGenAdjSetSrfBoundaries(0.0, 0.0, 0.0, 0.0);

        return GMSilExtractBndry(PObj);
    }
    else if (IP_IS_TRIVAR_OBJ(PObj)) {
        int i;
        CagdSrfStruct
	    **Bnrdies = TrivBndrySrfsFromTV(PObj -> U.Trivars);
	IPObjectStruct
	    *ListObj = IPGenLISTObject(NULL);

	for (i = 0; i < 6; i++)
	    IPListObjectInsert(ListObj, i, IPGenSRFObject(Bnrdies[i]));

	IPListObjectInsert(ListObj, i, NULL);
	return ListObj;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the topological aspect graph of a freeform surface.             M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:       A surface to compute its topological aspect graph on the     M
*		unit sphere, S^2.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The topological aspect graph of surface Srf on S^2,   M
*		as piecewise linear approximation on S^2.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormTopoAspectGraph                                                  M
*****************************************************************************/
IPObjectStruct *FreeFormTopoAspectGraph(IPObjectStruct *PSrf)
{
    int Resolution = GetResolution(FALSE);
    IrtRType
        RelResolution = AttrGetObjectRealAttrib(PSrf, "resolution");
    IPPolygonStruct *PPoly;

    if (Resolution < MIN_FREE_FORM_RES)
        Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
        Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

    PPoly = UserSrfTopoAspectGraph(PSrf -> U.Srfs, Resolution);

    if (PPoly != NULL)
        return IPGenPOLYLINEObject(PPoly);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the higher order silhouette contact points.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:        A surface to compute its higher order contact points.	     M
*   ViewDir:	 The direction of the view.				     M
*   SubdivTol:   Tolerance of the subdivision stage.                         M
*   NumerTol:    Tolerance of the numerical improvement stage.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The list of higher order contact points, in UV space. M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormSilhInflections                                                  M
*****************************************************************************/
IPObjectStruct *FreeFormSilhInflections(IPObjectStruct *PSrf,
					IrtVecType ViewDir,
					IrtRType *SubdivTol,
					IrtRType *NumerTol)
{
    int i = 0;
    IrtRType
	R = 0.0;
    MvarPtStruct *Pt,
	*Pts = MvarSrfSilhInflections(PSrf -> U.Srfs, ViewDir,
				      *SubdivTol, *NumerTol);
    IPObjectStruct *ObjPtList;

    if (Pts == NULL)
	return NULL;

    ObjPtList = IPGenLISTObject(NULL);

    /* Silhouette points of higher contact order on PSrf. */
    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	IPListObjectInsert(ObjPtList, i++,
			   IPGenPTObject(&Pt -> Pt[0], &Pt -> Pt[1], &R));
    }
    IPListObjectInsert(ObjPtList, i, NULL);

    MvarPtFreeList(Pts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the silhouettes of a freeform surface.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:         A surface to compute its silhouettes.	                     M
*   VDir:         The viewing direction (orthographic).		             M
*   REuclidean:   If TRUE, returns the silhouettes in Euclidean space.       M
*		  Otherwise, the silhouette edges are returned in the        M
*		  Parametric domain.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The silhouettes of surface Srf from VDir.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormPolarSilhouette                                                  M
*****************************************************************************/
IPObjectStruct *FreeFormPolarSilhouette(IPObjectStruct *PSrf,
					IrtVecType VDir,
					IrtRType *REuclidean)
{
    int Euclidean = IRIT_REAL_PTR_TO_INT(REuclidean),
	Resolution = GetResolution(FALSE);
    IrtRType
        RelResolution = AttrGetObjectRealAttrib(PSrf, "resolution");
    IPPolygonStruct *PPoly;

    if (Resolution < MIN_FREE_FORM_RES)
        Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
        Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

    PPoly = SymbSrfPolarSilhouette(PSrf -> U.Srfs, VDir,
				   Resolution, Euclidean);

    if (PPoly != NULL)
        return IPGenPOLYLINEObject(PPoly);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the isoclines of a freeform surface.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:         A surface to compute its isoclines.		             M
*   VDir:         The viewing direction (orthographic).		             M
*   Theta:        The fixed angle between the viewing direction and the      M
*		  surface normal, in degrees.				     M
*		  An angle of 90 degrees yields the silhouettes.	     M
*   REuclidean:   If TRUE, returns the isoclines in Euclidean space.         M
*		  Otherwise, the isocline edges are returned in the          M
*		  Parametric domain.					     M
*   RMoreLessRgn: +1 (-1) to extract only the surface regions of more (less) M
*		  angular deviation between normal and VDir.		     M
*                 -2 to also add the ruled regions as relief angle srfs.     M
*		  0 to just extract the isocline curves.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The isoclines of surface Srf from VDir.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormIsocline                                                         M
*****************************************************************************/
IPObjectStruct *FreeFormIsocline(IPObjectStruct *PSrf,
				 IrtVecType VDir,
				 IrtRType *Theta,
				 IrtRType *REuclidean,
				 IrtRType *RMoreLessRgn)
{
    int Euclidean = IRIT_REAL_PTR_TO_INT(REuclidean),
	MoreLessRgn = IRIT_REAL_PTR_TO_INT(RMoreLessRgn),
	Resolution = GetResolution(FALSE);
    IrtRType
        RelResolution = AttrGetObjectRealAttrib(PSrf, "resolution");
    IPPolygonStruct *PPoly;

    if (Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

    if (MoreLessRgn == 0) {
	PPoly = SymbSrfIsocline(PSrf -> U.Srfs, VDir, *Theta,
				Resolution, Euclidean);

	if (PPoly != NULL)
	    return IPGenPOLYLINEObject(PPoly);
	else
	    return NULL;
    }
    else {
        int i = 0;
	TrimSrfStruct *TSrf,
	    *TSrfs = UserMoldReliefAngle2Srf(PSrf -> U.Srfs, VDir, *Theta,
					     MoreLessRgn > 0, Resolution);
	IPObjectStruct
	    *RetObj = IPGenLISTObject(NULL);

	while (TSrfs != NULL) {
	    IRIT_LIST_POP(TSrf, TSrfs);

	    IPListObjectInsert(RetObj, i++, IPGenTRIMSRFObject(TSrf));
	}
	IPListObjectInsert(RetObj, i, NULL);

	if (MoreLessRgn == -2) {
	    CagdSrfStruct *Srf,
		*RuledSrfs = UserMoldRuledRelief2Srf(PSrf -> U.Srfs, VDir,
						     *Theta, Resolution);
	    IPObjectStruct
	        *RetObj2 = IPGenLISTObject(NULL),
	        *RetObj3 = IPGenLISTObject(RetObj);

	    /* Insert the ruled surfaces in. */
	    i = 0;
	    while (RuledSrfs != NULL) {
	        IRIT_LIST_POP(Srf, RuledSrfs);

		IPListObjectInsert(RetObj2, i++, IPGenSRFObject(Srf));
	    }
	    IPListObjectInsert(RetObj2, i, NULL);

	    /* And build the new returned object -  a list of lists... */
	    IPListObjectInsert(RetObj3, 1, RetObj2);
	    IPListObjectInsert(RetObj3, 2, NULL);

	    RetObj = RetObj3;
	}

	return RetObj;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if freeform has poles.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To check for poles.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  TRUE if FreeForm has poles, FALSE otherwise.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormPoles	                                                     M
*****************************************************************************/
IPObjectStruct *FreeFormPoles(IPObjectStruct *FreeForm)
{
    int Length;
    CagdRType **Points;

    if (IP_IS_CRV_OBJ(FreeForm)) {
	Points = FreeForm -> U.Crvs -> Points;
	Length = FreeForm -> U.Crvs -> Length;
    }
    else if (IP_IS_SRF_OBJ(FreeForm)) {
	Points = FreeForm -> U.Srfs -> Points;
	Length = FreeForm -> U.Srfs -> ULength * FreeForm -> U.Srfs -> VLength;
    }
    else if (IP_IS_TRIVAR_OBJ(FreeForm)) {
	Points = FreeForm -> U.Trivars -> Points;
	Length = FreeForm -> U.Trivars -> ULength *
	         FreeForm -> U.Trivars -> VLength *
	         FreeForm -> U.Trivars -> WLength;
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeForm)) {
	Points = FreeForm -> U.TrimSrfs -> Srf -> Points;
	Length = FreeForm -> U.TrimSrfs -> Srf -> ULength *
	         FreeForm -> U.TrimSrfs -> Srf -> VLength;
    }
    else if (IP_IS_TRISRF_OBJ(FreeForm)) {
	Points = FreeForm -> U.TriSrfs -> Points;
	Length = TRNG_TRISRF_MESH_SIZE(FreeForm -> U.TriSrfs);
    }
    else {
	IRIT_FATAL_ERROR("FFPoles: Invalid freeform to check for poles!");
	return NULL;
    }

    return IPGenNUMValObject(CagdPointsHasPoles(Points, Length));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes curvature properties of an iso surface at a given position Pos  M
* for a scalar triavariate function.					     M
* This function is geared toward numerous evaluations.  Hence, an invokation M
* with:									     M
*    Compute == -1 :  Would initialize auxiliary differential vector fields. V
*    Compute == 0  :  Would release all auxiliary data structures.           V
*    Compute == 1  :  Would compute the gradient of the trivariate, at Pos.  V
*    Compute == 2  :  Would compute the Hessian of the trivariate, at Pos.   V
*    Compute == 3  :  Would compute the principal curvatures/directions of   V
*		      the trivariate, at Pos.			             V
*                                                                            *
* PARAMETERS:                                                                M
*   TrivObj:       To compute differential/curvature properties for.         M
*   Pos:	   Where to evaluate the differentail/curvature properties.  M
*   RCompute:      What to preprocess/compute/free.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  TRUE if Compute == -1 or Compute == 0 and successful. M
*               The gradient vector if Compute == 1. The Hessian (list of    M
*		three vectors) if Compute == 2.  A list with two principal   M
*		curvatures and two principal directions if Computer == 3.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivIsoContourCurvature                                                  M
*****************************************************************************/
IPObjectStruct *TrivIsoContourCurvature(IPObjectStruct *TrivObj,
					IrtPtType Pos,
					IrtRType *RCompute)
{
    int i,
	Compute = IRIT_REAL_PTR_TO_INT(RCompute);
    CagdRType PCurv1, PCurv2;
    CagdVType PDir1, PDir2, Gradient, Hessian[3];
    IPObjectStruct *PObjList;

    switch (Compute) {
	case -1:
            if (!IPGenNUMValObject(TrivEvalTVCurvaturePrelude(TrivObj -> U.Trivars))) {
	        IRIT_WNDW_PUT_STR("Failed to initialized trivariate's auxiliary differential fields.");
		return NULL;
	    }
	    return IPGenNUMValObject(TRUE);
	case 0:
	    TrivEvalTVCurvaturePostlude();
	    return IPGenNUMValObject(TRUE);
	case 1:
	    if (!TrivEvalGradient(Pos, Gradient)) {
	        IRIT_WNDW_PUT_STR("Failed to compute gradient (not initialized!?).");

	        return NULL;
	    }
	    return IPGenVECObject(&Gradient[0], &Gradient[1], &Gradient[2]);
	case 2:
	    if (!TrivEvalHessian(Pos, Hessian)) {
	        IRIT_WNDW_PUT_STR("Failed to compute Hessian (not initialized!?).");
	        return NULL;
	    }

	    PObjList = IPGenLISTObject(NULL);

	    for (i = 0; i < 3; i++)
		IPListObjectInsert(PObjList, i,
				   IPGenVECObject(&Hessian[i][0],
						  &Hessian[i][1],
						  &Hessian[i][2]));
	    IPListObjectInsert(PObjList, 3, NULL);
	    return PObjList;
	case 3:
	    if (!TrivEvalCurvature(Pos, &PCurv1, &PCurv2, PDir1, PDir2)) {
	        IRIT_WNDW_PUT_STR("Failed to compute Principal curvatures/directions (not initialized!?).");
	        return NULL;
	    }

	    PObjList = IPGenLISTObject(NULL);

	    IPListObjectInsert(PObjList, 0, IPGenNUMValObject(PCurv1));
	    IPListObjectInsert(PObjList, 1,
			      IPGenVECObject(&PDir1[0], &PDir1[1], &PDir1[2]));
	    IPListObjectInsert(PObjList, 2, IPGenNUMValObject(PCurv2));
	    IPListObjectInsert(PObjList, 3,
			      IPGenVECObject(&PDir2[0], &PDir2[1], &PDir2[2]));
	    IPListObjectInsert(PObjList, 4, NULL);
	    return PObjList;
	default:
	    IRIT_WNDW_PUT_STR("Wrong curvature property to evaluate.\n");
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute a polygonal iso surface approximation at iso value IsoVal to the M
* given volume specification.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   VolumeSpec:   Can be a list of one of the following forms:               M
*		1. "list( list( ImageName1, ..., ImageNameN) )".             M
*		2. "list(Trivar, Axis, Sampling Factor, Normals)" - here the M
8		   volume is a trivariate and the iso surface should be      M
*		   computed with respect to the prescribed Axis (1 for X, 2  M
*		   for Y, etc.), Sampling rate control, and whether or not   M
*		   normals are to be estimated as well.			     M
*		3. "list(FileName, DataType, Width, Height, Depth)" - here   M
*		   the volume is read from the given file of given three     M
*		   dimensions.  Each scalar value in the file can be an      M
*		   ascii string (DataType = 1), 2 bytes integer (2), 4 bytes M
*		   integer (4), 4 bytes float (5), and 8 bytes double (6).   M
*   CubeDim:    Width, height, and depth of a single cube, in object space   M
*		coordinates.						     M
*   RSkipFactor: Typically 1.  For 2, only every second sample is considered M
*		and for i, only every i'th sample is considered, in all axes.M
*   RIsoVal:    At which to extract the iso-surface.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A polygonal approximation of the iso-surface at IsoVal M
*		computed using the marching cubes algorithm.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCThresholdCube, MCExtractIsoSurface, MCExtractIsoSurface2               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MarchCubeVolume                                                          M
*****************************************************************************/
IPObjectStruct *MarchCubeVolume(IPObjectStruct *VolumeSpec,
				IrtPtType CubeDim,
				IrtRType *RSkipFactor,
				IrtRType *RIsoVal)
{
    IPObjectStruct *PObj1, *PObj2, *PObj3, *PObj4, *PObj5,
	*RetVal = NULL;

    switch (IPListObjectLength(VolumeSpec)) {
	case 1:
	    PObj1 = IPListObjectGet(VolumeSpec, 0);
	    if (PObj1 && IP_IS_OLST_OBJ(PObj1)) {
	        RetVal = MCExtractIsoSurface3(PObj1,
					      CubeDim,
					      IRIT_REAL_PTR_TO_INT(RSkipFactor),
					      *RIsoVal);
	    }
	    else
	        IRIT_NON_FATAL_ERROR("Failed to march cube the volume (wrong input!?).");
	    break;
	case 4:
	    PObj1 = IPListObjectGet(VolumeSpec, 0);
	    PObj2 = IPListObjectGet(VolumeSpec, 1);
	    PObj3 = IPListObjectGet(VolumeSpec, 2);
	    PObj4 = IPListObjectGet(VolumeSpec, 3);
	    if (PObj1 && IP_IS_TRIVAR_OBJ(PObj1) &&
		PObj2 && IP_IS_NUM_OBJ(PObj2) &&
		PObj3 && IP_IS_NUM_OBJ(PObj3) &&
		PObj4 && IP_IS_NUM_OBJ(PObj4)) {
	        RetVal = MCExtractIsoSurface2(PObj1 -> U.Trivars,
					      IRIT_REAL_TO_INT(PObj2 -> U.R),
					      IRIT_REAL_TO_INT(PObj4 -> U.R),
					      CubeDim,
					      IRIT_REAL_PTR_TO_INT(RSkipFactor),
					      PObj3 -> U.R,
					      *RIsoVal);
	    }
	    else
	        IRIT_NON_FATAL_ERROR("Failed to march cube the volume (wrong input!?).");
	    break;
	case 5:
	    PObj1 = IPListObjectGet(VolumeSpec, 0);
	    PObj2 = IPListObjectGet(VolumeSpec, 1);
	    PObj3 = IPListObjectGet(VolumeSpec, 2);
	    PObj4 = IPListObjectGet(VolumeSpec, 3);
	    PObj5 = IPListObjectGet(VolumeSpec, 4);
	    if (PObj1 && IP_IS_STR_OBJ(PObj1) &&
		PObj2 && IP_IS_NUM_OBJ(PObj2) &&
		PObj3 && IP_IS_NUM_OBJ(PObj3) &&
		PObj4 && IP_IS_NUM_OBJ(PObj4) &&
		PObj5 && IP_IS_NUM_OBJ(PObj5)) {
	        RetVal = MCExtractIsoSurface(PObj1 -> U.Str,
					     IRIT_REAL_TO_INT(PObj2 -> U.R),
					     CubeDim,
					     IRIT_REAL_TO_INT(PObj3 -> U.R),
					     IRIT_REAL_TO_INT(PObj4 -> U.R),
					     IRIT_REAL_TO_INT(PObj5 -> U.R),
					     IRIT_REAL_PTR_TO_INT(RSkipFactor),
					     *RIsoVal);
	    }
	    else
	        IRIT_NON_FATAL_ERROR("Failed to march cube the volume (wrong input!?).");
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Expecting either 1, 4 or 5 entities in volume spec.");
	    break;
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a uniform point coverage of a polygonal model.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyObj:   To compute a point coverage for.                              M
*   Rn:        Estimated number of points desired.                           M
*   Dir:       A given direction to compute distribution from, or a zero     M
*	       vector for direction independent Euclidean measure.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A set of points covering PolyObj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PointCoverPolyObj                                                        M
*****************************************************************************/
IPObjectStruct *PointCoverPolyObj(IPObjectStruct *PolyObj,
				  IrtRType *Rn,
				  IrtRType *Dir)
{
    if (IRIT_PT_APX_EQ_ZERO_EPS(Dir, IRIT_EPS))
	Dir = NULL;

    return GMPointCoverOfPolyObj(PolyObj, IRIT_REAL_PTR_TO_INT(Rn), Dir, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes curve coverage of a trivariate model, for specific iso surface. M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:         To compute a curve based coverage for.                    M
*   RNumStrokes:   Number of strokes to handle and generate.                 M
*   RStrokeType:   A two bits pattern:  Bit zero controls the drawing of     M
*		   a stroke in the minimal curvature's direction.  Bit one   M
*		   the drawing of a stroke in maximal curvature's direction. M
*   MinMaxPwrLen:  Arc length of each stroke (randomized in between).	     M
*		   a triplet of the form (Min, Max, Power) that determines   M
*		   the length of each stroke as,			     M
*			Avg = (Max + Min) / 2,      Dev = (Max - Min) / 2    V
*		        Length = Avg + Dev * Random(0, 1)^ Pwr		     V
*   RStepSize:	   Steps to take in the piecewise linear approximation.      M
*   RIsoVal:       Of iso surface of TV that coverage is to be computed for. M
*   ViewDir:	   Direction of view, used for silhouette computation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A set of points covering PolyObj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IsoCoverTVObj                                                            M
*****************************************************************************/
IPObjectStruct *IsoCoverTVObj(IPObjectStruct *TVObj,
			      IrtRType *RNumStrokes,
			      IrtRType *RStrokeType,
			      IrtVecType MinMaxPwrLen,
			      IrtRType *RStepSize,
			      IrtRType *RIsoVal,
			      IrtVecType ViewDir)
{
    return IPGenCRVObject(
	   TrivCoverIsoSurfaceUsingStrokes(TVObj -> U.Trivars,
					   IRIT_REAL_PTR_TO_INT(RNumStrokes),
					   IRIT_REAL_PTR_TO_INT(RStrokeType),
					   MinMaxPwrLen,
					   *RStepSize,
					   *RIsoVal,
					   ViewDir));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Loads a volumetric data set as a trivariate function of prescribed       M
* orders.  Uniform open end conditions are created for it.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:    To load the trivariate data set from.                       M
*   RDataType:   Type of scalar value in volume data file.  Can be one of:   M
*		   1 - Regular ascii (separated by while spaces.             M
*		   2 - Two bytes short integer.				     M
*		   3 - Four bytes long integer.				     M
*		   4 - One byte (char) integer.				     M
*		   5 - Four bytes float.				     M
*		   6 - Eight bytes double.				     M
*   VolSize:     Dimensions of trivariate voluem.                            M
*   Orders:      Orders of the three axis of the volume (in U, V, and W).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A trivariate, or NULL if error.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   LoadVolumeIntoTV                                                         M
*****************************************************************************/
IPObjectStruct *LoadVolumeIntoTV(const char *FileName,
				 const IrtRType *RDataType,
				 IrtVecType VolSize,
				 IrtVecType Orders)
{
    TrivTVStruct
	*TV = TrivLoadVolumeIntoTV(FileName,
				   IRIT_REAL_PTR_TO_INT(RDataType),
				   VolSize,
				   Orders);

    return TV == NULL ? NULL : IPGenTRIVARObject(TV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to subdivide a multivariate function into two in specified         M
* direction and specified parameter value.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:        Multivariate to subdivide.                                 M
*   RDir:         Direction of subdivision.                                  M
*   ParamVal:     Parameter value at which subdivision should occur.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object of two multivar objects, result of the  M
*		       subdivision.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DivideMultivarObject                                                     M
*****************************************************************************/
IPObjectStruct *DivideMultivarObject(IPObjectStruct *MVObj,
				     IrtRType *RDir,
				     IrtRType *ParamVal)
{
    MvarMVDirType
	Dir = (MvarMVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    MvarMVStruct
	*MV = MvarMVSubdivAtParam(MVObj -> U.MultiVars, *ParamVal, Dir);
    IPObjectStruct *MV1, *MV2, *MVList;

    if (MV == NULL)
	return NULL;

    MV1 = IPGenMULTIVARObject(MV);
    IP_ATTR_SAFECOPY_ATTRS(MV1 -> Attr, MVObj -> Attr);
    MV2 = IPGenMULTIVARObject(MV -> Pnext);
    IP_ATTR_SAFECOPY_ATTRS(MV2 -> Attr, MVObj -> Attr);

    MV -> Pnext = NULL;

    MVList = IPGenLISTObject(MV1);
    IPListObjectInsert(MVList, 1, MV2);
    IPListObjectInsert(MVList, 2, NULL);

    return MVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a region of a multivariate in specified direction       M
* and specified parameter values.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:       Multivariate to extract a region from.                      M
*   RDir:        Direction of region extraction.			     M
*   ParamVal1:   Parameter of beginning of region.                           M
*   ParamVal2:   Parameter of end of region.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A region of MVObj,                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   RegionFromMultivarObject                                                 M
*****************************************************************************/
IPObjectStruct *RegionFromMultivarObject(IPObjectStruct *MVObj, 
					 IrtRType *RDir,
					 IrtRType *ParamVal1,
					 IrtRType *ParamVal2)
{
    MvarMVDirType
	Dir = (MvarMVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    MvarMVStruct *MV;

    if (Dir >= MVObj -> U.MultiVars -> Dim || Dir < 0) {
	IRIT_NON_FATAL_ERROR("Region extraction direction out of domain.");
	return NULL;
    }

    if ((MV = MvarMVRegionFromMV(MVObj -> U.MultiVars,
				 *ParamVal1, *ParamVal2, Dir)) == NULL)
	return NULL;

    MVObj = IPGenMULTIVARObject(MV);

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to refine a multivariate function in specified direction and       M
* knot vector.								     M
*   If, however, Replace is non zero, KnotsObj REPLACES current vector.      M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:      Multivariate to refine in direction RDir.                    M
*   RDir:       Direction of refinement.				     M
*   RReplace:   If TRUE KnotsObj will replace the RDir knot vector of MVObj. M
*		Otherwise, the knots in KnotsObj will be added to it.	     M
*   KnotsObj:   A list of knots.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A refined multivar, or a multivar with a replaced    M
*                       knot vector.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   RefineMultivarObject                                                     M
*****************************************************************************/
IPObjectStruct *RefineMultivarObject(IPObjectStruct *MVObj,
				     IrtRType *RDir,
				     IrtRType *RReplace,
				     IPObjectStruct *KnotsObj)
{
    int n,
	Replace = IRIT_REAL_PTR_TO_INT(RReplace);
    MvarMVDirType
	Dir = (MvarMVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    char *ErrStr;
    CagdRType *t;
    MvarMVStruct *RefMV;
    IPObjectStruct *RefMVObj;

    if (Dir >= MVObj -> U.MultiVars -> Dim || Dir < 0) {
	IRIT_NON_FATAL_ERROR("Refining direction out of domain.");
	return NULL;
    }

    if ((t = GetKnotVector(KnotsObj, 0, &n, &ErrStr, FALSE)) == NULL) {
	IPFreeObject(MVObj);
	IRIT_NON_FATAL_ERROR2("REFINE: %s, empty object result.\n", ErrStr);
	return NULL;
    }
    RefMV = MvarMVRefineAtParams(MVObj -> U.MultiVars, Dir, Replace, t, n);
    IritFree(t);
    if (RefMV == NULL)
	return NULL;

    RefMVObj = IPGenMULTIVARObject(RefMV);
    IP_ATTR_SAFECOPY_ATTRS(RefMVObj -> Attr, MVObj -> Attr);

    return RefMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to differentiate a multivariate function in Dir.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:     Multivar to differentiate.                                    M
*   Dir:       Direction of differentiation. Either U or V or W.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A differentiated multivar.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   DeriveMultivarObject                                                     M
*****************************************************************************/
IPObjectStruct *DeriveMultivarObject(IPObjectStruct *MVObj, IrtRType *Dir)
{
    MvarMVStruct *DerivMV;
    IPObjectStruct *DerivMVObj;

    if (IRIT_REAL_PTR_TO_INT(Dir) >= MVObj -> U.MultiVars -> Dim ||
	IRIT_REAL_PTR_TO_INT(Dir) < 0) {
	IRIT_NON_FATAL_ERROR("Differentiation direction out of domain.");
	return NULL;
    }

    DerivMV = MvarMVDerive(MVObj -> U.MultiVars,
				(MvarMVDirType) IRIT_REAL_PTR_TO_INT(Dir));
    DerivMVObj = IPGenMULTIVARObject(DerivMV);

    return DerivMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compues the dual of the given object.  Duality is between (tangent)      M
* lines/planes and points in R3/R2.                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:         To compute its dual.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The dual object.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormDuality                                                          M
*****************************************************************************/
IPObjectStruct *FreeFormDuality(IPObjectStruct *Obj)
{
    if (IP_IS_SRF_OBJ(Obj))
	return IPGenSRFObject(SymbSrfDual(Obj -> U.Srfs));
    else if (IP_IS_CRV_OBJ(Obj))
	return IPGenCRVObject(SymbCrvDual(Obj -> U.Crvs));
    else {
	IRIT_NON_FATAL_ERROR("Expecting a curve or a surface.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds up algebraically the given two curves, C1(r) and C2(t).  The result M
* is a bivariate surface S(r, t) = C1(r) + C2(t).                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Obj, Crv2Obj:    Two curves to add.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A surface represent their sum.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoCrvsAlgebraicSum                                                      M
*****************************************************************************/
IPObjectStruct *TwoCrvsAlgebraicSum(IPObjectStruct *Crv1Obj,
				    IPObjectStruct *Crv2Obj)
{
    CagdSrfStruct
	*Srf = SymbAlgebraicSumSrf(Crv1Obj -> U.Crvs, Crv2Obj -> U.Crvs);

    return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds up algebraically the given two curves, C1(r) and C2(t).  The result M
* is a bivariate surface S(r, t) = C1(r) + C2(t).                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Obj, Crv2Obj:    Two curves to add.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A surface represent their sum.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoCrvsSwungAlgSum                                                       M
*****************************************************************************/
IPObjectStruct *TwoCrvsSwungAlgSum(IPObjectStruct *Crv1Obj,
				   IPObjectStruct *Crv2Obj)
{
    CagdSrfStruct
	*Srf = SymbSwungAlgSumSrf(Crv1Obj -> U.Crvs, Crv2Obj -> U.Crvs);

    return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to promote a multivariate function in Dir.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:     Multivar to promote.		                             M
*   PrmList:   Parameters' list - either one Dir parameter or two parameters M
*	       holding (StartAxis, NewDim) of promoted multivariate.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A promoted multivar.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   PromoteMultiVar                                                          M
*****************************************************************************/
IPObjectStruct *PromoteMultiVar(IPObjectStruct *MVObj, IPObjectStruct *PrmList)
{
    int IPrm1, IPrm2;
    MvarMVStruct *ProMV;
    IPObjectStruct
	*Prm1Obj = IPListObjectGet(PrmList, 0),
	*Prm2Obj = IPListObjectGet(PrmList, 1);

    if (Prm1Obj == NULL || !IP_IS_NUM_OBJ(Prm1Obj)) {
	IRIT_WNDW_PUT_STR("Expected a numeric first parameter in list.\n");
	return NULL;
    }
    else
	IPrm1 = IRIT_REAL_TO_INT(Prm1Obj -> U.R);

    if (Prm2Obj == NULL) {
	ProMV = MvarPromoteMVToMV(MVObj -> U.MultiVars,
				  (MvarMVDirType) IPrm1);
    }
    else {
	if (!IP_IS_NUM_OBJ(Prm2Obj)) {
	    IRIT_WNDW_PUT_STR("Expected a numeric second parameter in list.\n");
	    return NULL;
	}
	else
	    IPrm2 = IRIT_REAL_TO_INT(Prm2Obj -> U.R);

	ProMV = MvarPromoteMVToMV2(MVObj -> U.MultiVars,
				   (MvarMVDirType) IPrm1, IPrm2);
    }

    return IPGenMULTIVARObject(ProMV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to reverse two directions in a multivariate function.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:         Multivar to reverse.	                                     M
*   Axis1, Axis2:  Two axis to flip around.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A reversed multivar.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   ReverseMultiVar                                                          M
*****************************************************************************/
IPObjectStruct *ReverseMultiVar(IPObjectStruct *MVObj,
				IrtRType *Axis1,
				IrtRType *Axis2)
{
    MvarMVStruct
	*RevMV = MvarMVReverse(MVObj -> U.MultiVars,
				(MvarMVDirType) IRIT_REAL_PTR_TO_INT(Axis1),
				(MvarMVDirType) IRIT_REAL_PTR_TO_INT(Axis2));
    IPObjectStruct
	*RevMVObj = IPGenMULTIVARObject(RevMV);

    return RevMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a subspace MV from the given MV in direction DIR and    M
* parameter value of t.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:     Multivar to extract a subspace from.                          M
*   Dir:       Direction of extraction the sub space.			     M
*   t:	       Parameter value at which to extract the subspace from MV.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An extracted multivar of one less dimension.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarFromMultiVar                                                     M
*****************************************************************************/
IPObjectStruct *MultiVarFromMultiVar(IPObjectStruct *MVObj,
				     IrtRType *Dir,
				     IrtRType *t)
{
    MvarMVStruct
	*SubMV = MvarMVFromMV(MVObj -> U.MultiVars, *t,
			      (MvarMVDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*SubMVObj = IPGenMULTIVARObject(SubMV);

    return SubMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a multivariate function in Dir from the given MV by     M
* extraction one layer out of the original MV's mesh.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:     Multivar to extract a subspace from.                          M
*   Dir:       Direction of extraction the sub space.			     M
*   Index:     Mesh index at which to extract the subspace from MV.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An extracted multivar of one less dimension.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarFromMVMesh                                                       M
*****************************************************************************/
IPObjectStruct *MultiVarFromMVMesh(IPObjectStruct *MVObj,
				   IrtRType *Dir,
				   IrtRType *Index)
{
    MvarMVStruct
	*SubMV = MvarMVFromMesh(MVObj -> U.MultiVars,
				  IRIT_REAL_PTR_TO_INT(Index),
				  (MvarMVDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*SubMVObj = IPGenMULTIVARObject(SubMV);

    return SubMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate a multivariate function.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   MVObj:       Multivar to evaluate.	                                     M
*   ParamObjs:   Parameter values to evaluate at.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Evaluated point.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalMultiVarObject                                                       M
*****************************************************************************/
IPObjectStruct *EvalMultiVarObject(IPObjectStruct *MVObj,
				   IPObjectStruct *ParamObjs)
{
    int i = 0;
    MvarMVStruct
	*MV = MVObj -> U.MultiVars;
    CagdRType *Pt,
	*Params = (CagdRType *) IritMalloc(sizeof(CagdRType) * MV -> Dim);
    IPObjectStruct *PrmObj, *CtlPtObj;

    while ((PrmObj = IPListObjectGet(ParamObjs, i)) != NULL && i < MV -> Dim) {
	if (!IP_IS_NUM_OBJ(PrmObj)) {
	    IRIT_WNDW_PUT_STR("Non numeric object found in list");
	    return NULL;
	}

	Params[i++] = PrmObj -> U.R;
    }

    Pt = MvarMVEval(MV, Params);
    IritFree(Params);

    CtlPtObj = IPGenCTLPTObject((CagdPointType) MV -> PType, Pt);

    return CtlPtObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to find the simultaneous zeros of several multivariates.  The      M
* number of multivariates cannot be larger than the dimension of the MVs.    M
*                                                                            *
* PARAMETERS:                                                                M
*   MVListObj:    Multivariate to find their simultaneous zeros.             M
*   SubdivTol:    Tolerance of the subdivision stage.                        M
*   NumerTol:     Tolerance of the numerical improvement stage.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of (control) points on the simultaneous zero   M
*		       set.              				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalMultiVarZeros                                                        M
*****************************************************************************/
IPObjectStruct *EvalMultiVarZeros(IPObjectStruct *MVListObj,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol)
{
    int i, l;
    MvarMVStruct
	**MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *)
					     * MVListObj -> U.Lst.ListMaxLen);
    MvarConstraintType *Constraints;
    MvarPtStruct *MVPts, *MVPt;
    IPObjectStruct *PObj,
	*ObjPtList = IPGenLISTObject(NULL);
    CagdPointType PType;

    for (l = 0; (PObj = IPListObjectGet(MVListObj, l)) != NULL; l++) {
	if (!IP_IS_MVAR_OBJ(PObj)) {
	    IRIT_WNDW_PUT_STR("Only multivariates expected in list");
	    IritFree(MVs);
	    return NULL;
	}
	else
	    MVs[l] = PObj -> U.MultiVars;
    }

    Constraints = IritMalloc(sizeof(MvarConstraintType) * l);
    for (i = 0; i < l; i++)
	Constraints[i] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, l, *SubdivTol, *NumerTol);

    IritFree(Constraints);

    if (MVPts == NULL) {
	IritFree(MVs);
	IPListObjectInsert(ObjPtList, 0, NULL);
	return ObjPtList;
    }

    /* Convert the MVPts to regular control points. */
    PType = CAGD_MAKE_PT_TYPE(FALSE, IRIT_MIN(MVs[0] -> Dim,
			      CAGD_MAX_PT_COORD));

    /* Points on bisector sheet are returned. */
    for (MVPt = MVPts, i = 0; MVPt != NULL; MVPt = MVPt -> Pnext) {
	IPListObjectInsert(ObjPtList, i++,
			   PObj = IPGenCTLPTObject(PType, &MVPt -> Pt[-1]));
	IP_ATTR_SAFECOPY_ATTRS(PObj -> Attr, MVPt -> Attr);
    }
    IPListObjectInsert(ObjPtList, i, NULL);

    MvarPtFreeList(MVPts);
    IritFree(MVs);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to find the simultaneous zeros of several multivariates.  The      M
* number of multivariates must be one smaller than the dimension of the MVs, M
* yielding a univariate solution space.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVListObj:    Multivariate to find their simultaneous zeros.             M
*   Step:         Tolerance for the univariate curve tracing.		     M
*   SubdivTol:    Tolerance of the subdivision stage.                        M
*   NumerTol:     Tolerance of the numerical improvement stage.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of polylines on the simultaneous zero set.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalMultiVarZeros                                                        M
*****************************************************************************/
IPObjectStruct *EvalMultiVarUnivarZeros(IPObjectStruct *MVListObj,
					IrtRType *Step,
					IrtRType *SubdivTol,
					IrtRType *NumerTol)
{
    int l;
    MvarMVStruct
	**MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *)
					     * MVListObj -> U.Lst.ListMaxLen);
    MvarPolyStruct *MVPlls;
    IPObjectStruct *PObj, *ObjList;

    for (l = 0; (PObj = IPListObjectGet(MVListObj, l)) != NULL; l++) {
	if (!IP_IS_MVAR_OBJ(PObj)) {
	    IRIT_WNDW_PUT_STR("Only multivariates expected in list");
	    IritFree(MVs);
	    return NULL;
	}
	else
	    MVs[l] = PObj -> U.MultiVars;
    }

    MVPlls = MvarMVUnivarInter(MVs, *Step, *SubdivTol, *NumerTol);
    IritFree(MVs);

    if (MVPlls == NULL)
	return NULL;

    ObjList = MvarCnvrtMVPolysToIritPolys2(MVPlls, TRUE);
    MvarPolyFreeList(MVPlls);

    return ObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two multivariates along the prescribed direction.  The end of MV1   M
* is assumed to be identical to the beginning of mV2.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1Obj, MV2Obj:   Two objects to merge.                                  M
*   Dir:        Direction of extraction the sub space.			     M
*   RDiscont:   If TRUE, assumes the merged "edge" is discontinuous.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Merged multivariate.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MergeMVMV                                                                M
*****************************************************************************/
IPObjectStruct *MergeMVMV(IPObjectStruct *MV1Obj,
			  IPObjectStruct *MV2Obj,
			  IrtRType *Dir,
			  IrtRType *RDiscont)
{
    MvarMVStruct
	*MergedMV = MvarMergeMVMV(MV1Obj -> U.MultiVars, MV2Obj -> U.MultiVars,
				  (MvarMVDirType) IRIT_REAL_PTR_TO_INT(Dir),
				  IRIT_REAL_PTR_TO_INT(RDiscont));
    IPObjectStruct
	*MergedMVObj = IPGenMULTIVARObject(MergedMV);

    return MergedMVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector of two multivariates.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1Obj, MV2Obj:   Two multivariates to consider.                         M
*   ROutputType:      Expected output type:				     M
*		      1. For the computed multivariate constraints.	     M
*		      2. For the computed point cloud on the bisector.	     M
*		      3. Points in a form of (u1, v2, x, y, z) where         M
*			 (u1, v1) are the parameter space of the first       M
*			 surface.					     M
*		      4. Use marching cubes, if possible.		     M
*   SubdivTol:        Tolerance of first zero set finding subdivision stage. M
*   NumerTol:         Tolerance of second zero set finding numeric stage.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The bisector of the given two multivariates.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultivarBisector                                                         M
*****************************************************************************/
IPObjectStruct *MultivarBisector(IPObjectStruct *MV1Obj,
				 IPObjectStruct *MV2Obj,
				 IrtRType *ROutputType,
				 IrtRType *SubdivTol,
				 IrtRType *NumerTol)
{
    int OutputType = IRIT_REAL_PTR_TO_INT(ROutputType),
        MrchCubes = OutputType == 4;

    if (MrchCubes)
	OutputType = 1;			/* Get the multivariate constraints. */

    if (IRIT_APX_EQ_EPS(*SubdivTol, 0.0, IRIT_UEPS))
	return IPGenMULTIVARObject(MvarMVsBisector(MV1Obj -> U.MultiVars,
						   MV2Obj -> U.MultiVars));
    else {
	VoidPtr *RetVal;

	if (MV1Obj -> U.MultiVars -> Dim == 2 &&
	    MV2Obj -> U.MultiVars -> Dim == 2)
	    RetVal = MvarSrfSrfBisectorApprox(MV1Obj -> U.MultiVars,
					      MV2Obj -> U.MultiVars,
					      OutputType,
					      *SubdivTol, *NumerTol);
	else if (MV1Obj -> U.MultiVars -> Dim == 1 &&
		 MV2Obj -> U.MultiVars -> Dim == 2)
	    RetVal = MvarCrvSrfBisectorApprox(MV1Obj -> U.MultiVars,
					      MV2Obj -> U.MultiVars,
					      OutputType,
					      *SubdivTol, *NumerTol);
	else if (MV1Obj -> U.MultiVars -> Dim == 2 &&
		 MV2Obj -> U.MultiVars -> Dim == 1)
	    RetVal = MvarCrvSrfBisectorApprox(MV2Obj -> U.MultiVars,
					      MV1Obj -> U.MultiVars,
					      OutputType,
					      *SubdivTol, *NumerTol);
	else
	    RetVal = NULL;

	if (OutputType == 1) {
	    MvarMVStruct
		*Mv = (MvarMVStruct *) RetVal,
	        *MvEuclid = AttrGetPtrAttrib(Mv -> Attr, "MVEuclid");

	    AttrFreeOneAttribute(&Mv -> Attr, "MVEuclid");

	    if (MvEuclid != NULL && MrchCubes) {
		CagdRType Fineness[3];
	        TrivTVStruct
		    *Tv = MvarMVToTV(Mv),
		    *TvEuclid = MvarMVToTV(MvEuclid);
	        IPObjectStruct *PlObj;

		Fineness[0] = Fineness[1] = Fineness[2] = *SubdivTol;
		PlObj = UserTrivarZeros(Tv, TvEuclid,
					(int) *NumerTol, Fineness);

		/* A polygonal approximation of the zero is returned. */
		TrivTVFree(Tv);
		TrivTVFree(TvEuclid);
		MvarMVFree(Mv);
		MvarMVFree(MvEuclid);

		return PlObj;
	    }
	    else {
		/* The function whose zero set is the bisector is returned. */
	        if (MvEuclid)
		    MvarMVFree(MvEuclid);

		return IPGenMULTIVARObject(Mv);
	    }
	}
	else if (OutputType == 2) {
	    int i = 0;
	    MvarPtStruct *MVPt,
	        *MVPts = (MvarPtStruct *) RetVal;
	    IPObjectStruct
		*ObjPtList = IPGenLISTObject(NULL);

	    /* Points on bisector sheet are returned. */
	    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
		IPListObjectInsert(ObjPtList, i++,
				   IPGenPTObject(&MVPt -> Pt[0],
						 &MVPt -> Pt[1],
						 &MVPt -> Pt[2]));
	    }
	    IPListObjectInsert(ObjPtList, i, NULL);

	    MvarPtFreeList(MVPts);

	    if (i == 0) {
	        IPFreeObject(ObjPtList);
		ObjPtList = NULL;
	    }

	    return ObjPtList;
	}
	else {
	    int i = 0;
	    MvarPtStruct *MVPt,
	        *MVPts = (MvarPtStruct *) RetVal;
	    IPObjectStruct
		*ObjPtList = IPGenLISTObject(NULL);

	    /* Points on bisector sheet are returned. */
	    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
		IPListObjectInsert(ObjPtList, i++,
				   IPGenCTLPTObject(CAGD_PT_E5_TYPE,
						    &MVPt -> Pt[-1]));
	    }
	    IPListObjectInsert(ObjPtList, i, NULL);

	    MvarPtFreeList(MVPts);

	    if (i == 0) {
	        IPFreeObject(ObjPtList);
		ObjPtList = NULL;
	    }

	    return ObjPtList;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the trisector of three curves or surfaces.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   FF1Obj, FF2Obj, FF3Obj:   Three curves or surfaces to consider.          M
*   RStep:        Step size of numeric solver.				     M
*   SubdivTol:    Tolerance of first zero set finding subdivision stage.     M
*   NumerTol:     Tolerance of second zero set finding numeric stage.        M
*   BBoxMin, BBoxMax:  Domain to trace the trisector in.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The bisector of the given two multivariates.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultivarTrisector                                                        M
*****************************************************************************/
IPObjectStruct *MultivarTrisector(IPObjectStruct *FF1Obj,
				  IPObjectStruct *FF2Obj,
				  IPObjectStruct *FF3Obj,
				  IrtRType *RStep,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtVecType BBoxMin,
				  IrtVecType BBoxMax)
{
    MvarPolyStruct
        *Plls = MvarTrisectorCrvs(FF1Obj -> U.Crvs,   /* Srfs are fine too. */
				  FF2Obj -> U.Crvs,
				  FF3Obj -> U.Crvs,
				  *RStep, *SubdivTol, *NumerTol,
				  BBoxMin, BBoxMax);
    IPObjectStruct
        *PllsObj = MvarCnvrtMVPolysToIritPolys2(Plls, TRUE);

    MvarPolyFreeList(Plls);

    return PllsObj; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the (not actually!) maximal packing of 3 circles in a triangles.M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:  The 3 points defining the triangles in the XY plane.     M
*   InteriorSolsOnly:   TRUE for interior to triangle solutions only.	     M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of 3 circles packed inside the triangle, or  M
*                        NULL if error.                                      M
*			                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   Mvar3CircsInTriangles                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarPack3CircsInTri	                                             M
*****************************************************************************/
IPObjectStruct *MultiVarPack3CircsInTri(const IrtRType *Pt1,
					const IrtRType *Pt2,
					const IrtRType *Pt3,
					const IrtRType *InteriorSolsOnly,
					const IrtRType *SubdivTol,
					const IrtRType *NumerTol)
{
    int i;
    CagdRType Rad;
    const CagdPType Pts[3];
    CagdPtStruct Center;
    MvarPtStruct *MVPts, *MVPt;
    CagdCrvStruct *Circ;
    IPObjectStruct *PObjCirc, *PObjTriCircs, *PObjAllTriCircs;

    IRIT_PT_COPY(Pts[0], Pt1);
    IRIT_PT_COPY(Pts[1], Pt2);
    IRIT_PT_COPY(Pts[2], Pt3);

    if ((MVPts = Mvar3CircsInTriangles(Pts, *SubdivTol, *NumerTol)) == NULL)
        return NULL;

    Center.Pt[2] = 0.0;
    PObjAllTriCircs = IPGenLISTObject(NULL);

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        PObjTriCircs = IPGenLISTObject(NULL);

        for (i = 0; i < 9; i += 3) {
	    Center.Pt[0] = MVPt -> Pt[i];
	    Center.Pt[1] = MVPt -> Pt[i + 1];
	    Rad = MVPt -> Pt[i + 2];
	    Circ = BspCrvCreateCircle(&Center, Rad);

	    PObjCirc = IPGenCRVObject(Circ);
	    AttrSetObjectRealAttrib(PObjCirc, "CircX", Center.Pt[0]);
	    AttrSetObjectRealAttrib(PObjCirc, "CircY", Center.Pt[1]);
	    AttrSetObjectRealAttrib(PObjCirc, "CircR", Rad);

	    if (AttrGetIntAttrib(MVPt -> Attr, "InTriangle")) {
	        /* Inside the triangle. */
	        AttrSetObjectRGBColor(PObjCirc, 0, 255, 100);

		IPListObjectAppend(PObjTriCircs, PObjCirc);
	    }
	    else if (!IRIT_REAL_PTR_TO_INT(InteriorSolsOnly)) {
	        AttrSetObjectRGBColor(PObjCirc, 255, 0, 200);

		IPListObjectAppend(PObjTriCircs, PObjCirc);
	    }
	    else {
	        IPFreeObject(PObjCirc);
	        IPFreeObject(PObjTriCircs);
		PObjTriCircs = NULL;
		break;
	    }
	}

	if (PObjTriCircs != NULL)
	    IPListObjectAppend(PObjAllTriCircs, PObjTriCircs);
    }

    MvarPtFreeList(MVPts);

    return PObjAllTriCircs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the (not actually!) maximal packing of 6 circles in a triangles.M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:  The 3 points defining the triangles in the XY plane.     M
*   InteriorSolsOnly:   TRUE for interior to triangle solutions only.	     M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of 6 circles packed inside the triangle, or  M
*                        NULL if error.                                      M
*			                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   Mvar6CircsInTriangles                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarPack6CircsInTri	                                             M
*****************************************************************************/
IPObjectStruct *MultiVarPack6CircsInTri(const IrtRType *Pt1,
					const IrtRType *Pt2,
					const IrtRType *Pt3,
					const IrtRType *InteriorSolsOnly,
					const IrtRType *SubdivTol,
					const IrtRType *NumerTol)
{
    int i;
    CagdRType Rad;
    const CagdPType Pts[3];
    CagdPtStruct Center;
    MvarPtStruct *MVPts, *MVPt;
    CagdCrvStruct *Circ;
    IPObjectStruct *PObjCirc, *PObjTriCircs, *PObjAllTriCircs;

    IRIT_PT_COPY(Pts[0], Pt1);
    IRIT_PT_COPY(Pts[1], Pt2);
    IRIT_PT_COPY(Pts[2], Pt3);

    if ((MVPts = Mvar6CircsInTriangles(Pts, *SubdivTol, *NumerTol)) == NULL)
        return NULL;

    Center.Pt[2] = 0.0;
    PObjAllTriCircs = IPGenLISTObject(NULL);

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        PObjTriCircs = IPGenLISTObject(NULL);

        for (i = 0; i < 18; i += 3) {
	    Center.Pt[0] = MVPt -> Pt[i];
	    Center.Pt[1] = MVPt -> Pt[i + 1];
	    Rad = MVPt -> Pt[i + 2];
	    Circ = BspCrvCreateCircle(&Center, Rad);

	    PObjCirc = IPGenCRVObject(Circ);
	    AttrSetObjectRealAttrib(PObjCirc, "CircX", Center.Pt[0]);
	    AttrSetObjectRealAttrib(PObjCirc, "CircY", Center.Pt[1]);
	    AttrSetObjectRealAttrib(PObjCirc, "CircR", Rad);

	    if (AttrGetIntAttrib(MVPt -> Attr, "InTriangle")) {
	        /* Inside the triangle. */
	        AttrSetObjectRGBColor(PObjCirc, 0, 255, 100);

		IPListObjectAppend(PObjTriCircs, PObjCirc);
	    }
	    else if (!IRIT_REAL_PTR_TO_INT(InteriorSolsOnly)) {
	        AttrSetObjectRGBColor(PObjCirc, 255, 0, 200);

		IPListObjectAppend(PObjTriCircs, PObjCirc);
	    }
	    else {
	        IPFreeObject(PObjCirc);
	        IPFreeObject(PObjTriCircs);
		PObjTriCircs = NULL;
		break;
	    }
	}

	if (PObjTriCircs != NULL)
	    IPListObjectAppend(PObjAllTriCircs, PObjTriCircs);
    }

    MvarPtFreeList(MVPts);

    return PObjAllTriCircs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the light ray traps of n curves or surfaces.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjList:              A list of curves or surfaces.                      M
*   ROrient:              Apply orientation constaint as well?               M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*   UseExprTree:          TRUE to use expression trees, false regular MVs.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    List of locations of ray-traps as parameter values  M
*			of the curves.  Given n curves, returns a list of    M
*			control points in En.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultivarRayTrap                                                          M
*****************************************************************************/
IPObjectStruct *MultivarRayTrap(IPObjectStruct *ObjList,
				IrtRType *ROrient,
				IrtRType *SubdivTol,
				IrtRType *NumerTol,
				IrtRType *UseExprTree)
{
    int i;
    MvarPtStruct *MVPt, *MVPts;
    IPObjectStruct
        *PObj = IPListObjectGet(ObjList, 0),
	*ObjPtList = IPGenLISTObject(NULL);
    CagdPointType PType;

    if (PObj == NULL) {
        IRIT_WNDW_PUT_STR("Empty list of curves/surfaces detected.\n");
	return NULL;
    }

    if (IP_IS_CRV_OBJ(PObj)) {
        CagdCrvStruct
	    *LastCrv = NULL,
	    *Crvs = NULL;

        for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++) {
	    if (!IP_IS_CRV_OBJ(PObj)) {
	        IRIT_WNDW_PUT_STR("Only curves expected in list.\n");
		return NULL;
	    }
	}

	/* Chain all curves into one large linked list. */
	for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++) {
	    if (i == 0)
	        Crvs = LastCrv = PObj -> U.Crvs;
	    else {
	        LastCrv -> Pnext = PObj -> U.Crvs;
		LastCrv = PObj -> U.Crvs;
	    }
	}

	MVPts = MvarComputeRayTraps(Crvs, IRIT_REAL_PTR_TO_INT(ROrient),
				    *SubdivTol, *NumerTol,
				    IRIT_REAL_PTR_TO_INT(UseExprTree));

	/* separate all curves back into individual curves. */
	for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++)
	    PObj -> U.Crvs -> Pnext = NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
        CagdSrfStruct
	    *LastSrf = NULL,
	    *Srfs = NULL;

        for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++) {
	    if (!IP_IS_SRF_OBJ(PObj)) {
	        IRIT_WNDW_PUT_STR("Only surfaces expected in list.\n");
		return NULL;
	    }
	}

	/* Chain all surfaces into one large linked list. */
	for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++) {
	    if (i == 0)
	        Srfs = LastSrf = PObj -> U.Srfs;
	    else {
	        LastSrf -> Pnext = PObj -> U.Srfs;
		LastSrf = PObj -> U.Srfs;
	    }
	}

	MVPts = MvarComputeRayTraps3D(Srfs, IRIT_REAL_PTR_TO_INT(ROrient),
				      *SubdivTol, *NumerTol,
				      IRIT_REAL_PTR_TO_INT(UseExprTree));

	/* separate all surfaces back into individual surfaces. */
	for (i = 0; (PObj = IPListObjectGet(ObjList, i)) != NULL; i++)
	    PObj -> U.Srfs -> Pnext = NULL;
    }
    else {
        IRIT_WNDW_PUT_STR("Only curves or surfaces expected in list.\n");
	return NULL;
    }


    if (MVPts == NULL) {
	IPListObjectInsert(ObjPtList, 0, NULL);
	return ObjPtList;
    }

    /* Convert the solution to our form. */
    PType = CAGD_MAKE_PT_TYPE(FALSE, IRIT_MIN(MVPts -> Dim,
			      CAGD_MAX_PT_COORD));

    for (MVPt = MVPts, i = 0; MVPt != NULL; MVPt = MVPt -> Pnext) {
	IPListObjectInsert(ObjPtList, i++,
			   PObj = IPGenCTLPTObject(PType, &MVPt -> Pt[-1]));
	IP_ATTR_SAFECOPY_ATTRS(PObj -> Attr, MVPt -> Attr);
    }
    IPListObjectInsert(ObjPtList, i, NULL);

    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the light ray traps of n curves.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PosSrf:               Position surface to access.	                     M
*   OrientSrf:            Orientation field of access direction.             M
*   CheckSrf:             Surface to check against.                          M
*   AccessDir:		  Optional limit on a hemisphere of directions       M
*		          around AccessDir, if a vector object.		     M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of (u, v, s, t) locations of the boundary of    M
*			the accessible regions. (u, v) is the space of       M
*			PosSrf and OrientSrf, whereas (s, t) is of CheckSrf. M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceAccessibility                                                     M
*****************************************************************************/
IPObjectStruct *SurfaceAccessibility(IPObjectStruct *PosSrf,
				     IPObjectStruct *OrientSrf,
				     IPObjectStruct *CheckSrf,
				     IPObjectStruct *AccessDir,
				     IrtRType *SubdivTol,
				     IrtRType *NumerTol)
{
    int i;
    MvarPtStruct *MVPt, *MVPts;
    IPObjectStruct *PObj,
	*ObjPtList = IPGenLISTObject(NULL);
    CagdPointType PType;

    MVPts = MvarSrfAccessibility(PosSrf -> U.Srfs,
				 IP_IS_SRF_OBJ(OrientSrf) ? OrientSrf -> U.Srfs
							  : NULL,
				 CheckSrf -> U.Srfs,
				 IP_IS_VEC_OBJ(AccessDir) ? AccessDir -> U.Vec
							  : NULL,
				 *SubdivTol, *NumerTol);

    if (MVPts == NULL)
	return ObjPtList;

    /* Convert the solution to our form. */
    PType = CAGD_MAKE_PT_TYPE(FALSE, IRIT_MIN(MVPts -> Dim,
			      CAGD_MAX_PT_COORD));

    /* Points on boundary of the accessible regions on PosSrf are returned. */
    for (MVPt = MVPts, i = 0; MVPt != NULL; MVPt = MVPt -> Pnext) {
	IPListObjectInsert(ObjPtList, i++,
			   PObj = IPGenCTLPTObject(PType, &MVPt -> Pt[-1]));
	IP_ATTR_SAFECOPY_ATTRS(PObj -> Attr, MVPt -> Attr);
    }
    IPListObjectInsert(ObjPtList, i, NULL);

    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the flecnodal curves on a freeform surface, if any.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:                  Surface to analyze.		                     M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*   MergeTol:		  Tolerance of point to polyline merge, negative for M
*			  performing no merge into polyline.		     M
*   RContactOrder:	  3 for flecnodal curves, 4 for flecnodal points.    M
*   RUnivarMVSolver:	  TRUE to employ the univariate MV solver, FALSE to  M
*			  use the zero-dimension solution space solver.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of (u, v, a, b) locations along the flecnodal   M
*			where (u, v) are the  surface locations and (a, b)   M
*			are the direction of the 4th order tangency, as      M
*			(a dS/du + b dS/dv).                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceFlecnodals                                                        M
*****************************************************************************/
IPObjectStruct *SurfaceFlecnodals(IPObjectStruct *Srf,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *MergeTol,
				  IrtRType *RContactOrder,
				  IrtRType *RUnivarMVSolver)
{
    int ContactOrder = IRIT_REAL_PTR_TO_INT(RContactOrder),
        UnivarMVSolver = IRIT_REAL_PTR_TO_INT(RUnivarMVSolver);

    if (UnivarMVSolver && ContactOrder == 3) {
        MvarPolyStruct
	    *MVPlls = MvarSrfFlecnodalCrvs2(Srf -> U.Srfs,
					    *SubdivTol / 10.0, /* STep size. */
					    *SubdivTol,
					    *NumerTol);
	IPObjectStruct
	    *PllsObj = MvarCnvrtMVPolysToIritPolys(MVPlls);

	MvarPolyFreeList(MVPlls);

	return PllsObj;
    }
    else {
        IPObjectStruct *ObjPtList;
	MvarPtStruct
	    *MVPts = ContactOrder == 4 ?
	            MvarSrfFlecnodalPts(Srf -> U.Srfs, *SubdivTol, *NumerTol) :
	            MvarSrfFlecnodalCrvs(Srf -> U.Srfs, *SubdivTol, *NumerTol);

	if (MVPts == NULL)
	    return NULL;

	ObjPtList = MvarCnvrtMVPtsToCtlPts(MVPts, *MergeTol);
	
	MvarPtFreeList(MVPts);

	return ObjPtList;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bitangent developables for a given surface, if any.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:                 Surface(s) to analyze.	                     M
*   Orientation:          0 for no effect, -1 or +1 for a request to get     M
*		          opposite or similar normal orientation bi          M
*			  tangencies only.				     M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*   MergeTol:		  Tolerance of point to polyline merge, negative for M
*			  performing no merge into polyline.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of (u, v, s, t) locations prescribing the       M
*			where (u, v) on the surface is bitangent with (s, t).M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceBiTangents                                                        M
*****************************************************************************/
IPObjectStruct *SurfaceBiTangents(IPObjectStruct *Srfs,
				  IrtRType *Orientation,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *MergeTol)
{
    IPObjectStruct *ObjPtList;
    MvarPtStruct *MVPts;
    MvarMVStruct *MVSrf1, *MVSrf2;

    if (IP_IS_SRF_OBJ(Srfs)) {
	MVSrf1 = MvarSrfToMV(Srfs -> U.Srfs);
	MVSrf2 = NULL;
    }
    else {
        IPObjectStruct
	    *Srf1 = IPListObjectGet(Srfs, 0),
	    *Srf2 = IPListObjectGet(Srfs, 1);

	if (Srf1 == NULL || !IP_IS_SRF_OBJ(Srf1) ||
	    Srf2 == NULL || !IP_IS_SRF_OBJ(Srf2)) {
	    IRIT_NON_FATAL_ERROR("Expecting a list object with two surfacs");
	    return NULL;
	}
	MVSrf1 = MvarSrfToMV(Srf1 -> U.Srfs);		  
	MVSrf2 = MvarSrfToMV(Srf2 -> U.Srfs);		  
    }

    MVPts = MvarMVBiTangents(MVSrf1, MVSrf2, IRIT_REAL_PTR_TO_INT(Orientation),
			     *SubdivTol, *NumerTol);
    MvarMVFree(MVSrf1);
    if (MVSrf2 != NULL)
	MvarMVFree(MVSrf2);

    if (MVPts == NULL)
	return NULL;

#ifdef FILTER_ANTIPODAL_PTS
    if (MVSrf2 == NULL) {
        MvarPtStruct *MVPt;

        /* Filter duplicated solutions */
        for (MVPt = MVPts; MVPt -> Pnext != NULL; ) {
	    MvarPtStruct
		*MVPtNext = MVPt -> Pnext;

	    if (MVPtNext -> Pt[0] > MVPtNext -> Pt[2] + IRIT_EPS ||
		(IRIT_APX_EQ(MVPtNext -> Pt[0], MVPtNext -> Pt[2]) &&
		 MVPtNext -> Pt[1] > MVPtNext -> Pt[3])) {
	        MVPt -> Pnext = MVPtNext -> Pnext;
		MvarPtFree(MVPtNext);
	    }
	    else
	        MVPt = MVPt -> Pnext;
	}

	/* Now test first point... */
	if (MVPts != NULL &&
	    (MVPts -> Pt[0] > MVPts -> Pt[2] + IRIT_EPS ||
	     (IRIT_APX_EQ(MVPts -> Pt[0], MVPts -> Pt[2]) &&
	      MVPts -> Pt[1] > MVPts -> Pt[3]))) {
	     MVPt = MVPts;
	     MVPts = MVPts -> Pnext;
	     MvarPtFree(MVPt);
	}
    }
#endif /* FILTER_ANTIPODAL_PTS */

    ObjPtList = MvarCnvrtMVPtsToCtlPts(MVPts, *MergeTol);

    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the parabolic curves of a freeform surface, by computing the    M
* zero set of its Gaussian curvature field.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf:    To compute parabolic curves for.                             M
*   Euclidean:  TRUE for curves in R3 on the surface, FALSE for curves in    M
*		parametric space.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polylines approximating the parabolic curves.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   ParabolicCurvesOnSrf                                                     M
*****************************************************************************/
IPObjectStruct *ParabolicCurvesOnSrf(IPObjectStruct *PObjSrf,
				     IrtRType *Euclidean)
{
    IRIT_STATIC_DATA IrtPlnType
	GaussPlane = { 1.0, 0.0, 0.0, 1.161188e-12 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdSrfStruct
	*GaussSrf = SymbSrfGaussCurvature(PObjSrf -> U.Srfs, TRUE);
    IPObjectStruct
	*PGaussObj = IPGenSRFObject(GaussSrf),
        *ParabolicCurves = ContourFreeform(PGaussObj, GaussPlane,
					   IRIT_APX_EQ(*Euclidean, 0.0) ? NULL
					                           : PObjSrf,
					   NULL);

    IPFreeObject(PGaussObj);

    BspMultComputationMethod(OldInterpFlag);

    return ParabolicCurves;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the scalar fields that are the coefficients of the fundamental  M
* forms of the given surface.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf:   Surface to compute the fundamental form for.                  M
*   FormNum:   Fundamental form to derive - 1,2,3 for first, second, or      M
*	       third.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of three scalar surface fields holding the    M
*		coefficients of the fundamental form as (L11, L12, L22).     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfTff                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfFundamentalForms                                                      M
*****************************************************************************/
IPObjectStruct *SrfFundamentalForms(IPObjectStruct *PObjSrf,
				    IrtRType *FormNum)
{
    CagdSrfStruct *DuSrf, *DvSrf, *Nrml,
	*L11 = NULL,
	*L12 = NULL,
	*L22 = NULL;
    IPObjectStruct *RetObj;

    switch (IRIT_REAL_PTR_TO_INT(FormNum)) {
	case 1:
	    SymbSrfFff(PObjSrf -> U.Srfs, &DuSrf, &DvSrf,
		       &L11, &L12, &L22);
	    CagdSrfFree(DuSrf);
	    CagdSrfFree(DvSrf);
	    break;
	case 2:
	    DuSrf = CagdSrfDerive(PObjSrf -> U.Srfs, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(PObjSrf -> U.Srfs, CAGD_CONST_V_DIR);
	    SymbSrfSff(DuSrf, DvSrf, &L11, &L12, &L22, &Nrml);
	    CagdSrfFree(DuSrf);
	    CagdSrfFree(DvSrf);
	    CagdSrfFree(Nrml);
	    break;
	case 3:
	    SymbSrfTff(PObjSrf -> U.Srfs, &L11, &L12, &L22);
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Only 1/2/3 for first/second/third fundamental forms");
	    break;
    }

    if (L11 != NULL) {
	RetObj = IPGenLISTObject(IPGenSRFObject(L11));

        IPListObjectInsert(RetObj, 1, IPGenSRFObject(L12));
        IPListObjectInsert(RetObj, 2, IPGenSRFObject(L22));
        IPListObjectInsert(RetObj, 3, NULL);

	return RetObj;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the intersection point of the normals of the given two points   *
* on the given two curves.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1:      First curve of the matching mid point.                        *
*   t1:        Parameter value of first curve's mid point.                   *
*   Crv2       Second curve of the matching mid point.                       *
*   t2:        Parameter value of second curve's mid point.                  *
*   Dists:     Distances from the found location to the two curves.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   Point of intersection, statically allocated.              *
*****************************************************************************/
static CagdRType *ComputeInterMidPoint(CagdCrvStruct *Crv1,
				       CagdRType t1,
				       CagdCrvStruct *Crv2,
				       CagdRType t2,
				       CagdRType *Dists)
{
    IRIT_STATIC_DATA CagdPType Inter1;
    CagdPType Pt1, Pt2, Nrml1, Nrml2, Inter2;
    CagdRType *R;
    CagdVecStruct *Vec;

    R = CagdCrvEval(Crv1, t1);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, t2);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    Vec = CagdCrvNormalXY(Crv1, t1, TRUE);
    IRIT_PT_COPY(Nrml1, Vec -> Vec);
    Vec = CagdCrvNormalXY(Crv2, t2, TRUE);
    IRIT_PT_COPY(Nrml2, Vec -> Vec);

    GM2PointsFromLineLine(Pt1, Nrml1, Pt2, Nrml2, Inter1, &t1, Inter2, &t2);

    if (t1 < 0.0 || t2 < 0.0)
        return NULL;

    IRIT_PT_BLEND(Inter1, Inter1, Inter2, 0.5);/* Use an avg. as an answer. */

    Dists[0] = IRIT_PT_PT_DIST(Pt1, Inter1);
    Dists[1] = IRIT_PT_PT_DIST(Pt2, Inter1);

    return Inter1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct  rational quadratic curve out of the 6 coefficients of the     M
* conic section: A x^2 + B xy + C y^2 + D x + E y + F = 0.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ABCDEFObj:            The six coefficients of the conic curve.           M
*			  or if conic between two curves - the two curves.   M
*   ZLevel:		  Sets the Z level of this XY parallel conic curve.  M
*   Distance:		  If a conic between two curves, the distance.       M
*   RCrvEval:		  If a conic between two curves, CrvEval with 0 to   M
*			  return the function, 1 to return the contours in   M
*			  UV space, and 2 to return full Euclidean contours. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A quadratic curve representing the conic.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCreateConicCurve                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenConicSection                                                          M
*****************************************************************************/
IPObjectStruct *GenConicSection(IPObjectStruct *ABCDEFObj,
				IrtRType *ZLevel,
				IrtRType *Distance,
				IrtRType *RCrvEval)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, 1.301060e-8 };
    int i = 0,
	CrvEval = IRIT_REAL_PTR_TO_INT(RCrvEval);
    CagdRType ABCDEF[6];
    CagdCrvStruct *Conic;
    IPObjectStruct *PrmObj1, *PrmObj2;

    if ((PrmObj1 = IPListObjectGet(ABCDEFObj, 0)) != NULL &&
	IP_IS_CRV_OBJ(PrmObj1) &&
	(PrmObj2 = IPListObjectGet(ABCDEFObj, 1)) != NULL &&
	IP_IS_CRV_OBJ(PrmObj2)) {
        /* Needs to compute elliptic/hyperbolic distance to two curves. */
        IrtRType TMin1, TMax1, TMin2, TMax2;
	CagdCrvStruct
	    *Crv1 = PrmObj1 -> U.Crvs,
	    *Crv2 = PrmObj2 -> U.Crvs;
	CagdSrfStruct *Srf;
	IPObjectStruct *PObj, *PCntrObj;
	IPPolygonStruct *Pl;

	if (*Distance == 0.0) {
	    IRIT_WNDW_PUT_STR("Expecting positive distance");
	    return NULL;
        }

	CagdCrvDomain(Crv1, &TMin1, &TMax1);
	CagdCrvDomain(Crv2, &TMin2, &TMax2);

	Srf = SymbConicDistCrvCrv(Crv1, Crv2, *Distance);
	PObj = IPGenSRFObject(Srf);
	if (CrvEval == 0.0)
	    return PObj;

	if ((PCntrObj = ContourFreeform(PObj, Plane,
					NULL, NULL)) == NULL)
	    return NULL;
	for (Pl = PCntrObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *V;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		V -> Coord[0] = IRIT_BOUND(V -> Coord[0], TMin1, TMax1);
		V -> Coord[1] = IRIT_BOUND(V -> Coord[1], TMin2, TMax2);
	    }
	}        

	IPFreeObject(PObj);
	if (CrvEval == 1.0)
	    return PCntrObj;

	Pl = PCntrObj -> U.Pl;
	PCntrObj -> U.Pl = NULL;

	while (Pl != NULL) {
	    IPPolygonStruct
		*PlNext = Pl -> Pnext;
	    IPVertexStruct *V, *PrevV;

	    Pl -> Pnext = NULL;

	    for (V = PrevV = Pl -> PVertex; V != NULL; ) {
	         CagdRType *R, Dists[2], d1, d2;

		 if ((R = ComputeInterMidPoint(Crv1, V -> Coord[0],
					       Crv2, V -> Coord[1],
					       Dists)) != NULL) {
		     V -> Coord[0] = R[0];
		     V -> Coord[1] = R[1];

		     d1 = IRIT_FABS(Dists[1] + Dists[0]);
		     d2 = IRIT_FABS(Dists[1] - Dists[0]);
		 }
		 else
		     d1 = d2 = 0.0;

		 if (R == NULL ||
		     (IRIT_FABS(d1 - *Distance) / *Distance > CONIC_CRVS_ERROR &&
		      IRIT_FABS(d2 - *Distance) / *Distance > CONIC_CRVS_ERROR)) {
		     /* Purge this point! */
		     if (V == Pl -> PVertex) {
		         PrevV = Pl -> PVertex = V -> Pnext;
			 IPFreeVertex(V);
			 V = PrevV;
		     }
		     else {
		         PrevV -> Pnext = V -> Pnext;
			 IPFreeVertex(V);
			 V = PrevV -> Pnext;
		     }
		 }
		 else {
		     PrevV = V;
		     V = V -> Pnext;
		 }
	    }

	    if (IPVrtxListLen(Pl -> PVertex) > 0) {
	        IRIT_LIST_PUSH(Pl, PCntrObj -> U.Pl);
	    }
	    else
	        IPFreePolygon(Pl);

	    Pl = PlNext;
	}        

	return PCntrObj;
    }

    /* If we are here - input is likely to be six coef. of a conic sec. */
    while ((PrmObj1 = IPListObjectGet(ABCDEFObj, i)) != NULL && i < 6) {
	if (!IP_IS_NUM_OBJ(PrmObj1)) {
	    IRIT_WNDW_PUT_STR("Non numeric object found in list");
	    return NULL;
	}

	ABCDEF[i++] = PrmObj1 -> U.R;
    }
    if (i < 6) {
	IRIT_WNDW_PUT_STR("Expecting six conic section coefficients, found less");
	return NULL;
    }

    if ((Conic = CagdCreateConicCurve(ABCDEF[0],
				      ABCDEF[1],
				      ABCDEF[2],
				      ABCDEF[3],
				      ABCDEF[4],
				      ABCDEF[5],
				      *ZLevel,
				      TRUE)) != NULL) {
	return IPGenCRVObject(Conic);
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Create a planar ellipse of minimal area throu given three points.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:  The three points thru which to fit minimum area ellipse. M
*   Offset:         Offset amount to apply to the ellipse at the end.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The A,B,C,D,E,F coefficients of the ellipse in form   M
*		A x^2 + B xy + C y^2 + D x + E y + F = 0, or NULL otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdEllipse3Points                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenEllipse3Points                                                        M
*****************************************************************************/
IPObjectStruct *GenEllipse3Points(IrtPtType Pt1,
				  IrtPtType Pt2,
				  IrtPtType Pt3,
				  IrtRType *Offset)
{
    CagdRType A, B, C, D, E, F;

    if (CagdEllipse3Points(Pt1, Pt2, Pt3, &A, &B, &C, &D, &E, &F)) {
        IPObjectStruct *RetObj;

	if (*Offset != 0.0) {
	    if (!CagdEllipseOffset(&A, &B, &C, &D, &E, &F, *Offset)) {
	        IRIT_WNDW_PUT_STR("Failed to compute offset to ellipse");
		return NULL;
	    }
	}

	RetObj = IPGenLISTObject(NULL);
	IPListObjectInsert(RetObj, 0, IPGenNUMValObject(A));
	IPListObjectInsert(RetObj, 1, IPGenNUMValObject(B));
	IPListObjectInsert(RetObj, 2, IPGenNUMValObject(C));
	IPListObjectInsert(RetObj, 3, IPGenNUMValObject(D));
	IPListObjectInsert(RetObj, 4, IPGenNUMValObject(E));
	IPListObjectInsert(RetObj, 5, IPGenNUMValObject(F));
	IPListObjectInsert(RetObj, 6, NULL);

	return RetObj;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct the ten coefficients of a quadric surface out of given conic   M
* A-F coefficients A x^2 + B xy + C y^2 + D x + E y + F = 0, by promoting    M
* the conic to a quadric of height Z as
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0.      M
*                                                                            *
* PARAMETERS:                                                                M
*   ABCDEFObj:   The six/ten coefficients of the conic.			     M
*   Z:		 Height of created quadric, in the Z axis.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A quadric surface.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GenQuadric, CagdConic2Quadric			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PromoteConicToQuadric                                                    M
*****************************************************************************/
IPObjectStruct *PromoteConicToQuadric(IPObjectStruct *ABCDEFObj, IrtRType *Z)
{
    int i = 0;
    CagdRType ABCDEFGHIJ[10];
    IPObjectStruct *PrmObj, *RetObj;

    while ((PrmObj = IPListObjectGet(ABCDEFObj, i)) != NULL && i < 6) {
	if (!IP_IS_NUM_OBJ(PrmObj)) {
	    IRIT_WNDW_PUT_STR("Non numeric object found in list");
	    return NULL;
	}

	ABCDEFGHIJ[i++] = PrmObj -> U.R;
    }

    if (i == 6) {
        /* Promote the conic to a quadric. */
        ABCDEFGHIJ[9] = *Z;
        if (!CagdConic2Quadric(&ABCDEFGHIJ[0],
			       &ABCDEFGHIJ[1],
			       &ABCDEFGHIJ[2],
			       &ABCDEFGHIJ[3],
			       &ABCDEFGHIJ[4],
			       &ABCDEFGHIJ[5],
			       &ABCDEFGHIJ[6],
			       &ABCDEFGHIJ[7],
			       &ABCDEFGHIJ[8],
			       &ABCDEFGHIJ[9])) {
	    IRIT_WNDW_PUT_STR("Failed to promote conic - improper parameters!?");
	    return NULL;
	}
    }
    else {
	IRIT_WNDW_PUT_STR("Expecting six conic coefficients, found less.");
	return NULL;
    }

    RetObj = IPGenLISTObject(NULL);
    for (i = 0; i < 10; i++)
        IPListObjectInsert(RetObj, i, IPGenNUMValObject(ABCDEFGHIJ[i]));
    IPListObjectInsert(RetObj, i, NULL);

    return RetObj;

}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a rational quadric surface out of the ten coefficients of      M
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0.      M
*   If only seven coefficients (A-F & Height) are given, it is assumed a     M
* conic that is promoted to 3D surfaces of Z-height Height.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ABCDEFGHIJObj:   The seven/ten coefficients of the conic+height or       M
*		     quadric surface.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A quadric surface.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   PromoteConicToQuadric, CagdCreateQuadricSrf, CagdConic2Quadric           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenQuadric	                                                             M
*****************************************************************************/
IPObjectStruct *GenQuadric(IPObjectStruct *ABCDEFGHIJObj)
{
    int i = 0;
    CagdRType ABCDEFGHIJ[10];
    IPObjectStruct *PrmObj;
    CagdSrfStruct *Quadric;

    while ((PrmObj = IPListObjectGet(ABCDEFGHIJObj, i)) != NULL && i < 10) {
	if (!IP_IS_NUM_OBJ(PrmObj)) {
	    IRIT_WNDW_PUT_STR("Non numeric object found in list");
	    return NULL;
	}

	ABCDEFGHIJ[i++] = PrmObj -> U.R;
    }

    if (i == 7) {
        /* Promote the conic to a quadric. */
        ABCDEFGHIJ[9] = ABCDEFGHIJ[6];
        if (!CagdConic2Quadric(&ABCDEFGHIJ[0],
			       &ABCDEFGHIJ[1],
			       &ABCDEFGHIJ[2],
			       &ABCDEFGHIJ[3],
			       &ABCDEFGHIJ[4],
			       &ABCDEFGHIJ[5],
			       &ABCDEFGHIJ[6],
			       &ABCDEFGHIJ[7],
			       &ABCDEFGHIJ[8],
			       &ABCDEFGHIJ[9])) {
	    IRIT_WNDW_PUT_STR("Failed to promote conic - improper parameters!?");
	    return NULL;
	}
    }
    else if (i < 10) {
	IRIT_WNDW_PUT_STR("Expecting ten quadric coefficients, found less");
	return NULL;
    }

    if ((Quadric = CagdCreateQuadricSrf(ABCDEFGHIJ[0],
					ABCDEFGHIJ[1],
					ABCDEFGHIJ[2],
					ABCDEFGHIJ[3],
					ABCDEFGHIJ[4],
					ABCDEFGHIJ[5],
					ABCDEFGHIJ[6],
					ABCDEFGHIJ[7],
					ABCDEFGHIJ[8],
					ABCDEFGHIJ[9])) != NULL) {
	return IPGenSRFObject(Quadric);
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   transforms the implicit conic/quadric given as seven/ten coefficients as M
* A x^2 + B xy + C y^2 + D x + E y + F = 0     for the conic or 	     M
* A x^2 + B y^2 + C z^2 + D xy + E xz + F yz + G x + H y + I z + J = 0       M
* for the quartic.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Oper:            1 - to transform a conic using TransMat.                M
*		     2 - to transform a qaudric using TransMat.              M
*                    3 - to promote a conic (in A-F) to a quadric of         M
8			 z-height J.  TransMat is ignored here.		     M
*   ABCDEFGHIJObj:   The seven/ten coefficients of the conic/quadric.	     M
*   TransMat:        the desired transformation matrix.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     The mapped conic/quadric.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCreateQuadricSrf, CagdConic2Quadric	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TransformImplicit                                                        M
*****************************************************************************/
IPObjectStruct *TransformImplicit(IrtRType *Oper,
				  IPObjectStruct *ABCDEFGHIJObj,
				  IPObjectStruct *TransMat)
{
    int i = 0;
    CagdRType ABCDEFGHIJ[10];
    IPObjectStruct *PrmObj, *RetObj;

    while ((PrmObj = IPListObjectGet(ABCDEFGHIJObj, i)) != NULL && i < 10) {
	if (!IP_IS_NUM_OBJ(PrmObj)) {
	    IRIT_WNDW_PUT_STR("Non numeric object found in list");
	    return NULL;
	}

	ABCDEFGHIJ[i++] = PrmObj -> U.R;
    }

    if (i == 6) {
        CagdConicMatTransform(&ABCDEFGHIJ[0],
			      &ABCDEFGHIJ[1],
			      &ABCDEFGHIJ[2],
			      &ABCDEFGHIJ[3],
			      &ABCDEFGHIJ[4],
			      &ABCDEFGHIJ[5],
			      *(TransMat -> U.Mat));
    }
    else if (i == 7) {
        /* Promote the conic to a quadric. */
        ABCDEFGHIJ[9] = ABCDEFGHIJ[6];
        if (!CagdConic2Quadric(&ABCDEFGHIJ[0],
			       &ABCDEFGHIJ[1],
			       &ABCDEFGHIJ[2],
			       &ABCDEFGHIJ[3],
			       &ABCDEFGHIJ[4],
			       &ABCDEFGHIJ[5],
			       &ABCDEFGHIJ[6],
			       &ABCDEFGHIJ[7],
			       &ABCDEFGHIJ[8],
			       &ABCDEFGHIJ[9])) {
	    IRIT_WNDW_PUT_STR("Failed to promote conic - improper parameters!?");
	    return NULL;
	}
	i = 6;
    }
    else if (i == 10) {
        CagdQuadricMatTransform(&ABCDEFGHIJ[0],
				&ABCDEFGHIJ[1],
				&ABCDEFGHIJ[2],
				&ABCDEFGHIJ[3],
				&ABCDEFGHIJ[4],
				&ABCDEFGHIJ[5],
				&ABCDEFGHIJ[6],
				&ABCDEFGHIJ[7],
				&ABCDEFGHIJ[8],
				&ABCDEFGHIJ[9],
				*(TransMat -> U.Mat));
    }
    else {
	IRIT_WNDW_PUT_STR("Expecting ten quadric coefficients, found less");
	return NULL;
    }

    RetObj = IPGenLISTObject(NULL);
    IPListObjectInsert(RetObj, i--, NULL);
    for ( ; i >= 0; i--)
        IPListObjectInsert(RetObj, i, IPGenNUMValObject(ABCDEFGHIJ[i]));

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the given geometry to be one of several possibilities such as    M
* a line, sphere, surface of revolution, etc.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Geom:        Geometry to examine.                                        M
*   RIsType:     Is Geom of IsType type?			             M
*   Eps:	 Epsilon of test.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Returns a list object whose first element is TRUE    M
*		or FALSE.  If first element is TRUE, other elements in the   M
*		list might exist describing the decomposition of the         M
*		geometry to its construction blocks.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   WhatIsGeometryType                                                       M
*****************************************************************************/
IPObjectStruct *WhatIsGeometryType(const IPObjectStruct *Geom,
			           IrtRType *RIsType,
			           const IrtRType *Eps)
{
    CagdIsGeometryType
	IsType = (CagdIsGeometryType) IRIT_REAL_PTR_TO_INT(RIsType);
    CagdRType R;
    CagdPType Pt;
    CagdVType Dir;
    IrtPlnType Pln;
    CagdBType
	Detected = FALSE;
    IPObjectStruct
	*RetObj = IPGenLISTObject(NULL);
    CagdCtlPtStruct *CtlPt;
    CagdCrvStruct *Crv1, *Crv2;

    if (!IP_IS_CRV_OBJ(Geom) && !IP_IS_SRF_OBJ(Geom)) {
        IPListObjectInsert(RetObj, 0, IPGenNUMValObject(FALSE));
        IPListObjectInsert(RetObj, 1, NULL);
	return RetObj;
    }

    switch (IsType) {
	case CAGD_GEOM_CONST:
	    if (IP_IS_CRV_OBJ(Geom))
		Detected = SymbIsConstCrv(Geom -> U.Crvs, &CtlPt, *Eps);
	    else if (IP_IS_SRF_OBJ(Geom))
		Detected = SymbIsConstSrf(Geom -> U.Srfs, &CtlPt, *Eps);
	    else {
		assert(0);
		CtlPt = NULL;
	    }
	    if (Detected) {
		IPListObjectInsert(RetObj, 1, IPGenCTLPTObject(CtlPt -> PtType,
							       CtlPt -> Coords));
		IPListObjectInsert(RetObj, 2, NULL);
	    }
	    break;
	case CAGD_GEOM_LINEAR:
	    if (IP_IS_CRV_OBJ(Geom) &&
		(Detected = SymbIsLineCrv(Geom -> U.Crvs, Pt, Dir, *Eps))
								    != FALSE) {
	        IPListObjectInsert(RetObj, 1,
				   IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]));
	        IPListObjectInsert(RetObj, 2,
				   IPGenVECObject(&Dir[0], &Dir[1], &Dir[2]));
		IPListObjectInsert(RetObj, 3, NULL);
	    }
	    break;
	case CAGD_GEOM_CIRCULAR:
	    if (IP_IS_CRV_OBJ(Geom) &&
		(Detected = SymbIsCircularCrv(Geom -> U.Crvs, Pt, &R, *Eps))
								    != FALSE) {
	        IPListObjectInsert(RetObj, 1,
				   IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]));
	        IPListObjectInsert(RetObj, 2, IPGenNUMValObject(R));
		IPListObjectInsert(RetObj, 3, NULL);
	    }
	    break;
	case CAGD_GEOM_PLANAR:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsPlanarSrf(Geom -> U.Srfs, Pln, *Eps))
								    != FALSE) {
	        IPListObjectInsert(RetObj, 1,
				   IPGenPLANEObject(&Pln[0], &Pln[1],
						    &Pln[2], &Pln[3]));
		IPListObjectInsert(RetObj, 2, NULL);
	    }
	    break;
	case CAGD_GEOM_SPHERICAL:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsSphericalSrf(Geom -> U.Srfs, Pt, &R, *Eps))
								    != FALSE) {
	        IPListObjectInsert(RetObj, 1,
				   IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]));
	        IPListObjectInsert(RetObj, 2, IPGenNUMValObject(R));
		IPListObjectInsert(RetObj, 3, NULL);
	    }
	    break;
	case CAGD_GEOM_SRF_OF_REV:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsSrfOfRevSrf(Geom -> U.Srfs, &Crv1,
					      Pt, Dir, *Eps)) != FALSE) {
	        IPListObjectInsert(RetObj, 1, IPGenCRVObject(Crv1));
	        IPListObjectInsert(RetObj, 2,
				   IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]));
	        IPListObjectInsert(RetObj, 3,
				   IPGenVECObject(&Dir[0], &Dir[1], &Dir[2]));
		IPListObjectInsert(RetObj, 4, NULL);
	    }
	    break;
	case CAGD_GEOM_EXTRUSION:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsExtrusionSrf(Geom -> U.Srfs, &Crv1,
					      Dir, *Eps)) != FALSE) {
	        IPListObjectInsert(RetObj, 1, IPGenCRVObject(Crv1));
	        IPListObjectInsert(RetObj, 2,
				   IPGenVECObject(&Dir[0], &Dir[1], &Dir[2]));
		IPListObjectInsert(RetObj, 3, NULL);
	    }
	    break;
	case CAGD_GEOM_RULED_SRF:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsRuledSrf(Geom -> U.Srfs, &Crv1, &Crv2,
					      *Eps)) != FALSE) {
	        IPListObjectInsert(RetObj, 1, IPGenCRVObject(Crv1));
	        IPListObjectInsert(RetObj, 2, IPGenCRVObject(Crv2));
		IPListObjectInsert(RetObj, 3, NULL);
	    }
	    break;
	case CAGD_GEOM_DEVELOP_SRF:
	    if (IP_IS_SRF_OBJ(Geom) &&
		(Detected = SymbIsDevelopSrf(Geom -> U.Srfs,
					     *Eps)) != FALSE) {
		IPListObjectInsert(RetObj, 1, NULL);
	    }
	    break;
	case CAGD_GEOM_SWEEP_SRF:
	    IRIT_NON_FATAL_ERROR("Sweep surface testing is not supported\n");
	    break;
        default:
	    assert(0);
	    return NULL;
    }

    IPListObjectInsert(RetObj, 0, IPGenNUMValObject(Detected));
    if (!Detected)
        IPListObjectInsert(RetObj, 1, NULL);

    return RetObj;
}
