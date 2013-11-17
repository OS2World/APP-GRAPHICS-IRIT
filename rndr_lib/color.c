/*****************************************************************************
* Color of the scan-line pixel evaluation algorithms.                        *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
* Modified by:  David Shafrir & Alex Reicher            Aug. 2003            *
*****************************************************************************/

#include "rndr_loc.h"
#include "color.h"

IRIT_GLOBAL_DATA IRndrColorType Colors[] =
{
    { 0,    0,    0 },                                  /*  0. Black.        */
    { 0,    0,    1 },                                  /*  1. Blue.         */
    { 0,    1,    0 },                                  /*  2. Green.        */
    { 0,    1,    1 },                                  /*  3. Cyan.         */
    { 1,    0,    0 },                                  /*  4. Red.          */
    { 1,    0,    1 },                                  /*  5. Magenta.      */
    { 0.5,  0.5,  0 },                                  /*  6. Brown.        */
    { 0.5,  0.5,  0.5 },                                /*  7. Lightgrey.    */
    { 0.25, 0.25, 0.25 },                               /*  8. Darkgray.     */
    { 0.25, 0.25, 1 },                                  /*  9. Lightblue.    */
    { 0.25, 1,    0.25 },                               /* 10. Lightgreen.   */
    { 0.25, 1,    1 },                                  /* 11. Lightcyan.    */
    { 1,    0.25, 0.25 },                               /* 12. Lightred.     */
    { 1,    0.25, 1 },                                  /* 13. Lightmagenta. */
    { 1,    1,    0.25 },                               /* 14. Yellow.       */
    { 1,    1,    1 }                                   /* 15. White.        */
};

static void ColorAdd(IRndrColorType c,
                     const IRndrColorType l,
                     const IRndrColorType o,
                     const IRndrIntensivityStruct *i);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes intensivity diffuse and specular values using specular model of M
*   illumination (see Foily, Van Dam). Differs between point and vector      M
*   viewer, point and vector light sources, which is defined in Light object M
*   and by calling IS_VIEWER_POINT() function.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   l:       IN, pointer to the light source object.                         M
*   p:       IN, point for which intensivity is computing.                   M
*   n:       IN, normal to the surface in the point "p".                     M
*   o:       IN, pointer to the object with surface characteristics.         M
*   Scene:   IN, pointer to the scene the object belongs.                    M
*   i:       OUT, pointer to resulting intensivity object.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LightIntensivity, specular, shading                                      M
*****************************************************************************/
void LightIntensivity(IRndrLightStruct *l,
                      const IrtPtType p,
                      const IrtNrmlType n,
                      const IRndrObjectStruct *o,
                      IRndrSceneStruct *Scene,
		      IRndrIntensivityStruct *i)
{
    IrtRType CosTheta, CosAlpha, *Light, *Sight;  /* For efficiency reasons. */
    IRIT_STATIC_DATA IrtNrmlType Normal;

    if (l -> Type == RNDR_POINT_LIGHT) {
        IRIT_STATIC_DATA IrtPtType LightStorage;

        Light = &LightStorage[0];                  /* We elliminate copying. */
        IRIT_PT_SUB(Light, l -> Where, p);
        IRIT_PT_NORMALIZE(Light);
    }
    else
        Light = l -> Where;

    if (!Scene -> Matrices.ParallelProjection ) {
        IRIT_STATIC_DATA IrtPtType SightStorage;

        Sight = &SightStorage[0];
        IRIT_PT_SUB(Sight, Scene -> Matrices.Viewer, p);

        IRIT_PT_NORMALIZE(Sight);

    }
    else
        Sight = Scene -> Matrices.Viewer;

    i -> Diff = i -> Spec = 0.0;

    /* Minus corrects assumed direction - "to the center of sphere". */
    IRIT_PT_COPY(Normal, n);

    IRIT_PT_SCALE(Normal, -1.0);

    CosTheta = IRIT_DOT_PROD(Light, Normal);

    if (CosTheta > IRIT_EPS) {      /* Light passes from behind of the poly. */
        IRIT_STATIC_DATA IrtPtType Mirrored;

        i -> Diff = o -> KDiffuse * CosTheta;
        IRIT_PT_SCALE(Normal, 2 * CosTheta);
        IRIT_PT_SUB(Mirrored, Normal, Light);
        CosAlpha = IRIT_DOT_PROD(Sight, Mirrored);
        if (CosAlpha > IRIT_EPS) {
            i -> Spec = o -> KSpecular * pow(CosAlpha, (IrtRType) o -> Power);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Aux. function which adds to resulting color light source color scaled by *
*   diffuse-specualr composition.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   c:      IN, OUT, resulting color.                                        *
*   l:      IN, light source color.                                          *
*   o:      IN, object color.                                                *
*   i:      IN, pointer to an intensivity object.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ColorAdd(IRndrColorType c,
                     const IRndrColorType l,
                     const IRndrColorType o,
                     const IRndrIntensivityStruct *i)
{
    c[RNDR_RED_CLR]   +=
        l[RNDR_RED_CLR]   * (o[RNDR_RED_CLR]   * i -> Diff + i -> Spec);
    c[RNDR_GREEN_CLR] +=
        l[RNDR_GREEN_CLR] * (o[RNDR_GREEN_CLR] * i -> Diff + i -> Spec);
    c[RNDR_BLUE_CLR]  +=
        l[RNDR_BLUE_CLR]  * (o[RNDR_BLUE_CLR]  * i -> Diff + i -> Spec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   For scan line "y" and pixel "x" in it computes color value in [0, 1]     M
*   RGB format.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:    IN, the polygon the pixel belongs.                              M
*   x:       IN, scan line pixel position.                                   M
*   y:       IN, scan line number.                                           M
*   o:       IN, the object of the triangle.                                 M
*   Scene:   IN, the scene context.                                          M
*   Value:   IN, OUT, interpolation value.                                   M
*   r:       OUT, resulting color.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TriangleColorEval, RGB, intensivity, scan line, shading                  M
*****************************************************************************/
void TriangleColorEval(IPPolygonStruct *Poly,
		       int x,
		       int y,
		       IRndrObjectStruct *o,
		       IRndrSceneStruct *Scene,
		       IRndrInterpolStruct *Value,
		       IRndrColorType r)
{
    IRndrColorType Orig;
    IrtPtType p;
    int j,
        ShadeModel = Scene -> ShadeModel;
    IrtRType w = 1.0 / Value -> w;

    if (o -> noShade) {        /* Take care of special objects (pure color). */
        IRIT_PT_COPY(r, o -> Color);
        return;
    }

    /* Initialize current image point and transform to object space. */
    p[RNDR_X_AXIS] = x;
    p[RNDR_Y_AXIS] = y;
    p[RNDR_Z_AXIS] = Value -> z;
    MatMultPtby4by4(p, p, Scene -> Matrices.ViewInvMat);

    /* Transform normal to object space, using homogeneous coordinates     */
    /* interpolation.					                   */
    IRIT_PT_SCALE(Value -> n, w);

    /* Fill resulting color with (texture) color, or object color. */
    if (Value -> HasColor)
	IRIT_PT_COPY(r, Value -> c);
    else
        IRIT_PT_COPY(r, o -> Color);

    /* If no shading is request, abort now. */
    if (ShadeModel == RNDR_SHADING_NONE)
        return;

    switch (o -> Txtr.Type) {
        case TEXTURE_TYPE_RSTR:
            {
		IrtImgPixelStruct
		    *Pxl = TextureImageGetPixel(&o -> Txtr,
						o -> Txtr.PrmImage,
						p,
						Value -> v * w,
						Value -> u * w,
						Poly);

		r[RNDR_RED_CLR]   = Pxl -> r;
		r[RNDR_GREEN_CLR] = Pxl -> g;
		r[RNDR_BLUE_CLR]  = Pxl -> b;
		r[RNDR_ALPHA_CLR] = o -> Txtr.PrmImage -> Alpha ?
			             ((IrtImgRGBAPxlStruct *) Pxl) -> a : 255;
		IRIT_PT4D_SCALE(r, 1.0 / 255);
	    }
	    break;
	case TEXTURE_TYPE_PROC:
	    {
		CagdRType Uv[2];

		Uv[0] = Value -> u * w;
		Uv[1] = Value -> v * w;

		IRIT_PT_NORMALIZE(Value -> n);
		p[0] *= o -> Txtr.tScale[0];
		p[1] *= o -> Txtr.tScale[1];
		p[2] *= o -> Txtr.tScale[2];
		IRIT_PT_COPY(Orig, r);
		(*o -> Txtr.vTexture)(&o -> Txtr, p, Value -> n, Uv, r);
		r[0] *= Orig[0];
		r[1] *= Orig[1];
		r[2] *= Orig[2];
	    }
	    break;
	case TEXTURE_TYPE_SRF:
	    {
	        CagdRType t, UMin, VMin, UMax, VMax, *Pt,
		    u = o -> Txtr.SrfParamDomain[0][2] *
			    (Value -> u *  w) +
				o -> Txtr.SrfParamDomain[0][0],
		    v = o -> Txtr.SrfParamDomain[1][2] *
			    (Value -> v * w) +
		                o -> Txtr.SrfParamDomain[1][0];

	        CagdSrfDomain(o -> Txtr.Srf, &UMin, &UMax, &VMin, &VMax);
		u = IRIT_BOUND(u, UMin, UMax);
		v = IRIT_BOUND(v, VMin, VMax);
		Pt = CagdSrfEval(o -> Txtr.Srf, u, v);

		switch (o -> Txtr.Srf -> PType) {
		    default:
		    case CAGD_PT_E1_TYPE:
		    case CAGD_PT_P1_TYPE:
		        t = Pt[1];
			break;
		    case CAGD_PT_E2_TYPE:
		    case CAGD_PT_P2_TYPE:
			t = sqrt(IRIT_SQR(Pt[1]) + IRIT_SQR(Pt[2]));
			break;
		    case CAGD_PT_E3_TYPE:
		    case CAGD_PT_P3_TYPE:
			t = sqrt(IRIT_SQR(Pt[1]) +
				 IRIT_SQR(Pt[2]) +
				 IRIT_SQR(Pt[3]));
			break;
		}
		t = CAGD_IS_RATIONAL_SRF(o -> Txtr.Srf) ? t / Pt[0] : t;

		switch (o -> Txtr.SrfFunc) {
		    case STEXTURE_FUNC_SQRT:
		        t = sqrt(IRIT_FABS(t));
			break;
		    case STEXTURE_FUNC_ABS:
			t = IRIT_FABS(t);
			break;
		    }

		t = (t - o -> Txtr.SrfScaleMinMax[0]) /
		    (o -> Txtr.SrfScaleMinMax[1] -
		     o -> Txtr.SrfScaleMinMax[0]);
		t = IRIT_BOUND(t, 0.0, 1.0);

		if (o -> Txtr.SrfScale != NULL) {
		    IrtImgPixelStruct
			*Pxl = TextureImageGetPixel(&o -> Txtr,
						    o -> Txtr.SrfScale,
						    p, t, 0, Poly);

		    r[RNDR_RED_CLR]   = Pxl -> r;
		    r[RNDR_GREEN_CLR] = Pxl -> g;
		    r[RNDR_BLUE_CLR]  = Pxl -> b;
		    r[RNDR_ALPHA_CLR] = o -> Txtr.PrmImage -> Alpha ?
			             ((IrtImgRGBAPxlStruct *) Pxl) -> a : 255;
		    IRIT_PT4D_SCALE(r, 1.0 / 255);
		}
		else {
		    IrtRType
			t1 = 1.0 - t;
		    
		    /* Use our own scale: */
		    r[RNDR_RED_CLR]   = t * t;
		    r[RNDR_GREEN_CLR] = 2 * t * t1;
		    r[RNDR_BLUE_CLR]  = t1 * t1;
		}
	    }
	    break;
    }

    IRIT_PT_COPY(Orig, r);
    IRIT_PT_SCALE(r, Scene -> Ambient); /* Compose ambient color in result. */
    if (ShadeModel == RNDR_SHADING_PHONG)    /* Ensure nrml is a real nrml. */
        IRIT_PT_NORMALIZE(Value -> n);

    /* Add to the resulting color diffuse and specular components from all  */
    /* light sources, check shadows by the way.                             */
    for (j = 0; j < Scene -> Lights.n ; j++) {
        if (ShadeModel == RNDR_SHADING_PHONG) {
            IRndrIntensivityStruct i;

            LightIntensivity(&Scene -> Lights.Src[j],
			     p, Value -> n, o, Scene, &i);
            ColorAdd(r, Scene -> Lights.Src[j].Color, Orig, &i);
        }
	else {
            if (ShadeModel == RNDR_SHADING_GOURAUD) {
		Value -> i[j].Diff *= w;
		Value -> i[j].Spec *= w;
            }
            ColorAdd(r, Scene -> Lights.Src[j].Color, Orig, &Value -> i[j]);
        }
    }

    RNDR_MINM(r[RNDR_RED_CLR], 1);   /* Ensure color is in [0, 1] interval. */
    RNDR_MINM(r[RNDR_GREEN_CLR], 1);
    RNDR_MINM(r[RNDR_BLUE_CLR], 1);
}
