/*****************************************************************************
*   Default 3d line drawing routine common to graphics drivers.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "grap_loc.h"

IRIT_STATIC_DATA IrtRType CurrentPos[3];

/*****************************************************************************
* DESCRIPTION:                                                               M
* A point poloy command in 3D object space.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       Location to plot point at.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPlotTo3D                                                               M
*****************************************************************************/
void IGPlotTo3D(IrtRType *Pt)
{
    MatMultPtby4by4(CurrentPos, Pt, IGGlblCrntViewMat);

    if (IGGlblDepthCue) {
	if (CurrentPos[2] <= 0.0 && IGGlblIntensityHighState)
	    IGSetColorIntensity(FALSE);
	else if (CurrentPos[2] > 0.0 && !IGGlblIntensityHighState)
	    IGSetColorIntensity(TRUE);
    }

    IGPlotTo2D(CurrentPos[0], CurrentPos[1]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* A move to command in 3D object space.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       Location to move to.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGMoveTo3D                                                               M
*****************************************************************************/
void IGMoveTo3D(IrtRType *Pt)
{
    MatMultPtby4by4(CurrentPos, Pt, IGGlblCrntViewMat);
    IGMoveTo2D(CurrentPos[0], CurrentPos[1]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* A line to command in 3D object space.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       Location to draw to.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGLineTo3D                                                               M
*****************************************************************************/
void IGLineTo3D(IrtRType *Pt)
{
    IrtRType NewPos[3];

    MatMultPtby4by4(NewPos, Pt, IGGlblCrntViewMat);

    if (IGGlblDepthCue) {
	if (CurrentPos[2] <= 0.0 && NewPos[2] <= 0.0) {
	    if (IGGlblIntensityHighState)
		IGSetColorIntensity(FALSE);

	    IGLineTo2D(NewPos[0], NewPos[1]);
	}
	else if ((CurrentPos[2] >= 0.0 && NewPos[2] >= 0.0) ||
		 IRIT_FABS(CurrentPos[2] - NewPos[2]) < IRIT_EPS) {
	    if (!IGGlblIntensityHighState)
		IGSetColorIntensity(TRUE);

	    IGLineTo2D(NewPos[0], NewPos[1]);
	}
	else {				      /* Line intersect Z = 0 plane. */
	    IrtRType MidPos[3],
		t = CurrentPos[2] / (CurrentPos[2] - NewPos[2]);

	    MidPos[0] = CurrentPos[0] * (1.0 - t) + NewPos[0] * t;
	    MidPos[1] = CurrentPos[1] * (1.0 - t) + NewPos[1] * t;

	    if (IGGlblIntensityHighState) {
		if (CurrentPos[2] > 0.0) {
		    IGLineTo2D(MidPos[0], MidPos[1]);
		}
		else {
		    IGLineTo2D(NewPos[0], NewPos[1]);
		    IGMoveTo2D(MidPos[0], MidPos[1]);
		}

		IGSetColorIntensity(FALSE);

		if (CurrentPos[2] > 0.0) {
		    IGLineTo2D(NewPos[0], NewPos[1]);
		}
		else {
		    IGLineTo2D(CurrentPos[0], CurrentPos[1]);
		    IGMoveTo2D(NewPos[0], NewPos[1]);
		}
	    }
	    else {
		if (CurrentPos[2] < 0.0) {
		    IGLineTo2D(MidPos[0], MidPos[1]);
		}
		else {
		    IGMoveTo2D(NewPos[0], NewPos[1]);
		    IGLineTo2D(MidPos[0], MidPos[1]);
		}

		IGSetColorIntensity(TRUE);

		if (CurrentPos[2] < 0.0) {
		    IGLineTo2D(NewPos[0], NewPos[1]);
		}
		else {
		    IGLineTo2D(CurrentPos[0], CurrentPos[1]);
		    IGMoveTo2D(NewPos[0], NewPos[1]);
		}
	    }
	}
    }
    else {
	IGLineTo2D(NewPos[0], NewPos[1]);
    }

    IRIT_GEN_COPY(CurrentPos, NewPos, 3 * sizeof(IrtRType));
}
