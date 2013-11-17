/******************************************************************************
* Quatrnn.c - Generic quaternion library.                                     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Moshe Zur, Kesem Korakin and Eran Yariv,    Ver 0.1, Sep. 1997.  *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"
#include "extra_fn.h"

static void QuatFindAtan2(IrtRType a, IrtRType b, IrtRType Res[2]);
static void QuatFindAsin(IrtRType a, IrtRType Res[2]);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms a quaternion to a matrix.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   q:         Source quaternion.                                            M
*   Mat:       Destination matrix.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatToQuat, GMQuatNormalize                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatToMat, Quaternion                                                  M
*****************************************************************************/
void GMQuatToMat(GMQuatType q, IrtHmgnMatType Mat)
{
    IrtRType s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;

    s = 2.0 / (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    xs = q[0] * s;
    ys = q[1] * s;
    zs = q[2] * s;
    wx = q[3] * xs;
    wy = q[3] * ys;
    wz = q[3] * zs;
    xx = q[0] * xs;
    xy = q[0] * ys;
    xz = q[0] * zs;
    yy = q[1] * ys;
    yz = q[1] * zs;
    zz = q[2] * zs;

    Mat[0][0] = 1.0 - (yy + zz);
    Mat[0][1] = xy + wz;
    Mat[0][2] = xz - wy;

    Mat[1][0] = xy - wz;
    Mat[1][1] = 1.0 - (xx + zz);
    Mat[1][2] = yz + wx;

    Mat[2][0] = xz + wy;
    Mat[2][1] = yz - wx;
    Mat[2][2] = 1.0 - (xx + yy);

    Mat[0][3] = Mat[1][3] = Mat[2][3] = Mat[3][0] = Mat[3][1] = Mat[3][2] = 0.0;
    Mat[3][3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms a matrix to a quaternion.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Source matrix.                                                M
*   q:         Destination quaternion.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatToMat                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMatToQuat, Quaternion                                              M
*****************************************************************************/
void GMQuatMatToQuat(IrtHmgnMatType Mat, GMQuatType q)
{
    IrtRType s,
	Tr = Mat[0][0] + Mat[1][1] + Mat[2][2];

    if (Tr > 0.0) {
        s = sqrt(Tr + 1.0);
        q[3] = s * 0.5;
        s = 0.5 / s;

        q[0] = (Mat[1][2] - Mat[2][1]) * s;
        q[1] = (Mat[2][0] - Mat[0][2]) * s;
        q[2] = (Mat[0][1] - Mat[1][0]) * s;
    }
    else {
	IRIT_STATIC_DATA int
	    Nxt[3] = { 1, 2, 0 };
        int i, j, k;

        i = 0;
        if (Mat[1][1] > Mat[0][0])
            i = 1;
        if (Mat[2][2] > Mat[i][i])
            i = 2;
        j = Nxt[i];
        k = Nxt[j];
        s = sqrt((Mat[i][i] - (Mat[j][j] + Mat[k][k])) + 1.0);

        q[i] = s * 0.5;
        s = 0.5 / s;
        q[3] = (Mat[j][k] - Mat[k][j]) * s;
        q[j] = (Mat[i][j] + Mat[j][i]) * s;
        q[k] = (Mat[i][k] + Mat[k][i]) * s;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a quaternion from an arbitrary rotation in X-Y-Z order.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Xangle:    Roation angle around X axis                                   M
*   Yangle:    Roation angle around Y axis                                   M
*   Zangle:    Roation angle around Z axis                                   M
*   q:         Destination quaternion.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatToMat, GMQuatToRotation                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatRotationToQuat, Quaternion                                         M
*****************************************************************************/
void GMQuatRotationToQuat(IrtRType Xangle,
			  IrtRType Yangle, 
			  IrtRType Zangle,
			  GMQuatType q)
{
    GMQuatTransVecType Tv;
    IrtHmgnMatType ResMat;

    Tv[GM_QUAT_ROT_X] = Xangle;
    Tv[GM_QUAT_ROT_Y] = Yangle;
    Tv[GM_QUAT_ROT_Z] = Zangle;

    GMQuatVecToRotMatrix(Tv, ResMat);
    GMQuatMatToQuat(ResMat, q);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds rotation angles in X-Y-Z order from a given                          M
* quaternion representation.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   q:            Rotation quaternion.                                       M
*   Angles:       All possible rotation angles (up to 8).                    M
*   NumSolutions: Pointer to buffer that holds number of solutions found.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatToMat, GMQuatRotationToQuat                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatToRotation, Quaternion                                             M
*****************************************************************************/
void GMQuatToRotation(GMQuatType q, IrtVecType *Angles, int *NumSolutions)
{
    IrtHmgnMatType Mat;

    GMQuatToMat(q, Mat);
    *NumSolutions = GMQuatMatrixToAngles(Mat, Angles);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Mutliplies two quatenrions.                                                M
* Order of arguments counts.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   q1:        Left quaternion.                                              M
*   q2:        Right quaternion.                                             M
*   QRes:      Result quaternion.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatToQuat                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMul, Quaternion                                                    M
*****************************************************************************/
void GMQuatMul(GMQuatType q1, GMQuatType q2, GMQuatType QRes)
{
    GMQuatType Tmp;

    Tmp[3] = q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2];
    Tmp[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
    Tmp[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
    Tmp[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
    GM_QUAT_COPY(Tmp, QRes);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Adds two quatenrions.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   q1:        Left quaternion.                                              M
*   q2:        Right quaternion.                                             M
*   QRes:      Result quaternion.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatToQuat                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatAdd, Quaternion                                                    M
*****************************************************************************/
void GMQuatAdd(GMQuatType q1, GMQuatType q2, GMQuatType QRes)
{
    int i;
    GMQuatType Tmp;

    for (i = 0; i <= 3; i++)
        Tmp[i] = q1[i] + q2[i];
    GM_QUAT_COPY(Tmp, QRes);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Checks if a given quaternion is of unit magnitude.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   q:         Tested quaternion.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Non-zero if TRUE.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatToQuat, GMQuatNormalize                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatIsUnitQuat, Quaternion                                             M
*****************************************************************************/
int GMQuatIsUnitQuat(GMQuatType q)
{
    return IRIT_APX_EQ_EPS((q[3] * q[3] + q[0] * q[0] + q[1] * q[1] + q[2] * q[2]),
		      1.0, IRIT_EPS);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Normalizes a quaternion into a unit size quaternion (as a 4 vector)        M
*                                                                            *
* PARAMETERS:                                                                M
*   q:         quaternion.                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatIsUnitQuat, GMQuatInverse, GMQuatIsUnitQuat                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatNormalize, Quaternion                                              M
*****************************************************************************/
void GMQuatNormalize(GMQuatType q)
{
    int i;
    IrtRType s;

    if (GMQuatIsUnitQuat(q))
        return;

    s = q[3] * q[3] + q[0] * q[0] + q[1] * q[1] + q[2] * q[2];
    if (IRIT_APX_EQ_EPS(s, 0.0, IRIT_EPS)) {
        q[0] = q[1] = q[2] = 0.0;
        q[3] = 1.0;
        return;
    }
    s = 1.0 / sqrt(s);
    for (i = 0; i <= 3; i++)
        q[i] *= s;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates q^(-1) from a quaternion.                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   SrcQ:        Source quaternion.                                          M
*   DstQ:        Destination inversed quaternion.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatNormalize, GMQuatMatToQuat, GMQuatIsUnitQuat                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatInverse, Quaternion                                                M
*****************************************************************************/
void GMQuatInverse(GMQuatType SrcQ, GMQuatType DstQ)
{
    IrtRType
	NormSqr = IRIT_SQR(SrcQ[3]) + IRIT_SQR(SrcQ[0]) +
		  IRIT_SQR(SrcQ[1]) + IRIT_SQR(SrcQ[2]);

    DstQ[3] =  SrcQ[3] * NormSqr;
    DstQ[0] = -SrcQ[0] * NormSqr;
    DstQ[1] = -SrcQ[1] * NormSqr;
    DstQ[2] = -SrcQ[2] * NormSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Rotates a vector using a rotation quaternion.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigVec:     Original (source) vector.                                   M
*   RotQ:        Rotation quaternion.                                        M
*   DestVec:     Detination (rotated) vector.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatToQuat                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatRotateVec, Quaternion                                              M
*****************************************************************************/
void GMQuatRotateVec(IrtVecType OrigVec, GMQuatType RotQ, IrtVecType DestVec)
{
    GMQuatType ResQ, InvQ, VecQ;

    VecQ[0] = OrigVec[0];
    VecQ[1] = OrigVec[1];
    VecQ[2] = OrigVec[2];
    VecQ[3] = 0.0;

    GMQuatMul(RotQ, VecQ, ResQ);                      /* ResQ = RotQ * VecQ. */
    GMQuatInverse(RotQ, InvQ);                        /* InvQ = RotQ ^ (-1). */

    /* VecQ = ResQ * InvQ or in other words: VecQ = RotQ * VecQ * RotQ^(-1). */
    GMQuatMul(ResQ, InvQ, VecQ);
    
    DestVec[0] = VecQ[0];
    DestVec[1] = VecQ[1];
    DestVec[2] = VecQ[2];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Calculates the logarithm of a quaternion.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   SrcQ:        Source quaternion.                                          M
*   DstVec:      Detination (result) vector.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatExp                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatLog, Quaternion                                                    M
*****************************************************************************/
void GMQuatLog(GMQuatType SrcQ, IrtVecType DstVec)
{
    if (SrcQ[3] == 0.0) {
        DstVec[0] = M_PI_DIV_2 * SrcQ[0];
        DstVec[1] = M_PI_DIV_2 * SrcQ[1];
        DstVec[2] = M_PI_DIV_2 * SrcQ[2];
    } else {
        IrtRType
	    NormU = sqrt(IRIT_SQR(SrcQ[0]) +
			 IRIT_SQR(SrcQ[1]) +
			 IRIT_SQR(SrcQ[2])),
	    a = atan2(NormU, SrcQ[3]);

        DstVec[0] = a * SrcQ[0] / NormU;
        DstVec[1] = a * SrcQ[1] / NormU;
        DstVec[2] = a * SrcQ[2] / NormU;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Calculates the exponent quaternion of a vector.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   SrcVec:      Source vector.                                              M
*   DstQ:        Detination (result) quaternion.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatLog                                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatExp, Quaternion                                                    M
*****************************************************************************/
void GMQuatExp(IrtVecType SrcVec, GMQuatType DstQ)
{
    if (SrcVec[0] == 0.0 && SrcVec[1] == 0.0 && SrcVec[2] == 0.0) {
        DstQ[3] = 1.0;
        DstQ[0] = DstQ[1] = DstQ[2] = 0.0;
    } else {
        IrtRType
	    NormU = sqrt(IRIT_SQR(SrcVec[0]) +
			 IRIT_SQR(SrcVec[1]) +
			 IRIT_SQR(SrcVec[2])),
	    a = sin(NormU) / NormU;

        DstQ[3] = cos(NormU);
        DstQ[0] = a * SrcVec[0];
        DstQ[1] = a * SrcVec[1];
        DstQ[2] = a * SrcVec[2];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Calculates the quaternion to the power of a real exponenet.                M
*                                                                            *
* PARAMETERS:                                                                M
*   MantisQ:     Mantisa quaternion.                                         M
*   Expon:       Real exponent.                                              M
*   DstQ:        Detination (result) quaternion.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatLog, GMQuatExp                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatPow, Quaternion                                                    M
*****************************************************************************/
void GMQuatPow(GMQuatType MantisQ, IrtRType Expon, GMQuatType DstQ)
{
    int i;
    IrtVecType TmpV;

    GMQuatLog(MantisQ, TmpV);
    for (i = 0; i <= 2; i++)
        TmpV[i] *= Expon;
    GMQuatExp(TmpV, DstQ);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds all possible arc-tangent of (a/b) results between -PI to +PI.      *
*                                                                            *
* PARAMETERS:                                                                *
*   a:        Divisor.                                                       *
*   b:        Divider.                                                       *
*   Res:      Buffer of possible results (up to 2).                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void QuatFindAtan2(IrtRType a, IrtRType b, IrtRType Res[2])
{
    IrtRType t;

    if (a == 0.0) {
        Res[0] = 0.0;
        Res[1] = M_PI;
        return;
    }
    if (b == 0.0) {
        Res[0] = M_PI_DIV_2;
        Res[1] = -M_PI_DIV_2;
        return;
    }
    t = a / b;
    Res[0] = atan(t);
    if (Res[0] > 0.0)
        Res[1] = Res[0] - M_PI;
    else
        Res[1] = Res[0] + M_PI;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds all possible arc-sine results between -PI to +PI.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   a:        Sine of angle.                                                 *
*   Res:      Buffer of possible results (up to 2).                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void QuatFindAsin(IrtRType a, IrtRType Res[2])
{
    if (a < -1.0 || a > 1.0) {
        Res[0] = Res[1] = IRIT_INFNTY;                        /* Bad Result. */
        return;
    }
    Res[0] = asin(a);
    if (Res[0] >= 0.0)
        Res[1] = Res[0] + 2.0 * (M_PI_DIV_2 - Res[0]);
    else
        Res[1] = Res[0] - 2.0 * (Res[0] + M_PI_DIV_2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Calculates the angle of rotation in each axis, from a given                M
* rotation matrix.                                                           M
*                                                                            M
* The rotation, being axis-dependant, must be performed in a predefined      M
* order: rotate by X, then by Y and finally by Z.                            M
*                                                                            M
* A rotation angle in the X-Y-Z order looks like this:                       M
*                                                                            M
*          c2*c3                c2*s3                 -s2             0      V
*          s1*s2*c3 - c1*s3     s1*s2*s3 + c1*c3      s1*c2           0      V
*          c1*s2*c3 + s1*s3     c1*s2*s3 - s1*c3      c1*c2           0      V
*          0                    0                     0               1      V
*                                                                            V
*  where c1 = cos x, c2 = cos y, c3 = cos z                                  V
*        s1 = sin x, s2 = sin y, s3 = sin z                                  V
*                                                                            V
*  This is compared to our matrix:                                           V
*          a                    b                     c               0      V
*          d                    e                     f               0      V
*          g                    h                     i               0      V
*          0                    0                     0               1      V
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Source roation matrix.                                        M
*   Vec:       Destination angles vectors (up to 8).                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       The number of possible solutions (0 = no solution).           M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatrixToScale, GMQuatMatrixToTranslation                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMatrixToAngles                                                     M
*****************************************************************************/
int GMQuatMatrixToAngles(IrtHmgnMatType Mat, IrtVecType *Vec)
{
    int xi, yi, zi,
        NumSolutions = 0;
    IrtRType c1, c2, c3, s1, s2, s3, x[2], y[2], z[2],
	a = Mat[0][0],
	b = Mat[0][1],
	c = Mat[0][2],
	d = Mat[1][0],
	e = Mat[1][1],
	f = Mat[1][2],
	g = Mat[2][0],
	h = Mat[2][1],
	i = Mat[2][2];

    if (Mat[0][3] != 0.0 || Mat[1][3] != 0.0 || Mat[2][3] != 0.0 ||
        Mat[3][0] != 0.0 || Mat[3][1] != 0.0 || Mat[3][2] != 0.0 ||
        Mat[3][3] != 1.0 || IRIT_FABS(c) > 1.0)
        return FALSE;                              /* Not a rotation Matrix. */

    QuatFindAtan2(b, a, z);
    QuatFindAtan2(f, i, x);
    QuatFindAsin(-c, y);
    for (xi = 0; xi <= 1; xi++) {
        for (yi = 0; yi <= 1; yi++) {
            for (zi = 0; zi <= 1; zi++) {
                c1 = cos(x[xi]);
                s1 = sin(x[xi]);
                c2 = cos(y[yi]);
                s2 = sin(y[yi]);
                c3 = cos(z[zi]);
                s3 = sin(z[zi]);
                if (IRIT_APX_EQ_EPS(c2 * c3, a, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(c2 * s3, b, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(s1 * s2 * c3 - c1 * s3, d, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(s1 * s2 * s3 + c1 * c3, e, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(c2 * s1, f, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(c1 * s2 * c3 + s1 * s3, g, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(c1 * s2 * s3 - s1 * c3, h, IRIT_EPS) &&
                    IRIT_APX_EQ_EPS(c2 * c1, i, IRIT_EPS)) {
		    Vec[NumSolutions][0] = x[xi];
		    Vec[NumSolutions][1] = y[yi];
		    Vec[NumSolutions][2] = z[zi];
		    NumSolutions++;
		}
            }
	}
    }

    return NumSolutions;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract the translation vector from a given transformation matrix.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Source transformation matrix.                                 M
*   Vec:       Destination translation vector.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatrixToScale, GMQuatRotMatrixToAngles                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMatrixToTranslation, Transformation, Quaternion                    M
*****************************************************************************/
void GMQuatMatrixToTranslation(IrtHmgnMatType Mat, IrtVecType Vec)
{
    Vec[0] = Mat[3][0];
    Vec[1] = Mat[3][1];
    Vec[2] = Mat[3][2];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract the uniform scale factor from a given transformation matrix.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Source transformation matrix.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The uniform scale factor result.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatrixToAngles, GMQuatMatrixToTranslation                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMatrixToScale                                                      M
*****************************************************************************/
IrtRType GMQuatMatrixToScale(IrtHmgnMatType Mat)
{
    IrtRType
	DetRes = MatDeterminantMatrix(Mat);

    /* Returns determinant to the power of 1/3 (3rd root of determinant). */
    return pow(DetRes, 1.0 / 3.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract the transformation parameters vector                               M
* from a given transformation matrix.                                        M
*                                                                            M
* A transformation vector contains the rotation angles in all 3 axis,        M
* the translation in all 3 axis and a uniform scaling factor.                M
*                                                                            M
* Since there are many ways to combine these parameters, we chose            M
* the following order: To create a transformation matrix out of a            M
* transformation vector - first create a rotation matrix (rotate by X then   M
* by Y and then by Z), then create a uniform scaling matrix and finally      M
* create a translation matrix.                                               M
*                                                                            M
* Apply roation, then scale and finally translation to obtain the original   M
* transformation matrix.                                                     M
*                                                                            M
* To create a transformation vector out of a transformation matrix, we       M
* simply do it all in reverse : extract and cancel transformation effects,   M
* extract and cancel scaling effects and then extract roation effects.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Source transformation matrix.                                 M
*   TransVec:  The result transformation parameters vector.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE only if the input matrix is a transformation matrix.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatMatrixToScale, GMQuatMatrixToTranslation, GMQuatMatrixToAngles     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatMatrixToVector, Transformation, Quaternion                         M
*****************************************************************************/
int GMQuatMatrixToVector(IrtHmgnMatType Mat, GMQuatTransVecType TransVec)
{
    int i, j;
    IrtRType ScaleFactor;
    IrtVecType Translation, RotVec[8];
    IrtHmgnMatType TmpMat;

    /* Make a copy of the original Matrix. */
    IRIT_HMGN_MAT_COPY(TmpMat, Mat);
    GMQuatMatrixToTranslation(TmpMat, Translation);      /* Get translation. */

    /* Cancel translation effect and get uniform scale factor next. */
    TmpMat[3][0] = TmpMat[3][1] = TmpMat[3][2] = 0.0;
    ScaleFactor = GMQuatMatrixToScale(TmpMat);
    if (ScaleFactor <= 0.0)
        return FALSE;

    /* Cancel scaling effect. */
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            TmpMat[i][j] /= ScaleFactor;

    if (GMQuatMatrixToAngles(TmpMat, RotVec) == 0)        /* Get rot angles. */
        return FALSE;

    /* Copy all values to transformation vector. */
    TransVec[GM_QUAT_ROT_X] = RotVec[0][0];
    TransVec[GM_QUAT_ROT_Y] = RotVec[0][1];
    TransVec[GM_QUAT_ROT_Z] = RotVec[0][2];
    TransVec[GM_QUAT_TRANS_X] = Translation[0];
    TransVec[GM_QUAT_TRANS_Y] = Translation[1];
    TransVec[GM_QUAT_TRANS_Z] = Translation[2];
    TransVec[GM_QUAT_SCALE] = ScaleFactor;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a transformation parameters vector to a transformation matrix.    M
*                                                                            M
* A transformation vector contains the rotation angles in all 3 axis,        M
* the translation in all 3 axis and a uniform scaling factor.                M
*                                                                            M
* Since there are many ways to combine these parameters, we chose            M
* the following order: To create a transformation matrix out of a            M
* transformation vector - first create a rotation matrix (rotate by X, then  M
* by Y, and then by Z), then create a uniform scaling matrix and finally     M
* create a translation matrix.                                               M
*                                                                            M
* Apply rotation, then scale and finally translation to obtain the original  M
* transformation matrix.                                                     M
*                                                                            M
* To create a transformation vector out of a transformation matrix, we       M
* simply do it all in reverse: extract and cancel transformation effects,    M
* extract and cancel scaling effects and then extract rotation effects.      M
*                                                                            *
* PARAMETERS:                                                                M
*   TransVec:  The source transformation parameters vector.                  M
*   Mat:       The result transformation matrix.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatVecToScaleMatrix, GMQuatVecToRotMatrix, GMQuatVecToTransMatrix     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatVectorToMatrix, Transformation, Quaternion                         M
*****************************************************************************/
void GMQuatVectorToMatrix(GMQuatTransVecType TransVec, IrtHmgnMatType Mat)
{
    IrtHmgnMatType ScaleMatrix, RotMatrix, TransMatrix, TmpMat;

    GMQuatVecToScaleMatrix(TransVec, ScaleMatrix);
    GMQuatVecToRotMatrix(TransVec, RotMatrix);
    GMQuatVecToTransMatrix(TransVec, TransMatrix);

    /* Apply rotation, then scaling. */
    MatMultTwo4by4(TmpMat, RotMatrix, ScaleMatrix);
    
    /* Finally, apply translation. */
    MatMultTwo4by4(Mat, TmpMat, TransMatrix);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a scale matrix from a transformation parameters vector.           M
*                                                                            *
* PARAMETERS:                                                                M
*   TransVec:     The source transformation parameters vector.               M
*   ScaleMatrix:  The result scale matrix.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatVecToRotMatrix, GMQuatVecToTransMatrix                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatVecToScaleMatrix, Transformation, Quaternion                       M
*****************************************************************************/
void GMQuatVecToScaleMatrix(GMQuatTransVecType TransVec, 
			    IrtHmgnMatType ScaleMatrix)
{
    MatGenUnitMat(ScaleMatrix);
    ScaleMatrix[0][0] = TransVec[GM_QUAT_SCALE];
    ScaleMatrix[1][1] = TransVec[GM_QUAT_SCALE];
    ScaleMatrix[2][2] = TransVec[GM_QUAT_SCALE];
    ScaleMatrix[3][3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a rotation matrix from a transformation parameters vector.        M
*   The rotation, being axis-dependant, must be performed in a predefined    M
* order: rotate by X, then by Y and finally by Z.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   TransVec:       The source transformation parameters vector.             M
*   RotMatrix:      The result rotation matrix.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatVecToTransMatrix, GMQuatVecToScaleMatrix                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatVecToRotMatrix, Transformation, Quaternion                         M
*****************************************************************************/
void GMQuatVecToRotMatrix(GMQuatTransVecType TransVec,
			  IrtHmgnMatType RotMatrix)
{
    IrtRType
	CosX = cos(TransVec[GM_QUAT_ROT_X]),
	SinX = sin(TransVec[GM_QUAT_ROT_X]),
	CosY = cos(TransVec[GM_QUAT_ROT_Y]),
	SinY = sin(TransVec[GM_QUAT_ROT_Y]),
	CosZ = cos(TransVec[GM_QUAT_ROT_Z]),
	SinZ = sin(TransVec[GM_QUAT_ROT_Z]);

    MatGenUnitMat(RotMatrix);
    RotMatrix[0][0] = CosY * CosZ;
    RotMatrix[0][1] = CosY * SinZ;
    RotMatrix[0][2] = -SinY;
    RotMatrix[1][0] = SinX * SinY * CosZ - CosX * SinZ;
    RotMatrix[1][1] = SinX * SinY * SinZ + CosX * CosZ;
    RotMatrix[1][2] = SinX * CosY;
    RotMatrix[2][0] = CosX * SinY * CosZ + SinX * SinZ;
    RotMatrix[2][1] = CosX * SinY * SinZ - SinX * CosZ;
    RotMatrix[2][2] = CosX * CosY;
    RotMatrix[3][3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a translation matrix from a transformation parameters vector.     M
*                                                                            *
* PARAMETERS:                                                                M
*   TransVec:           The source transformation parameters vector.         M
*   TransMatrix:        The result translation matrix.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMQuatVecToRotMatrix, GMQuatVecToScaleMatrix                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMQuatVecToTransMatrix, Transformation, Quaternion                       M
*****************************************************************************/
void GMQuatVecToTransMatrix(GMQuatTransVecType TransVec,
			    IrtHmgnMatType TransMatrix)
{
    MatGenUnitMat(TransMatrix);
    TransMatrix[3][0] = TransVec[GM_QUAT_TRANS_X];
    TransMatrix[3][1] = TransVec[GM_QUAT_TRANS_Y];
    TransMatrix[3][2] = TransVec[GM_QUAT_TRANS_Z];
    TransMatrix[3][3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Decompose the affine transformation matrix into uniform scaling, rotation  M
* and translation components. Rotation is defined as normal and angle.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:      Source matrix to decompose.                                    M
*   S:	      Scaling components.					     M
*   R:        Rotation Components.                                           M
*   T:        Translation components.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMatrixToTransform                                                      M
*****************************************************************************/
void GMMatrixToTransform(IrtHmgnMatType Mat, 
			 IrtVecType S,
			 GMQuatType R,
			 IrtVecType T)

{
    IrtHmgnMatType RM;
    IrtRType u[3][3], v[3][3];
    int i, j;

    GMQuatMatrixToTranslation(Mat, T);
    SvdMatrix4x4(Mat, u, S, v);
    IRIT_HMGN_MAT_COPY(RM, Mat);
    for (i = 0; i <= 2; i++)
        for (j = 0; j <= 2; j++)
            RM[i][j] /= S[j];
    GMQuatMatToQuat(RM, R);
}

