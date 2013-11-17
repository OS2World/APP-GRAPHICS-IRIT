/*****************************************************************************
* Definitions for the Illustrate program:				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef ILLUSTRATE_H
#define ILLUSTRATE_H

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "symb_lib.h"

#define DEFAULT_MAX_LINE_LEN		0.05
#define DEFAULT_TRIM_INTERSECT		0.03

#define WIDEN_END_START			1
#define WIDEN_END_END			2

#define INTER_SAME_Z			0.03

IRIT_GLOBAL_DATA_HEADER IrtRType
    GlblInterSameZ,
    GlblTrimIntersect;
IRIT_GLOBAL_DATA_HEADER int
    GlblAngularDistance,
    GlblOpenPolyData,
    GlblVertexPoints,
    GlblSplitLongLines;

/* Illustrt module prototypes: */
void IllustrateExit(int ExitCode);

/* Intersct module prototypes: */
void ProcessIntersections(IPObjectStruct *PObjects);
IrtRType SegmentLength(IPVertexStruct *V);

/* SpltSort module prototypes: */
void SplitLongLines(IPObjectStruct *PObjects, IrtRType MaxLen);
void RemoveInternalVertices(IPObjectStruct *PObj);
void SortOutput(IPObjectStruct **PObjects);

#endif /* ILLUSTRATE_H */
