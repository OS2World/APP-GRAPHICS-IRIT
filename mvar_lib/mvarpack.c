/******************************************************************************
* MvarPack.c - Packing related problems.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 09.					      *
******************************************************************************/

#include "mvar_loc.h"
#include "geom_lib.h"

#define MVAR_PACK_MAX_ROT_ATTEMPTS	10
#define MVAR_PACK_PT_IN_LINE(Pt, Ln)  (Pt[0] * Ln[0] + Pt[1] * Ln[1] + Ln[2])

static MvarMVStruct *Mvar3CircsInTrianglesEqn(int Idx,
					      MvarMVStruct *Y[3],
					      const IrtLnType L,
					      const IrtLnType Lnext,
					      const IrtLnType LPrev,
					      CagdRType dBRet[3],
					      CagdRType dCRet[3]);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a triangles in the XY plane, specified by its 3 vertices Pts,      M
* Find the 3 circles packed inside the triangle (and tangent to it while     M
* also tangent to each other.						     M
*   This problem is also known as the (incorrect solution to the)            M
M "Malfatti Circles" problem.					             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:          3 vertices of triangle in the plane (only XY coordinates). M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvement stage. M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Points in R9 as (X1, Y1, R1, X2, Y2, R2, X3, Y3, R3).  M
*                       Each such solution is also tagged with "InTriangle"  M
*                     that is TRUE if all solution is inside the triangle.   M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   Mvar3CircsInTriangles                                                    M
*****************************************************************************/
MvarPtStruct *Mvar3CircsInTriangles(const CagdPType Pts[3],
				    CagdRType SubdivTol,
				    CagdRType NumericTol)
{
    int i, j, Inside, Lengths[3];
    CagdRType Min[2], Max[2], dB[3], dC[3], Rad,
        RotAngle = 0.0;
    CagdPType Cntr, RotPts[3];
    IrtHmgnMatType RotMat;
    IrtLnType L01, L12, L20;
    MvarMVStruct *MVs[3], *Y[3];
    MvarPtStruct *Sols, *Sol, *RetCirc,
        *RetCircs = NULL;
    MvarConstraintType Constraints[3];

    /* Try to build constraints and randomly rotate input until success. */
    MatGenUnitMat(RotMat);
    for (i = 0; i < MVAR_PACK_MAX_ROT_ATTEMPTS; i++) {
        /* Some configuration of the points of the triangle will fails as   */
        /* we express x-centers as function of y-center locations.  Hence   */
        /* we apply some random rotations - compute the rotated points.     */
        for (j = 0; j < 3; j++)
	    MatMultPtby4by4(RotPts[j], Pts[j], RotMat);

        /* Build the 3 lines of the triangles out of the given 3 points. */
        GMLineFrom2Points(L01, RotPts[1], RotPts[0]);
	GMLineFrom2Points(L12, RotPts[2], RotPts[1]);
	GMLineFrom2Points(L20, RotPts[0], RotPts[2]);

	IRIT_PT2D_ADD(Cntr, RotPts[0], RotPts[1]);
	IRIT_PT2D_ADD(Cntr, Cntr, RotPts[2]);
	IRIT_PT2D_SCALE(Cntr, 1.0 / 3.0);

	/* If the center is negative in the lines - flip all lines. */
	if (MVAR_PACK_PT_IN_LINE(Cntr, L01) < 0.0) {
	    IRIT_PT_SCALE(L01, -1.0);
	    IRIT_PT_SCALE(L12, -1.0);
	    IRIT_PT_SCALE(L20, -1.0);
	}
	if (MVAR_PACK_PT_IN_LINE(Cntr, L01) < IRIT_EPS * 10 ||
	    MVAR_PACK_PT_IN_LINE(Cntr, L12) < IRIT_EPS * 10 ||
	    MVAR_PACK_PT_IN_LINE(Cntr, L20) < IRIT_EPS * 10)
	    return NULL;

	/* Find the domain to search for XY locations and build the 3 DOFs. */
	Min[0] = IRIT_MIN(IRIT_MIN(RotPts[0][0], RotPts[1][0]), RotPts[2][0]);
	Min[1] = IRIT_MIN(IRIT_MIN(RotPts[0][1], RotPts[1][1]), RotPts[2][1]);
	Max[0] = IRIT_MAX(IRIT_MAX(RotPts[0][0], RotPts[1][0]), RotPts[2][0]);
	Max[1] = IRIT_MAX(IRIT_MAX(RotPts[0][1], RotPts[1][1]), RotPts[2][1]);

	Lengths[0] = 2;
	Lengths[1] = 1;
	Lengths[2] = 1;
	Y[0] = MvarBzrMVNew(3, Lengths, MVAR_PT_E1_TYPE);
	Y[0] -> Points[1][0] = Min[1];
	Y[0] -> Points[1][1] = Max[1];

	Lengths[0] = 1;
	Lengths[1] = 2;
	Lengths[2] = 1;
	Y[1] = MvarBzrMVNew(3, Lengths, MVAR_PT_E1_TYPE);
	Y[1] -> Points[1][0] = Min[1];
	Y[1] -> Points[1][1] = Max[1];

	Lengths[0] = 1;
	Lengths[1] = 1;
	Lengths[2] = 2;
	Y[2] = MvarBzrMVNew(3, Lengths, MVAR_PT_E1_TYPE);
	Y[2] -> Points[1][0] = Min[1];
	Y[2] -> Points[1][1] = Max[1];

        MVs[0] = Mvar3CircsInTrianglesEqn(0, Y, L01, L12, L20, dB, dC);
	MVs[1] = Mvar3CircsInTrianglesEqn(1, Y, L12, L20, L01, dB, dC);
	MVs[2] = Mvar3CircsInTrianglesEqn(2, Y, L20, L01, L12, dB, dC);

	if (MVs[0] != NULL && MVs[1] != NULL && MVs[2] != NULL)
	    break;        /* Done - we have valid constraints to solve for. */

	for (j = 0; j < 3; j++) {
	    if (Y[j] != NULL)
		MvarMVFree(Y[j]);
	    if (MVs[j] != NULL)
	        MvarMVFree(MVs[j]);
	}

	MatGenMatRotZ1(RotAngle = IritRandom(-100, 100), RotMat);
    }

    /* Solve the constraints. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
    if (MVs[0] != NULL && MVs[1] != NULL && MVs[2] != NULL)
        Sols = MvarMVsZeros(MVs, Constraints, 3, SubdivTol, NumericTol);
    else
        Sols = NULL;

    for (i = 0; i < 3; i++) {
        MvarMVFree(Y[i]);
	MvarMVFree(MVs[i]);
    }

    /* Map the solution to (X, Y, R) of the three circles. */
    MatGenMatRotZ1(-RotAngle, RotMat);
    for (Sol = Sols; Sol != NULL; Sol = Sol -> Pnext) {
        RetCirc = MvarPtNew(9);
	/* Update Y values. */
	RetCirc -> Pt[1] = Sol -> Pt[0] * (Max[1] - Min[1]) + Min[1];
	RetCirc -> Pt[4] = Sol -> Pt[1] * (Max[1] - Min[1]) + Min[1];
	RetCirc -> Pt[7] = Sol -> Pt[2] * (Max[1] - Min[1]) + Min[1];

	/* Update X values. */
	RetCirc -> Pt[0] = dB[0] * RetCirc -> Pt[1] + dC[0];
	RetCirc -> Pt[3] = dB[1] * RetCirc -> Pt[4] + dC[1];
	RetCirc -> Pt[6] = dB[2] * RetCirc -> Pt[7] + dC[2];

	/* Update R values. */
	RetCirc -> Pt[2] = L01[0] * RetCirc -> Pt[0] + 
			   L01[1] * RetCirc -> Pt[1] + 
			   L01[2];
	RetCirc -> Pt[5] = L12[0] * RetCirc -> Pt[3] + 
	                   L12[1] * RetCirc -> Pt[4] + 
			   L12[2];
	RetCirc -> Pt[8] = L20[0] * RetCirc -> Pt[6] + 
	                   L20[1] * RetCirc -> Pt[7] + 
			   L20[2];

	/* Tag the solution as inside the triangle or not. */
	Inside = TRUE;
	for (i = 0; i < 9; i += 3) {
	    Cntr[0] = RetCirc -> Pt[i];
	    Cntr[1] = RetCirc -> Pt[i + 1];
	    Cntr[2] = 0.0;
	    Rad = RetCirc -> Pt[i + 2] - fabs(NumericTol) * 10;
	    /* Verify that center is inside and then all the circle is in. */
	    if (GMBaryCentric3Pts2D(RotPts[0], RotPts[1], RotPts[2],
				    Cntr) != NULL &&
		MVAR_PACK_PT_IN_LINE(Cntr, L01) > Rad &&
		MVAR_PACK_PT_IN_LINE(Cntr, L12) > Rad &&
		MVAR_PACK_PT_IN_LINE(Cntr, L20) > Rad) {
	        /* Inside! */
	    }
	    else {
	        Inside = FALSE;
		break;
	    }
	}
	AttrSetIntAttrib(&RetCirc -> Attr, "InTriangle", Inside);

	/* Rotate back the solutions, Z (=Rad) is unaffected. */
	for (i = 0; i < 9; i += 3)
	    MatMultPtby4by4(&RetCirc -> Pt[i], &RetCirc -> Pt[i], RotMat);

	IRIT_LIST_PUSH(RetCirc, RetCircs);
    }

    MvarPtFreeList(Sols);

    return RetCircs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct one constraint for the circle near Pt (index Idx) in triangle. *
*   Let Ai x + Bi y + Ci = 0, Ai^2 + Bi^2 = 1 be the Idx'th line from Pt to  *
* PtNext.							             *
*   We have two linear constraints for circle center j, (Xj, Yj) to be at    *
* distance Rj from the two edges of the triangle it is tangent to as:	     *
*									     *
*      Ai Xj + Bi Yj + Ci             = Rj,				     *
*      A(i-1) Xj + B(i-1) Yj + C(i-1) = Rj.				     *
*									     *
*   Let dAi = Ai - A(i-1), etc.  Then:					     *
*									     *
*      (Ai - A(i-1)) Xj + (Bi - B(i-1)) Yj + (Ci - C(i-1)) = 0, or	     *
*      dAi Xj + dBi Yj + dCi = 0.			(1)		     *
*									     *
*   We now impose a quadratic distance constraint between circle center      *
* (Xj, Yj) and circle center (X(j-1), Y(j-1)) (note j-1 is taken mod 3):     *
*									     *
*      (X(j-1) - Xj)^2 + (Y(j-1) - Yj)^2 = (Rj + R(j-1)^2.		     *
*									     *
*   Using Equation (1) above, and assuming dAi != 0, we have		     *
* Xj = DBi Yj + DCi, where DBi = -dBi/dAi and DCi = -dCi/dAi.  Substituting: *
*									     *
*   (DB(i-1) Y(j-1) + DC(i-1) - DBi Yj + DCi)^2 + (Y(j-1) - Yj)^2 =          *
*   (Ai (DBi Yj + DCi) + Bi Yj + Ci +					     *
*    A(i-1) (DB(i-1) Y(j-1) + DC(i-1)) + B(i-1) Y(j-1) + C(i-1))^2.	     *
*									     *
*   which is the final constraint in the 3 Y variables only. One has to      *
* collect all the (up to quadratic) terms of Yj and Y(j-1): 		     *
*									     *
*   Y(j-1)^2  : DB(i-1)^2 + 1 - (A(i-1) DB(i-1) + B(i-1))^2,		     *
*   Yj^2      : DBi^2 + 1 - (Ai DBi + Bi)^2,				     *
*   Y(j-1) Yj : 2 DB(i-1) DBi - 2 + 2 (A(i-1) DB(i-1) + B(i-1))(Ai DBi + Bi),*
*									     *
* and similarly for the linear/constant terms.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Idx:       Index of point in the triangle, 0 for first point.            *
*   Y:         The 3 Y DOFs (Y coefs. of centers of circles).		     *
*   L, LNext, LPrev: The lines of the triangles. L & LPrev are around Pt Idx.*
*   dBRet, dCRet: to Recover the X value from the Y value.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:  The constructed constraint.                             *
*****************************************************************************/
static MvarMVStruct *Mvar3CircsInTrianglesEqn(int Idx,
					      MvarMVStruct *Y[3],
					      const IrtLnType L,
					      const IrtLnType LNext,
					      const IrtLnType LPrev,
					      CagdRType dBRet[3],
					      CagdRType dCRet[3])
{
    int IdxPrev = Idx == 0 ? 2 : Idx - 1;
    CagdRType c, d, e, f, Coef,
        dAi = L[0] - LPrev[0],
        dBi = L[1] - LPrev[1],
        dCi = L[2] - LPrev[2],
        dAi1 = LPrev[0] - LNext[0],
        dBi1 = LPrev[1] - LNext[1],
        dCi1 = LPrev[2] - LNext[2];
    MvarMVStruct *MVTmp1, *MVTmp2, *MVRet;

    if (fabs(dAi) < IRIT_EPS * 0.1 || fabs(dAi1) < IRIT_EPS * 0.1)
        return NULL;
    dBRet[Idx] = dBi = -dBi / dAi;
    dCRet[Idx] = dCi = -dCi / dAi;
    dBi1 = -dBi1 / dAi1;
    dCi1 = -dCi1 / dAi1;

    /* Build some temporary vars. */
    c = dCi1 - dCi;
    d = L[0] * dBi + L[1];
    e = LPrev[0] * dBi1 + LPrev[1];
    f = L[0] * dCi + L[2] + LPrev[0] * dCi1 + LPrev[2];

    /* And build the constraint: */
    Coef = IRIT_SQR(dBi1) + 1 - IRIT_SQR(e);		  /* Y(i-1)^2 term. */
    MVTmp1 = MvarMVMult(Y[IdxPrev], Y[IdxPrev]);
    MVRet = MvarMVScalarScale(MVTmp1, Coef);
    MvarMVFree(MVTmp1);

    Coef = IRIT_SQR(dBi) + 1 - IRIT_SQR(d);		      /* Yi^2 term. */
    MVTmp1 = MvarMVMult(Y[Idx], Y[Idx]);
    MVTmp2 = MvarMVScalarScale(MVTmp1, Coef);
    MvarMVFree(MVTmp1);
    MVTmp1 = MvarMVAdd(MVRet, MVTmp2);
    MvarMVFree(MVRet);
    MvarMVFree(MVTmp2);
    MVRet = MVTmp1;

    Coef = -2 * (dBi * dBi1 + 1 + d * e);		  /* Y(i-1)Yi term. */
    MVTmp1 = MvarMVMult(Y[Idx], Y[IdxPrev]);
    MVTmp2 = MvarMVScalarScale(MVTmp1, Coef);
    MvarMVFree(MVTmp1);
    MVTmp1 = MvarMVAdd(MVRet, MVTmp2);
    MvarMVFree(MVRet);
    MvarMVFree(MVTmp2);
    MVRet = MVTmp1;
    Coef = 2 * (dBi1 * c - e * f);			    /* Y(i-1) term. */

    MVTmp2 = MvarMVScalarScale(Y[IdxPrev], Coef);
    MVTmp1 = MvarMVAdd(MVRet, MVTmp2);
    MvarMVFree(MVRet);
    MvarMVFree(MVTmp2);
    MVRet = MVTmp1;

    Coef = 2 * (-dBi * c - d * f);			        /* Yi term. */
    MVTmp2 = MvarMVScalarScale(Y[Idx], Coef);
    MVTmp1 = MvarMVAdd(MVRet, MVTmp2);
    MvarMVFree(MVRet);
    MvarMVFree(MVTmp2);
    MVRet = MVTmp1;

    Coef = IRIT_SQR(c) - IRIT_SQR(f);			     /* const term. */
    MvarMVTransform(MVRet, &Coef, 1.0);

    return MVRet;
}
