/******************************************************************************
* ff_krnl.c - Cumputation of kernel for freeforms.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 2002.					      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "attribut.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "bool_lib.h"
#include "ip_cnvrt.h"

#define USER_PARAB_LINE_EPS	1e-3

IRIT_STATIC_DATA jmp_buf LclLongJumpBuffer;    /* Trap fatal Boolean error. */

static void FFKrnlBoolFatalError(BoolFatalErrorType ErrID);
static IPObjectStruct *FFKrnlBooleanAND(IPObjectStruct *PObj1,
					IPObjectStruct *PObj2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the kernel of a freeform closed C^1 continuous surface.         M
*   The parabolic curves are computed and surface tangent planes swept along M
* these parabolic curves clip the volume, resulting with the kernel.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute its kernel.					     M
*   Tolerance:  Accuracy of parabolic piecewise linear approximation.        M
*   SkipRate:   Step size over the parabolic points, 1 to process them all.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A polyhedra approximating the kernel, or NULL if     M
*		empty set.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserCntrSrfWithPlane                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfKernel                                                            M
*****************************************************************************/
IPObjectStruct *UserSrfKernel(const CagdSrfStruct *Srf,
			      CagdRType Tolerance,
			      int SkipRate)
{
    IRIT_STATIC_DATA IrtPlnType
	GaussPlane = { 1.0, 0.0, 0.0, 1.050964e-12 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType MaxSize;
    CagdVType Sizes;
    CagdBBoxStruct BBox;
    CagdSrfStruct
	*GaussSrf = SymbSrfGaussCurvature(Srf, TRUE);
    IPPolygonStruct *Pl,
	*Plls = UserCntrSrfWithPlane(GaussSrf, GaussPlane, Tolerance);
    IPObjectStruct *PolyObj, *PlnObj;
    BoolFatalErrorFuncType
        OldBoolErrorFunc = BoolSetFatalErrorFunc(FFKrnlBoolFatalError);

    CagdSrfFree(GaussSrf);
    BspMultComputationMethod(OldInterpFlag);

    CagdSrfBBox(Srf, &BBox);
    IRIT_VEC_SUB(Sizes, BBox.Max, BBox.Min);
    MaxSize = IRIT_MAX(Sizes[0], IRIT_MAX(Sizes[1], Sizes[2])) * 4;

    /* Build the plane to make the trimmings with. */
    {
        int Rvrsd;
        IrtPtType Pt1, Pt2, Pt3, PtIn;

	Pt1[0] =  MaxSize * cos(M_PI / 6);
	Pt1[1] = -MaxSize * sin(M_PI / 6);
	Pt1[2] = 0;

	Pt2[0] = 0;
	Pt2[1] = MaxSize;
	Pt2[2] = 0;

	Pt3[0] = -MaxSize * cos(M_PI / 6);
	Pt3[1] = -MaxSize * sin(M_PI / 6);
	Pt3[2] = 0;

	PtIn[0] = 0;
	PtIn[1] = 0;
	PtIn[2] = 1;

	PlnObj = IPGenPOLYObject(PrimGenPolygon3Vrtx(Pt1, Pt2, Pt3,
						     PtIn, &Rvrsd, NULL));
    }

    /* Create a polygonal version of the surface. */
    PolyObj = IPGenPOLYObject(IPSurface2Polygons((CagdSrfStruct *) Srf,
						 FALSE, Tolerance,
						 FALSE, FALSE, FALSE));

    /* Trim the object with tangent planes at all parabolic points. */
    for (Pl = Plls; Pl != NULL && PolyObj -> U.Pl != NULL; Pl = Pl -> Pnext) {
        int Skip = SkipRate;
	IPVertexStruct
	    *V = Pl -> PVertex;

	for ( ; V != NULL && PolyObj -> U.Pl != NULL; V = V -> Pnext) {
	    if (--Skip == 0) {
	        CagdRType
		    *R = CagdSrfEval(Srf, V -> Coord[1], V -> Coord[2]);
		CagdPType Pos;
		CagdVecStruct
		    *Nrml = CagdSrfNormal(Srf, V -> Coord[1], V -> Coord[2],
					  TRUE);
		IrtHmgnMatType Mat1, Mat2;
		IPObjectStruct *NewPolyObj, *TPlnObj;

		CagdCoerceToE3(Pos, &R, -1, Srf -> PType);

		/* Map the plane and use Booleans to perform the trimming. */
		MatGenMatTrans(Pos[0], Pos[1], Pos[2], Mat1);
		GMGenMatrixZ2Dir(Mat2, Nrml -> Vec);
		MatMultTwo4by4(Mat1, Mat2, Mat1);
		TPlnObj = GMTransformObject(PlnObj, Mat1);

		if ((NewPolyObj = FFKrnlBooleanAND(PolyObj,
						   TPlnObj)) != NULL) {
		    if (NewPolyObj -> U.Pl == NULL) {
		        IPVertexStruct
			    *V = PolyObj -> U.Pl -> PVertex;
			CagdPType Pos2;

		        /* No intersection - make sure current kernel is in */
		        /* the right side of the plane or purge the kernel. */
			IRIT_PT_BLEND(Pos2, V -> Coord, V -> Pnext -> Coord, 0.5);
			IRIT_PT_SUB(Pos2, Pos2, Pos);
			if (IRIT_DOT_PROD(Pos2, Nrml -> Vec) > 0) {
			    /* Kernel on the right side of this plane. */
			    IPFreeObject(NewPolyObj);
			}
			else {
			    /* Kernel is empty. */
			    IPFreeObject(PolyObj);
			    PolyObj = NewPolyObj;
			}
		    }
		    else {
		        IPFreeObject(PolyObj);
			PolyObj = NewPolyObj;
		    }
		}

		IPFreeObject(TPlnObj);

		Skip = SkipRate;
	    }
	}
    }

    IPFreePolygonList(Plls);
    IPFreeObject(PlnObj);

    BoolSetFatalErrorFunc(OldBoolErrorFunc);

    /* Reset all vertices' normals in the result. */
    for (Pl = PolyObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex;

	do {
	    IP_RST_NORMAL_VRTX(V);
	    IP_RST_INTERNAL_VRTX(V);
	    V = V -> Pnext;
	}
	while (V != Pl -> PVertex);
    }

    return PolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Traps Bool_lib errors right here. Call back function of bool_lib.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrID:    Error number in bool_lib library.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FFKrnlBoolFatalError(BoolFatalErrorType ErrID)
{
    longjmp(LclLongJumpBuffer, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Boolean between two objects while we capture here failures.            *
*   No errors in the Booleans are broadcasted beyond the returned NULL.      *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1, PObj2:   The two objects to compute their AND.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Result of computation or NULL if error.             *
*****************************************************************************/
static IPObjectStruct *FFKrnlBooleanAND(IPObjectStruct *PObj1,
					IPObjectStruct *PObj2)
{
    if (setjmp(LclLongJumpBuffer) == 0) {    /* Its the setjmp itself call! */
        return BooleanAND(PObj1, PObj2);
    }
    else {                   /* We gain control from fatal error long jump. */
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the parabolic lines of the given surface, if any.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         A surface to derive its parabolic lines.                    M
*   Step:	 Step size for curve tracing.				     M
*   SubdivTol:	 The subdivision tolerance to use.			     M
*   NumericTol:	 The numerical tolerance to use.			     M
*   Euclidean:   TRUE to evaluate the parabolic lines into Euclidean space.  M
*		 Ignored if DecompSrfs is TRUE.				     M
*   DecompSrfs:  TRUE to decompose the given Srf into convex/concave and     M
*		 saddle regions.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Parabolic lines of Srf as a list of polylines        M
*		approximation, or NULL if none.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfparabolicSheets                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfParabolicLines                                                    M
*****************************************************************************/
IPObjectStruct *UserSrfParabolicLines(const CagdSrfStruct *Srf,
				      CagdRType Step,
				      CagdRType SubdivTol,
				      CagdRType NumericTol,
				      int Euclidean,
				      int DecompSrfs)
{
    CagdSrfStruct
        *GaussSrf = SymbSrfGaussCurvature(Srf, TRUE);
    MvarMVStruct
        *GaussMV = MvarSrfToMV(GaussSrf);
    IPObjectStruct *ParabLines;
    MvarPolyStruct
        *MVPlls = MvarMVUnivarInter(&GaussMV, Step, SubdivTol, NumericTol);

    if (DecompSrfs)
	Euclidean = FALSE;

    CagdSrfFree(GaussSrf);
    MvarMVFree(GaussMV);

    ParabLines = MvarCnvrtMVPolysToIritPolys2(MVPlls, TRUE);
    MvarPolyFreeList(MVPlls);

    if (ParabLines != NULL) {
        IPPolygonStruct *Pl;

#define DEBUG_DUMP_PL_END_POINTS
#ifdef DEBUG_DUMP_PL_END_POINTS
	for (Pl = ParabLines -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct
	        *V = Pl -> PVertex,
	        *VLast = IPGetLastVrtx(V);

	    fprintf(stderr,
		    "Polyline end points [%8f %8f %8f] to [%8f %8f %8f]\n",
		    V -> Coord[0], V -> Coord[1], V -> Coord[2],
		    VLast -> Coord[0], VLast -> Coord[1], VLast -> Coord[2]);
	    
	}
#endif /* DEBUG_DUMP_PL_END_POINTS */

	for (Pl = ParabLines -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct
	        *V = Pl -> PVertex;

	    for ( ; V != NULL; V = V -> Pnext) {
	        CagdRType
		    *R  = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);

		AttrSetUVAttrib(&V -> Attr, "uvvals",
				V -> Coord[0], V -> Coord[1]);
		CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	    }
	}

	/* Invoke twice to clean polylines of zero vertices from 1st call. */
	ParabLines -> U.Pl = GMCleanUpPolylineList(&ParabLines -> U.Pl,
						   USER_PARAB_LINE_EPS);
	ParabLines -> U.Pl = GMCleanUpPolylineList(&ParabLines -> U.Pl,
						   USER_PARAB_LINE_EPS);

	if (!Euclidean) {    /* Recover the UV values from the attributes. */
	    IPPolygonStruct
	        *Pl = ParabLines -> U.Pl;

	    for ( ; Pl != NULL; Pl = Pl -> Pnext) {
	        IPVertexStruct
		    *V = Pl -> PVertex;

		for ( ; V != NULL; V = V -> Pnext) {
		    const float
		        *UV = AttrGetUVAttrib(V -> Attr, "uvvals");

		    assert(UV != NULL);

		    V -> Coord[0] = UV[0];
		    V -> Coord[1] = UV[1];
		    V -> Coord[2] = 0.0;
		}
	    }
	}

	if (DecompSrfs) {
	    TrimSrfStruct
		*DSrfs = TrimSrfsFromContours(Srf, ParabLines -> U.Pl);

#	ifdef DEBUG
	    {
		IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugParabLnsTrimmedSrfs, FALSE) {
		    IPStderrObject(IPGenPOLYLINEObject(ParabLines -> U.Pl));
		    IPStderrObject(IPGenTRIMSRFObject(DSrfs));
		}
	    }
#	endif /* DEBUG */

	    IPFreeObject(ParabLines);

	    return DSrfs == NULL ? NULL :  IPGenTRIMSRFObject(DSrfs);
	}
    }

    return ParabLines;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the parabolic lines of the given surface, if any.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         A surface to derive its parabolic lines.                    M
*   Step:	 Step size for curve tracing.				     M
*   SubdivTol:	 The subdivision tolerance to use.			     M
*   NumericTol:	 The numerical tolerance to use.			     M
*   SheetExtent: Amount to extent the sheet from the parabolic lines.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Parabolic lines of Srf as a list of polylines        M
*		approximation, or NULL if none.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserSrfparabolicLines                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfParabolicSheets                                                   M
*****************************************************************************/
IPObjectStruct *UserSrfParabolicSheets(const CagdSrfStruct *Srf,
				       CagdRType Step,
				       CagdRType SubdivTol,
				       CagdRType NumericTol,
				       CagdRType SheetExtent)
{
    IPObjectStruct
        *ParabLines = UserSrfParabolicLines(Srf, Step, SubdivTol, NumericTol,
					    TRUE, FALSE);
    IPPolygonStruct *Pl,
	*ParabSheet = NULL;
    IPVertexStruct *V, *VPrev;

    for (Pl = ParabLines -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        if (Pl -> PVertex == NULL || Pl -> PVertex -> Pnext == NULL)
	    continue;

        for (VPrev = NULL, V = Pl -> PVertex;
	     V != NULL;
	     VPrev = V, V = V -> Pnext) {
	    const float
	        *UV = AttrGetUVAttrib(V -> Attr, "uvvals");
	    IrtVecType T, VSheet;
	    const CagdVecStruct *n;
	    IPObjectStruct *VSheetObj;

	    assert(UV != NULL);

	    /* Compute tangent T for each vertex and using Srf's normal,   */
	    /* n, derive a vector in the tangent plane of Srf orthogonal   */
	    /* to both T and n.  					   */
	    if (VPrev != NULL && V -> Pnext != NULL)
	        IRIT_VEC_SUB(T, V -> Pnext -> Coord, VPrev -> Coord)
	    else if (VPrev == NULL)
	        IRIT_VEC_SUB(T, V -> Pnext -> Coord, V -> Coord)
	    else /* V -> Pnext == NULL */
	        IRIT_VEC_SUB(T, V -> Coord, VPrev -> Coord);

	    n = CagdSrfNormal(Srf, UV[0], UV[1], FALSE);

	    IRIT_PT_COPY(V -> Normal, n -> Vec);

	    IRIT_CROSS_PROD(VSheet, n -> Vec, T);
	    IRIT_VEC_NORMALIZE(VSheet);
	    IRIT_VEC_SCALE(VSheet, SheetExtent);

	    VSheetObj = IPGenVECObject(&VSheet[0], &VSheet[1], &VSheet[2]);
	    AttrSetObjAttrib(&V -> Attr, "VSheet", VSheetObj, FALSE);
	}
        for (V = Pl -> PVertex; V -> Pnext != NULL; ) {
	    int VrtcsRvrsd;
	    IPVertexStruct
	        *VNext = V -> Pnext;
	    IrtVecType V1, V2, V3, V4, Vin;
	    IPObjectStruct
	        *VSheetObj = AttrGetObjAttrib(V -> Attr, "VSheet"),
	        *VNextSheetObj = AttrGetObjAttrib(V -> Pnext -> Attr, "VSheet");

	    assert(VSheetObj != NULL && VNextSheetObj != NULL);

	    /* For each pair of neighboring vertices, create two triangles */
	    /* spanning the sheet to the desired extent.	           */
	    IRIT_PT_ADD(V1, V -> Coord, VSheetObj -> U.Vec);
	    IRIT_PT_SUB(V2, V -> Coord, VSheetObj -> U.Vec);
	    IRIT_PT_ADD(V3, VNext -> Coord, VNextSheetObj -> U.Vec);
	    IRIT_PT_SUB(V4, VNext -> Coord, VNextSheetObj -> U.Vec);

	    IRIT_PT_ADD(Vin, V3, V -> Pnext -> Normal);
	    ParabSheet = PrimGenPolygon3Vrtx(V1, V2, V3, Vin,
					     &VrtcsRvrsd, ParabSheet);
	    ParabSheet = PrimGenPolygon3Vrtx(V4, V3, V2, Vin,
					     &VrtcsRvrsd, ParabSheet);

	    V = VNext;
	}
    }

    IPFreeObject(ParabLines);

    return IPGenPOLYObject(ParabSheet);
}
