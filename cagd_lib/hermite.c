/******************************************************************************
* Hermite.c - Hermite curve and surface constructors.            	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Apr. 95.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a cubic Bezier curve using the Hermite constraints - two       M
* positions and two tangents.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2:   Starting and end points of curve.                            M
*   Dir1, Dir2: Starting and end vectors of curve.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A cubic Bezier curve, satisfying the four constrants. M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCubicHermiteCrv, Hermite                                             M
*****************************************************************************/
CagdCrvStruct *CagdCubicHermiteCrv(const CagdPType Pt1,
				   const CagdPType Pt2,
				   const CagdVType Dir1,
				   const CagdVType Dir2)
{
    int i;
    CagdCrvStruct
	*Crv = BzrCrvNew(4, CAGD_PT_E3_TYPE);
    CagdRType
	**Points = Crv -> Points;

    for (i = 0; i < 3; i++) {
	Points[i + 1][0] = Pt1[i];
	Points[i + 1][1] = Pt1[i] + Dir1[i] / 3.0;
	Points[i + 1][2] = Pt2[i] - Dir2[i] / 3.0;
	Points[i + 1][3] = Pt2[i];
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a cubic surface using the Hermite constraints - two            M
* positions and two tangents.  Other direction's degree depends on input.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CPos1Crv, CPos2Crv:  Starting and end curves of surface.                 M
*   CDir1Crv, CDir2Crv:  Starting and end tangent fields surface.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A cubic by something Bezier surface, satisfying the   M
*		       four constrants. The other something degree is the    M
*		       largest of the four given curves.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCubicHermiteSrf, Hermite                                             M
*****************************************************************************/
CagdSrfStruct *CagdCubicHermiteSrf(const CagdCrvStruct *CPos1Crv,
				   const CagdCrvStruct *CPos2Crv,
				   const CagdCrvStruct *CDir1Crv,
				   const CagdCrvStruct *CDir2Crv)
{
    int i, j, MaxAxis;
    CagdSrfStruct *Srf;
    CagdRType **Points;
    CagdCrvStruct *Pos1Crv, *Pos2Crv, *Dir1Crv, *Dir2Crv;

    Pos1Crv = CagdCrvCopy(CPos1Crv);
    Pos2Crv = CagdCrvCopy(CPos2Crv);
    Dir1Crv = CagdCrvCopy(CDir1Crv);
    Dir2Crv = CagdCrvCopy(CDir2Crv);

    if (!CagdMakeCrvsCompatible(&Pos1Crv, &Pos2Crv, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&Pos1Crv, &Dir1Crv, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&Pos1Crv, &Dir2Crv, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&Pos2Crv, &Dir1Crv, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&Pos2Crv, &Dir2Crv, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&Dir1Crv, &Dir2Crv, TRUE, TRUE)) {
	CAGD_FATAL_ERROR(CAGD_ERR_CRV_FAIL_CMPT);
	CagdCrvFree(Pos1Crv);
	CagdCrvFree(Pos2Crv);
	CagdCrvFree(Dir1Crv);
	CagdCrvFree(Dir2Crv);
	return NULL;
    }

    if (CAGD_IS_BEZIER_CRV(Pos1Crv))
	Srf = BzrSrfNew(4, Pos1Crv -> Order, Pos1Crv -> PType);
    else {
	Srf = BspSrfNew(4, Pos1Crv -> Length,
			4, Pos1Crv -> Order, Pos1Crv -> PType);
	BspKnotUniformOpen(4, 4, Srf -> UKnotVector);
	CAGD_GEN_COPY(Srf -> VKnotVector, Pos1Crv -> KnotVector,
		      sizeof(CagdRType) *
		          (Pos1Crv -> Length + Pos1Crv -> Order +
			   (Pos1Crv -> Periodic ? Pos1Crv -> Order - 1 : 0)));
    }
    Points = Srf -> Points;

    MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);

    for (j = 0; j < Pos1Crv -> Length; j++) {
	int Offset = 4 * j;

	for (i = !CAGD_IS_RATIONAL_SRF(Srf); i <= MaxAxis; i++) {
	    Points[i][Offset]     = Pos1Crv -> Points[i][j];
	    Points[i][Offset + 1] = Pos1Crv -> Points[i][j] +
					Dir1Crv -> Points[i][j] / 3.0;
	    Points[i][Offset + 2] = Pos2Crv -> Points[i][j] -
					Dir2Crv -> Points[i][j] / 3.0;
	    Points[i][Offset + 3] = Pos2Crv -> Points[i][j];
	}
    }

    CagdCrvFree(Pos1Crv);
    CagdCrvFree(Pos2Crv);
    CagdCrvFree(Dir1Crv);
    CagdCrvFree(Dir2Crv);

    return Srf;
}
