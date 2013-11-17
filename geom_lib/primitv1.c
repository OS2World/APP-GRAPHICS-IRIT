/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to generate the geometric primitives defined in the system. The   *
* primitives currently defined are:					     *
* 1. BOX - main planes parallel box.					     *
* 2. GBOX - generalized box - 6 arbitrary planes.			     *
* 3. CYLIN - cylinder with any main direction.				     *
* 4. CONE, CONE2 - cone with any main direction (two bases).		     *
* 5. SPHERE								     *
* 6. TORUS - with any main direction.					     *
* 7. PLANE - non closed, single polygon object: circle with resolution edges *
* 8. POLY - directly define single polygon object by specifing its vertices. *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "mdl_lib.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_loc.h"

#define	MIN_RESOLUTION 4

IRIT_STATIC_DATA int
    GlblSetGeneratePrim = GM_GEN_PRIM_POLYS,
    GlblSurfaceRational = TRUE;
IRIT_STATIC_DATA IrtVecType
    GlblOrigin = { 0.0, 0.0, 0.0 };

IRIT_GLOBAL_DATA int
    _PrimGlblResolution = 16;

static void UpdateVertexNormal(IrtNrmlType Normal,
			       const IrtPtType Pt,
			       const IrtPtType InPt,
			       int Perpendicular,
			       const IrtPtType PerpPt);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the way primitives are constructed - as polygons, as a freeform     M
* surface, or as a model surface.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   SetGeneratePrimitive:      0 - polygonal primitive.                      M
*	        	       1 - surface primitie.                         M
*		               2 - model primitive.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old value of PolygonalPrimitive flag.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetSurfacePrimitiveRational, PrimGenBOXObject, PrimGenGBOXObject,    M
*   PrimGenCONEObject, PrimGenCONE2Object, PrimGenCYLINObject,		     M
*   PrimGenSPHEREObject, PrimGenTORUSObject				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimSetGeneratePrimType                                                  M
*****************************************************************************/
int PrimSetGeneratePrimType(int SetGeneratePrimitive)
{
    int OldVal = GlblSetGeneratePrim;

    GlblSetGeneratePrim = SetGeneratePrimitive;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the way surface primitives are constructed - as exact rational      M
* form or approximated polynomial (integral) form.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SurfaceRational:  TRUE for rational, FALSE for integral form.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old value of PolygonalPrimitive flag.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONEObject, PrimGenCONE2Object, PrimGenCYLINObject,		     M
*   PrimGenSPHEREObject, PrimGenTORUSObject				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimSetSurfacePrimitiveRational                                          M
*****************************************************************************/
int PrimSetSurfacePrimitiveRational(int SurfaceRational)
{
    int OldVal = GlblSurfaceRational;

    GlblSurfaceRational = SurfaceRational;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a BOX geometric object defined by Pt - the minimum     V
* 3d point, and Width - Dx Dy & Dz vector.		4		     V
*   Order of vertices is as                         5       7		     V
* follows in the picture:                           |   6   |		     V
*						    |   |   |		     V
* (Note vertex 0 is hidden behind edge 2-6)	    |	|   |		     V
*						    1   |   3                V
*							2		     V
*   All dimensions can be negative, denoting the reversed direction.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:          Low end corner of BOX.                                      M
*   WidthX:      Width of BOX (X axis).                                      M
*   WidthY:      Depth of BOX( Y axis).                                      M
*   WidthZ:      Height of BOX( Z axis).                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A BOX primitive                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenGBOXObject, PrimGenCONEObject,	     M
*   PrimGenCONE2Object, PrimGenCYLINObject, PrimGenSPHEREObject,	     M
*   PrimGenTORUSObject, PrimGenBOXWIREObject				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenBOXObject, box, primitives                                        M
*****************************************************************************/
IPObjectStruct *PrimGenBOXObject(const IrtVecType Pt,
				 IrtRType WidthX,
				 IrtRType WidthY,
				 IrtRType WidthZ)
{
    IrtVecType Dir1, Dir2, Dir3;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct
	        *Srf = CagdPrimBoxSrf(Pt[0], Pt[1], Pt[2],
				      Pt[0] + WidthX,
				      Pt[1] + WidthY,
				      Pt[2] + WidthZ);

	    return IPGenSRFObject(Srf);
	}
        case GM_GEN_PRIM_MDLS:
	{
	    MdlModelStruct
	        *Mdl = MdlPrimBox(Pt[0], Pt[1], Pt[2],
				  Pt[0] + WidthX,
				  Pt[1] + WidthY,
				  Pt[2] + WidthZ);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    IRIT_PT_RESET(Dir1);
	    Dir1[0] = WidthX;               /* Prepare direction vectors. */
	    IRIT_PT_RESET(Dir2);
	    Dir2[1] = WidthY;	                /* Parallel to main axes. */
	    IRIT_PT_RESET(Dir3);
	    Dir3[2] = WidthZ;		                /* For GBOX call. */

	    return PrimGenGBOXObject(Pt, Dir1, Dir2, Dir3);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a BOX Wireframe geometric object defined by Pt - the   V
* minimum 3d point, and Width - Dx Dy & Dz vector.	4		     V
*   Order of vertices is as                         5       7		     V
* follows in the picture:                           |   6   |		     V
*						    |   |   |		     V
* (Note vertex 0 is hidden behind edge 2-6)	    |	|   |		     V
*						    1   |   3                V
*							2		     V
*   All dimensions can be negative, denoting the reversed direction.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:          Low end corner of BOX.                                      M
*   WidthX:      Width of BOX (X axis).                                      M
*   WidthY:      Depth of BOX( Y axis).                                      M
*   WidthZ:      Height of BOX( Z axis).                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A BOX primitive                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenGBOXObject, PrimGenCONEObject,	     M
*   PrimGenCONE2Object, PrimGenCYLINObject, PrimGenSPHEREObject,	     M
*   PrimGenTORUSObject, PrimGenBOXObject				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenBOXWIREObject, box, primitives                                    M
*****************************************************************************/
IPObjectStruct *PrimGenBOXWIREObject(const IrtVecType Pt,
				     IrtRType WidthX,
				     IrtRType WidthY,
				     IrtRType WidthZ)
{
    int i, j, k;
    IrtVecType Crnrs[8];
    IPPolygonStruct *Pl;

    /* Create the eight corners of the box. */
    for (k = 0; k < 2; k++) {
	for (j = 0; j < 2; j++) {
	    for (i = 0; i < 2; i++) {
	        int Idx = i + j * 2 + k * 4;

		Crnrs[Idx][0] = Pt[0] + (i == 0 ? 0 : WidthX);
		Crnrs[Idx][1] = Pt[1] + (j == 0 ? 0 : WidthY);
		Crnrs[Idx][2] = Pt[2] + (k == 0 ? 0 : WidthZ);
	    }
	}
    }

    /* Create the wireframe box. */
    Pl = PrimGenPolyline4Vrtx(Crnrs[0], Crnrs[1], Crnrs[3], Crnrs[2], NULL);
    Pl = PrimGenPolyline4Vrtx(Crnrs[4], Crnrs[5], Crnrs[7], Crnrs[6], Pl);
    Pl = GMGenPolyline2Vrtx(Crnrs[0], Crnrs[4], Pl);
    Pl = GMGenPolyline2Vrtx(Crnrs[1], Crnrs[5], Pl);
    Pl = GMGenPolyline2Vrtx(Crnrs[2], Crnrs[6], Pl);
    Pl = GMGenPolyline2Vrtx(Crnrs[3], Crnrs[7], Pl);

    return IPGenPolylineObject("BboxFrame", Pl, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a GBOX geometric object defined by Pt - the minimum    M
* 3d point, and 3 direction Vectors Dir1, Dir2, Dir3.                        M
*   If two of the direction vectors are parallel the GBOX degenerates to     M
* a zero volume object. A NULL pointer is returned in that case.   	     M
* 							4		     V
* Order of vertices is as                           5       7		     V
* follows in the picture:                           |   6   |		     V
*						    |   |   |		     V
* (Note vertex 0 is hidden behind edge 2-6)	    |	|   |		     V
*						    1   |   3                V
*							2		     V
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:                Low end corner of GBOX.				     M
*   Dir1, Dir2, Dir3:  Three independent directional vectors to define GBOX. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A GBOX primitive.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenCONEObject,	     M
*   PrimGenCONE2Object, PrimGenCYLINObject, PrimGenSPHEREObject,	     M
*   PrimGenTORUSObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenGBOXObject, general box, box, primitives                          M
*****************************************************************************/
IPObjectStruct *PrimGenGBOXObject(const IrtVecType Pt,
				  const IrtVecType Dir1,
				  const IrtVecType Dir2,
				  const IrtVecType Dir3)
{
    int i, Rvrsd;
    IrtVecType Temp;
    IrtVecType V[8];				  /* Hold 8 vertices of BOX. */
    IPVertexStruct *PVertex;
    IPPolygonStruct *PPolygon;
    IPObjectStruct *PBox;

    GMVecCrossProd(Temp, Dir1, Dir2);
    if (IRIT_PT_APX_EQ_ZERO_EPS(Temp, IRIT_UEPS))
	return NULL;
    GMVecCrossProd(Temp, Dir2, Dir3);
    if (IRIT_PT_APX_EQ_ZERO_EPS(Temp, IRIT_UEPS))
	return NULL;
    GMVecCrossProd(Temp, Dir3, Dir1);
    if (IRIT_PT_APX_EQ_ZERO_EPS(Temp, IRIT_UEPS))
	return NULL;

    /* Also the 0..7 sequence is binary decoded such that bit 0 is Dir1, */
    /* bit 1 Dir2, and bit 2 is Dir3 increment:				 */
    for (i = 0; i < 8; i++) {
	IRIT_PT_COPY(V[i], Pt);

	if (i & 1)
	    IRIT_PT_ADD(V[i], V[i], Dir1);
	if (i & 2)
	    IRIT_PT_ADD(V[i], V[i], Dir2);
	if (i & 4)
	    IRIT_PT_ADD(V[i], V[i], Dir3);
    }

    PBox = IPGenPolyObject("", NULL, NULL);   /* Generate BOX object itself: */

    /* And generate the 6 polygons (Bottom, top and 4 sides in this order):  */
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[0], V[1], V[3], V[2], V[4],
				       &Rvrsd, PBox -> U.Pl);
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[6], V[7], V[5], V[4], V[0],
				       &Rvrsd, PBox -> U.Pl);
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[4], V[5], V[1], V[0], V[2],
				       &Rvrsd, PBox -> U.Pl);
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[5], V[7], V[3], V[1], V[0],
				       &Rvrsd, PBox -> U.Pl);
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[7], V[6], V[2], V[3], V[1],
				       &Rvrsd, PBox -> U.Pl);
    PBox -> U.Pl = PrimGenPolygon4Vrtx(V[6], V[4], V[0], V[2], V[3],
				       &Rvrsd, PBox -> U.Pl);

    /* Update the vertices normals using the polygon plane equation: */
    for (PPolygon = PBox -> U.Pl;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext) {
	PVertex = PPolygon -> PVertex;
	do {
	    IRIT_PT_COPY(PVertex -> Normal, PPolygon -> Plane);
	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != PPolygon -> PVertex);
    }

    return PBox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a CONE geometric object defined by Pt - the base       M
* 3d center point, Dir - the cone direction and height, and base radius R.   M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:        Center location of Base of CONE.                              M
*   Dir:       Direction and distance from Pt to apex of CONE.               M
*   R:         Radius of Base of the cone.                                   M
*   Bases:     0 for none, 1 for bottom, 2 for top, 3 for both.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A CONE Primitive.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONE2Object, PrimGenCYLINObject, PrimGenSPHEREObject,	     M
*   PrimGenTORUSObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenCONEObject, cone, primitives                                      M
*****************************************************************************/
IPObjectStruct *PrimGenCONEObject(const IrtVecType Pt,
				  const IrtVecType Dir,
				  IrtRType R,
				  int Bases)
{
    int i;
    IrtRType Angle, AngleStep;
    IrtPtType LastCirclePt, CirclePt, ApexPt;
    IrtVecType UnitDir;
    IrtNrmlType LastCircleNrml, CircleNrml, ApexNrml;
    IrtHmgnMatType Mat;
    IPVertexStruct *VBase, *PVertex;
    IPPolygonStruct *PBase;
    IPObjectStruct *PCone;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct *TSrf,
		*Srf = CagdPrimConeSrf(GlblOrigin, R, IRIT_PT_LENGTH(Dir),
				       GlblSurfaceRational,
				       (CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    TSrf = CagdSrfMatTransform(Srf, Mat);
	    CagdSrfFree(Srf);
	    Srf = TSrf;

	    CagdSrfTransform(Srf, Pt, 1.0);

	    if (GlblSetGeneratePrim == GM_GEN_PRIM_SRFS)
	        return IPGenSRFObject(Srf);
	    else {
	        MdlModelStruct
		    *Mdl = MdlModelNew(Srf, NULL, FALSE);

	        return IPGenMODELObject(Mdl);
	    }
	    break;
	}
        case GM_GEN_PRIM_MDLS:
	{
	    int OldVal = MdlStitchSelfSrfPrims(FALSE);
	    MdlModelStruct
	        *Mdl = MdlPrimCone(GlblOrigin, R,
				   IRIT_PT_LENGTH(Dir),
				   GlblSurfaceRational,
				   (CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    MdlModelMatTransform(Mdl, Mat);
	    MdlModelTransform(Mdl, Pt, 1.0);

	    MdlStitchSelfSrfPrims(OldVal);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    break;
    }

    GMGenTransMatrixZ2Dir(Mat, Pt, Dir, R);   /* Transform from unit circle. */

    IRIT_PT_COPY(ApexPt, Pt);		   /* Find the apex point: Pt + Dir. */
    IRIT_PT_ADD(ApexPt, ApexPt, Dir);
    IRIT_PT_COPY(UnitDir, Dir);
    IRIT_PT_NORMALIZE(UnitDir);

    PCone = IPGenPolyObject("", NULL, NULL);     /* Gen. CONE object itself: */
    /* Also allocate the base polygon header with first vertex on it: */
    PBase = IPAllocPolygon(0, VBase = IPAllocVertex2(NULL), NULL);

    LastCirclePt[0] = 1.0;		/* First point is allways Angle = 0. */
    LastCirclePt[1] = 0.0;
    LastCirclePt[2] = 0.0;
    MatMultPtby4by4(LastCirclePt, LastCirclePt, Mat);

    UpdateVertexNormal(LastCircleNrml, LastCirclePt, Pt, TRUE, ApexPt);

    IRIT_PT_COPY(VBase -> Coord, LastCirclePt);/* Update first pt in base pl.*/
    IRIT_PT_COPY(VBase -> Normal, UnitDir);

    AngleStep = M_PI_MUL_2 / _PrimGlblResolution;

    for (i = 1; i <= _PrimGlblResolution; i++) {  /* Pass whole base circle. */
        int Rvrsd;

	Angle = AngleStep * i;		     /* Prevent from additive error. */

	CirclePt[0] = cos(Angle);
	CirclePt[1] = sin(Angle);
	CirclePt[2] = 0.0;
	MatMultPtby4by4(CirclePt, CirclePt, Mat);

 	UpdateVertexNormal(CircleNrml, CirclePt, Pt, TRUE, ApexPt);

	PCone -> U.Pl = PrimGenPolygon3Vrtx(LastCirclePt, ApexPt, CirclePt,
					    Pt, &Rvrsd, PCone -> U.Pl);

	/* Update the normals for this cone side polygon vertices: */
	PVertex = PCone -> U.Pl -> PVertex;
	IRIT_PT_COPY(PVertex -> Normal, LastCircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	/* The apex normal is the average of the two base vertices: */
	IRIT_PT_ADD(ApexNrml, CircleNrml, LastCircleNrml);
	IRIT_PT_NORMALIZE(ApexNrml);
	IRIT_PT_COPY(PVertex -> Normal, ApexNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, CircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);

	/* And add this vertex to base polygon: */
	if (i == _PrimGlblResolution)  /* Its last point - make it circular. */
	    VBase -> Pnext = PBase -> PVertex;
	else {
	    VBase -> Pnext = IPAllocVertex2(NULL);
	    VBase = VBase -> Pnext;
	    IRIT_PT_COPY(VBase -> Normal, UnitDir);
	    IRIT_PT_COPY(VBase -> Coord, CirclePt);
	}

	/* Save pt in last pt for next time. */
	IRIT_PT_COPY(LastCirclePt, CirclePt);
	IRIT_PT_COPY(LastCircleNrml, CircleNrml);
    }

    if (Bases & 0x01) {
	/* Update base polygon plane equation. */
	IPUpdatePolyPlane2(PBase, ApexPt);
	IP_SET_CONVEX_POLY(PBase);	       /* Mark it as convex polygon. */
	PBase -> Pnext = PCone -> U.Pl;  /* And stick it into cone polygons. */
	PCone -> U.Pl = PBase;
    }
    else
        IPFreePolygon(PBase);

    return PCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a truncated CONE, CON2, geometric object defined by    M
* Pt - the base 3d center point, Dir - the cone direction and height, and    M
* two base radii R1 and R2.						     M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:      Center location of Base of CON2.                                M
*   Dir:     Direction and distance from Pt to center of other base of CON2. M
*   R1, R2:  Two base radii of the truncated CON2                            M
*   Bases:   0 for none, 1 for bottom, 2 for top, 3 for both.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A CON2 Primitive.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONEObject, PrimGenCYLINObject, PrimGenSPHEREObject,		     M
*   PrimGenTORUSObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenCONE2Object, cone, primitives                                     M
*****************************************************************************/
IPObjectStruct *PrimGenCONE2Object(const IrtVecType Pt,
				   const IrtVecType Dir,
				   IrtRType R1,
				   IrtRType R2,
				   int Bases)
{
    int i;
    IrtRType Angle, AngleStep;
    IrtPtType LastCirclePt, CirclePt, ApexPt, LastApexPt1, ApexPt1;
    IrtNrmlType LastCircleNrml, CircleNrml;
    IrtVecType InvDir, UnitDir;
    IrtHmgnMatType Mat1, Mat2;
    IPVertexStruct *VBase1, *VBase2, *PVertex;
    IPPolygonStruct *PBase1, *PBase2;
    IPObjectStruct *PCone;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct *TSrf,
	        *Srf = CagdPrimCone2Srf(GlblOrigin, R1, R2,
					IRIT_PT_LENGTH(Dir),
					GlblSurfaceRational,
					(CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    TSrf = CagdSrfMatTransform(Srf, Mat);
	    CagdSrfFree(Srf);
	    Srf = TSrf;

	    CagdSrfTransform(Srf, Pt, 1.0);

	    return IPGenSRFObject(Srf);
	}
        case GM_GEN_PRIM_MDLS:
	{
	    int OldVal = MdlStitchSelfSrfPrims(FALSE);
	    MdlModelStruct
	        *Mdl = MdlPrimCone2(GlblOrigin, R1, R2,
				    IRIT_PT_LENGTH(Dir),
				    GlblSurfaceRational,
				    (CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    MdlModelMatTransform(Mdl, Mat);
	    MdlModelTransform(Mdl, Pt, 1.0);

	    MdlStitchSelfSrfPrims(OldVal);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    break;
    }

    IRIT_PT_COPY(ApexPt, Pt);		   /* Find the apex point: Pt + Dir. */
    IRIT_PT_ADD(ApexPt, ApexPt, Dir);
    IRIT_PT_COPY(UnitDir, Dir);
    IRIT_PT_NORMALIZE(UnitDir);
    IRIT_PT_COPY(InvDir, UnitDir);
    IRIT_PT_SCALE(InvDir, -1.0);

    GMGenTransMatrixZ2Dir(Mat1, Pt, UnitDir, R1);/* Trans. from unit circle. */
    GMGenTransMatrixZ2Dir(Mat2, ApexPt, UnitDir, R2);

    PCone = IPGenPolyObject("", NULL, NULL);     /* Gen. CONE object itself: */
    /* Also allocate the base polygon header with first vertex on it: */
    PBase1 = IPAllocPolygon(0, VBase1 = IPAllocVertex2(NULL), NULL);
    PBase2 = IPAllocPolygon(0, VBase2 = IPAllocVertex2(NULL), NULL);

    /* First point is allways at Angle = 0. */
    LastCirclePt[0] = LastApexPt1[0] = 1.0;
    LastCirclePt[1] = LastApexPt1[1] = 0.0;
    LastCirclePt[2] = LastApexPt1[2] = 0.0;
    MatMultPtby4by4(LastCirclePt, LastCirclePt, Mat1);
    MatMultPtby4by4(LastApexPt1, LastApexPt1, Mat2);

    UpdateVertexNormal(LastCircleNrml, LastCirclePt, Pt, TRUE, ApexPt);

    IRIT_PT_COPY(VBase1 -> Coord, LastCirclePt);/* Update 1st pt in base1 plygn. */
    IRIT_PT_COPY(VBase1 -> Normal, UnitDir);
    IRIT_PT_COPY(VBase2 -> Coord, LastApexPt1);/* Update 1st pt in base2 polygon. */
    IRIT_PT_COPY(VBase2 -> Normal, InvDir);

    AngleStep = M_PI_MUL_2 / _PrimGlblResolution;

    for (i = 1; i <= _PrimGlblResolution; i++) {  /* Pass whole base circle. */
        int Rvrsd;

	Angle = AngleStep * i;		     /* Prevent from additive error. */

	CirclePt[0] = ApexPt1[0] = cos(Angle);
	CirclePt[1] = ApexPt1[1] = sin(Angle);
	CirclePt[2] = ApexPt1[2] = 0.0;
	MatMultPtby4by4(CirclePt, CirclePt, Mat1);
	MatMultPtby4by4(ApexPt1, ApexPt1, Mat2);

 	UpdateVertexNormal(CircleNrml, CirclePt, Pt, TRUE, ApexPt);

	PCone -> U.Pl = PrimGenPolygon4Vrtx(LastCirclePt, LastApexPt1, ApexPt1,
					    CirclePt, Pt, &Rvrsd,
					    PCone -> U.Pl);

	/* Update the normals for this cone side polygon vertices: */
	PVertex = PCone -> U.Pl -> PVertex;
	IRIT_PT_COPY(PVertex -> Normal, LastCircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, LastCircleNrml );
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, CircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, CircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);

	/* And add these vertices to base polygons: */
	if (i == _PrimGlblResolution) {/* Its last point - make it circular. */
	    VBase1 -> Pnext = PBase1 -> PVertex;
	    VBase2 -> Pnext = PBase2 -> PVertex;
	}
	else {
	    VBase1 -> Pnext = IPAllocVertex2(NULL);
	    VBase1 = VBase1 -> Pnext;
	    IRIT_PT_COPY(VBase1 -> Coord, CirclePt);
	    IRIT_PT_COPY(VBase1 -> Normal, UnitDir);
	    VBase2 -> Pnext = IPAllocVertex2(NULL);
	    VBase2 = VBase2 -> Pnext;
	    IRIT_PT_COPY(VBase2 -> Coord, ApexPt1);
	    IRIT_PT_COPY(VBase2 -> Normal, InvDir);
	}

	IRIT_PT_COPY(LastCirclePt, CirclePt);/* Save pt in last pt for next time. */
	IRIT_PT_COPY(LastApexPt1, ApexPt1);
	IRIT_PT_COPY(LastCircleNrml, CircleNrml);
    }

    if (Bases & 0x01) {
	/* Update base polygon plane equation. */
	IPUpdatePolyPlane2(PBase1, ApexPt);
	IP_SET_CONVEX_POLY(PBase1);	       /* Mark it as convex polygon. */
	PBase1 -> Pnext = PCone -> U.Pl;    /* And stick into cone polygons. */
	PCone -> U.Pl = PBase1;
    }
    else
        IPFreePolygon(PBase1);

    if (Bases & 0x02) {
	/* Update base polygon plane equation. */
	IPUpdatePolyPlane2(PBase2, Pt);
	IP_SET_CONVEX_POLY(PBase2);	       /* Mark it as convex polygon. */
	PBase2 -> Pnext = PCone -> U.Pl;    /* And stick into cone polygons. */
	PCone -> U.Pl = PBase2;
    }
    else
        IPFreePolygon(PBase2);

    return PCone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a CYLINder geometric object defined by Pt - the base   M
* 3d center point, Dir - the cylinder direction and height, and radius R.    M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Center location of Base of CYLINder.                         M
*   Dir:        Direction and distance from Pt to other base of cylinder.    M
*   R:          Radius of Base of the cylinder.                              M
*   Bases:     0 for none, 1 for bottom, 2 for top, 3 for both.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A CYLINDER Primitive.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONEObject, PrimGenCONE2Object, PrimGenSPHEREObject,		     M
    PrimGenTORUSObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenCYLINObject, cylinder, primitives                                 M
*****************************************************************************/
IPObjectStruct *PrimGenCYLINObject(const IrtVecType Pt,
				   const IrtVecType Dir,
				   IrtRType R,
				   int Bases)
{
    int i;
    IrtRType Angle, AngleStep;
    IrtPtType LastCirclePt, CirclePt, TLastCirclePt, TCirclePt, TPt, Dummy;
    IrtVecType ForwardDir, BackwardDir;
    IrtNrmlType LastCircleNrml, CircleNrml;
    IrtHmgnMatType Mat;
    IPVertexStruct *VBase1, *VBase2, *PVertex;
    IPPolygonStruct *PBase1, *PBase2;
    IPObjectStruct *PCylin;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct *TSrf,
	        *Srf = CagdPrimCylinderSrf(GlblOrigin, R,
					   IRIT_PT_LENGTH(Dir),
					   GlblSurfaceRational,
					   (CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    TSrf = CagdSrfMatTransform(Srf, Mat);
	    CagdSrfFree(Srf);
	    Srf = TSrf;

	    CagdSrfTransform(Srf, Pt, 1.0);

	    return IPGenSRFObject(Srf);
	}
        case GM_GEN_PRIM_MDLS:
	{
	    int OldVal = MdlStitchSelfSrfPrims(FALSE);
	    MdlModelStruct
	        *Mdl = MdlPrimCylinder(GlblOrigin, R,
				       IRIT_PT_LENGTH(Dir),
				       GlblSurfaceRational,
				       (CagdPrimCapsType) Bases);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Dir);

	    MdlModelMatTransform(Mdl, Mat);
	    MdlModelTransform(Mdl, Pt, 1.0);

	    MdlStitchSelfSrfPrims(OldVal);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    break;
    }

    GMGenTransMatrixZ2Dir(Mat, Pt, Dir, R);   /* Transform from unit circle. */

    PCylin = IPGenPolyObject("", NULL, NULL);   /* Gen. CYLIN object itself: */
    /* Also allocate the bases polygon header with first vertex on it: */
    PBase1 = IPAllocPolygon(0, VBase1 = IPAllocVertex2(NULL), NULL);
    PBase2 = IPAllocPolygon(0, VBase2 = IPAllocVertex2(NULL), NULL);

    IRIT_PT_ADD(TPt, Pt, Dir);	       /* Translated circle center (by Dir). */

    /* Prepare the normal directions for the two bases: */
    IRIT_PT_COPY(ForwardDir, Dir);
    IRIT_PT_NORMALIZE(ForwardDir);
    IRIT_PT_COPY(BackwardDir, ForwardDir);
    IRIT_PT_SCALE(BackwardDir, -1.0);

    LastCirclePt[0] = 1.0;		/* First point is allways Angle = 0. */
    LastCirclePt[1] = 0.0;
    LastCirclePt[2] = 0.0;
    MatMultPtby4by4(LastCirclePt, LastCirclePt, Mat);

    UpdateVertexNormal(LastCircleNrml, LastCirclePt, Pt, FALSE, Dummy);

    /* Update 1st pt in base1 polygon. */
    IRIT_PT_COPY(VBase1 -> Coord, LastCirclePt);
    IRIT_PT_COPY(VBase1 -> Normal, ForwardDir);
    IRIT_PT_ADD(TLastCirclePt, LastCirclePt, Dir);/* Trans. circle (by Dir). */

    /* Update 1st pt in base2 polygon. */
    IRIT_PT_COPY(VBase2 -> Coord, TLastCirclePt);
    IRIT_PT_COPY(VBase2 -> Normal, BackwardDir);

    AngleStep = M_PI_MUL_2 / _PrimGlblResolution;

    for (i = 1; i <= _PrimGlblResolution; i++) {  /* Pass whole base circle. */
        int Rvrsd;

	Angle = AngleStep * i;		     /* Prevent from additive error. */

	CirclePt[0] = cos(Angle);
	CirclePt[1] = sin(Angle);
	CirclePt[2] = 0.0;
	MatMultPtby4by4(CirclePt, CirclePt, Mat);

	UpdateVertexNormal(CircleNrml, CirclePt, Pt, FALSE, Dummy);

	IRIT_PT_ADD(TCirclePt, CirclePt, Dir);    /* Trans. circle (by Dir). */

	PCylin -> U.Pl = PrimGenPolygon4Vrtx(TLastCirclePt, TCirclePt,
					     CirclePt, LastCirclePt,
					     Pt, &Rvrsd, PCylin -> U.Pl);
	/* Update the normals for this cylinder side polygon vertices: */
	PVertex = PCylin -> U.Pl -> PVertex;
	IRIT_PT_COPY(PVertex -> Normal, LastCircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, CircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, CircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);
	PVertex = PVertex -> Pnext;
	IRIT_PT_COPY(PVertex -> Normal, LastCircleNrml);
	IP_SET_NORMAL_VRTX(PVertex);

	/* And add this vertices to the two cylinder bases: 		     */
	/* Note Base1 is build forward, while Base2 is build backward so it  */
	/* will be consistent - cross product of 2 consecutive edges will    */
	/* point into the model.					     */
	if (i == _PrimGlblResolution) {/* Its last point - make it circular. */
	    VBase1 -> Pnext = PBase1 -> PVertex;
	    VBase2 -> Pnext = PBase2 -> PVertex;
	}
	else {
	    VBase1 -> Pnext = IPAllocVertex2(NULL);
	    VBase1 = VBase1 -> Pnext;
	    IRIT_PT_COPY(VBase1 -> Coord, CirclePt);
	    IRIT_PT_COPY(VBase1 -> Normal, ForwardDir);
	    PBase2 -> PVertex = IPAllocVertex2(PBase2 -> PVertex);
	    IRIT_PT_COPY(PBase2 -> PVertex -> Coord, TCirclePt);
	    IRIT_PT_COPY(PBase2 -> PVertex -> Normal, BackwardDir);
	}

	/* Save pt in last pt for next time. */
	IRIT_PT_COPY(LastCirclePt, CirclePt);
	IRIT_PT_COPY(TLastCirclePt, TCirclePt);
	IRIT_PT_COPY(LastCircleNrml, CircleNrml);
    }

    if (Bases & 0x01) {
	/* Update base polygon plane equation. */
	IPUpdatePolyPlane2(PBase1, TPt);
	IP_SET_CONVEX_POLY(PBase1);	       /* Mark it as convex polygon. */
	PBase1 -> Pnext = PCylin -> U.Pl;   /* And stick into cone polygons. */
	PCylin -> U.Pl = PBase1;
    }
    else
        IPFreePolygon(PBase1);

    if (Bases & 0x02) {
	/* Update base polygon plane equation. */
	IPUpdatePolyPlane2(PBase2, Pt);
	IP_SET_CONVEX_POLY(PBase2);	       /* Mark it as convex polygon. */
	PBase2 -> Pnext = PCylin -> U.Pl;   /* And stick into cone polygons. */
	PCylin -> U.Pl = PBase2;
    }
    else
        IPFreePolygon(PBase2);

    return PCylin;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a SPHERE geometric object defined by Center, the       M
* center of the sphere and R, its radius.				     M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   Center location of SPHERE.                                     M
*   R:        Radius of sphere.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A SPHERE Primitive.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONEObject, PrimGenCONE2Object, PrimGenCYLINObject,		     M
*   PrimGenTORUSObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenSPHEREObject, sphere, primitives                                  M
*****************************************************************************/
IPObjectStruct *PrimGenSPHEREObject(const IrtVecType Center, IrtRType R)
{
    int i, j, k;
    IrtRType TetaAngle, TetaAngleStep, FeeAngle, FeeAngleStep,
	CosFeeAngle1, SinFeeAngle1, CosFeeAngle2, SinFeeAngle2;
    IrtPtType LastCircleLastPt, LastCirclePt, CirclePt, CircleLastPt, Dummy;
    IPVertexStruct *PVertex;
    IPObjectStruct *PSphere;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct
	        *Srf = CagdPrimSphereSrf(Center, R, GlblSurfaceRational);

	    return IPGenSRFObject(Srf);
	}
        case GM_GEN_PRIM_MDLS:
	{
	    int OldVal = MdlStitchSelfSrfPrims(FALSE);
	    MdlModelStruct
	        *Mdl = MdlPrimSphere(Center, R, GlblSurfaceRational);

	    MdlStitchSelfSrfPrims(OldVal);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    break;
    }

    PSphere = IPGenPolyObject("", NULL, NULL);  /* Gen SPHERE object itself: */

    TetaAngleStep = M_PI_MUL_2 / _PrimGlblResolution;     /* Runs 0 to 2*PI. */
    FeeAngleStep = M_PI_MUL_2 / _PrimGlblResolution; /* Runs -PI/2 to +PI/2. */

    /* Generate the lowest (south pole) triangular polygons: */
    FeeAngle = -M_PI_DIV_2 + FeeAngleStep; /* First circle above south pole. */
    CosFeeAngle1 = cos(FeeAngle) * R;
    SinFeeAngle1 = sin(FeeAngle) * R;
    IRIT_PT_COPY(LastCirclePt, Center);		/* Calculate the south pole. */
    LastCirclePt[2] -= R;
    IRIT_PT_COPY(CircleLastPt, Center);  /* Calc. last point on crnt circle. */
    CircleLastPt[0] += CosFeeAngle1;
    CircleLastPt[2] += SinFeeAngle1;

    for (i = 1; i <= _PrimGlblResolution; i++) {/* Whole (horizontal) circle.*/
        int Rvrsd;

	TetaAngle = TetaAngleStep * i;	     /* Prevent from additive error. */

	IRIT_PT_COPY(CirclePt, Center); /* Calc. crnt point on current circ. */
	CirclePt[0] += cos(TetaAngle) * CosFeeAngle1;
	CirclePt[1] += sin(TetaAngle) * CosFeeAngle1;
	CirclePt[2] += SinFeeAngle1;

	PSphere -> U.Pl = PrimGenPolygon3Vrtx(LastCirclePt, CircleLastPt,
					      CirclePt, Center,
					      &Rvrsd, PSphere -> U.Pl);
	/* Update normals: */
	for (j = 0, PVertex = PSphere -> U.Pl -> PVertex;
	     j < 3;
	     j++, PVertex = PVertex -> Pnext) {
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, Center,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);
	}

	/* Save pt in last pt for next time. */
	IRIT_PT_COPY(CircleLastPt, CirclePt);
    }

    /* Generate the middle rectangular polygons: */
    for (i = 1; i < (_PrimGlblResolution >> 1) - 1; i++) { /* All hor. circs.*/
	FeeAngle = -M_PI_DIV_2 + FeeAngleStep * i;
	CosFeeAngle1 = cos(FeeAngle) * R;
	SinFeeAngle1 = sin(FeeAngle) * R;
	FeeAngle = -M_PI_DIV_2 + FeeAngleStep * (i + 1);
	CosFeeAngle2 = cos(FeeAngle) * R;
	SinFeeAngle2 = sin(FeeAngle) * R;
	IRIT_PT_COPY(CircleLastPt, Center);/* Calc. last point on crnt circ. */
	CircleLastPt[0] += CosFeeAngle2;
	CircleLastPt[2] += SinFeeAngle2;
	IRIT_PT_COPY(LastCircleLastPt, Center);/* Calc. last pt on last circ.*/
	LastCircleLastPt[0] += CosFeeAngle1;
	LastCircleLastPt[2] += SinFeeAngle1;

	for (j = 1; j <= _PrimGlblResolution; j++) { /* Whole (horiz.) circ. */
	    int Rvrsd;

	    TetaAngle = TetaAngleStep * j;   /* Prevent from additive error. */

	    IRIT_PT_COPY(CirclePt, Center);/* Calc. current pt on crnt circ. */
	    CirclePt[0] += cos(TetaAngle) * CosFeeAngle2;
	    CirclePt[1] += sin(TetaAngle) * CosFeeAngle2;
	    CirclePt[2] += SinFeeAngle2;
	    IRIT_PT_COPY(LastCirclePt, Center);/* Calc. crnt pt on last circ.*/
	    LastCirclePt[0] += cos(TetaAngle) * CosFeeAngle1;
	    LastCirclePt[1] += sin(TetaAngle) * CosFeeAngle1;
	    LastCirclePt[2] += SinFeeAngle1;

	    PSphere -> U.Pl =
		PrimGenPolygon4Vrtx(LastCirclePt, LastCircleLastPt,
				    CircleLastPt, CirclePt,
				    Center, &Rvrsd, PSphere -> U.Pl);
	    /* Update normals: */
	    for (k = 0, PVertex = PSphere -> U.Pl -> PVertex;
		 k < 4;
		 k++, PVertex = PVertex -> Pnext) {
		UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, Center,
								FALSE, Dummy);
		IP_SET_NORMAL_VRTX(PVertex);
	    }

	    IRIT_PT_COPY(CircleLastPt, CirclePt);     /* Save pt in last pt. */
	    IRIT_PT_COPY(LastCircleLastPt, LastCirclePt);
	}
    }

    /* Generate the upper most (north pole) triangular polygons: */
    FeeAngle = M_PI_DIV_2 - FeeAngleStep;  /* First circle below north pole. */
    CosFeeAngle1 = cos(FeeAngle) * R;
    SinFeeAngle1 = sin(FeeAngle) * R;
    IRIT_PT_COPY(LastCirclePt, Center);		/* Calculate the north pole. */
    LastCirclePt[2] += R;
    IRIT_PT_COPY(CircleLastPt, Center);  /* Calc. last point on crnt circle. */
    CircleLastPt[0] += CosFeeAngle1;
    CircleLastPt[2] += SinFeeAngle1;

    for (i = 1; i <= _PrimGlblResolution; i++) {/* Pass whole (horiz) circle.*/
        int Rvrsd;

	TetaAngle = TetaAngleStep * i;	     /* Prevent from additive error. */

	IRIT_PT_COPY(CirclePt, Center); /* Calc. current point on crnt circ. */
	CirclePt[0] += cos(TetaAngle) * CosFeeAngle1;
	CirclePt[1] += sin(TetaAngle) * CosFeeAngle1;
	CirclePt[2] += SinFeeAngle1;

	PSphere -> U.Pl =
	    PrimGenPolygon3Vrtx(LastCirclePt, CirclePt, CircleLastPt,
				Center, &Rvrsd, PSphere -> U.Pl);

	/* Update normals: */
	for (j = 0, PVertex = PSphere -> U.Pl -> PVertex;
	     j < 3;
	     j++, PVertex = PVertex -> Pnext) {
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, Center,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);
	}

	/* Save pt in last pt for next time. */
	IRIT_PT_COPY(CircleLastPt, CirclePt);
    }

    return PSphere;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a TORUS geometric object defined by Center - torus 3d  M
* center point, the main torus plane normal Normal, major radius Rmajor and  M
* minor radius Rminor (Tube radius).					     M
*   Teta runs on the major circle, Fee on the minor one. Then                M
* X = (Rmajor + Rminor * cos(Fee)) * cos(Teta)				     V
* Y = (Rmajor + Rminor * cos(Fee)) * sin(Teta)				     V
* Z = Rminor * sin(Fee)							     V
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:  Center location of the TORUS primitive.                         M
*   Normal:  Normal to the major plane of the torus.                         M
*   Rmajor:  Major radius of torus.                                          M
*   Rminor:  Minor radius of torus.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A TOURS Primitive.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetGeneratePrimType, PrimGenBOXObject, PrimGenGBOXObject,	     M
*   PrimGenCONEObject, PrimGenCONE2Object, PrimGenCYLINObject,		     M
*   PrimGenSPHEREObject							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenTORUSObject, torus, primitives                                    M
*****************************************************************************/
IPObjectStruct *PrimGenTORUSObject(const IrtVecType Center,
				   const IrtVecType Normal,
				   IrtRType Rmajor,
				   IrtRType Rminor)
{
    int i, j;
    IrtRType TetaAngle, TetaAngleStep, FeeAngle, FeeAngleStep,
	CosFeeAngle1, SinFeeAngle1, CosFeeAngle2, SinFeeAngle2;
    IrtPtType LastCircleLastPt, LastCirclePt, CirclePt, CircleLastPt,
	LastInPt, InPt, Dummy;
    IrtHmgnMatType Mat;
    IPVertexStruct *PVertex;
    IPObjectStruct *PTorus;

    switch (GlblSetGeneratePrim) {
        case GM_GEN_PRIM_SRFS:
	{
	    CagdSrfStruct *TSrf,
	        *Srf = CagdPrimTorusSrf(GlblOrigin, Rmajor, Rminor,
					GlblSurfaceRational);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Normal);

	    TSrf = CagdSrfMatTransform(Srf, Mat);
	    CagdSrfFree(Srf);
	    Srf = TSrf;

	    CagdSrfTransform(Srf, Center, 1.0);

	    return IPGenSRFObject(Srf);
	}
        case GM_GEN_PRIM_MDLS:
	{
	    int OldVal = MdlStitchSelfSrfPrims(FALSE);
	    MdlModelStruct
	        *Mdl = MdlPrimTorus(GlblOrigin, Rmajor, Rminor,
				    GlblSurfaceRational);
	    IrtHmgnMatType Mat;

	    GMGenMatrixZ2Dir(Mat, Normal);

	    MdlModelMatTransform(Mdl, Mat);
	    MdlModelTransform(Mdl, Center, 1.0);

	    MdlStitchSelfSrfPrims(OldVal);

	    return IPGenMODELObject(Mdl);
	}
        default:
        case GM_GEN_PRIM_POLYS:
	    break;
    }

    GMGenTransMatrixZ2Dir(Mat, Center, Normal, 1.0);/* Trans from unit circ. */

    PTorus = IPGenPolyObject("", NULL, NULL);   /* Gen. Torus object itself: */

    TetaAngleStep = M_PI_MUL_2 / _PrimGlblResolution;/* Runs from 0 to 2*PI. */
    FeeAngleStep = M_PI_MUL_2 / _PrimGlblResolution; /* Runs from 0 to 2*PI. */

    for (i = 1; i <= _PrimGlblResolution; i++) {
	FeeAngle = FeeAngleStep * (i - 1);
	CosFeeAngle1 = cos(FeeAngle) * Rminor;
	SinFeeAngle1 = sin(FeeAngle) * Rminor;
	FeeAngle = FeeAngleStep * i;
	CosFeeAngle2 = cos(FeeAngle) * Rminor;
	SinFeeAngle2 = sin(FeeAngle) * Rminor;
	LastCircleLastPt[0] = Rmajor + CosFeeAngle1;
	LastCircleLastPt[1] = 0.0;
	LastCircleLastPt[2] = SinFeeAngle1;
	MatMultPtby4by4(LastCircleLastPt, LastCircleLastPt, Mat);
	LastCirclePt[0] = Rmajor + CosFeeAngle2;
	LastCirclePt[1] = 0.0;
	LastCirclePt[2] = SinFeeAngle2;
	MatMultPtby4by4(LastCirclePt, LastCirclePt, Mat);
	/* Point inside the object relative to this polygon: */
	LastInPt[0] = Rmajor;
	LastInPt[1] = 0.0;
	LastInPt[2] = 0.0;
	MatMultPtby4by4(LastInPt, LastInPt, Mat);

	for (j = 1; j <= _PrimGlblResolution; j++) {
	    int Rvrsd;

	    TetaAngle = TetaAngleStep * j;   /* Prevent from additive error. */

	    CircleLastPt[0] = (Rmajor + CosFeeAngle1) * cos(TetaAngle);
	    CircleLastPt[1] = (Rmajor + CosFeeAngle1) * sin(TetaAngle);
	    CircleLastPt[2] = SinFeeAngle1;
	    MatMultPtby4by4(CircleLastPt, CircleLastPt, Mat);
	    CirclePt[0] = (Rmajor + CosFeeAngle2) * cos(TetaAngle);
	    CirclePt[1] = (Rmajor + CosFeeAngle2) * sin(TetaAngle);
	    CirclePt[2] = SinFeeAngle2;
	    MatMultPtby4by4(CirclePt, CirclePt, Mat);
	    /* Point inside the object relative to this polygon: */
	    InPt[0] = Rmajor * cos(TetaAngle);
	    InPt[1] = Rmajor * sin(TetaAngle);
	    InPt[2] = 0.0;
	    MatMultPtby4by4(InPt, InPt, Mat);

	    PTorus -> U.Pl =
		PrimGenPolygon4Vrtx(CirclePt, CircleLastPt, LastCircleLastPt,
				   LastCirclePt, InPt, &Rvrsd, PTorus -> U.Pl);

	    /* Update normals: */
	    PVertex = PTorus -> U.Pl -> PVertex;
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, InPt,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);
	    PVertex = PVertex -> Pnext;
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, InPt,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);
	    PVertex = PVertex -> Pnext;
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, LastInPt,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);
	    PVertex = PVertex -> Pnext;
	    UpdateVertexNormal(PVertex -> Normal, PVertex -> Coord, LastInPt,
								FALSE, Dummy);
	    IP_SET_NORMAL_VRTX(PVertex);

	    IRIT_PT_COPY(LastCirclePt, CirclePt);    /* Save pt in last pt. */
	    IRIT_PT_COPY(LastCircleLastPt, CircleLastPt);
	    IRIT_PT_COPY(LastInPt, InPt);
	}
    }

    return PTorus;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a POLYDISK geometric object defined by the normal N    M
* and the translation vector T. The object is a planar disk (a circle of     M
* _PrimGlblResolution points in it...) and its radius is equal to R.	     M
*   The normal direction is assumed to point to the inside of the object.    M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Nrml:      Normal to the plane this disk included in.                    M
*   Trns:      A translation factor of the center of the disk.               M
*   R:         Radius of the disk.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A single polygon object - a disk.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenPOLYDISKObject, disk, primitives                                  M
*****************************************************************************/
IPObjectStruct *PrimGenPOLYDISKObject(const IrtVecType Nrml,
				      const IrtVecType Trns,
				      IrtRType R)
{
    int i;
    IrtRType Angle, AngleStep;
    IrtPtType CirclePt;
    IrtVecType N;
    IrtHmgnMatType Mat;
    IPVertexStruct *V;
    IPPolygonStruct *PCirc;
    IPObjectStruct *PPDisk;

    IRIT_VEC_COPY(N, Nrml);
    IRIT_VEC_NORMALIZE(N);
    GMGenTransMatrixZ2Dir(Mat, Trns, N, R);   /* Transform from unit circle. */

    PPDisk = IPGenPolyObject("", NULL, NULL);   /* Gen. PLANE object itself: */
    PCirc = IPAllocPolygon(0, V = IPAllocVertex2(NULL), NULL);
    PPDisk -> U.Pl = PCirc;

    CirclePt[0] = 1.0;			/* First point is allways Angle = 0. */
    CirclePt[1] = 0.0;
    CirclePt[2] = 0.0;
    MatMultPtby4by4(CirclePt, CirclePt, Mat);
    IRIT_PT_COPY(V -> Coord, CirclePt);  /* Update first pt in circ polygon. */
    IRIT_PT_COPY(V -> Normal, N);

    AngleStep = M_PI_MUL_2 / _PrimGlblResolution;

    for (i = 1; i <= _PrimGlblResolution; i++) {  /* Pass whole base circle. */
	Angle = AngleStep * i;		     /* Prevent from additive error. */

	CirclePt[0] = cos(Angle);
	CirclePt[1] = sin(Angle);
	CirclePt[2] = 0.0;

	MatMultPtby4by4(CirclePt, CirclePt, Mat);

	/* And add this vertices to the two cylinder bases: */
	if (i == _PrimGlblResolution) {/* Its last point - make it circular. */
	    V -> Pnext = PCirc -> PVertex;
	}
	else {
	    V -> Pnext = IPAllocVertex2(NULL);
	    V = V -> Pnext;
	    IRIT_PT_COPY(V -> Coord, CirclePt);
	    IRIT_PT_COPY(V -> Normal, N);
	}
    }

    IRIT_PT_ADD(CirclePt, CirclePt, N); /* Make a point "IN" the circle obj. */
    /* Update base polygon plane equation. */
    IPUpdatePolyPlane2(PCirc, CirclePt);
    IP_SET_CONVEX_POLY(PCirc);		       /* Mark it as convex polygon. */

    return PPDisk;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a POLYGON/LINE directly from its specified vertices.   M
*   The validity of the elements in the provided list is tested to make sure M
* they are vectors or points.                                                M
*   No test is made to make sure all vertices are on one plane, and that no  M
* two vertices are similar.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:     List of vertices/points to construct as a polygon/line.    M
*   IsPolyline:   If TRUE, make a polyline, otherwise a polygon.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A polygon/line constructed from PObjList.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenPOLYGONObject, polygon, polyline, primitives                      M
*****************************************************************************/
IPObjectStruct *PrimGenPOLYGONObject(IPObjectStruct *PObjList, int IsPolyline)
{
    int i,
	NumVertices = 0;
    IPVertexStruct *V, *VHead,
	*VTail = NULL;
    IPPolygonStruct *PPoly;
    IPObjectStruct *PObj, *PObjPoly;

    if (!IP_IS_OLST_OBJ(PObjList))
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_LIST_OBJ);

    i = 0;
    while ((PObj = IPListObjectGet(PObjList, i++)) != NULL) {
	if (!IP_IS_VEC_OBJ(PObj) &&
	    !IP_IS_POINT_OBJ(PObj) &&
	    !IP_IS_CTLPT_OBJ(PObj) &&
	    !IP_IS_POLY_OBJ(PObj)) {
	    IRIT_WARNING_MSG("None vertex object found in list, empty object result.");
	    return NULL;
	}
	NumVertices += IP_IS_POLY_OBJ(PObj) ?
				    IPVrtxListLen(PObj -> U.Pl -> PVertex) : 1;
    }

    if (NumVertices < 2 + !IsPolyline) {
	IRIT_WARNING_MSG("Too few vertices, empty object result.");
	return NULL;
    }

    PPoly = IPAllocPolygon(0, VHead = NULL, NULL);
    i = 0;
    while ((PObj = IPListObjectGet(PObjList, i++)) != NULL) {
	if (IP_IS_POLY_OBJ(PObj)) {
	    V = IPCopyVertexList(PObj -> U.Pl -> PVertex);
	}
	else {
	    V = IPAllocVertex2(NULL);
	    if (IP_IS_VEC_OBJ(PObj))
		IRIT_PT_COPY(V -> Coord, PObj -> U.Vec);
	    else if (IP_IS_POINT_OBJ(PObj))
		IRIT_PT_COPY(V -> Coord, PObj -> U.Pt);
	    else if (IP_IS_CTLPT_OBJ(PObj)) {
		IPObjectStruct
		    *PVec = IPCoerceObjectTo(PObj, IP_OBJ_VECTOR);

		IRIT_PT_COPY(V -> Coord, PVec -> U.Vec);
		IPFreeObject(PVec);
	    }

	    if (PObj -> Attr != NULL) {
		int WasNormal = FALSE;
		const char *PNormalStr;

		V -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

		if ((PNormalStr = AttrGetStrAttrib(V -> Attr,
						   "Normal")) != NULL) {
		    IrtVecType N;

		    if (
#ifdef IRIT_DOUBLE
			(sscanf(PNormalStr, "%lf %lf %lf",
				&N[0], &N[1], &N[2]) == 3 ||
			 sscanf(PNormalStr, "%lf,%lf,%lf",
				&N[0], &N[1], &N[2]) == 3)) {
#else
			(sscanf(PNormalStr, "%f %f %f",
				&N[0], &N[1], &N[2]) == 3 ||
			 sscanf(PNormalStr, "%f,%f,%f",
				&N[0], &N[1], &N[2]) == 3)) {
#endif /* IRIT_DOUBLE */
			    IRIT_PT_COPY(V -> Normal, N);
			    IP_SET_NORMAL_VRTX(V);
			}
			WasNormal = TRUE;
		    }

		if (WasNormal)
		    AttrFreeOneAttribute(&V -> Attr, "Normal");
	    }
	}

	if (VHead == NULL) {
	    PPoly -> PVertex = VHead = V;
	}
	else {
	    VTail -> Pnext = V;
	}
	VTail = IPGetLastVrtx(V);
    }

    PObjPoly = IPGenPolyObject("", PPoly, NULL); /* Gen. POLY object itself: */
    if (IsPolyline) {
	IP_SET_POLYLINE_OBJ(PObjPoly);
    }
    else {
	IP_SET_POLYGON_OBJ(PObjPoly);

	VTail -> Pnext = VHead;		      /* Close the vertex list loop. */

	/* Update polygon plane equation and vertices normals. */
	IPUpdatePolyPlane(PPoly);

	V = VHead;
	do {
	    if (!IP_HAS_NORMAL_VRTX(V))
		IRIT_PT_COPY(V -> Normal, PPoly -> Plane);
	    V = V -> Pnext;
	}
	while (V != VHead);
    }

    return PObjPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create an OBJECT directly from set of specified polys.          M
*   No test is made for the validity of the model in any sense.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:      List of polygonal objects.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A single object containing all polygons in all       M
*                       provided objects, by a simple union.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenObjectFromPolyList, primitives                                    M
*****************************************************************************/
IPObjectStruct *PrimGenObjectFromPolyList(IPObjectStruct *PObjList)
{
    int i;
    IPPolygonStruct *PPoly,
	*PTail = NULL;
    IPObjectStruct *PObj, *PObjPoly;

    if (!IP_IS_OLST_OBJ(PObjList))
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_LIST_OBJ);

    i = 0;
    while ((PObj = IPListObjectGet(PObjList, i++)) != NULL) {
	if (!IP_IS_POLY_OBJ(PObj)) {
	    IRIT_WARNING_MSG("None polygon object found in list, empty object result.");
	    return NULL;
	}
    }

    PObjPoly = IPGenPolyObject("", NULL, NULL);  /* Gen. POLY object itself: */
    i = 0;
    while ((PObj = IPListObjectGet(PObjList, i)) != NULL) {
	if (i++ == 0) {
	    if (IP_IS_POLYLINE_OBJ(PObj))
	        IP_SET_POLYLINE_OBJ(PObjPoly);
	    else
		IP_SET_POLYGON_OBJ(PObjPoly);
	}
	else {
	    if ((IP_IS_POLYLINE_OBJ(PObj) && !IP_IS_POLYLINE_OBJ(PObjPoly)) ||
		(IP_IS_POLYGON_OBJ(PObj) && !IP_IS_POLYGON_OBJ(PObjPoly))) {
		IRIT_WARNING_MSG("Polygons mixed with polylines.");
		return NULL;
	    }	      
	}

	PPoly = IPCopyPolygonList(PObj -> U.Pl);

	if (PTail == NULL) {
	    PObjPoly -> U.Pl = PPoly;
	}
	else {
	    PTail -> Pnext = PPoly;
	}
	for (PTail = PPoly; PTail -> Pnext != NULL; PTail = PTail -> Pnext);
    }

    return PObjPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Not supported.                                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:                                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:                                                        *
*****************************************************************************/
IPObjectStruct *PrimGenCROSSECObject(const IPObjectStruct *PObj)
{
    if (PObj && !IP_IS_POLY_OBJ(PObj))
	GEOM_FATAL_ERROR(GEOM_ERR_EXPCT_POLYHEDRA);

    IRIT_WARNING_MSG("GenCrossSecObject not implemented");

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a polygon out of a list of 4 vertices V1/2/3/4.	     M
*   The fifth vertex is inside (actually, this is not true, as this point    M
* will be in the positive part of the plane, which only locally in the       M
* object...) the object, so the polygon's normal direction can be evaluated  M
* uniquely.								     M
*   No test is made to make sure the 4 points are co-planar...		     M
*   The points are placed in order.                      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2, V3, V4:    Four vertices of the constructed polygon.             M
*   Vin:               A vertex that can be assumed to be inside the         M
*                      object for normal evaluation of the plane of polygon. M
*		          Can be NULL in which case vrtcs order is kept.     M
*   VrtcsRvrsd:        Set to TRUE if has Vin and order of vertices is       M
*		       oriented in reverse.				     M
*   Pnext:             Next is chain of polygons, in linked list.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The constructed polygon.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenPolygon3Vrtx, GMGenPolyline2Vrtx                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenPolygon4Vrtx                                                      M
*****************************************************************************/
IPPolygonStruct *PrimGenPolygon4Vrtx(const IrtVecType V1,
				     const IrtVecType V2,
				     const IrtVecType V3,
				     const IrtVecType V4,
				     const IrtVecType Vin,
				     int *VrtcsRvrsd,
				     IPPolygonStruct *Pnext)
{
    IPPolygonStruct *PPoly;
    IPVertexStruct *V;

    /* In singular cases (like the north pole of a surface of revolution)  */
    /* the rectangle is degenerate - create a triangle then.               */
    if (IRIT_PT_APX_EQ(V1, V2))
        return PrimGenPolygon3Vrtx(V1, V3, V4, Vin, VrtcsRvrsd, Pnext);
    else if (IRIT_PT_APX_EQ(V2, V3))
        return PrimGenPolygon3Vrtx(V1, V2, V4, Vin, VrtcsRvrsd, Pnext);
    else if (IRIT_PT_APX_EQ(V3, V4) || IRIT_PT_APX_EQ(V4, V1))
        return PrimGenPolygon3Vrtx(V1, V2, V3, Vin, VrtcsRvrsd, Pnext);

    *VrtcsRvrsd = FALSE;

    PPoly = IPAllocPolygon(0, V = IPAllocVertex2(NULL), Pnext);
    IRIT_PT_COPY(V -> Coord, V1);

    V -> Pnext = IPAllocVertex2(NULL);
    V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V2);

    V -> Pnext = IPAllocVertex2(NULL);
    V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V3);

    V -> Pnext = IPAllocVertex2(NULL);
    V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V4);

    V -> Pnext = PPoly -> PVertex;	   /* Make the Vertex list circular. */

    /* Update plane equation. */
    if (Vin == NULL)
        IPUpdatePolyPlane(PPoly);
    else
	*VrtcsRvrsd = IPUpdatePolyPlane2(PPoly, Vin) == -1;

    IP_SET_CONVEX_POLY(PPoly);		       /* Mark it as convex polygon. */

    return PPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a polygon out of a list of 3 vertices V1/2/3.	     M
*   The fourth vertex is inside (actually, this is not true, as this point   M
* will be in the positive part of the plane, which only locally in the       M
* object...) the object, so the polygon's normal direction can be evaluated  M
* uniquely.								     M
*   No test is made to make sure the 3 points are not co-linear...	     M
*   The points are placed in order.                      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2, V3:    Three vertices of the constructed polygon.                M
*   Vin:           A vertex that can be assumed to be inside the             M
*                  object for normal evaluation of the plane of polygon.     M
*		     Can be NULL in which case vrtcs order is kept.	     M
*   VrtcsRvrsd:    set to TRUE if has Vin and order of vertices is oriented  M
*		   in reverse.						     M
*   Pnext:         Next is chain of polygons, in linked list.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The constructed polygon.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenPolygon4Vrtx, GMGenPolyline2Vrtx                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenPolygon3Vrtx                                                      M
*****************************************************************************/
IPPolygonStruct *PrimGenPolygon3Vrtx(const IrtVecType V1,
				     const IrtVecType V2,
				     const IrtVecType V3,
				     const IrtVecType Vin,
				     int *VrtcsRvrsd,
				     IPPolygonStruct *Pnext)
{
    IPPolygonStruct *PPoly;
    IPVertexStruct *V;

#ifdef DEBUG
    /* In singular cases (like the north pole of a surface of revolution)  */
    /* the triangle can be degenerate - issue a warning.                   */
    if (IRIT_PT_APX_EQ(V1, V2) ||
	IRIT_PT_APX_EQ(V2, V3) ||
        IRIT_PT_APX_EQ(V3, V1))
        IRIT_WARNING_MSG("Constructed a degenerate triangle polygon.");
#endif /* DEBUG */

    *VrtcsRvrsd = FALSE;

    PPoly = IPAllocPolygon(0, V = IPAllocVertex2(NULL), Pnext);
    IRIT_PT_COPY(V -> Coord, V1);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V2);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V3);

    V -> Pnext = PPoly -> PVertex;	   /* Make the Vertex list circular. */

    /* Update plane equation. */
    if (Vin == NULL)
        IPUpdatePolyPlane(PPoly);
    else
	*VrtcsRvrsd = IPUpdatePolyPlane2(PPoly, Vin) == -1;

    IP_SET_CONVEX_POLY(PPoly);		       /* Mark it as convex polygon. */

    return PPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to update the Vertex Pt normal equation. The normal should point *
* InPt but should be perpendicular to the line Pt-PerpPt if Perpendicular is *
* TRUE. THe normal is normalized to a unit length.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Normal:        To update.                                                *
*   Pt:            The normal belongs to this location.                      *
*   InPt:          To define the normal direction as InPt - Pt.              *
*   Perpendicular: If TRUE, Normal also perpedicular to PerpPt - Pt.         *
*   PerpPt:        To define perpendicular relation as PerpPt - Pt.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateVertexNormal(IrtNrmlType Normal,
			       const IrtPtType Pt,
			       const IrtPtType InPt,
			       int Perpendicular,
			       const IrtPtType PerpPt)
{
    IrtVecType V1, V2;

    IRIT_PT_SUB(Normal, InPt, Pt);

    if (Perpendicular) {
	IRIT_PT_SUB(V1, PerpPt, Pt);
	GMVecCrossProd(V2, V1, Normal);
	GMVecCrossProd(Normal, V2, V1);
    }

    IRIT_PT_NORMALIZE(Normal);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to set the polygonal resolution (fineness).                        M
*   Resolution sroutghly the number of edges a circular primitive will have  M
* along the entire circle.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Resolution:    To set as new resolution for all primitve constructors.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old resolution value.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimSetResolution, primitives, resolution                                M
*****************************************************************************/
int PrimSetResolution(int Resolution)
{
    int OldVal = _PrimGlblResolution;

    _PrimGlblResolution = IRIT_MAX(Resolution, MIN_RESOLUTION);

    return OldVal;
}
