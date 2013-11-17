/******************************************************************************
* AUXILIARY:								      *
* This horrible piece of code is an extract from SBUTILS demo of starbase.    *
*   I do not want to nor have any interest into understanding this horror     *
* story. All I want is a starbase window under X11 using HPUX.                *
*   This is an extremely displeasing situation that must be improved by HP.   *
* 	                                  Gershon Elber,      February 1995.  *
******************************************************************************/

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XHPlib.h>
#include <stdio.h>
#include "xsbdrvs.h"
#include "grap_loc.h"

#define STATIC_GRAY	0x01
#define GRAY_SCALE	0x02
#define PSEUDO_COLOR	0x04
#define TRUE_COLOR	0x10
#define DIRECT_COLOR	0x11

STATIC_DATA int
    weCreateServerOverlayVisualsProperty = False;

static void SetServerOverlayVisualsProperty();

/******************************************************************************
* AUXILIARY:								     *
 *
 * GetXVisualInfo()
 *
 * This routine takes an X11 Display, screen number, and returns whether the
 * screen supports transparent overlays and three arrays:
 *
 *	1) All of the XVisualInfo struct's for the screen.
 *	2) All of the OverlayInfo struct's for the screen.
 *	3) An array of pointers to the screen's image plane XVisualInfo
 *	   structs.
 *
 * The code below obtains the array of all the screen's visuals, and obtains
 * the array of all the screen's overlay visual information.  It then processes
 * the array of the screen's visuals, determining whether the visual is an
 * overlay or image visual.
 *
 * If the routine sucessfully obtained the visual information, it returns zero.
 * If the routine didn't obtain the visual information, it returns non-zero.
 *
 ******************************************************************************/

int GetXVisualInfo(display, screen, transparentOverlays,
		   numVisuals, pVisuals,
		   numOverlayVisuals, pOverlayVisuals,
		   numImageVisuals, pImageVisuals)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    int		*transparentOverlays;	/* Non-zero if there's at least one
					 * overlay visual and if at least one
					 * of those supports a transparent
					 * pixel. */
    int		*numVisuals;		/* Number of XVisualInfo struct's
					 * pointed to to by pVisuals. */
    XVisualInfo	**pVisuals;		/* All of the device's visuals. */
    int		*numOverlayVisuals;	/* Number of OverlayInfo's pointed
					 * to by pOverlayVisuals.  If this
					 * number is zero, the device does
					 * not have overlay planes. */
    OverlayInfo	**pOverlayVisuals;	/* The device's overlay plane visual
					 * information. */
    int		*numImageVisuals;	/* Number of XVisualInfo's pointed
					 * to by pImageVisuals. */
    XVisualInfo	***pImageVisuals;	/* The device's image visuals. */
{
    XVisualInfo	getVisInfo;		/* Paramters of XGetVisualInfo */
    int		mask;
    XVisualInfo	*pVis, **pIVis;		/* Faster, local copies */
    OverlayInfo	*pOVis;
    OverlayVisualPropertyRec	*pOOldVis;
    int		nVisuals, nOVisuals, nIVisuals;
    Atom	overlayVisualsAtom;	/* Parameters for XGetWindowProperty */
    Atom	actualType;
    unsigned long numLongs, bytesAfter;
    int		actualFormat;
    int		nImageVisualsAlloced;	/* Values to process the XVisualInfo */
    int		imageVisual;		/* array */


    /* First, get the list of visuals for this screen. */
    getVisInfo.screen = screen;
    mask = VisualScreenMask; 

    *pVisuals = XGetVisualInfo(display, mask, &getVisInfo, numVisuals);
    if ((nVisuals = *numVisuals) <= 0)
    {
	/* Return that the information wasn't sucessfully obtained: */
	return(1);
    }
    pVis = *pVisuals;


    /* Now, get the overlay visual information for this screen.  To obtain
     * this information, get the SERVER_OVERLAY_VISUALS property.
     */
    overlayVisualsAtom = XInternAtom(display, "SERVER_OVERLAY_VISUALS", True);
    if (overlayVisualsAtom != None)
    {
	/* Since the Atom exists, we can request the property's contents.  The
	 * do-while loop makes sure we get the entire list from the X server.
	 */
	bytesAfter = 0;
	numLongs = sizeof(OverlayVisualPropertyRec) / 4;
	do
	{
	    numLongs += bytesAfter * 4;
	    XGetWindowProperty(display, RootWindow(display, screen),
			       overlayVisualsAtom, 0, numLongs, False,
			       overlayVisualsAtom, &actualType, &actualFormat,
			       &numLongs, &bytesAfter,
			       (unsigned char **) pOverlayVisuals);
	}
	while (bytesAfter > 0);


	/* Calculate the number of overlay visuals in the list. */
	*numOverlayVisuals = numLongs / (sizeof(OverlayVisualPropertyRec) / 4);
    }
    else if (
	strcmp(display, "Hewlett-Packard Company") == 0 &&
	XHPGetServerMode(display, screen) == XHPCOMBINED_MODE)
    {
	/* This is an old X server that supports overlay plane windows, but
	 * doesn't set the SERVER_OVERLAY_VISUALS propery.  Call a routine
	 * to set the property and return an array of OverlayInfo structs.
	 * NOTE: This code won't be necessary in the future once all servers
	 * that support overlays set this property.
	 */
	SetServerOverlayVisualsProperty(display, screen,
					numOverlayVisuals, pOverlayVisuals);
    }
    else
    {
	/* This screen doesn't have overlay planes. */
	*numOverlayVisuals = 0;
	*pOverlayVisuals = NULL;
	*transparentOverlays = 0;
    }


    /* Process the pVisuals array. */
    *numImageVisuals = 0;
    nImageVisualsAlloced = 1;
    pIVis = *pImageVisuals = (XVisualInfo **) malloc(4);
    while (--nVisuals >= 0)
    {
	nOVisuals = *numOverlayVisuals;
	pOVis = *pOverlayVisuals;
	imageVisual = True;
	while (--nOVisuals >= 0)
	{
	    pOOldVis = (OverlayVisualPropertyRec *) pOVis;
	    if (pVis->visualid == pOOldVis->visualID)
	    {
		imageVisual = False;
		pOVis->pOverlayVisualInfo = pVis;
		if (pOVis->transparentType == TransparentPixel)
		    *transparentOverlays = 1;
	    }
	    pOVis++;
	}
	if (imageVisual)
	{
	    if ((*numImageVisuals += 1) > nImageVisualsAlloced)
	    {
		nImageVisualsAlloced++;
		*pImageVisuals = (XVisualInfo **)
		    realloc(*pImageVisuals, (nImageVisualsAlloced * 4));
		pIVis = *pImageVisuals + (*numImageVisuals - 1);
	    }
	    *pIVis++ = pVis;
	}
	pVis++;
    }


    /* Return that the information was sucessfully obtained: */
    return(0);

} /* GetXVisualInfo() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * SetServerOverlayVisualsProperty()
 *
 * This routine sets the property and returns an array of OverlayInfo structs.
 * NOTE: This code won't be necessary in the future once all servers that
 * support overlays set this property.
 *
 ******************************************************************************/

static void SetServerOverlayVisualsProperty(display, screen,
					    numOverlayVisuals, pOverlayVisuals)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    int		*numOverlayVisuals;	/* Number of OverlayVisualInfo's
					 * pointed to by pOverlayVisuals.
					 * If this number is zero, the device
					 * does not have overlay planes. */
    OverlayInfo	**pOverlayVisuals;	/* The device's overlay plane visual
					 * information. */
{
    OverlayVisualPropertyRec	*pOVis;
    Atom	overlayVisualsAtom;	/* Parameters for XGetWindowProperty */


    /* On "old" HP "combined mode" devices, the default visual is the one
     * and only overlay visual.  These devices really support a transparent
     * pixel "value" (which is what the overlay planes are painted with above
     * image planes windows), but they don't yet advertise that pixel value
     * (be careful, with what you look at down there).  The transparent pixel
     * "value" is the same number as the number of colormap entries.  For
     * example, if the device returns a 4-plane overlay visual, the
     * "map_entries" value below is 15 (the colormap has 15 allocatable
     * colors: 0-14), and the transparent pixel value is 15.  If the device
     * returns a 3-plane visual, the "map_entries" value below is 7, and the
     * transparent pixel value is 7.
     */
    pOVis = ((OverlayVisualPropertyRec *)
	     malloc(sizeof(OverlayVisualPropertyRec)));
    pOVis->visualID = DefaultVisual(display, screen)->visualid;
    pOVis->transparentType = TransparentPixel;
    pOVis->value = DefaultVisual(display, screen)->map_entries;
    pOVis->layer = 1;

    overlayVisualsAtom = XInternAtom(display, "SERVER_OVERLAY_VISUALS", False);
    XChangeProperty(display, RootWindow(display, screen), overlayVisualsAtom,
		    overlayVisualsAtom, 32, PropModeReplace,
		    (unsigned char *) pOVis, 4);
    XSync(display, False);

    weCreateServerOverlayVisualsProperty = True;

    *numOverlayVisuals = 1;
    *pOverlayVisuals = (OverlayInfo *) pOVis;

} /* SetServerOverlayVisualsProperty() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * FreeXVisualInfo()
 *
 * This routine frees the data that was allocated by GetXVisualInfo().
 *
 ******************************************************************************/

void FreeXVisualInfo(pVisuals, pOverlayVisuals, pImageVisuals)

    XVisualInfo	*pVisuals;
    OverlayInfo	*pOverlayVisuals;
    XVisualInfo	**pImageVisuals;
{
    XFree(pVisuals);
    if (weCreateServerOverlayVisualsProperty)
	free(pOverlayVisuals);
    else
	XFree(pOverlayVisuals);
    free(pImageVisuals);

} /* FreeXVisualInfo() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * FindImagePlanesVisual()
 *
 * This routine attempts to find a visual to use to create an image planes
 * window based upon the information passed in.
 *
 * The "Hint" values give guides to the routine as to what the program wants.
 * The "depthFlexibility" value tells the routine how much the program wants
 * the actual "depthHint" specified.  If the program can't live with the
 * screen's image planes visuals, the routine returns non-zero, and the
 * "depthObtained" and "pImageVisualToUse" return parameters are NOT valid.
 * Otherwise, the "depthObtained" and "pImageVisualToUse" return parameters
 * are valid and the routine returns zero.
 *
 * NOTE: This is just an example of what can be done.  It may or may not be
 * useful for any specific application.
 *
 ******************************************************************************/

int FindImagePlanesVisual(display, screen, numImageVisuals, pImageVisuals,
			  sbCmapHint, depthHint, depthFlexibility,
			  pImageVisualToUse, depthObtained)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    int		numImageVisuals;	/* Number of XVisualInfo's pointed
					 * to by pImageVisuals. */
    XVisualInfo	**pImageVisuals;	/* The device's image visuals. */
    int		sbCmapHint;		/* What Starbase cmap modes will be
					 * used with the visual.  NOTE: This
					 * is a mask of the possible values. */
    int		depthHint;		/* Desired depth. */
    int		depthFlexibility;	/* How much the actual value in
					 * "depthHint" is desired. */
    Visual	**pImageVisualToUse;	/* The screen's image visual to use. */
    int		*depthObtained;		/* Actual depth of the visual. */
{
    XVisualInfo	*pVisualInfoToUse;	/* The current best visual to use. */
    int		i;
    int		visMask;
    int		curDelta, newDelta;



    switch (sbCmapHint)
    {
	case SB_CMAP_TYPE_NORMAL:
	case SB_CMAP_TYPE_NORMAL | SB_CMAP_TYPE_MONOTONIC:
	case SB_CMAP_TYPE_NORMAL | SB_CMAP_TYPE_MONOTONIC | SB_CMAP_TYPE_FULL:
	case SB_CMAP_TYPE_NORMAL | SB_CMAP_TYPE_FULL:
	    visMask = GRAY_SCALE | PSEUDO_COLOR;
	    break;
	case SB_CMAP_TYPE_MONOTONIC:
	    visMask = STATIC_GRAY | GRAY_SCALE | PSEUDO_COLOR;
	    break;
	case SB_CMAP_TYPE_MONOTONIC | SB_CMAP_TYPE_FULL:
	    visMask = GRAY_SCALE | PSEUDO_COLOR;
	    break;
	case SB_CMAP_TYPE_FULL:
	    visMask = GRAY_SCALE | PSEUDO_COLOR | TRUE_COLOR | DIRECT_COLOR;
	    break;
	default:
	    /* The caller didn't specify a valid combination of CMAP_ type: */
	    return(1);
    } /* switch (sbCmapHint) */


    pVisualInfoToUse = NULL;

    for (i = 0 ; i < numImageVisuals ; i++)
    {
	switch (pImageVisuals[i]->class)
	{
	    case StaticGray:
		if (visMask & STATIC_GRAY)
		{
		    if (pVisualInfoToUse == NULL)
		    {
			if ((pImageVisuals[i]->depth == depthHint) ||
			    depthFlexibility)
			{
			    pVisualInfoToUse = pImageVisuals[i];
			}
		    }
		    else
		    {
			if (pImageVisuals[i]->depth == depthHint)
			    pVisualInfoToUse = pImageVisuals[i];
			else if (depthFlexibility)
			{
			    /* The basic hueristic used is to find the closest
			     * depth to "depthHint" that's also greater than
			     * "depthHint", or just the closest depth:
			     */
			    if ((curDelta = pVisualInfoToUse->depth -
				 depthHint) > 0)
			    {
				/* Only choose this new visual if it's also
				 * deeper than "depthHint" and closer to
				 * "depthHint" than the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) && (newDelta < curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			    else
			    {
				/* Choose this new visual if it's deeper than
				 * "depthHint" or closer to "depthHint" than
				 * the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) ||
				    (-newDelta < -curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			}
		    }
		}
		break;
	    case GrayScale:
		if (visMask & GRAY_SCALE)
		{
		    if (pVisualInfoToUse == NULL)
		    {
			if ((pImageVisuals[i]->depth == depthHint) ||
			    depthFlexibility)
			{
			    pVisualInfoToUse = pImageVisuals[i];
			}
		    }
		    else if (!((sbCmapHint & SB_CMAP_TYPE_FULL) &&
			       (pVisualInfoToUse->class == DirectColor)))
		    {
			if (pImageVisuals[i]->depth == depthHint)
			    pVisualInfoToUse = pImageVisuals[i];
			else if (depthFlexibility)
			{
			    /* The basic hueristic used is to find the closest
			     * depth to "depthHint" that's also greater than
			     * "depthHint", or just the closest depth:
			     */
			    if ((curDelta = pVisualInfoToUse->depth -
				 depthHint) > 0)
			    {
				/* Only choose this new visual if it's also
				 * deeper than "depthHint" and closer to
				 * "depthHint" than the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) && (newDelta < curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			    else
			    {
				/* Choose this new visual if it's deeper than
				 * "depthHint" or closer to "depthHint" than
				 * the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) ||
				    (-newDelta < -curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			}
		    }
		}
		break;
	    case PseudoColor:
		if (visMask & PSEUDO_COLOR)
		{
		    if (pVisualInfoToUse == NULL)
		    {
			if ((pImageVisuals[i]->depth == depthHint) ||
			    depthFlexibility)
			{
			    pVisualInfoToUse = pImageVisuals[i];
			}
		    }
		    else if (!((sbCmapHint & SB_CMAP_TYPE_FULL) &&
			       (pVisualInfoToUse->class == DirectColor)))
		    {
			if (pImageVisuals[i]->depth == depthHint)
			    pVisualInfoToUse = pImageVisuals[i];
			else if (depthFlexibility)
			{
			    /* The basic hueristic used is to find the closest
			     * depth to "depthHint" that's also greater than
			     * "depthHint", or just the closest depth:
			     */
			    if ((curDelta = pVisualInfoToUse->depth -
				 depthHint) > 0)
			    {
				/* Only choose this new visual if it's also
				 * deeper than "depthHint" and closer to
				 * "depthHint" than the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) && (newDelta < curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			    else
			    {
				/* Choose this new visual if it's deeper than
				 * "depthHint" or closer to "depthHint" than
				 * the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) ||
				    (-newDelta < -curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			}
		    }
		}
		break;
	    case StaticColor:
		/* Starbase doesn't work well with StaticColor visuals: */
		break;
	    case TrueColor:
		if (visMask & TRUE_COLOR)
		{
		    /* The only Starbase cmap type that TrueColor works with
		     * is SB_CMAP_TYPE_FULL, so we know that SB_CMAP_TYPE_FULL
		     * is what the program wants:
		     */
		    if (pVisualInfoToUse == NULL)
		    {
			if ((pImageVisuals[i]->depth == depthHint) ||
			    depthFlexibility)
			{
			    pVisualInfoToUse = pImageVisuals[i];
			}
		    }
		    /* This example code prefers DirectColor to TrueColor: */
		    else if (pVisualInfoToUse->class != DirectColor)
		    {
			if (pImageVisuals[i]->depth == depthHint)
			    pVisualInfoToUse = pImageVisuals[i];
			else if (depthFlexibility)
			{
			    /* The basic hueristic used is to find the closest
			     * depth to "depthHint" that's also greater than
			     * "depthHint", or just the closest depth:
			     */
			    if ((curDelta = pVisualInfoToUse->depth -
				 depthHint) > 0)
			    {
				/* Only choose this new visual if it's also
				 * deeper than "depthHint" and closer to
				 * "depthHint" than the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) && (newDelta < curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			    else
			    {
				/* Choose this new visual if it's deeper than
				 * "depthHint" or closer to "depthHint" than
				 * the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) ||
				    (-newDelta < -curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			}
		    }
		}
		break;
	    case DirectColor:
		if (visMask & DIRECT_COLOR)
		{
		    if (pVisualInfoToUse == NULL)
		    {
			if ((pImageVisuals[i]->depth == depthHint) ||
			    depthFlexibility)
			{
			    pVisualInfoToUse = pImageVisuals[i];
			}
		    }
		    else
		    {
			if (pImageVisuals[i]->depth == depthHint)
			    pVisualInfoToUse = pImageVisuals[i];
			else if (depthFlexibility)
			{
			    /* The basic hueristic used is to find the closest
			     * depth to "depthHint" that's also greater than
			     * "depthHint", or just the closest depth:
			     */
			    if ((curDelta = pVisualInfoToUse->depth -
				 depthHint) > 0)
			    {
				/* Only choose this new visual if it's also
				 * deeper than "depthHint" and closer to
				 * "depthHint" than the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) && (newDelta < curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			    else
			    {
				/* Choose this new visual if it's deeper than
				 * "depthHint" or closer to "depthHint" than
				 * the currently chosen visual:
				 */
				if (((newDelta = pImageVisuals[i]->depth -
				      depthHint) > 0) ||
				    (-newDelta < -curDelta))
				{
				    pVisualInfoToUse = pImageVisuals[i];
				}
			    }
			}
		    }
		}
		break;
	} /* switch (pImageVisuals[i]->class) */
    } /* for (i = 0 ; i < numImageVisuals ; i++) */


    if (pVisualInfoToUse != NULL)
    {
	*pImageVisualToUse = pVisualInfoToUse->visual;
	*depthObtained = pVisualInfoToUse->depth;
	return(0);
    }
    else
    {
	/* Couldn't find an appropriate visual class: */
	return(1);
    }

} /* FindImagePlanesVisual() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * FindOverlayPlanesVisual()
 *
 * This routine attempts to find a visual to use to create an overlay planes
 * window based upon the information passed in.
 *
 * While the CreateImagePlanesWindow() routine took a sbCmapHint, this
 * routine doesn't.  Starbase's CMAP_FULL shouldn't be used in overlay planes
 * windows.  This is partially because this functionality is better suited in
 * the image planes where there are generally more planes, and partially
 * because the overlay planes generally have PseudoColor visuals with one
 * color being transparent (the transparent normally being the "white" color
 * for CMAP_FULL).
 *
 * The "depthHint" values give guides to the routine as to what depth the
 * program wants the window to be.  The "depthFlexibility" value tells the
 * routine how much the program wants the actual "depthHint" specified.  If
 * the program can't live with the screen's overlay planes visuals, the
 * routine returns non-zero, and the "depthObtained" and "pOverlayVisualToUse"
 * return parameters are NOT valid.  Otherwise, the "depthObtained" and
 * "pOverlayVisualToUse" return parameters are valid and the routine returns
 * zero.
 *
 * NOTE: This is just an example of what can be done.  It may or may not be
 * useful for any specific application.
 *
 ******************************************************************************/

int FindOverlayPlanesVisual(display, screen, numOverlayVisuals, pOverlayVisuals,
			    depthHint, depthFlexibility, transparentBackground,
			    pOverlayVisualToUse, depthObtained,
			    transparentColor)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    int		numOverlayVisuals;	/* Number of OverlayInfo's pointed
					 * to by pOverlayVisuals. */
    OverlayInfo	*pOverlayVisuals;	/* The device's overlay plane visual
					 * information. */
    int		depthHint;		/* Desired depth. */
    int		depthFlexibility;	/* How much the actual value in
					 * "depthHint" is desired. */
    int		transparentBackground;	/* Non-zero if the visual must have
					 * a transparent color. */
    Visual	**pOverlayVisualToUse;	/* The screen's overlay visual to
					 * use. */
    int		*depthObtained;		/* Actual depth of the visual. */
    int		*transparentColor;	/* The transparent color the program
					 * can use with the visual. */
{
    XVisualInfo	*pCandidateVisual;	/* Fast/local pointer. */
    XVisualInfo	*pVisualInfoToUse;	/* The current best visual to use. */
    int		i;
    int		visMask;
    int		curDelta, newDelta;



    pVisualInfoToUse = NULL;

    for (i = 0 ; i < numOverlayVisuals ; i++)
    {
	pCandidateVisual = pOverlayVisuals[i].pOverlayVisualInfo;
	if (pVisualInfoToUse == NULL)
	{
	    if (((pCandidateVisual->depth == depthHint) || depthFlexibility) &&
		!transparentBackground ||
		(pOverlayVisuals[i].transparentType == TransparentPixel))
	    {
		pVisualInfoToUse = pCandidateVisual;
		*transparentColor = pOverlayVisuals[i].value;
	    }
	}
	else if (!transparentBackground ||
		 (pOverlayVisuals[i].transparentType == TransparentPixel))
	{
	    if (pCandidateVisual->depth == depthHint)
	    {
		pVisualInfoToUse = pCandidateVisual;
		*transparentColor = pOverlayVisuals[i].value;
	    }
	    else if (depthFlexibility)
	    {
		/* The basic hueristic used is to find the closest depth to
		 * "depthHint" that's also greater than "depthHint", or just
		 * the closest depth:
		 */
		if ((curDelta = pVisualInfoToUse->depth - depthHint) > 0)
		{
		    /* Only choose this new visual if it's also deeper than
		     * "depthHint" and closer to "depthHint" than the currently
		     * chosen visual:
		     */
		    if (((newDelta = pCandidateVisual->depth - depthHint) > 0)
			&& (newDelta < curDelta))
		    {
			pVisualInfoToUse = pCandidateVisual;
			*transparentColor = pOverlayVisuals[i].value;
		    }
		}
		else
		{
		    /* Choose this new visual if it's deeper than "depthHint"
		     * or closer to "depthHint" than the currently chosen
		     * visual:
		     */
		    if (((newDelta = pCandidateVisual->depth - depthHint) > 0)
			|| (-newDelta < -curDelta))
		    {
			pVisualInfoToUse = pCandidateVisual;
			*transparentColor = pOverlayVisuals[i].value;
		    }
		}
	    }
	}
    } /* for (i = 0 ; i < numOverlayVisuals ; i++) */


    if (pVisualInfoToUse != NULL)
    {
	*pOverlayVisualToUse = pVisualInfoToUse->visual;
	*depthObtained = pVisualInfoToUse->depth;
	return(0);
    }
    else
    {
	/* Couldn't find an appropriate visual class: */
	return(1);
    }

} /* FindOverlayPlanesVisual() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * CreateImagePlanesWindow()
 *
 * This routine creates an image planes window, potentially creates a colormap
 * for the window to use, and sets the window's standard properties, based
 * upon the information passed in to the routine.  While "created," the window
 * has not been mapped.
 *
 * If the routine suceeds, it returns zero and the return parameters
 * "imageWindow", "imageColormap" and "mustFreeImageColormap" are valid.
 * Otherwise, the routine returns non-zero and the return parameters are
 * NOT valid.
 *
 * NOTE: This is just an example of what can be done.  It may or may not be
 * useful for any specific application.
 *
 ******************************************************************************/

int CreateImagePlanesWindow(display, screen, parentWindow,
			    windowX, windowY, windowWidth, windowHeight,
			    windowDepth, pImageVisualToUse,
			    argc, argv, windowName, iconName,
			    imageWindow, imageColormap, mustFreeImageColormap)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    Window	parentWindow;		/* Window ID of the parent window for
					 * the created window. */
    int		windowX;		/* Desired X coord. of the window. */
    int		windowY;		/* Desired Y coord of the window. */
    int		windowWidth;		/* Desired width of the window. */
    int		windowHeight;		/* Desired height of the window. */
    int		windowDepth;		/* Desired depth of the window. */
    Visual	*pImageVisualToUse;	/* The window's image planes visual. */
    int		argc;			/* Program's argc parameter. */
    char	*argv[];		/* Program's argv parameter. */
    char	*windowName;		/* Name to put on window's border. */
    char	*iconName;		/* Name to put on window's icon. */
    Window	*imageWindow;		/* Window ID of the created window. */
    Colormap	*imageColormap;		/* The window's colormap. */
    int		*mustFreeImageColormap;	/* Non-zero if the program must call
					 * XFreeColormap() for imageColormap. */
{
    XSetWindowAttributes winAttributes;	/* Attributes for window creation */
    XSizeHints	hints;



    if (pImageVisualToUse == DefaultVisual(display, screen))
    {
	*mustFreeImageColormap = False;
	*imageColormap = winAttributes.colormap = DefaultColormap(display,
								  screen);
	winAttributes.background_pixel = BlackPixel(display, screen);
	winAttributes.border_pixel = WhitePixel(display, screen);
    }
    else
    {
	XColor		actualColor, databaseColor;

	*mustFreeImageColormap = True;
	*imageColormap = winAttributes.colormap =
	    XCreateColormap(display, RootWindow(display, screen),
			    pImageVisualToUse, AllocNone);
	XAllocNamedColor(display, winAttributes.colormap, "Black",
			 &actualColor, &databaseColor);
	winAttributes.background_pixel = actualColor.pixel;
	XAllocNamedColor(display, winAttributes.colormap, "White",
			 &actualColor, &databaseColor);
	winAttributes.border_pixel = actualColor.pixel;
    }
    winAttributes.event_mask = ExposureMask;


    *imageWindow = XCreateWindow(display, parentWindow,
				 0, 0, windowWidth, windowHeight, 2,
				 windowDepth, InputOutput, pImageVisualToUse,
				 (CWBackPixel | CWColormap |
				  CWBorderPixel | CWEventMask),
				 &winAttributes);


    hints.flags = (USSize | USPosition);
    hints.x = windowX;
    hints.y = windowY;
    hints.width  = windowWidth;
    hints.height = windowHeight;
    XSetStandardProperties(display, *imageWindow, windowName, iconName,
			   None, argv, argc, &hints);


    return(0);

} /* CreateImagePlanesWindow() */


/******************************************************************************
* AUXILIARY:								     *
 *
 * CreateOverlayPlanesWindow()
 *
 * This routine creates an overlay planes window, potentially creates a colormap
 * for the window to use, and sets the window's standard properties, based
 * upon the information passed in to the routine.  While "created," the window
 * has not been mapped.
 *
 * If the routine suceeds, it returns zero and the return parameters
 * "overlayWindow", "overlayColormap" and "mustFreeOverlayColormap" are valid.
 * Otherwise, the routine returns non-zero and the return parameters are
 * NOT valid.
 *
 * NOTE: This is just an example of what can be done.  It may or may not be
 * useful for any specific application.
 *
 ******************************************************************************/

int CreateOverlayPlanesWindow(display, screen, parentWindow,
			      windowX, windowY, windowWidth, windowHeight,
			      windowDepth, pOverlayVisualToUse,
			      argc, argv, windowName, iconName,
			      transparentBackground, transparentColor,
			      overlayWindow, overlayColormap,
			      mustFreeOverlayColormap)

    Display	*display;		/* Which X server (aka "display"). */
    int		screen;			/* Which screen of the "display". */
    Window	parentWindow;		/* Window ID of the parent window for
					 * the created window. */
    int		windowX;		/* Desired X coord. of the window. */
    int		windowY;		/* Desired Y coord of the window. */
    int		windowWidth;		/* Desired width of the window. */
    int		windowHeight;		/* Desired height of the window. */
    int		windowDepth;		/* Desired depth of the window. */
    Visual	*pOverlayVisualToUse;	/* The window's overlay planes visual.*/
    int		argc;			/* Program's argc parameter. */
    char	*argv[];		/* Program's argv parameter. */
    char	*windowName;		/* Name to put on window's border. */
    char	*iconName;		/* Name to put on window's icon. */
    int		transparentBackground;	/* Non-zero if the window's background
					 * should be a transparent color. */
    int		*transparentColor;	/* The transparent color to use as the
					 * window's background. */
    Window	*overlayWindow;		/* Window ID of the created window. */
    Colormap	*overlayColormap;	/* The window's colormap. */
    int		*mustFreeOverlayColormap;/* Non-zero if the program must call
					  * XFreeColormap() for
					  * overlayColormap. */
{
    XSetWindowAttributes winAttributes;	/* Attributes for window creation */
    XSizeHints	hints;
    int		borderWidth;



    if (pOverlayVisualToUse == DefaultVisual(display, screen))
    {
	*mustFreeOverlayColormap = False;
	*overlayColormap = winAttributes.colormap = DefaultColormap(display,
								    screen);
	if (transparentBackground)
	    winAttributes.background_pixel = *transparentColor;
	else
	    winAttributes.background_pixel = BlackPixel(display, screen);
	winAttributes.border_pixel = WhitePixel(display, screen);
    }
    else
    {
	XColor		actualColor, databaseColor;

	*mustFreeOverlayColormap = True;
	*overlayColormap = winAttributes.colormap =
	    XCreateColormap(display, RootWindow(display, screen),
			    pOverlayVisualToUse, AllocNone);
	if (transparentBackground)
	    winAttributes.background_pixel = *transparentColor;
	else
	{
	    XAllocNamedColor(display, winAttributes.colormap, "Black",
			     &actualColor, &databaseColor);
	    winAttributes.background_pixel = actualColor.pixel;
	}
	XAllocNamedColor(display, winAttributes.colormap, "White",
			 &actualColor, &databaseColor);
	winAttributes.border_pixel = actualColor.pixel;
    }
    winAttributes.event_mask = ExposureMask;


    if (transparentBackground && (parentWindow == RootWindow(display, screen)))
	borderWidth = 2;
    else
	borderWidth = 0;


    *overlayWindow = XCreateWindow(display, parentWindow,
				   0, 0, windowWidth, windowHeight,
				   borderWidth, windowDepth, InputOutput,
				   pOverlayVisualToUse,
				   (CWBackPixel | CWColormap |
				    CWBorderPixel | CWEventMask),
				   &winAttributes);


    hints.flags = (USSize | PPosition);
    hints.x = windowX;
    hints.y = windowY;
    hints.width  = windowWidth;
    hints.height = windowHeight;
    XSetStandardProperties(display, *overlayWindow, windowName, iconName,
			   None, argv, argc, &hints);


    return(0);

} /* CreateOverlayPlanesWindow() */
