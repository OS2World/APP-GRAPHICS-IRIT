/******************************************************************************
* MshPlanr.c - Test collinearity of control meshes/polygons.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Fits a plane through the four points from Points indices Index?. Points    M
* may be either E2 or E3 only.						     M
*   Returns 0.0 if failed to fit a plane, otherwise a measure on the size of M
* the mesh data (distance between points) is returned.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:       To compute and save here.                                   M
*   PType:       Point type expected of four points. Must be E2 or E3.       M
*   Points:      Point array where to look for the four points.              M
*   Index1, Index2, Index3, Index4:   Four indices of the points.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:     Measure  on the distance between the data points, 0.0 if  M
*                  fitting failed.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdFitPlaneThruCtlPts, plane fit                                        M
*****************************************************************************/
CagdRType CagdFitPlaneThruCtlPts(CagdPlaneStruct *Plane,
				 CagdPointType PType,
			         CagdRType * const *Points,
			         int Index1,
				 int Index2,
				 int Index3,
				 int Index4)
{
    int i, j, Indices[4];
    CagdRType SizeMeasure,
	MaxDist = 0.0;
    CagdVecStruct V1, V2, V3;

    Indices[0] = Index1;
    Indices[1] = Index2;
    Indices[2] = Index3;
    Indices[3] = Index4;

    /* Find out the pair of vertices most separated: */
    for (i = 0; i < 4; i++)
	for (j = i + 1; j < 4; j++) {
	    CagdRType
		Dist = CagdDistTwoCtlPt(Points, Indices[i],
					Points, Indices[j], PType);

	    if (Dist > MaxDist) {
		MaxDist = Dist;
		Index1 = i;
		Index2 = j;
	    }
	}

    if (MaxDist < IRIT_UEPS)
	return 0.0;
    SizeMeasure = MaxDist;
    MaxDist = 0.0;

    /* Find a third most separated than the selected two. */
    for (i = 0; i < 4; i++)
	if (i != Index1 && i != Index2) {
	    CagdRType
		Dist1 = CagdDistTwoCtlPt(Points, Indices[Index1],
					 Points, Indices[i], PType),
		Dist2 = CagdDistTwoCtlPt(Points, Indices[Index2],
					 Points, Indices[i], PType),
		Dist = IRIT_MIN(Dist1, Dist2);

	    if (Dist > MaxDist) {
		MaxDist = Dist;
		Index3 = i;
	    }
	}
    if (MaxDist < IRIT_UEPS)
	return 0.0;

    /* Now we have the 3 most separated vertices to fit a plane thru. */

    switch (PType) {
	case CAGD_PT_E2_TYPE:
	    /* This is trivial since the plane must be Z = 0. */
	    Plane -> Plane[0] = 0.0;
	    Plane -> Plane[1] = 0.0;
	    Plane -> Plane[2] = 1.0;
	    Plane -> Plane[3] = 0.0;
	    break;
	case CAGD_PT_E3_TYPE:
	    /* Compute normal as a cross product of two vectors thru pts. */
	    for (i = 0; i < 3; i++) {
		j = i + 1;
		V1.Vec[i] = Points[j][Index2] - Points[j][Index1];
		V2.Vec[i] = Points[j][Index3] - Points[j][Index2];
	    }
	    IRIT_CROSS_PROD(V3.Vec, V1.Vec, V2.Vec);
	    CAGD_NORMALIZE_VECTOR_MSG_ZERO(V3);
	    for (i = 0; i < 3; i++)
		Plane -> Plane[i] = V3.Vec[i];

	    Plane -> Plane[3] = (-(V3.Vec[0] * Points[1][Index1] +
				   V3.Vec[1] * Points[2][Index1] +
				   V3.Vec[2] * Points[3][Index1]));
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
	    break;
    }

    return SizeMeasure;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes and returns distance between point Index and given plane which is M
* assumed to be normalized, so that the A B C plane;s normal has a unit      M
* length.		                                                     M
*   Also assumes the Points are non rational with MaxDim dimension.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:      To compute the distance to.                                  M
*   Points:     To compute the distance from.                                M
*   Index:      Index in Points for the point to consider.                   M
*   MaxDim:     Number of dimensions to consider. Less or equal to three.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Resulting distance.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDistPtPlane, point plane distance                                    M
*****************************************************************************/
CagdRType CagdDistPtPlane(CagdPlaneStruct const *Plane,
			  CagdRType * const *Points,
			  int Index,
			  int MaxDim)
{
    int i;
    CagdRType
	R = Plane -> Plane[3];

    for (i = 0; i < MaxDim; i++)
	R += Plane -> Plane[i] * Points[i + 1][Index];

    return IRIT_FABS(R);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes and returns distance between point Index and the line from point  *
* index 0 in direction LineDir (usually the line direction to last the       *
* point).                                                                    *
*   LineDir is assumed to be unit length normalized vector.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   LineDir:    To compute distance to.                                      *
*   Plane:      To compute the distance to.                                  *
*   Points:     To compute the distance from.                                *
*   Index:      Index in Points for the point to consider.                   *
*   MaxDim:     Number of dimensions to consider. Less or equal to three.    *
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Resulting distance.                                         M
*****************************************************************************/
CagdRType CagdDistPtLine(CagdVecStruct const *LineDir,
			 CagdRType * const *Points,
			 int Index,
			 int MaxDim)
{
    int i;
    CagdRType R;
    CagdVecStruct V1, V2;

    for (i = 0; i < MaxDim; i++)
	V1.Vec[i] = Points[i+1][Index] - Points[i+1][0];
    if (MaxDim < 3)
	V1.Vec[2] = 0.0;

    /* Let V1 be the vector from point zero to point index. Then distance    */
    /* from point Index to the line is: | (LineDir . V1) LineDir - V1 |.     */
    V2 = *LineDir;
    R = IRIT_DOT_PROD(V1.Vec, V2.Vec);
    CAGD_MULT_VECTOR(V2, R);
    CAGD_SUB_VECTOR(V2, V1);

    return CAGD_LEN_VECTOR(V2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Tests polygonal collinearity by testing the distance of interior control   M
* points from the line connecting the two control polygon end points.	     M
*   Returns a relative ratio of deviation from line relative to its length.  M
*   Zero means all points are collinear.				     M
*   If two end points are same (no line can be fit) IRIT_INFNTY is returned. M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To measure its collinearity.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Collinearity relative measure.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEstimateCrvCollinearity, conversion, collinearity                    M
*****************************************************************************/
CagdRType CagdEstimateCrvCollinearity(const CagdCrvStruct *Crv)
{
    int i,
	MaxDim = 3,
	Length = Crv -> Length,
	Length1 = Length - 1;
    CagdRType LineLen,
	MaxDist = 0.0;
    CagdRType
	* const *Points = Crv -> Points;
    CagdCrvStruct
	*CoercedCrv = NULL;
    CagdPointType
	PType = Crv ->PType;
    CagdVecStruct LineDir;

    switch (PType) {	      /* Convert the rational cases to non rational. */
	case CAGD_PT_P2_TYPE:
	    CoercedCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
	    Points = CoercedCrv -> Points;
	    PType = CoercedCrv -> PType;
	    break;
	case CAGD_PT_P3_TYPE:
	    CoercedCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);
	    Points = CoercedCrv -> Points;
	    PType = CoercedCrv -> PType;
	    break;
	default:
	    break;
    }

    switch (PType) {
	case CAGD_PT_E2_TYPE:
	    LineDir.Vec[0] = Points[1][Length1] - Points[1][0];
	    LineDir.Vec[1] = Points[2][Length1] - Points[2][0];
	    LineDir.Vec[2] = 0.0;
	    MaxDim = 2;
	    break;
	case CAGD_PT_E3_TYPE:
	    LineDir.Vec[0] = Points[1][Length1] - Points[1][0];
	    LineDir.Vec[1] = Points[2][Length1] - Points[2][0];
	    LineDir.Vec[2] = Points[3][Length1] - Points[3][0];
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
	    break;
    }

    LineLen = CAGD_LEN_VECTOR(LineDir);
    if (LineLen < IRIT_UEPS) {
	if (CoercedCrv != NULL)
	    CagdCrvFree(CoercedCrv);
	return IRIT_INFNTY;
    }

    CAGD_DIV_VECTOR(LineDir, LineLen);

    for (i = 1; i < Length1; i++) {
	CagdRType
	    Dist = CagdDistPtLine(&LineDir, Points, i, MaxDim);

	if (Dist > MaxDist)
	    MaxDist = Dist;
    }

    if (CoercedCrv != NULL)
	CagdCrvFree(CoercedCrv);

    return MaxDist / LineLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
 Tests mesh collinearity by testing the distance of interior points from the M
* plane thru 3 corner points.						     M
*   Returns a relative ratio of deviation from plane relative to its size.   M
*   Zero means all points are coplanar.					     M
*   If end points are same ( no plane can be fit ) IRIT_INFNTY is returned.  M
*                                                                            M
* PARAMETERS:                                                                M
*   Srf:        To measure its coplanarity.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Coplanarity measure.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEstimateSrfPlanarity, conversion, coplanarity                        M
*****************************************************************************/
CagdRType CagdEstimateSrfPlanarity(const CagdSrfStruct *Srf)
{
    int i,
	ULength = Srf -> ULength,
	ULength1 = ULength - 1,
	VLength = Srf -> VLength,
	VLength1 = VLength - 1;
    CagdRType
	PlaneSize = 0.0,
	MaxDist = 0.0;
    CagdRType
	* const *Points = Srf -> Points;
    CagdSrfStruct
	*CoercedSrf = NULL;
    CagdPointType
	PType = Srf ->PType;
    CagdPlaneStruct Plane;

    switch (PType) {	      /* Convert the rational cases to non rational. */
	case CAGD_PT_P2_TYPE:
	case CAGD_PT_E2_TYPE:
	    /* Trivial case - it is planar surface. */
	    return 0.0;
	case CAGD_PT_P3_TYPE:
	    CoercedSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
	    Points = CoercedSrf -> Points;
	    PType = CoercedSrf -> PType;
	    break;
	default:
	    break;
    }

    switch (PType) {
	case CAGD_PT_E3_TYPE:
	    PlaneSize = CagdFitPlaneThruCtlPts(&Plane, PType, Points,
					CAGD_MESH_UV(Srf, 0,        0),
					CAGD_MESH_UV(Srf, 0,        VLength1),
					CAGD_MESH_UV(Srf, ULength1, 0),
					CAGD_MESH_UV(Srf, ULength1, VLength1));
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
	    break;
    }

    if (PlaneSize < IRIT_UEPS) {
	if (CoercedSrf != NULL)
	    CagdSrfFree(CoercedSrf);
	return IRIT_INFNTY;
    }

    for (i = ULength * VLength; i > 0; i--) {
	CagdRType
	    Dist = CagdDistPtPlane(&Plane, Points, i, 3);

	if (Dist > MaxDist)
	    MaxDist = Dist;
    }

    if (CoercedSrf != NULL)
	CagdSrfFree(CoercedSrf);

    return MaxDist / PlaneSize;
}
