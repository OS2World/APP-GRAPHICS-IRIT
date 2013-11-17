/*****************************************************************************
*   Default polyline/gon drawing routine common to graphics drivers.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"

static void IGDfltDrawPoly(IPObjectStruct *PObj);

IRIT_GLOBAL_DATA IGDrawObjectFuncType
    IGDrawPolyFuncPtr = IGDfltDrawPoly;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draw a single Poly object using current modes and transformations.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to draw.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDfltDrawPoly                                                           M
*****************************************************************************/
static void IGDfltDrawPoly(IPObjectStruct *PObj)
{
    fprintf(stderr, "No draw poly function provided.\n");
    assert(0);
}
