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
#include "objects.h"
#include "mrchcube.h"
#include "mdl_lib.h"
#include "freeform.h"

#define RFLCT_LN_SIL_EPS	0.01
#define IRIT_HFDIST_SUBDIV_TOL	0.01

static void ComputeSmoothPolyNormalsAux(IPObjectStruct *PObj,
					IrtRType MaxAngle);
static void FixPolyNormalsAux(IPObjectStruct *PObj, int TrustPixPt);
static void FixPolyGeometryAux(IPObjectStruct *PObj, int Op, IrtRType Eps);
static CagdRType *ListObj2NumericVector(IPObjectStruct *PObj, int *Length);
static IPPolygonStruct *FilterPolylineData(CagdSrfStruct *Srf,
					   int RflctLines,
					   IrtVecType ViewDir,
					   IPPolygonStruct *Pls,
					   int Euclidean,
					   IrtRType *LinePt,
					   IrtRType *LineDir);
static IrtPtType *ConvertListOfPtsToArray(IPObjectStruct *PtSet, int *n);
static IrtPtType *GetPointList(IPObjectStruct *PtList, int *n);
static IPObjectStruct *GetControlMVMesh(int n,
					int *Lengths,
					int *Orders,
					IPObjectStruct *PtLstObj,
					MvarGeomType GType,
					char **ErrStr);
static IPObjectStruct *ReverseDistParams(IPObjectStruct *Result);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute smooth normals to vertices of a polygonal object without normals M
* to vertices.                                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Polygonal object to estimate normals to its vertices.         M
*   MaxAngle:  Maximum angle between approximated normal and face normal of  M
*	       a vertex to consider valid and smooth it.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but with smooth normals        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBlendNormalsToVertices                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeSmoothPolyNormals                                                 M
*****************************************************************************/
IPObjectStruct *ComputeSmoothPolyNormals(IPObjectStruct *PObj,
					 IrtRType *MaxAngle)
{
    if (!IP_IS_OLST_OBJ(PObj) &&
	!IP_IS_POLY_OBJ(PObj) &&
	!IP_IS_POLYGON_OBJ(PObj)) {
	IRIT_WNDW_PUT_STR("Expecting a polygonal (list) object.");
	return NULL;
    }

    PObj = IPCopyObject(NULL, PObj, TRUE);

    ComputeSmoothPolyNormalsAux(PObj, *MaxAngle);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of ComputeSmoothPolyNormals                           *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object to estimate normals to its vertices.         *
*   MaxAngle:  Maximum angle between approximated normal and face normal of  *
*	       a vertex to consider valid and smooth it.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void ComputeSmoothPolyNormalsAux(IPObjectStruct *PObj,
					IrtRType MaxAngle)
{
    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
        IPObjectStruct *Obj;

        while ((Obj = IPListObjectGet(PObj, i++)) != NULL)
	    ComputeSmoothPolyNormalsAux(Obj, MaxAngle);
    }
    else if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj)) {
        GMBlendNormalsToVertices(PObj -> U.Pl, MaxAngle);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Correct the normals of a polygonal object.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         Polygonal object to correct normals.			     M
*   TrustPixPt:   0 to trust the vertices' normal,			     M
*		  1 to trust the orientation of polygons' normals,	     M
*		  2 to reorient the polygons so all plane normals point      M
*		    outside or all inside (based on first poly).	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but with fixed normals.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBlendNormalsToVertices                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   FixPolyNormals		                                             M
*****************************************************************************/
IPObjectStruct *FixPolyNormals(IPObjectStruct *PObj, IrtRType *TrustPixPt)
{
    if (!IP_IS_OLST_OBJ(PObj) &&
	!IP_IS_POLY_OBJ(PObj) &&
	!IP_IS_POLYGON_OBJ(PObj)) {
	IRIT_WNDW_PUT_STR("Expecting a polygonal (list) object.");
	return NULL;
    }

    PObj = IPCopyObject(NULL, PObj, TRUE);

    FixPolyNormalsAux(PObj, IRIT_REAL_PTR_TO_INT(TrustPixPt));

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of FixPolyNormals.	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         Polygonal object to correct normals.			     *
*   TrustPixPt:   0 to trust the vertices' normal,			     *
*		  1 to trust the orientation of polygons' normals,	     *
*		  2 to reorient the polygons so all plane normals point      *
*		    outside or all inside (based on first poly).	     *
*                 3 same as 2 but splits disjoint parts in the input object  *
*                   into different objects.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void FixPolyNormalsAux(IPObjectStruct *PObj, int TrustPixPt)
{
    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
        IPObjectStruct *Obj;

        while ((Obj = IPListObjectGet(PObj, i++)) != NULL)
	    FixPolyNormalsAux(Obj, TrustPixPt);
    }
    else if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj)) {
        GMFixNormalsOfPolyModel(PObj -> U.Pl,
				TrustPixPt == 3 ? 2 : TrustPixPt);

	if (TrustPixPt == 3) {
	    IPPolygonStruct *PlPrev, *PlHead,
	        *Pl = PObj -> U.Pl;
	    int i;
	    char Name[IRIT_LINE_LEN];

	    /* We return a list object of polygon objects: */
	    PObj -> U.Pl = NULL;
	    IPReallocNewTypeObject(PObj, IP_OBJ_LIST_OBJ);

	    /* Separate connected components into different objects. */
	    PlPrev = NULL;
	    for (PlHead = Pl, i = 0; Pl != NULL; ) {
	        if (AttrGetIntAttrib(Pl -> Attr, "_OrientDisjoint") == TRUE  &&
		    PlPrev != NULL) {
		    /* A new set of polys - a disjoint component - separate. */
		    AttrFreeOneAttribute(&Pl -> Attr, "_OrientDisjoint");

		    PlPrev -> Pnext = NULL;
		    sprintf(Name, "Part%d", i + 1);
		    IPListObjectInsert(PObj, i++, IPGenPolyObject(Name,
								  PlHead,
								  NULL));
		    PlHead = Pl;
		    PlPrev = NULL;
		}
		else {
		    PlPrev = Pl;
		    Pl = Pl -> Pnext;
		}
	    }

	    if (PlHead != NULL) {
	        sprintf(Name, "Part%d", i + 1);
	        IPListObjectInsert(PObj, i++, IPGenPolyObject(Name, PlHead,
							      NULL));
	    }

	    IPListObjectInsert(PObj, i++, NULL);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Correct the normals of a polygonal object.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         Polygonal object to correct normals.			     M
*   Op:           0 to remove duplicated polygons,			     *
*		  1 to remove zero length edges.			     *
*   Eps:	  Tolerance of vertices equality, etc.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but with fixed normals.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMBlendNormalsToVertices                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   FixPolyGeometry		                                             M
*****************************************************************************/
IPObjectStruct *FixPolyGeometry(IPObjectStruct *PObj,
				IrtRType *Op,
				IrtRType *Eps)
{
    if (!IP_IS_OLST_OBJ(PObj) &&
	!IP_IS_POLY_OBJ(PObj) &&
	!IP_IS_POLYGON_OBJ(PObj)) {
	IRIT_WNDW_PUT_STR("Expecting a polygonal (list) object.");
	return NULL;
    }

    PObj = IPCopyObject(NULL, PObj, TRUE);

    FixPolyGeometryAux(PObj, IRIT_REAL_PTR_TO_INT(Op), *Eps);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of FixPolyGeometry.	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         Polygonal object to correct normals.			     *
*   Op:           0 to remove duplicated polygons,			     *
*		  1 to remove zero length edges.			     *
*   Eps:	  Tolerance of vertices equality, etc.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void FixPolyGeometryAux(IPObjectStruct *PObj, int Op, IrtRType Eps)
{
    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
        IPObjectStruct *Obj;

        while ((Obj = IPListObjectGet(PObj, i++)) != NULL)
	    FixPolyGeometryAux(Obj, Op, Eps);
    }
    else if (IP_IS_POLY_OBJ(PObj) && IP_IS_POLYGON_OBJ(PObj)) {
	switch (Op) {
	    case 0:
	        GMCleanUpDupPolys(&PObj -> U.Pl, Eps);
		break;
	    case 1:
		GMCleanUpPolygonList(&PObj -> U.Pl, Eps);
		break;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a list of numeric values into a vector.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   A list object to fetch all its numeric values.                   *
*   Length: Set to the length of the fetched vector.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   A dynamically allocated vector with all numeric values,   *
*		   NULL if error.				             *
*****************************************************************************/
static CagdRType *ListObj2NumericVector(IPObjectStruct *PObj, int *Length)
{
    int i = 0;
    IPObjectStruct *O;
    CagdRType *NumVec;

    if (!IP_IS_OLST_OBJ(PObj)) {
        IRIT_NON_FATAL_ERROR("Expecting a LIST of numeric values.");
	return NULL;
    }

    *Length = IPListObjectLength(PObj);

    NumVec = (CagdRType *) IritMalloc(sizeof(CagdRType) * *Length);

    while ((O = IPListObjectGet(PObj, i)) != NULL && i < *Length) {
        if (!IP_IS_NUM_OBJ(O)) {
	    IritFree(NumVec);
	    IRIT_NON_FATAL_ERROR("Expecting a list of NUMERIC values.");
	    return NULL;
	}

	NumVec[i++] = O -> U.R;
    }

    return NumVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Blossom evaluation of freeforms.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Geometry to blossom.                                        M
*   BlsmVals:    Blossom values.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Evaluated Blossom.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBlossomEval, CagdSrfBlossomEval                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BlossomEvaluation                                                        M
*****************************************************************************/
IPObjectStruct *BlossomEvaluation(IPObjectStruct *PObj,
				  IPObjectStruct *BlsmVals)
{
    int BlsmsLen = 0,
	UBlsmsLen = 0,
	VBlsmsLen = 0;
    CagdRType *R,
	*Blsms = NULL,
	*UBlsms = NULL,
	*VBlsms = NULL;
    IPObjectStruct
	*RetVal = NULL;

    if (IP_IS_OLST_OBJ(BlsmVals)) {
        if (IP_IS_CRV_OBJ(PObj)) {
	    if ((Blsms = ListObj2NumericVector(BlsmVals, &BlsmsLen)) == NULL)
	        return NULL;
	}
	else if (IP_IS_SRF_OBJ(PObj)) {
	    IPObjectStruct
	        *UBlsmVals = IPListObjectGet(BlsmVals, 0),
	        *VBlsmVals = IPListObjectGet(BlsmVals, 1);

	    if ((UBlsms = ListObj2NumericVector(UBlsmVals,
						&UBlsmsLen)) == NULL ||
		(VBlsms = ListObj2NumericVector(VBlsmVals,
						&VBlsmsLen)) == NULL)
	        return NULL;
	}
    }
    else {
        IRIT_NON_FATAL_ERROR("Expecting a LIST of blossom values.");
    }

    if (IP_IS_CRV_OBJ(PObj)) {
	if (BlsmsLen >= PObj -> U.Crvs -> Order) {
	    IRIT_NON_FATAL_ERROR("Blossom LIST length larger than degree.");
        }
	else {
	    R = CagdCrvBlossomEval(PObj -> U.Crvs, Blsms, BlsmsLen);
	    RetVal = IPGenCTLPTObject(PObj -> U.Crvs -> PType, R);
	}
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
	if (UBlsmsLen >= PObj -> U.Srfs -> UOrder ||
	    VBlsmsLen >= PObj -> U.Srfs -> VOrder) {
	    IRIT_NON_FATAL_ERROR("Blossom LIST length larger than degree.");
        }
	else {
	    R = CagdSrfBlossomEval(PObj -> U.Srfs, UBlsms, UBlsmsLen,
				                   VBlsms, VBlsmsLen);

	    RetVal = IPGenCTLPTObject(PObj -> U.Crvs -> PType, R);
	}
    }
    else
        IRIT_NON_FATAL_ERROR("Only curve or surface can be blossomed.");

    IritFree(Blsms);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Remove unnecessary (redundant) knots from a given Bspline curve.         M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:       To remove unneeded knots from.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Reduced curve, that is pretty much identical to the  M
8			original curve!					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRmKntBspCrvCleanKnots, RemoveKnots                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CleanRefinedKnots                                                        M
*****************************************************************************/
IPObjectStruct *CleanRefinedKnots(IPObjectStruct *CrvObj)
{
    CagdCrvStruct
	*Crv = SymbRmKntBspCrvCleanKnots(CrvObj -> U.Crvs);

    if (Crv != NULL)
	return IPGenCRVObject(Crv);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Remove unnecessary knots from a given Bspline curve.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:       To remove unneeded knots from.                             M
*   Tolerance:    To preserve while removing knots.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Reduced curve, that is similar to original curve!    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRmKntBspCrvRemoveKnotsError, SymbRmKntBspCrvRemoveKnots, RemoveKnots M
*                                                                            *
* KEYWORDS:                                                                  M
*   RemoveKnots                                                              M
*****************************************************************************/
IPObjectStruct *RemoveKnots(IPObjectStruct *CrvObj,
			    IrtRType *Tolerance)
{
    CagdCrvStruct
	*Crv = SymbRmKntBspCrvRemoveKnots(CrvObj -> U.Crvs, *Tolerance);

    if (Crv != NULL)
	return IPGenCRVObject(Crv);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the reflection lines of a freeform surface.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:       Surface to compute reflection lines for.                   M
*   ViewDir:	  Direction of view.					     M
*   LnsSprs:      A list of either:					     M
*                 1. A vector and list of points prescribing the infinite    M
*		     (parallel) lines to be reflected off the surface.       M
*                 2. A center sphere point and list of cone opening angles   M
*		     prescribing the circular curves to be reflected off the M
*		     surface.						     M
*		  If this object is not a list object, it is assumed the     M
*		  auxiliary data precomputed in previous reflection          M
*		  computation is to be freed.			             M
*   Euclidean:    TRUE for reflection lines in R3 on the surface, FALSE for  M
*		  reflection lines in parametric space of surface.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of reflection lines.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRflctLnGen, SymbRflctCircGen					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ReflectionLines                                                          M
*****************************************************************************/
IPObjectStruct *ReflectionLines(IPObjectStruct *SrfObj,
				IrtVecType ViewDir,
				IPObjectStruct *LnsSprs,
				IrtRType *Euclidean)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, 1.280791e-6 };
    int i, j;
    IPObjectStruct *PCntrObj, *LinePt, *LinesPts, *LinesDir,
	*SprCntr, *ConeAngles, *CAngObj,
	*PCntrsObj = IPGenLISTObject(NULL);
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdSrfStruct
	*Srf = SrfObj -> U.Srfs;

    if (IRIT_PT_EQ_ZERO(ViewDir) &&
	IP_IS_OLST_OBJ(LnsSprs) &&
	(LinesDir = IPListObjectGet(LnsSprs, 0)) != NULL &&
	IP_IS_VEC_OBJ(LinesDir) &&
	(LinesPts = IPListObjectGet(LnsSprs, 1)) != NULL &&
	IP_IS_OLST_OBJ(LinesPts)) {
        for (i = j = 0; (LinePt = IPListObjectGet(LinesPts, i++)) != NULL; ) {
	    if (IP_IS_POINT_OBJ(LinePt)) {
	        IPObjectStruct
		    *PZeroObj = IPGenSRFObject(SymbHighlightLnGen(Srf,
							LinePt -> U.Pt,
							LinesDir -> U.Vec,
							NULL));

		if ((PCntrObj = ContourFreeform(PZeroObj, Plane,
						NULL, NULL)) != NULL) {
		    /* Filter out all unrelevant/redundant components. */
		    PCntrObj -> U.Pl = FilterPolylineData(Srf, TRUE, ViewDir,
							  PCntrObj -> U.Pl,
							  IRIT_APX_EQ(*Euclidean,
								 0.0),
							  LinePt -> U.Pt,
							  LinesDir -> U.Vec);

		    if (PCntrObj -> U.Pl != NULL)
		        IPListObjectInsert(PCntrsObj, j++, PCntrObj);
		    else
		        IPFreeObject(PCntrObj);
		}

		IPFreeObject(PZeroObj);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Expecting a point on reflection line.");
	    }
	}
	IPListObjectInsert(PCntrsObj, j, NULL);

	/* Free any left auxiliary data structure related to highlight comp. */
	SymbHighlightLnFree(SrfObj -> U.Srfs, NULL);
    }
    else if (IP_IS_OLST_OBJ(LnsSprs) &&
	     (LinesDir = IPListObjectGet(LnsSprs, 0)) != NULL &&
	     IP_IS_VEC_OBJ(LinesDir) &&
	     (LinesPts = IPListObjectGet(LnsSprs, 1)) != NULL &&
	     IP_IS_OLST_OBJ(LinesPts)) {
        for (i = j = 0; (LinePt = IPListObjectGet(LinesPts, i++)) != NULL; ) {
	    if (IP_IS_POINT_OBJ(LinePt)) {
	        IPObjectStruct
		    *PZeroObj = IPGenSRFObject(SymbRflctLnGen(Srf,
							    ViewDir,
							    LinePt -> U.Pt,
							    LinesDir -> U.Vec,
							    NULL));

		if ((PCntrObj = ContourFreeform(PZeroObj, Plane,
						NULL, NULL)) != NULL) {
		    /* Filter out all unrelevant/redundant components. */
		    PCntrObj -> U.Pl = FilterPolylineData(Srf, TRUE, ViewDir,
							  PCntrObj -> U.Pl,
							  IRIT_APX_EQ(*Euclidean,
								 0.0),
							  LinePt -> U.Pt,
							  LinesDir -> U.Vec);

		    if (PCntrObj -> U.Pl != NULL)
		        IPListObjectInsert(PCntrsObj, j++, PCntrObj);
		    else
		        IPFreeObject(PCntrObj);
		}

		IPFreeObject(PZeroObj);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Expecting a point on reflection line.");
	    }
	}
	IPListObjectInsert(PCntrsObj, j, NULL);

	/* Free any left auxiliary data structure related to reflect. comp. */
	SymbRflctLnFree(SrfObj -> U.Srfs, NULL);
    }
    else if (IP_IS_OLST_OBJ(LnsSprs) &&
	     (SprCntr = IPListObjectGet(LnsSprs, 0)) != NULL &&
	     IP_IS_POINT_OBJ(SprCntr) &&
	     (ConeAngles = IPListObjectGet(LnsSprs, 1)) != NULL &&
	     IP_IS_OLST_OBJ(ConeAngles)) {
        for (i = j = 0;
	     (CAngObj = IPListObjectGet(ConeAngles, i++)) != NULL;
	     ) {
	    if (IP_IS_NUM_OBJ(CAngObj)) {
	        IPObjectStruct
		    *PZeroObj = IPGenSRFObject(SymbRflctCircGen(Srf,
							      ViewDir,
							      SprCntr -> U.Pt,
							      CAngObj -> U.R,
							      NULL));

		if ((PCntrObj = ContourFreeform(PZeroObj, Plane,
						NULL, NULL)) != NULL) {
		    /* Filter out all unrelevant/redundant components. */
		    PCntrObj -> U.Pl = FilterPolylineData(Srf, FALSE, ViewDir,
							  PCntrObj -> U.Pl,
							  IRIT_APX_EQ(*Euclidean,
								 0.0),
							  NULL, NULL);

		    if (PCntrObj -> U.Pl != NULL)
		        IPListObjectInsert(PCntrsObj, j++, PCntrObj);
		    else
		        IPFreeObject(PCntrObj);
		}

		IPFreeObject(PZeroObj);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Expecting a cone's opening angle for reflection circle.");
	    }
	}
	IPListObjectInsert(PCntrsObj, j, NULL);

	/* Free any left auxiliary data structure related to reflect. comp. */
	SymbRflctCircFree(SrfObj -> U.Srfs, NULL);
    }
    else {
	/* Return an empty list object. */
	IPListObjectInsert(PCntrsObj, 0, NULL);
    }

    BspMultComputationMethod(OldInterpFlag);

    return PCntrsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Filter out all unrelevant/redundant components.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        Surface geometry where this polyline data came from.         *
*   RflctLines: Are we reflecting lines (TRUE), or circles (FALSE).	     *
*   ViewDir:	Direction of view, or (0, 0, 0) to ignore.		     *
*   Pls:        Polyline data to filter.                                     *
*   Euclidean:  TRUE for reflection lines in R3 on the surface, FALSE for    *
*		reflection lines in parametric space of surface.	     *
*   LinePt:     If reflecting lines, a point on the reflected line.          *
*   LineDir:    If reflecting lines, the direction of the line.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Filtered data                                        *
*****************************************************************************/
static IPPolygonStruct *FilterPolylineData(CagdSrfStruct *Srf,
					   int RflctLines,
					   IrtVecType ViewDir,
					   IPPolygonStruct *Pls,
					   int Euclidean,
					   IrtRType *LinePt,
					   IrtRType *LineDir)
{
    IPPolygonStruct *Pl, *PlPrev;
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    for (Pl = PlPrev = Pls; Pl != NULL; ) {
        IPVertexStruct *VPrev, *V;

	for (V = VPrev = Pl -> PVertex; V != NULL; ) {
	    int Purge = FALSE;
	    CagdRType t, t1, t2, *P;
	    CagdPType PtE3, Pt1, Pt2;
	    CagdVType RflctDir;
	    CagdVecStruct *Nrml;

	    V -> Coord[0] = IRIT_BOUND(V -> Coord[0], UMin, UMax);
	    V -> Coord[1] = IRIT_BOUND(V -> Coord[1], VMin, VMax);

	    P = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);
	    CagdCoerceToE3(PtE3, &P, -1, Srf -> PType);

	    if (!IRIT_PT_EQ_ZERO(ViewDir)) {
		Nrml = CagdSrfNormal(Srf, V -> Coord[0], V -> Coord[1], TRUE);
		t = IRIT_DOT_PROD(Nrml -> Vec, ViewDir);
		if (IRIT_FABS(t) < RFLCT_LN_SIL_EPS)
		    Purge = TRUE;  /* Almost a silhouette point. */
		else {
		    /* Compute reflection direction... */
		    IRIT_VEC_COPY(RflctDir, Nrml -> Vec);
		    IRIT_VEC_SCALE(RflctDir, 2.0 * t);
		    IRIT_VEC_SUB(RflctDir, RflctDir, ViewDir);

		    if (RflctLines) {
			/* Solve for inter. point of reflection line */
			/* and reflection direction off surface.     */
			GM2PointsFromLineLine(PtE3, RflctDir, LinePt, LineDir,
					      Pt1, &t1, Pt2, &t2);

			Purge = ((t < 0.0) ^ (t1 > 0.0));
		    }
		}
	    }

	    if (Purge) {
	        if (V == Pl -> PVertex) {
		    Pl -> PVertex = V -> Pnext;
		    IPFreeVertex(V);
		    V = VPrev = Pl -> PVertex;
		}
		else {
		    /* Break the polyline there. */
		    VPrev -> Pnext = NULL;
		    Pl -> Pnext = IPAllocPolygon(0, V -> Pnext, Pl -> Pnext);
		    IPFreeVertex(V);
		    V = NULL;
		}
	    }
	    else {
	        if (Euclidean) {
		    V -> Coord[0] = V -> Coord[1];
		    V -> Coord[1] = V -> Coord[2];
		    V -> Coord[2] = 0.0;
		}
		else {
		    IRIT_PT_COPY(V -> Coord, PtE3);
		}

		VPrev = V;
		V = V -> Pnext;
	    }
	}

	if (Pl -> PVertex == NULL) {
	    /* We have a completely deleted polyline. */
	    if (Pl == Pls) {
	        Pls = Pl -> Pnext;
		IPFreePolygon(Pl);
		Pl = PlPrev = Pls;
	    }
	    else {
	        PlPrev -> Pnext = Pl -> Pnext;
		IPFreePolygon(Pl);
		Pl = PlPrev -> Pnext;
	    }
	}
	else {
	    PlPrev = Pl;
	    Pl = Pl -> Pnext;
	}
    }

    return Pls;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the area integral of a given curve.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:       To compute area integral for.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Integral of area as a scalar.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvEnclosedArea			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvAreaIntergal                                                          M
*****************************************************************************/
IPObjectStruct *CrvAreaIntergal(IPObjectStruct *CrvObj)
{
    CagdCrvStruct
	*Crv = SymbCrvEnclosedArea(CrvObj -> U.Crvs);

    if (Crv != NULL)
	return IPGenCRVObject(Crv);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the volume integral of a given surface.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:      To compute volume integral for.                             M
*   Method:      Either 1 (for volume to XY plane) or			     M
*			2 (for volume to origin).			     M
*   Eval:	 TRUE to evaluate the volume integral to its numeric result. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Integral of volume (A surface volume scalar field or M
*			a numeric volume result).			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfVolume1Srf, SymbSrfVolume1, SymbSrfVolume2Srf, SymbSrfVolume2     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfVolumeIntergal                                                        M
*****************************************************************************/
IPObjectStruct *SrfVolumeIntergal(IPObjectStruct *SrfObj,
				  IrtRType *Method,
				  IrtRType *Eval)
{
    IrtRType Vol;
    CagdSrfStruct *VolSrf;

    switch (IRIT_REAL_PTR_TO_INT(Method)) {
	case 1:
	    if (IRIT_REAL_PTR_TO_INT(Eval)) {
	        Vol = SymbSrfVolume1(SrfObj -> U.Srfs);
		return IPGenNUMValObject(Vol);

	    }
	    else {
	        VolSrf = SymbSrfVolume1Srf(SrfObj -> U.Srfs, TRUE);
		return IPGenSRFObject(VolSrf);
	    }
        case 2:
	default:
	    if (IRIT_REAL_PTR_TO_INT(Eval)) {
	        Vol = SymbSrfVolume2(SrfObj -> U.Srfs);
		return IPGenNUMValObject(Vol);
	    }
	    else {
	        VolSrf = SymbSrfVolume2Srf(SrfObj -> U.Srfs, TRUE);
		return IPGenSRFObject(VolSrf);
	    }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the volume integral of a given surface.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:      To compute volume integral for.                             M
*   Moment:	 Either 1 or 2 for first or second moment.		     M
*   Axis1, Axis2:  1 for X, 2 for Y, 3 for Z.	Axis2 is ignored for first   M
*		 order moments.						     M
*   Eval:	 TRUE to evaluate the volume integral to its numeric result. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Integral of volume (A surface volume scalar field or M
*			a numeric volume result).			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFirstMoment, SymbSrfFirstMomentSrf,                               M
*   SymbSrfSecondMoment, SymbSrfSecondMomentSrf                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfMomentIntergal                                                        M
*****************************************************************************/
IPObjectStruct *SrfMomentIntergal(IPObjectStruct *SrfObj,
				  IrtRType *Moment,
				  IrtRType *Axis1,
				  IrtRType *Axis2,
				  IrtRType *Eval)
{
    CagdRType Mom;
    CagdSrfStruct *MomSrf;

    switch (IRIT_REAL_PTR_TO_INT(Moment)) {
	case 1:
	    if (IRIT_REAL_PTR_TO_INT(Eval)) {
	        Mom = SymbSrfFirstMoment(SrfObj -> U.Srfs,
					 IRIT_REAL_PTR_TO_INT(Axis1));
		return IPGenNUMValObject(Mom);
	    }
	    else {
	        MomSrf = SymbSrfFirstMomentSrf(SrfObj -> U.Srfs,
					       IRIT_REAL_PTR_TO_INT(Axis1),
					       TRUE);
		return IPGenSRFObject(MomSrf);
	    }
	case 2:
	    if (IRIT_REAL_PTR_TO_INT(Eval)) {
	        Mom = SymbSrfSecondMoment(SrfObj -> U.Srfs,
					  IRIT_REAL_PTR_TO_INT(Axis1),
					  IRIT_REAL_PTR_TO_INT(Axis2));
		return IPGenNUMValObject(Mom);
	    }
	    else {
	        MomSrf = SymbSrfSecondMomentSrf(SrfObj -> U.Srfs,
						IRIT_REAL_PTR_TO_INT(Axis1),
						IRIT_REAL_PTR_TO_INT(Axis2),
						TRUE);
		return IPGenSRFObject(MomSrf);
	    }
	default:
	    IRIT_NON_FATAL_ERROR("Invalid moment - only first or second.");
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the given set of points into an array.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   PtSet:   A list object of a set of points.                               *
*   n:       To place here the number of points converted.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtPtType *:    Array of n points, allocated dynamically.                *
*****************************************************************************/
static IrtPtType *ConvertListOfPtsToArray(IPObjectStruct *PtSet, int *n)
{
    int i,
        NumVertices = -1;
    IrtPtType *PtArray;
    IPObjectStruct *PtObj;

    while ((PtObj = IPListObjectGet(PtSet, ++NumVertices)) != NULL) {
	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    IRIT_NON_FATAL_ERROR("Non point object found in list");
	    return NULL;
	}
    }
    *n = NumVertices;
    PtArray = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * NumVertices);

    for (i = 0; i < NumVertices; i++) {
        PtObj = IPCoerceObjectTo(IPListObjectGet(PtSet, i), IP_OBJ_POINT);

	IRIT_PT_COPY(PtArray[i], PtObj -> U.Pt);
	IPFreeObject(PtObj);
    }

    return PtArray;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Register (find the rigid motion transformation) to match points set      M
* PointSet1 with either a second point set PointSet2 or with surface Srf2.   M
*   Returns a rigid motion transformation that brings the first point set,   M
* PointsSet1 to match the second set, PointSetSrf2.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PointsSet1:      The first points set as set of points.                  M
*   PointsSetSrf2:   The second - either a points set or a surface.          M
*   AlphaConverge:   Numerical controller between (more than) zero and one.  M
*		     For (almost) zero, the convergance is slow but stable   M
*		     and for one it will be fast but less stable.	     M
*   Tolerance:       Of registeration, in L infinity sense.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A matrix object represented the computed           M
*			  matrix of the registration.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRegisterTwoPointSets                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   RegisterPointSet                                                         M
*****************************************************************************/
IPObjectStruct *RegisterPointSet(IPObjectStruct *PointsSet1,
				 IPObjectStruct *PointsSetSrf2,
				 IrtRType *AlphaConverge,
				 IrtRType *Tolerance)
{
    if (IP_IS_OLST_OBJ(PointsSetSrf2)) {
        int n1, n2;
        IrtPtType
	    *PtSet1 = ConvertListOfPtsToArray(PointsSet1, &n1),
	    *PtSet2 = ConvertListOfPtsToArray(PointsSetSrf2, &n2);
	IrtHmgnMatType RegMat;
	IrtRType CompTol;

	if (PtSet1 == NULL || PtSet2 == NULL) {
	    if (PtSet1 != NULL)
	        IritFree(PtSet1);
	    if (PtSet2 != NULL)
	        IritFree(PtSet2);
	    return NULL;
	}
	
	CompTol = UserRegisterTwoPointSets(n1, PtSet1, n2, PtSet2,
					   *AlphaConverge, *Tolerance,
					   NULL, RegMat);

	IritFree(PtSet1);
	IritFree(PtSet2);

	if (CompTol > *Tolerance)
	    IRIT_NON_FATAL_ERROR("Failed to converge to desired tolerance.");

	return IPGenMATObject(RegMat);
    }
    else if (IP_IS_SRF_OBJ(PointsSetSrf2)) {
        int n1;
        IrtPtType
	    *PtSet1 = ConvertListOfPtsToArray(PointsSet1, &n1);
	IrtHmgnMatType RegMat;
	IrtRType CompTol;

	if (PtSet1 == NULL)
	    return NULL;
	
	CompTol = UserRegisterPointSetSrf(n1, PtSet1, PointsSetSrf2 -> U.Srfs,
					  *AlphaConverge, *Tolerance,
					  NULL, RegMat);

	IritFree(PtSet1);

	if (CompTol > *Tolerance)
	    IRIT_NON_FATAL_ERROR("Failed to converge to desired tolerance.");

	return IPGenMATObject(RegMat);
    }
    else {
        IRIT_NON_FATAL_ERROR("Expecting either a points set or a surface.");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a real 3D bump mapping over surface Srf with texture Texture  M
* tiled over the surface (DuDup x DvDup) times.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to construct 3D bump texture geometry for.            M
*   Texture:   The geometry of a single tile.  Spans [0, 1] in x and y.      M
*   DuDup:     Number of times to place the tile along the U axis.           M
*   DvDup:     Number of times to place the tile along the V axis.           M
*   LclUVs:    TRUE for using local Texture tile UV's, FALSE for Srf UV's.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The geometry of the bumy surface.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserDDMPolysOverSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalDDMForSrf                                                            M
*****************************************************************************/
IPObjectStruct *EvalDDMForSrf(IPObjectStruct *Srf,
			      IPObjectStruct *Texture,
			      IrtRType *DuDup,
			      IrtRType *DvDup,
			      IrtRType *LclUVs)
{
    if (IP_IS_SRF_OBJ(Srf)) {
        return UserDDMPolysOverSrf(Srf -> U.Srfs, Texture, *DuDup, *DvDup,
				   IRIT_REAL_PTR_TO_INT(LclUVs));
    }
    else if (IP_IS_POLY_OBJ(Srf) && IP_IS_POLYGON_OBJ(Srf)) {
        return UserDDMPolysOverPolys(Srf, Texture, *DuDup, *DvDup,
				     IRIT_REAL_PTR_TO_INT(LclUVs));
    }
    else {
        IRIT_NON_FATAL_ERROR("Expecting either a polygonal mesh or a surface.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch a list of point objects into a vector of IrtPtType.                *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:   A list object of points.                                       *
*   n:        Updated with the length of the created vector of points.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtPtType *:  A vector of points holding the fetched list.               *
*****************************************************************************/
static IrtPtType *GetPointList(IPObjectStruct *PtList, int *n)
{
    int i, j,
        NumVertices = -1;
    IrtPtType *Pts;
    IPObjectStruct *PtObj;
    CagdPointType PtType;

    if (!IP_IS_OLST_OBJ(PtList)) {
	IRIT_NON_FATAL_ERROR("Input not object list object!");
	return NULL;
    }

    while ((PtObj = IPListObjectGet(PtList, ++NumVertices)) != NULL) {
	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    IRIT_NON_FATAL_ERROR("Non point object found in list");
	    return NULL;
	}
    }

    /* Coerce all points to a common space, in place. */
    if ((PtType = IPCoercePtsListTo(PtList, CAGD_PT_E1_TYPE)) == CAGD_PT_NONE)
	return NULL;

    Pts = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * NumVertices);

    for (i = 0; i < NumVertices; i++) {
	IPObjectStruct
	    *VObj = IPListObjectGet(PtList, i);
	IrtRType
	    *v = VObj -> U.CtlPt.Coords;

	if (CAGD_IS_RATIONAL_PT(PtType)) {
	    CagdRType 
	        w = *v++;

	    for (j = 0; j < 3; j++)
		Pts[i][j] = *v++ / w;
	}
	else
	    for (j = 0; j < 3; j++)
		Pts[i][j] = *++v;
    }

    *n = NumVertices;

    return Pts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Analytic fit to a given set of points.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   UVPts:          List of parametric domain points.                        M
*   EucPts:         List of Euclidean points (R^3). Same length as UVPts.    M
*   RFirstAtOrigin: TRUE to coerce the first point to be at UV origin.       M
*   RFitDegree:     1 for a bilinear, 2 for quadrics.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  4/6 coefficient points of the fitted surfaces as:     M
*    RFitDegree = 1:  A + B * u + C * v + D * u * v                          M
*    RFitDegree = 2:  A + B * u + C * v + D * u * u + E * u * v + F * v * v  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSrfBilinearFit, GMSrfQuadricFit                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   AnalyticSrfFit                                                           M
*****************************************************************************/
IPObjectStruct *AnalyticSrfFit(IPObjectStruct *UVPts,
			       IPObjectStruct *EucPts,
			       IrtRType *RFirstAtOrigin,
			       IrtRType *RFitDegree)
{
    int i, UVLen, EucLen,
	FitLen = 0,
	FirstAtOrigin = IRIT_REAL_PTR_TO_INT(RFirstAtOrigin),
	FitDegree = IRIT_REAL_PTR_TO_INT(RFitDegree);
    IrtPtType *FitPts,
	*Uvs = GetPointList(UVPts, &UVLen),
	*Eucs = GetPointList(EucPts, &EucLen);
    IPObjectStruct *RetFit;

    if (Uvs == NULL || Eucs == NULL || UVLen != EucLen) {
	if (Uvs != NULL)
	    IritFree(Uvs);
	if (Eucs != NULL)
	    IritFree(Eucs);

	return NULL;
    }
    
    switch(FitDegree) {
	case 1:
	    FitPts = GMSrfBilinearFit(Uvs, Eucs, FirstAtOrigin, UVLen);
	    FitLen = 4;
	    break;
	case 2:
	    FitPts = GMSrfQuadricFit(Uvs, Eucs, FirstAtOrigin, UVLen);
	    FitLen = 6;
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Invalid fit order!");
    }

    IritFree(Uvs);
    IritFree(Eucs);

    if (FitLen == 0)
        return NULL;

    RetFit = IPGenLISTObject(NULL);

    for (i = 0; i < FitLen; i++) {
	IPListObjectInsert(RetFit, i, IPGenPTObject(&FitPts[i][0],
						    &FitPts[i][1],
						    &FitPts[i][2]));
    }
    IPListObjectInsert(RetFit, i, NULL);

    return RetFit;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clip a polygonal model against a prescribed plane.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:   A polygonal object to clip.                                      M
*   Plane:  To clip polygonal object Poly against.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of three polygonal objects (that can be        M
*		empty!): the polygon in the positive side of the Plane, the  M
*		polygons that intersects the Plane and the polygons on the   M
*		negative side of Plane, in this order.	                     M
*		If one of the polygonal lists is empty, a numeric value of   M
*		zero is substituted instead.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMClipPolysAgainstPlane                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyPlaneClipping                                                        M
*****************************************************************************/
IPObjectStruct *PolyPlaneClipping(IPObjectStruct *Poly, IrtPlnType Plane)
{
    IPObjectStruct *PObj;
    IPPolygonStruct *PNeg, *PInter,
	*PPos = GMClipPolysAgainstPlane(Poly -> U.Pl, &PNeg, &PInter, Plane);
    IPObjectStruct *(* IPGenPolyFuncPtr)(IPPolygonStruct *);

    if (IP_IS_POLYGON_OBJ(Poly))
        IPGenPolyFuncPtr = IPGenPOLYObject;
    else
        IPGenPolyFuncPtr = IPGenPOLYLINEObject;

    PObj = IPGenLISTObject(PPos == NULL ? IPGenNUMValObject(0)
			                : IPGenPolyFuncPtr(PPos));
    IPListObjectInsert(PObj, 1, PInter == NULL ? IPGenNUMValObject(0)
			                       : IPGenPolyFuncPtr(PInter));
    IPListObjectInsert(PObj, 2, PNeg == NULL ? IPGenNUMValObject(0)
			                     : IPGenPolyFuncPtr(PNeg));
    IPListObjectInsert(PObj, 3, NULL);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the curvature of the given curve and location.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:   Curve to evaluate the curvature for.			     M
*   t:      Parametric location where to evaluate the curvature.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The curvature value.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalCrvCurvature, EvalSurfaceCurvature                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalCurveCurvature                                                       M
*****************************************************************************/
IPObjectStruct *EvalCurveCurvature(IPObjectStruct *PCrv, IrtRType *t)
{
    CagdRType k;
    IPObjectStruct *PRetVal;

    if (AttrGetPtrAttrib(PCrv -> U.Crvs -> Attr, "_EvalCurv") == NULL)
	SymbEvalCrvCurvPrep(PCrv -> U.Crvs, TRUE);

    SymbEvalCrvCurvature(PCrv -> U.Crvs, *t, &k);

    SymbEvalCrvCurvPrep(PCrv -> U.Crvs, FALSE);
    AttrFreeOneAttribute(&PCrv -> U.Crvs -> Attr, "_EvalCurv");

    PRetVal = IPGenNUMValObject(k);

    return PRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate principle curvatures and directions for the given surface.      M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:  Surface to evaluate the prinicple curvatures/directions.          M
*   U, V:   Parametric location where to evaluate the curvature properties.  M
*   REuclidean:  TRUE for asymptotes in Euclidean tangent space,	     M
*		 FALSE for parameteric space.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of four entries as (k1, D1, k2, D2) where    M
*			 ki are principle curvatures and Di are principle    M
*			 directions.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvature, EvalCurveCurvature                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalSurfaceCurvature                                                     M
*****************************************************************************/
IPObjectStruct *EvalSurfaceCurvature(IPObjectStruct *PSrf,
				     IrtRType *U,
				     IrtRType *V,
				     IrtRType *REuclidean)
{
    CagdRType k1, k2;
    CagdVType D1, D2;
    IPObjectStruct *PRetVal;

    if (AttrGetPtrAttrib(PSrf -> U.Srfs -> Attr, "_EvalCurv") == NULL)
	SymbEvalSrfCurvPrep(PSrf -> U.Srfs, TRUE);

    SymbEvalSrfCurvature(PSrf -> U.Srfs, *U, *V,
			 !IRIT_REAL_PTR_TO_INT(REuclidean), &k1, &k2, D1, D2);

    SymbEvalSrfCurvPrep(PSrf -> U.Srfs, FALSE);
    AttrFreeOneAttribute(&PSrf -> U.Srfs -> Attr, "_EvalCurv");

    PRetVal = IPGenLISTObject(IPGenNUMValObject(k1));
    IPListObjectInsert(PRetVal, 1, IPGenVECObject(&D1[0], &D1[1], &D1[2]));
    IPListObjectInsert(PRetVal, 2, IPGenNUMValObject(k2));
    IPListObjectInsert(PRetVal, 3, IPGenVECObject(&D2[0], &D2[1], &D2[2]));
    IPListObjectInsert(PRetVal, 4, NULL);

    return PRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate asymptotic directions for the given surface/location.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:  Surface to evaluate asymptotic directions for.	             M
*   U, V:  Parametric location where to evaluate the asymptotes.	     M
*   REuclidean:  TRUE for asymptotes in Euclidean tangent space,	     M
*		 FALSE for parameteric space.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of upto two vector, the asymptotes.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvature                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalSurfaceAsympDir                                                      M
*****************************************************************************/
IPObjectStruct *EvalSurfaceAsympDir(IPObjectStruct *PSrf,
				    IrtRType *U,
				    IrtRType *V,
				    IrtRType *REuclidean)
{
    int n,
	i = 0;
    CagdVType Dir1, Dir2;
    IPObjectStruct *PRetVal;

    /* Note we never free this "_EvalCurv" attribute so it is essentially */
    /* a memory leak...							  */
    if (AttrGetPtrAttrib(PSrf -> U.Srfs -> Attr, "_EvalCurv") == NULL)
	SymbEvalSrfCurvPrep(PSrf -> U.Srfs, TRUE);

    n = SymbEvalSrfAsympDir(PSrf -> U.Srfs, *U, *V,
			    !IRIT_REAL_PTR_TO_INT(REuclidean), Dir1, Dir2);

    SymbEvalSrfCurvPrep(PSrf -> U.Srfs, FALSE);
    AttrFreeOneAttribute(&PSrf -> U.Srfs -> Attr, "_EvalCurv");

    PRetVal = IPGenLISTObject(NULL);
    if (n >= 1)
        IPListObjectInsert(PRetVal, i++,
			   IPGenVECObject(&Dir1[0], &Dir1[1], &Dir1[2]));
    if (n >= 2)
	IPListObjectInsert(PRetVal, i++,
			   IPGenVECObject(&Dir2[0], &Dir2[1], &Dir2[2]));
    IPListObjectInsert(PRetVal, i, NULL);

    return PRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an arc length approximation to a given.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:       Curve to construct an arc length approximation for.          M
*   Fineness:   Tolerance to use is sampling the original curve.	     M
*   ROrder:     Order of expected approximation.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Constructed arc length approximating curve.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvArcLenCrv                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvArcLenApprox                                                          M
*****************************************************************************/
IPObjectStruct *CrvArcLenApprox(IPObjectStruct *PCrv,
				IrtRType *Fineness,
				IrtRType *ROrder)
{
    CagdCrvStruct
	*Crv = SymbCrvArcLenCrv(PCrv -> U.Crvs,	*Fineness,
				IRIT_REAL_PTR_TO_INT(ROrder));

    return Crv == NULL ? NULL : IPGenCRVObject(Crv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Warps the given text, using the current loaded font, in Bezier form in   M
* surface Srf using composition.  The characters of Txt are laid one after   M
* the other until the entire surface is filled horizontally, or characters   M
* in Txt are exhausted.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:         Surface to warp the text Txt along.  The text is laid      M
*		  along the u axis with the v axis being the height.         M
*   Txt:          Text to warp insid Srf.				     M
*   HSpace:       Horizonal space between characters.			     M
*   VBase, VTop:  Minimal and maximaal fraaction of height for regular       M
*		  charaacters.  Measured on the letter 'A'.                  M
*   RLigatures:   If TRUE, better handle the ligatures between chars.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Warped text.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserWarpTextOnSurface, GMLoadTextFont, GMMakeTextGeometry,		     M
*   SymbComposeSrfCrv              			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextWarpThroughSrf                                                       M
*****************************************************************************/
IPObjectStruct *TextWarpThroughSrf(const IPObjectStruct *PSrf,
				   const char *Txt,
				   const IrtRType *HSpace,
				   const IrtRType *VBase,
				   const IrtRType *VTop,
				   const IrtRType *RLigatures)
{
    return UserWarpTextOnSurface(PSrf -> U.Srfs, Txt, *HSpace,
				 *VBase, *VTop, *RLigatures);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the ray-surface intersection using Bezier clipping.              M
*                                                                            *
* PARAMETERS:                                                                M
*   RayPt:    Ray origin location.					     M
*   RayDir:   Ray direction.						     M
*   PSrf:     Surface to process and intersect.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list object of the form			     M
*                       list( #Inter, UV0, EucPt0, ..., UVn, EucPtn ).	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdRayTraceBzrSrf                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BezierRayClipping                                                        M
*****************************************************************************/
IPObjectStruct *BezierRayClipping(IrtPtType RayPt,
				  IrtVecType RayDir,
				  IPObjectStruct *PSrf)
{
    int i = 0;
    CagdRType
	Zero = 0.0;
    IPObjectStruct
	*PRetVal = IPGenLISTObject(NULL);
    CagdUVStruct *IntrPrm, *TmpIntrPrm;
    CagdPtStruct *IntrPt, *TmpIntrPt;

    if (CagdRayTraceBzrSrf(RayPt, RayDir, PSrf -> U.Srfs, &IntrPrm, &IntrPt)) {
        IPListObjectInsert(PRetVal, i++,
			   IPGenNUMValObject(CagdListLength(IntrPrm)));
	for (TmpIntrPrm = IntrPrm, TmpIntrPt = IntrPt;
	     TmpIntrPrm != NULL && TmpIntrPt != NULL;
	     TmpIntrPrm = TmpIntrPrm -> Pnext, TmpIntrPt = TmpIntrPt -> Pnext) {
	    IPListObjectInsert(PRetVal, i++,
			       IPGenPTObject(&TmpIntrPrm -> UV[0],
					     &TmpIntrPrm -> UV[1],
					     &Zero));
	    IPListObjectInsert(PRetVal, i++,
			       IPGenPTObject(&TmpIntrPt -> Pt[0],
					     &TmpIntrPt -> Pt[1],
					     &TmpIntrPt -> Pt[2]));
        }
        CagdUVFreeList(IntrPrm);
        CagdPtFreeList(IntrPt);
    }
    else {
        IPListObjectInsert(PRetVal, i++, IPGenNUMValObject(0));
    }
    IPListObjectInsert(PRetVal, i, NULL);

    return PRetVal;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the control mesh lists to a multivariate control mesh.     *
*   The multivariate is allocated here as well.				     *
*   Returns the multivariate if o.k., otherwise NULL.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   n:          Number of dimensions to this multivariates.                  *
*   Lengths:    n lengths in each dimension of the multivariate.             *
*   Orders:     n orders in each dimension of the multivariate.              *
*   PtLstObj:   A list of control points of size                             *
*				(Lengths[0] * Lengths[1] * .. Lengths[n-1]). *
*   GType:      Geometry type - Bezier, Bspline etc.                         *
*   ErrStr:     If an error, detected, this is initialized with description. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A trivar object if successful, NULL otherwise.       *
*****************************************************************************/
static IPObjectStruct *GetControlMVMesh(int n,
					int *Lengths,
					int *Orders,
					IPObjectStruct *PtLstObj,
					MvarGeomType GType,
					char **ErrStr)
{
    int i, j, PtSize,
	NumCtlPt = 1;
    CagdRType **r;
    IrtRType *v;
    IPObjectStruct *MVObj, *PtObj;
    CagdPointType
	PtType = CAGD_PT_E1_TYPE;

    for (i = 0; i < n; i++) {
        if (Lengths[i] < 2) {
	    *ErrStr = IRIT_EXP_STR("Less than two points in some direction");
	    return NULL;
	}

        NumCtlPt *= Lengths[i];
    }

    if (IPListObjectLength(PtLstObj) != NumCtlPt) {
        *ErrStr = IRIT_EXP_STR("Wrong number of control points found");
	return NULL;
    }

    if ((PtType = IPCoerceCommonSpace(PtLstObj, PtType)) == CAGD_PT_NONE) {
        *ErrStr = "";
	return NULL;
    }

    /* Coerce all points to a common space, in place. */
    if (IPCoercePtsListTo(PtLstObj, PtType) == CAGD_PT_NONE) {
        *ErrStr = "";
	return NULL;
    }

    MVObj = IPGenMULTIVARObject(NULL);
    switch (GType) {
	case MVAR_POWER_TYPE:
	    MVObj -> U.MultiVars =  MvarMVNew(n, MVAR_POWER_TYPE,
					      (MvarPointType) PtType, Lengths);
	    CAGD_GEN_COPY(MVObj -> U.MultiVars -> Orders,
			  MVObj -> U.MultiVars -> Lengths, n * sizeof(int));
	    break;
	case MVAR_BEZIER_TYPE:
	    MVObj -> U.MultiVars = MvarBzrMVNew(n, Lengths,
						(MvarPointType) PtType);
	    break;
	case MVAR_BSPLINE_TYPE:
	    MVObj -> U.MultiVars = MvarBspMVNew(n, Lengths, Orders,
						(MvarPointType) PtType);
	    break;
	default:
	    break;
    }
    PtSize = CAGD_IS_RATIONAL_PT(PtType) + CAGD_NUM_OF_PT_COORD(PtType);

    for (i = 0, r = MVObj -> U.MultiVars -> Points; i < NumCtlPt; i++) {
        PtObj = IPListObjectGet(PtLstObj, i);

	v = PtObj -> U.CtlPt.Coords;

	if (CAGD_IS_RATIONAL_PT(PtType))
	    for (j = 0; j < PtSize; j++)
	        r[j][i] = *v++;
	else
	    for (j = 1; j <= PtSize; j++)
	        r[j][i] = *++v;
      }

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier trivar geometric object defined by a list of  M
* lists of lists of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   LensLstObj:    List of lengths in each of the dimensions of the MV.      M
*   PtLstObj:      A list of control points of size                          M
*			(LenLstObj[0] * LenLstObj[1] * .. LenLstObj[n-1]).   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier trivar object if successful, NULL otherwise.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBezierMVObject                                                        M
*****************************************************************************/
IPObjectStruct *GenBezierMVObject(IPObjectStruct *LensLstObj,
				  IPObjectStruct *PtLstObj)
{
    int i,
	n = IPListObjectLength(LensLstObj),
	*Lens = (int *) IritMalloc(sizeof(int *) * n);
    char *ErrStr;
    IPObjectStruct *MVObj;

    for (i = 0; i < n; i++) {
        IPObjectStruct
	    *PTmp = IPListObjectGet(LensLstObj, i);

	if (IP_IS_NUM_OBJ(PTmp))
	    Lens[i] = IRIT_REAL_TO_INT(PTmp -> U.R);
	else {
	    IritFree(Lens);

	    IRIT_NON_FATAL_ERROR("MBEZIER: Dimensions' list contains non-numeric");

	    return NULL;
	}
    }

    MVObj = GetControlMVMesh(n, Lens, Lens, PtLstObj,
			     MVAR_BEZIER_TYPE, &ErrStr);

    IritFree(Lens);

    if (MVObj == NULL) {
	IRIT_NON_FATAL_ERROR2("MBEZIER: %s, empty object result", ErrStr);
    }

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier trivar geometric object defined by a list of  M
* lists of lists of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   LensLstObj:    List of lengths in each of the dimensions of the MV.      M
*   PtLstObj:      A list of control points of size                          M
*			(LenLstObj[0] * LenLstObj[1] * .. LenLstObj[n-1]).   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier trivar object if successful, NULL otherwise.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPowerMVObject                                                         M
*****************************************************************************/
IPObjectStruct *GenPowerMVObject(IPObjectStruct *LensLstObj,
				 IPObjectStruct *PtLstObj)
{
    int i,
	n = IPListObjectLength(LensLstObj),
	*Lens = (int *) IritMalloc(sizeof(int *) * n);
    char *ErrStr;
    IPObjectStruct *MVObj;

    for (i = 0; i < n; i++) {
        IPObjectStruct
	    *PTmp = IPListObjectGet(LensLstObj, i);

	if (IP_IS_NUM_OBJ(PTmp))
	    Lens[i] = IRIT_REAL_TO_INT(PTmp -> U.R);
	else {
	    IritFree(Lens);

	    IRIT_NON_FATAL_ERROR("MPOWER: Dimensions' list contains non-numeric");

	    return NULL;
	}
    }

    MVObj = GetControlMVMesh(n, Lens, Lens, PtLstObj,
			     MVAR_POWER_TYPE, &ErrStr);

    IritFree(Lens);

    if (MVObj == NULL) {
	IRIT_NON_FATAL_ERROR2("MPOWER: %s, empty object result", ErrStr);
    }

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bspline trivar geometric object defined by a list    M
* of lists of lists of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   LensLstObj:    List of lengths in each of the dimensions of the MV.      M
*   OrdersLstObj:  List of orders in each of the dimensions of the MV.       M
*   PtLstObj:      A list of control points of size                          M
*			(LenLstObj[0] * LenLstObj[1] * .. LenLstObj[n-1]).   M
*   KVLstObj:      A list of knot vector lists in each of the dimensions of  M
*		   the MV.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bspline trivar object if successful, NULL otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBsplineMVObject                                                       M
*****************************************************************************/
IPObjectStruct *GenBsplineMVObject(IPObjectStruct *LensLstObj,
				   IPObjectStruct *OrdersLstObj,
				   IPObjectStruct *PtLstObj,
				   IPObjectStruct *KVLstObj)
{
    int i,
	n = IPListObjectLength(LensLstObj),
	*Lens = (int *) IritMalloc(sizeof(int *) * n),
	*Orders = (int *) IritMalloc(sizeof(int *) * n);
    char *ErrStr;
    IPObjectStruct *MVObj;

    if (IPListObjectLength(OrdersLstObj) != n ||
	IPListObjectLength(KVLstObj) != n) {
	IritFree(Lens);
	IritFree(Orders);

	IRIT_NON_FATAL_ERROR("MBSPLINE: Length of orders and lengths list not equal");

	return NULL;
    }

    for (i = 0; i < n; i++) {
        IPObjectStruct
	    *PTmp1 = IPListObjectGet(LensLstObj, i),
	    *PTmp2 = IPListObjectGet(OrdersLstObj, i);

	if (IP_IS_NUM_OBJ(PTmp1) && IP_IS_NUM_OBJ(PTmp2)) {
	    Lens[i] = IRIT_REAL_TO_INT(PTmp1 -> U.R);
	    Orders[i] = IRIT_REAL_TO_INT(PTmp2 -> U.R);
	}
	else {
	    IritFree(Lens);
	    IritFree(Orders);

	    IRIT_NON_FATAL_ERROR("MBSPLINE: lengths/orders list contains non-numeric");

	    return NULL;
	}

	if (Lens[i] < 2 || Orders[i] < Lens[i]) {
	    IritFree(Lens);
	    IritFree(Orders);

	    IRIT_NON_FATAL_ERROR("MBSPLINE: Invalid length or order");

	    return NULL;
	}
    }

    MVObj = GetControlMVMesh(n, Lens, Orders, PtLstObj,
			     MVAR_BSPLINE_TYPE, &ErrStr);

    if (MVObj == NULL) {
        IritFree(Lens);
	IritFree(Orders);

	IRIT_NON_FATAL_ERROR2("MBSPLINE: %s, empty object result.\n", ErrStr);
	return NULL;
    }
    else {
	for (i = 0; i < n; i++) {
	    IritFree(MVObj -> U.MultiVars -> KnotVectors[i]);
	    if ((MVObj -> U.MultiVars -> KnotVectors[i] =
		 GetKnotVector(IPListObjectGet(KVLstObj, i), Orders[i],
			       &Lens[i], &ErrStr, TRUE)) == NULL) {
	        IPFreeObject(MVObj);

		IritFree(Lens);
		IritFree(Orders);

		IRIT_NON_FATAL_ERROR2("MBSPLINE: Knot vector invalid, %s, empty object result.\n",
				      ErrStr);
		return NULL;
	    }

	    if (Lens[i] != MVObj -> U.MultiVars -> Lengths[i] +
		           MVObj -> U.MultiVars -> Orders[i]) {
	        if (Lens[i] == MVObj -> U.MultiVars -> Lengths[i] +
		               MVObj -> U.MultiVars -> Orders[i] +
		               MVObj -> U.MultiVars -> Orders[i]  - 1)
		    MVObj -> U.MultiVars -> Periodic[i] = TRUE;
		else {
		    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
		    return NULL;
		}
	    }
	}
    }

    IritFree(Lens);
    IritFree(Orders);

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the umbilical points of a given surface.                          M
*   The umbilicals are computed as the zeros of the function C = H^2 - K.    M
* C is never nagative and hence we actually solve for dC/du = dC/dv = 0 and  M
* test the values of C there.						     M
*   Hence (We only consider numerator of C which is sufficient for zeros),   M
*      C = H^2 - K = (2FM - EN - GL)^2 - 4(LN - M^2)(EG - F^2).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrfObj:   Surface to compute its umbilical points, if any.              M
*   SubTol:    Subdivision tolerance of computation.			     M
*   NumTol:    Numerical tolerance of computation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List of umbilical points of PSrfObj, nil if none.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserSrfUmbilicalPts                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfUmbilicPts                                                            M
*****************************************************************************/
IPObjectStruct *SrfUmbilicPts(IPObjectStruct *PSrfObj,
			      IrtRType *SubTol,
			      IrtRType *NumTol)
{
    int i = 0;
    IrtRType
	R = 0.0;
    MvarPtStruct *MVPt,
        *MVPts = UserSrfUmbilicalPts(PSrfObj -> U.Srfs, *SubTol, *NumTol);
    IPObjectStruct
	*PRetVal = IPGenLISTObject(NULL);

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        IPListObjectInsert(PRetVal, i++,
			   IPGenPTObject(&MVPt -> Pt[0],
					 &MVPt -> Pt[1],
					 &R));
    }
    MvarPtFreeList(MVPts);

    IPListObjectInsert(PRetVal, i, NULL);

    return PRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Approximates a given freeform curve using piecewise (bi)arcs.            M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:      The curve to piecewise biarc approximate.                     M
*   Tolerance: Of approximation.                                             M
*   MaxAngle:  Maximum angular span of arcs allowed.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    List of arcs with "center" point obj attributes     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvBiArcApprox                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvBiArcApprox                                                           M
*****************************************************************************/
IPObjectStruct *CrvBiArcApprox(IPObjectStruct *PCrv,
			       IrtRType *Tolerance,
			       IrtRType *MaxAngle)
{
    int i;
    SymbArcStruct *Arc,
	*Arcs = SymbCrvBiArcApprox(PCrv -> U.Crvs, *Tolerance, *MaxAngle);
    IPObjectStruct *PObj,
	*ObjArcList = IPGenLISTObject(NULL);

    
    for (Arc = Arcs, i = 0; Arc != NULL; Arc = Arc -> Pnext) {
        CagdCrvStruct *Crv;
	
	if (Arc -> Arc) {
	    CagdPtStruct Start, Center, End;

	    /* It is an arc. */

	    IRIT_PT_COPY(Start.Pt, Arc -> Pt1);
	    IRIT_PT_COPY(Center.Pt, Arc -> Cntr);
	    IRIT_PT_COPY(End.Pt, Arc -> Pt2);

	    Crv = BzrCrvCreateArc(&Start, &Center, &End);

	    PObj = IPGenCRVObject(Crv);

	    AttrSetObjectObjAttrib(PObj, "center",
				   IPGenPTObject(&Arc -> Cntr[0],
						 &Arc -> Cntr[1],
						 &Arc -> Cntr[2]), FALSE);
	    IPListObjectInsert(ObjArcList, i++, PObj);
	}
	else {
	    CagdPtStruct Pt1, Pt2;

	    /* It is a line segment. */

	    IRIT_PT_COPY(Pt1.Pt, Arc -> Pt1);
	    IRIT_PT_COPY(Pt2.Pt, Arc -> Pt2);
	    Crv = CagdMergePtPt(&Pt1, &Pt2);

	    PObj = IPGenCRVObject(Crv);

	    IPListObjectInsert(ObjArcList, i++, PObj);
	}
    }

    SymbArcFreeList(Arcs);

    IPListObjectInsert(ObjArcList, i, NULL);

    return ObjArcList;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the distance function square between the given two entities.	     M
*   Entities could be curves and/or surfaces.  If a curve and a surface are  M
* given, the curve must be first.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2:  Two entities to compute the distance function square      M
*                  between.		                                     M
*   RDistType:     0 for distance vector function,			     M
*		   1 for distance square function,			     M
*		   2 for distance vector projected on the normal of Entity1, M
*		   3 for distance vector projected on the normal of Entity2. M
*		   In cases 2 and 3 the normal is not normalized.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of intersection locations. If SelfInter is TRUE M
*                       assume PObj1 and PObj2 are the same and search for   M
*			self intersections.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DistanceTwoFreeforms                                                     M
*****************************************************************************/
IPObjectStruct *DistanceTwoFreeforms(IPObjectStruct *PObj1,
				     IPObjectStruct *PObj2,
				     IrtRType *RDistType)
{
    IPObjectStruct
	*DistObj = NULL;

    if (IP_IS_CRV_OBJ(PObj1)) {
        if (IP_IS_CRV_OBJ(PObj2)) {
	    CagdSrfStruct
		*DSqrSrf = SymbSrfDistCrvCrv(PObj1 -> U.Crvs, PObj2 -> U.Crvs,
					     IRIT_REAL_PTR_TO_INT(RDistType));

	    DistObj = IPGenSRFObject(DSqrSrf);
	}
	else if (IP_IS_SRF_OBJ(PObj2)) {
	    MvarMVStruct
		*DSqrMV = MvarMVDistCrvSrf(PObj1 -> U.Crvs, PObj2 -> U.Srfs,
					   IRIT_REAL_PTR_TO_INT(RDistType));

	    DistObj = IPGenMULTIVARObject(DSqrMV);
	}
    }
    else if (IP_IS_SRF_OBJ(PObj1)) {
        if (IP_IS_SRF_OBJ(PObj2)) {
	    MvarMVStruct
		*DSqrMV = MvarMVDistSrfSrf(PObj1 -> U.Srfs, PObj2 -> U.Srfs,
					   IRIT_REAL_PTR_TO_INT(RDistType));

	    DistObj = IPGenMULTIVARObject(DSqrMV);
	}
    }

    if (DistObj == NULL) {
        IRIT_NON_FATAL_ERROR("DIST2FF: None curves/surfaces as input");
    }

    return DistObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Compares two objects up to rigid motion and scale in the XY plane.         *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1, PObj2:  The two objects to compare.                               *
*   Eps:           Tolerance of equality.		                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A numeric zero object if failed or a list object     *
*                       with the Trans point, and scale and rotate factors.  *
*                                                                            *
* KEYWORDS:                                                                  *
*   FreeformCompareUptoRigidScale2D                                          *
*****************************************************************************/
IPObjectStruct *FreeformCompareUptoRigidScale2D(IPObjectStruct *PObj1,
						IPObjectStruct *PObj2,
						CagdRType *Eps)
{
    IrtPtType Trans;
    CagdRType Rot;
    CagdRType Scl;
    IPObjectStruct *PRet;
			     
    if (PObj1 -> ObjType != PObj2 -> ObjType)
	return IPGenNUMValObject(0);

    switch (PObj1 -> ObjType) {
	case IP_OBJ_CURVE:
	    if (!CagdCrvsSameUptoRigidScl2D(PObj1 -> U.Crvs,
					    PObj2 -> U.Crvs,
					    Trans, &Rot, &Scl, *Eps))
	        return IPGenNUMValObject(0);
	    break;
	case IP_OBJ_SURFACE:
	    if (!CagdSrfsSameUptoRigidScl2D(PObj1 -> U.Srfs,
					    PObj2 -> U.Srfs,
					    Trans, &Rot, &Scl, *Eps))
	        return IPGenNUMValObject(0);
	    break;
	default:
	    return IPGenNUMValObject(0);
    }

    PRet = IPGenLISTObject(IPGenPTObject(&Trans[0], &Trans[1], &Trans[2]));
    IPListObjectInsert(PRet, 1, IPGenNUMValObject(Rot));
    IPListObjectInsert(PRet, 2, IPGenNUMValObject(Scl));
    IPListObjectInsert(PRet, 3, NULL);

    return PRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a piecewise-cubics polynomial approximations to a given curve.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to approximate as piecewise cubics.                    M
*   Tol:        Tolerance of approximation.			             M
*   MaxLen:     An optional limit on the length of the cubics segements.     M
*		Non positive value to ignore.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of cubic segments.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvAsBzrCubics, ApproxCrvAsQuadratics	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ApproxCrvAsCubics                                                        M
*****************************************************************************/
IPObjectStruct *ApproxCrvAsCubics(IPObjectStruct *Crv,
				  IrtRType *Tol,
				  IrtRType *MaxLen)
{
    CagdCrvStruct
	*CubicCrvs = SymbApproxCrvAsBzrCubics(Crv -> U.Crvs, *Tol,
				        *MaxLen <= 0 ? IRIT_INFNTY : *MaxLen);

    return IPLnkListToListObject(CubicCrvs, IP_OBJ_CURVE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a piecewise-quadratics polynomial approximations to a given     M
* curve.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve to approximate as piecewise quadratics.                M
*   Tol:        Tolerance of approximation.			             M
*   MaxLen:     An optional limit on the length of the quadratics segements. M
*		Non positive value to ignore.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of quadratic segments.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvAsBzrQuadratics, ApproxCrvAsCubics                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   ApproxCrvAsQuadratics                                                    M
*****************************************************************************/
IPObjectStruct *ApproxCrvAsQuadratics(IPObjectStruct *Crv,
				      IrtRType *Tol,
				      IrtRType *MaxLen)
{
    CagdCrvStruct
	*QuadraticCrvs = SymbApproxCrvAsBzrQuadratics(Crv -> U.Crvs, *Tol,
				        *MaxLen <= 0 ? IRIT_INFNTY : *MaxLen);

    return IPLnkListToListObject(QuadraticCrvs, IP_OBJ_CURVE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates b-spline curve that fits (approximates) the given points      M
*   cloud using Squared Distance Minimization (SDM) algorith.                M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:  Points cloud we want to approximate.                         M
*   InitInfo:   Input data for fitting.  Can be either Curve or              M
*	        (Length, Order, Periodic), prescribing the input curve info. M
*   RFitType:   Fitting algorithm type:					     M
*               1 - CAGD_PDM_FITTING,					     M
*               2 - CAGD_SDM_FITTING.					     M
*   Constants:  Input algorithm constants list. May contain the following    M
*               constants in the following order: MaxIterations, ErrorLimit, M
*               ErrorChangeLimit, Lambda (regularization term weight).       M
*               Any missing constant is replaced with default value)         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Output B-spline curve.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvBsplineFitting, CagdCrvBsplineFittingIter                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvFitting, Squared Distance Minimization                             M
*****************************************************************************/
IPObjectStruct *BspCrvFitting(IPObjectStruct *PtObjList, 
                              IPObjectStruct *InitInfo,
                              IrtRType *RFitType,
                              IPObjectStruct *Constants)
{
    int NumOfPoints,
        FitType = IRIT_REAL_PTR_TO_INT(RFitType),
	MaxIterations = 10;
    CagdRType
	ErrorLimit = 0.001,				  /* Default values. */
        ErrorChangeLimit = ErrorLimit * 0.01,
        Lambda = 1e-7;
    CagdPType
	*PtList = ConvertListOfPtsToArray(PtObjList, &NumOfPoints);
    CagdCrvStruct
	*CagdCrv = NULL;
    IPObjectStruct *IPCrv;

    /* Handle constants list: MaxIter, ErrorLimit, ErrorChangeLimit, Lambda. */
    if (IP_IS_OLST_OBJ(Constants)) {
        IPObjectStruct *PCurObj;

        /* 0: MaxIterations, */
        if ((PCurObj = IPListObjectGet(Constants, 0)) != NULL && 
	    IP_IS_NUM_OBJ(PCurObj)) {
            MaxIterations = IRIT_REAL_TO_INT(PCurObj -> U.R);

	    /* 1: ErrorLimit. */
	    if ((PCurObj = IPListObjectGet(Constants, 1)) != NULL &&
		IP_IS_NUM_OBJ(PCurObj)) {
	        ErrorLimit = PCurObj -> U.R;
		ErrorChangeLimit = ErrorLimit * 0.01;        /* One percent. */

		/* 2: ErrorChangeLimit. */
		if ((PCurObj = IPListObjectGet(Constants, 2)) != NULL &&
		    IP_IS_NUM_OBJ(PCurObj)) {
		    ErrorChangeLimit = PCurObj -> U.R;

		    /* 3: Lambda. */
		    if ((PCurObj = IPListObjectGet(Constants, 3)) != NULL &&
			IP_IS_NUM_OBJ(PCurObj))
		        Lambda = PCurObj -> U.R;
		    else {
		        IRIT_WNDW_PUT_STR("CBSP_FIT: default values selected for \"(Lambda)\".");
			PCurObj = NULL;
		    }
		}
		else {
		    IRIT_WNDW_PUT_STR("CBSP_FIT: default values selected for \"(ErrorChangeLimit, Lambda)\".");
		    PCurObj = NULL;
		}
	    }
	    else {
	        IRIT_WNDW_PUT_STR("CBSP_FIT: default values selected for \"(ErrorLimit, ErrorChangeLimit, Lambda)\".");
		PCurObj = NULL;
	    }
	}
	else {
	    IRIT_WNDW_PUT_STR("CBSP_FIT: default values selected for \"(MaxIter, ErrorLimit, ErrorChangeLimit, Lambda)\".");
	    PCurObj = NULL;
	}
    }

    switch (FitType) {
        case 1:
	    FitType = CAGD_PDM_FITTING;
	    break;
        default:
	    IRIT_WNDW_PUT_STR("CBSP_FIT: Invalid fit type, SDM selected.");
        case 2:
	    FitType = CAGD_SDM_FITTING;
	    break;
    }

    /* Activate the fitting algoritm. */
    if (IP_IS_CRV_OBJ(InitInfo)) {
        CagdCrv = CagdBsplineCrvFittingWithInitCrv(PtList,
				                  NumOfPoints,
				                  InitInfo -> U.Crvs,
						  (CagdBspFittingType) FitType,
                                                  MaxIterations,
                                                  ErrorLimit,
                                                  ErrorChangeLimit,
                                                  Lambda);
    }
    else {
        IPObjectStruct
            *PObj1 = IPListObjectGet(InitInfo, 0),
            *PObj2 = IPListObjectGet(InitInfo, 1),
            *PObj3 = IPListObjectGet(InitInfo, 2);

	if (IP_IS_NUM_OBJ(PObj1) &&
	    IP_IS_NUM_OBJ(PObj2) &&
	    IP_IS_NUM_OBJ(PObj3)) {
	    CagdCrv = CagdBsplineCrvFitting(PtList, 
					    NumOfPoints,
					    IRIT_REAL_TO_INT(PObj1 -> U.R),
					    IRIT_REAL_TO_INT(PObj2 -> U.R),
					    IRIT_REAL_TO_INT(PObj3 -> U.R),
					    (CagdBspFittingType) FitType,
					    MaxIterations,
					    ErrorLimit,
					    ErrorChangeLimit,
					    Lambda);
	}
	else {
	    IRIT_WNDW_PUT_STR("CBSP_FIT: Wrong input.");
	    return NULL;
	}
    }

    IritFree(PtList);

    if (CagdCrv != NULL) {
        IPCrv = IPGenCRVObject(CagdCrv);

	return IPCrv;
    }
    else {
        IRIT_WNDW_PUT_STR("CBSP_FIT: Wrong input.");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the antipodal points of freeforms - opposite points with normals M
* along the exact same line, in the opposite direction.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   FF:          The freeform to process.  Eitehr a curve or a surface.      M
*   SubdivTol:   Tolerance of the subdivision stage.                         M
*   NumerTol:    Tolerance of the numerical improvement stage.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of E4 control points, defining one antipodal  M
*		point each as (u, v, s, t), having S(u, v) and S(s, t) the   M
*		two antipodal points.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeformAntipodalPoints                                                  M
*****************************************************************************/
IPObjectStruct *FreeformAntipodalPoints(IPObjectStruct *FF,
					IrtRType *SubdivTol,
					IrtRType *NumerTol)
{
    MvarPtStruct *MVPts;
    IPObjectStruct *CtlPts;

    if (IP_IS_CRV_OBJ(FF))
	MVPts = MvarCrvAntipodalPoints(FF -> U.Crvs, *SubdivTol, *NumerTol);
    else if (IP_IS_SRF_OBJ(FF))
        MVPts = MvarSrfAntipodalPoints(FF -> U.Srfs, *SubdivTol, *NumerTol);
    else {
        IRIT_WNDW_PUT_STR("AntiPodal: Wrong input.");
	return NULL;
    }

    CtlPts = MvarCnvrtMVPtsToCtlPts(MVPts, 0.0);
    MvarPtFreeList(MVPts);

    return CtlPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute self intersections in freeforms.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   FF:         The freeform to consider.                                    M
*   SubdivTol:  Tolerance of the subdivision stage.                          M
*   NumerTol:   Tolerance of the numerical improvement stage.                M
*   MinNrmlDeviation:   At the intersection points.  Zero for 90 degrees     M
*	minimum deviation, positive for smaller minimal deviation and	     M
*	negative for a larger minimal deviation.  A negative value invokes   M
*       a diagonal factor removal instead of the normal deviation test.      M
*   Euclidean:  TRUE for a result in the Euclidean space, FALSE for	     M
*	        parameter space.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The self intersection, as points (for curves) and     M
*		       piecewise linear curves (for surfaces).		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints, MvarSrfSelfInterNrmlDev                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeformSelfInter                                                        M
*****************************************************************************/
IPObjectStruct *FreeformSelfInter(IPObjectStruct *FF,
				  IrtRType *SubdivTol,
				  IrtRType *NumerTol,
				  IrtRType *MinNrmlDeviation,
				  IrtRType *Euclidean)
{
    int i, j;
    IrtRType *R;
    MvarPtStruct *MVPts;
    IPObjectStruct *Pt, *PtLst,
	*CtlPts = NULL;

    if (IP_IS_CRV_OBJ(FF)) {
	if (*MinNrmlDeviation < 0.0)
	    MVPts = MvarCrvSelfInterDiagFactor(FF -> U.Crvs, *SubdivTol,
					       *NumerTol);
	else
	    MVPts = MvarCrvSelfInterNrmlDev(FF -> U.Crvs, *SubdivTol,
					    *NumerTol, *MinNrmlDeviation);

	CtlPts = MvarCnvrtMVPtsToCtlPts(MVPts, 0.0);
	MvarPtFreeList(MVPts);

	if (!IRIT_APX_EQ(*Euclidean, 0.0)) {
	    for (i = 0; (Pt = IPListObjectGet(CtlPts, i++)) != NULL; ) {
	        R = CagdCrvEval(FF -> U.Crvs, Pt -> U.CtlPt.Coords[1]);
		CagdCoerceToE2(&Pt -> U.CtlPt.Coords[1],
			       &R, -1, FF -> U.Crvs -> PType);
		Pt -> U.CtlPt.PtType = CAGD_PT_E2_TYPE;
	    }
	}
    }
    else if (IP_IS_SRF_OBJ(FF)) {
	if (*MinNrmlDeviation < 0.0)
	    MVPts = MvarSrfSelfInterDiagFactor(FF -> U.Srfs, *SubdivTol,
					       *NumerTol);
	else
	    MVPts = MvarSrfSelfInterNrmlDev(FF -> U.Srfs, *SubdivTol,
					    *NumerTol, *MinNrmlDeviation);

	CtlPts = MvarCnvrtMVPtsToPolys(MVPts, NULL, 2 * *SubdivTol);
	MvarPtFreeList(MVPts);

	if (!IRIT_APX_EQ(*Euclidean, 0.0)) {
	    for (j = 0; (PtLst = IPListObjectGet(CtlPts, j++)) != NULL; ) {
	        for (i = 0; (Pt = IPListObjectGet(PtLst, i++)) != NULL; ) {
		    R = CagdSrfEval(FF -> U.Srfs,
				    Pt -> U.CtlPt.Coords[1],
				    Pt -> U.CtlPt.Coords[2]);
		    CagdCoerceToE3(&Pt -> U.CtlPt.Coords[1],
				   &R, -1, FF -> U.Srfs -> PType);
		    Pt -> U.CtlPt.PtType = CAGD_PT_E3_TYPE;
		}
	    }
	}
    }

    return CtlPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merges points in R^n (type En) into polylines in R^n.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PtsList:    A list of (control) points of arbitrary En dimension.        M
*               All points are assumed to be of the same size.               M
*   MergeTol:   maximal distance to still merge neighboring points.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Merged polylines as ordered list(s) of points.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCnvrtMVPtsToPolys			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   PointsToPolys                                                            M
*****************************************************************************/
IPObjectStruct *PointsToPolys(IPObjectStruct *PtsList, IrtRType *MergeTol)
{
  int i, n,
        Dim = 0;
    MvarPtStruct *MVPt,
        *MVPts = NULL;
    IPObjectStruct *PtObj, *PlyList;

    /* Convert the data. */
    for (n = 0; (PtObj = IPListObjectGet(PtsList, n++)) != NULL; ) {
        if (IP_IS_CTLPT_OBJ(PtObj)) {
	    if (Dim == 0)
	        Dim = CAGD_NUM_OF_PT_COORD(PtObj -> U.CtlPt.PtType);
	    else if (Dim != CAGD_NUM_OF_PT_COORD(PtObj -> U.CtlPt.PtType)) {
	        IRIT_WNDW_PUT_STR("PTS2PLYS: different point sizes detected.");
		MvarPtFreeList(MVPts);
		return NULL;
	    }

	    MVPt = MvarPtNew(Dim);
	    for (i = 0; i < Dim; i++)
	        MVPt -> Pt[i] = PtObj -> U.CtlPt.Coords[i + 1];
	}
	else if (IP_IS_POINT_OBJ(PtObj)) {
	    if (Dim == 0)
	        Dim = 3;
	    else if (Dim != 3) {
	        IRIT_WNDW_PUT_STR("PTS2PLYS: different point sizes detected.");
		MvarPtFreeList(MVPts);
		return NULL;
	    }

	    MVPt = MvarPtNew(Dim);
	    for (i = 0; i < Dim; i++)
	        MVPt -> Pt[i] = PtObj -> U.Pt[i];
	}
	else {
	    IRIT_WNDW_PUT_STR("PTS2PLYS: Only (control) points expected.");
	    MvarPtFreeList(MVPts);
	    return NULL;
	}

	IRIT_LIST_PUSH(MVPt, MVPts);
    }

    PlyList = MvarCnvrtMVPtsToPolys(MVPts, NULL, *MergeTol);
    MvarPtFreeList(MVPts);

    return PlyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a parametric curve, Crv, and a control point index CtlPtIdx,       M
* compute the curvature sign field of the curve as function of the Euclidean M
* locations of control point index CtlPtIdx. 				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:      To compute its curvature behaviour (convex vs. concave) as a  M
*	       function of the curve parameter and the Euclidean coordiate   M
*	       of the CtlPtIdx's control point.				     M
*   CtlPtIdx:  Index of control point to make a parameter for the curvature. M
*   Min, Max:  Domain each coordinate of CtlPtIdx point should vary.         M
*   SubdivTol, NumerTol:  Tolerance for the silhouette solving, if any.      M
*   Operation: 1. Returned is a multivariate of dimension "1 + Dim(Crv)",    M
*		  where Dim(Crv) is the dimension of curve (E2, E3, etc.).   M
*	       2. Extract the zero set of 1. using marching cubes.	     M
*              3. Computes the t's silhouette of the 1. by simultaneously    M
*		  solving for 1 and its derivative with respect to t.        M
*              4. Same as 3 but evaluate the result into Euclidean space.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either the multivariate (1 above) or its t's	     M
*			silhouette (2 above).				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvCrvtrByOneCtlPt, MvarCrvCrvtrByOneCtlPt, SymbCrv2DCurvatureSign,  M
*   MvarCrvMakeCtlPtParam			 			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCrvtrByOneCtlPt                                                       M
*****************************************************************************/
IPObjectStruct *CrvCrvtrByOneCtlPt(IPObjectStruct *PCrv,
				   IrtRType *CtlPtIdx,
				   IrtRType *Min,
				   IrtRType *Max,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol,
				   IrtRType *Operation)
{
    return UserCrvCrvtrByOneCtlPt(PCrv -> U.Crvs,
				  IRIT_REAL_PTR_TO_INT(CtlPtIdx),
				  *Min, *Max, *SubdivTol, *NumerTol,
				  IRIT_REAL_PTR_TO_INT(Operation));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reversed the second and third object, in place, in the Result object.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Result:   Object to swap its second and third objects, in place.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:                                                        *
*****************************************************************************/
static IPObjectStruct *ReverseDistParams(IPObjectStruct *Result)
{
    IPObjectStruct 
        *Prm1 = IPListObjectGet(Result, 1),
        *Prm2 = IPListObjectGet(Result, 2);

    IP_SET_OBJ_NAME2(Prm1, "Param2");
    IP_SET_OBJ_NAME2(Prm2, "Param1");

    IPListObjectInsert(Result, 2, Prm1);
    IPListObjectInsert(Result, 1, Prm2);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimal distance between points, curves, and surfaces.      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:     First point/curve/surface.                                    M
*   PObj2:     Second point/curve/surface.                                   M
*   Eps:       Tolerance of computation.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of 3 elements that includes:                  M
*			  1. The computed minimal distance.                  M
*			  2. The parameters in PObj1 at the minimal dist.    M
*			  3. The parameters in PObj2 at the minimal dist.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvCrvMinimalDist, MvarCrvSrfMinimalDist, MvarSrfSrfMinimalDist,     M
*   HausdorffDistance					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MinimalDistance                                                          M
*****************************************************************************/
IPObjectStruct *MinimalDistance(IPObjectStruct *PObj1,
				IPObjectStruct *PObj2,
				IrtRType *Eps)
{
    int i;
    CagdRType Dist, t, *R;
    CagdPType Pt1, Pt2;
    IPObjectStruct *PTmp, *PTmp2, *RetVal;
    MvarHFDistParamStruct Param1, Param2;
    MvarPtStruct *Pt;

    switch (PObj1 -> ObjType) {
	case IP_OBJ_POINT:
	case IP_OBJ_CTLPT:
	    if (PObj1 -> ObjType == IP_OBJ_CTLPT) {
	        PTmp = IPCoerceObjectTo(PObj1, IP_OBJ_POINT);
		IRIT_PT_COPY(Pt1, PTmp -> U.Pt);
		IPFreeObject(PTmp);
	    }
	    else {
		IRIT_PT_COPY(Pt1, PObj1 -> U.Pt);
	    }

	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		    if (PObj2 -> ObjType == IP_OBJ_CTLPT) {
		        PTmp = IPCoerceObjectTo(PObj2, IP_OBJ_POINT);
			IRIT_PT_COPY(Pt2, PTmp -> U.Pt);
			IPFreeObject(PTmp);
		    }
		    else {
		        IRIT_PT_COPY(Pt2, PObj2 -> U.Pt);
		    }
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = Param2.NumOfParams = 0;
		    break;
		case IP_OBJ_CURVE:
		    t = SymbDistCrvPoint(PObj2 -> U.Crvs, Pt1, TRUE, *Eps);
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 1;
		    Param2.T[0] = t;
		    R = CagdCrvEval(PObj2 -> U.Crvs, t);
		    CagdCoerceToE3(Pt2, &R, -1, PObj2 -> U.Crvs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = 0;
		    break;
		case IP_OBJ_SURFACE:
		    R = MvarDistSrfPoint(PObj2 -> U.Srfs, Pt1, TRUE,
					 IRIT_HFDIST_SUBDIV_TOL, *Eps);
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 2;
		    Param2.UV[0][0] = R[0];
		    Param2.UV[0][1] = R[1];
		    R = CagdSrfEval(PObj2 -> U.Srfs, R[0], R[1]);
		    CagdCoerceToE3(Pt2, &R, -1, PObj2 -> U.Srfs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = 0;

		    break;
	        default:
		    assert(0);
	    }
	    break;
	case IP_OBJ_CURVE:
	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		    return ReverseDistParams(MinimalDistance(PObj2, PObj1,
							     Eps));
		case IP_OBJ_CURVE:
		    Pt = MvarCrvCrvMinimalDist(PObj1 -> U.Crvs,
					       PObj2 -> U.Crvs, &Dist,
					       TRUE, *Eps);
		    Param1.NumOfParams = 1;
		    Param1.ManifoldDim = 1;
		    Param1.T[0] = Pt -> Pt[0];
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 1;
		    Param2.T[0] = Pt -> Pt[1];
		    MvarPtFree(Pt);
		    break;
		case IP_OBJ_SURFACE:
		    Pt = MvarCrvSrfMinimalDist(PObj2 -> U.Srfs,
					       PObj1 -> U.Crvs, &Dist);
		    Param1.NumOfParams = 1;
		    Param1.ManifoldDim = 1;
		    Param1.T[0] = Pt -> Pt[2];
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 2;
		    Param2.UV[0][0] = Pt -> Pt[0];
		    Param2.UV[0][1] = Pt -> Pt[1];
		    MvarPtFree(Pt);
		    break;
	        default:
		    assert(0);
	    }
	    break;
	case IP_OBJ_SURFACE:
	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		case IP_OBJ_CURVE:
		    return ReverseDistParams(MinimalDistance(PObj2, PObj1,
							     Eps));
		case IP_OBJ_SURFACE:
		    Pt = MvarSrfSrfMinimalDist(PObj1 -> U.Srfs,
					       PObj2 -> U.Srfs, &Dist);
		    Param1.NumOfParams = 1;
		    Param1.ManifoldDim = 2;
		    Param1.UV[0][0] = Pt -> Pt[0];
		    Param1.UV[0][1] = Pt -> Pt[1];
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 2;
		    Param2.UV[0][0] = Pt -> Pt[2];
		    Param2.UV[0][1] = Pt -> Pt[3];
		    MvarPtFree(Pt);
		    break;
	        default:
		    assert(0);
	    }
	    break;
	default:
	    IRIT_WNDW_PUT_STR("MinDist: Expecting points, curves, or surfaces only.");
	    return NULL;
    }

    /* Comvert Dist, Param1, Param2 to IPObjectStructs. */
    RetVal = IPGenListObject("MinDist",
			     IPGenNumObject("MinDist", &Dist, NULL), NULL);

    PTmp = IPGenListObject("Param1", NULL, NULL);
    for (i = Param1.NumOfParams - 1; i >= 0; i--) {
        switch (Param1.ManifoldDim) {
	    case 1:
	        IPListObjectAppend(PTmp,
				   IPGenNumObject("T", &Param1.T[i], NULL));
		break;
	    case 2:
	        PTmp2 = IPGenLISTObject(IPGenNumObject("U", &Param1.UV[i][0],
						       NULL));
		IPListObjectAppend(PTmp2,
				   IPGenNumObject("V", &Param1.UV[i][1],
						  NULL));
		IPListObjectAppend(PTmp, PTmp2);
	        break;
	}
    }
    IPListObjectAppend(RetVal, PTmp);

    PTmp = IPGenListObject("Param2", NULL, NULL);
    for (i = Param2.NumOfParams - 1; i >= 0; i--) {
        switch (Param2.ManifoldDim) {
	    case 1:
	        IPListObjectAppend(PTmp,
				   IPGenNumObject("T", &Param2.T[i], NULL));
		break;
	    case 2:
	        PTmp2 = IPGenLISTObject(IPGenNumObject("U", &Param2.UV[i][0],
						       NULL));
		IPListObjectAppend(PTmp2,
				   IPGenNumObject("V", &Param2.UV[i][1],
						  NULL));
		IPListObjectAppend(PTmp, PTmp2);
	        break;
	}
    }
    IPListObjectAppend(RetVal, PTmp);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Hausdorff distance between points, curves, and surfaces.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:     First point/curve/surface.                                    M
*   PObj2:     Second point/curve/surface.                                   M
*   Eps:       Tolerance of computation.                                     M
*   ROneSided: TRUE if one sided distance from PObj1 ro PObj2 is desired.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of 3 elements that includes:                  M
*			  1. The computed Hausdorff distance.                M
*			  2. The parameters in PObj1 at the maximal dist.    M
*			  3. The parameters in PObj2 at the maximal dist.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarHFDistPointCrvC1, MvarHFDistCrvCrvC1, MvarHFDistSrfCrvC1,	     M
*   MvarHFDistSrfSrfC1, MinimalDistance				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   HausdorffDistance                                                        M
*****************************************************************************/
IPObjectStruct *HausdorffDistance(IPObjectStruct *PObj1,
				  IPObjectStruct *PObj2,
				  IrtRType *Eps,
				  IrtRType *ROneSided)
{
    int i;
    CagdBType
	OneSided =  IRIT_REAL_PTR_TO_INT(ROneSided);
    CagdRType Dist, t, *R;
    CagdPType Pt1, Pt2;
    IPObjectStruct *PTmp, *PTmp2, *RetVal;
    MvarHFDistParamStruct Param1, Param2;

    switch (PObj1 -> ObjType) {
	case IP_OBJ_POINT:
	case IP_OBJ_CTLPT:
	    if (PObj1 -> ObjType == IP_OBJ_CTLPT) {
	        PTmp = IPCoerceObjectTo(PObj1, IP_OBJ_POINT);
		IRIT_PT_COPY(Pt1, PTmp -> U.Pt);
		IPFreeObject(PTmp);
	    }
	    else {
		IRIT_PT_COPY(Pt1, PObj1 -> U.Pt);
	    }

	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		    if (PObj2 -> ObjType == IP_OBJ_CTLPT) {
		        PTmp = IPCoerceObjectTo(PObj2, IP_OBJ_POINT);
			IRIT_PT_COPY(Pt2, PTmp -> U.Pt);
			IPFreeObject(PTmp);
		    }
		    else {
		        IRIT_PT_COPY(Pt2, PObj2 -> U.Pt);
		    }
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = Param2.NumOfParams = 0;
		    break;
		case IP_OBJ_CURVE:
		    t = SymbDistCrvPoint(PObj2 -> U.Crvs, Pt1, OneSided, *Eps);
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 1;
		    Param2.T[0] = t;
		    R = CagdCrvEval(PObj2 -> U.Crvs, t);
		    CagdCoerceToE3(Pt2, &R, -1, PObj2 -> U.Crvs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = 0;
		    break;
		case IP_OBJ_SURFACE:
		    R = MvarDistSrfPoint(PObj2 -> U.Srfs, Pt1, OneSided,
					 IRIT_HFDIST_SUBDIV_TOL, *Eps);
		    Param2.NumOfParams = 1;
		    Param2.ManifoldDim = 2;
		    Param2.UV[0][0] = R[0];
		    Param2.UV[0][1] = R[1];
		    R = CagdSrfEval(PObj2 -> U.Srfs, R[0], R[1]);
		    CagdCoerceToE3(Pt2, &R, -1, PObj2 -> U.Srfs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param1.NumOfParams = 0;
		    break;
	        default:
		    assert(0);
	    }
	    break;
	case IP_OBJ_CURVE:
	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		    if (PObj2 -> ObjType == IP_OBJ_CTLPT) {
		        PTmp = IPCoerceObjectTo(PObj2, IP_OBJ_POINT);
			IRIT_PT_COPY(Pt2, PTmp -> U.Pt);
			IPFreeObject(PTmp);
		    }
		    else {
		        IRIT_PT_COPY(Pt2, PObj2 -> U.Pt);
		    }
		    t = SymbDistCrvPoint(PObj1 -> U.Crvs, Pt2, FALSE, *Eps);
		    Param1.NumOfParams = 1;
		    Param1.ManifoldDim = 1;
		    Param1.T[0] = t;
		    R = CagdCrvEval(PObj1 -> U.Crvs, t);
		    CagdCoerceToE3(Pt1, &R, -1, PObj1 -> U.Crvs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param2.NumOfParams = 0;
		    break;
		case IP_OBJ_CURVE:
		    if (OneSided)
		        Dist = MvarHFDistFromCrvToCrvC1(PObj1 -> U.Crvs,
							PObj2 -> U.Crvs,
							&Param1, &Param2,
							*Eps);
		    else
		        Dist = MvarHFDistCrvCrvC1(PObj1 -> U.Crvs,
						  PObj2 -> U.Crvs,
						  &Param1, &Param2, *Eps);
		    break;
		case IP_OBJ_SURFACE:
		    if (OneSided)
		        Dist = MvarHFDistFromCrvToSrfC1(PObj1 -> U.Crvs,
							PObj2 -> U.Srfs,
							&Param1, &Param2);
		    else
		        Dist = MvarHFDistSrfCrvC1(PObj2 -> U.Srfs,
						  PObj1 -> U.Crvs,
						  &Param1, &Param2);
		    break;
	        default:
		    assert(0);
	    }
	    break;
	case IP_OBJ_SURFACE:
	    switch (PObj2 -> ObjType) {
	        case IP_OBJ_POINT:
	        case IP_OBJ_CTLPT:
		    if (PObj2 -> ObjType == IP_OBJ_CTLPT) {
		        PTmp = IPCoerceObjectTo(PObj2, IP_OBJ_POINT);
			IRIT_PT_COPY(Pt2, PTmp -> U.Pt);
			IPFreeObject(PTmp);
		    }
		    else {
		        IRIT_PT_COPY(Pt2, PObj2 -> U.Pt);
		    }

		    R = MvarDistSrfPoint(PObj1 -> U.Srfs, Pt1, FALSE,
					 IRIT_HFDIST_SUBDIV_TOL, *Eps);
		    Param1.NumOfParams = 1;
		    Param2.ManifoldDim = 2;
		    Param1.UV[0][0] = R[0];
		    Param1.UV[0][1] = R[1];
		    R = CagdSrfEval(PObj1 -> U.Srfs, R[0], R[1]);
		    CagdCoerceToE3(Pt1, &R, -1, PObj1 -> U.Srfs -> PType);
		    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
		    Param2.NumOfParams = 0;
		    break;
		case IP_OBJ_CURVE:
		    if (OneSided)
		        Dist = MvarHFDistFromSrfToCrvC1(PObj1 -> U.Srfs,
							PObj2 -> U.Crvs,
							&Param1, &Param2);
		    else
		        Dist = MvarHFDistSrfCrvC1(PObj1 -> U.Srfs,
						  PObj2 -> U.Crvs,
						  &Param1, &Param2);
		    break;
		case IP_OBJ_SURFACE:
		    if (OneSided)
		        Dist = MvarHFDistFromSrfToSrfC1(PObj1 -> U.Srfs,
							PObj2 -> U.Srfs,
							&Param1, &Param2);
		    else
		        Dist = MvarHFDistSrfSrfC1(PObj1 -> U.Srfs,
						  PObj2 -> U.Srfs,
						  &Param1, &Param2);
		    break;
	        default:
		    assert(0);
	    }
	    break;
	default:
	    IRIT_WNDW_PUT_STR("Hausdorff: Expecting points, curves, or surfaces only.");
	    return NULL;
    }

    /* Comvert Dist, Param1, Param2 to IPObjectStructs. */
    RetVal = IPGenListObject("HausdorffDist",
			     IPGenNumObject("HFDist", &Dist, NULL), NULL);

    PTmp = IPGenListObject("Param1", NULL, NULL);
    for (i = Param1.NumOfParams - 1; i >= 0; i--) {
        switch (Param1.ManifoldDim) {
	    case 1:
	        IPListObjectAppend(PTmp,
				   IPGenNumObject("T", &Param1.T[i], NULL));
		break;
	    case 2:
	        PTmp2 = IPGenLISTObject(IPGenNumObject("U", &Param1.UV[i][0],
						       NULL));
		IPListObjectAppend(PTmp2,
				   IPGenNumObject("V", &Param1.UV[i][1],
						  NULL));
		IPListObjectAppend(PTmp, PTmp2);
	        break;
	}
    }
    IPListObjectAppend(RetVal, PTmp);

    PTmp = IPGenListObject("Param2", NULL, NULL);
    for (i = Param2.NumOfParams - 1; i >= 0; i--) {
        switch (Param2.ManifoldDim) {
	    case 1:
	        IPListObjectAppend(PTmp,
				   IPGenNumObject("T", &Param2.T[i], NULL));
		break;
	    case 2:
	        PTmp2 = IPGenLISTObject(IPGenNumObject("U", &Param2.UV[i][0],
						       NULL));
		IPListObjectAppend(PTmp2,
				   IPGenNumObject("V", &Param2.UV[i][1],
						  NULL));
		IPListObjectAppend(PTmp, PTmp2);
	        break;
	}
    }
    IPListObjectAppend(RetVal, PTmp);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve(s), if any, between two surfaces.        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf1, PObjSrf2:   The two surface objects to consider.               M
*   Step:           Forward step size for numerical tracing stage.           M
*   SubdivTol:	    The subdivision tolerance to use.			     M
*   NumericTol:	    The numerical tolerance to use.			     M
*   Euclidean:      TRUE to return intersection curves in Euclidean space.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The intersection curve(s), if any, as piecewise      M
*                       linear curves.  Each intersection curve is returned  M
*			as a pair of curves, one for each surface.  If       M
*			Euclidean, the curves are also evaluated into R3.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfSrfInter2                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfSrfInter2                                                             M
*****************************************************************************/
IPObjectStruct *SrfSrfInter2(IPObjectStruct *PObjSrf1,
			     IPObjectStruct *PObjSrf2,
			     IrtRType *Step,
			     IrtRType *SubdivTol,
			     IrtRType *NumerTol,
			     IrtRType *Euclidean)
{
    int n = 0;
    CagdSrfStruct
        *Srf1 = PObjSrf1 -> U.Srfs,
        *Srf2 = PObjSrf2 -> U.Srfs;
    MvarPolyStruct *MVPl,
        *MVPlls = MvarSrfSrfInter2(Srf1, Srf2, *Step, *SubdivTol, *NumerTol);
    IPPolygonStruct
        *Plls = NULL;
    IPObjectStruct
        *PtsObj = NULL,
        *PllObj = NULL,
        *AllObj = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

    for (MVPl = MVPlls; MVPl != NULL; MVPl = MVPl -> Pnext) {
        CagdRType *R;
        MvarPtStruct *MVPt,
	    *MVPts = MVPl -> Pl;

	if (MVPts != NULL && MVPts -> Pnext == NULL) {
	    /* A single point of intersection.  Return a point object. */
	    if (IRIT_REAL_PTR_TO_INT(Euclidean)) {
		CagdPType Pt;

		R = CagdSrfEval(Srf1, MVPts -> Pt[0], MVPts -> Pt[1]);
		CagdCoerceToE3(Pt, &R, -1, Srf1 -> PType);
		PtsObj = IPGenPtObject("SSIPt", &Pt[0], &Pt[1], &Pt[2],
				       PtsObj);

		R = CagdSrfEval(Srf2, MVPts -> Pt[2], MVPts -> Pt[3]);
		CagdCoerceToE3(Pt, &R, -1, Srf1 -> PType);
		PtsObj = IPGenPtObject("SSIPt", &Pt[0], &Pt[1], &Pt[2],
				       PtsObj);
	    }
	    else {
	        CagdRType
		    Z = 0.0;

		PtsObj = IPGenPtObject("SSIPt",
				       &MVPts -> Pt[0], &MVPts -> Pt[1], &Z,
				       PtsObj);
		PtsObj = IPGenPtObject("SSIPt",
				       &MVPts -> Pt[2], &MVPts -> Pt[3], &Z,
				       PtsObj);
	    }
	}
	else {
	    IPVertexStruct *V1, *V2;
	    IPPolygonStruct
	        *Pl1 = IPAllocPolygon(0, NULL, NULL),
	        *Pl2 = IPAllocPolygon(0, NULL, NULL);

	    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	        V1 = IPAllocVertex2(Pl1 -> PVertex);
		Pl1 -> PVertex = V1;
		V2 = IPAllocVertex2(Pl2 -> PVertex);
		Pl2 -> PVertex = V2;

		if (IRIT_REAL_PTR_TO_INT(Euclidean)) {
		    R = CagdSrfEval(Srf1, MVPt -> Pt[0], MVPt -> Pt[1]);
		    CagdCoerceToE3(V1 -> Coord, &R, -1, Srf1 -> PType);
		    R = CagdSrfEval(Srf2, MVPt -> Pt[2], MVPt -> Pt[3]);
		    CagdCoerceToE3(V2 -> Coord, &R, -1, Srf2 -> PType);
		}
		else {
		    V1 -> Coord[0] = MVPt -> Pt[0];
		    V1 -> Coord[1] = MVPt -> Pt[1];
		    V1 -> Coord[2] = 0.0;
		    V2 -> Coord[0] = MVPt -> Pt[2];
		    V2 -> Coord[1] = MVPt -> Pt[3];
		    V2 -> Coord[2] = 0.0;
		}
	    }
	    Pl1 -> PVertex = IPReverseVrtxList2(Pl1 -> PVertex);
	    Pl2 -> PVertex = IPReverseVrtxList2(Pl2 -> PVertex);	

	    IRIT_LIST_PUSH(Pl2, Plls);
	    IRIT_LIST_PUSH(Pl1, Plls);
	}
    }

    MvarPolyFreeList(MVPlls);

    /* Do we have polylines in the output? */
    if (Plls != NULL) {
        PllObj = IPGenPOLYLINEObject(Plls);
	IPListObjectInsert(AllObj, n++, PllObj);
    }

    /* Do we have points in the output? */
    if (PtsObj != NULL) {
        PtsObj = IPObjLnkListToListObject(PtsObj);
	IPListObjectInsert(AllObj, n++, PtsObj);
    }

    IPListObjectInsert(AllObj, n, NULL);

    return AllObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes constant X contouring tool path to 3-axis machine from +Z       M
* direction the given geometry.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Object to process and create 3-axis machining tool path for. M
*   Offset:     Tool radius to offset the geometry in PObj with. 	     M
*   ZBaseLevel: Bottom level to machine.  Created tool path will never be    M
*	        below this level.					     M
*   TPathSpace: Space between adjacent paths.  Also serves as a tolerance.   M
*   RUnits:	0 for Inches, 1 for milimeters.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of constant X polylines, in zigzag motion, that M
*		covers PObj from above (offseted by Offset).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserNCContourToolPath                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   NCContourPath                                                            M
*****************************************************************************/
IPObjectStruct *NCContourPath(IPObjectStruct *PObj,
			      IrtRType *Offset,
			      IrtRType *ZBaseLevel,
			      IrtRType *TPathSpace,
			      IrtRType *RUnits)
{
    return UserNCContourToolPath(PObj, *Offset, *ZBaseLevel, *TPathSpace,
				 IRIT_REAL_PTR_TO_INT(RUnits));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes tool path to 2D pocket machining from +Z direction the given    M
* geometry (curves/polylines).  Geometry is assumed to be closed, possibly   M
* with closed islands.			                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Object to process and create 3-axis machining tool path for.M
*   ToolRadius:  Tool radius to offset the geometry in PObj with. 	     M
*   RoughOffset: Offset amount to use in the roughing stage.		     M
*   TPathSpace:  Space between adjacent parallel cut lines.  		     M
*   TPathJoin:   Maximal distance to two two tpaths into one.  		     M
*   RUnits: 	 0 for Inches, 1 for milimeters.			     M
*   TrimSelfInters:  TRUE to try and trim self intersections.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of constant X polylines, in zigzag motion, that M
*		covers PObj from above (offseted by Offset).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserNCContourToolPath                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   NCContourPath                                                            M
*****************************************************************************/
IPObjectStruct *NCPocketPath(IPObjectStruct *PObj,
			     IrtRType *ToolRadius,
			     IrtRType *RoughOffset,
			     IrtRType *TPathSpace,
			     IrtRType *TPathJoin,
			     IrtRType *RUnits,
			     IrtRType *TrimSelfInters)
{
    return UserNCPocketToolPath(PObj, *ToolRadius, *RoughOffset,
				*TPathSpace, *TPathJoin,
				IRIT_REAL_PTR_TO_INT(RUnits),
				IRIT_REAL_PTR_TO_INT(TrimSelfInters));
}
