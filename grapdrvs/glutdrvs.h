/*****************************************************************************
* GLut interface functions.						     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 2006    *
*****************************************************************************/

#ifndef GLUT_DRVS_H
#define GLUT_DRVS_H

typedef enum GlutDrvsMouseButtonActive {
    GLUT_DRVS_MOUSE_NO_BUTTON,
    GLUT_DRVS_MOUSE_LEFT_BUTTON,
    GLUT_DRVS_MOUSE_MIDDLE_BUTTON,
    GLUT_DRVS_MOUSE_RIGHT_BUTTON
} GlutDrvsMouseButtonActive;

IRIT_GLOBAL_DATA int
    GlblMouseInitPos[2],
    GlblMouseLastPos[2],
    GlblMouseActiveButton;

#define GLUT_DRVS_DEFAULT_VIEW_WIDTH	512
#define GLUT_DRVS_DEFAULT_VIEW_HEIGHT	512
#define GLUT_DRVS_MAX_CONSOLE_LINES	30

void *IGCaptureWindowPixels(int *x, int *y);

#endif /* GLUT_DRVS_H */
