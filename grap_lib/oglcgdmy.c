/*****************************************************************************
*  Dummy Open GL CG (programmable hardware) drawing functions for safe link. *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Eran Karpen & Sagi Schein	     Ver 0.1, January 2005.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"

/* Dummy functions to link to if no Open GL CG graphics is available. */
int IGCGDrawDTexture(IPObjectStruct *PObj) { return FALSE; }
void IGCGFreeDTexture(IPObjectStruct *PObj) {}
