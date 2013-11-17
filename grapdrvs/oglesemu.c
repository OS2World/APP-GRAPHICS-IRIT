/*****************************************************************************
*   Generic Open GL emulations for Open GL ES.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#include <math.h>
#include <windows.h>
#include <GLES/gl.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "oglesemu.h"
#include "wntdrvs.h"

#define OGLES_MAX_ARRAY_SIZE 10000

typedef float OGLESPtType[3];
typedef float OGLESNrmlType[3];
typedef float OGLESUVType[2];

IRIT_STATIC_DATA int
    OGLESPtIndex = 0,
    OGLESNrmlIndex = 0,
    OGLESUVIndex = 0,
    OGLESMode = 0;
IRIT_STATIC_DATA OGLESNrmlType
    *OGLESNrmlArray = NULL;
IRIT_STATIC_DATA OGLESPtType
    *OGLESPtArray = NULL;
IRIT_STATIC_DATA OGLESUVType
    *OGLESUVArray = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   left, right, bottom, top, zNear, zFar:  View Frustum.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glOrtho                                                                  M
*****************************************************************************/
void glOrtho(IrtRType left,
	     IrtRType right,
	     IrtRType bottom,
	     IrtRType top,
	     IrtRType zNear,
	     IrtRType zFar)
{
    glOrthof((float) left, (float) right,
	     (float) bottom, (float) top,
	     (float) zNear, (float) zFar);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   mode: front vs. back.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glDrawBuffer                                                             M
*****************************************************************************/
void glDrawBuffer(int mode)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   op:    N.S.F.I.                                                          M
*   value: N.S.F.I.                                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glAccum                                                                  M
*****************************************************************************/
void glAccum(int op, float value)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Red:   N.S.F.I.                                                          M
*   Green: N.S.F.I.                                                          M
*   Blue:  N.S.F.I.                                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glColor3ub                                                               M
*****************************************************************************/
void glColor3ub(unsigned char Red, unsigned char Green, unsigned char Blue)
{
    glColor4f((float) (Red / 255.0),
	      (float) (Green / 255.0),
	      (float) (Blue / 255.0), 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Red:   N.S.F.I.                                                          M
*   Green: N.S.F.I.                                                          M
*   Blue:  N.S.F.I.                                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glColor3d                                                                M
*****************************************************************************/
void glColor3d(double Red, double Green, double Blue)
{
    glColor4f((float) Red, (float) Green, (float) Blue, 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Red:   N.S.F.I.                                                          M
*   Green: N.S.F.I.                                                          M
*   Blue:  N.S.F.I.                                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glColor3f                                                                M
*****************************************************************************/
void glColor3f(float Red, float Green, float Blue)
{
    glColor4f(Red, Green, Blue, 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   x: N.S.F.I.                                                              M
*   y: N.S.F.I.                                                              M
*   z: N.S.F.I.                                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glTranslated                                                             M
*****************************************************************************/
void glTranslated(double x, double y, double z)
{
    glTranslatef((float) x, (float) y, (float) z);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   x: N.S.F.I.                                                              M
*   y: N.S.F.I.                                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glVertex3d                                                               M
*****************************************************************************/
void glVertex2d(double x, double y)
{
    glVertex3d(x, y, 0.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   x: N.S.F.I.                                                              M
*   y: N.S.F.I.                                                              M
*   z: N.S.F.I.                                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glVertex3d                                                               M
*****************************************************************************/
void glVertex3d(double x, double y, double z)
{
    if (OGLESPtIndex >= OGLES_MAX_ARRAY_SIZE) {
        IGIritError("OGL array too large");
        return;
    }
    OGLESPtArray[OGLESPtIndex][0] = (float) x;
    OGLESPtArray[OGLESPtIndex][1] = (float) y;
    OGLESPtArray[OGLESPtIndex++][2] = (float) z;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   xyz: N.S.F.I.                                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glVertex3d                                                               M
*****************************************************************************/
void glVertex3dv(double *xyz)
{
    if (OGLESPtIndex >= OGLES_MAX_ARRAY_SIZE) {
        IGIritError("OGL array too large");
        return;
    }
    OGLESPtArray[OGLESPtIndex][0] = (float) xyz[0];
    OGLESPtArray[OGLESPtIndex][1] = (float) xyz[1];
    OGLESPtArray[OGLESPtIndex++][2] = (float) xyz[2];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   nx: N.S.F.I.                                                             M
*   ny: N.S.F.I.                                                             M
*   nz: N.S.F.I.                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glNormal3d                                                               M
*****************************************************************************/
void glNormal3d(double nx, double ny, double nz)
{
    if (OGLESNrmlIndex >= OGLES_MAX_ARRAY_SIZE) {
        IGIritError("OGL array too large");
        return;
    }
    OGLESNrmlArray[OGLESNrmlIndex][0] = (float) nx;
    OGLESNrmlArray[OGLESNrmlIndex][1] = (float) ny;
    OGLESNrmlArray[OGLESNrmlIndex++][2] = (float) nz;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   s: N.S.F.I.                                                              M
*   t: N.S.F.I.                                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glTexCoord2f                                                             M
*****************************************************************************/
void glTexCoord2f(float s, float t)
{
    if (OGLESUVIndex >= OGLES_MAX_ARRAY_SIZE) {
        IGIritError("OGL array too large");
        return;
    }
    OGLESUVArray[OGLESUVIndex][0] = s;
    OGLESUVArray[OGLESUVIndex++][1] = t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Mode: N.S.F.I.                                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glBegin                                                                  M
*****************************************************************************/
void glBegin(int Mode)
{
    /* Allocate the arrays if not allocated yet. */
    if (OGLESPtArray == NULL) {
        OGLESPtArray = (OGLESPtType *)
	    IritMalloc(sizeof (OGLESPtType) * OGLES_MAX_ARRAY_SIZE);
	OGLESNrmlArray = (OGLESNrmlType *)
	    IritMalloc(sizeof (OGLESNrmlType) * OGLES_MAX_ARRAY_SIZE);
	OGLESUVArray = (OGLESUVType *)
	    IritMalloc(sizeof (OGLESUVType) * OGLES_MAX_ARRAY_SIZE);
    };

    OGLESPtIndex = OGLESNrmlIndex = OGLESUVIndex = 0;
    OGLESMode = Mode;
}
 
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Emulator function for Open GL ES.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glEnd                                                                    M
*****************************************************************************/
void glEnd(void)
{
    if (OGLESPtIndex == 0)
        return;

    glVertexPointer(3, GL_FLOAT, 0, OGLESPtArray);
    if (OGLESNrmlIndex == OGLESPtIndex)
        glNormalPointer(GL_FLOAT, 0, OGLESNrmlArray);
    if (OGLESUVIndex == OGLESPtIndex)
        glTexCoordPointer(2, GL_FLOAT, 0, OGLESUVArray);

    glEnableClientState(GL_VERTEX_ARRAY);
    if (OGLESNrmlIndex == OGLESPtIndex)
	glEnableClientState(GL_NORMAL_ARRAY);
    if (OGLESUVIndex == OGLESPtIndex)
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glDrawArrays(OGLESMode, 0, OGLESPtIndex);

    glDisableClientState(GL_VERTEX_ARRAY);
    if (OGLESNrmlIndex == OGLESPtIndex)
	glDisableClientState(GL_NORMAL_ARRAY);
    if (OGLESUVIndex == OGLESPtIndex)
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    OGLESPtIndex = OGLESNrmlIndex = OGLESUVIndex = 0;
}
