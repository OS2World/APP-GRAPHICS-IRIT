/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to generate the geometric primitives defined in the system. The   *
* following lower level operations are defined to create objects -           *
* EXTRUDE, RULED and SURFREV, requiring a polygon/curve and a vector or      *
* two polys to extrude/rule/rotate the curve/polygon along.		     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_loc.h"

static IPPolygonStruct *GenInsidePoly(IPPolygonStruct *Pl);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create surface of revolution by rotating the given cross    M
* section along the Z axis.						     M
*   Input can either be a polygon/line or a freefrom curve object.           M
*   If input is a polyline/gon, it must never be coplanar with the Z axis.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming a surface of revolution.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A surface of revolution.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVAxisObject, PrimGenSURFREV2Object,      M
*   PrimGenSURFREV2AxisObject, PrimGenEXTRUDEObject, PrimGenRULEDObject      M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenSURFREVObject, surface of revolution, primitives                  M
*****************************************************************************/
IPObjectStruct *PrimGenSURFREVObject(const IPObjectStruct *Cross)
{
    int i;
    IrtHmgnMatType Mat;			   /* Rotation Matrix around Z axes. */
    IPVertexStruct *V1, *V1Head, *V2, *V2Head, *VIn, *VInHead;
    IPPolygonStruct *Pl1, *Pl2, *PlIn,
	*PlNew = NULL;
    IPObjectStruct *PSurfRev;

    if (IP_IS_POLY_OBJ(Cross)) {
	if (IRIT_APX_EQ(Cross -> U.Pl -> Plane[0], 0.0) &&
	    IRIT_APX_EQ(Cross -> U.Pl -> Plane[1], 0.0)) {
	    IRIT_WARNING_MSG("Cross-section perpendicular to Z. Empty object result");
	    return NULL;
	}

	Pl1 = IPAllocPolygon(0,
		   V1Head = IPCopyVertexList(Cross -> U.Pl -> PVertex), NULL);
	IRIT_PLANE_COPY(Pl1 -> Plane, Cross -> U.Pl -> Plane);
	Pl2 = IPAllocPolygon(0,
		   V2Head = IPCopyVertexList(Cross -> U.Pl -> PVertex), NULL);
	IRIT_PLANE_COPY(Pl2 -> Plane, Cross -> U.Pl -> Plane);
	PlIn = GenInsidePoly(Pl1);
	VInHead = PlIn -> PVertex;
	MatGenMatRotZ1(M_PI_MUL_2 / _PrimGlblResolution, Mat);

	for (i = 0; i < _PrimGlblResolution; i++) {
	    V2 = V2Head;
	    do {
		MatMultPtby4by4(V2 -> Coord, V2 -> Coord , Mat);
		V2 = V2 -> Pnext;
	    }
	    while (V2 != NULL && V2 != V2Head);

	    V1 = V1Head;
	    if (i < _PrimGlblResolution - 1)    /* If last loop use original */
	        V2 = V2Head; /* poly as we might accumulate error during the */
	    else			/* transformations along the circle. */
		V2 = Cross -> U.Pl -> PVertex;
	    VIn = VInHead;

	    do {
	        int Rvrsd, j;
		IrtRType XYSize;
		IPVertexStruct *V;

		PlNew = PrimGenPolygon4Vrtx(V1 -> Coord, V1 -> Pnext -> Coord,
					    V2 -> Pnext -> Coord, V2 -> Coord,
					    VIn -> Coord, &Rvrsd, PlNew);

	        /* Update normals: */
	        for (j = 0, V = PlNew -> PVertex; j < 4; j++, V = V -> Pnext) {
		    V -> Normal[0] = V -> Coord[0];
		    V -> Normal[1] = V -> Coord[1];
		    V -> Normal[2] = 0.0;
		    /* Make sure normal does not point in opposite direction.*/
		    if (IRIT_DOT_PROD(V -> Normal, PlNew -> Plane) < 0.0)
		        IRIT_PT_SCALE(V -> Normal, -1.0);

		    /* Z normal component should be fixed for all normals:   */
		    V -> Normal[2] = PlNew -> Plane[2] + IRIT_UEPS;
		    XYSize = IRIT_SQR(V -> Coord[0]) + IRIT_SQR(V -> Coord[1]);
		    if (XYSize > IRIT_EPS &&
			!IRIT_APX_EQ(IRIT_FABS(PlNew -> Plane[2]), 1.0)) {
		        XYSize = 1 - IRIT_SQR(PlNew -> Plane[2]);
			XYSize = sqrt(XYSize / (IRIT_SQR(V -> Coord[0]) +
						IRIT_SQR(V -> Coord[1])));
			V -> Normal[0] *= XYSize;
			V -> Normal[1] *= XYSize;
		    }
		    else
		        V -> Normal[2] = IRIT_SIGN(V -> Normal[2]);

		    IRIT_VEC_NORMALIZE(V -> Normal);
		    IP_SET_NORMAL_VRTX(V);
	        }

	        VIn = VIn -> Pnext;
	        V1 = V1 -> Pnext;
	        V2 = V2 -> Pnext;
	    }
	    while (V1 -> Pnext != NULL && V1 != V1Head);

	    V1 = V1Head;
	    do {
	        MatMultPtby4by4(V1 -> Coord, V1 -> Coord , Mat);
	        V1 = V1 -> Pnext;
	    }
	    while (V1 != NULL && V1 != V1Head);
	    VIn = VInHead;
	    do {
	        MatMultPtby4by4(VIn -> Coord, VIn -> Coord , Mat);
	        VIn = VIn -> Pnext;
	    }
	    while (VIn != NULL && VIn != VInHead);
        }

        IPFreePolygonList(PlIn);
        IPFreePolygonList(Pl1);
        IPFreePolygonList(Pl2);

        PSurfRev = IPGenPolyObject("", NULL, NULL);
	PSurfRev -> U.Pl = GMCleanUpPolygonList(&PlNew, IRIT_EPS);

        return PSurfRev;
    }
    else if (IP_IS_CRV_OBJ(Cross)) {
	if (CAGD_NUM_OF_PT_COORD(Cross -> U.Crvs -> PType) < 3) {
	    IRIT_WARNING_MSG("Cross-section perpendicular to Z. Empty object result");
	    return NULL;
	}

        PSurfRev = IPGenSRFObject(CagdSurfaceRev(Cross -> U.Crvs));
	return PSurfRev;
    }
    else {                /* !IP_IS_POLY_OBJ(Cross) && !IP_IS_CRV_OBJ(Cross) */
	IRIT_WARNING_MSG("Cross section is not poly/crv. Empty object result");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create surface of revolution by rotating the given cross    M
* section along the Axis axis.						     M
*   Input can either be a polygon/line or a freefrom curve object.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around Axis axis forming a surface of revolution.   M
*   Axis:      Axis of rotation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A surface of revolution.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVObject, PrimGenSURFREV2Object,          M
*   PrimGenSURFREV2AxisObject, PrimGenEXTRUDEObject, PrimGenRULEDObject      M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenSURFREVAxisObject, surface of revolution, primitives              M
*****************************************************************************/
IPObjectStruct *PrimGenSURFREVAxisObject(IPObjectStruct *Cross,
					 const IrtVecType Axis)
{
    IrtVecType UnitAxis;
    IrtHmgnMatType Mat, InvMat;
    IPObjectStruct *CrossTmp, *PObj, *PObjTmp;

    IRIT_VEC_COPY(UnitAxis, Axis);
    IRIT_VEC_NORMALIZE(UnitAxis);

    GMGenMatrixZ2Dir(Mat, UnitAxis);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    CrossTmp = GMTransformObject(Cross, InvMat);
    PObjTmp = PrimGenSURFREVObject(CrossTmp);
    IPFreeObject(CrossTmp);

    if (PObjTmp == NULL)
	return NULL;
    PObj = GMTransformObject(PObjTmp, Mat);
    IPFreeObject(PObjTmp);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create surface of revolution by rotating the given cross    M
* section along the Z axis, from StartAngle to EndAngle.		     M
*   Input can either be a polygon/line or a freefrom curve object.           M
*   If input is a polyline/gon, it must never be coplanar with the Z axis.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming a surface of revolution.  M
*   StartAngle, EndAngle:  angles of portion of surface of revolution,	     M
*			   in degrees, between 0 and 360.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A (portion of) a surface of revolution.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVObject, PrimGenSURFREVAxisObject,       M
*   PrimGenSURFREV2AxisObject, PrimGenEXTRUDEObject, PrimGenRULEDObject      M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenSURFREV2Object, surface of revolution, primitives                 M
*****************************************************************************/
IPObjectStruct *PrimGenSURFREV2Object(const IPObjectStruct *Cross,
				      IrtRType StartAngle,
				      IrtRType EndAngle)
{
    int i, j, Resolution;
    IrtRType XYSize,
	DAngles = EndAngle - StartAngle;
    IrtHmgnMatType Mat;			   /* Rotation Matrix around Z axes. */
    IPVertexStruct *V, *V1, *V1Head, *V2, *V2Head, *VIn, *VInHead;
    IPPolygonStruct *Pl1, *Pl2, *PlIn, *PlNew = NULL;
    IPObjectStruct *PSurfRev;

    if (IP_IS_POLY_OBJ(Cross)) {
	if (IRIT_APX_EQ(Cross -> U.Pl -> Plane[0], 0.0) &&
	    IRIT_APX_EQ(Cross -> U.Pl -> Plane[1], 0.0)) {
	    IRIT_WARNING_MSG("Cross-section perpendicular to Z. Empty object result");
	    return NULL;
	}

	if (DAngles < 0) {
	    IRIT_SWAP(IrtRType, EndAngle, StartAngle);
	    DAngles = -DAngles;
	}
	else if (DAngles == 0) {
	    IRIT_WARNING_MSG("Start and End angles of surface of revolution must be different");
	    return NULL;
	}
	Resolution = (int) (_PrimGlblResolution * DAngles / 360.0);
	if (Resolution < 2)
	    Resolution = 2;

	/* Bring the two cross sections to StartAngle angle. */
	MatGenMatRotZ1(IRIT_DEG2RAD(StartAngle), Mat);
	Pl1 = IPAllocPolygon(0,
		   V1Head = IPCopyVertexList(Cross -> U.Pl -> PVertex), NULL);
	IPUpdatePolyPlane(Pl1);
	V1 = V1Head;
	do {
	    MatMultPtby4by4(V1 -> Coord, V1 -> Coord , Mat);
	    V1 = V1 -> Pnext;
	}
	while (V1 != NULL && V1 != V1Head);

	Pl2 = IPAllocPolygon(0, V2Head = IPCopyVertexList(Pl1 -> PVertex), NULL);
	IPUpdatePolyPlane(Pl2);

	PlIn = GenInsidePoly(Pl1);
	VInHead = PlIn -> PVertex;

	MatGenMatRotZ1(IRIT_DEG2RAD(DAngles) / Resolution, Mat);

	for (i = 0; i < Resolution; i++) {
	    V2 = V2Head;
	    do {
		MatMultPtby4by4(V2 -> Coord, V2 -> Coord , Mat);
		V2 = V2 -> Pnext;
	    }
	    while (V2 != NULL && V2 != V2Head);

	    V1 = V1Head;
	    V2 = V2Head;
	    VIn = VInHead;

	    do {
	        int Rvrsd;

		PlNew = PrimGenPolygon4Vrtx(V1 -> Coord, V1 -> Pnext -> Coord,
					    V2 -> Pnext -> Coord, V2 -> Coord,
					    VIn -> Coord, &Rvrsd, PlNew);

	        /* Update normals: */
	        for (j = 0, V = PlNew -> PVertex; j < 4; j++, V = V -> Pnext) {
		    V -> Normal[0] = V -> Coord[0];
		    V -> Normal[1] = V -> Coord[1];
		    V -> Normal[2] = 0.0;
		    /* Make sure normal does not point in opposite direction.*/
		    if (IRIT_DOT_PROD(V -> Normal, PlNew -> Plane) < 0.0)
		        IRIT_PT_SCALE(V -> Normal, -1.0);

		    /* Z normal component should be fixed for all normals:   */
		    V -> Normal[2] = PlNew -> Plane[2];
		    XYSize = IRIT_APX_EQ(IRIT_FABS(PlNew -> Plane[2]), 1.0) ?
					0.0 : 1 - IRIT_SQR(PlNew -> Plane[2]);
		    XYSize = sqrt(XYSize / (IRIT_SQR(V -> Coord[0]) +
					    IRIT_SQR(V -> Coord[1])));
		    V -> Normal[0] *= XYSize;
		    V -> Normal[1] *= XYSize;

		    IP_SET_NORMAL_VRTX(V);
	        }

	        VIn = VIn -> Pnext;
	        V1 = V1 -> Pnext;
	        V2 = V2 -> Pnext;
	    }
	    while (V1 -> Pnext != NULL && V1 != V1Head);

	    V1 = V1Head;
	    do {
	        MatMultPtby4by4(V1 -> Coord, V1 -> Coord , Mat);
	        V1 = V1 -> Pnext;
	    }
	    while (V1 != NULL && V1 != V1Head);
	    VIn = VInHead;
	    do {
	        MatMultPtby4by4(VIn -> Coord, VIn -> Coord , Mat);
	        VIn = VIn -> Pnext;
	    }
	    while (VIn != NULL && VIn != VInHead);
        }

        IPFreePolygonList(PlIn);
        IPFreePolygonList(Pl1);
        IPFreePolygonList(Pl2);

        PSurfRev = IPGenPolyObject("", NULL, NULL);
	PSurfRev -> U.Pl = GMCleanUpPolygonList(&PlNew, IRIT_EPS);

        return PSurfRev;
    }
    else if (IP_IS_CRV_OBJ(Cross)) {
	if (CAGD_NUM_OF_PT_COORD(Cross -> U.Crvs -> PType) < 3) {
	    IRIT_WARNING_MSG("Cross-section perpendicular to Z. Empty object result");
	    return NULL;
	}

        PSurfRev = IPGenSRFObject(CagdSurfaceRev2(Cross -> U.Crvs,
						  StartAngle, EndAngle));
	return PSurfRev;
    }
    else {                /* !IP_IS_POLY_OBJ(Cross) && !IP_IS_CRV_OBJ(Cross) */
	IRIT_WARNING_MSG("Cross section is not poly/crv. Empty object result");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create surface of revolution by rotating the given cross    M
* section along the Axis axis.						     M
*   Input can either be a polygon/line or a freefrom curve object.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around Axis axis forming a surface of revolution.   M
*   StartAngle, EndAngle:  angles of portion of surface of revolution,	     M
*			   in degrees, between 0 and 360.		     M
*   Axis:      Axis of rotation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A (portion of) a surface of revolution.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVObject, PrimGenSURFREVAxisObject,       M
*   PrimGenSURFREV2Object, PrimGenEXTRUDEObject, PrimGenRULEDObject          M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenSURFREV2AxisObject, surface of revolution, primitives             M
*****************************************************************************/
IPObjectStruct *PrimGenSURFREV2AxisObject(IPObjectStruct *Cross,
					  IrtRType StartAngle,
					  IrtRType EndAngle,
					  const IrtVecType Axis)
{
    IrtVecType UnitAxis;
    IrtHmgnMatType Mat, InvMat;
    IPObjectStruct *CrossTmp, *PObj, *PObjTmp;

    IRIT_VEC_COPY(UnitAxis, Axis);
    IRIT_VEC_NORMALIZE(UnitAxis);

    GMGenMatrixZ2Dir(Mat, UnitAxis);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    CrossTmp = GMTransformObject(Cross, InvMat);
    PObjTmp = PrimGenSURFREV2Object(CrossTmp, StartAngle, EndAngle);
    IPFreeObject(CrossTmp);

    if (PObjTmp == NULL)
	return NULL;
    PObj = GMTransformObject(PObjTmp, Mat);
    IPFreeObject(PObjTmp);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create an extrusion surface out of the given cross section  M
* and the given direction.                                                   M
*   Input can either be a polygon/line or a freefrom curve object.           M
*   If input is a polyline/gon, it must never be coplanar with Dir.          M
*   See also PrimSetResolution on fineness control of approximation of the   M
* primitive using flat faces.                                                M
*									     M
* PARAMETERS:                                                                M
*   Cross:     To extrude in direction Dir.                                  M
*   Dir:       Direction and magnitude of extrusion.                         M
*   Bases:     0 for none, 1 for bottom, 2 for top, 3 for both.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     An extrusion surface.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVObject, PrimGenSURFREVAxisObject,       M
*   PrimGenSURFREV2Object, PrimGenSURFREV2AxisObject, PrimGenRULEDObject     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenEXTRUDEObject, extrusion surface, primitives                      M
*****************************************************************************/
IPObjectStruct *PrimGenEXTRUDEObject(const IPObjectStruct *Cross,
				     const IrtVecType Dir,
				     int Bases)
{
    IPObjectStruct *PExtrude;

    if (!IP_IS_POLY_OBJ(Cross) && !IP_IS_CRV_OBJ(Cross)) {
	IRIT_WARNING_MSG("Cross section is not poly/crv. Empty object result");
	return NULL;
    }

    if (IP_IS_POLY_OBJ(Cross)) {
	int PolyLn = IP_IS_POLYLINE_OBJ(Cross);
	IrtRType
	    R = PolyLn ? 1.0 : IRIT_DOT_PROD(Cross -> U.Pl -> Plane, Dir);
	IPVertexStruct *V1, *V1Head, *V2, *VIn;
	IPPolygonStruct *PBase1, *PBase2, *PlIn,
	    *Pl = NULL;

	if (IRIT_APX_EQ(R, 0.0)) {
	    IRIT_WARNING_MSG("Extrusion direction in cross-section plane. Empty object result");
	    return NULL;
	}

	/* Prepare two bases (update their plane normal to point INDISE): */
	PBase1 = IPAllocPolygon(0,
				IPCopyVertexList(Cross -> U.Pl -> PVertex),
				NULL);
	PBase2 = IPAllocPolygon(0,
				IPCopyVertexList(Cross -> U.Pl -> PVertex),
				NULL);
	V1 = V1Head = PBase2 -> PVertex;
	do {
	    IRIT_PT_ADD(V1 -> Coord, Dir, V1 -> Coord);
	    V1 = V1 -> Pnext;
	}
	while (V1 != NULL && V1 != V1Head);

	if (!PolyLn) {
	    int i;

	    if (R > 0.0) {
		IRIT_PLANE_COPY(PBase1 -> Plane, Cross -> U.Pl -> Plane);
		for (i = 0; i < 3; i++)
		    PBase2 -> Plane[i] = (-Cross -> U.Pl -> Plane[i]);
		PBase2 -> Plane[3] = (-IRIT_DOT_PROD(PBase2 -> Plane,
						PBase2 -> PVertex -> Coord));
	    }
	    else {
		for (i = 0; i < 4; i++)
		    PBase1 -> Plane[i] = (-Cross -> U.Pl -> Plane[i]);
		IRIT_PLANE_COPY(PBase2 -> Plane, Cross -> U.Pl -> Plane);
		PBase2 -> Plane[3] = (-IRIT_DOT_PROD(PBase2 -> Plane,
						PBase2 -> PVertex -> Coord));
	    }
	    IP_SET_PLANE_POLY(PBase1);
	    IP_SET_PLANE_POLY(PBase2);
	}

	/* Now generate all the 4 corner polygon between the two bases: */
	V1 = V1Head = PBase1 -> PVertex;
	V2 = PBase2 -> PVertex;
	if (PolyLn) {
	    PlIn = NULL;
	    VIn = NULL;
	}
	else {
	    PlIn = GenInsidePoly(PBase1);
	    VIn = PlIn -> PVertex;
	}

	do {
	    int Rvrsd;

	    Pl = PrimGenPolygon4Vrtx(V1 -> Coord, V1 -> Pnext -> Coord,
				     V2 -> Pnext -> Coord, V2 -> Coord,
				     VIn != NULL ? VIn -> Coord : NULL,
				     &Rvrsd, Pl);

	    if (VIn != NULL)
	        VIn = VIn -> Pnext;
	    V1 = V1 -> Pnext;
	    V2 = V2 -> Pnext;
	}
	while (V1 -> Pnext != NULL && V1 != V1Head);

	if (PlIn != NULL)
	    IPFreePolygonList(PlIn);

	PExtrude = IPGenPolyObject("", NULL, NULL);
	PExtrude -> U.Pl = Pl;

	if (PolyLn || Bases == 0) {
	    /* Throw bases away. */
	    IPFreePolygon(PBase1);
	    IPFreePolygon(PBase2);
	}
	else {
	    /* Properly orient the vertices of the two bases. */
	    if (IRIT_DOT_PROD(Cross -> U.Pl -> Plane, PBase1 -> Plane) < 0.0) {
		/* Reverse the polygon. */
		IPReverseVrtxList(PBase1);
	    }
	    if (IRIT_DOT_PROD(Cross -> U.Pl -> Plane, PBase2 -> Plane) < 0.0) {
		/* Reverse the polygon. */
		IPReverseVrtxList(PBase2);
	    }

	    /* Place the bases at the end. */
	    Pl = IPGetLastPoly(Pl);

	    if (Bases & 0x01) {
	        Pl -> Pnext = PBase1;
		Pl = PBase1;
	    }
	    else 
	        IPFreePolygon(PBase1);

	    if (Bases & 0x02) {
	        Pl -> Pnext = PBase2;
	    }
	    else
	        IPFreePolygon(PBase2);
	}

	/* Update all the polygon vertices normals. */
	for (Pl = PExtrude -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    V1 = V1Head = Pl -> PVertex;
    	    do {
    	        IRIT_PT_COPY(V1 -> Normal, Pl -> Plane);
    	        V1 = V1 -> Pnext;
    	    }
    	    while (V1 != NULL && V1 != V1Head);
	}

	return PExtrude;
    }
    else if (IP_IS_CRV_OBJ(Cross)) {
	int i,
	    MaxDim = 3;
	CagdVecStruct CagdDir;
	CagdCrvStruct *CrossCrv;
	CagdSrfStruct *Srf;

	if (Dir[2] == 0.0) {
	    MaxDim = 2;

	    if (Dir[1] == 0.0)
	        MaxDim = 1;
	}

	if (CAGD_NUM_OF_PT_COORD(Cross -> U.Crvs -> PType) >= MaxDim) {
	    CrossCrv = CagdCrvCopy(Cross -> U.Crvs);
	}
	else {
	    CrossCrv = CagdCoerceCrvTo(Cross -> U.Crvs,
		 CAGD_MAKE_PT_TYPE(CAGD_IS_RATIONAL_CRV(Cross -> U.Crvs),
				   MaxDim), FALSE);
	}

        for (i = 0; i < 3; i++)
	    CagdDir.Vec[i] = Dir[i];
	Srf = CagdExtrudeSrf(CrossCrv, &CagdDir);

	if (Bases == 0)
	    PExtrude = IPGenSRFObject(Srf);
	else {
	    int Count = 0;

	    PExtrude = IPAllocObject("Extrusion", IP_OBJ_LIST_OBJ, NULL);
	    IPListObjectInsert(PExtrude, Count++, IPGenSRFObject(Srf));
	    if (CagdIsClosedCrv(CrossCrv)) {
	        CagdRType UMin, UMax, VMin, VMax;
		CagdVecStruct *Nrml;

		if (Bases & 0x01) {
		    Srf = CagdOneBoolSumSrf(CrossCrv);

		    /* Orient the surface properly. */
		    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
		    Nrml = CagdSrfNormal(Srf, (UMin + UMax) * 0.5,
				              (VMin + VMax) * 0.5, FALSE);
		    if (IRIT_DOT_PROD(Nrml -> Vec, Dir) < 0) {
		        CagdSrfStruct
			    *TSrf = CagdSrfReverse2(Srf);

			CagdSrfFree(Srf);
			Srf = TSrf;
		    }

		    IPListObjectInsert(PExtrude, Count++,
				       IPGenSrfObject("Base1", Srf, NULL));
		}
		if (Bases & 0x02) {
		    CagdCrvTransform(CrossCrv, Dir, 1.0);
		    Srf = CagdOneBoolSumSrf(CrossCrv);

		    /* Orient the surface properly. */
		    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
		    Nrml = CagdSrfNormal(Srf, (UMin + UMax) * 0.5,
				              (VMin + VMax) * 0.5, FALSE);
		    if (IRIT_DOT_PROD(Nrml -> Vec, Dir) > 0) {
		        CagdSrfStruct
			    *TSrf = CagdSrfReverse2(Srf);

			CagdSrfFree(Srf);
			Srf = TSrf;
		    }

		    IPListObjectInsert(PExtrude, Count++,
				       IPGenSrfObject("Base2", Srf, NULL));
		}
	    }
	    IPListObjectInsert(PExtrude, Count++, NULL);
	}

	CagdCrvFree(CrossCrv);

        return PExtrude;
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to a create a ruled surface out of the given two cross sections. M
*									     *
* PARAMETERS:                                                                M
*   Cross1, Cross2:     Polylines to rule a surface between.                 M
*			If both cross sections are in the XY plane, a single M
*			planar polygon is constructed. Otherwise, the number M
*			of vertices in Cross1 and Cross2 must be equal and   M
*			a rectangular polygon is constructed for each edge.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     A single polygon representing the ruled surface.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimSetResolution, PrimGenSURFREVObject, PrimGenSURFREVAxisObject,       M
*   PrimGenSURFREV2Object, PrimGenSURFREV2AxisObject, PrimGenEXTRUDEObject   M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenRULEDObject, ruled surface, primitives                            M
*****************************************************************************/
IPObjectStruct *PrimGenRULEDObject(const IPObjectStruct *Cross1,
				   const IPObjectStruct *Cross2)
{
    IPVertexStruct *V1, *V1Head, *V2;
    IPObjectStruct *PRuled;
    IPPolygonStruct
	*Pl = NULL,
	*Pl1 = Cross1 -> U.Pl,
	*Pl2 = Cross2 -> U.Pl;
    int XYPlane = TRUE;

    if (!IP_IS_POLY_OBJ(Cross1) || !IP_IS_POLY_OBJ(Cross2)) {
	IRIT_WARNING_MSG("Cross sections are not polylines. Empty object result");
	return NULL;
    }

    /* Do we have polylines in the XY plane!? */
    V1 = Pl1 -> PVertex;
    do {
        XYPlane &= IRIT_APX_EQ(V1 -> Coord[2], 0.0);
	V1 = V1 -> Pnext;
    }
    while (V1 != NULL && V1 != Pl1 -> PVertex && XYPlane);

    V2 = Pl2 -> PVertex;
    do {
        XYPlane &= IRIT_APX_EQ(V2 -> Coord[2], 0.0);
	V2 = V2 -> Pnext;
    }
    while (V2 != NULL && V2 != Pl2 -> PVertex && XYPlane);

    if (XYPlane) {
	PRuled =
	    IPGenPOLYObject(IPAllocPolygon(0,
				   IPCopyVertexList(Cross1 -> U.Pl -> PVertex),
				   NULL));
	V1 = IPGetLastVrtx(PRuled -> U.Pl -> PVertex);
	V1 -> Pnext =
	    IPReverseVrtxList2(IPCopyVertexList(Cross2 -> U.Pl -> PVertex));
	V1 = IPGetLastVrtx(V1);
	V1 -> Pnext = PRuled -> U.Pl -> PVertex;   /* Make vertex list circ. */
        IPUpdatePolyPlane(PRuled -> U.Pl);
    }
    else {
	int Rvrsd;

	if (IPVrtxListLen(Pl1 -> PVertex) !=
	    IPVrtxListLen(Pl2 -> PVertex)) {
	    IRIT_WARNING_MSG("Cross sections are not of same number of points. Empty object result");
	    return NULL;
	}

	V1 = V1Head = Pl1 -> PVertex;
	V2 = Pl2 -> PVertex;
	do {
	    Pl = PrimGenPolygon4Vrtx(V1 -> Coord, V1 -> Pnext -> Coord,
				     V2 -> Pnext -> Coord, V2 -> Coord,
				     NULL, &Rvrsd, Pl);
	    V1 = V1 -> Pnext;
	    V2 = V2 -> Pnext;
	}
	while (V1 -> Pnext != NULL && V1 != V1Head);

	/* Add the last closing edge. */
	if (IP_IS_POLYGON_OBJ(Cross1) && IP_IS_POLYGON_OBJ(Cross2))
	    Pl = PrimGenPolygon4Vrtx(V1 -> Coord, Pl1 -> PVertex -> Coord,
				     Pl1 -> PVertex -> Coord, V2 -> Coord,
				     NULL, &Rvrsd, Pl);

	PRuled = IPGenPOLYObject(GMCleanUpPolygonList(&Pl, IRIT_EPS));
    }

    /* Update all the polygon's vertices normals. */
    for (Pl = PRuled -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	V1 = V1Head = Pl -> PVertex;
	do {
	    IRIT_PT_COPY(V1 -> Normal, Pl -> Plane);
	    V1 = V1 -> Pnext;
	}
	while (V1 != NULL && V1 != V1Head);
    }

    IP_SET_POLYGON_OBJ(PRuled);

    return PRuled;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to create a pseudo polygon out of a given polygon such that each *
* vertex Vi is in the inside side of the corresponding edge ViVi+1 in the    *
* given polygon. Used in polygon generation for EXTRUDE/SURFREV operations.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:        Input polygon                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  Pseudo one.                                          *
*****************************************************************************/
static IPPolygonStruct *GenInsidePoly(IPPolygonStruct *Pl)
{
    int Axes;
    IrtRType Dx, Dy;
    IrtPtType Pt;
    IrtHmgnMatType Mat;
    IPPolygonStruct *PlIn;
    IPVertexStruct *VHead, *V, *Vnext, *VInHead,
	*VIn = NULL;

    PlIn = IPAllocPolygon(0, VInHead = NULL, NULL);

    /* Generate transformation matrix to bring polygon to a XY parallel      */
    /* plane, and transform a copy of the polygon to that plane.	     */
    GMGenRotateMatrix(Mat, Pl -> Plane);
    /* We dont want to modify original! */
    VHead = V = IPCopyVertexList(Pl -> PVertex);
    Pl = IPAllocPolygon(0, VHead, NULL);
    do {
	MatMultPtby4by4(V -> Coord, V -> Coord, Mat);
	V = V -> Pnext;
    }
    while (V != NULL && V != VHead);

    V = VHead;
    do {
	Vnext = V -> Pnext;
	Dx = IRIT_FABS(V -> Coord[0] - Vnext -> Coord[0]);
	Dy = IRIT_FABS(V -> Coord[1] - Vnext -> Coord[1]);
	/* Prepare middle point. */
	Pt[0] = (V -> Coord[0] + Vnext -> Coord[0]) * 0.5;
	Pt[1] = (V -> Coord[1] + Vnext -> Coord[1]) * 0.5;
	Pt[2] = V -> Coord[2];
	/* If Dx > Dy fire ray in +Y direction, otherwise in +X direction    */
	/* and if number of intersections is even (excluding the given point */
	/* itself) then that direction is the outside, otherwise, its inside.*/
	Axes = (Dx > Dy ? 1 : 0);
	if (GMPolygonRayInter(Pl, Pt, Axes) % 2 == 0) {
	    /* The amount we move along Axes is not of a big meaning as long */
	    /* as it is not 0, so IRIT_MAX(Dx, Dy) guarantee non zero value. */
	    Pt[Axes] -= IRIT_MAX(Dx, Dy);
	}
	else {
	    Pt[Axes] += IRIT_MAX(Dx, Dy);
	}

	/* Now Pt holds point which is in the inside part of vertex V, Vnext.*/
	/* Put it in the pseudo inside polygon PlIn:			     */
	if (VInHead) {
	    VIn -> Pnext = IPAllocVertex2(NULL);
	    VIn = VIn -> Pnext;
	}
	else {
	    PlIn -> PVertex = VInHead = VIn = IPAllocVertex2(NULL);
	}
	IRIT_PT_COPY(VIn -> Coord, Pt);

	V = Vnext;
    }
    while (V != NULL && V != VHead);
    VIn -> Pnext = VInHead;

    IPFreePolygonList(Pl);	      /* Free copied (and trans.) vrtx list. */

    /* Transform PlIn to the plane where original Pl is... */
    MatTranspMatrix(Mat, Mat);			     /* Compute the inverse. */
    VIn = VInHead;
    do {
	MatMultPtby4by4(VIn -> Coord, VIn -> Coord, Mat);
	VIn = VIn -> Pnext;
    }
    while (VIn != NULL && VIn != VInHead);

    return PlIn;
}
