/*****************************************************************************
*   Default object drawing routine common to graphics drivers.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "trng_lib.h"
#include "attribut.h"
#include "allocate.h"
#include "grap_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single object using current modes and transformations.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawObject                                                             M
*****************************************************************************/
void IGDrawObject(IPObjectStruct *PObj)
{
    CagdRType *R;
    IrtPtType Pt;
    int HasTexture = FALSE,
	Width = AttrGetObjectIntAttrib(PObj, "DWidth"),
	LPattern = AttrGetObjectIntAttrib(PObj, "LnPtrn");
    IrtRType
	Transp = AttrGetObjectRealAttrib(PObj, "Transp");

    if (IP_ATTR_IS_BAD_INT(Width))
        Width = 1;

    if (!IP_ATTR_IS_BAD_REAL(Transp))
	IGSetTranspObj(Transp);

    if (!IP_ATTR_IS_BAD_INT(LPattern))
        IGSetLinePattern(LPattern);

    IGSetWidthObj(Width * IGGlblLineWidth);
    IGSetColorObj(PObj);

    if (IG_IS_HIGHLIGHT1_OBJ(PObj)) {
        IGSetWidthObj(Width * 2 * IGGlblLineWidth);
	IGSetColorRGB(IGGlblHighlight1Color);
    }
    else if (IG_IS_HIGHLIGHT2_OBJ(PObj)) {
        IGSetWidthObj(Width * 2 * IGGlblLineWidth);
	IGSetColorRGB(IGGlblHighlight2Color);
    }
    else if ((HasTexture = IGInitSrfTexture(PObj)) != FALSE &&
	     IGSetTexture(PObj)) {
    }
#   ifdef IRIT_HAVE_OGL_CG_LIB
    else if (AttrGetObjectStrAttrib(PObj, "DTexture") != NULL &&
	     IGCGDrawDTexture(PObj))
		    return;
    else if (AttrGetObjectStrAttrib(PObj, "ffd_texture") != NULL &&
	     IGCGFfdDraw(PObj))
        return;
#   endif /* IRIT_HAVE_OGL_CG_LIB */

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    IGDrawPolyFuncPtr(PObj);
	    break;
	case IP_OBJ_CTLPT:
	    /* Coerce, in place, a control points to a regular point. */
	    R = PObj -> U.CtlPt.Coords;
	    CagdCoercePointTo(Pt, CAGD_PT_E3_TYPE,
			      &R, -1, PObj -> U.CtlPt.PtType);
	    IRIT_PT_COPY(PObj -> U.Pt, Pt);
	    PObj -> ObjType = IP_OBJ_POINT;
	case IP_OBJ_POINT:
	    if (!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(PObj,
							   "LIGHT_SOURCE"))) {
		if (IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(PObj,
							      "ACTIVATED"))) {
		    const char *p;
		    int i, Red, Green, Blue,
		        LightIndex = -1;
		    IGLightType LightPos;
		    IrtVecType LightColor;

		    for (i = 0; i < 3; i++)
			LightPos[i] = (float) PObj -> U.Pt[i];
		    LightPos[3] = (float)
			!((p = AttrGetObjectStrAttrib(PObj, "TYPE")) != NULL &&
			  stricmp(p, "POINT_INFTY"));
		    
		    if (AttrGetObjectRGBColor(PObj, &Red, &Green, &Blue)) {
			LightColor[0] = Red / 255.0;
			LightColor[1] = Green / 255.0;
			LightColor[2] = Blue/ 255.0;
		    }
		    else {
			LightColor[0] = LightColor[1] = LightColor[2] = 1.0;
		    }

		    i = AttrGetObjectIntAttrib(PObj, "INDEX");
		    if (!IP_ATTR_IS_BAD_INT(i))
			LightIndex = i;
			
		    IGSetLightSource(LightPos, LightColor, LightIndex);

		    AttrSetObjectIntAttrib(PObj, "ACTIVATED", TRUE);
		}
	    }
	    else
		IGDrawPtVec(PObj);
	    break;
	case IP_OBJ_VECTOR:
	    IGDrawPtVec(PObj);
	    break;
	case IP_OBJ_STRING:
	    IGDrawString(PObj);
	    break;
	case IP_OBJ_CURVE:
	    if (PObj -> U.Crvs == NULL)    /* Can occur in the curve editor. */
	        break;
	    if (CAGD_IS_POWER_CRV(PObj -> U.Crvs)) {
	        CagdCrvStruct
		    *Crv = CagdCnvrtPwr2BzrCrv(PObj -> U.Crvs);

		CagdCrvFree(PObj -> U.Crvs);
		PObj -> U.Crvs = Crv;
	    }
	    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Crvs -> PType) == 1) {
	        CagdCrvStruct
		    *Crv = CagdCoerceCrvTo(PObj -> U.Crvs,
			CAGD_IS_RATIONAL_CRV(PObj -> U.Crvs) ? CAGD_PT_P2_TYPE
							     : CAGD_PT_E2_TYPE,
					   FALSE);

		CagdCrvFree(PObj -> U.Crvs);
		PObj -> U.Crvs = Crv;
	    }

	    IGDrawCurve(PObj);
	    break;
	case IP_OBJ_TRIMSRF:
	    if (CAGD_IS_POWER_SRF(PObj -> U.TrimSrfs -> Srf)) {
	        CagdSrfStruct
		    *Srf = CagdCnvrtPwr2BzrSrf(PObj -> U.TrimSrfs -> Srf);

		CagdSrfFree(PObj -> U.TrimSrfs -> Srf);
		PObj -> U.TrimSrfs -> Srf = Srf;
	    }

	    if (PObj -> U.TrimSrfs -> TrimCrvList == NULL) {
	        CagdSrfStruct
		    *Srf = PObj -> U.TrimSrfs -> Srf;

	        if (IGGlblMore)
		    IRIT_INFO_MSG_PRINTF("Trimmed surface \"%s\" with no trimming curves detected\n\tand converted to regular surface\n",
			                 IP_GET_OBJ_NAME(PObj));
		
		PObj -> U.TrimSrfs -> Srf = NULL;
		TrimSrfFree(PObj -> U.TrimSrfs);
		PObj -> U.Srfs = Srf;
		PObj -> ObjType = IP_OBJ_SURFACE;
	    }
	    else {
	        IGDrawTrimSrf(PObj);
		break;
	    }
	case IP_OBJ_SURFACE:
	    if (PObj -> U.Srfs == NULL)  /* Can occur in the surface editor. */
	        break;
	    if (CAGD_IS_POWER_SRF(PObj -> U.Srfs)) {
	        CagdSrfStruct
		    *Srf = CagdCnvrtPwr2BzrSrf(PObj -> U.Srfs);

		CagdSrfFree(PObj -> U.Srfs);
		PObj -> U.Srfs = Srf;
	    }
	    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) == 1) {
	        CagdSrfStruct
		    *Srf = CagdCoerceSrfTo(PObj -> U.Srfs,
			CAGD_IS_RATIONAL_SRF(PObj -> U.Srfs) ? CAGD_PT_P2_TYPE
							     : CAGD_PT_E2_TYPE,
					     FALSE);

		CagdSrfFree(PObj -> U.Srfs);
		PObj -> U.Srfs = Srf;
	    }

	    IGDrawSurface(PObj);
	    break;
	case IP_OBJ_TRIVAR:
	    IGDrawTrivar(PObj);
	    break;
	case IP_OBJ_MULTIVAR:
	    if (PObj -> U.MultiVars -> Dim < 4) { /* Curve, Surface, Trivar. */
		IPObjectStruct *PTmp;

		if ((PTmp = AttrGetObjectObjAttrib(PObj,
						   "_Coerced")) == NULL) {
		    MvarMVStruct
			*MV = PObj -> U.MultiVars;

		    switch (MV -> Dim) {
			case 1:
			    PTmp = IPGenCRVObject(MvarMVToCrv(MV));

			    if (CAGD_IS_POWER_CRV(PTmp -> U.Crvs)) {
			        CagdCrvStruct
				    *Crv = CagdCnvrtPwr2BzrCrv(PTmp -> U.Crvs);

				CagdCrvFree(PTmp -> U.Crvs);
				PTmp -> U.Crvs = Crv;
			    }
			    break;
			case 2:
			    PTmp = IPGenSRFObject(MvarMVToSrf(MV));

			    if (CAGD_IS_POWER_SRF(PTmp -> U.Srfs)) {
			        CagdSrfStruct
				    *Srf = CagdCnvrtPwr2BzrSrf(PTmp -> U.Srfs);

				CagdSrfFree(PTmp -> U.Srfs);
				PTmp -> U.Srfs = Srf;
			    }
			    break;
			case 3:
			    PTmp = IPGenTRIVARObject(MvarMVToTV(MV));
			    break;
		    }
		    PTmp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		    IP_SET_OBJ_NAME2(PTmp, IP_GET_OBJ_NAME(PObj));
		    IGUpdateObjectBBox(PTmp);

		    AttrSetObjectObjAttrib(PObj, "_Coerced", PTmp, FALSE);
		}

		if (IG_IS_HIGHLIGHT1_OBJ(PObj))
		    IG_SET_HIGHLIGHT1_OBJ(PTmp);
		else
		    IG_RST_HIGHLIGHT1_OBJ(PTmp);
		if (IG_IS_HIGHLIGHT2_OBJ(PObj))
		    IG_SET_HIGHLIGHT2_OBJ(PTmp);
		else
		    IG_RST_HIGHLIGHT2_OBJ(PTmp);

		IGDrawObject(PTmp);
	    }
	    break;
	case IP_OBJ_MODEL:
	    IGDrawModel(PObj);
	    break;
	case IP_OBJ_TRISRF:
	    if (TRNG_IS_GREGORY_TRISRF(PObj -> U.TriSrfs)) {
	        TrngTriangSrfStruct
		    *TriSrf = TrngCnvrtGregory2BzrTriSrf(PObj -> U.TriSrfs);

	        TrngTriSrfFree(PObj -> U.TriSrfs);
	        PObj -> U.TriSrfs = TriSrf;
	    }
	    IGDrawTriangSrf(PObj);
	    break;
	case IP_OBJ_LIST_OBJ:
	    IRIT_FATAL_ERROR("Should not have lists at this time");
	    break;
	default:
	    break;
    }

    if (HasTexture)
        IGSetTexture(NULL);	                 /* Disable texture mapping. */

    if (!IP_ATTR_IS_BAD_REAL(Transp))
	IGSetTranspObj(0.0);			    /* Disable transparency. */
}
