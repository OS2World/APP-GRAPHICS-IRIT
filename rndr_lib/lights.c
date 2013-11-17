/*****************************************************************************
* Create list of light sorces initialized for use with rendering.            *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "rndr_loc.h"
#include "color.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates an empty light sources list.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Lights:   OUT, pointer to LightList object which is initialized through. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LightListInitEmpty                                                       M
*****************************************************************************/
void LightListInitEmpty(IRndrLightListStruct *Lights)
{
    Lights -> n = 0;
    Lights -> Src = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new light source to the list.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Lights:   OUT, pointer to LightList object which is initialized through. M
*   NewSrc:   IN, pointer to Light source.                                   M
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    LightListAdd                                                            M
*****************************************************************************/
void LightListAdd(IRndrLightListStruct *Lights, IRndrLightStruct *NewSrc)
{
    int i;
    IRndrLightStruct
        *OldSrc = Lights -> Src;

    Lights -> Src = RNDR_MALLOC(IRndrLightStruct, Lights -> n+1);
    for (i = 0; i < Lights -> n ; i++ ) {
        Lights -> Src[i] = OldSrc[i];
    }
    Lights -> Src[i] = *NewSrc;

    if (NewSrc -> Type == RNDR_VECTOR_LIGHT)
	IRIT_PT_NORMALIZE(Lights -> Src[i].Where);

    Lights -> n++;

    RNDR_FREE(OldSrc);
}
