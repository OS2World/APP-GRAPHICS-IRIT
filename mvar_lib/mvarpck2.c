/******************************************************************************
* MvarPck2.c - Packing related problems.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 09.					      *
******************************************************************************/

#include "mvar_loc.h"
#include "geom_lib.h"

#define MVAR_PACK_PT_IN_LINE(Pt, Ln)  \
			    ((Pt)[0] * (Ln)[0] + (Pt)[1] * (Ln)[1] + (Ln)[2])

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a triangles in the XY plane, specified by its 3 vertices Pts,      M
* Find the 6 circles packed inside the triangle (and tangent to it while     M
* also tangent to each other.						     M
*   The code to this function was synthesized automatically using the code   M
* below.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:          3 vertices of triangle in the plane (only XY coordinates). M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvement stage. M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Points in R9 as (x2, x4, x5, y1, y2, y3, y4, y5, y6).  M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   Mvar6CircsInTriangles                                                    M
*****************************************************************************/
MvarPtStruct *Mvar6CircsInTriangles(const CagdPType Pts[3],
				    CagdRType SubdivTol,
				    CagdRType NumericTol)
{
    int i;
    CagdRType R, A1, B1, C1, A2, B2, C2, A3, B3, C3, A23INV, A12INV, A13INV,
	Min[2], Max[2];
    CagdPType Cntr;
    IrtLnType L1, L2, L3;
    MvarPtStruct *MVPts, *MVPt, *MVPt18,
	*MVPts18 = NULL;
    MvarConstraintType Constrs[27];
    MvarMVStruct *MVs[27],
	*X2, *X4, *X5, *Y1, *Y2, *Y3, *Y4, *Y5, *Y6,
	*MVT1, *MVT2, *MVT3, *MVT4, *MVT5, *MVT6, *MVT7, *MVT8,
	*MVT9, *MVT10, *MVT11, *MVT12, *MVT13, *MVT14, *MVT15, *MVT16,
	*MVT17, *MVT18, *MVT19, *MVT20, *MVT21, *MVT22, *MVT23, *MVT24,
	*MVT25, *MVT26, *MVT27;

    if (!GMLineFrom2Points(L1, Pts[0], Pts[1]) ||
	!GMLineFrom2Points(L2, Pts[1], Pts[2]) ||
	!GMLineFrom2Points(L3, Pts[2], Pts[0]) ||
	IRIT_APX_EQ(L1[0] - L2[0], IRIT_UEPS) ||
	IRIT_APX_EQ(L1[0] - L3[0], IRIT_UEPS) ||
	IRIT_APX_EQ(L2[0] - L3[0], IRIT_UEPS))
        return NULL;

    IRIT_PT2D_ADD(Cntr, Pts[0], Pts[1]);
    IRIT_PT2D_ADD(Cntr, Cntr, Pts[2]);
    IRIT_PT2D_SCALE(Cntr, 1.0 / 3.0);

    /* If the center is negative in the lines - flip all lines. */
    if (MVAR_PACK_PT_IN_LINE(Cntr, L1) < 0.0) {
        IRIT_PT_SCALE(L1, -1.0);
	IRIT_PT_SCALE(L2, -1.0);
	IRIT_PT_SCALE(L3, -1.0);
    }
    /* If the center is too close to one of the edges, abort. */
    if (MVAR_PACK_PT_IN_LINE(Cntr, L1) < IRIT_EPS * 10 ||
	MVAR_PACK_PT_IN_LINE(Cntr, L2) < IRIT_EPS * 10 ||
	MVAR_PACK_PT_IN_LINE(Cntr, L3) < IRIT_EPS * 10)
        return NULL;

    A1 = L1[0];
    B1 = L1[1];
    C1 = L1[2];

    A2 = L2[0];
    B2 = L2[1];
    C2 = L2[2];

    A3 = L3[0];
    B3 = L3[1];
    C3 = L3[2];

    /* Note we checked for signularities above for theis code: */
    A12INV = 1.0 / (A1 - A2);
    A13INV = 1.0 / (A1 - A3);
    A23INV = 1.0 / (A2 - A3);

    /* Find the domain to search for XY locations and build the 3 DOFs. */
    Min[0] = IRIT_MIN(IRIT_MIN(Pts[0][0], Pts[1][0]), Pts[2][0]);
    Min[1] = IRIT_MIN(IRIT_MIN(Pts[0][1], Pts[1][1]), Pts[2][1]);
    Max[0] = IRIT_MAX(IRIT_MAX(Pts[0][0], Pts[1][0]), Pts[2][0]);
    Max[1] = IRIT_MAX(IRIT_MAX(Pts[0][1], Pts[1][1]), Pts[2][1]);

    /* Build the xi, yj parameters. */
    X2 = MvarBuildParamMV(9, 0, Min[0], Max[0]);
    X4 = MvarBuildParamMV(9, 1, Min[0], Max[0]);
    X5 = MvarBuildParamMV(9, 2, Min[0], Max[0]);
    Y1 = MvarBuildParamMV(9, 3, Min[1], Max[1]);
    Y2 = MvarBuildParamMV(9, 4, Min[1], Max[1]);
    Y3 = MvarBuildParamMV(9, 5, Min[1], Max[1]);
    Y4 = MvarBuildParamMV(9, 6, Min[1], Max[1]);
    Y5 = MvarBuildParamMV(9, 7, Min[1], Max[1]);
    Y6 = MvarBuildParamMV(9, 8, Min[1], Max[1]);

    /* Expression: "(x1 - x2)^2 + (y1 - y2)^2 - (r1 + r2)^2". */
    /* Expanded: "((A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) - X2)^2 + (Y1 - Y2)^2 - ((A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2) + (A2 * X2 + B2 * Y2 + C2))^2". */
    {
	MVT1 = MvarMVScalarScale(Y1, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y1, B2);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A23INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(MVT6, X2);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y1, Y2);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(Y1, B3);
	MVT13 = MvarMVCopy(MVT12);
	R = C3;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVScalarScale(Y1, B2);
	MVT15 = MvarMVCopy(MVT14);
	R = C2;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVSub(MVT13, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT13);
	MVT17 = MvarMVScalarScale(MVT16, A23INV);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(MVT17, A2);
	MvarMVFree(MVT17);
	MVT19 = MvarMVScalarScale(Y1, B2);
	MVT20 = MvarMVAdd(MVT18, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT18);
	MVT21 = MvarMVCopy(MVT20);
	R = C2;
	MvarMVTransform(MVT21, &R, 1.0);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(X2, A2);
	MVT23 = MvarMVScalarScale(Y2, B2);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C2;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT21, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT21);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[0] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }


    /* Expression: "(x2 - x3)^2 + (y2 - y3)^2 - (r2 + r3)^2". */
    /* Expanded: "(X2 - (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))))^2 + (Y2 - Y3)^2 - ((A2 * X2 + B2 * Y2 + C2) + (A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1))^2". */
    {
	MVT1 = MvarMVScalarScale(Y3, B2);
	MVT2 = MvarMVCopy(MVT1);
	R = C2;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y3, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A12INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(X2, MVT6);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y2, Y3);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(X2, A2);
	MVT13 = MvarMVScalarScale(Y2, B2);
	MVT14 = MvarMVAdd(MVT12, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT12);
	MVT15 = MvarMVCopy(MVT14);
	R = C2;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVScalarScale(Y3, B2);
	MVT17 = MvarMVCopy(MVT16);
	R = C2;
	MvarMVTransform(MVT17, &R, 1.0);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y3, B1);
	MVT19 = MvarMVCopy(MVT18);
	R = C1;
	MvarMVTransform(MVT19, &R, 1.0);
	MvarMVFree(MVT18);
	MVT20 = MvarMVSub(MVT17, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT17);
	MVT21 = MvarMVScalarScale(MVT20, A12INV);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(MVT21, A1);
	MvarMVFree(MVT21);
	MVT23 = MvarMVScalarScale(Y3, B1);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C1;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT15, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT15);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[1] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }


    /* Expression: "(x1 - x4)^2 + (y1 - y4)^2 - (r1 + r4)^2". */
    /* Expanded: "((A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) - X4)^2 + (Y1 - Y4)^2 - ((A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2) + (A3 * X4 + B3 * Y4 + C3))^2". */
    {
	MVT1 = MvarMVScalarScale(Y1, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y1, B2);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A23INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(MVT6, X4);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y1, Y4);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(Y1, B3);
	MVT13 = MvarMVCopy(MVT12);
	R = C3;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVScalarScale(Y1, B2);
	MVT15 = MvarMVCopy(MVT14);
	R = C2;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVSub(MVT13, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT13);
	MVT17 = MvarMVScalarScale(MVT16, A23INV);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(MVT17, A2);
	MvarMVFree(MVT17);
	MVT19 = MvarMVScalarScale(Y1, B2);
	MVT20 = MvarMVAdd(MVT18, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT18);
	MVT21 = MvarMVCopy(MVT20);
	R = C2;
	MvarMVTransform(MVT21, &R, 1.0);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(X4, A3);
	MVT23 = MvarMVScalarScale(Y4, B3);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C3;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT21, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT21);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[2] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }


    /* Expression: "(x2 - x4)^2 + (y2 - y4)^2 - (r2 + r4)^2". */
    /* Expanded: "(X2 - X4)^2 + (Y2 - Y4)^2 - ((A2 * X2 + B2 * Y2 + C2) + (A3 * X4 + B3 * Y4 + C3))^2". */
    {
	MVT1 = MvarMVSub(X2, X4);
	MVT2 = MvarMVCopy(MVT1);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT2, MVT1);
	    MvarMVFree(MVT2);
	    MVT2 = MVTmp;
	}
	MvarMVFree(MVT1);
	MVT3 = MvarMVSub(Y2, Y4);
	MVT4 = MvarMVCopy(MVT3);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT4, MVT3);
	    MvarMVFree(MVT4);
	    MVT4 = MVTmp;
	}
	MvarMVFree(MVT3);
	MVT5 = MvarMVAdd(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(X2, A2);
	MVT7 = MvarMVScalarScale(Y2, B2);
	MVT8 = MvarMVAdd(MVT6, MVT7);
	MvarMVFree(MVT7);
	MvarMVFree(MVT6);
	MVT9 = MvarMVCopy(MVT8);
	R = C2;
	MvarMVTransform(MVT9, &R, 1.0);
	MvarMVFree(MVT8);
	MVT10 = MvarMVScalarScale(X4, A3);
	MVT11 = MvarMVScalarScale(Y4, B3);
	MVT12 = MvarMVAdd(MVT10, MVT11);
	MvarMVFree(MVT11);
	MvarMVFree(MVT10);
	MVT13 = MvarMVCopy(MVT12);
	R = C3;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVAdd(MVT9, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT9);
	MVT15 = MvarMVCopy(MVT14);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT15, MVT14);
	    MvarMVFree(MVT15);
	    MVT15 = MVTmp;
	}
	MvarMVFree(MVT14);
	MVs[3] = MvarMVSub(MVT5, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT5);
    }


    /* Expression: "(x2 - x5)^2 + (y2 - y5)^2 - (r2 + r5)^2". */
    /* Expanded: "(X2 - X5)^2 + (Y2 - Y5)^2 - ((A2 * X2 + B2 * Y2 + C2) + (A1 * X5 + B1 * Y5 + C1))^2". */
    {
	MVT1 = MvarMVSub(X2, X5);
	MVT2 = MvarMVCopy(MVT1);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT2, MVT1);
	    MvarMVFree(MVT2);
	    MVT2 = MVTmp;
	}
	MvarMVFree(MVT1);
	MVT3 = MvarMVSub(Y2, Y5);
	MVT4 = MvarMVCopy(MVT3);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT4, MVT3);
	    MvarMVFree(MVT4);
	    MVT4 = MVTmp;
	}
	MvarMVFree(MVT3);
	MVT5 = MvarMVAdd(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(X2, A2);
	MVT7 = MvarMVScalarScale(Y2, B2);
	MVT8 = MvarMVAdd(MVT6, MVT7);
	MvarMVFree(MVT7);
	MvarMVFree(MVT6);
	MVT9 = MvarMVCopy(MVT8);
	R = C2;
	MvarMVTransform(MVT9, &R, 1.0);
	MvarMVFree(MVT8);
	MVT10 = MvarMVScalarScale(X5, A1);
	MVT11 = MvarMVScalarScale(Y5, B1);
	MVT12 = MvarMVAdd(MVT10, MVT11);
	MvarMVFree(MVT11);
	MvarMVFree(MVT10);
	MVT13 = MvarMVCopy(MVT12);
	R = C1;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVAdd(MVT9, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT9);
	MVT15 = MvarMVCopy(MVT14);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT15, MVT14);
	    MvarMVFree(MVT15);
	    MVT15 = MVTmp;
	}
	MvarMVFree(MVT14);
	MVs[4] = MvarMVSub(MVT5, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT5);
    }


    /* Expression: "(x3 - x5)^2 + (y3 - y5)^2 - (r3 + r5)^2". */
    /* Expanded: "((A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) - X5)^2 + (Y3 - Y5)^2 - ((A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1) + (A1 * X5 + B1 * Y5 + C1))^2". */
    {
	MVT1 = MvarMVScalarScale(Y3, B2);
	MVT2 = MvarMVCopy(MVT1);
	R = C2;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y3, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A12INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(MVT6, X5);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y3, Y5);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(Y3, B2);
	MVT13 = MvarMVCopy(MVT12);
	R = C2;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVScalarScale(Y3, B1);
	MVT15 = MvarMVCopy(MVT14);
	R = C1;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVSub(MVT13, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT13);
	MVT17 = MvarMVScalarScale(MVT16, A12INV);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(MVT17, A1);
	MvarMVFree(MVT17);
	MVT19 = MvarMVScalarScale(Y3, B1);
	MVT20 = MvarMVAdd(MVT18, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT18);
	MVT21 = MvarMVCopy(MVT20);
	R = C1;
	MvarMVTransform(MVT21, &R, 1.0);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(X5, A1);
	MVT23 = MvarMVScalarScale(Y5, B1);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C1;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT21, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT21);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[5] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }


    /* Expression: "(x4 - x5)^2 + (y4 - y5)^2 - (r4 + r5)^2". */
    /* Expanded: "(X4 - X5)^2 + (Y4 - Y5)^2 - ((A3 * X4 + B3 * Y4 + C3) + (A1 * X5 + B1 * Y5 + C1))^2". */
    {
	MVT1 = MvarMVSub(X4, X5);
	MVT2 = MvarMVCopy(MVT1);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT2, MVT1);
	    MvarMVFree(MVT2);
	    MVT2 = MVTmp;
	}
	MvarMVFree(MVT1);
	MVT3 = MvarMVSub(Y4, Y5);
	MVT4 = MvarMVCopy(MVT3);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT4, MVT3);
	    MvarMVFree(MVT4);
	    MVT4 = MVTmp;
	}
	MvarMVFree(MVT3);
	MVT5 = MvarMVAdd(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(X4, A3);
	MVT7 = MvarMVScalarScale(Y4, B3);
	MVT8 = MvarMVAdd(MVT6, MVT7);
	MvarMVFree(MVT7);
	MvarMVFree(MVT6);
	MVT9 = MvarMVCopy(MVT8);
	R = C3;
	MvarMVTransform(MVT9, &R, 1.0);
	MvarMVFree(MVT8);
	MVT10 = MvarMVScalarScale(X5, A1);
	MVT11 = MvarMVScalarScale(Y5, B1);
	MVT12 = MvarMVAdd(MVT10, MVT11);
	MvarMVFree(MVT11);
	MvarMVFree(MVT10);
	MVT13 = MvarMVCopy(MVT12);
	R = C1;
	MvarMVTransform(MVT13, &R, 1.0);
	MvarMVFree(MVT12);
	MVT14 = MvarMVAdd(MVT9, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT9);
	MVT15 = MvarMVCopy(MVT14);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT15, MVT14);
	    MvarMVFree(MVT15);
	    MVT15 = MVTmp;
	}
	MvarMVFree(MVT14);
	MVs[6] = MvarMVSub(MVT5, MVT15);
	MvarMVFree(MVT15);
	MvarMVFree(MVT5);
    }


    /* Expression: "(x4 - x6)^2 + (y4 - y6)^2 - (r4 + r6)^2". */
    /* Expanded: "(X4 - (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))))^2 + (Y4 - Y6)^2 - ((A3 * X4 + B3 * Y4 + C3) + (A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1))^2". */
    {
	MVT1 = MvarMVScalarScale(Y6, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y6, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A13INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(X4, MVT6);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y4, Y6);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(X4, A3);
	MVT13 = MvarMVScalarScale(Y4, B3);
	MVT14 = MvarMVAdd(MVT12, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT12);
	MVT15 = MvarMVCopy(MVT14);
	R = C3;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVScalarScale(Y6, B3);
	MVT17 = MvarMVCopy(MVT16);
	R = C3;
	MvarMVTransform(MVT17, &R, 1.0);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y6, B1);
	MVT19 = MvarMVCopy(MVT18);
	R = C1;
	MvarMVTransform(MVT19, &R, 1.0);
	MvarMVFree(MVT18);
	MVT20 = MvarMVSub(MVT17, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT17);
	MVT21 = MvarMVScalarScale(MVT20, A13INV);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(MVT21, A1);
	MvarMVFree(MVT21);
	MVT23 = MvarMVScalarScale(Y6, B1);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C1;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT15, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT15);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[7] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }


    /* Expression: "(x5 - x6)^2 + (y5 - y6)^2 - (r5 + r6)^2". */
    /* Expanded: "(X5 - (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))))^2 + (Y5 - Y6)^2 - ((A1 * X5 + B1 * Y5 + C1) + (A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1))^2". */
    {
	MVT1 = MvarMVScalarScale(Y6, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y6, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A13INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVSub(X5, MVT6);
	MvarMVFree(MVT6);
	MVT8 = MvarMVCopy(MVT7);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT8, MVT7);
	    MvarMVFree(MVT8);
	    MVT8 = MVTmp;
	}
	MvarMVFree(MVT7);
	MVT9 = MvarMVSub(Y5, Y6);
	MVT10 = MvarMVCopy(MVT9);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT10, MVT9);
	    MvarMVFree(MVT10);
	    MVT10 = MVTmp;
	}
	MvarMVFree(MVT9);
	MVT11 = MvarMVAdd(MVT8, MVT10);
	MvarMVFree(MVT10);
	MvarMVFree(MVT8);
	MVT12 = MvarMVScalarScale(X5, A1);
	MVT13 = MvarMVScalarScale(Y5, B1);
	MVT14 = MvarMVAdd(MVT12, MVT13);
	MvarMVFree(MVT13);
	MvarMVFree(MVT12);
	MVT15 = MvarMVCopy(MVT14);
	R = C1;
	MvarMVTransform(MVT15, &R, 1.0);
	MvarMVFree(MVT14);
	MVT16 = MvarMVScalarScale(Y6, B3);
	MVT17 = MvarMVCopy(MVT16);
	R = C3;
	MvarMVTransform(MVT17, &R, 1.0);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y6, B1);
	MVT19 = MvarMVCopy(MVT18);
	R = C1;
	MvarMVTransform(MVT19, &R, 1.0);
	MvarMVFree(MVT18);
	MVT20 = MvarMVSub(MVT17, MVT19);
	MvarMVFree(MVT19);
	MvarMVFree(MVT17);
	MVT21 = MvarMVScalarScale(MVT20, A13INV);
	MvarMVFree(MVT20);
	MVT22 = MvarMVScalarScale(MVT21, A1);
	MvarMVFree(MVT21);
	MVT23 = MvarMVScalarScale(Y6, B1);
	MVT24 = MvarMVAdd(MVT22, MVT23);
	MvarMVFree(MVT23);
	MvarMVFree(MVT22);
	MVT25 = MvarMVCopy(MVT24);
	R = C1;
	MvarMVTransform(MVT25, &R, 1.0);
	MvarMVFree(MVT24);
	MVT26 = MvarMVAdd(MVT15, MVT25);
	MvarMVFree(MVT25);
	MvarMVFree(MVT15);
	MVT27 = MvarMVCopy(MVT26);
	for (i = 1; i < 2; i++) {
	    MvarMVStruct
		*MVTmp = MvarMVMult(MVT27, MVT26);
	    MvarMVFree(MVT27);
	    MVT27 = MVTmp;
	}
	MvarMVFree(MVT26);
	MVs[8] = MvarMVSub(MVT11, MVT27);
	MvarMVFree(MVT27);
	MvarMVFree(MVT11);
    }



    /* Expression: "A1 * x1 + B1 * y1 + C1 - r1 * 0.99". */
    /* Expanded: "A1 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B1 * Y1 + C1 - (A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y1, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y1, B2);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A23INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A1);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y1, B1);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C1;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y1, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y1, B2);
	MVT14 = MvarMVCopy(MVT13);
	R = C2;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A23INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A2);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y1, B2);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C2;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[9] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A2 * x1 + B2 * y1 + C2 - r1 * 0.99". */
    /* Expanded: "A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2 - (A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y1, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y1, B2);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A23INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A2);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y1, B2);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C2;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y1, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y1, B2);
	MVT14 = MvarMVCopy(MVT13);
	R = C2;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A23INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A2);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y1, B2);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C2;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[10] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A3 * x1 + B3 * y1 + C3 - r1 * 0.99". */
    /* Expanded: "A3 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B3 * Y1 + C3 - (A2 * (A23INV * ((B3 * Y1 + C3) - (B2 * Y1 + C2))) + B2 * Y1 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y1, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y1, B2);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A23INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A3);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y1, B3);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C3;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y1, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y1, B2);
	MVT14 = MvarMVCopy(MVT13);
	R = C2;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A23INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A2);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y1, B2);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C2;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[11] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A1 * x2 + B1 * y2 + C1 - r2 * 0.99". */
    /* Expanded: "A1 * X2 + B1 * Y2 + C1 - (A2 * X2 + B2 * Y2 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X2, A1);
	MVT2 = MvarMVScalarScale(Y2, B1);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X2, A2);
	MVT6 = MvarMVScalarScale(Y2, B2);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C2;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[12] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A2 * x2 + B2 * y2 + C2 - r2 * 0.99". */
    /* Expanded: "A2 * X2 + B2 * Y2 + C2 - (A2 * X2 + B2 * Y2 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X2, A2);
	MVT2 = MvarMVScalarScale(Y2, B2);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X2, A2);
	MVT6 = MvarMVScalarScale(Y2, B2);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C2;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[13] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A3 * x2 + B3 * y2 + C3 - r2 * 0.99". */
    /* Expanded: "A3 * X2 + B3 * Y2 + C3 - (A2 * X2 + B2 * Y2 + C2) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X2, A3);
	MVT2 = MvarMVScalarScale(Y2, B3);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C3;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X2, A2);
	MVT6 = MvarMVScalarScale(Y2, B2);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C2;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[14] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A1 * x3 + B1 * y3 + C1 - r3 * 0.99". */
    /* Expanded: "A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1 - (A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y3, B2);
	MVT2 = MvarMVCopy(MVT1);
	R = C2;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y3, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A12INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A1);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y3, B1);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C1;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y3, B2);
	MVT12 = MvarMVCopy(MVT11);
	R = C2;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y3, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A12INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y3, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[15] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A2 * x3 + B2 * y3 + C2 - r3 * 0.99". */
    /* Expanded: "A2 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B2 * Y3 + C2 - (A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y3, B2);
	MVT2 = MvarMVCopy(MVT1);
	R = C2;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y3, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A12INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A2);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y3, B2);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C2;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y3, B2);
	MVT12 = MvarMVCopy(MVT11);
	R = C2;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y3, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A12INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y3, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[16] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A3 * x3 + B3 * y3 + C3 - r3 * 0.99". */
    /* Expanded: "A3 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B3 * Y3 + C3 - (A1 * (A12INV * ((B2 * Y3 + C2) - (B1 * Y3 + C1))) + B1 * Y3 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y3, B2);
	MVT2 = MvarMVCopy(MVT1);
	R = C2;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y3, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A12INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A3);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y3, B3);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C3;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y3, B2);
	MVT12 = MvarMVCopy(MVT11);
	R = C2;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y3, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A12INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y3, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[17] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A1 * x4 + B1 * y4 + C1 - r4 * 0.99". */
    /* Expanded: "A1 * X4 + B1 * Y4 + C1 - (A3 * X4 + B3 * Y4 + C3) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X4, A1);
	MVT2 = MvarMVScalarScale(Y4, B1);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X4, A3);
	MVT6 = MvarMVScalarScale(Y4, B3);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C3;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[18] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A2 * x4 + B2 * y4 + C2 - r4 * 0.99". */
    /* Expanded: "A2 * X4 + B2 * Y4 + C2 - (A3 * X4 + B3 * Y4 + C3) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X4, A2);
	MVT2 = MvarMVScalarScale(Y4, B2);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X4, A3);
	MVT6 = MvarMVScalarScale(Y4, B3);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C3;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[19] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A3 * x4 + B3 * y4 + C3 - r4 * 0.99". */
    /* Expanded: "A3 * X4 + B3 * Y4 + C3 - (A3 * X4 + B3 * Y4 + C3) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X4, A3);
	MVT2 = MvarMVScalarScale(Y4, B3);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C3;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X4, A3);
	MVT6 = MvarMVScalarScale(Y4, B3);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C3;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[20] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A1 * x5 + B1 * y5 + C1 - r5 * 0.99". */
    /* Expanded: "A1 * X5 + B1 * Y5 + C1 - (A1 * X5 + B1 * Y5 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X5, A1);
	MVT2 = MvarMVScalarScale(Y5, B1);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X5, A1);
	MVT6 = MvarMVScalarScale(Y5, B1);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C1;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[21] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A2 * x5 + B2 * y5 + C2 - r5 * 0.99". */
    /* Expanded: "A2 * X5 + B2 * Y5 + C2 - (A1 * X5 + B1 * Y5 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X5, A2);
	MVT2 = MvarMVScalarScale(Y5, B2);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C2;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X5, A1);
	MVT6 = MvarMVScalarScale(Y5, B1);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C1;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[22] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A3 * x5 + B3 * y5 + C3 - r5 * 0.99". */
    /* Expanded: "A3 * X5 + B3 * Y5 + C3 - (A1 * X5 + B1 * Y5 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(X5, A3);
	MVT2 = MvarMVScalarScale(Y5, B3);
	MVT3 = MvarMVAdd(MVT1, MVT2);
	MvarMVFree(MVT2);
	MvarMVFree(MVT1);
	MVT4 = MvarMVCopy(MVT3);
	R = C3;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVScalarScale(X5, A1);
	MVT6 = MvarMVScalarScale(Y5, B1);
	MVT7 = MvarMVAdd(MVT5, MVT6);
	MvarMVFree(MVT6);
	MvarMVFree(MVT5);
	MVT8 = MvarMVCopy(MVT7);
	R = C1;
	MvarMVTransform(MVT8, &R, 1.0);
	MvarMVFree(MVT7);
	MVT9 = MvarMVScalarScale(MVT8, 0.990000);
	MvarMVFree(MVT8);
	MVs[23] = MvarMVSub(MVT4, MVT9);
	MvarMVFree(MVT9);
	MvarMVFree(MVT4);
    }


    /* Expression: "A1 * x6 + B1 * y6 + C1 - r6 * 0.99". */
    /* Expanded: "A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1 - (A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y6, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y6, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A13INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A1);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y6, B1);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C1;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y6, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y6, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A13INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y6, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[24] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A2 * x6 + B2 * y6 + C2 - r6 * 0.99". */
    /* Expanded: "A2 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B2 * Y6 + C2 - (A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y6, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y6, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A13INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A2);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y6, B2);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C2;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y6, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y6, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A13INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y6, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[25] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    /* Expression: "A3 * x6 + B3 * y6 + C3 - r6 * 0.99". */
    /* Expanded: "A3 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B3 * Y6 + C3 - (A1 * (A13INV * ((B3 * Y6 + C3) - (B1 * Y6 + C1))) + B1 * Y6 + C1) * 0.99". */
    {
	MVT1 = MvarMVScalarScale(Y6, B3);
	MVT2 = MvarMVCopy(MVT1);
	R = C3;
	MvarMVTransform(MVT2, &R, 1.0);
	MvarMVFree(MVT1);
	MVT3 = MvarMVScalarScale(Y6, B1);
	MVT4 = MvarMVCopy(MVT3);
	R = C1;
	MvarMVTransform(MVT4, &R, 1.0);
	MvarMVFree(MVT3);
	MVT5 = MvarMVSub(MVT2, MVT4);
	MvarMVFree(MVT4);
	MvarMVFree(MVT2);
	MVT6 = MvarMVScalarScale(MVT5, A13INV);
	MvarMVFree(MVT5);
	MVT7 = MvarMVScalarScale(MVT6, A3);
	MvarMVFree(MVT6);
	MVT8 = MvarMVScalarScale(Y6, B3);
	MVT9 = MvarMVAdd(MVT7, MVT8);
	MvarMVFree(MVT8);
	MvarMVFree(MVT7);
	MVT10 = MvarMVCopy(MVT9);
	R = C3;
	MvarMVTransform(MVT10, &R, 1.0);
	MvarMVFree(MVT9);
	MVT11 = MvarMVScalarScale(Y6, B3);
	MVT12 = MvarMVCopy(MVT11);
	R = C3;
	MvarMVTransform(MVT12, &R, 1.0);
	MvarMVFree(MVT11);
	MVT13 = MvarMVScalarScale(Y6, B1);
	MVT14 = MvarMVCopy(MVT13);
	R = C1;
	MvarMVTransform(MVT14, &R, 1.0);
	MvarMVFree(MVT13);
	MVT15 = MvarMVSub(MVT12, MVT14);
	MvarMVFree(MVT14);
	MvarMVFree(MVT12);
	MVT16 = MvarMVScalarScale(MVT15, A13INV);
	MvarMVFree(MVT15);
	MVT17 = MvarMVScalarScale(MVT16, A1);
	MvarMVFree(MVT16);
	MVT18 = MvarMVScalarScale(Y6, B1);
	MVT19 = MvarMVAdd(MVT17, MVT18);
	MvarMVFree(MVT18);
	MvarMVFree(MVT17);
	MVT20 = MvarMVCopy(MVT19);
	R = C1;
	MvarMVTransform(MVT20, &R, 1.0);
	MvarMVFree(MVT19);
	MVT21 = MvarMVScalarScale(MVT20, 0.990000);
	MvarMVFree(MVT20);
	MVs[26] = MvarMVSub(MVT10, MVT21);
	MvarMVFree(MVT21);
	MvarMVFree(MVT10);
    }


    Constrs[0] = MVAR_CNSTRNT_ZERO;
    Constrs[1] = MVAR_CNSTRNT_ZERO;
    Constrs[2] = MVAR_CNSTRNT_ZERO;
    Constrs[3] = MVAR_CNSTRNT_ZERO;
    Constrs[4] = MVAR_CNSTRNT_ZERO;
    Constrs[5] = MVAR_CNSTRNT_ZERO;
    Constrs[6] = MVAR_CNSTRNT_ZERO;
    Constrs[7] = MVAR_CNSTRNT_ZERO;
    Constrs[8] = MVAR_CNSTRNT_ZERO;
    Constrs[9] = MVAR_CNSTRNT_POSITIVE;
    Constrs[10] = MVAR_CNSTRNT_POSITIVE;
    Constrs[11] = MVAR_CNSTRNT_POSITIVE;
    Constrs[12] = MVAR_CNSTRNT_POSITIVE;
    Constrs[13] = MVAR_CNSTRNT_POSITIVE;
    Constrs[14] = MVAR_CNSTRNT_POSITIVE;
    Constrs[15] = MVAR_CNSTRNT_POSITIVE;
    Constrs[16] = MVAR_CNSTRNT_POSITIVE;
    Constrs[17] = MVAR_CNSTRNT_POSITIVE;
    Constrs[18] = MVAR_CNSTRNT_POSITIVE;
    Constrs[19] = MVAR_CNSTRNT_POSITIVE;
    Constrs[20] = MVAR_CNSTRNT_POSITIVE;
    Constrs[21] = MVAR_CNSTRNT_POSITIVE;
    Constrs[22] = MVAR_CNSTRNT_POSITIVE;
    Constrs[23] = MVAR_CNSTRNT_POSITIVE;
    Constrs[24] = MVAR_CNSTRNT_POSITIVE;
    Constrs[25] = MVAR_CNSTRNT_POSITIVE;
    Constrs[26] = MVAR_CNSTRNT_POSITIVE;

    MVPts = MvarMVsZeros(MVs, Constrs, 27, SubdivTol, NumericTol);

    for (i = 0; i < 27; i++)
        MvarMVFree(MVs[i]);

    MvarMVFree(X2);
    MvarMVFree(X4);
    MvarMVFree(X5);
    MvarMVFree(Y1);
    MvarMVFree(Y2);
    MvarMVFree(Y3);
    MvarMVFree(Y4);
    MvarMVFree(Y5);
    MvarMVFree(Y6);

    /* Time to evaluate (xi, yi, ri) for all 6 circles. */
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        MVPt18 = MvarPtNew(18);

	/* Map solutions from parameter to Euclidean space. */
	for (i = 0; i < 3; i++)
	    MVPt -> Pt[i] = MVPt -> Pt[i] * (Max[0] - Min[0]) + Min[0];
	for (i = 3; i < 9; i++)
	    MVPt -> Pt[i] = MVPt -> Pt[i] * (Max[1] - Min[1]) + Min[1];

	/* x1, y1, r1. */
	MVPt18 -> Pt[0] = A23INV * ((B3 * MVPt -> Pt[3] + C3) -
				    (B2 * MVPt -> Pt[3] + C2));  
	MVPt18 -> Pt[1] = MVPt -> Pt[3];  
	MVPt18 -> Pt[2] = A2 * MVPt18 -> Pt[0] + B2 * MVPt18 -> Pt[1] + C2;

	/* x2, y2, r2. */
	MVPt18 -> Pt[3] = MVPt -> Pt[0];  
	MVPt18 -> Pt[4] = MVPt -> Pt[4];  
	MVPt18 -> Pt[5] = A2 * MVPt18 -> Pt[3] + B2 * MVPt18 -> Pt[4] + C2;

	/* x3, y3, r3. */
	MVPt18 -> Pt[6] = A12INV * ((B2 *  MVPt -> Pt[5] + C2) -
				    (B1 *  MVPt -> Pt[5] + C1));
	MVPt18 -> Pt[7] = MVPt -> Pt[5];  
	MVPt18 -> Pt[8] = A1 * MVPt18 -> Pt[6] + B1 * MVPt18 -> Pt[7] + C1;

	/* x4, y4, r4. */
	MVPt18 -> Pt[9] = MVPt -> Pt[1];  
	MVPt18 -> Pt[10] = MVPt -> Pt[6];  
	MVPt18 -> Pt[11] = A3 * MVPt18 -> Pt[9] + B3 * MVPt18 -> Pt[10] + C3;

	/* x5, y5, r5. */
	MVPt18 -> Pt[12] = MVPt -> Pt[2];  
	MVPt18 -> Pt[13] = MVPt -> Pt[7];  
	MVPt18 -> Pt[14] = A1 * MVPt18 -> Pt[12] + B1 * MVPt18 -> Pt[13] + C1;

	/* x6, y6, r6. */
	MVPt18 -> Pt[15] = A13INV * ((B3 * MVPt -> Pt[8] + C3) -
				     (B1 * MVPt -> Pt[8] + C1));
	MVPt18 -> Pt[16] = MVPt -> Pt[8];  
	MVPt18 -> Pt[17] = A1 * MVPt18 -> Pt[15] + B1 * MVPt18 -> Pt[16] + C1;

	/* Make sure solution is in the triangle. */
	for (i = 0; i < 18; i += 3) {
	    if (MVAR_PACK_PT_IN_LINE(&MVPt18 -> Pt[i], L1) < 0.0 ||
		MVAR_PACK_PT_IN_LINE(&MVPt18 -> Pt[i], L2) < 0.0 ||
		MVAR_PACK_PT_IN_LINE(&MVPt18 -> Pt[i], L3) < 0.0)
		break;
	}
	if (i < 18) {
	    MvarPtFree(MVPt18);       /* Some centers are outside triangle. */
	}
	else
	    IRIT_LIST_PUSH(MVPt18, MVPts18);
    }

    MvarPtFreeList(MVPts);

    return MVPts18;
}

/* This code is used to automatically synthesize the above solution to 6    */
/* circles in a triangle, in function Mvar6CircsInTriangles.		    */

#ifdef MVAR_GEN_MV_ZR_CODE

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "mvar_lib.h"

#define MV_ZR_VERIFY(x, Str) \
    if (!x) { \
        fprintf(stderr, "%s (%d %d)\n", \
		Str, IritE2TParseError(), IritE2TDerivError()); \
    }

void main(int argc, char **argv)
{
    void
        *MVZrAl = MvarZrAlgCreate();

    /*            P0		 	 	 	 	 */
    /*            /\		 	 	 	 	 */
    /*           /  \	 	 	 		 	 */
    /*     L3   /    \		 Li = (A1, Bi, Ci) 	 	 */
    /*         / Cr6  \  L1	 	 	 		 */
    /*        /        \	 	 	 		 */
    /*       /          \	 	 	 		 */
    /*      /  Cr4  Cr5  \	 Cri = (xi, yi, ri) 		 */
    /*     /              \	 	 	 		 */
    /*    / Cr1   Cr2  Cr3 \	 	 	 		 */
    /* P2 ------------------ P1	 	 		 	 */
    /*				 	 	 		 */
    /*             L2 		 	 	 		 */
    
    /* 12 numeric variables. */
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A1", 0.0),
		 "Error in assignment numvar 1\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "B1", 0.0),
		 "Error in assignment numvar 2\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "C1", 0.0),
		 "Error in assignment numvar 3\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A2", 0.0),
		 "Error in assignment numvar 4\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "B2", 0.0),
		 "Error in assignment numvar 5\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "C2", 0.0),
		 "Error in assignment numvar 6\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A3", 0.0),
		 "Error in assignment numvar 7\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "B3", 0.0),
		 "Error in assignment numvar 8\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "C3", 0.0),
		 "Error in assignment numvar 9\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A12Inv", 0.0), /* 1/(A1-A2). */
		 "Error in assignment numvar 10\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A23Inv", 0.0),
		 "Error in assignment numvar 11\n");
    MV_ZR_VERIFY(MvarZrAlgAssignNumVar(MVZrAl, "A13Inv", 0.0),
		 "Error in assignment numvar 12\n");

    /* 9 linear equations - distances of circles centers from triangle's edges. */
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "x1", "A23Inv * ((B3 * y1 + C3) - (B2 * y1 + C2))"),
		 "Error linear equation 1\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "x3", "A12Inv * ((B2 * y3 + C2) - (B1 * y3 + C1))"),
		 "Error linear equation 2\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "x6", "A13Inv * ((B3 * y6 + C3) - (B1 * y6 + C1))"),
		 "Error linear equation 3\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r1", "A2 * x1 + B2 * y1 + C2"),
		 "Error linear equation 4\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r3", "A1 * x3 + B1 * y3 + C1"),
		 "Error linear equation 5\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r6", "A1 * x6 + B1 * y6 + C1"),
		 "Error linear equation 6\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r2", "A2 * x2 + B2 * y2 + C2"),
		 "Error linear equation 7\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r4", "A3 * x4 + B3 * y4 + C3"),
		 "Error linear equation 8\n");
    MV_ZR_VERIFY(MvarZrAlgAssignExpr(MVZrAl, "r5", "A1 * x5 + B1 * y5 + C1"),
		 "Error linear equation 9\n");

    /* 9 quadratic equations. */
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x1 - x2)^2 + (y1 - y2)^2 - (r1 + r2)^2",
				    stdout),
		 "Error in code generations 1\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x2 - x3)^2 + (y2 - y3)^2 - (r2 + r3)^2",
				    stdout),
		 "Error in code generations 2\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x1 - x4)^2 + (y1 - y4)^2 - (r1 + r4)^2",
				    stdout),
		 "Error in code generations 3\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x2 - x4)^2 + (y2 - y4)^2 - (r2 + r4)^2",
				    stdout),
		 "Error in code generations 4\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x2 - x5)^2 + (y2 - y5)^2 - (r2 + r5)^2",
				    stdout),
		 "Error in code generations 5\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x3 - x5)^2 + (y3 - y5)^2 - (r3 + r5)^2",
				    stdout),
		 "Error in code generations 6\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x4 - x5)^2 + (y4 - y5)^2 - (r4 + r5)^2",
				    stdout),
		 "Error in code generations 7\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x4 - x6)^2 + (y4 - y6)^2 - (r4 + r6)^2",
				    stdout),
		 "Error in code generations 8\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "(x5 - x6)^2 + (y5 - y6)^2 - (r5 + r6)^2",
				    stdout),
		 "Error in code generations 9\n");


    /* These are inequalities to purge out-of-triangle solutions. */
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x1 + B1 * y1 + C1 - r1 * 0.99", stdout),
		 "Error in code generations 10\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x1 + B2 * y1 + C2 - r1 * 0.99", stdout),
		 "Error in code generations 11\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x1 + B3 * y1 + C3 - r1 * 0.99", stdout),
		 "Error in code generations 12\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x2 + B1 * y2 + C1 - r2 * 0.99", stdout),
		 "Error in code generations 13\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x2 + B2 * y2 + C2 - r2 * 0.99", stdout),
		 "Error in code generations 14\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x2 + B3 * y2 + C3 - r2 * 0.99", stdout),
		 "Error in code generations 15\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x3 + B1 * y3 + C1 - r3 * 0.99", stdout),
		 "Error in code generations 16\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x3 + B2 * y3 + C2 - r3 * 0.99", stdout),
		 "Error in code generations 17\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x3 + B3 * y3 + C3 - r3 * 0.99", stdout),
		 "Error in code generations 18\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x4 + B1 * y4 + C1 - r4 * 0.99", stdout),
		 "Error in code generations 19\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x4 + B2 * y4 + C2 - r4 * 0.99", stdout),
		 "Error in code generations 20\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x4 + B3 * y4 + C3 - r4 * 0.99", stdout),
		 "Error in code generations 21\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x5 + B1 * y5 + C1 - r5 * 0.99", stdout),
		 "Error in code generations 22\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x5 + B2 * y5 + C2 - r5 * 0.99", stdout),
		 "Error in code generations 23\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x5 + B3 * y5 + C3 - r5 * 0.99", stdout),
		 "Error in code generations 24\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A1 * x6 + B1 * y6 + C1 - r6 * 0.99", stdout),
		 "Error in code generations 25\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A2 * x6 + B2 * y6 + C2 - r6 * 0.99", stdout),
		 "Error in code generations 26\n");
    MV_ZR_VERIFY(MvarZrAlgGenMVCode(MVZrAl, "A3 * x6 + B3 * y6 + C3 - r6 * 0.99", stdout),
		 "Error in code generations 27\n");

    MvarZrAlgDelete(MVZrAl);
}

#endif /* MVAR_GEN_MV_ZR_CODE */
