/*****************************************************************************
*  Animation module - call back functions used by display devices - supplied *
* here dummies for use in batch programs.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*							Ver 0.1, Feb. 1995.  *
*****************************************************************************/

#include "irit_sm.h"
#include "grap_lib.h"
#include "geom_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraws the view window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWindow                                                       M
*****************************************************************************/
void IGRedrawViewWindow(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation. Senses the event queue of X11.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we need to abort, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves one iteration of the animation sequence as an image.		     M
*									     *
* PARAMETERS:								     M
*   Anim:	Animation structure.					     M
*   PObjs:	Objects to render.					     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*									     *
* KEYWORDS:								     M
*   GMAnimSaveIterationsAsImages, animation				     M
*****************************************************************************/
void GMAnimSaveIterationsAsImages(GMAnimationStruct *Anim,
				  IPObjectStruct *PObjs)
{
}
