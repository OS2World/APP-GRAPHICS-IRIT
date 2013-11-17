/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber		               Ver 0.1, Mar. 1990    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* MSDOS graphical interface for IRIT. Based on intr_lib windowing library.   *
*****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "djggraph.h"
#include "grap_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of djgdrvs - Amiga graphics driver of IRIT.             	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int i;
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;
    IPObjectStruct *PObjects;

    IGConfigureGlobals("djgdrvs", argc, argv);

    IGCreateStateMenu();

    InitIntrLibWindows();

    while ((Event = GetGraphicEvent(ChangeFactor)) != IG_EVENT_QUIT) {
	ChangeFactor[0] *= IGGlblChangeFactor;
	ChangeFactor[1] *= IGGlblChangeFactor;

	if (IGProcessEvent(Event, ChangeFactor))
	    if (IntrWndwIsAllVisible(IGGlblViewWindowID)) {
		IntrIntFunc
		    RefreshFunc = IntrWndwGetRefreshFunc(IGGlblViewWindowID);

		RefreshFunc(IGGlblViewWindowID);
            }
	    else
		IntrWndwRedrawAll();
    }

    CloseIntrLibWindows();

    return 0;
}
