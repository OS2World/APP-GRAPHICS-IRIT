/*****************************************************************************
*  Header file for Open GL FrameBuffer Object (Render To Texture) functions. *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, November 2005. *
*****************************************************************************/

#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

typedef struct FrameBuffer_t *FrameBuffer;

FrameBuffer FrameBufferCreate(int SizeX, int SizeY, int Attachment);
void FrameBufferDestroy(FrameBuffer *fb, int KeepTexture);
int FrameBufferBeginRender(FrameBuffer fb);
int FrameBufferEndRender(FrameBuffer fb);
GLuint FrameBufferGetTexture(FrameBuffer fb);
void CheckFramebufferStatus();

#endif
