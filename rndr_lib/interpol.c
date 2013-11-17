/*****************************************************************************
* Abstraction of the data been interpolated in rendering process.            *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
*****************************************************************************/

#include "rndr_loc.h"
#include "interpol.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs COPY operation on object of Interpol type, which contains data  M
*   to be interpolated during polygons scan converting. Interpol objects has M
*   some sort of linked structure, so copy is tricky.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Dst:      OUT, pointer to destination object.                            M
*   Src:      IN, pointer to source object.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrInterpolStruct *:  Pointer to destination object.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   InterpolCopy, Interpol structure, interpolation                          M
*****************************************************************************/
IRndrInterpolStruct *InterpolCopy(IRndrInterpolStruct *Dst,
				  IRndrInterpolStruct *Src)
{
    IRndrIntensivityStruct
        *Keep = Dst -> i;

    *Dst = *Src;
    Dst -> i = NULL;
    if (Keep && Src -> i)
        IRIT_GEN_COPY(Dst -> i = Keep, Src -> i,
		      sizeof(IRndrIntensivityStruct) * Src -> IntensSize);

    return Dst;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize object of Interpol type to be an increment in interpolation   M
*   between first and second Interpol objects in scan conversion process.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Dst:     OUT, pointer to delta(increment) object to be initialized.      M
*   v1:      IN, pointer to the first Interpol object.                       M
*   v2:      IN, pointer to the second Interpol object.                      M
*   d:       IN, scaling factor determined by dimension of the polygon on    M
*            the current scan line.                                          *
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrInterpolStruct *: pointer to dst object.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   InterpolDelta, Interpol, interpolation                                   M
*****************************************************************************/
IRndrInterpolStruct *InterpolDelta(IRndrInterpolStruct *Dst,
			      IRndrInterpolStruct *v1,
			      IRndrInterpolStruct *v2,
			      IrtRType d)
{
    int j;
    IrtRType Fact;

    if (IRIT_FABS(d) < IRIT_EPS)
        d = IRIT_EPS;
    Fact = 1.0 / d;
    Dst -> IntensSize = v1 -> IntensSize;
    Dst -> z = (v1 -> z - v2 -> z) * Fact;
    Dst -> w = (v1 -> w - v2 -> w) * Fact;
    Dst -> u = (v1 -> u - v2 -> u) * Fact;
    Dst -> v = (v1 -> v - v2 -> v) * Fact;
    IRIT_PT_SUB(Dst -> n, v1 -> n, v2 -> n);
    IRIT_PT_SCALE(Dst -> n, Fact);
    if (v1 -> HasColor && v2 -> HasColor) {
	IRIT_PT_SUB(Dst -> c, v1 -> c, v2 -> c);
	IRIT_PT_SCALE(Dst -> c, Fact);
    }

    if (Dst -> i && v1 -> i && v2 -> i) {
        IRndrIntensivityStruct
	    *DstI = Dst -> i,
	    *v1I = v1 -> i,
	    *v2I = v2 -> i;

        for (j = 0; j++ < v1 -> IntensSize; DstI++, v1I++, v2I++) {
            DstI -> Diff = (v1I -> Diff - v2I -> Diff) * Fact;
            DstI -> Spec = (v1I -> Spec - v2I -> Spec) * Fact;
	}
    }
    else
        Dst -> i = NULL;

    return Dst;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increments destination Interpol object by delta Interpol object computed M
*   by "InterpolDelta" call. By that way we progress interpolation process.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Dst:     IN OUT, pointer to object to be incremented.                    M
*   d:       IN, pointer to delta object.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrInterpolStruct *: pointer to destination object.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   InterpolIncr, Interpol, interpolation                                    M
*****************************************************************************/
IRndrInterpolStruct *InterpolIncr(IRndrInterpolStruct *Dst,
				  IRndrInterpolStruct *d)
{
    int j;

    Dst -> z += d -> z;
    Dst -> w += d -> w;
    Dst -> u += d -> u;
    Dst -> v += d -> v;

    IRIT_PT_ADD(Dst -> n, Dst -> n, d -> n);

    if (Dst -> HasColor)
	IRIT_PT_ADD(Dst -> c, Dst -> c, d -> c);

    if (Dst -> i && d -> i) {
        IRndrIntensivityStruct
	    *DstI = Dst -> i,
	    *dI = d -> i;

        for (j = 0; j++ < Dst -> IntensSize; DstI++, dI++) {
            DstI -> Diff += dI -> Diff;
            DstI -> Spec += dI -> Spec;
	}
    }

    return Dst;
}
