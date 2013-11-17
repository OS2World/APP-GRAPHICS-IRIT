/*****************************************************************************
* Handle geometry of parser of "irit" solid modeller.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, Sep. 2000   *
*****************************************************************************/

#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"

#define CONVEX_IRIT_EPS		1e-3

IRIT_GLOBAL_DATA int
    _IPPolyListCirc = FALSE,
    IPWasViewMat = FALSE,
    IPWasPrspMat = FALSE;
IRIT_GLOBAL_DATA IrtHmgnMatType IPViewMat = {      /* Isometric view, by default. */
    { -0.707107, -0.408248, 0.577350, 0.000000 },
    {  0.707107, -0.408248, 0.577350, 0.000000 },
    {  0.000000,  0.816496, 0.577350, 0.000000 },
    {  0.000000,  0.000000, 0.000000, 1.000000 }
};
IRIT_GLOBAL_DATA IrtHmgnMatType IPPrspMat = {
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, -0.35 },
    { 0, 0, 0, 1.0 }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches the current view matrix.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   WasViewMat:  TRUE if parser until now detected a view matrix.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtHmgnMatType *:   A reference to the matrix.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetPrspMat                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetViewMat                                                             M
*****************************************************************************/
IrtHmgnMatType *IPGetViewMat(int *WasViewMat)
{
    *WasViewMat = IPWasViewMat;

    return &IPViewMat;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches the current perspective matrix.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   WasPrspMat:  TRUE if parser until now detected a perspective matrix.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtHmgnMatType *:   A reference to the matrix.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetViewMat                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPGetPrspMat                                                             M
*****************************************************************************/
IrtHmgnMatType *IPGetPrspMat(int *WasPrspMat)
{
    *WasPrspMat = IPWasPrspMat;

    return &IPPrspMat;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to update the Plane equation of the given polygon by the order     M
* of the most robust three vertices of that polygon to define the normal.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:     To update its normal/plane equation.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if succesful, FALSE otherwise.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPUpdatePolyPlane, files, parser                                         M
*****************************************************************************/
int IPUpdatePolyPlane(IPPolygonStruct *PPoly)
{
    IrtRType LenSqr, Len1, V1[3], V2[3],
	MaxLenSqr = IRIT_SQR(IRIT_UEPS);
    IPVertexStruct *VLast, *Vnext,
	*V = PPoly -> PVertex;
    IrtPlnType Plane;

    if (V == NULL || V -> Pnext == NULL || V -> Pnext -> Pnext == NULL)
	IP_FATAL_ERROR(IP_ERR_LESS_THAN_3_VRTCS);

    /* Force list to be circular. Will be recovered immediately after. */
    VLast = IPGetLastVrtx(V);
    if (VLast -> Pnext != NULL)
        VLast = NULL;				/* List is circular already. */
    else
	VLast -> Pnext = V;

    Vnext = V -> Pnext;
    IRIT_PT_SUB(V1, V -> Coord, Vnext -> Coord);
    V = Vnext;
    do {
        Vnext = V -> Pnext;

	IRIT_PT_SUB(V2, V -> Coord, Vnext -> Coord);

	Plane[0] = V1[1] * V2[2] - V2[1] * V1[2];
	Plane[1] = V1[2] * V2[0] - V2[2] * V1[0];
	Plane[2] = V1[0] * V2[1] - V2[0] * V1[1];

	/* Normalize the plane such that the normal has length of 1: */
	if ((LenSqr = IRIT_VEC_SQR_LENGTH(Plane)) > MaxLenSqr) {
	    IRIT_VEC_COPY(PPoly -> Plane, Plane);

	    MaxLenSqr = LenSqr;
	}

	V = Vnext;
	IRIT_VEC_COPY(V1, V2);
    }
    while (V != PPoly -> PVertex -> Pnext);

    if (VLast != NULL)    /* Recover non circular list, if was non circular. */
	VLast -> Pnext = NULL;

    if (MaxLenSqr <= IRIT_SQR(IRIT_UEPS))
	return FALSE;

    Len1 = 1 / sqrt(MaxLenSqr);
    IRIT_VEC_SCALE(PPoly -> Plane, Len1);

    PPoly -> Plane[3] = -IRIT_DOT_PROD(PPoly -> Plane, V -> Coord);

    IP_SET_PLANE_POLY(PPoly);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to update the Plane equation of the given polygon such that the  M
* Vin vertex will be in the positive side of it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:     To update its normal/plane equation.                          M
*   Vin:       A vertex to be considered in the inside, respective to PPoly. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       0 if failed, 1 if successful, -1 if sucessful but vertices    M
*              reversed.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPUpdatePolyPlane2, files, parser                                        M
*****************************************************************************/
int IPUpdatePolyPlane2(IPPolygonStruct *PPoly, const IrtVecType Vin)
{
    int i;

    if (!IPUpdatePolyPlane(PPoly))
        return FALSE;

    if (IRIT_DOT_PROD(PPoly -> Plane, Vin) + PPoly -> Plane[3] < 0) {
	/* Flip plane normal and reverse the vertex list. */
	IPReverseVrtxList(PPoly);
	for (i = 0; i < 4; i++)
	    PPoly -> Plane[i] = (-PPoly -> Plane[i]);

	return -1;
    }

    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to update all vertices in polygon to hold a default normal if      M
* have none already.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:       Polygon to update normal information.                       M
*   DefNrml:     Normal tp use in update.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPUpdateVrtxNrml, files, parser			 		     M
*****************************************************************************/
void IPUpdateVrtxNrml(IPPolygonStruct *PPoly, IrtVecType DefNrml)
{
    IPVertexStruct
	*V = PPoly -> PVertex;

    do {
	if (!IP_HAS_NORMAL_VRTX(V)) {
	    IRIT_PT_COPY(V -> Normal, DefNrml);
	    IP_SET_NORMAL_VRTX(V);
	}
	V = V -> Pnext;
    }
    while (V != NULL && V != PPoly -> PVertex);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls vertex list in polygons. Do we want it circular?		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Circ:     If TRUE, vertex lists of polygons will be circular. If FALSE,  M
*             the lists will be NULL terminated.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      old value of flag.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPGetObjects                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetPolyListCirc, files, parser                                         M
*****************************************************************************/
int IPSetPolyListCirc(int Circ)
{
    int OldVal = _IPPolyListCirc;

    _IPPolyListCirc = Circ;

    return OldVal;
}
