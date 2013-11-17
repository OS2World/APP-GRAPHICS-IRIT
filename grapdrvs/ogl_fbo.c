/*****************************************************************************
*  Open GL FrameBuffer Object (Render To Texture) functions.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, November 2005. *
*****************************************************************************/

#ifndef GLH_EXT_SINGLE_FILE
#define GLH_EXT_SINGLE_FILE
#endif

#ifdef IRIT_HAVE_OGL_CG_LIB

#include <gl/glew.h>

#include <stdio.h>
#include <windows.h>
#include <GL/gl.h>

#include "ogl_fbo.h"

typedef struct FrameBuffer_t {
    int	    SizeX;
    int	    SizeY;
    GLuint  FbId;
    GLuint  Tex0;
    GLuint  DepthId;
};

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Check for FrameBufferObject status and print the error code.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   void:								     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void 								     *
*****************************************************************************/
void CheckFramebufferStatus(void)
{
    GLenum
	Status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    switch (Status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
	    printf("Incomplete attachment\n");
	    break;
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            break;
#ifdef OGL_FBO_OLD_SDK_9_5
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, duplicate attachment\n");
            break;
#endif /* OGL_FBO_OLD_SDK_9_5 */
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            break;
        default:
            break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Create new FrameBuffer Object.					     *
*									     *
* PARAMETERS:                                                                *
*   SizeX:      Width of FBO.				                     *
*   SizeY:      Height of FBO.				                     *
*   Attachment: OpenGl attachment type (can render colors/depth maps).	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   FrameBuffer: If success FrameBuffer will hold a pointer to		     *
*   a new struct FrameBuffer_t. Failure will return NULL.                    *
*****************************************************************************/
FrameBuffer FrameBufferCreate(int SizeX, int SizeY, int Attachment)
{
    FrameBuffer Fb;

#ifdef OGL_FBO_OLD_SDK_9_5
    if (!glh_init_extensions("GL_EXT_framebuffer_object "
			     "GL_ARB_multitexture")) {
        printf("Unable to load the following extension(s): %s\n",
	       glh_get_unsupported_extensions());
	return NULL;
    }
#endif /* OGL_FBO_OLD_SDK_9_5 */

    Fb = malloc(sizeof(struct FrameBuffer_t));

    if (Fb == NULL) {
	return NULL;
    }

    Fb -> SizeX = SizeX;
    Fb -> SizeY = SizeY;

    glGenFramebuffersEXT(1, &(Fb -> FbId)); /* Generate FrameBuffer Id */
    glGenTextures(1, &(Fb -> Tex0)); /* Generate Texture Id */
    glGenRenderbuffersEXT(1, &(Fb -> DepthId));

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fb -> FbId);

    if (Attachment != GL_DEPTH_ATTACHMENT_EXT) {
	/* Initialize Texture. */
	glBindTexture(GL_TEXTURE_2D, Fb -> Tex0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA_FLOAT32_ATI, SizeX, SizeY, 0, 
			GL_RGBA, GL_FLOAT, NULL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, Attachment, 
				    GL_TEXTURE_2D, Fb -> Tex0, 0);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, Fb -> DepthId);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, 
	    GL_DEPTH_COMPONENT24, SizeX, SizeY);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				     GL_DEPTH_ATTACHMENT_EXT, 
				     GL_RENDERBUFFER_EXT, Fb -> DepthId);
    }
    else { /* Depth Texture. */
	/* Initialize Texture. */
	glBindTexture(GL_TEXTURE_2D, Fb -> Tex0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SizeX, SizeY, 0, 
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE); 

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, Attachment, 
				  GL_TEXTURE_2D, Fb -> Tex0, 0);

	glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    CheckFramebufferStatus();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    return Fb;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Destroy existing FrameBuffer Object.				     *
*									     *
* PARAMETERS:                                                                *
*   Fb:		 Existing FrameBuffer Object.		                     *
*   KeepTexture: Whether to keep the texture of FBO in GPU's memory.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void						    		     *
*****************************************************************************/
void FrameBufferDestroy(FrameBuffer *Fb, int KeepTexture)
{
    glDeleteRenderbuffersEXT(1, &(*Fb) -> DepthId);
    glDeleteFramebuffersEXT(1, &(*Fb) -> FbId);
    if (!KeepTexture) {
	glDeleteTextures(1, &(*Fb) -> Tex0);
    }
    free(*Fb);
    *Fb = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Begin to render to FBO.						     *
*									     *
* PARAMETERS:                                                                *
*   Fb: The FBO to render to.				                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   1 for success. 0 otherwise						     *
*****************************************************************************/
int FrameBufferBeginRender(FrameBuffer Fb)
{
    if (Fb == NULL)
	return 0;

    glActiveTextureARB(GL_TEXTURE0_ARB); 
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fb -> FbId);
    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    End render to FBO.							     *
*									     *
* PARAMETERS:                                                                *
*   Fb: The FBO to render to.				                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   1 for success. 0 otherwise						     *
*****************************************************************************/
int FrameBufferEndRender(FrameBuffer Fb)
{
    if (Fb == NULL)
	return 0;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Gets the texture of FBO.						     *
*									     *
* PARAMETERS:                                                                *
*   Fb: The FBO to render to.				                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   OpenGL texture handle if success, -1 for failure.			     *
*****************************************************************************/
GLuint FrameBufferGetTexture(FrameBuffer Fb)
{
    if (Fb == NULL)
	return -1;

    return Fb -> Tex0;
}

#endif /* IRIT_HAVE_OGL_CG_LIB */
