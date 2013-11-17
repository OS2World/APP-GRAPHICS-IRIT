/*****************************************************************************
*   Default point/vector drawing routine common to graphics drivers.	     *
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

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Point/Vector object using current modes and transformations. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A point/vector object to draw.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPtVec                                                              M
*****************************************************************************/
void IGDrawPtVec(IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA IrtPtType
	Zero = { 0.0, 0.0, 0.0 };
    int i;
    IrtPtType Ends[6];
    IrtRType
	*Pt = PObj -> U.Pt;

    
    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
	IGPlotTo3D(Pt);
    }
    else {
        for (i = 0; i < 6; i++)
	    IRIT_PT_COPY(Ends[i], Pt);

	Ends[0][0] -= IGGlblPointSize;
	Ends[1][0] += IGGlblPointSize;
	Ends[2][1] -= IGGlblPointSize;
	Ends[3][1] += IGGlblPointSize;
	Ends[4][2] -= IGGlblPointSize;
	Ends[5][2] += IGGlblPointSize;

	for (i = 0; i < 6; i += 2) {
	    IGMoveTo3D(Ends[i]);
	    IGLineTo3D(Ends[i+1]);
	}

	if (IP_IS_VEC_OBJ(PObj)) {
	    IGMoveTo3D(Pt);
	    IGLineTo3D(Zero);
	}
    }
}
