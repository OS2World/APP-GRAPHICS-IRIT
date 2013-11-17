/*****************************************************************************
* Grap_loc.h - local header file for the GRAP library.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef GRAP_LOC_H
#define GRAP_LOC_H

#include "grap_lib.h"

IRIT_GLOBAL_DATA_HEADER IGDrawUpdateFuncType
    _IGDrawSrfPolysPreFunc,
    _IGDrawSrfPolysPostFunc,
    _IGDrawSrfWirePreFunc,
    _IGDrawSrfWirePostFunc,
    _IGDrawSketchPreFunc,
    _IGDrawSketchPostFunc,
    _IGDrawCtlMeshPreFunc,
    _IGDrawCtlMeshPostFunc;

#endif /* GRAP_LOC_H */
