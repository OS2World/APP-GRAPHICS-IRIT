/******************************************************************************
* CagdPrim.c - Primitive surfaces' constructors.                 	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 96.					      *
******************************************************************************/

#include <string.h>
#include "cagd_loc.h"

/* Ctlpts of one patch (out of six) of a cube-topology sphere.              */
/* See also: "Tiling the Sphere with Rational Bezier Patches" by Jim Cobb.  */
IRIT_STATIC_DATA CagdRType GlblCagdCubeSpherePatchCtlpts[25][4] = {
    { 5.0717967697244912, -2.9282032302755088, 2.9282032302755088, -2.9282032302755088 },
    { 4.5200421036033447, -1.4142135623730951, 3.2073645067092023, -3.2073645067092023 },
    { 4.357265589908164, 0., 3.2854688201836724, -3.2854688201836724 },
    { 4.5200421036033447, 1.4142135623730951, 3.2073645067092023, -3.2073645067092023 },
    { 5.0717967697244912, 2.9282032302755088, 2.9282032302755088, -2.9282032302755088 },

    { 4.5200421036033447, -3.2073645067092023, 1.4142135623730951, -3.2073645067092023 },
    { 3.8660254037844384, -1.598076211353316, 1.598076211353316, -3.8660254037844384 },
    { 3.6449237056739161, 0., 1.6668384836817698, -4.0824829046386304 },
    { 3.8660254037844384, 1.598076211353316, 1.598076211353316, -3.8660254037844384 },
    { 4.5200421036033447, 3.2073645067092023, 1.4142135623730951, -3.2073645067092023 },

    { 4.357265589908164, -3.2854688201836724, 0., -3.2854688201836724 },
    { 3.6449237056739161, -1.6668384836817698, 0., -4.0824829046386304 },
    { 3.4045573501530604, 0., 0., -4.357265589908164 },
    { 3.6449237056739161, 1.6668384836817698, 0., -4.0824829046386304 },
    { 4.357265589908164, 3.2854688201836724, 0., -3.2854688201836724 },

    { 4.5200421036033447, -3.2073645067092023, -1.4142135623730951, -3.2073645067092023 },
    { 3.8660254037844384, -1.598076211353316, -1.598076211353316, -3.8660254037844384 },
    { 3.6449237056739161, 0., -1.6668384836817698, -4.0824829046386304 },
    { 3.8660254037844384, 1.598076211353316, -1.598076211353316, -3.8660254037844384 },
    { 4.5200421036033447, 3.2073645067092023, -1.4142135623730951, -3.2073645067092023 },

    { 5.0717967697244912, -2.9282032302755088, -2.9282032302755088, -2.9282032302755088 },
    { 4.5200421036033447, -1.4142135623730951, -3.2073645067092023, -3.2073645067092023 },
    { 4.357265589908164, 0., -3.2854688201836724, -3.2854688201836724 },
    { 4.5200421036033447, 1.4142135623730951, -3.2073645067092023, -3.2073645067092023 },
    { 5.0717967697244912, 2.9282032302755088, -2.9282032302755088, -2.9282032302755088 }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A curve constructor of a rectangle, parallel to XY plane at level Zlevel.M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY:  Minimum XY coordinates of rectangle.                        M
*   MaxX, MaxY:  Maximum XY coordinates of rectangle.                        M
*   ZLevel:      Z level of rectangle, parallel to the XY plane.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Constructed curve.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimRectangleCrv                                                     M
*****************************************************************************/
CagdCrvStruct *CagdPrimRectangleCrv(CagdRType MinX,
				    CagdRType MinY,
				    CagdRType MaxX,
				    CagdRType MaxY,
				    CagdRType ZLevel)
{
    CagdPtStruct Pt1, Pt2;
    CagdCrvStruct *Crv1, *Crv2, *Crv3;

    Pt1.Pt[0] = MinX;
    Pt1.Pt[1] = MinY;
    Pt1.Pt[2] = ZLevel;
    Pt2.Pt[0] = MaxX;
    Pt2.Pt[1] = MinY;
    Pt2.Pt[2] = ZLevel;
    Crv1 = CagdMergePtPt(&Pt1, &Pt2);
    
    Pt1.Pt[1] = MaxY;
    Pt2.Pt[1] = MaxY;
    Crv2 = CagdMergePtPt(&Pt2, &Pt1);

    Crv3 = CagdMergeCrvCrv(Crv1, Crv2, TRUE);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    Pt1.Pt[1] = MinY;
    Crv1 = CagdMergeCrvPt(Crv3, &Pt1);
    CagdCrvFree(Crv3);

    return Crv1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a plane, parallel to XY plane at level Zlevel.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY:  Minimum X coordinates of plane.                             M
*   MaxX, MaxY:  Maximum Y coordinates of plane.                             M
*   ZLevel:      Z level of plane, parallel to the XY plane.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed plane, as a bilinear surface.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf, CagdPrimRectangleCrv                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimPlaneSrf                                                         M
*****************************************************************************/
CagdSrfStruct *CagdPrimPlaneSrf(CagdRType MinX,
				CagdRType MinY,
				CagdRType MaxX,
				CagdRType MaxY,
				CagdRType ZLevel)
{
    CagdPtStruct P11, P12, P21, P22;
    CagdSrfStruct *Srf;

    P11.Pt[2] = P12.Pt[2] = P21.Pt[2] = P22.Pt[2] = ZLevel;

    P11.Pt[0] = MinX;
    P11.Pt[1] = MinY;

    P12.Pt[0] = MinX;
    P12.Pt[1] = MaxY;

    P21.Pt[0] = MaxX;
    P21.Pt[1] = MinY;

    P22.Pt[0] = MaxX;
    P22.Pt[1] = MaxY;

    Srf = CagdBilinearSrf(&P11, &P21, &P12, &P22);
    if (CAGD_NUM_OF_PT_COORD(Srf -> PType) < 3) {
        CagdSrfStruct
	    *TSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);

	CagdSrfFree(Srf);
	Srf = TSrf;
    }

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_PLANAR);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a plane, parallel to XY plane at level Zlevel.  M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY:  Minimum XY coordinates of plane.                            M
*   MaxX, MaxY:  Maximum XY coordinates of plane.                            M
*   ZLevel:      Z level of plane, parallel to the XY plane.                 M
*   Order:	 Order of plane surface that is requested.		     M
*   Len:         Number of control points (via refinement).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed surface.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf, CagdPrimPlaneSrf                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimPlaneSrfOrderLen                                                 M
*****************************************************************************/
CagdSrfStruct *CagdPrimPlaneSrfOrderLen(CagdRType MinX,
					CagdRType MinY,
					CagdRType MaxX,
					CagdRType MaxY,
					CagdRType ZLevel,
					int Order,
					int Len)
{
    CagdSrfStruct
	*TSrf = CagdPrimPlaneSrf(MinX, MinY, MaxX, MaxY, ZLevel),
	*Srf = CagdSrfDegreeRaiseN(TSrf, Order, Order);

    CagdSrfFree(TSrf);

    if (Len > Order) {
        CagdRType
	    *KV = BspKnotUniformOpen(Len, Order, NULL);
	CagdSrfStruct
	    *RSrf1 = CagdSrfRefineAtParams(Srf, CAGD_CONST_U_DIR, FALSE,
					   &KV[Order], Len - Order),
	    *RSrf2 = CagdSrfRefineAtParams(RSrf1, CAGD_CONST_V_DIR, FALSE,
					   &KV[Order], Len - Order);

	IritFree(KV);
	CagdSrfFree(Srf);
	CagdSrfFree(RSrf1);
	Srf = RSrf2;
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of an XZ plane, parallel to XZ plane at level      M
* Ylevel.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinZ:  Minimum X coordinates of plane.                             M
*   MaxX, MaxZ:  Maximum Z coordinates of plane.                             M
*   YLevel:      Y level of plane, parallel to the XZ plane.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed plane, as a bilinear surface.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf, CagdPrimRectangleCrv, CagdPrimPlaneSrf              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimPlaneXZSrf                                                       M
*****************************************************************************/
CagdSrfStruct *CagdPrimPlaneXZSrf(CagdRType MinX,
				  CagdRType MinZ,
				  CagdRType MaxX,
				  CagdRType MaxZ,
				  CagdRType YLevel)
{
    CagdPtStruct P11, P12, P21, P22;
    CagdSrfStruct *Srf;

    P11.Pt[1] = P12.Pt[1] = P21.Pt[1] = P22.Pt[1] = YLevel;

    P11.Pt[0] = MinX;
    P11.Pt[2] = MinZ;

    P12.Pt[0] = MinX;
    P12.Pt[2] = MaxZ;

    P21.Pt[0] = MaxX;
    P21.Pt[2] = MinZ;

    P22.Pt[0] = MaxX;
    P22.Pt[2] = MaxZ;

    Srf = CagdBilinearSrf(&P11, &P21, &P12, &P22);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_PLANAR);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of an YZ plane, parallel to YZ plane at level      M
* Ylevel.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MinY, MinZ:  Minimum Y coordinates of plane.                             M
*   MaxY, MaxZ:  Maximum Z coordinates of plane.                             M
*   XLevel:      X level of plane, parallel to the YZ plane.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed plane, as a bilinear surface.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf, CagdPrimRectangleCrv, CagdPrimPlaneSrf              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimPlaneYZSrf                                                       M
*****************************************************************************/
CagdSrfStruct *CagdPrimPlaneYZSrf(CagdRType MinY,
				  CagdRType MinZ,
				  CagdRType MaxY,
				  CagdRType MaxZ,
				  CagdRType XLevel)
{
    CagdPtStruct P11, P12, P21, P22;
    CagdSrfStruct *Srf;

    P11.Pt[0] = P12.Pt[0] = P21.Pt[0] = P22.Pt[0] = XLevel;

    P11.Pt[1] = MinY;
    P11.Pt[2] = MinZ;

    P12.Pt[1] = MinY;
    P12.Pt[2] = MaxZ;

    P21.Pt[1] = MaxY;
    P21.Pt[2] = MinZ;

    P22.Pt[1] = MaxY;
    P22.Pt[2] = MaxZ;

    Srf = CagdBilinearSrf(&P11, &P21, &P12, &P22);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_PLANAR);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a box, parallel to main axes.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MinX, MinY, MinZ:   Minimum range of box model.                          M
*   MaxX, MaxY, MaxZ:   Maximum range of box model.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Constructed box model, as a set of six bilinear srfs.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimBoxSrf                                                           M
*****************************************************************************/
CagdSrfStruct *CagdPrimBoxSrf(CagdRType MinX,
			      CagdRType MinY,
			      CagdRType MinZ,
			      CagdRType MaxX,
			      CagdRType MaxY,
			      CagdRType MaxZ)
{
    CagdSrfStruct
        *SrfXYMin = CagdPrimPlaneSrf(MaxX, MinY, MinX, MaxY, MinZ),
        *SrfXYMax = CagdPrimPlaneSrf(MinX, MinY, MaxX, MaxY, MaxZ),
        *SrfXZMin = CagdPrimPlaneXZSrf(MinX, MinZ, MaxX, MaxZ, MinY),
        *SrfXZMax = CagdPrimPlaneXZSrf(MaxX, MinZ, MinX, MaxZ, MaxY),
        *SrfYZMin = CagdPrimPlaneYZSrf(MaxY, MinZ, MinY, MaxZ, MinX),
        *SrfYZMax = CagdPrimPlaneYZSrf(MinY, MinZ, MaxY, MaxZ, MaxX);

    SrfXYMin -> Pnext = SrfXYMax;
    SrfXYMax -> Pnext = SrfXZMin;
    SrfXZMin -> Pnext = SrfXZMax;
    SrfXZMax -> Pnext = SrfYZMin;
    SrfYZMin -> Pnext = SrfYZMax;

    return SrfXYMin;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a sphere, centered at Center and radius Radius. M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   of constructed sphere.                                         M
*   Radius:   of constructed sphere.                                         M
*   Rational: If TRUE exact ration sphere is created.  If FALSE an	     M
*		 approximated integral surface is created.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a sphere.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,   M
*   CagdPrimCylinderSrf, CagdPrimCubeSphereSrf				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimSphereSrf                                                        M
*****************************************************************************/
CagdSrfStruct *CagdPrimSphereSrf(const CagdVType Center,
				 CagdRType Radius,
				 CagdBType Rational)
{
    CagdMType Mat;
    CagdRType TMin, TMax;
    CagdCrvStruct *Arc180, *TCrv,
	*Circle = Rational ? BspCrvCreateUnitCircle() :
			     BspCrvCreateUnitPCircle();
    CagdSrfStruct *Spr;

    CagdCrvDomain(Circle, &TMin, &TMax);
    Arc180 = CagdCrvRegionFromCrv(Circle, TMin, TMin + (TMax - TMin) * 0.5);

    CagdCrvFree(Circle);

    MatGenMatRotY1(M_PI * 0.5, Mat);		         /* Map to YZ plane. */
    TCrv = CagdCrvMatTransform(Arc180, Mat);
    CagdCrvFree(Arc180);
    Arc180 = TCrv;

    if (Rational)
	Spr = CagdSurfaceRev(Arc180);
    else
        Spr = CagdSurfaceRevPolynomialApprox(Arc180);

    CagdCrvFree(Arc180);

    CagdSrfTransform(Spr, NULL, Radius);
    CagdSrfTransform(Spr, Center, 1.0);

    CAGD_SET_GEOM_TYPE(Spr, CAGD_GEOM_SPHERICAL);

    return Spr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a sphere, centered at Center and radius Radius. M
* Constructs the sphere out of six patches in a cube topology.		     M
*   See also: "Tiling the Sphere with Rational Bezier Patches" by Jim Cobb.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   of constructed sphere.                                         M
*   Radius:   of constructed sphere.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a sphere.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, CagdPrimConeSrf,   M
*   CagdPrimCylinderSrf, CagdPrimSphereSrf				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimCubeSphereSrf                                                    M
*****************************************************************************/
CagdSrfStruct *CagdPrimCubeSphereSrf(const CagdVType Center,
				     CagdRType Radius)
{
    int i, n;
    CagdSrfStruct *Srf2, *Srf3, *Srf4, *Srf5, *Srf6, 
        *Srf1 = BzrSrfNew(5, 5, CAGD_PT_P3_TYPE);
    CagdRType
        **Pts = Srf1 -> Points;
    IrtHmgnMatType RotMat;

    /* Copy the hardcoded mesh into the sphere. */
    for (n = 0; n < 25; n++)
        for (i = 0; i <= 3; i++)
	    Pts[i][n] = GlblCagdCubeSpherePatchCtlpts[n][i];

    MatGenMatRotX1(IRIT_DEG2RAD(90), RotMat);
    Srf2 = CagdSrfMatTransform(Srf1, RotMat);
    Srf3 = CagdSrfMatTransform(Srf2, RotMat);
    Srf4 = CagdSrfMatTransform(Srf3, RotMat);

    MatGenMatRotY1(IRIT_DEG2RAD(90), RotMat);
    Srf5 = CagdSrfMatTransform(Srf1, RotMat);

    MatGenMatRotY1(IRIT_DEG2RAD(-90), RotMat);
    Srf6 = CagdSrfMatTransform(Srf1, RotMat);

    /* Map the surfaces to the destination location and chain into a list. */
    CagdSrfTransform(Srf1, NULL, Radius);
    CagdSrfTransform(Srf1, Center, 1.0);
    CagdSrfTransform(Srf2, NULL, Radius);
    CagdSrfTransform(Srf2, Center, 1.0);
    CagdSrfTransform(Srf3, NULL, Radius);
    CagdSrfTransform(Srf3, Center, 1.0);
    CagdSrfTransform(Srf4, NULL, Radius);
    CagdSrfTransform(Srf4, Center, 1.0);
    CagdSrfTransform(Srf5, NULL, Radius);
    CagdSrfTransform(Srf5, Center, 1.0);
    CagdSrfTransform(Srf6, NULL, Radius);
    CagdSrfTransform(Srf6, Center, 1.0);

    Srf1 -> Pnext = Srf2;
    Srf2 -> Pnext = Srf3;
    Srf3 -> Pnext = Srf4;
    Srf4 -> Pnext = Srf5;
    Srf5 -> Pnext = Srf6;

    return Srf1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a torus, centered at Center and radii of        M
* MajorRadius and MinorRadius.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed torus.                                     M
*   MajorRadius:   of constructed torus.                                     M
*   MinorRadius:   of constructed torus.                                     M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a torus.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimSphereSrf, CagdPrimCone2Srf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimTorusSrf                                                         M
*****************************************************************************/
CagdSrfStruct *CagdPrimTorusSrf(const CagdVType Center,
				CagdRType MajorRadius,
				CagdRType MinorRadius,
				CagdBType Rational)
{
    CagdVType Trans;
    CagdMType Mat;
    CagdCrvStruct *TCrv,
	*Circle = Rational ? BspCrvCreateUnitCircle() :
			     BspCrvCreateUnitPCircle();
    CagdSrfStruct *Trs;

    CagdCrvTransform(Circle, NULL, MinorRadius);
    IRIT_PT_RESET(Trans);
    Trans[1] = MajorRadius;
    CagdCrvTransform(Circle, Trans, 1.0);

    MatGenMatRotY1(M_PI * 0.5, Mat);		         /* Map to YZ plane. */
    TCrv = CagdCrvMatTransform(Circle, Mat);
    CagdCrvFree(Circle);
    Circle = TCrv;

    if (Rational)
	Trs = CagdSurfaceRev(Circle);
    else
	Trs = CagdSurfaceRevPolynomialApprox(Circle);

    CagdCrvFree(Circle);

    CagdSrfTransform(Trs, Center, 1.0);

    CAGD_SET_GEOM_TYPE(Trs, CAGD_GEOM_TORUS);

    return Trs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a truncated cone, centered at Center and radii  M
* of MajorRadius and MinorRadius. A MinorRadius of zero would construct a    M
* regular cone.  Otherwise, a truncated cone.  Axis of cone is Z axis.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed cone (center of its base).                 M
*   MajorRadius:   of constructed cone.                                      M
*   MinorRadius:   of constructed cone.                                      M
*   Height:	   of constructed cone.                                      M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a cone.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimConeSrf,  M
*   CagdPrimCylinderSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimCone2Srf                                                         M
*****************************************************************************/
CagdSrfStruct *CagdPrimCone2Srf(const CagdVType Center,
				CagdRType MajorRadius,
				CagdRType MinorRadius,
				CagdRType Height,
				CagdBType Rational,
				CagdPrimCapsType Caps)
{
    IRIT_STATIC_DATA CagdPtStruct
	XAxisPt =  { NULL, NULL, { 1.0, 0.0, 0.0 } },
	ZXAxisPt = { NULL, NULL, { 1.0, 0.0, 1.0 } },
	ZAxisPt =  { NULL, NULL, { 0.0, 0.0, 1.0 } };
    IRIT_STATIC_DATA CagdPtStruct
        Origin = { NULL, NULL, { 0.0, 0.0, 0.0 } };
    CagdSrfStruct *Cone;
    CagdCrvStruct *CrvAux, *CrvAux2, *Cross;

    /* Build the cross section. */
    XAxisPt.Pt[0] = MajorRadius;
    ZXAxisPt.Pt[0] = MinorRadius;
    ZXAxisPt.Pt[2] = Height;
    ZAxisPt.Pt[2] = Height;
    if (IRIT_APX_EQ(MinorRadius, 0.0)) {
	CrvAux2 = CagdMergePtPt(&XAxisPt, &ZAxisPt);
    }
    else if (Caps != CAGD_PRIM_CAPS_TOP && Caps != CAGD_PRIM_CAPS_BOTH) {
	CrvAux2 = CagdMergePtPt(&XAxisPt, &ZXAxisPt);
    }
    else {
	CrvAux = CagdMergePtPt(&ZXAxisPt, &ZAxisPt);
	CrvAux2 = CagdMergePtCrv(&XAxisPt, CrvAux);
	CagdCrvFree(CrvAux);
    }

    if (Caps == CAGD_PRIM_CAPS_BOTTOM || Caps == CAGD_PRIM_CAPS_BOTH) {
        Cross = CagdMergePtCrv(&Origin, CrvAux2);
	CagdCrvFree(CrvAux2);
    }
    else
        Cross = CrvAux2;

    if (Rational)
	Cone = CagdSurfaceRev(Cross);
    else
	Cone = CagdSurfaceRevPolynomialApprox(Cross);

    CagdCrvFree(Cross);

    CagdSrfTransform(Cone, Center, 1.0);

    CAGD_SET_GEOM_TYPE(Cone, CAGD_GEOM_CONICAL);

    return Cone;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a cone, centered at Center, radii of Radius,    M
* and height of Height.	 Axis of cone is Z axis.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed cone (center of its base).                 M
*   Radius:        of constructed cone's base.                               M
*   Height:	   of constructed cone.                                      M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a cone.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, M
*   CagdPrimCylinderSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimConeSrf                                                          M
*****************************************************************************/
CagdSrfStruct *CagdPrimConeSrf(const CagdVType Center,
			       CagdRType Radius,
			       CagdRType Height,
			       CagdBType Rational,
			       CagdPrimCapsType Caps)
{
    return CagdPrimCone2Srf(Center, Radius, 0.0, Height, Rational, Caps);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A surface constructor of a Cylinder, centered at Center, radii of        M
* Radius, and height of Height.	 Axis of cylinder is Z axis.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:        of constructed Cylinder (center of its base).             M
*   Radius:        of constructed Cylinder.                                  M
*   Height:	   of constructed Cylinder.                                  M
*   Rational:      If TRUE exact ration sphere is created.  If FALSE an	     M
*		   approximated integral surface is created.		     M
*   Caps:	   Do we want caps (top and/or bottom) for the cone?	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing a cylinder.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPrimPlaneSrf, CagdPrimSphereSrf, CagdPrimTorusSrf, CagdPrimCone2Srf, M
*   CagdPrimConeSrf							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPrimCylinderSrf                                                      M
*****************************************************************************/
CagdSrfStruct *CagdPrimCylinderSrf(const CagdVType Center,
				   CagdRType Radius,
				   CagdRType Height,
				   CagdBType Rational,
				   CagdPrimCapsType Caps)
{
    CagdSrfStruct
	*Cyl = CagdPrimCone2Srf(Center, Radius, Radius,
				Height, Rational, Caps);

    CAGD_SET_GEOM_TYPE(Cyl, CAGD_GEOM_CYLINDRICAL);

    return Cyl;
}
