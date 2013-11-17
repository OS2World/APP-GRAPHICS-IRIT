/******************************************************************************
* GeoMat4d.c - Trans. Matrices , Vector computation, and Comp.geom, in 4D.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, November 1994.                                    *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a hyperplane in four space through the given four points.         M
* Based on a direct solution in Maple of:                                    M
*									     M
*  with(linalg);							     V
*  readlib(C);								     V
*  									     V
*  d := det( matrix( [ [x - x1, y - y1, z - z1, w - w1],		     V
*                      [x2 - x1, y2 - y1, z2 - z1, w2 - w1],      	     V
*                      [x3 - x2, y3 - y2, z3 - z2, w3 - w2],		     V
*                      [x4 - x3, y4 - y3, z4 - z3, w4 - w3]] ) );	     V
*  coeff( d, x );							     V
*  coeff( d, y );							     V
*  coeff( d, z );							     V
*  coeff( d, w );							     V
*									     *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3, Pt4:  The four points the plane should go through.        M
*   Plane:               Where the result should be placed.		     M
* RETURN VALUE:                                                              M
*   int:     TRUE if successful, FALSE otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivPlaneFrom4Points, hyper plane, plane                                 M
*****************************************************************************/
int TrivPlaneFrom4Points(const TrivPType Pt1,
			 const TrivPType Pt2,
			 const TrivPType Pt3,
			 const TrivPType Pt4,
			 TrivIrtPlnType Plane)
{
    Plane[0] = - Pt2[1] * Pt3[3] * Pt4[2] -
                 Pt1[1] * Pt2[2] * Pt3[3] +
                 Pt2[1] * Pt3[2] * Pt4[3] -
                 Pt1[1] * Pt3[2] * Pt4[3] +
                 Pt1[1] * Pt2[2] * Pt4[3] +
                 Pt1[1] * Pt3[3] * Pt4[2] -
                 Pt1[1] * Pt2[3] * Pt4[2] +
                 Pt1[1] * Pt2[3] * Pt3[2] -
                 Pt3[1] * Pt2[2] * Pt4[3] +
                 Pt3[1] * Pt1[2] * Pt4[3] +
                 Pt3[1] * Pt2[3] * Pt4[2] -
                 Pt3[1] * Pt1[3] * Pt4[2] -
                 Pt2[1] * Pt1[2] * Pt4[3] +
                 Pt2[1] * Pt1[2] * Pt3[3] +
                 Pt2[1] * Pt1[3] * Pt4[2] -
                 Pt2[1] * Pt1[3] * Pt3[2] +
                 Pt4[1] * Pt2[2] * Pt3[3] -
                 Pt4[1] * Pt1[2] * Pt3[3] +
                 Pt4[1] * Pt1[2] * Pt2[3] -
                 Pt4[1] * Pt2[3] * Pt3[2] +
                 Pt4[1] * Pt1[3] * Pt3[2] -
                 Pt4[1] * Pt1[3] * Pt2[2] -
                 Pt3[1] * Pt1[2] * Pt2[3] +
                 Pt3[1] * Pt1[3] * Pt2[2];
    Plane[1] = - Pt2[0] * Pt3[2] * Pt4[3] +
                 Pt2[0] * Pt3[3] * Pt4[2] -
                 Pt1[0] * Pt2[2] * Pt4[3] +
                 Pt1[0] * Pt3[2] * Pt4[3] +
                 Pt1[0] * Pt2[2] * Pt3[3] -
                 Pt1[0] * Pt3[3] * Pt4[2] +
                 Pt1[0] * Pt2[3] * Pt4[2] -
                 Pt1[0] * Pt2[3] * Pt3[2] +
                 Pt3[0] * Pt2[2] * Pt4[3] -
                 Pt3[0] * Pt2[3] * Pt4[2] -
                 Pt3[0] * Pt1[2] * Pt4[3] +
                 Pt3[0] * Pt1[3] * Pt4[2] +
                 Pt2[0] * Pt1[2] * Pt4[3] -
                 Pt2[0] * Pt1[2] * Pt3[3] -
                 Pt2[0] * Pt1[3] * Pt4[2] +
                 Pt2[0] * Pt1[3] * Pt3[2] -
                 Pt4[0] * Pt2[2] * Pt3[3] +
                 Pt4[0] * Pt2[3] * Pt3[2] +
                 Pt4[0] * Pt1[2] * Pt3[3] -
                 Pt4[0] * Pt1[3] * Pt3[2] -
                 Pt4[0] * Pt1[2] * Pt2[3] +
                 Pt4[0] * Pt1[3] * Pt2[2] +
                 Pt3[0] * Pt1[2] * Pt2[3] -
                 Pt3[0] * Pt1[3] * Pt2[2];
    Plane[2] =   Pt2[0] * Pt3[1] * Pt4[3] -
                 Pt2[0] * Pt4[1] * Pt3[3] -
                 Pt1[0] * Pt2[1] * Pt3[3] -
                 Pt1[0] * Pt3[1] * Pt4[3] +
                 Pt1[0] * Pt2[1] * Pt4[3] +
                 Pt1[0] * Pt4[1] * Pt3[3] -
                 Pt1[0] * Pt4[1] * Pt2[3] +
                 Pt1[0] * Pt3[1] * Pt2[3] -
                 Pt3[0] * Pt2[1] * Pt4[3] +
                 Pt3[0] * Pt4[1] * Pt2[3] +
                 Pt3[0] * Pt1[1] * Pt4[3] -
                 Pt3[0] * Pt4[1] * Pt1[3] -
                 Pt2[0] * Pt1[1] * Pt4[3] +
                 Pt2[0] * Pt1[1] * Pt3[3] +
                 Pt2[0] * Pt4[1] * Pt1[3] -
                 Pt2[0] * Pt3[1] * Pt1[3] +
                 Pt4[0] * Pt2[1] * Pt3[3] -
                 Pt4[0] * Pt3[1] * Pt2[3] -
                 Pt4[0] * Pt1[1] * Pt3[3] +
                 Pt4[0] * Pt3[1] * Pt1[3] +
                 Pt4[0] * Pt1[1] * Pt2[3] -
                 Pt4[0] * Pt2[1] * Pt1[3] -
                 Pt3[0] * Pt1[1] * Pt2[3] +
                 Pt3[0] * Pt2[1] * Pt1[3];
    Plane[3] = - Pt2[0] * Pt3[1] * Pt4[2] +
                 Pt2[0] * Pt4[1] * Pt3[2] +
                 Pt1[0] * Pt2[1] * Pt3[2] +
                 Pt1[0] * Pt3[1] * Pt4[2] -
                 Pt1[0] * Pt2[1] * Pt4[2] -
                 Pt1[0] * Pt4[1] * Pt3[2] +
                 Pt1[0] * Pt4[1] * Pt2[2] -
                 Pt1[0] * Pt3[1] * Pt2[2] +
                 Pt3[0] * Pt2[1] * Pt4[2] -
                 Pt3[0] * Pt4[1] * Pt2[2] -
                 Pt3[0] * Pt1[1] * Pt4[2] +
                 Pt3[0] * Pt4[1] * Pt1[2] +
                 Pt2[0] * Pt1[1] * Pt4[2] -
                 Pt2[0] * Pt1[1] * Pt3[2] -
                 Pt2[0] * Pt4[1] * Pt1[2] +
                 Pt2[0] * Pt3[1] * Pt1[2] -
                 Pt4[0] * Pt2[1] * Pt3[2] +
                 Pt4[0] * Pt3[1] * Pt2[2] +
                 Pt4[0] * Pt1[1] * Pt3[2] -
                 Pt4[0] * Pt3[1] * Pt1[2] -
                 Pt4[0] * Pt1[1] * Pt2[2] +
                 Pt4[0] * Pt2[1] * Pt1[2] +
                 Pt3[0] * Pt1[1] * Pt2[2] -
                 Pt3[0] * Pt2[1] * Pt1[2];

    Plane[4] = -(Plane[0] * Pt1[0] + Plane[1] * Pt1[1] +
		 Plane[2] * Pt1[2] + Plane[3] * Pt1[3]);

    return IRIT_FABS(Plane[0]) > IRIT_EPS ||
	   IRIT_FABS(Plane[1]) > IRIT_EPS ||
	   IRIT_FABS(Plane[2]) > IRIT_EPS ||
	   IRIT_FABS(Plane[3]) > IRIT_EPS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a vector in R^4 that is perpendicular to the given three vectors. M
*									     M
*  with(linalg);							     V
*  readlib(C);								     V
*  									     V
*  d := det( matrix( [ [I, J, K, L],					     V
*  		     [A[0], A[1], A[2], A[3]],				     V
*                    [B[0], B[1], B[2], B[3]],				     V
*                    [C[0], C[1], C[2], C[3]] ] ) );			     V
*  coeff( d, I );							     V
*  coeff( d, J );							     V
*  coeff( d, K );							     V
*  coeff( d, L );							     V
*									     *
* PARAMETERS:                                                                M
*   A, B, C:   The three vectors to compute their cross product.             M
*   Res:       Where the output goes into.                 		     M
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivVectCross3Vecs, cross product                                        M
*****************************************************************************/
void TrivVectCross3Vecs(const TrivVType A,
			const TrivVType B,
			const TrivVType C,
			TrivVType Res)
{
    Res[0] = A[1] * B[2] * C[3] - A[1] * B[3] * C[2]
           - B[1] * A[2] * C[3] + B[1] * A[3] * C[2]
           + C[1] * A[2] * B[3] - C[1] * A[3] * B[2];
    Res[1] = - A[0] * B[2] * C[3] + A[0] * B[3] * C[2]
             + B[0] * A[2] * C[3] - B[0] * A[3] * C[2]
             - C[0] * A[2] * B[3] + C[0] * A[3] * B[2];
    Res[2] = A[0] * B[1] * C[3] - A[0] * C[1] * B[3]
           - B[0] * A[1] * C[3] + B[0] * C[1] * A[3]
           + C[0] * A[1] * B[3] - C[0] * B[1] * A[3];
    Res[3] = - A[0] * B[1] * C[2] + A[0] * C[1] * B[2]
             + B[0] * A[1] * C[2] - B[0] * C[1] * A[2]
             - C[0] * A[1] * B[2] + C[0] * B[1] * A[2];
}

#ifdef TEST_PLANE_4D
void main(void)
{
    int i, j;
    TrivIrtPlnType Plane;
    TrivPType
	Pt1 = { 1, 0, 0, 0 },
	Pt2 = { 0, 1, 0, 0 },
	Pt3 = { 0, 0, 1, 0 },
	Pt4 = { 0, 0, 0, 1 };

    TrivPlaneFrom4Points(Pt1, Pt2, Pt3, Pt4, Plane);
    printf("[%lf %lf %lf %lf %lf]\n",
	   Plane[0], Plane[1], Plane[2], Plane[3], Plane[4]);

    TrivPlaneFrom4Points(Pt1, Pt1, Pt3, Pt4, Plane);
    printf("[%lf %lf %lf %lf %lf]\n",
	   Plane[0], Plane[1], Plane[2], Plane[3], Plane[4]);

    TrivPlaneFrom4Points(Pt1, Pt1, Pt1, Pt4, Plane);
    printf("[%lf %lf %lf %lf %lf]\n",
	   Plane[0], Plane[1], Plane[2], Plane[3], Plane[4]);

    TrivPlaneFrom4Points(Pt1, Pt1, Pt1, Pt1, Plane);
    printf("[%lf %lf %lf %lf %lf]\n",
	   Plane[0], Plane[1], Plane[2], Plane[3], Plane[4]);

    for (i = 0; i < 100; i++) {
	for (j = 0; j < 4; j++) {
	    Pt1[j] = IritRandom(-10, 10);
	    Pt2[j] = IritRandom(-10, 10);
	    Pt3[j] = IritRandom(-10, 10);
	    Pt4[j] = IritRandom(-10, 10);
	}

	TrivPlaneFrom4Points(Pt1, Pt2, Pt3, Pt4, Plane);
	if (!IRIT_APX_EQ(-Plane[4], (Plane[0] * Pt1[0] + Plane[1] * Pt1[1] +
			        Plane[2] * Pt1[2] + Plane[3] * Pt1[3])) ||
	    !IRIT_APX_EQ(-Plane[4], (Plane[0] * Pt2[0] + Plane[1] * Pt2[1] +
			        Plane[2] * Pt2[2] + Plane[3] * Pt2[3])) ||
	    !IRIT_APX_EQ(-Plane[4], (Plane[0] * Pt3[0] + Plane[1] * Pt3[1] +
			        Plane[2] * Pt3[2] + Plane[3] * Pt3[3])) ||
	    !IRIT_APX_EQ(-Plane[4], (Plane[0] * Pt4[0] + Plane[1] * Pt4[1] +
			        Plane[2] * Pt4[2] + Plane[3] * Pt4[3])))
	    printf("Error (%d) [%lf %lf %lf %lf %lf]\n", i,
		   Plane[0], Plane[1], Plane[2], Plane[3], Plane[4]);
    }
}

#endif /* TEST_PLANE_4D */

