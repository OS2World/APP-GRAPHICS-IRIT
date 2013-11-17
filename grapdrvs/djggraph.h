/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber		               Ver 0.1, Mar. 1990    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* MSDOS graphical interface for IRIT. Based on intr_lib windowing library.   *
*****************************************************************************/

#ifndef DJGGRAPH_H
#define DJGGRAPH_H
;

#include "grap_loc.h"
#include "intr_lib.h"
#include "intr_gr.h"
#include "grap_loc.h"

IRIT_GLOBAL_DATA_HEADER int
    IGGlblInputWindowID,
    IGGlblViewWindowID,
    IGGlblTransWindowID;

void InitIntrLibWindows(void);
IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor);
void CloseIntrLibWindows(void);

#endif /* DJGGRAPH_H */
