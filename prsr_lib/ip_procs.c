/*****************************************************************************
* Generic freeform curves and surface processing function for irit parser.   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, April 1993  *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "attribut.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default processor of read freeform geometry.                               M
*   This routine does not process the freeform geometry in any way.          M
*   Other programs can, for example, convert the freeform shapes to polygons M
* or polylines using the call back function or purge the freeform data if    M
* it is not supported.  Processing should be done in place.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:  Freeform geometry to process, in place.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Processed freeform geometry. This function simply    M
*                       returns what it got.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessFreeForm, conversion                                            M
*****************************************************************************/
IPObjectStruct *IPProcessFreeForm(IPFreeFormStruct *FreeForms)
{
    return IPConcatFreeForm(FreeForms);
}
