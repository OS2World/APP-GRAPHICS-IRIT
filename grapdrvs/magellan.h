/*****************************************************************************
*   Magellan/SpaceMouse interface support.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Oleg Ilushin				Ver 0.1, Dec 2002.   *
*****************************************************************************/

#ifndef	MAGELLAN_H
#define MAGELLAN_H

#include <windows.h>
#include "irit_sm.h"

#ifdef HAVE_MAGELLAN_LIB
#   include "si.h"
#   include "siapp.h"
#endif /* HAVE_MAGELLAN_LIB */

#define MAGELLAN_SENSITIVITY_FRACTION 2.0

typedef struct MagellanSpaceMouseDataStruct {
#ifdef HAVE_MAGELLAN_LIB
    SiHdl          hdl;
    SiOpenData     oData;
    SiGetEventData eData;
    SiSpwEvent     siEvent;
#endif /* HAVE_MAGELLAN_LIB */

    int      bAllowRotation;	    /* Allow Spaceball rotation elements.    */
    int      bAllowTranslation;	    /* Allow Spaceball translation elements. */
    int      bDominant;		    /* Apply dominant direction filter.      */
    IrtRType Sensitivity;	    /* Affects motion sensitivity.           */
} MagellanSpaceMouseDataStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

int MagellanInitSpaceMouseDevice(MagellanSpaceMouseDataStruct *pData,
				 HWND hWndFrame);
int MagellanSpaceMouseHandleEvent(MagellanSpaceMouseDataStruct *pData,
				  MSG *pMsg);
void MagellanCloseSpaceMouseDevice(MagellanSpaceMouseDataStruct *pData);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* MAGELLAN_H */
