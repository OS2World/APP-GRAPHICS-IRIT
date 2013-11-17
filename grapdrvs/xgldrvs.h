/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Jan. 1992   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	xgldrvs interface.			             *
*****************************************************************************/

#ifndef	XGLDRVS_H	/* Define only once */
#define	XGLDRVS_H

IRIT_GLOBAL_DATA_HEADER long
    ViewWinID,
    ViewWinWidth,
    ViewWinWidth2,
    ViewWinHeight,
    ViewWinHeight2,
    ViewWinLow,
    ViewWinLeft;

void SetViewWindow(int argc, char **argv);

#endif /* XGLDRVS_H */
