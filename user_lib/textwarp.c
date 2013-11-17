/******************************************************************************
* TextWarp.c - Bezier text fonts warping along freeform surfaces.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jan 03.					      *
******************************************************************************/

#include <ctype.h>
#include "irit_sm.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "symb_lib.h"
#include "user_loc.h"

#define USER_WT_SAMPLES_PER_CURVE	100
#define USER_WT_MAX_ITER_LIGATURE	10
#define USER_WT_REL_ERR_LIGATURE	0.01

static IPObjectStruct *UserWarpTextOneChar(CagdSrfStruct *Srf,
					   IPObjectStruct *CharObj);
static IPObjectStruct *UserWarpTextRepositionChar(IPObjectStruct *LastCharObj,
						  IPObjectStruct *CharObj,
						  IrtRType *CrntSpace,
						  IrtRType Ligature);
static IPVertexStruct *UserWarpTextSamplePts(IPObjectStruct *CharObj);
static IPVertexStruct *UserWarpTextSampleCrvPts(CagdCrvStruct *Crv);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Warps the given text, using the current loaded font, in Bezier form in   M
* surface Srf using composition.  The characters of Txt are laid one after   M
* the other until the entire surface is filled horizontally, or characters   M
* in Txt are exhausted.							     M
*   The characters are scaled to fit VBase to VTop of the height (v)         M
* direction of the parameteric domain of Srf, for the letter 'A'.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          Surface to warp the text Txt along.  The text is laid      M
*		  along the u axis with the v axis being the height.         M
*   Txt:          Text to warp insid Srf.				     M
*   HSpace:       Horizonal space between characters.			     M
*   VBase, VTop:  Minimal and maximal fraction of height for regular         M
*		  charaacters.  Measured on the letter 'A'.                  M
*   Ligatures:    If non zero, amount of ligatures' contraction between      M
8		  adjacent characters.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List object of warped text, one object per character, M
*		       each character is a list of Bezier curves.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMLoadTextFont, GMMakeTextGeometry, SymbComposeSrfCrv                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserWarpTextOnSurface                                                    M
*****************************************************************************/
IPObjectStruct *UserWarpTextOnSurface(CagdSrfStruct *Srf,
				      const char *Txt,
				      IrtRType HSpace,
				      IrtRType VBase,
				      IrtRType VTop,
				      IrtRType Ligatures)
{
    static IrtRType
	NoSpacing[3] = { 0.0, 0.0, 0.0 },
	NoScaling = 1.0;
    char Len1Str[2];
    int i,
	RetObjIndex = 0;
    IrtRType
	UMin, UMax, VMin, VMax, VTrans, Scale, CrntSpace, UCrnt;
    IPObjectStruct
	*RetObjs = IPGenLISTObject(NULL),
	*LastCharObj = NULL,
	*CharObj = GMMakeTextGeometry("A", NoSpacing, &NoScaling);
    GMBBBboxStruct
	*BBox = GMBBComputeBboxObject(CharObj);

    IPFreeObject(CharObj);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Mapping from letter heights to VBase-VTop of surface domain height.  */
    /* Every character ctlpt P will be mapped to P * Scale + VTrans.        */
    Scale = ((VTop - VBase) * (VMax - VMin)) /
					    (BBox -> Max[1] - BBox -> Min[1]);
    VTrans = VBase * (VMax - VMin) + VMin - BBox -> Min[1] * Scale;

    /* Build the characters one after the other. */
    CrntSpace = HSpace;
    UCrnt = UMin + IRIT_EPS;
    Len1Str[1] = 0;
    for (i = 0; i < (int) strlen(Txt); i++) {
	if (isspace(Txt[i])) {
	    CrntSpace += HSpace;
	}
	else if (isgraph(Txt[i])) {
	    IrtHmgnMatType Mat;
	    IPObjectStruct *TmpObj;

	    Len1Str[0] = Txt[i];
	    CharObj = GMMakeTextGeometry(Len1Str, NoSpacing, &Scale);

	    /* Move the object to (UCrnt, VBase). */
	    BBox = GMBBComputeBboxObject(CharObj);
	    MatGenMatTrans(UCrnt + CrntSpace - BBox -> Min[0], VTrans, 0.0,
			   Mat);
	    TmpObj = GMTransformObject(CharObj, Mat);
	    IPFreeObject(CharObj);
	    CharObj = TmpObj;

	    /* Do a precise positioning with respect to the previous char. */
	    if (Ligatures > 0) {
		TmpObj = UserWarpTextRepositionChar(LastCharObj, CharObj,
						    &CrntSpace, Ligatures);
		if (LastCharObj)
		    IPFreeObject(LastCharObj);
		IPFreeObject(CharObj);
		CharObj = TmpObj;
		LastCharObj = IPCopyObject(NULL, CharObj, FALSE);
	    }

	    UCrnt += BBox -> Max[0] - BBox -> Min[0] + CrntSpace;

	    CrntSpace = HSpace;

	    if (UCrnt > UMax)		    /* Exhausted all the U domain!? */
	        break;

	    /* Insert the warped character to the returned output list. */
	    IPListObjectInsert(RetObjs, RetObjIndex++,
			       UserWarpTextOneChar(Srf, CharObj));
	    IPFreeObject(CharObj);
	}
    }

    if (LastCharObj)
	IPFreeObject(LastCharObj);

    IPListObjectInsert(RetObjs, RetObjIndex, NULL);
    return RetObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traverses the hierarch of objects found in CharObj and process the       *
* leaves through the composition with Srf.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:          Surface to warp the text Txt along.  The text is laid      *
*		  along the u axis with the v axis being the height.         *
*   CharObj:      A hierachy of curves/polys that prescribes the shape of    *
*		  character to compose with surface Srf.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A hierarchy of warped curves.                        *
*****************************************************************************/
static IPObjectStruct *UserWarpTextOneChar(CagdSrfStruct *Srf,
					   IPObjectStruct *CharObj)
{
    IPObjectStruct *RetObj;

    if (IP_IS_OLST_OBJ(CharObj)) {
        int j = 0;
	IPObjectStruct *PObj;

	RetObj = IPGenLISTObject(NULL);

        while ((PObj = IPListObjectGet(CharObj, j)) != NULL) {
	    IPListObjectInsert(RetObj, j++, UserWarpTextOneChar(Srf, PObj));
	}
	IPListObjectInsert(RetObj, j, NULL);
    }
    else if (IP_IS_POLY_OBJ(CharObj)) {
        /* Convert to a linear Bspline curve and then compose. */
        CagdCrvStruct
	    *Crv = IPPolyline2Curve(CharObj -> U.Pl, 2);

	RetObj = IPGenCRVObject(SymbComposeSrfCrv(Srf, Crv));
	CagdCrvFree(Crv);

    }
    else if (IP_IS_CRV_OBJ(CharObj)) {
        RetObj = IPGenCRVObject(SymbComposeSrfCrv(Srf, CharObj -> U.Crvs));

    }
    else {
        USER_FATAL_ERROR(USER_ERR_NON_CRV_OBJ_IN_FONT);
	RetObj = NULL;
    }

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Estimates the distance between the current character and the last one    *
* by sampling points on both and looking for the minimal distance.           *
*                                                                            *
* PARAMETERS:                                                                *
*   LastCharObj:  Previous character as list of Bezier curves, NULL if none. *
*   CharObj:      Current character as list of Bezier curves.		     *
*   CrntSpace:    The space required from last character.		     *
*   Ligature:     parameter between zero and one on amount of Ligature.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   CharObj translated horizontally toward CrntSpace.    *
*****************************************************************************/
static IPObjectStruct *UserWarpTextRepositionChar(IPObjectStruct *LastCharObj,
						  IPObjectStruct *CharObj,
						  IrtRType *CrntSpace,
						  IrtRType Ligature)
{
    int i;
    IrtRType
	AccumXTrans = 0.0;
    IrtHmgnMatType Mat;
    IPVertexStruct *V1, *V2;
    IPObjectStruct *LastPts, *Pts, *RetObj, *TmpObj;

    if (LastCharObj == NULL)
	return IPCopyObject(NULL, CharObj, FALSE);

    /* Get the necessary point data. */
    if ((LastPts = AttrGetObjectObjAttrib(LastCharObj, "_SampPts")) == NULL) {
	LastPts = IPGenPOLYLINEObject(
		  IPAllocPolygon(0, UserWarpTextSamplePts(LastCharObj), NULL));
	AttrSetObjectObjAttrib(LastCharObj, "_SampPts", LastPts, FALSE);
    }

    Pts = IPGenPOLYLINEObject(IPAllocPolygon(0, UserWarpTextSamplePts(CharObj),
					     NULL));

    RetObj = IPCopyObject(NULL, CharObj, FALSE);

    for (i = 0; i < USER_WT_MAX_ITER_LIGATURE; i++) {
        IrtRType XTrans,
	    MinDistSqr = IRIT_INFNTY;

        /* Do an n^2 comparison for the minimal distance. */
        for (V1 = LastPts -> U.Pl -> PVertex; V1 != NULL; V1 = V1 -> Pnext) {
	    for (V2 = Pts -> U.Pl -> PVertex; V2 != NULL; V2 = V2 -> Pnext) {
	        IrtVecType V;
		IrtRType DistSqr;

		IRIT_VEC_SUB(V, V1 -> Coord, V2 -> Coord);
		DistSqr = IRIT_DOT_PROD(V, V);

		if (MinDistSqr > DistSqr)
		    MinDistSqr = DistSqr;
	    }
	}

	/* Now translate CharObj horizontally to CrntSpace distance. */
	XTrans = sqrt(MinDistSqr) - *CrntSpace;
	AccumXTrans -= XTrans;

	for (V2 = Pts -> U.Pl -> PVertex; V2 != NULL; V2 = V2 -> Pnext)
	    V2 -> Coord[0] -= XTrans;
	
	if (IRIT_FABS(XTrans / *CrntSpace) < USER_WT_REL_ERR_LIGATURE)
	    break;
    }

    *CrntSpace = AccumXTrans * Ligature;

    MatGenMatTrans(AccumXTrans * Ligature, 0.0, 0.0, Mat);
    TmpObj = GMTransformObject(RetObj, Mat);
    IPFreeObject(RetObj);
    RetObj = TmpObj;
    AttrSetObjectObjAttrib(RetObj, "_SampPts", Pts, FALSE);

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Samples points along the different curves and polys of the char.         *
*                                                                            *
* PARAMETERS:                                                                *
*   CharObj:      A hierachy of curves/polys that prescribes the shape of    *
*		  character to compose withe surface Srf.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   A list of vertices holding the sampled points.       *
*****************************************************************************/
static IPVertexStruct *UserWarpTextSamplePts(IPObjectStruct *CharObj)
{
    IPVertexStruct
	*VList = NULL;

    if (IP_IS_OLST_OBJ(CharObj)) {
        int j = 0;
	IPObjectStruct *PObj;

        while ((PObj = IPListObjectGet(CharObj, j++)) != NULL) {
	    VList = IPAppendVrtxLists(UserWarpTextSamplePts(PObj), VList);
	}
    }
    else if (IP_IS_POLY_OBJ(CharObj)) {
        /* Convert to a linear Bspline curve and then compose. */
        CagdCrvStruct
	    *Crv = IPPolyline2Curve(CharObj -> U.Pl,2 );

	VList = UserWarpTextSampleCrvPts(Crv);

	CagdCrvFree(Crv);
    }
    else if (IP_IS_CRV_OBJ(CharObj)) {
	VList = UserWarpTextSampleCrvPts(CharObj -> U.Crvs);
    }
    else {
        USER_FATAL_ERROR(USER_ERR_NON_CRV_OBJ_IN_FONT);
	VList = NULL;
    }

    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to sample one curve.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:           To sample.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   A list of vertices holding the sampled points.       *
*****************************************************************************/
static IPVertexStruct *UserWarpTextSampleCrvPts(CagdCrvStruct *Crv)
{
    CagdBType
	NewCrv = FALSE;
    CagdPolylineStruct *CagdPoly;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv)) {
	Crv = BspCrvOpenEnd(Crv);
	NewCrv = TRUE;
    }

    CagdPoly = CagdCrv2Polyline(Crv, USER_WT_SAMPLES_PER_CURVE, FALSE);

    if (NewCrv)
	CagdCrvFree(Crv);

    Pl = IPCagdPllns2IritPllns(CagdPoly);
    V = Pl -> PVertex;

    Pl -> PVertex = NULL;
    IPFreePolygon(Pl);

    return V;
}
