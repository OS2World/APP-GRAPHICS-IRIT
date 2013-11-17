/******************************************************************************
* CagdCoer.c - Handle point coercions/conversions.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerce Srf/Crv Point from index Index of Points array of Type PType to a   M
* point type E2.                                                             M
*   If however Index < 0 Points is considered single point.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   E2Point:   Where the coerced information is to besaved.                  M
*   Points:    Array of vectors if Index >= 0, a single point if Index < 0.  M
*   Index:     Index into the vectors of Points.                             M
*   PType:     Point type to be expected from Points.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceToE2, coercion                                                 M
*****************************************************************************/
void CagdCoerceToE2(CagdRType *E2Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType)
{
    int i,
        MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType Weight;
    CagdRType const *Point;

    if (MaxCoord > 2)
	MaxCoord = 2;

    if (Index < 0) {			      /* Points is one single point. */
        Point = *Points;
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    Weight = Point[0] == 0.0 ? IRIT_LARGE : 1.0 / Point[0];

	    for (i = 1; i <= MaxCoord; i++)
	        *E2Point++ = Point[i] * Weight;
	}
	else {
	    for (i = 1; i <= MaxCoord; i++)
	        *E2Point++ = Point[i];
	}
    }
    else {                       /* Points is a full arrays from Srf or Crv. */
        if (CAGD_IS_RATIONAL_PT(PType)) {
	    Weight = Points[0][Index] == 0.0 ? IRIT_LARGE 
					     : 1.0 / Points[0][Index];
	    for (i = 1; i <= MaxCoord; i++)
	        *E2Point++ = Points[i][Index] * Weight;
	}
	else {
	    for (i = 1; i <= MaxCoord; i++)
	        *E2Point++ = Points[i][Index];
	}
    }

    for (i = MaxCoord; i < 2; i++)
	*E2Point++ = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerce Srf/Crv Point from index Index of Points array of Type PType to a   M
* point type E3.                                                             M
*   If however Index < 0 Points is considered single point.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   E3Point:   Where the coerced information is to besaved.                  M
*   Points:    Array of vectors if Index >= 0, a single point if Index < 0.  M
*   Index:     Index into the vectors of Points.                             M
*   PType:     Point type to be expected from Points.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceToE3, coercion                                                 M
*****************************************************************************/
void CagdCoerceToE3(CagdRType *E3Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType)
{
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType Weight;
    CagdRType const *Point;

    if (MaxCoord > 3)
	MaxCoord = 3;

    if (Index < 0) {			      /* Points is one single point. */
        Point = *Points;
	if (CAGD_IS_RATIONAL_PT(PType)) {
	    Weight = Point[0] == 0.0 ? IRIT_LARGE : 1.0 / Point[0];

	    for (i = 1; i <= MaxCoord; i++)
	        *E3Point++ = Point[i] * Weight;
	}
	else {
	    for (i = 1; i <= MaxCoord; i++)
	        *E3Point++ = Point[i];
	}
    }
    else {                       /* Points is a full arrays from Srf or Crv. */
        if (CAGD_IS_RATIONAL_PT(PType)) {
	    Weight = Points[0][Index] == 0.0 ? IRIT_LARGE 
					     : 1.0 / Points[0][Index];
	    for (i = 1; i <= MaxCoord; i++)
	        *E3Point++ = Points[i][Index] * Weight;
	}
	else {
	    for (i = 1; i <= MaxCoord; i++)
	        *E3Point++ = Points[i][Index];
	}
    }

    for (i = MaxCoord; i < 3; i++)
	*E3Point++ = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerce Srf/Crv Point from index Index of Points array of Type PType to a   M
* point type P2.                                                             M
*   If however Index < 0 Points is considered single point.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   P2Point:   Where the coerced information is to besaved.                  M
*   Points:    Array of vectors if Index >= 0, a single point if Index < 0.  M
*   Index:     Index into the vectors of Points.                             M
*   PType:     Point type to be expected from Points.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceToP2, coercion                                                 M
*****************************************************************************/
void CagdCoerceToP2(CagdRType *P2Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i,
        MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType const *Point;

    if (MaxCoord > 2)
	MaxCoord = 2;

    if (Index < 0) {			      /* Points is one single point. */
        Point = *Points;
	*P2Point++ = IsRational ? Point[0] : 1.0;
	for (i = 1; i <= MaxCoord; i++)
	    *P2Point++ = Point[i];
    }
    else {                       /* Points is a full arrays from Srf or Crv. */
	*P2Point++ = IsRational ? Points[0][Index] : 1.0;
	for (i = 1; i <= MaxCoord; i++)
	    *P2Point++ = Points[i][Index];
    }

    for (i = MaxCoord; i < 2; i++)
	*P2Point++ = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerce Srf/Crv Point from index Index of Points array of Type PType to a   M
* point type P3.                                                             M
*   If however Index < 0 Points is considered single point.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   P3Point:   Where the coerced information is to besaved.                  M
*   Points:    Array of vectors if Index >= 0, a single point if Index < 0.  M
*   Index:     Index into the vectors of Points.                             M
*   PType:     Point type to be expected from Points.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceToP3, coercion                                                 M
*****************************************************************************/
void CagdCoerceToP3(CagdRType *P3Point,
		    CagdRType * const Points[CAGD_MAX_PT_SIZE],
		    int Index,
		    CagdPointType PType)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType const *Point;

    if (MaxCoord > 3)
	MaxCoord = 3;

    if (Index < 0) {			      /* Points is one single point. */
	Point = *Points;
	*P3Point++ = IsRational ? Point[0] : 1.0;
	for (i = 1; i <= MaxCoord; i++)
	    *P3Point++ = Point[i];
    }
    else {                       /* Points is a full arrays from Srf or Crv. */
	*P3Point++ = IsRational ? Points[0][Index] : 1.0;
	for (i = 1; i <= MaxCoord; i++)
	    *P3Point++ = Points[i][Index];
    }

    for (i = MaxCoord; i < 3; i++)
	*P3Point++ = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces Srf/Crv Point from index Index of Points array of Type PType to a  M
* new type NewPType. If however Index < 0 Points is considered single point. M
*                                                                            *
* PARAMETERS:                                                                M
*   NewPoint:  Where the coerced information is to besaved.                  M
*   NewPType:  Point type of the coerced new point.                          M
*   Points:    Array of vectors if Index >= 0, a single point if Index < 0.  M
*   Index:     Index into the vectors of Points.                             M
*   OldPType:  Point type to be expected from Points.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoercePointTo, coercion                                              M
*****************************************************************************/
void CagdCoercePointTo(CagdRType *NewPoint,
		       CagdPointType NewPType,
		       CagdRType * const Points[CAGD_MAX_PT_SIZE],
		       int Index,
		       CagdPointType OldPType)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(OldPType);
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(OldPType),
	NewMaxCoord = CAGD_NUM_OF_PT_COORD(NewPType);
    CagdRType Weight;
    CagdRType const *Point;

    if (MaxCoord > NewMaxCoord)
	MaxCoord = NewMaxCoord;

    if (Index < 0) {			      /* Points is one single point. */
        Point = *Points;
	Weight = IsRational ? Point[0] : 1.0;
	if (CAGD_IS_RATIONAL_PT(NewPType)) {
	    *NewPoint++ = Weight;
	    Weight = 1.0;
	}

	Weight = 1.0 / Weight;
	for (i = 1; i <= MaxCoord; i++)
	    *NewPoint++ = Point[i] * Weight;
    }
    else {                       /* Points is a full arrays from Srf or Crv. */
	Weight = IsRational ? Points[0][Index] : 1.0;
	if (CAGD_IS_RATIONAL_PT(NewPType)) {
	    *NewPoint++ = Weight;
	    Weight = 1.0;
	}
	Weight = Weight == 0.0 ? IRIT_LARGE : 1.0 / Weight;
	for (i = 1; i <= MaxCoord; i++)
	    *NewPoint++ = Points[i][Index] * Weight;
    }

    for (i = MaxCoord; i < NewMaxCoord; i++)
	*NewPoint++ = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two control meshs for similarity.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Mesh1, Mesh2:   Two control meshs to compare.                            M
*   Len:            Length of control meshs.                                 M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if control meshs are the same, FALSE otehrwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotVectorsSame, CagdCrvsSame, CagdSrfsSame                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlMeshsSame                                                         M
*****************************************************************************/
CagdBType CagdCtlMeshsSame(CagdRType * const Mesh1[],
			   CagdRType * const Mesh2[],
			   int Len,
			   CagdRType Eps)
{
    int i;

    for (i = 0; i <= CAGD_MAX_PT_COORD; i++) {
	int j;
	CagdRType const
	    *m1 = Mesh1[i],
	    *m2 = Mesh2[i];

	if ((m1 == NULL || m2 == NULL) && m1 != m2)
	    return FALSE;

	if (m1 == NULL && m2 == NULL)
	    continue;

	/* Check for similarity - relative error test. */
	for (j = 0; j < Len; j++)
	    if (!IRIT_APX_EQ_EPS(m1[j], m2[j],
				 IRIT_MAX(1.0, IRIT_FABS(m1[j])) * Eps))
		return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two control meshs for similarity up to rigid motion and scale. M
*   Comparison is conducted in the XY plane and only X and Y (and W) are     M
* considered.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mesh1, Mesh2:   Two control meshs to compare.                            M
*   Len:            Length of control meshs.                                 M
*   Trans:          Translation amount to apply second to Mesh1 to bring to  M
*		    Mesh2.						     M
*   Rot, Scl:       Rotation and scale amounts to apply first to Mesh1 to    M
*		    bring to Mesh2. Rot is specified in degrees.	     M
*   Eps:            Tolerance of equality.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if control meshs are the same, FALSE otehrwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotVectorsSame, CagdCrvsSame, CagdSrfsSame                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlMeshsSameUptoRigidScl2D                                           M
*****************************************************************************/
CagdBType CagdCtlMeshsSameUptoRigidScl2D(CagdRType * const Mesh1[],
					 CagdRType * const Mesh2[],
					 int Len,
					 IrtPtType Trans,
					 CagdRType *Rot,
					 CagdRType *Scl,
					 CagdRType Eps)
{
    int i, MaxIndex;
    CagdRType MaxDistSqr, R1, R2, W0, WMI;
    CagdRType const 
	*M1w = Mesh1[0],
	*M1x = Mesh1[1],
	*M1y = Mesh1[2],
	*M2w = Mesh2[0],
        *M2x = Mesh2[1],
	*M2y = Mesh2[2];
    IrtHmgnMatType Mat1, Mat2;

    /* If we get weights in the meshes - check them first for similarity. */
    if (M1w != NULL || M2w != NULL) {
        if (M1w != NULL && M2w != NULL) {
	    /* Weights are tested for identical similarity. */
	    for (i = 0; i < Len; i++)
	        if (!IRIT_APX_EQ_EPS(M1w[i], M2w[i], Eps))
		    return FALSE;
	}
	else
	    return FALSE;
    }
    W0 = 1.0 / (M1w ? M1w[0] : 1.0);

    /* Find the farthest point in Mesh1 with respect to Mesh2. */
    MaxDistSqr = 0.0;
    MaxIndex = 0;
    for (i = 1; i < Len; i++) {
        CagdRType d,
	    WI = 1.0 / (M1w ? M1w[i] : 1.0);
	
	if ((d = IRIT_SQR(M1x[i] * WI - M1x[0] * W0) +
	         IRIT_SQR(M1y[i] * WI - M1y[0] * W0)) > MaxDistSqr) {
	    MaxDistSqr = d;
	    MaxIndex = i;
	}
    }
    if (MaxDistSqr < IRIT_UEPS)
        return FALSE;
    WMI = 1.0 / (M1w ? M1w[MaxIndex] : 1.0);

    /* Compute the scale and rotate factors based on this farthest point. */
    *Scl = sqrt(IRIT_SQR(M2x[MaxIndex] * WMI - M2x[0] * W0) + 
		IRIT_SQR(M2y[MaxIndex] * WMI - M2y[0] * W0)) /
           sqrt(MaxDistSqr);
    if (*Scl < IRIT_UEPS)
        return FALSE;

    R1 = atan2(M1y[MaxIndex] * WMI - M1y[0] * W0,
	       M1x[MaxIndex] * WMI - M1x[0] * W0);
    R2 = atan2(M2y[MaxIndex] * WMI - M2y[0] * W0,
	       M2x[MaxIndex] * WMI - M2x[0] * W0);
    if ((*Rot = R2 - R1) < 0.0)
	*Rot += M_PI_MUL_2;

    /* Build the matrix that does the transformation from Mesh1 to Mesh2.  */
    /* First scale & rotate.					           */
    MatGenMatRotZ1(*Rot, Mat1);
    MatGenMatUnifScale(*Scl, Mat2);
    MatMultTwo4by4(Mat2, Mat2, Mat1);

    /* Test all the control points for rigid motion/scale similarity: */
    for (i = 0; i < Len; i++) {
        IrtPtType Pt;
	CagdRType
	    WI = 1.0 / (M1w ? M1w[i] : 1.0);

	Pt[0] = M1x[i] * WI;
	Pt[1] = M1y[i] * WI;
	Pt[2] = 0.0;

	MatMultPtby4by4(Pt, Pt, Mat2);
	if (i == 0) {
	    /* Add the translation factor to the matrix. */
	    Trans[0] = M2x[i] * WI - Pt[0];
	    Trans[1] = M2y[i] * WI - Pt[1];
	    Trans[2] = 0.0;

	    MatGenMatTrans(Trans[0], Trans[1], Trans[2], Mat1);
	    MatMultTwo4by4(Mat2, Mat2, Mat1);
	}
	else if (!IRIT_APX_EQ_EPS(Pt[0], M2x[i] * WI, Eps) ||
		 !IRIT_APX_EQ_EPS(Pt[1], M2y[i] * WI, Eps))
	    return FALSE;
    }

    *Rot = IRIT_RAD2DEG(*Rot);			    /* Convert to degrees. */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces an array of vectors of points of point type OldPType to point type M
* NewPType, in place.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:    Where the old and new points are placed.                      M
*   Len:       Length of vectors in the array of vectors, Points.            M
*   OldPType:  Point type to be expected from Points.                        M
*   NewPType:  Point type of the coerced new point.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdCoercePointsTo, coercion                                             M
*****************************************************************************/
void CagdCoercePointsTo(CagdRType *Points[],
			int Len,
			CagdPointType OldPType,
			CagdPointType NewPType)
{
    int i, j,
	OldIsRational = CAGD_IS_RATIONAL_PT(OldPType),
	OldNumOfCoords = CAGD_NUM_OF_PT_COORD(OldPType),
	NewIsRational = CAGD_IS_RATIONAL_PT(NewPType),
	NewNumOfCoords = CAGD_NUM_OF_PT_COORD(NewPType);
    CagdRType *NewPoints[CAGD_MAX_PT_SIZE];

    for (i = !NewIsRational; i <= NewNumOfCoords; i++)
	NewPoints[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);

    if (OldIsRational) {
        if (NewIsRational) {
	    for (i = 0; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++)
	        CAGD_GEN_COPY(NewPoints[i], Points[i],
			      sizeof(CagdRType) * Len);
	}
	else {
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++) {
	        CagdRType
		    *WPts = Points[0],
		    *OPts = Points[i],
		    *NPts = NewPoints[i];

		for (j = 0; j < Len; j++, WPts++)
		    *NPts++ = *OPts++ / (*WPts == 0.0 ? IRIT_UEPS : *WPts);
	    }
	}
    }
    else { /* Old is not rational. */
        if (NewIsRational) {
	    CagdRType
	        *WPts = NewPoints[0];
		    
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++)
	        CAGD_GEN_COPY(NewPoints[i], Points[i],
			      sizeof(CagdRType) * Len);
	    for (j = 0; j < Len; j++)
	        *WPts++ = 1.0;
	}
	else {
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++) {
	        CAGD_GEN_COPY(NewPoints[i], Points[i],
			      sizeof(CagdRType) * Len);
	    }
	}
    }
    for ( ; i <= NewNumOfCoords; i++)
        IRIT_ZAP_MEM(NewPoints[i], sizeof(CagdRType) * Len);

    /* Replace old rep. with new. */
    for (i = !OldIsRational; i <= OldNumOfCoords; i++)
	IritFree(Points[i]);

    IRIT_ZAP_MEM(Points, sizeof(CagdRType *) * CAGD_MAX_PT_SIZE);
    for (i = !NewIsRational; i <= NewNumOfCoords; i++)
	Points[i] = NewPoints[i];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces an array of vectors of points of point type OldPType to point type M
* NewPType, while duplicating the parent's structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   OldPoints:     Where the old points in OldStruct are placed.             M
*   OldStruct:     A pointer to the original structure hold Points.          M
*   OldStructLen:  Sizeof OldStruct structure.				     M
*   ExtraMem:      Do we seek to allocate extra memory at the end?           M
*   PtsLen:        Length of vectors in the array of vectors, Points.        M
*   OldPType:      Point type to be expected from Points.                    M
*   NewPType:      Point type of the coerced new point.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    A duplicated parent structure with new point types.          M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdStructOnceCoercePointsTo, coercion                                   M
*****************************************************************************/
VoidPtr CagdStructOnceCoercePointsTo(CagdRType * const *OldPoints,
				     const VoidPtr OldStruct,
				     int OldStructLen,
				     int ExtraMem,
				     int PtsLen,
				     CagdPointType OldPType,
				     CagdPointType NewPType)
{
    int i, j,
	OldIsRational = CAGD_IS_RATIONAL_PT(OldPType),
	OldNumOfCoords = CAGD_NUM_OF_PT_COORD(OldPType),
	NewIsRational = CAGD_IS_RATIONAL_PT(NewPType),
	NewNumOfCoords = CAGD_NUM_OF_PT_COORD(NewPType);
    VoidPtr
	*NewStruct = IritMalloc(OldStructLen + ExtraMem +
				sizeof(CagdRType) * (PtsLen + 1) *
				    (NewIsRational + NewNumOfCoords));
    CagdRType
	**NewPoints = (CagdRType **) (((char *) NewStruct) +
			        (((char *) OldPoints) - ((char *) OldStruct))),
	*p = (CagdRType *) (((char *) NewStruct) + OldStructLen);

    /* Align it to 8 bytes. */
    p = (CagdRType *) ((((IritIntPtrSizeType) p) + 7) & ~0x07);

    IRIT_ZAP_MEM(NewPoints, sizeof(CagdRType *) * CAGD_MAX_PT_SIZE);

    for (i = !NewIsRational; i <= NewNumOfCoords; i++) {
        NewPoints[i] = p;
        p += PtsLen;
    }

    if (OldIsRational) {
        if (NewIsRational) {
	    for (i = 0; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++)
	        CAGD_GEN_COPY(NewPoints[i], OldPoints[i],
			      sizeof(CagdRType) * PtsLen);
	}
	else {
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++) {
	        CagdRType const
		    *WPts = OldPoints[0],
		    *OPts = OldPoints[i];
		CagdRType
		    *NPts = NewPoints[i];

		for (j = 0; j < PtsLen; j++, WPts++)
		    *NPts++ = *OPts++ / (*WPts == 0.0 ? IRIT_UEPS : *WPts);
	    }
	}
    }
    else { /* Old is not rational. */
        if (NewIsRational) {
	    CagdRType
	        *WPts = NewPoints[0];
		    
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++)
	        CAGD_GEN_COPY(NewPoints[i], OldPoints[i],
			      sizeof(CagdRType) * PtsLen);

	    for (j = 0; j < PtsLen; j++)
	        *WPts++ = 1.0;
	}
	else {
	    for (i = 1; i <= IRIT_MIN(OldNumOfCoords, NewNumOfCoords); i++) {
	        CAGD_GEN_COPY(NewPoints[i], OldPoints[i],
			      sizeof(CagdRType) * PtsLen);
	    }
	}
    }

    for ( ; i <= NewNumOfCoords; i++)
        IRIT_ZAP_MEM(NewPoints[i], sizeof(CagdRType) * PtsLen);

    return NewStruct;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of curves to a new point type PType. If given curves are    M
* E1 or P1 and requested new type is E2 or P2 the Y coefficients are updated M
* to hold the parametric domain of the curve, if AddParametrization.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be coerced to a new point type PType.                      M
*   PType:     New point type for Crv.                                       M
*   AddParametrization:  If TRUE, the input is a scalar curve and the        M
*	       requested output is 2D, add a parametrization to newly	     M
*	       added axis.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The new, coerced to PType, curves.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceCrvsTo, coercion                                               M
*****************************************************************************/
CagdCrvStruct *CagdCoerceCrvsTo(const CagdCrvStruct *Crv,
				CagdPointType PType,
				CagdBType AddParametrization)
{
    CagdCrvStruct *CoercedCrv,
        *CoercedCrvs = NULL;

    for ( ; Crv != NULL; Crv = Crv -> Pnext) {
        CoercedCrv = CagdCoerceCrvTo(Crv, PType, AddParametrization);
	IRIT_LIST_PUSH(CoercedCrv, CoercedCrvs);
    }

    return CagdListReverse(CoercedCrvs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a curve to a new point type PType. If given curve is E1 or P1      M
* and requested new type is E2 or P2 the Y coefficients are updated to       M
* hold the parametric domain of the curve, if AddParametrization.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be coerced to a new point type PType.                      M
*   PType:     New point type for Crv.                                       M
*   AddParametrization:  If TRUE, the input is a scalar curve and the        M
*	       requested output is 2D, add a parametrization to newly	     M
*	       added axis.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The new, coerced to PType, curve.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceCrvTo, coercion                                                M
*****************************************************************************/
CagdCrvStruct *CagdCoerceCrvTo(const CagdCrvStruct *Crv,
			       CagdPointType PType,
			       CagdBType AddParametrization)
{
    CagdCrvStruct *TCrv;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    TCrv = CagdStructOnceCoercePointsTo(Crv -> Points, Crv,
					sizeof(CagdCrvStruct), 0,
					Crv -> Length, Crv -> PType, PType);
    TCrv -> GType = Crv -> GType;
    TCrv -> PType = Crv -> PType;
    TCrv -> Length = Crv -> Length;
    TCrv -> Order = Crv -> Order;
    TCrv -> Periodic = Crv -> Periodic;
    if (Crv -> KnotVector != NULL)
	TCrv -> KnotVector = BspKnotCopy(NULL, Crv -> KnotVector,
				   Crv -> Length + Crv -> Order +
				   (Crv -> Periodic ? Crv -> Order - 1 : 0));
    else
	TCrv -> KnotVector = NULL;
    TCrv -> Pnext = NULL;
    TCrv -> Attr = NULL;
    CAGD_PROPAGATE_ATTR(TCrv, Crv);
#else
    TCrv = CagdCrvCopy(Crv);
    CagdCoercePointsTo(TCrv -> Points, TCrv -> Length, TCrv -> PType, PType);
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    if (AddParametrization &&
	CAGD_NUM_OF_PT_COORD(TCrv -> PType) == 1 &&
	CAGD_NUM_OF_PT_COORD(PType) == 2) {
	/* Update the parameter space to be the second axis. */
	CagdRType
	    *WPts = TCrv -> Points[0],
	    *Pts = TCrv -> Points[2],
	    *Nodes = CagdCrvNodes(TCrv);

	CAGD_GEN_COPY(Pts, Nodes, sizeof(CagdRType) * TCrv -> Length);
	if (WPts != NULL) {
	    int i;

	    for (i = 0; i < TCrv -> Length; i++)
	        *Pts++ *= *WPts++;
	}

	IritFree(Nodes);
    }

    TCrv -> PType = PType;

    return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of surfaces to a new point type PType. If given surfaces    M
* are E1 or P1 and requested new type is E2 or P2 the Y coefficients are     M
* updated to hold the parametric domain of surface, if AddParametrization.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be coerced to a new point type PType.                      M
*   PType:     New point type for Srf.                                       M
*   AddParametrization:  If TRUE, the input is a scalar surface and the      M
*	       requested output is 2D, add a parametrization to newly	     M
*	       added axis.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The new, coerced to PType, surfaces.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceSrfsTo, coercion                                               M
*****************************************************************************/
CagdSrfStruct *CagdCoerceSrfsTo(const CagdSrfStruct *Srf,
				CagdPointType PType,
				CagdBType AddParametrization)
{
    CagdSrfStruct *CoercedSrf,
        *CoercedSrfs = NULL;

    for ( ; Srf != NULL; Srf = Srf -> Pnext) {
        CoercedSrf = CagdCoerceSrfTo(Srf, PType, AddParametrization);
	IRIT_LIST_PUSH(CoercedSrf, CoercedSrfs);
    }

    return CagdListReverse(CoercedSrfs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a surface to a new point type PType. If given surface is E1 or P1  M
* and requested new type is E3 or P3 the Y and Z coefficients are updated to M
* hold the parametric domain of the surface, if AddParametrization.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be coerced to a new point type PType.                      M
*   PType:     New point type for Srf.                                       M
*   AddParametrization:  If TRUE, the input is a scalar surface and the      M
*	       requested output is 3D, add a parametrization to newly	     M
*	       added axis.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The new, coerced to PType, surface.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCoerceSrfTo, coercion                                                M
*****************************************************************************/
CagdSrfStruct *CagdCoerceSrfTo(const CagdSrfStruct *Srf,
			       CagdPointType PType,
			       CagdBType AddParametrization)
{
    CagdSrfStruct *TSrf;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    TSrf = CagdStructOnceCoercePointsTo(Srf -> Points, Srf,
					sizeof(CagdSrfStruct), 0,
					Srf -> ULength * Srf -> VLength,
					Srf -> PType, PType);

    TSrf -> GType = Srf -> GType;
    TSrf -> PType = Srf -> PType;
    TSrf -> ULength = Srf -> ULength;
    TSrf -> VLength = Srf -> VLength;
    TSrf -> UOrder = Srf -> UOrder;
    TSrf -> VOrder = Srf -> VOrder;
    TSrf -> UPeriodic = Srf -> UPeriodic;
    TSrf -> VPeriodic = Srf -> VPeriodic;
    TSrf -> PAux = NULL;

    if (Srf -> UKnotVector != NULL)
	TSrf -> UKnotVector = BspKnotCopy(NULL, Srf -> UKnotVector,
				   Srf -> ULength + Srf -> UOrder +
				   (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));
    else
	TSrf -> UKnotVector = NULL;
    if (Srf -> VKnotVector != NULL)
	TSrf -> VKnotVector = BspKnotCopy(NULL, Srf -> VKnotVector,
				   Srf -> VLength + Srf -> VOrder +
				   (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));
    else
	TSrf -> VKnotVector = NULL;
    TSrf -> Pnext = NULL;
    TSrf -> Attr = NULL;

    CAGD_PROPAGATE_ATTR(TSrf, Srf);
#else
    TSrf = CagdSrfCopy(Srf);
    CagdCoercePointsTo(TSrf -> Points, TSrf -> ULength * TSrf -> VLength,
		       TSrf -> PType, PType);
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    if (AddParametrization &&
	CAGD_NUM_OF_PT_COORD(Srf -> PType) == 1 &&
	CAGD_NUM_OF_PT_COORD(PType) == 3) {
	/* Update the parameter space to be the second and third axis. */
	int i;
	CagdRType *PtsY, *PtsZ,
	    *WPts = TSrf -> Points[0],
	    *UNodes = CagdSrfNodes(TSrf, CAGD_CONST_U_DIR),
	    *VNodes = CagdSrfNodes(TSrf, CAGD_CONST_V_DIR);

	for (i = 0, PtsY = TSrf -> Points[2];
	     i < TSrf -> VLength;
	     i++, PtsY += TSrf -> ULength)
	    CAGD_GEN_COPY(PtsY, UNodes, sizeof(CagdRType) * TSrf -> ULength);

	for (i = 0, PtsZ = TSrf -> Points[3]; i < TSrf -> VLength; i++) {
	    int j;

	    for (j = 0; j < TSrf -> ULength; j++)
	        *PtsZ++ = VNodes[i];
	}

	if (WPts != NULL) {
	    PtsY = TSrf -> Points[2];
	    PtsZ = TSrf -> Points[3];
	    for (i = 0; i < TSrf -> ULength * TSrf -> VLength; i++) {
	        *PtsY++ *= *WPts;
	        *PtsZ++ *= *WPts++;

	    }
	}

	IritFree(UNodes);
	IritFree(VNodes);
    }

    TSrf -> PType = PType;

    return TSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a point type which spans the spaces of both two given point types. M
*                                                                            *
* PARAMETERS:                                                                M
*   PType1, PType2: To point types to find the point type of their union.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPointType:  A point type of the union of the spaces of PType1 and    M
*                   PType2.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeIrtPtType, coercion                                             M
*****************************************************************************/
CagdPointType CagdMergeIrtPtType(CagdPointType PType1, CagdPointType PType2)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType1) || CAGD_IS_RATIONAL_PT(PType2);
    int NumCoords = IRIT_MAX(CAGD_NUM_OF_PT_COORD(PType1),
		        CAGD_NUM_OF_PT_COORD(PType2));

    return CAGD_MAKE_PT_TYPE(IsRational, NumCoords);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the L2 distance between two arbitrary control points.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Index1, Pt2, Index2:      Two Control points to compute distance    M
*		      between, and incides into the vectors of Points, Pt1   M
8		      and Pt2.  If, however, Index? < 0, Pt? is a single     M
*		      point.						     M
*   PType:	      Type of points Pt?.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The distance between Pt1 and Pt2                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDistTwoCtlPt                                                         M
*****************************************************************************/
CagdRType CagdDistTwoCtlPt(CagdRType * const Pt1[CAGD_MAX_PT_SIZE],
			   int Index1,
			   CagdRType * const Pt2[CAGD_MAX_PT_SIZE],
			   int Index2,
			   CagdPointType PType)
{
    int i,
	NumCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType Pt1Coerced[CAGD_MAX_PT_SIZE], Pt2Coerced[CAGD_MAX_PT_SIZE],
	R = 0.0;
    CagdPointType
        NewPType = CAGD_MAKE_PT_TYPE(FALSE, NumCoord);

    CagdCoercePointTo(Pt1Coerced, NewPType, Pt1, Index1, PType);
    CagdCoercePointTo(Pt2Coerced, NewPType, Pt2, Index2, PType);
    for (i = 0; i < NumCoord; i++)
        R += IRIT_SQR(Pt1Coerced[i] - Pt2Coerced[i]);

    return sqrt(R);
}

