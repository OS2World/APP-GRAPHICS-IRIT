/*****************************************************************************
*    Set and retrieve properties of the scene context.                       *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "scene.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the view and perspective matrices for the scene.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Scene:     OUT, pointer to the scene.                                    M
*   ViewMat:   IN, the view matrix.                                          M
*   PrspMat:   IN, the perspective matrix, NULL if parallel projection.      M
*   ScrnMat:   IN, the mapping to the screen or NULL if scale [-1,+1] to     M
*	       image size.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SceneSetMatrices, matrix, view, perspective                              M
*****************************************************************************/
void SceneSetMatrices(IRndrSceneStruct *Scene,
                      IrtHmgnMatType ViewMat,
                      IrtHmgnMatType PrspMat,
		      IrtHmgnMatType ScrnMat)
{
    IrtHmgnMatType Mat, InvMat;
    IRndrMatrixContextStruct
        *Matrices = &Scene -> Matrices;

    MatGenUnitMat(Matrices -> TransMat);
    Matrices -> Viewer[RNDR_X_AXIS] = 0;
    Matrices -> Viewer[RNDR_Y_AXIS] = 0;
    Matrices -> Viewer[RNDR_Z_AXIS] = RNDR_VIEWER_SIGHT;

    if (ViewMat)
        MatMultTwo4by4(Matrices -> TransMat, Matrices -> TransMat, ViewMat);
    if (PrspMat) {
        MatMultTwo4by4(Matrices -> TransMat, Matrices -> TransMat, PrspMat);
        Matrices -> Viewer[RNDR_X_AXIS] =
        Matrices -> Viewer[RNDR_Y_AXIS] =
        Matrices -> Viewer[RNDR_Z_AXIS] = 0.0;
        Matrices -> ParallelProjection = 0;
    }
    else {
        Matrices -> ParallelProjection = 1;
    }

    if (ScrnMat) {
	IRIT_HMGN_MAT_COPY(Matrices -> ScreenMat, ScrnMat);
    }
    else {
        int Size = IRIT_MIN(Scene -> SizeX, Scene -> SizeY);

        MatGenMatScale(Size * 0.5, Size * 0.5, 1.0, Mat);
	MatGenMatTrans(Scene -> SizeX * 0.5, Scene -> SizeY * 0.5, 0.0,
		       Matrices -> ScreenMat);
	MatMultTwo4by4(Matrices -> ScreenMat, Mat, Matrices -> ScreenMat);
    }

    if (!MatInverseMatrix(Matrices -> TransMat, InvMat))
        _IRndrReportFatal(IRIT_EXP_STR("Error: non-invertable matrix.\n"));

    MatMultPtby4by4(Matrices -> Viewer, Matrices -> Viewer, InvMat);

    if (Matrices -> ParallelProjection) {
        IRIT_STATIC_DATA IrtPtType
	    Zero = { 0.0, 0.0, 0.0 };

	/* Only vectors need to be adjusted. */
        MatMultPtby4by4(Zero, Zero, InvMat);
        IRIT_PT_SUB(Matrices -> Viewer, Matrices -> Viewer, Zero);
        IRIT_PT_NORMALIZE(Matrices -> Viewer);
    }

    MatMultTwo4by4(Matrices -> ViewMat, Matrices -> TransMat,
		   Matrices -> ScreenMat);

    if (!MatInverseMatrix(Matrices -> ViewMat, Matrices -> ViewInvMat))
        _IRndrReportFatal(IRIT_EXP_STR("Error: non-invertable matrix.\n"));

    Scene -> ZNear = 1;
    Scene -> ZFarValid = 0;

    Scene -> XMax = -(IrtRType) IRIT_MAX_INT;
    Scene -> XMin =  (IrtRType) IRIT_MAX_INT;
    Scene -> YMax = -(IrtRType) IRIT_MAX_INT;
    Scene -> YMin =  (IrtRType) IRIT_MAX_INT;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrives one of the clipping plane defining the view frustrum.           M
*   The planes are built so that inside the view frustum the half planes     M
* are positive (and negative outside).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Scene:   IN, pointer to the scene.                                       M
*   Axis:    IN, the axis normal to the plane(X_AXIS, Y_AXIS, Z_AXIS).       M
*   Min:     IN, whether the "near" or "far" clipping planes.                M
*   Result:  OUT, the result  plane.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SceneGetClippingPlane, clipping planes                                   M
*****************************************************************************/
void SceneGetClippingPlane(IRndrSceneStruct *Scene,
                           int Axis,
                           int Min,
                           IrtPlnType Result)
{
    /* 3 arbitrary positively directed points, whose parallel      */
    /* projection's are not a line.                                */
    IrtPtType4
        p[3] = { { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 } };
    int i;
    IrtRType d;

    if (Axis == RNDR_Z_AXIS && Min && !Scene -> ZFarValid) {
        Result[0] = Result[1] = Result[2] = Result[3] = 0;
        return;
    }
    switch (Axis) {
        case RNDR_X_AXIS:
            p[0][0] = p[1][0] = p[2][0] = Min ? -1.0 : 1.0;
	    break;
        case RNDR_Y_AXIS:
	    p[0][1] = p[1][1] = p[2][1] = Min ? -1.0 : 1.0;
	    break;
        case RNDR_Z_AXIS:
	    p[0][2] = p[1][2] = p[2][2] = Min ? Scene -> ZFar : Scene -> ZNear;
	    break;
    }
    for (i = 0; i < 3; i++)
        MatMultPtby4by4(p[i], p[i], Scene -> Matrices.ViewInvMat);
    GMPlaneFrom3Points(Result, p[0], p[1], p[2]);

    /* Set the plane orientation so inside will be positive. */
    IRIT_PT_RESET(p[0]);
    switch (Axis) {
        case RNDR_X_AXIS:
            p[0][0] = Min ? 1.0 : -1.0;
	    break;
        case RNDR_Y_AXIS:
	    p[0][1] = Min ? 1.0 : -1.0;
	    break;
        case RNDR_Z_AXIS:
	    p[0][2] = Min ? Scene -> ZNear : Scene -> ZFar;
	    break;
    }
    MatMultPtby4by4(p[0], p[0], Scene -> Matrices.ViewInvMat);

    d = IRIT_DOT_PROD(Result, p[0]) + Result[3];
    if ((Scene -> Matrices.ParallelProjection ? d : -d) < 0) {
        for (i = 0; i < 4; i++) {
            Result[i] *= -1;
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Sets the near and far XY clipping planes.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Scene:        IN, pointer to the scene.                                  M
*   ZNear, ZFar:  IN, the near and far z-coordinate of the planes.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SceneSetZClippingPlanes, clipping planes                                 M
*****************************************************************************/
void SceneSetZClippingPlanes(IRndrSceneStruct *Scene,
			     IrtRType ZNear,
			     IrtRType ZFar)
{
    if (ZNear < ZFar) {
         /* Actual measure is in the opposite direction. */
        Scene -> ZNear = -ZNear;
        Scene -> ZFar = -ZFar;
        Scene -> ZFarValid = 1;
    }
    else {
        Scene -> ZNear = -ZNear;
        Scene -> ZFarValid = 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Free the memory of the scene.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Scene:   IN, pointer to the scene.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SceneRelease, memory free, release                                       M
*****************************************************************************/
void SceneRelease(IRndrSceneStruct *Scene)
{
    RNDR_FREE(Scene -> Lights.Src);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Get the scene matrices.	                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Scene:   IN, pointer to the scene.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   struct IRndrMatrixContextStruct *: pointer to matrices struct.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SceneGetMatrices			                                     M
*****************************************************************************/
struct IRndrMatrixContextStruct *SceneGetMatrices(IRndrSceneStruct *Scene)
{
    return &Scene -> Matrices;
}
