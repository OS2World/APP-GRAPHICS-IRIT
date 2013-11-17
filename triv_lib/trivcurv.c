/******************************************************************************
* TrivEval.c - tri-variate function handling routines - evaluation routines.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "triv_loc.h"

#define BILINEAR_FORM(x, M, y) \
	(x[0] * M[0][0] * y[0] + \
	 x[0] * M[0][1] * y[1] + \
	 x[0] * M[0][2] * y[2] + \
	 x[1] * M[1][0] * y[0] + \
	 x[1] * M[1][1] * y[1] + \
	 x[1] * M[1][2] * y[2] + \
	 x[2] * M[2][0] * y[0] + \
	 x[2] * M[2][1] * y[1] + \
	 x[2] * M[2][2] * y[2])

IRIT_STATIC_DATA TrivTVStruct
     *GlblTVGradient[3] = { NULL, NULL, NULL },
     *GlblTVHessian[3][3] = { { NULL, NULL, NULL },
			      { NULL, NULL, NULL },
			      { NULL, NULL, NULL } };
IRIT_STATIC_DATA CagdBType
     HaveGradientHessian = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release all allocated auxiliary trivariate derivatives, for curvature    M
* analysis.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivEvalTVCurvaturePrelude, TrivEvalTVCurvature, TrivEvalHessian,        M
*   TrivEvalGradient							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEvalTVCurvaturePostlude                                              M
*****************************************************************************/
void TrivEvalTVCurvaturePostlude(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        int j;

        if (GlblTVGradient[i] != NULL) {
	    TrivTVFree(GlblTVGradient[i]);
	    GlblTVGradient[i] = NULL;
	}

	for (j = i; j < 3; j++)
	    if (GlblTVHessian[j][i] != NULL) {
	        TrivTVFree(GlblTVHessian[j][i]);
	        GlblTVHessian[j][i] = NULL;
	    }
    }

    HaveGradientHessian = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepare the necessary derivative vector fields of TV for curvature       M
* processing at prescribed locations, later on.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       to process and prepare for further curvature evaluations.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivEvalTVCurvaturePostlude, TrivEvalTVCurvature, TrivEvalHessian,	     M
*   TrivEvalGradient							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEvalTVCurvaturePrelude                                               M
*****************************************************************************/
CagdBType TrivEvalTVCurvaturePrelude(const TrivTVStruct *TV)
{
    /* In case of something from previous evaluation that must freed. */
    TrivEvalTVCurvaturePostlude();

    if (CAGD_NUM_OF_PT_COORD(TV -> PType) != 1) {
        TRIV_FATAL_ERROR(TRIV_ERR_SCALAR_PT_EXPECTED);
        return FALSE;
    }

    HaveGradientHessian =
	 /* Compute first order derivatives (the Gradient) */
        ((GlblTVGradient[0] = TrivTVDerive(TV, TRIV_CONST_U_DIR)) != NULL &&
	 (GlblTVGradient[1] = TrivTVDerive(TV, TRIV_CONST_V_DIR)) != NULL &&
	 (GlblTVGradient[2] = TrivTVDerive(TV, TRIV_CONST_W_DIR)) != NULL);

    HaveGradientHessian = HaveGradientHessian &&
	 /* Compute second order derivatives (the Hessian) */
	((GlblTVHessian[0][0] = TrivTVDerive(GlblTVGradient[0],
					     TRIV_CONST_U_DIR)) != NULL &&
	 (GlblTVHessian[1][0] = TrivTVDerive(GlblTVGradient[1],
					     TRIV_CONST_U_DIR)) != NULL &&
	 (GlblTVHessian[2][0] = TrivTVDerive(GlblTVGradient[2],
					     TRIV_CONST_U_DIR)) != NULL &&
	 (GlblTVHessian[1][1] = TrivTVDerive(GlblTVGradient[1],
					     TRIV_CONST_V_DIR)) != NULL &&
	 (GlblTVHessian[2][1] = TrivTVDerive(GlblTVGradient[2],
					     TRIV_CONST_V_DIR)) != NULL &&
	 (GlblTVHessian[2][2] = TrivTVDerive(GlblTVGradient[2],
					     TRIV_CONST_W_DIR)) != NULL);

    return HaveGradientHessian;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the principal curvatures and principal directions of the       M
* isosurface at location Pos in trivariate that was preprocessed by the      M
* TrivEvalCurvaturePredule function.  Arbitrary number of invokations of     M
* this function are possible once TrivEvalCurvaturePredule is called.  Also  M
* one should invoke TrivEvalCurvaturePostlude once done to release all       M
* auxiliary allocated data structures.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos:	     Location in the parametric space of the trivariate.     M
*   PCurv1, PCurv2:  The two principal curvatures computed by this function. M
*   PDir1, PDir2:    The two principal directions computed by this function. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if successful, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivEvalCurvaturePrelude, TrivEvalCurvaturePostlude, TrivEvalHessian,    M
*   TrivEvalGradient							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEvalCurvature                                                        M
*****************************************************************************/
CagdBType TrivEvalCurvature(CagdPType Pos,
			    CagdRType *PCurv1,
			    CagdRType *PCurv2,
			    CagdVType PDir1,
			    CagdVType PDir2)
{
    int i;
    CagdRType hHh, fHf, hHf, Theta, CosTheta, SinTheta, Gamma, GradientMag,
	Gradient[3], Hessian[3][3];
    CagdVType h, f, V1, V2;

    if (!HaveGradientHessian)
	return FALSE;

    for (i = 0; i < 3; i++) {
        int j;
	CagdRType *R;

	/* Recall the trivariate is a scalar field! */
	R = TrivTVEval(GlblTVGradient[i], Pos[0], Pos[1], Pos[2]);
	Gradient[i] = R[1];

	for (j = i; j < 3; j++) {
	    R = TrivTVEval(GlblTVHessian[j][i], Pos[0], Pos[1], Pos[2]);
	    Hessian[j][i] = Hessian[i][j] = R[1];
	}
    }

    if ((Gamma = sqrt(IRIT_SQR(Gradient[0]) + IRIT_SQR(Gradient[1]))) == 0.0)
	Gamma = IRIT_UEPS;
    if ((GradientMag = IRIT_PT_LENGTH(Gradient)) == 0.0)
	GradientMag = IRIT_UEPS;

    /* A vector perpendicular to the Gradient. */
    h[0] =  Gradient[1] / Gamma;
    h[1] = -Gradient[0] / Gamma;
    h[2] =  0.0;

    /* A vector perpendicualr to both the Gradient and h. */
    f[0] = Gradient[0] * Gradient[2] / (Gamma * GradientMag);
    f[1] = Gradient[1] * Gradient[2] / (Gamma * GradientMag);
    f[2] = -Gamma / GradientMag;

    hHh = BILINEAR_FORM(h, Hessian, h);
    fHf = BILINEAR_FORM(f, Hessian, f);
    hHf = BILINEAR_FORM(h, Hessian, f);

    /* Compute the angle of the first principal curvature. */
    Theta = atan2(2 * hHf, fHf - hHh) * 0.5;
    CosTheta = cos(Theta);
    SinTheta = sin(Theta);
    IRIT_PT_COPY(V1, f);
    IRIT_PT_NORMALIZE(V1);
    IRIT_PT_COPY(V2, h);
    IRIT_PT_NORMALIZE(V2);

    /* Derive the principal directions. */
    for (i = 0; i < 3; i++)
	PDir1[i] = CosTheta * V1[i] + SinTheta * V2[i];
    IRIT_CROSS_PROD(PDir2, PDir1, Gradient);
    IRIT_PT_NORMALIZE(PDir2);

    /* Derive the principal curvatures. */
    *PCurv1 = BILINEAR_FORM(PDir1, Hessian, PDir1) / GradientMag;
    *PCurv2 = BILINEAR_FORM(PDir2, Hessian, PDir2) / GradientMag;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the Gradient of the isosurface at location Pos in trivariate   M
* that was preprocessed by the TrivEvalCurvaturePredule function.  Arbitrary M
* number of invokations of this function are possible once		     M
* TrivEvalCurvaturePredule is called.  Also one should invoke		     M
* TrivEvalCurvaturePostlude once done to release all auxiliary allocated     M
* data structures.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos:       Location in the parametric space of the trivariate.	     M
*   Gradient:  The Gradient computed by this function.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if successful, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivEvalCurvaturePrelude, TrivEvalCurvaturePostlude, TrivEvalCurvature,  M
*   TrivEvalHessian							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEvalGradient                                                         M
*****************************************************************************/
CagdBType TrivEvalGradient(CagdPType Pos, CagdVType Gradient)
{
    int i;

    if (!HaveGradientHessian)
	return FALSE;

    for (i = 0; i < 3; i++) {
	CagdRType *R;

	/* Recall the trivariate is a scalar field! */
	R = TrivTVEval(GlblTVGradient[i], Pos[0], Pos[1], Pos[2]);
	Gradient[i] = R[1];
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the Hessian of the isosurface at location Pos in trivariate    M
* that was preprocessed by the TrivEvalCurvaturePredule function.  Arbitrary M
* number of invokations of this function are possible once		     M
* TrivEvalCurvaturePredule is called.  Also one should invoke		     M
* TrivEvalCurvaturePostlude once done to release all auxiliary allocated     M
* data structures.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos:       Location in the parametric space of the trivariate.	     M
*   Hessian:   The Hessian computed by this function.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:      TRUE if successful, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivEvalCurvaturePrelude, TrivEvalCurvaturePostlude, TrivEvalCurvature,  M
*   TrivEvalHessian							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivEvalHessian                                                          M
*****************************************************************************/
CagdBType TrivEvalHessian(CagdPType Pos, CagdVType Hessian[3])
{
    int i;

    if (!HaveGradientHessian)
	return FALSE;

    for (i = 0; i < 3; i++) {
        int j;

	for (j = i; j < 3; j++) {
	    CagdRType *R;

	    /* Recall the trivariate is a scalar field! */
	    R = TrivTVEval(GlblTVHessian[j][i], Pos[0], Pos[1], Pos[2]);
	    Hessian[j][i] = Hessian[i][j] = R[1];
	}
    }

    return TRUE;
}
