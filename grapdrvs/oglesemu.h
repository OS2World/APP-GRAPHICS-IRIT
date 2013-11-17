/*****************************************************************************
*   Generic Open GL emulations for Open GL ES.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#ifndef OGL_ES_EMU
#define OGL_ES_EMU

#define GL_POLYGON	GL_TRIANGLE_FAN

void glOrtho(double left,
	     double right,
	     double bottom,
	     double top,
	     double zNear,
	     double zFar);
void glDrawBuffer(int mode);
void glAccum(int op, float value);

void glColor3ub(unsigned char Red, unsigned char Green, unsigned char Blue);
void glColor3d(double Red, double Green, double Blue);
void glColor3f(float Red, float Green, float Blue);

void glTranslated(double x, double y, double z);

void glVertex2d(double x, double y);
void glVertex3d(double x, double y, double z);
void glVertex3dv(double *xyz);
void glNormal3d(double nx, double ny, double nz);
void glTexCoord2f(float s, float t);

void glBegin(int mode);
void glEnd(void);

#endif /* OGL_ES_EMU */
