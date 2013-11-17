/*****************************************************************************
* Conversion routines from curvesand surfaces to polygons and polylines.     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
*****************************************************************************/

#include "irit_sm.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "ip_cnvrt.h"

IRIT_GLOBAL_DATA IPFreeformConvStateStruct IPFFCState = {
    FALSE,   /* Talkative */
    FALSE,   /* DumpObjsAsPolylines */
    TRUE,    /* DrawFFGeom */
    FALSE,   /* DrawFFMesh */
    { IG_DEFAULT_NUM_OF_ISOLINES,
	  IG_DEFAULT_NUM_OF_ISOLINES,
	      IG_DEFAULT_NUM_OF_ISOLINES },
    IG_DEFAULT_SAMPLES_PER_CURVE,
    SYMB_CRV_APPROX_UNIFORM,  /* CrvApproxMethod */
    FALSE,   /* ShowIntrnal */
    FALSE,   /* CubicCrvsAprox */
    IG_DEFAULT_POLYGON_FINENESS, /* FineNess */
    FALSE,   /* ComputeUV */
    TRUE,    /* ComputeNrml */
    FALSE,   /* FourPerFlat */
    0,       /* OptimalPolygons */
    FALSE,   /* BBoxGrid */
    TRUE,    /* LinearOnePolyFlag */
    FALSE
};

static IPPolygonStruct *Curve2Polylines(CagdCrvStruct *Crv,
					int DrawCurve,
					int DrawCtlPoly,
					CagdRType TolSamples,
					SymbCrvApproxMethodType Method);
static IPPolygonStruct *Surface2Polylines(CagdSrfStruct *Srf,
					  int DrawSurface,
					  int DrawMesh,
					  int NumOfIsolines[2],
					  CagdRType TolSamples,
					  SymbCrvApproxMethodType Method);
static IPPolygonStruct *TSrf2Polylines(TrimSrfStruct *TrimSrf,
				       int DrawSurface,
				       int DrawMesh,
				       int NumOfIsolines[2],
				       CagdRType TolSamples,
				       SymbCrvApproxMethodType Method);
static IPPolygonStruct *TV2Polylines(TrivTVStruct *Trivar,
				     int DrawTrivar,
				     int DrawMesh,
				     int NumOfIsolines[3],
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method);
static IPPolygonStruct *TriangSrf2Polylines(TrngTriangSrfStruct *Srf,
					    int DrawSurface,
					    int DrawMesh,
					    int NumOfIsolines[3],
					    CagdRType TolSamples,
					    SymbCrvApproxMethodType Method);
static void GetRelResolutions(const IPObjectStruct *PObj,
			      IrtRType *RelativeFineNess,
			      int NumOfIsolines[3],
			      int RelNumOfIsolines[3]);

/*****************************************************************************
* DESCRIPTION:                                                               *
* Convert a curve into a polyline.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:           A curve to piecewise linear approximate.                  *
*   DrawCurve:     Do we want to draw the curve?                             *
*   DrawCtlPoly:   Do we want to draw its control polygon?                   *
*   TolSamples:    Tolerance of approximation error (Method = 2) or          *
*                  Number of samples to compute on polyline (Method = 0, 1). *
*   Method:        0 - TolSamples are set uniformly in parametric space,     *
*                  1 - TolSamples are set optimally, considering the curve's *
*		       curvature.					     *
*		   2 - TolSamples sets the maximum error allowed between the *
*		       piecewise linear approximation and original curve.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   Polyline(s) representing the curve and its          *
*                        control polygon.                                    *
*****************************************************************************/
static IPPolygonStruct *Curve2Polylines(CagdCrvStruct *Crv,
					int DrawCurve,
					int DrawCtlPoly,
					CagdRType TolSamples,
					SymbCrvApproxMethodType Method)
{
    IPPolygonStruct *Poly,
	*PolyHead =
	    DrawCurve ? IPCurve2Polylines(Crv, TolSamples, Method)
		      : NULL;

    if (DrawCtlPoly) {
	Poly = IPCurve2CtlPoly(Crv);
	Poly -> Pnext = PolyHead;
	PolyHead = Poly;
    }

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a single surface into polylines, NumOfIsolines isolines in each   *
* axis into a polyline object list.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:           A surface to piecewise linear approximate.                *
*   DrawSurface:   Do we want to draw the surface?                           *
*   DrawMesh:      Do we want to draw its control mesh?                      *
*   NumOfIsolines: Number of isocurves in each parametric direction.         *
*   TolSamples:    Tolerance of approximation error (Method = 2) or          *
*                  Number of samples to compute on polyline (Method = 0, 1). *
*   Method:        0 - TolSamples are set uniformly in parametric space,     *
*                  1 - TolSamples are set optimally, considering the curve's *
*		       curvature.					     *
*		   2 - TolSamples sets the maximum error allowed between the *
*		       piecewise linear approximation and original curve.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Polyline(s) representing the surface and its         *
*                       control mesh.	                                     *
*****************************************************************************/
static IPPolygonStruct *Surface2Polylines(CagdSrfStruct *Srf,
					  int DrawSurface,
					  int DrawMesh,
					  int NumOfIsolines[2],
					  CagdRType TolSamples,
					  SymbCrvApproxMethodType Method)
{
    IPPolygonStruct
	*PolyHead = DrawSurface ?
	    IPSurface2Polylines(Srf, NumOfIsolines, TolSamples, Method) :
	    NULL;

    if (DrawMesh)
	PolyHead = IPAppendPolyLists(IPSurface2CtlMesh(Srf), PolyHead);

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a single trimmed surface into polylines, NumOfIsolines isolines   *
* in each axis into a polyline object list.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:       A trimmed surface to piecewise linear approximate.        *
*   DrawSurface:   Do we want to draw the surface?                           *
*   DrawMesh:      Do we want to draw its control mesh?                      *
*   NumOfIsolines: Number of isocurves in each parametric direction.         *
*   TolSamples:    Tolerance of approximation error (Method = 2) or          *
*                  Number of samples to compute on polyline (Method = 0, 1). *
*   Method:        0 - TolSamples are set uniformly in parametric space,     *
*                  1 - TolSamples are set optimally, considering the curve's *
*		       curvature.					     *
*		   2 - TolSamples sets the maximum error allowed between the *
*		       piecewise linear approximation and original curve.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Polyline(s) representing the trimmed surface and its *
*                       control mesh.	                                     *
*****************************************************************************/
static IPPolygonStruct *TSrf2Polylines(TrimSrfStruct *TrimSrf,
				       int DrawSurface,
				       int DrawMesh,
				       int NumOfIsolines[2],
				       CagdRType TolSamples,
				       SymbCrvApproxMethodType Method)
{
    IPPolygonStruct
	*PolyHead = DrawSurface ?
	    IPTrimSrf2Polylines(TrimSrf, NumOfIsolines, TolSamples,
				Method, TRUE, TRUE) :
	    NULL;

    if (DrawMesh)
	PolyHead = IPAppendPolyLists(IPSurface2CtlMesh(TrimSrf -> Srf),
				     PolyHead);

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a single trivariate function into polylines, NumOfIsolines        *
* isolines in each axis into a polyline object list.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Trivar:        A trivariate function to piecewise linear approximate.    *
*   DrawTrivar:    Do we want to draw the trivariate?                        *
*   DrawMesh:      Do we want to draw its control mesh?                      *
*   NumOfIsolines: Number of isocurves in each parametric direction.         *
*   TolSamples:    Tolerance of approximation error (Method = 2) or          *
*                  Number of samples to compute on polyline (Method = 0, 1). *
*   Method:        0 - TolSamples are set uniformly in parametric space,     *
*                  1 - TolSamples are set optimally, considering the curve's *
*		       curvature.					     *
*		   2 - TolSamples sets the maximum error allowed between the *
*		       piecewise linear approximation and original curve.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Polyline(s) representing the trivariate and its      *
*                       control mesh.	                                     *
*****************************************************************************/
static IPPolygonStruct *TV2Polylines(TrivTVStruct *Trivar,
				     int DrawTrivar,
				     int DrawMesh,
				     int NumOfIsolines[3],
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method)
{
    IPPolygonStruct
	*PolyHead = DrawTrivar ?
	    IPTrivar2Polylines(Trivar, NumOfIsolines, TolSamples, Method) :
	    NULL;

    if (DrawMesh)
	PolyHead = IPAppendPolyLists(IPTrivar2CtlMesh(Trivar), PolyHead);

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a single triangular surface into polylines with SamplesPerCurve   *
* samples, NumOfIsolines isolines into a polyline object list.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   TriSrf:          A triangular surface to piecewise linear approximate.   *
*   DrawSurface:     Do we want to draw the surface?                         *
*   DrawMesh:        Do we want to draw its control mesh?                    *
*   NumOfIsolines[3]:Number of isocurves in each parametric direction.       *
*   TolSamples:      Tolerance of approximation error (Method = 2) or        *
*                   Number of samples to compute on polyline (Method = 0, 1).*
*   OptimalPolyline: Do we want optimal yet expensive sampling?              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Polyline(s) representing the triangular surface and  *
*                       its control mesh.	                             *
*****************************************************************************/
static IPPolygonStruct *TriangSrf2Polylines(TrngTriangSrfStruct *TriSrf,
					    int DrawSurface,
					    int DrawMesh,
					    int NumOfIsolines[3],
					    CagdRType TolSamples,
					    SymbCrvApproxMethodType Method)
{
    IPPolygonStruct
	*PolyHead = DrawSurface ?
	    IPTriSrf2Polylines(TriSrf, NumOfIsolines, TolSamples, Method) :
	    NULL;

    if (DrawMesh)
	PolyHead = IPAppendPolyLists(IPTriSrf2CtlMesh(TriSrf), PolyHead);

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a whole set of geometry to polylines, in place.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:     Crvs/Srfs/Trimmed Srfs/Trivariates read from a file by    M
*                  the irit parser.					     M
*   Talkative:     Do we want some more information printed.		     M
*   DrawGeom:      Do we want to draw the geometry?                          M
*   DrawMesh:      Do we want to draw its control mesh?                      M
*   NumOfIsolines: Number of isocurves in each parametric direction.         M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polyline(s) representing the trivariate and its      M
*                       control mesh.	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeForm2Polylines                                                     M
*****************************************************************************/
IPObjectStruct *IPFreeForm2Polylines(IPFreeFormStruct *FreeForms,
				     int Talkative,
				     int DrawGeom,
				     int DrawMesh,
				     int NumOfIsolines[3],
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method)
{
    int RelNumOfIsolines[3];
    IrtRType RelativeFineNess;
    IPObjectStruct *PObj,
	*CrvObjs = FreeForms -> CrvObjs,
	*SrfObjs = FreeForms -> SrfObjs,
	*TrimSrfObjs = FreeForms -> TrimSrfObjs,
	*TrivarObjs = FreeForms -> TrivarObjs,
	*TriSrfObjs = FreeForms -> TriSrfObjs,
	*ModelObjs = FreeForms -> ModelObjs,
	*MultiVarObjs = FreeForms -> MultiVarObjs;
    CagdCrvStruct *Crv, *Crvs;
    CagdSrfStruct *Srf, *Srfs;
    TrimSrfStruct *TrimSrf, *TrimSrfs;
    TrivTVStruct *TV, *TVs;
    TrngTriangSrfStruct *TriSrf, *TriSrfs;
    IPPolygonStruct *PPolygon;


    if (CrvObjs) {
	for (PObj = CrvObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing curve object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    Crvs = PObj -> U.Crvs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
		PPolygon = Curve2Polylines(Crv, DrawGeom, DrawMesh,
					   RelativeFineNess * TolSamples,
					   Method);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    CagdCrvFreeList(Crvs);
	}
    }

    if (SrfObjs) {
	for (PObj = SrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    Srfs = PObj -> U.Srfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		PPolygon = Surface2Polylines(Srf, DrawGeom, DrawMesh,
					     RelNumOfIsolines,
					     RelativeFineNess * TolSamples,
					     Method);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    CagdSrfFreeList(Srfs);
	}
    }

    if (TrimSrfObjs) {
	for (PObj = TrimSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trimmed surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    TrimSrfs = PObj -> U.TrimSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (TrimSrf = TrimSrfs;
		 TrimSrf != NULL;
		 TrimSrf = TrimSrf -> Pnext) {
		PPolygon = TSrf2Polylines(TrimSrf, DrawGeom, DrawMesh,
					  RelNumOfIsolines, 
					  RelativeFineNess * TolSamples,
					  Method);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrimSrfFreeList(TrimSrfs);
	}
    }

    if (TrivarObjs) {
	for (PObj = TrivarObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trivariate object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    TVs = PObj -> U.Trivars;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (TV = TVs; TV != NULL; TV = TV -> Pnext) {
		PPolygon = TV2Polylines(TV, DrawGeom, DrawMesh,
					RelNumOfIsolines,
					RelativeFineNess * TolSamples,
					Method);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrivTVFreeList(TVs);
	}
    }

    if (TriSrfObjs) {
        int SamplesPerCurve = Method == SYMB_CRV_APPROX_UNIFORM ?
			      (int) TolSamples : IG_DEFAULT_SAMPLES_PER_CURVE;

	for (PObj = TriSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing triangular surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    TriSrfs = PObj -> U.TriSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
		PPolygon = TriangSrf2Polylines(TriSrf, DrawGeom, DrawMesh,
					       RelNumOfIsolines,
					       RelativeFineNess *
					           SamplesPerCurve,
					       Method);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrngTriSrfFreeList(TriSrfs);
	}
    }

    if (ModelObjs) {
	IPObjectStruct *PObjTmp,
	    *PTrimSrfObjs = NULL;

	GetRelResolutions(ModelObjs, &RelativeFineNess,
			  NumOfIsolines, RelNumOfIsolines);

	/* Convert to a set of trimmed surfaces and call recursively. */
	for (PObj = ModelObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    TrimSrfStruct *TSrf;

	    TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

	    for (TSrf = TrimSrfs; TSrf != NULL; ) {
		TrimSrfStruct
		    *TSrfTmp = TSrf -> Pnext;

		TSrf -> Pnext = NULL;
		PObjTmp = IPGenTRIMSRFObject(TSrf);
		IRIT_LIST_PUSH(PObjTmp, PTrimSrfObjs);

		TSrf = TSrfTmp;
	    }
	}

	IPFreeObjectList(ModelObjs);
	FreeForms -> TrimSrfObjs = PTrimSrfObjs;
	FreeForms -> ModelObjs = NULL;

	return IPFreeForm2Polylines(FreeForms, Talkative, DrawGeom, DrawMesh,
				    RelNumOfIsolines, TolSamples, Method);
    }

    if (MultiVarObjs) {
	if (MultiVarObjs -> U.MultiVars -> Dim < 4) {   /* Crv, srf, trivar. */
	    MvarMVStruct
	        *MV = MultiVarObjs -> U.MultiVars;
	    IPObjectStruct *PTmp;

	    GetRelResolutions(MultiVarObjs, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    switch (MV -> Dim) {
	        case 1:
		    PTmp = FreeForms -> CrvObjs 
		         = IPGenCRVObject(MvarMVToCrv(MV));
		    break;
		case 2:
		    PTmp = FreeForms -> SrfObjs
		         = IPGenSRFObject(MvarMVToSrf(MV));
		    break;
		case 3:
		    PTmp = FreeForms -> TrivarObjs
		         = IPGenTRIVARObject(MvarMVToTV(MV));
		    break;
	        default:
		    assert(0);
		    PTmp = NULL;
		    break;
	    }
	    PTmp -> Attr = IP_ATTR_COPY_ATTRS(MultiVarObjs -> Attr);
	    FreeForms -> MultiVarObjs = NULL;
	    IPFreeObjectList(MultiVarObjs);

	    return IPFreeForm2Polylines(FreeForms, Talkative, DrawGeom,
					DrawMesh, RelNumOfIsolines,
					TolSamples, Method);
	}
    }

    return IPConcatFreeForm(FreeForms);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get relative resolutions for this specific object, out of attributes.    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:               Object to examine its resolutions' attributes.       *
*   RelativeFineNess:   Polyline fineness attribute or 1.0 if none.          *
*			Based on "crv_resolution" attribue.		     *
*   NumOfIsolines:      Old number of isolines to scale relatively.          *
*   RelNumOfIsolines:   New number of isolines to scale relatively.  Based   *
*			on the "num_of_isolines" attribute.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetRelResolutions(const IPObjectStruct *PObj,
			      IrtRType *RelativeFineNess,
			      int NumOfIsolines[3],
			      int RelNumOfIsolines[3])
{
    int i;
    IrtRType RelativeNumIsos;

    *RelativeFineNess = AttrGetObjectRealAttrib(PObj, "crv_resolution");
    if (IP_ATTR_IS_BAD_REAL(*RelativeFineNess))
        *RelativeFineNess = 1.0;

    RelativeNumIsos = AttrGetObjectRealAttrib(PObj, "num_of_isolines");
    if (!IP_ATTR_IS_BAD_REAL(RelativeNumIsos)) {
        for (i = 0; i < 3; i++)
	    RelNumOfIsolines[i] = (int) (NumOfIsolines[i] * RelativeNumIsos);
    }
    else {
	for (i = 0; i < 3; i++)
	    RelNumOfIsolines[i] = NumOfIsolines[i];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a whole set of geometry to cubic bezier curves, in place.         M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:     Crvs/Srfs/Trimmed Srfs/Trivariates read from a file by    M
*                  the irit parser.					     M
*   Talkative:     Do we want some more information printed.		     M
*   DrawGeom:      Do we want to draw the geometry?                          M
*   DrawMesh:      Do we want to draw its control mesh?                      M
*   NumOfIsolines: Number of isocurves in each parametric direction.         M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polyline(s) representing the trivariate and its      M
*                       control mesh.	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeForm2CubicBzr                                                      M
*****************************************************************************/
IPObjectStruct *IPFreeForm2CubicBzr(IPFreeFormStruct *FreeForms,
				    int Talkative,
				    int DrawGeom,
				    int DrawMesh,
				    int NumOfIsolines[3],
				    CagdRType TolSamples,
				    SymbCrvApproxMethodType Method)
{
    int RelNumOfIsolines[3];
    CagdRType RelativeFineNess;
    IPObjectStruct *PObj;
    IPObjectStruct
	*CrvObjs = FreeForms -> CrvObjs,
	*SrfObjs = FreeForms -> SrfObjs,
	*TrimSrfObjs = FreeForms -> TrimSrfObjs,
	*TrivarObjs = FreeForms -> TrivarObjs,
	*TriSrfObjs = FreeForms -> TriSrfObjs,
	*ModelObjs = FreeForms -> ModelObjs,
	*MultiVarObjs = FreeForms -> MultiVarObjs;
    IPPolygonStruct *PPolygon;
    CagdCrvStruct *CubicBzrCrvs;

    /* Convert curves and surfaces into cubic polynomials (bezier). */
    if (CrvObjs) {
	for (PObj = CrvObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing curve object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    PPolygon = NULL;
	    CubicBzrCrvs = IPCurvesToCubicBzrCrvs(PObj -> U.Crvs, &PPolygon,
						  DrawGeom, DrawMesh,
						  IRIT_INFNTY);

	    if (CubicBzrCrvs) {
		CagdCrvFreeList(PObj -> U.Crvs);
		PObj -> U.Crvs = CubicBzrCrvs;
		PObj -> ObjType = IP_OBJ_CURVE;
	    }
	    else
		PObj -> ObjType = IP_OBJ_UNDEF;

	    if (PPolygon) {
		PObj -> Pnext = IPAllocObject(IP_GET_OBJ_NAME(PObj),
					      IP_OBJ_POLY,
					      PObj -> Pnext);
		IP_SET_POLYLINE_OBJ(PObj -> Pnext);
		PObj = PObj -> Pnext;
		PObj -> U.Pl = PPolygon;
	    }
	}
    }

    if (SrfObjs) {
	for (PObj = SrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    PPolygon = NULL;
	    CubicBzrCrvs =
		IPSurfacesToCubicBzrCrvs(PObj -> U.Srfs, &PPolygon,
					 DrawGeom, DrawMesh, RelNumOfIsolines,
					 IRIT_INFNTY);
	    if (CubicBzrCrvs) {
		CagdSrfFreeList(PObj -> U.Srfs);
		PObj -> U.Crvs = CubicBzrCrvs;
		PObj -> ObjType = IP_OBJ_CURVE;
	    }
	    else
		PObj -> ObjType = IP_OBJ_UNDEF;

	    if (PPolygon) {
		PObj -> Pnext = IPAllocObject("", IP_OBJ_POLY, PObj -> Pnext);
		IP_SET_POLYLINE_OBJ(PObj -> Pnext);
		PObj = PObj -> Pnext;
		PObj -> U.Pl = PPolygon;
	    }
	}
    }

    if (TrimSrfObjs) {
	for (PObj = TrimSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trimmed surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    PPolygon = NULL;
	    CubicBzrCrvs =
		IPTrimSrfsToCubicBzrCrvs(PObj -> U.TrimSrfs, &PPolygon,
					 DrawGeom, DrawMesh, RelNumOfIsolines,
					 IRIT_INFNTY);

	    /* Get the trimming curves as well. */
	    PObj -> Pnext = IPAllocObject("", IP_OBJ_POLY, PObj -> Pnext);
	    IP_SET_POLYLINE_OBJ(PObj -> Pnext);
	    PObj -> Pnext -> U.Pl = IPTrimSrf2Polylines(PObj -> U.TrimSrfs,
							RelNumOfIsolines,
							TolSamples,
							Method,
							TRUE, FALSE);

	    if (CubicBzrCrvs) {
		TrimSrfFreeList(PObj -> U.TrimSrfs);
		PObj -> U.Crvs = CubicBzrCrvs;
		PObj -> ObjType = IP_OBJ_CURVE;
	    }
	    else
		PObj -> ObjType = IP_OBJ_UNDEF;

	    /* And finally the control mesh, if any. */
	    PObj = PObj -> Pnext;       /* Skip the trimming curves object. */
	    if (PPolygon) {
		PObj -> Pnext = IPAllocObject(IP_GET_OBJ_NAME(PObj),
					      IP_OBJ_POLY,
					      PObj -> Pnext);
		IP_SET_POLYLINE_OBJ(PObj -> Pnext);
		PObj = PObj -> Pnext;
		PObj -> U.Pl = PPolygon;
	    }
	}
    }

    if (TrivarObjs) {
	for (PObj = TrivarObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trivariate object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    PPolygon = NULL;
	    CubicBzrCrvs =
		IPTrivarToCubicBzrCrvs(PObj -> U.Trivars, &PPolygon,
				       DrawGeom, DrawMesh, RelNumOfIsolines,
				       IRIT_INFNTY);
	    if (CubicBzrCrvs) {
		TrivTVFreeList(PObj -> U.Trivars);
		PObj -> U.Crvs = CubicBzrCrvs;
		PObj -> ObjType = IP_OBJ_CURVE;
	    }
	    else
		PObj -> ObjType = IP_OBJ_UNDEF;

	    if (PPolygon) {
	        PObj -> Pnext = IPAllocObject(IP_GET_OBJ_NAME(PObj),
					      IP_OBJ_POLY,
					      PObj -> Pnext);
		IP_SET_POLYLINE_OBJ(PObj -> Pnext);
		PObj = PObj -> Pnext;
		PObj -> U.Pl = PPolygon;
	    }
	}
    }

    if (TriSrfObjs) {
	for (PObj = TriSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    GetRelResolutions(PObj, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    PPolygon = NULL;
	    CubicBzrCrvs =
		IPTriSrfsToCubicBzrCrvs(PObj -> U.TriSrfs, &PPolygon,
					DrawGeom, DrawMesh, RelNumOfIsolines,
					IRIT_INFNTY);
	    if (CubicBzrCrvs) {
		TrngTriSrfFreeList(PObj -> U.TriSrfs);
		PObj -> U.Crvs = CubicBzrCrvs;
		PObj -> ObjType = IP_OBJ_CURVE;
	    }
	    else
		PObj -> ObjType = IP_OBJ_UNDEF;

	    if (PPolygon) {
		PObj -> Pnext = IPAllocObject("", IP_OBJ_POLY, PObj -> Pnext);
		IP_SET_POLYLINE_OBJ(PObj -> Pnext);
		PObj = PObj -> Pnext;
		PObj -> U.Pl = PPolygon;
	    }
	}
    }

    if (ModelObjs) {
	IPObjectStruct *PObjTmp,
	    *PTrimSrfObjs = NULL;

	GetRelResolutions(ModelObjs, &RelativeFineNess,
			  NumOfIsolines, RelNumOfIsolines);

	/* Convert to a set of trimmed surfaces and call recursively. */
	for (PObj = ModelObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    TrimSrfStruct *TSrf,
		*TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

	    for (TSrf = TrimSrfs; TSrf != NULL; ) {
		TrimSrfStruct
		    *TSrfTmp = TSrf -> Pnext;

		TSrf -> Pnext = NULL;
		PObjTmp = IPGenTRIMSRFObject(TSrf);
		IRIT_LIST_PUSH(PObjTmp, PTrimSrfObjs);

		TSrf = TSrfTmp;
	    }
	}

	IPFreeObjectList(ModelObjs);
	FreeForms -> TrimSrfObjs = PTrimSrfObjs;
	FreeForms -> ModelObjs = NULL;

	return IPFreeForm2CubicBzr(FreeForms, Talkative, DrawGeom, DrawMesh,
				   RelNumOfIsolines, TolSamples, Method);
    }

    if (MultiVarObjs) {
	if (MultiVarObjs -> U.MultiVars -> Dim < 4) {   /* Crv, srf, trivar. */
	    MvarMVStruct
	        *MV = MultiVarObjs -> U.MultiVars;
	    IPObjectStruct *PTmp;

	    GetRelResolutions(MultiVarObjs, &RelativeFineNess,
			      NumOfIsolines, RelNumOfIsolines);

	    switch (MV -> Dim) {
	        case 1:
		    PTmp = FreeForms -> CrvObjs
		         = IPGenCRVObject(MvarMVToCrv(MV));
		    break;
		case 2:
		    PTmp = FreeForms -> SrfObjs
		         = IPGenSRFObject(MvarMVToSrf(MV));
		    break;
		case 3:
		    PTmp = FreeForms -> TrivarObjs
		         = IPGenTRIVARObject(MvarMVToTV(MV));
		    break;
	        default:
		    assert(0);
		    PTmp = NULL;
		    break;
	    }
	    PTmp -> Attr = IP_ATTR_COPY_ATTRS(MultiVarObjs -> Attr);
	    FreeForms -> MultiVarObjs = NULL;
	    IPFreeObjectList(MultiVarObjs);

	    return IPFreeForm2CubicBzr(FreeForms, Talkative, DrawGeom,
				       DrawMesh, RelNumOfIsolines,
				       TolSamples, Method);
	}
    }

    return IPConcatFreeForm(FreeForms);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a whole set of geometry to polygons, in place.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:        Crvs/Srfs/Trimmed Srfs/Trivariates read from a file by M
*                     the irit parser.				             M
*   Talkative:        Do we want some more information printed?		     M
*   FourPerFlat:      See IPSurface2Polygons.	                             M
*   FineNess:         See IPSurface2Polygons.	                             M
*   ComputeUV:        See IPSurface2Polygons.	                             M
*   ComputeNrml:      See IPSurface2Polygons.	                             M
*   Optimal:          See IPSurface2Polygons.	                             M
*   BBoxGrid:         Do we want bounding box values and grid estimation.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polygon/line(s) representing the geometry and its    M
*                       control mesh.	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFreeForm2Polygons                                                      M
*****************************************************************************/
IPObjectStruct *IPFreeForm2Polygons(IPFreeFormStruct *FreeForms,
				    int Talkative,
				    int FourPerFlat,
				    IrtRType FineNess,
				    int ComputeUV,
				    int ComputeNrml,
				    int Optimal,
				    int BBoxGrid)
{
    int LocalFourPerFlat;
    IrtRType RelativeFineNess;
    CagdSrfStruct *Srf, *Srfs;
    TrimSrfStruct *TrimSrf, *TrimSrfs;
    TrivTVStruct *TV, *TVs;
    TrngTriangSrfStruct *TriSrf, *TriSrfs;
    IPPolygonStruct *PPolygon;
    IPObjectStruct *PObj,
	*SrfObjs = FreeForms -> SrfObjs,
	*TrimSrfObjs = FreeForms -> TrimSrfObjs,
	*TrivarObjs = FreeForms -> TrivarObjs,
	*TriSrfObjs = FreeForms -> TriSrfObjs,
	*MultiVarObjs = FreeForms -> MultiVarObjs;

    if (SrfObjs) {
	for (PObj = SrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    CagdBBoxStruct BBox, TempBBox;
	    char GridStr[IRIT_LINE_LEN];

	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    Srfs = PObj -> U.Srfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYGON_OBJ(PObj);

	    LocalFourPerFlat = FourPerFlat;
		
	    if (AttrGetObjectStrAttrib(PObj, "twoperflat"))
		LocalFourPerFlat = FALSE;
	    if (AttrGetObjectStrAttrib(PObj, "fourperflat"))
		LocalFourPerFlat = TRUE;

	    RelativeFineNess = AttrGetObjectRealAttrib(PObj, "resolution");
	    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
		RelativeFineNess = 1.0;
		
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		IrtRType t;

		if (BBoxGrid) {
		    /* Generate bounding box to surfaces and estimate */
		    /* the grid size for it using GlblGridSize.	  */
		    if (Srf == Srfs)
			CagdSrfBBox(Srf, &BBox);
		    else {
			CagdSrfBBox(Srf, &TempBBox);
			CagdMergeBBox(&BBox, &TempBBox);
		    }
		}

		t = AttrGetObjectRealAttrib(PObj, "u_resolution");
		if (!IP_ATTR_IS_BAD_REAL(t))
		    AttrSetRealAttrib(&Srf -> Attr, "u_resolution", t);
		t = AttrGetObjectRealAttrib(PObj, "v_resolution");
		if (!IP_ATTR_IS_BAD_REAL(t))
		    AttrSetRealAttrib(&Srf -> Attr, "v_resolution", t);

		PPolygon = IPSurface2Polygons(Srf, LocalFourPerFlat,
					      RelativeFineNess * FineNess,
					      ComputeUV, ComputeNrml,
					      Optimal);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    CagdSrfFreeList(Srfs);

	    if (BBoxGrid) {
		IrtRType
		    Dx = BBox.Max[0] - BBox.Min[0],
		    Dy = BBox.Max[1] - BBox.Min[1],
		    Dz = BBox.Max[2] - BBox.Min[2],
		    M = IRIT_MAX(IRIT_MAX(Dx, Dy), Dz);
		int IDx = (int) (BBoxGrid * (Dx / M)),
		    IDy = (int) (BBoxGrid * (Dy / M)),
		    IDz = (int) (BBoxGrid * (Dz / M));
		    
		/* Save grid information derived from the surface bbox. */
		sprintf(GridStr, "%d %d %d",
			IDx > 0 ? IDx : 1,
			IDy > 0 ? IDy : 1,
			IDz > 0 ? IDz : 1);
		AttrSetObjectStrAttrib(PObj, "GridSize", GridStr);
		sprintf(GridStr, "%f %f %f %f %f %f",
			BBox.Min[0], BBox.Max[0],
			BBox.Min[1], BBox.Max[1],
			BBox.Min[2], BBox.Max[2]);
		AttrSetObjectStrAttrib(PObj, "BBox", GridStr);
	    }
	}
    }

    /* Converts models, if any, to freeform trimmed surfaces, in place. */
    if (IPProcessModel2TrimSrfs(FreeForms))
	TrimSrfObjs = FreeForms -> TrimSrfObjs;

    if (TrimSrfObjs) {
	for (PObj = TrimSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    CagdBBoxStruct BBox, TempBBox;
	    char GridStr[IRIT_LINE_LEN];

	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trimmed surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    TrimSrfs = PObj -> U.TrimSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYGON_OBJ(PObj);

	    LocalFourPerFlat = FourPerFlat;
		
	    if (AttrGetObjectStrAttrib(PObj, "twoperflat"))
		LocalFourPerFlat = FALSE;
	    if (AttrGetObjectStrAttrib(PObj, "fourperflat"))
		LocalFourPerFlat = TRUE;

	    RelativeFineNess = AttrGetObjectRealAttrib(PObj, "resolution");
	    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
		RelativeFineNess = 1.0;
		
	    for (TrimSrf = TrimSrfs;
		 TrimSrf != NULL;
		 TrimSrf = TrimSrf -> Pnext) {
		IrtRType t;

		if (BBoxGrid) {
		    /* Generate bounding box to surfaces and estimate */
		    /* the grid size for it using GlblGridSize.	  */
		    if (TrimSrf == TrimSrfs)
			CagdSrfBBox(TrimSrf -> Srf, &BBox);
		    else {
			CagdSrfBBox(TrimSrf -> Srf, &TempBBox);
			CagdMergeBBox(&BBox, &TempBBox);
		    }
		}

		t = AttrGetObjectRealAttrib(PObj, "u_resolution");
		if (!IP_ATTR_IS_BAD_REAL(t))
		    AttrSetRealAttrib(&TrimSrf -> Attr, "u_resolution", t);
		t = AttrGetObjectRealAttrib(PObj, "v_resolution");
		if (!IP_ATTR_IS_BAD_REAL(t))
		    AttrSetRealAttrib(&TrimSrf -> Attr, "v_resolution", t);

		PPolygon = IPTrimSrf2Polygons(TrimSrf, LocalFourPerFlat,
					      RelativeFineNess * FineNess,
					      ComputeUV, ComputeNrml,
					      Optimal);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrimSrfFreeList(TrimSrfs);

	    if (BBoxGrid) {
		IrtRType
		    Dx = BBox.Max[0] - BBox.Min[0],
		    Dy = BBox.Max[1] - BBox.Min[1],
		    Dz = BBox.Max[2] - BBox.Min[2],
		    M = IRIT_MAX(IRIT_MAX(Dx, Dy), Dz);
		int IDx = (int) (BBoxGrid * (Dx / M)),
		    IDy = (int) (BBoxGrid * (Dy / M)),
		    IDz = (int) (BBoxGrid * (Dz / M));
		    
		/* Save grid information derived from the surface bbox. */
		sprintf(GridStr, "%d %d %d",
			IDx > 0 ? IDx : 1,
			IDy > 0 ? IDy : 1,
			IDz > 0 ? IDz : 1);
		AttrSetObjectStrAttrib(PObj, "GridSize", GridStr);
		sprintf(GridStr, "%f %f %f %f %f %f",
			BBox.Min[0], BBox.Max[0],
			BBox.Min[1], BBox.Max[1],
			BBox.Min[2], BBox.Max[2]);
		AttrSetObjectStrAttrib(PObj, "BBox", GridStr);
	    }
	}
    }

    if (TrivarObjs) {
	for (PObj = TrivarObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    CagdBBoxStruct BBox, TempBBox;
	    char GridStr[IRIT_LINE_LEN];

	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing trivariate object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    TVs = PObj -> U.Trivars;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYGON_OBJ(PObj);

	    LocalFourPerFlat = FourPerFlat;
		
	    if (AttrGetObjectStrAttrib(PObj, "twoperflat"))
		LocalFourPerFlat = FALSE;
	    if (AttrGetObjectStrAttrib(PObj, "fourperflat"))
		LocalFourPerFlat = TRUE;
		
	    RelativeFineNess = AttrGetObjectRealAttrib(PObj, "resolution");
	    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
		RelativeFineNess = 1.0;

	    for (TV = TVs; TV != NULL; TV = TV -> Pnext) {
		if (BBoxGrid) {
		    /* Generate bounding box to surfaces and estimate */
		    /* the grid size for it using BBoxGrid.	      */
		    if (TV == TVs)
			TrivTVBBox(TV, &BBox);
		    else {
			TrivTVBBox(TV, &TempBBox);
			CagdMergeBBox(&BBox, &TempBBox);
		    }
		}
		PPolygon = IPTrivar2Polygons(TV, LocalFourPerFlat,
					     RelativeFineNess * FineNess,
					     ComputeUV, ComputeNrml,
					     Optimal);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrivTVFreeList(TVs);

	    if (BBoxGrid) {
		IrtRType
		    Dx = BBox.Max[0] - BBox.Min[0],
		    Dy = BBox.Max[1] - BBox.Min[1],
		    Dz = BBox.Max[2] - BBox.Min[2],
		    M = IRIT_MAX(IRIT_MAX(Dx, Dy), Dz);
		int IDx = (int) (BBoxGrid * (Dx / M)),
		    IDy = (int) (BBoxGrid * (Dy / M)),
		    IDz = (int) (BBoxGrid * (Dz / M));
		    
		/* Save grid information derived from the surface bbox. */
		sprintf(GridStr, "%d %d %d",
			IDx > 0 ? IDx : 1,
			IDy > 0 ? IDy : 1,
			IDz > 0 ? IDz : 1);
		AttrSetObjectStrAttrib(PObj, "GridSize", GridStr);
		sprintf(GridStr, "%f %f %f %f %f %f",
			BBox.Min[0], BBox.Max[0],
			BBox.Min[1], BBox.Max[1],
			BBox.Min[2], BBox.Max[2]);
		AttrSetObjectStrAttrib(PObj, "BBox", GridStr);
	    }
	}
    }

    if (TriSrfObjs) {
	for (PObj = TriSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    CagdBBoxStruct BBox, TempBBox;
	    char GridStr[IRIT_LINE_LEN];

	    if (Talkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
				     IP_GET_OBJ_NAME(PObj));

	    TriSrfs = PObj -> U.TriSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYGON_OBJ(PObj);

	    RelativeFineNess = AttrGetObjectRealAttrib(PObj, "resolution");
	    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
		RelativeFineNess = 1.0;

	    for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
		if (BBoxGrid) {
		    /* Generate bounding box to surfaces and estimate */
		    /* the grid size for it using GlblGridSize.	  */
		    if (TriSrf == TriSrfs)
			TrngTriSrfBBox(TriSrf, &BBox);
		    else {
			TrngTriSrfBBox(TriSrf, &TempBBox);
			CagdMergeBBox(&BBox, &TempBBox);
		    }
		}

		PPolygon = IPTriSrf2Polygons(TriSrf,
					       RelativeFineNess * FineNess,
					       ComputeUV, ComputeNrml,
					       Optimal);

		PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
	    }
	    TrngTriSrfFreeList(TriSrfs);

	    if (BBoxGrid) {
		IrtRType
		    Dx = BBox.Max[0] - BBox.Min[0],
		    Dy = BBox.Max[1] - BBox.Min[1],
		    Dz = BBox.Max[2] - BBox.Min[2],
		    M = IRIT_MAX(IRIT_MAX(Dx, Dy), Dz);
		int IDx = (int) (BBoxGrid * (Dx / M)),
		    IDy = (int) (BBoxGrid * (Dy / M)),
		    IDz = (int) (BBoxGrid * (Dz / M));
		    
		/* Save grid information derived from the surface bbox. */
		sprintf(GridStr, "%d %d %d",
			IDx > 0 ? IDx : 1,
			IDy > 0 ? IDy : 1,
			IDz > 0 ? IDz : 1);
		AttrSetObjectStrAttrib(PObj, "GridSize", GridStr);
		sprintf(GridStr, "%f %f %f %f %f %f",
			BBox.Min[0], BBox.Max[0],
			BBox.Min[1], BBox.Max[1],
			BBox.Min[2], BBox.Max[2]);
		AttrSetObjectStrAttrib(PObj, "BBox", GridStr);
	    }
	}
    }

    if (MultiVarObjs) {
	if (MultiVarObjs -> U.MultiVars -> Dim < 4) {   /* Crv, srf, trivar. */
	    MvarMVStruct
	        *MV = MultiVarObjs -> U.MultiVars;
	    IPObjectStruct *PTmp;

	    /* Convert, in place the multivariate into crv/srf/trivar. */
	    switch (MV -> Dim) {
	        case 1:
		    PTmp = IPGenCRVObject(MvarMVToCrv(MV));
	            IPReallocNewTypeObject(MultiVarObjs, IP_OBJ_CURVE);
		    FreeForms -> CrvObjs = MultiVarObjs;
		    FreeForms -> CrvObjs -> U.Crvs = PTmp -> U.Crvs;
		    PTmp -> U.Crvs = NULL;
		    break;
		case 2:
		    PTmp = IPGenSRFObject(MvarMVToSrf(MV));
	            IPReallocNewTypeObject(MultiVarObjs, IP_OBJ_SURFACE);
		    FreeForms -> SrfObjs = MultiVarObjs;
		    FreeForms -> SrfObjs -> U.Srfs = PTmp -> U.Srfs;
		    PTmp -> U.Srfs = NULL;
		    break;
		case 3:
		    PTmp = IPGenTRIVARObject(MvarMVToTV(MV));
	            IPReallocNewTypeObject(MultiVarObjs, IP_OBJ_TRIVAR);
		    FreeForms -> TrivarObjs = MultiVarObjs;
		    FreeForms -> TrivarObjs -> U.Trivars = PTmp -> U.Trivars;
		    PTmp -> U.Trivars = NULL;
		    break;
	        default:
		    assert(0);
		    PTmp = NULL;
		    break;
	    }
	    FreeForms -> MultiVarObjs = NULL;
	    IPFreeObject(PTmp);

	    return IPFreeForm2Polygons(FreeForms, Talkative, FourPerFlat,
				       FineNess, ComputeUV, ComputeNrml,
				       Optimal, BBoxGrid);
	}
    }

    return IPConcatFreeForm(FreeForms);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single freeform geometry to polylines/polygons, in    M
* place.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  A Crv/Srf/Trimmed Srf/Trivariate freeform geoemtry.               M
*   State: The way the freeform geometry should be converted.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Processed freeform geometry.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   FFCnvrtSrfsToBicubicBezier                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPConvertFreeForm, conversion                                            M
*****************************************************************************/
IPObjectStruct *IPConvertFreeForm(IPObjectStruct *PObj,
				  IPFreeformConvStateStruct *State)
{
    IRIT_STATIC_DATA int
	FirstTime = TRUE;
    IPFreeFormStruct FreeForms;
    int DumpObjsAsPolylines = State -> DumpObjsAsPolylines,
	OldBspMultUsingInter = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    IPObjectStruct
	*PObjNext = PObj -> Pnext;

    PObj -> Pnext = NULL;

    if (FirstTime) {
	FirstTime = FALSE;

	if (IPFFCState.LinearOnePolyFlag)
	    CagdSetLinear2Poly(CAGD_ONE_POLY_PER_COLIN);
	else
	    CagdSetLinear2Poly(CAGD_REG_POLY_PER_LIN);
    }

    FreeForms.CrvObjs = NULL;
    FreeForms.SrfObjs = NULL;
    FreeForms.TrimSrfObjs = NULL;
    FreeForms.TrivarObjs = NULL;
    FreeForms.TriSrfObjs = NULL;
    FreeForms.ModelObjs = NULL;
    FreeForms.MultiVarObjs = NULL;

    switch (PObj -> ObjType) {
	case IP_OBJ_CURVE:
	    FreeForms.CrvObjs = PObj;
	    DumpObjsAsPolylines = TRUE;
	    break;
	case IP_OBJ_SURFACE:
	    FreeForms.SrfObjs = PObj;
	    break;
	case IP_OBJ_TRIMSRF:
	    FreeForms.TrimSrfObjs = PObj;
	    break;
	case IP_OBJ_TRIVAR:
	    FreeForms.TrivarObjs = PObj;
	    break;
	case IP_OBJ_TRISRF:
	    FreeForms.TriSrfObjs = PObj;
	    break;
	case IP_OBJ_MODEL:
	    FreeForms.ModelObjs = PObj;
	    break;
	case IP_OBJ_MULTIVAR:
	    FreeForms.MultiVarObjs = PObj;
	    break;
	default:
	    IRIT_WARNING_MSG("Non freeform geometry detected\n");
	    BspMultComputationMethod(OldBspMultUsingInter);
	    return NULL;
    }

    if (IPFFCState.CubicCrvsAprox) {
	PObj = IPFreeForm2CubicBzr(&FreeForms,
				   State -> Talkative,
				   State -> DrawFFGeom,
				   State -> DrawFFMesh,
				   State -> NumOfIsolines,
				   State -> CrvApproxTolSamples,
				   State -> CrvApproxMethod);
    }
    else if (DumpObjsAsPolylines) {
	PObj = IPFreeForm2Polylines(&FreeForms,
				    State -> Talkative,
				    State -> DrawFFGeom,
				    State -> DrawFFMesh,
				    State -> NumOfIsolines,
				    State -> CrvApproxTolSamples,
				    State -> CrvApproxMethod);
    }
    else {
	IPObjectStruct
	    *MeshObj = NULL;
	int CnvrtToPolys = TRUE;

	if (State -> DrawFFMesh) {
	    IPFreeformConvStateStruct
		TState = *State;

	    TState.DrawFFGeom = FALSE;
	    TState.DumpObjsAsPolylines = TRUE;
	    MeshObj = IPConvertFreeForm(IPCopyObject(NULL, PObj, TRUE),
					&TState);
	}

	if (IPFFCState.SrfsToBicubicBzr && PObj -> ObjType == IP_OBJ_SURFACE) {
	    CagdSrfStruct *Srf,
		*NoConvertionSrfs = NULL,
		*ConvertedSrfs = NULL;

	    /* Convert all surfaces that can be converted to bicubic Bezier. */
	
	    for (Srf = PObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		CagdSrfStruct *TSrfs, *TNoSrfs;

		TSrfs = IPSurfacesToCubicBzrSrfs(Srf, &TNoSrfs);
		if (TSrfs != NULL && TNoSrfs != NULL) {
		    IRIT_WARNING_MSG("Both convertable and non convertable to bicubic Bezier surfaces in object\n");
		    IRIT_WARNING_MSG_PRINTF("named \"%s\", convertion is ignored.\n",
					    IP_GET_OBJ_NAME(PObj));
		    CagdSrfFreeList(TSrfs);
		    CagdSrfFreeList(TNoSrfs);
		    CagdSrfFreeList(ConvertedSrfs);
		    CagdSrfFreeList(NoConvertionSrfs);
		    ConvertedSrfs = NoConvertionSrfs = NULL;
		    break;
		}

		ConvertedSrfs = CagdListAppend(TSrfs, ConvertedSrfs);
		NoConvertionSrfs = CagdListAppend(TNoSrfs, NoConvertionSrfs);
	    }

	    if (ConvertedSrfs != NULL || NoConvertionSrfs != NULL) {
		if (ConvertedSrfs != NULL) {
		    CagdSrfFreeList(PObj -> U.Srfs);
		    PObj -> U.Srfs = ConvertedSrfs;
		    CnvrtToPolys = FALSE;
		}
		else {
		    CagdSrfFreeList(NoConvertionSrfs);
		}
	    }
	}

	if (CnvrtToPolys)
	    PObj = IPFreeForm2Polygons(&FreeForms,
				       State -> Talkative,
				       State -> FourPerFlat,
				       State -> FineNess,
				       State -> ComputeUV,
				       State -> ComputeNrml,
				       State -> OptimalPolygons,
				       State -> BBoxGrid);
	if (MeshObj) {
	    MeshObj -> Pnext = PObj -> Pnext;
	    PObj -> Pnext = MeshObj;
	}
    }

    BspMultComputationMethod(OldBspMultUsingInter);

    return IPAppendObjLists(PObj, PObjNext);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Maps the object according to the given matrix, in place.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to map, in place, according to Mat.                    M
*   Mat:       Transformation matrix.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPMapObjectInPlace                                                       M
*****************************************************************************/
void IPMapObjectInPlace(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IPObjectStruct
	*Pnext = PObj -> Pnext,
	*MappedPObj = GMTransformObject(PObj, Mat);

    IPFreeObjectSlots(PObj);
    if (PObj -> ObjName)
        IritFree(PObj -> ObjName);

    IRIT_GEN_COPY(PObj, MappedPObj, sizeof(IPObjectStruct));
    PObj -> Pnext = Pnext;

    IPFreeObjectBase(MappedPObj);
}
