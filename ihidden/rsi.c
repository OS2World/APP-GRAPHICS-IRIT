/*****************************************************************************
*   Computes all ray surface intersections for all curves in scene.          *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan. 2000   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "grap_lib.h"

#ifdef __OPENGL__
#    if defined(__WINNT__) || defined(__WINCE__)
#	include <windows.h>
#    endif /* __WINNT__ */
#    include <GL/gl.h>
#endif /* __OPENGL__ */

#define REL_POLY_FINENESS	4
#define Z_BUFFER_SAFE_Z		1
#define Z_BUFFER_NRML_SHIFT	0.0025
#define Z_BUFFER_EPS		0.0025
#define NUM_Z_TESTS_PER_CRV	2
#define IHIDDEN_TRI_SRF_FINENESS 50

#define OBJ_2_SCRN(x) ((int) ((x) * GlblScreenSize2 + GlblScreenSize2))

IRIT_STATIC_DATA VoidPtr
    GlblZbufferID = NULL;
IRIT_STATIC_DATA IrtRType
    GlblScreenSize2 = 100.0;

static void ProcessPolyObject(IPObjectStruct *PObj);
static IrtRType TestPointVisibility(IrtRType X, IrtRType Y);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Shoot rays from middle of each given curve, through the scene to verify  M
* its visibility, if any.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigGeom:  Original geometry to shoot rays against.                      M
*   Crvs:    Curves to examine visibility for.  The curves will have "ctype" M
*	     attributes that defines the curve type (one of 'iso', 'bndry',  M
*	     'sil', and 'discont', an attribute that will be propagated	     M
*	     further.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RaySrfIntersections                                                      M
*****************************************************************************/
void RaySrfIntersections(IPObjectStruct *OrigGeom, CagdCrvStruct *Crvs)
{
    int i, l;
    IrtRType
        SrfTol = GlblIHidTolerance * 0.25;
    IPObjectStruct *PObj, *PolyObj;
    CagdCrvStruct *Crv;
    CagdSrfStruct **Srfs;

    /* Initialize the ZBuffer to render polygonal depth into. */
    GlblScreenSize2 = GlblScrnRSIFineness * 0.5;
    GlblZbufferID = GMZBufferInit(GlblScrnRSIFineness, GlblScrnRSIFineness);

    /* Create a polygonal approximation of the scene to test against. */
    if (!GlblQuiet)
        IRIT_INFO_MSG("Surfaces:      \b");

    for (l = 1, PObj = OrigGeom; PObj != NULL; PObj = PObj -> Pnext) {
	switch (PObj -> ObjType) {
	    case IP_OBJ_SURFACE:
		if (!GlblQuiet)
		    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", l++);

		PolyObj = IPGenPOLYObject(IPSurface2Polygons(
						       PObj -> U.Srfs, TRUE,
						       SrfTol,
						       FALSE, FALSE, TRUE));
		ProcessPolyObject(PolyObj);
		break;
	    case IP_OBJ_TRIMSRF:
		if (!GlblQuiet)
		    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", l++);

		PolyObj = IPGenPOLYObject(IPTrimSrf2Polygons(
						     PObj -> U.TrimSrfs, TRUE,
						     SrfTol,
						     FALSE, FALSE, TRUE));
		ProcessPolyObject(PolyObj);
		break;
	    case IP_OBJ_TRIVAR:
		if (!GlblQuiet)
		    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", l++);

		Srfs = TrivBndrySrfsFromTV(PObj -> U.Trivars);

		for (i = 0; i < 6; i++) {
		    PolyObj = IPGenPOLYObject(IPSurface2Polygons(Srfs[i],
						      TRUE, SrfTol,
						      FALSE, FALSE, TRUE));
		    ProcessPolyObject(PolyObj);
		    CagdSrfFree(Srfs[i]);
		    IPFreeObject(PolyObj);
		}
		PolyObj = NULL;
		break;
	    case IP_OBJ_TRISRF:
		if (!GlblQuiet)
		    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", l++);

		PolyObj = IPGenPOLYObject(IPTriSrf2Polygons(
						      PObj -> U.TriSrfs,
						      IHIDDEN_TRI_SRF_FINENESS,
						      FALSE, FALSE, FALSE));
		ProcessPolyObject(PolyObj);
		break;
	    default:
		PolyObj = NULL;
		break;
	}
	if (PolyObj != NULL)
	    IPFreeObject(PolyObj);
    }

    /* Go over all curves and test their visibility. */
    if (!GlblQuiet)
        IRIT_INFO_MSG_PRINTF(", Curves:      \b");

    for (l = 1, Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        int n,
	    IsHidden = 0;
	CagdRType *R, t, Depth, TMin, TMax;
	CagdPType PtE3;
        IPObjectStruct *OrigObj;
	CagdCrvStruct *UVCrv;

	if (!GlblQuiet)
	    IRIT_INFO_MSG_PRINTF("\b\b\b\b%4d", l++);

	for (n = 1; n <= NUM_Z_TESTS_PER_CRV; n++) {
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    t = n / (NUM_Z_TESTS_PER_CRV + 1.0);
	    t = TMin * t + TMax * (1 - t);
	    R = CagdCrvEval(Crv, t);
	    CagdCoerceToE3(PtE3, &R, -1, Crv -> PType);

	    Depth = TestPointVisibility(PtE3[0], PtE3[1]);

	    if ((UVCrv = (CagdCrvStruct *) AttrGetPtrAttrib(Crv -> Attr,
							"_Uv")) != NULL &&
		(OrigObj = (IPObjectStruct *) AttrGetPtrAttrib(Crv -> Attr,
						       "_OrigObj")) != NULL) {
	        CagdRType Depth1, Depth2, UV[2];
		CagdVecStruct
		    *Nrml = NULL;

		/* Examine it while moving along the surface normal as well. */
		CagdCrvDomain(UVCrv, &TMin, &TMax);
		t = IRIT_BOUND(t, TMin, TMax);
		R = CagdCrvEval(UVCrv, t);
		CagdCoerceToE2(UV, &R, -1, UVCrv -> PType);

		switch (OrigObj -> ObjType) {
		    case IP_OBJ_SURFACE:
		        Nrml = CagdSrfNormal(OrigObj -> U.Srfs,
					     UV[0], UV[1], TRUE);
			break;
		    case IP_OBJ_TRIMSRF:
			Nrml = CagdSrfNormal(OrigObj -> U.TrimSrfs -> Srf,
					     UV[0], UV[1], TRUE);
			break;
		    case IP_OBJ_TRIVAR:
			break;
		    case IP_OBJ_TRISRF:
			Nrml = TrngTriSrfNrml(OrigObj -> U.TriSrfs,
					      R[1], R[2]);
			break;
		    default:
			break;
		}
		if (Nrml != NULL) {
		    Depth1 = TestPointVisibility(
			PtE3[0] + Nrml -> Vec[0] * Z_BUFFER_NRML_SHIFT,
			PtE3[1] + Nrml -> Vec[1] * Z_BUFFER_NRML_SHIFT);
		    Depth2 = TestPointVisibility(
			PtE3[0] - Nrml -> Vec[0] * Z_BUFFER_NRML_SHIFT,
			PtE3[1] - Nrml -> Vec[1] * Z_BUFFER_NRML_SHIFT);

		    Depth = IRIT_MIN(Depth, Depth1);
		    Depth = IRIT_MIN(Depth, Depth2);
		}
	    }
	    else {
	        IRIT_WARNING_MSG(
			"Warning: Failed to find UV data for a curve.\n");
	    }

	    if (PtE3[2] + Z_BUFFER_EPS < Depth)
	        IsHidden++;
	}

	if (IsHidden >= NUM_Z_TESTS_PER_CRV)
	    AttrSetIntAttrib(&Crv -> Attr, "hidden", TRUE);
    }

    GMZBufferFree(GlblZbufferID);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests the visibility of given point.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   X, Y:      tests the visibility of the given Euclidean point in screen.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  Detected depth at that XY location.                           *
*****************************************************************************/
static IrtRType TestPointVisibility(IrtRType X, IrtRType Y)
{
    return GMZBufferQueryZ(GlblZbufferID,
			   (int) (X * GlblScreenSize2 + GlblScreenSize2),
			   (int) (Y * GlblScreenSize2 + GlblScreenSize2));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Render the polygonal representation into the Z buffer.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Polygonal object representing the freeform shape.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessPolyObject(IPObjectStruct *PObj)
{
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    /* Skip transparent objects and do not render them into the Z buffer. */
    if (!IP_ATTR_IS_BAD_REAL(AttrGetObjectRealAttrib(PObj, "transp")))
	return;

    PObj = GMConvertPolysToTriangles(PObj);

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct
	    *V1 = Pl -> PVertex,
	    *V2 = V1 -> Pnext,
	    *V3 = V2 -> Pnext;
	IrtRType
	    *R1 = V1 -> Coord,
	    *R2 = V2 -> Coord,
	    *R3 = V3 -> Coord;

	GMZBufferUpdateTri(GlblZbufferID,
			   OBJ_2_SCRN(R1[0]),
			   OBJ_2_SCRN(R1[1]),
			   R1[2],
			   OBJ_2_SCRN(R2[0]),
			   OBJ_2_SCRN(R2[1]),
			   R2[2],
			   OBJ_2_SCRN(R3[0]),
			   OBJ_2_SCRN(R3[1]),
			   R3[2]);
    }
    IPFreeObject(PObj);
}
