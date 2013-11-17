/*****************************************************************************
* Routines to compute various textures color values.                         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Bassarab Dmitri & Plavnik Michael       Ver 0.2, Apr. 1995    *
* Modified by:  Raviv Ran                                       Aug. 1997    *
* Modified by:  David Shafrir & Alex Reicher            Aug. 2003            *
*****************************************************************************/

#include "texture.h"
#include "geom_lib.h"
#include "rndr_loc.h"

#define XOR(x,y)    ((x) ^ (y))

static IrtRType *BaryCentric3Pts(IrtPtType Pt1,
				 IrtPtType Pt2,
				 IrtPtType Pt3, IrtPtType Pt);
static SmoothNoiseStruct *InitSmoothNoise(IrtRType Min,
					  IrtRType Max,
					  int n,
					  int Cyclic);
static NoiseStruct *InitNoise(IrtRType Scale);
static IrtRType SmoothNoise(IrtRType x, SmoothNoiseStruct *Noise);
static IrtRType Noise3D(IrtPtType p, NoiseStruct *Noise);
static IrtRType Turbulence(IrtPtType p, NoiseStruct *Noise);
static void MarbleColor(IrtRType x, IRndrColorType Color);
static int SetParAux(char *p, char *Name, IrtRType Dflt, IrtRType *Dest);
static int Set2ParAux(char *p,
		      char *Name,
		      IrtRType Dflt1,
		      IrtRType Dflt2,
		      IrtRType *Dest1,
		      IrtRType *Dest2);

IRIT_GLOBAL_DATA ProcIRndrTextureStruct ProcTextures[] =
{
    { "chocolate", TextureBumpChocolate },            /* Bump texture maps. */
    { "orange",   TextureBumpOrange },

    { "camouf",   TextureCamouf },                 /* Regular texture maps. */
    { "checker",  TextureChecker },
    { "marble",   TextureMarble },
    { "wood",     TextureWood },
    { "contour",  TextureContour },
    { "ncontour", TextureContourNormal },
    { "curvature", TextureCurvature },
    { "punky",    TexturePunky },
    { NULL,       NULL}
};

static int Set3ParAux(char *p, char *Name, IrtVecType Dflt, IrtVecType Dest);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets image pixel by two real coordinates u and v from the Image data.    M
*   Access function.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:    IN, general attributes and modifiers of the texture mapping.    M
*   i:       IN, pointer to the Image data structure.                        M
*   p:       IN, location in Euclidean space of texture color to evaluate.   M
*   v:       IN, real coordinate of the image pixel.                         M
*   u:       IN, real coordinate of the image pixel.                         M
*   Poly:    IN, pointer to the polygon.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *:  value of the image pixel at (u,v) point.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureImageGetPixel, image, texture                                     M
*****************************************************************************/
IrtImgPixelStruct *TextureImageGetPixel(IRndrTextureStruct *Txtr,
					IRndrImageStruct *i,
					IrtPtType p,
					IrtRType v,
					IrtRType u,
					IPPolygonStruct *Poly)
{
    int x, y, l;
    IrtRType t, Theta, Phi, *r;
    IrtVecType V;
    IrtPtType Pt;
    IPPolygonStruct *PlOrig, *PlBjct;

    switch (Txtr -> PrmTextureType) {
        default:
        case PTEXTURE_UV_TYPE:
            x = ((int) (u * i -> xSize * Txtr -> PrmUScale));
	    y = ((int) (v * i -> ySize * Txtr -> PrmVScale));
	    break;
	case PTEXTURE_SPHERICAL_TYPE:
	    IRIT_VEC_SUB(V, p, Txtr -> PrmOrg);
	    IRIT_VEC_NORMALIZE(V);
	    
	    /* Rotate vector to texture space. */
	    MatMultVecby4by4(V, V, Txtr -> PrmMat);

	    /* Compute and normalize Theta and Phi to be between [0, 1]. */
	    Theta = (asin(V[2]) + M_PI_DIV_2) / M_PI;
	    Phi = (atan2(V[1], V[0]) + IRIT_DEG2RAD(Txtr -> PrmAngle) + M_PI) /
		                                                (M_PI_MUL_2);

	    x = ((int) (Phi * i -> xSize * Txtr -> PrmUScale));
	    y = ((int) (Theta * i -> ySize * Txtr -> PrmVScale));
	    break;
	case PTEXTURE_SPHERE_BIJECT_TYPE:
	    PlBjct = NULL;
	    if ((PlOrig = (IPPolygonStruct *)
		 AttrGetPtrAttrib(Poly -> Attr, "_OrigPoly")) == NULL ||
		(PlBjct = (IPPolygonStruct *)
		                 AttrGetPtrAttrib(Poly -> Attr,
						  "_BjctPoly")) == NULL) {
		_IRndrReportFatal(IRIT_EXP_STR("Failed to extract polygons in bijective spherical texture map.\n"));
	    }
	    r = BaryCentric3Pts(PlOrig -> PVertex -> Coord,
				PlOrig -> PVertex -> Pnext -> Coord,
				PlOrig -> PVertex -> Pnext -> Pnext -> Coord,
				p);

	    for (l = 0; l < 3; l++) {
		Pt[l] = r[0] * PlBjct -> PVertex -> Coord[l] +
		    r[1] * PlBjct -> PVertex -> Pnext -> Coord[l] +
			r[2] * PlBjct -> PVertex -> Pnext -> Pnext -> Coord[l];
	    }

	    IRIT_VEC_SUB(V, Pt, Txtr -> PrmOrg);
	    IRIT_VEC_NORMALIZE(V);

	    /* Rotate vector to texture space. */
	    MatMultVecby4by4(V, V, Txtr -> PrmMat);

	    /* Compute and normalize Theta and Phi to be between [0, 1]. */
	    Theta = (asin(V[2]) + M_PI_DIV_2) / M_PI;
	    Phi = (atan2(V[1], V[0]) + IRIT_DEG2RAD(Txtr -> PrmAngle) + M_PI) /
		                                                (M_PI_MUL_2);

	    x = ((int) (Phi * i -> xSize * Txtr -> PrmUScale));
	    y = ((int) (Theta * i -> ySize * Txtr -> PrmVScale));
	    break;
	case PTEXTURE_CYLINDERICAL_TYPE:
	    GMPointFromPointLine(p, Txtr -> PrmOrg, Txtr -> PrmDir, Pt);
	    IRIT_VEC_SUB(V, p, Pt);

	    /* Rotate vector to texture space. */
	    MatMultVecby4by4(V, V, Txtr -> PrmMat);

	    Phi = (atan2(V[1], V[0]) + IRIT_DEG2RAD(Txtr -> PrmAngle) + M_PI) /
		                                                (M_PI_MUL_2);

	    x = ((int) (p[2] * i -> xSize * Txtr -> PrmUScale));
	    y = ((int) (Phi * i -> ySize * Txtr -> PrmVScale));
	    break;
	case PTEXTURE_PLANAR_TYPE:
	    /* Rotate vector to texture space. */
	    IRIT_VEC_SUB(V, p, Txtr -> PrmOrg);
	    MatMultVecby4by4(V, V, Txtr -> PrmMat);

	    /* Rotate around the Z axis by the angular prescription. */
	    t = V[0] * cos(IRIT_DEG2RAD(Txtr -> PrmAngle)) -
		V[1] * sin(IRIT_DEG2RAD(Txtr -> PrmAngle));
	    V[1] = V[0] * sin(IRIT_DEG2RAD(Txtr -> PrmAngle)) +
		   V[1] * cos(IRIT_DEG2RAD(Txtr -> PrmAngle));
	    V[0] = t;

	    x = ((int) (V[0] * i -> xSize * Txtr -> PrmUScale));
	    y = ((int) (V[1] * i -> ySize * Txtr -> PrmVScale));
	    break;
    }

    x = x % (i -> xSize + 1);
    if (x < 0)
        x += i -> xSize;
    y = y % (i -> ySize + 1);
    if (y < 0)
        y += i -> ySize;

    return i -> Alpha ? (IrtImgPixelStruct *)
					&i -> U.RGBA[y * (i -> xSize + 1) + x]
		      : &i -> U.RGB[y * (i -> xSize + 1) + x];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the Barycentric coordinates of given point Pt with respect to   *
* given Triangle Pt1 Pt2 Pt3. All points are assumed to be coplanar.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2, Pt3:  Three points forming a triangular in general position.   *
*   Pt:             A point for which the barycentric coordinates are to be  *
*                   computed.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *: A pointers to a static space holding the three Barycentric   *
*               coefficients, even if outside the triangle as we expect this *
*               to occur due to round off error and integer arithmetic.      *
*****************************************************************************/
static IrtRType *BaryCentric3Pts(IrtPtType Pt1,
				 IrtPtType Pt2,
				 IrtPtType Pt3,
				 IrtPtType Pt)
{
    IRIT_STATIC_DATA IrtVecType RetVal;
    IrtVecType V1, V2, V3, X12, X23, X31;
    IrtRType r;

    IRIT_PT_SUB(V1, Pt, Pt1);
    IRIT_PT_SUB(V2, Pt, Pt2);
    IRIT_PT_SUB(V3, Pt, Pt3);

    IRIT_CROSS_PROD(X12, V1, V2);
    IRIT_CROSS_PROD(X23, V2, V3);
    IRIT_CROSS_PROD(X31, V3, V1);

    RetVal[0] = sqrt(IRIT_DOT_PROD(X23, X23));
    RetVal[1] = sqrt(IRIT_DOT_PROD(X31, X31));
    RetVal[2] = sqrt(IRIT_DOT_PROD(X12, X12));

    if ((r = RetVal[0] + RetVal[1] + RetVal[2]) > 0.0) {
	r = 1.0 / r;
	IRIT_PT_SCALE(RetVal, r);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Allocates and initializes a new SmoothNoiseStruct.                         *
* Sets random values and calculate the polynom coefficients for each         *
*   interval.                                                                *
*   Should be called before "SmoothNoise" function is actualy called.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Min:     IN, beginning of the range.                                     *
*   Max:     IN, end of the range.                                           *
*   n:       IN, number of points to sample noise values.                    *
*   cyclic:  IN, flag for making the noise cyclic.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   SmoothNoiseStruct *:  Pointer to the new SmoothNoiseStruct.              *
*****************************************************************************/
static SmoothNoiseStruct *InitSmoothNoise(IrtRType Min,
					  IrtRType Max,
					  int n,
					  int Cyclic)
{
    int i;
    IrtRType f0, f1, d0, d1, *Values, *Dx;
    SmoothNoiseStruct *NewNoise;

    Values = RNDR_MALLOC(IrtRType, n);
    Dx = RNDR_MALLOC(IrtRType, n);

    NewNoise = RNDR_MALLOC(SmoothNoiseStruct, sizeof(SmoothNoiseStruct));
    NewNoise -> n = n;
    NewNoise -> Min = Min;
    NewNoise -> Max = Max;

    /* Set random noise values and differential values. */
    for (i = 0; i < n; i++) {
        Values[i] = IritRandom(0, 1);
        Dx[i]     = IritRandom(-2, 2);
    }
    if (Cyclic) {
        Values[n - 1] = Values[0];
        Dx[n - 1] = Dx[0];
    }

    /* Calculate coefficients for cubic interpolations. */
    for (i = 0; i < n - 1; i++) {
        f0 = Values[i];
        f1 = Values[i + 1];
        d0 = Dx[i] * (Max - Min) / (n - 1);
        d1 = Dx[i + 1] * (Max - Min) / (n - 1);
        NewNoise -> Coefs[i][0] = 2 * (f0 - f1) + d0 + d1;
        NewNoise -> Coefs[i][1] = 3 * (f1 - f0) - 2 * d0 - d1;
        NewNoise -> Coefs[i][2] = d0;
        NewNoise -> Coefs[i][3] = f0;
    }
    RNDR_FREE(Dx);
    RNDR_FREE(Values);

    return NewNoise;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates and initializes a new NoiseStruct.                             *
*   Should be called before "Noise" function is actualy called.              *
*                                                                            *
* PARAMETERS:                                                                *
*   scale:   IN, scale of the Lattice.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   NoiseStruct *:  Pointer to the new NoiseStruct.                          *
*****************************************************************************/
static NoiseStruct *InitNoise(IrtRType Scale)
{
    int x, y, z, xx, yy, zz;
    NoiseStruct *NewNoise;

    NewNoise = RNDR_MALLOC(NoiseStruct, 1);
    NewNoise -> Scale = Scale;
    for (x = 0; x < RNDR_MAX_NOISE; x++)
        for (y = 0; y < RNDR_MAX_NOISE; y++)
            for (z = 0; z < RNDR_MAX_NOISE; z++) {
                NewNoise -> Lattice[x][y][z] = IritRandom(0.0, 1.0);
                xx = (x + 1 == RNDR_MAX_NOISE) ? 0 : x;
                yy = (y + 1 == RNDR_MAX_NOISE) ? 0 : y;
                zz = (z + 1 == RNDR_MAX_NOISE) ? 0 : z;
                NewNoise -> Lattice[x][y][z] = NewNoise -> Lattice[xx][yy][zz];
            }

    return NewNoise;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Determines actual noise value for the point.                             *
*   Assumes that InitSmoothNoise function is called before to initialize     *
*   the coeficients values.                                                  *
*   The value is calculated using a cubic interpolation of the two near      *
*   points.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   x:       IN, the point.                                                  *
*   noise:   IN, pointer to the noise module to calculate from.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  noise value computed.                                         *
*****************************************************************************/
static IrtRType SmoothNoise(IrtRType x, SmoothNoiseStruct *Noise)
{
    int i;
    IrtRType Offset, IntervalSize, f, t;

    while (x > Noise -> Max)
        x = x - (Noise -> Max - Noise -> Min);
    while (x < Noise -> Min)
        x = x + (Noise -> Max - Noise -> Min);

    IntervalSize = (Noise -> Max - Noise -> Min) / (Noise -> n - 1);
    Offset = x - Noise -> Min;
    i = (int) floor(Offset / IntervalSize);

    /* Normalize so that the polynom paramter t is between 0 and 1. */
    t = fmod(Offset, IntervalSize) / IntervalSize;

    i = i % (Noise -> n - 1);
    f = Noise -> Coefs[i][3]         + Noise -> Coefs[i][2] * t +
        Noise -> Coefs[i][1] * t * t + Noise -> Coefs[i][0] * t * t * t;

    return f;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Determines actual noise value for the point.                             *
*   Assumes that InitNoise function is called before to initialize the noise *
*   Lattice values.                                                          *
*   The value is calculated using a linear interpolation of the 8 near       *
*   points.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   p:       IN, the point.                                                  *
*   noise:   IN, pointer to the noise module to calculate from.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  noise value computed.                                         *
*****************************************************************************/
static IrtRType Noise3D(IrtPtType p, NoiseStruct *Noise)
{
    double ox, oy, oz, n00, n01, n10, n11, n0, n1, n,
	x = p[RNDR_X_AXIS],
	y = p[RNDR_Y_AXIS],
	z = p[RNDR_Z_AXIS];
    int ix, iy, iz, ixx, iyy, izz;

    /* Move coordinates to positive location to simplify later */
    /* calculations.                                           */
    x += 10000000;
    y += 10000000;
    z += 10000000;

    x *= Noise -> Scale;
    z *= Noise -> Scale;
    y *= Noise -> Scale;

    ox = x - floor(x);
    oy = y - floor(y);
    oz = z - floor(z);

    ix = (int) x % RNDR_MAX_NOISE;
    iy = (int) y % RNDR_MAX_NOISE;
    iz = (int) z % RNDR_MAX_NOISE;

    ixx = (ix + 1) % RNDR_MAX_NOISE;
    iyy = (iy + 1) % RNDR_MAX_NOISE;
    izz = (iz + 1) % RNDR_MAX_NOISE;

    n   = Noise -> Lattice[ix][iy][iz];
    n00 = n + ox * (Noise -> Lattice[ixx][iy][iz] - n);
    n   = Noise -> Lattice[ix][iy][izz];
    n01 = n + ox * (Noise -> Lattice[ixx][iy][izz] - n);
    n   = Noise -> Lattice[ix][iyy][iz];
    n10 = n + ox * (Noise -> Lattice[ixx][iyy][iz] - n);
    n   = Noise -> Lattice[ix][iyy][izz];
    n11 = n + ox * (Noise -> Lattice[ixx][iyy][izz] - n);

    n0  = n00 + oy * (n10 - n00);
    n1  = n01 + oy * (n11 - n01);
    n   = n0  + oz * (n1  - n0);
    return (IrtRType) n;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Aux. function for MarbleTexture. Computes turbulence kind of noise       *
*   for the specific point in space.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   p:       IN, the point.                                                  *
*   noise:   IN, pointer to the NoiseStruct used for the turbulence.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  noise value computed.                                         *
*****************************************************************************/
static IrtRType Turbulence(IrtPtType p, NoiseStruct *Noise)
{
    IrtRType
	t = 0.0,
	Scale = 1.0;
    IrtPtType Pt;

    IRIT_PT_COPY(Pt, p);
    while (Scale > 0.01) {
        IRIT_PT_COPY(Pt, p);
        IRIT_PT_SCALE(Pt, 1 / Scale);
        t += IRIT_FABS(Noise3D(Pt, Noise) * Scale);
        Scale = Scale * 0.5;
    }
    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Aux. function to MarbleTexture.                                          *
*   Calculate the color from a scallar in the range 0-1.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   x:       IN, the scallar.                                                *
*   Color:   OUT, the calculated marble color.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MarbleColor(IrtRType x, IRndrColorType Color)
{
    IRIT_STATIC_DATA IRndrColorType
	Colors[5] = {
            { 0.95294, 0.92594, 0.90588 },
            { 0.95294, 0.92594, 0.90588 },
            { 0.88235, 0.80392, 0.74901 },
            { 0.46274, 0.22745, 0.41568 },
            { 0.46274, 0.30196, 0.22745 }
        };
    IRIT_STATIC_DATA IrtRType
	Edges[5] = { 0.0, 0.005, 0.7, 1, 1 };
    int i = 0;
    IrtRType t;

    if (x == 1)
        x = 0.999999;
    while (x > Edges[i + 1])
        i++;
    t = (x - Edges[i]) / (Edges[i + 1] - Edges[i]);

    Color[0] = Colors[i][0] + t * (Colors[i + 1][0] - Colors[i][0]);
    Color[1] = Colors[i][1] + t * (Colors[i + 1][1] - Colors[i][1]);
    Color[2] = Colors[i][2] + t * (Colors[i + 1][2] - Colors[i][2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "marble" texture.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureMarble, texture, image warping                                    M
*****************************************************************************/
void TextureMarble(IRndrTextureStruct *Txtr,
                   IrtPtType Point,
                   IrtNrmlType Normal,
		   IrtRType *Uv,
                   IRndrColorType Color)
{
    IRIT_STATIC_DATA NoiseStruct *Noise;
    IRIT_STATIC_DATA int
	InitNoiseFlag = FALSE;
    IrtRType m, x, Freq, TurFactor, NoiseScale;
    IrtPtType Pt;

    /* Get texture parameters. */
    NoiseScale = 0.2 * Txtr -> TurbulenceScale;
    TurFactor = Txtr -> TurbulenceFactor;
    Freq = 0.2 * Txtr -> Frequency;

    /* Initial texture setup. */
    if (!InitNoiseFlag) {
        Noise = InitNoise(NoiseScale);
        InitNoiseFlag = TRUE;
    }

    /* Transform Point to texture space. */
    MatMultPtby4by4(Pt, Point, Txtr -> Mat);

    x = Pt[RNDR_Z_AXIS] + TurFactor * Turbulence(Pt, Noise);
    m = 0.5 * (1 + sin(Freq * x));
    MarbleColor(m, Color);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "wood" texture.                                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN OUT, color value at the point.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureWood                                                              M
*****************************************************************************/
void TextureWood(IRndrTextureStruct *Txtr,
		 IrtPtType Point,
                 IrtNrmlType Normal,
		 IrtRType *Uv,
                 IRndrColorType Color)
{
    IRIT_STATIC_DATA NoiseStruct *RadiusNoise, *FiberNoise;
    IRIT_STATIC_DATA SmoothNoiseStruct *WaveNoise, *oxNoise, *oyNoise;
    IRIT_STATIC_DATA int
	InitNoiseFlag = FALSE;
    int WaveNoisePoints;
    IrtRType CenterNoiseFactor, FiberFactor, FiberScale, WaveNoiseFactor,
        Angle, CenterNoiseScale, Brightness, RadNoiseFreq, ox, oy, m, Radius;
    IrtPtType Pt, p;

    /* Get texture paramteres. */
    Brightness = Txtr -> Brightness;
    CenterNoiseScale = Txtr -> CenterScale;
    CenterNoiseFactor = Txtr -> CenterFactor;
    WaveNoiseFactor = Txtr -> WaveFactor;
    WaveNoisePoints = (int) floor(Txtr -> WaveNPoints);
    RadNoiseFreq = Txtr -> Frequency;
    FiberScale = Txtr -> FiberScale;
    FiberFactor = Txtr -> FiberFactor;

    /* Initial texture setup. */
    if (!InitNoiseFlag) {
        InitNoise(0.1);
        RadiusNoise = InitNoise(5);
        FiberNoise  = InitNoise(10);
        WaveNoise = InitSmoothNoise(-M_PI, M_PI, WaveNoisePoints + 1, TRUE);
        oxNoise = InitSmoothNoise(0, 8 * CenterNoiseScale, 20, TRUE);
        oyNoise = InitSmoothNoise(0, 8 * CenterNoiseScale, 20, TRUE);
        InitNoiseFlag = TRUE;
    }

    /* Transform Point to texture space. */
    MatMultPtby4by4(Pt, Point, Txtr -> Mat);

    /* Insert noise in center of circles. */
    ox = CenterNoiseFactor * (0.5 - SmoothNoise(Pt[RNDR_Z_AXIS], oxNoise));
    oy = CenterNoiseFactor * (0.5 - SmoothNoise(Pt[RNDR_Z_AXIS], oyNoise));

    /* Calculate angle and radius. */
    Angle  = atan2(Pt[RNDR_Y_AXIS] - oy, Pt[RNDR_X_AXIS] - ox) ;
    Radius = sqrt((Pt[RNDR_X_AXIS] - ox) * (Pt[RNDR_X_AXIS] - ox) +
                  (Pt[RNDR_Y_AXIS] - oy) * (Pt[RNDR_Y_AXIS] - oy));

    /* Change circles to a wavy closed shape. */
    Radius += Radius * WaveNoiseFactor * SmoothNoise(Angle, WaveNoise);

    /* Insert noise to the rings width. */
    IRIT_PT_RESET(p);
    p[RNDR_X_AXIS] = Radius;
    Radius += Noise3D(p, RadiusNoise);

    /* Normalize to the range [0,1.0]. */
    m = 0.5 * (1 + sin(RadNoiseFreq * Radius));

    /* insert noise to simulate fibers. */
    IRIT_PT_COPY(p, Pt);
    p[RNDR_X_AXIS] = p[RNDR_X_AXIS] * 3 / FiberScale;
    p[RNDR_Y_AXIS] = p[RNDR_Y_AXIS] * 3 / FiberScale;
    p[RNDR_Z_AXIS] = p[RNDR_Z_AXIS] / 4 / FiberScale;
    m += FiberFactor * (Noise3D(p, FiberNoise) - m);

    /* Normalize to the range [Brightness,1.0]. */
    m += Brightness * (1 - m);
    IRIT_PT_SCALE(Color, m);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "chekcer" texture.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureChecker                                                           M
*****************************************************************************/
void TextureChecker(IRndrTextureStruct *Txtr,
                    IrtPtType Point,
                    IrtNrmlType Normal,
                    IrtRType *Uv,
                    IRndrColorType Color)
{
    int Invers, Column;
    IrtPtType Pt;

    /* Transform Point to texture space. */
    MatMultPtby4by4(Pt, Point, Txtr -> Mat);

    /* Move coordinates to positive location to fix the simetry of fmod. */
    Invers = XOR(Pt[RNDR_Y_AXIS] < 0, Pt[RNDR_X_AXIS] < 0);
    Pt[RNDR_X_AXIS] = IRIT_FABS(Pt[RNDR_X_AXIS]);
    Pt[RNDR_Y_AXIS] = IRIT_FABS(Pt[RNDR_Y_AXIS]);

    Column = XOR(fmod(Pt[RNDR_X_AXIS], 2.0) < 1.0,
             XOR(fmod(Pt[RNDR_Y_AXIS], 2.0) < 1.0, Invers));

    /* Make a plane checker in transformed XY plane. */
    if (Txtr -> CheckerPlane) {
	if (Column)
	    IRIT_PT_COPY(Color, Txtr -> CheckerColor[0]);
	return;
    }

    /* Make a 3D checker in transformed texture space. */
    if (Column)
	Column = 2;
    Invers = Pt[RNDR_Z_AXIS] < 0;
    Pt[RNDR_Z_AXIS] = IRIT_FABS(Pt[RNDR_Z_AXIS]);
    if (XOR(fmod(Pt[RNDR_Z_AXIS], 2.0) < 1.0, Invers))
	Column++;
    if (Column != 3)
	IRIT_PT_COPY(Color, Txtr -> CheckerColor[Column]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "camouf" texture.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureCamouf                                                            M
*****************************************************************************/
void TextureCamouf(IRndrTextureStruct *Txtr, IrtPtType Point,
                   IrtNrmlType Normal,
                   IrtRType *Uv,
                   IRndrColorType Color)
{
    IRIT_STATIC_DATA NoiseStruct *Ns;
    IRIT_STATIC_DATA int
	InitNoiseFlag = FALSE;
    IRIT_STATIC_DATA IRndrColorType
	Colors[5] = {
            { 0.1529, 0.4509, 0.1019 },
            { 0.5254, 0.5019, 0.0588 },
            { 0.3843, 0.6666, 0.1921 },
            { 0.8784, 0.8431, 0.0941 },
            { 0.6509, 0.4039, 0.1019 }
	};
    IrtPtType Pt;
    IrtRType n;

    /* Initial texture setup. */
    if (!InitNoiseFlag) {
        Ns = InitNoise(1);
        InitNoiseFlag = TRUE;
    }

    IRIT_PT_COPY(Pt, Point);

    n = Noise3D(Pt, Ns);
    if (n > 0.65)
        IRIT_PT_COPY(Color, Colors[0]);
    else if (n > 0.5)
        IRIT_PT_COPY(Color, Colors[1]);
    else if (n > 0.35)
        IRIT_PT_COPY(Color, Colors[2]);
    else
        IRIT_PT_COPY(Color, Colors[3]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "orange" bump texture.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TextureBumpChocolate                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureBumpOrange                                                        M
*****************************************************************************/
void TextureBumpOrange(IRndrTextureStruct *Txtr,
		       IrtPtType Point,
		       IrtNrmlType Normal,
		       IrtRType *Uv,
		       IRndrColorType Color)
{
    IRIT_STATIC_DATA NoiseStruct *Dx, *Dy, *Dz;
    IRIT_STATIC_DATA int
	InitNoiseFlag = FALSE;
    IrtRType Depth;
    IrtPtType Pt;

    /* Initial texture setup. */
    if (!InitNoiseFlag) {
        Dx = InitNoise(1);
        Dy = InitNoise(1);
        Dz = InitNoise(1);
        InitNoiseFlag = TRUE;
    }

    IRIT_PT_COPY(Pt, Point);
    Depth = Txtr -> Depth;
    Normal[RNDR_X_AXIS] += Depth * (Noise3D(Pt, Dx) - 0.5);
    Normal[RNDR_Y_AXIS] += Depth * (Noise3D(Pt, Dy) - 0.5);
    Normal[RNDR_Z_AXIS] += Depth * (Noise3D(Pt, Dz) - 0.5);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "orange" bump texture.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TextureBumpOrange                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureBumpChocolate                                                     M
*****************************************************************************/
void TextureBumpChocolate(IRndrTextureStruct *Txtr,
			  IrtPtType Point,
			  IrtNrmlType Normal,
			  IrtRType *Uv,
			  IRndrColorType Color)
{
    int IUFrac = 0,
        IVFrac = 0;
    IrtRType Depth,
        Min = IRIT_BOUND(Txtr -> Width, 0.0, 0.5),
        Max = 1.0 - Min,
        u = Uv[0] * Txtr -> tScale[0],
        v = Uv[1] * Txtr -> tScale[1],
        UFrac = u - ((int) u),
        VFrac = v - ((int) v);
    IrtVecType Dir1, Dir2;

    /* Tile the chocolate piece between zero and one in both U and V. */
    if (UFrac < Min)
        IUFrac = -1;
    else if (UFrac > Max)
        IUFrac = 1;

    if (VFrac < Min)
        IVFrac = -1;
    else if (VFrac > Max)
        IVFrac = 1;

    if (IUFrac == 0 && IUFrac == 0 && IVFrac == 0 && IVFrac == 0)
	return;                           /* On the chocolate piece itself. */

    if (Txtr -> OrigSrf == NULL) {
	Dir1[0] = Dir1[1] = 0.0;
	Dir1[2] = 1.0;
    }
    else {
	CagdRType UMin, VMin, UMax, VMax;
	CagdVecStruct *Vec;

	CagdSrfDomain(Txtr -> OrigSrf -> U.Srfs, &UMin, &UMax, &VMin, &VMax);
	Uv[0] = IRIT_BOUND(Uv[0], UMin, UMax);
	Uv[1] = IRIT_BOUND(Uv[1], VMin, VMax);

	Vec = CagdSrfTangent(Txtr -> OrigSrf -> U.Srfs, Uv[0], Uv[1],
			     CAGD_CONST_U_DIR, TRUE);

	IRIT_PT_COPY(Dir1, Vec -> Vec);
    }

    IRIT_CROSS_PROD(Dir2, Dir1, Normal);
    if (IRIT_PT_SQR_LENGTH(Dir2) > IRIT_SQR(IRIT_EPS))
	IRIT_PT_NORMALIZE(Dir2);

    Depth = Txtr -> Depth;

    IRIT_PT_SCALE(Dir1, Depth * IUFrac);
    IRIT_PT_SCALE(Dir2, Depth * IVFrac);
    IRIT_PT_ADD(Normal, Normal, Dir1);
    IRIT_PT_ADD(Normal, Normal, Dir2);
    IRIT_PT_NORMALIZE(Normal);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "contour" texture.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureContour, texture, image warping                                   M
*****************************************************************************/
void TextureContour(IRndrTextureStruct *Txtr,
		    IrtPtType Point,
		    IrtNrmlType Normal,
		    IrtRType *Uv,
		    IRndrColorType Color)
{
    int i;
    IrtPtType Pt;

    for (i = 0; i < 3; i++) {
	Pt[i] = Point[i] - ((int) Point[i]);
        if (Pt[i] < 0.0)
            Pt[i] += 1.0;

        if (Pt[i] < 0 || Pt[i] > 1.0)
            _IRndrReportWarning(IRIT_EXP_STR("Pt[i] = %f\n"), Pt[i]);
    }

    if ((Pt[0] > 0 && Pt[0] < Txtr -> Width) ||
	(Pt[1] > 0 && Pt[1] < Txtr -> Width) ||
	(Pt[2] > 0 && Pt[2] < Txtr -> Width)) {
	Color[0] = Txtr -> Color.r;
	Color[1] = Txtr -> Color.g;
	Color[2] = Txtr -> Color.b;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "ncontour" texture.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, normal at the point.                                         M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureContourNormal, texture, image warping                             M
*****************************************************************************/
void TextureContourNormal(IRndrTextureStruct *Txtr,
			  IrtPtType Point,
			  IrtNrmlType Normal,
			  IrtRType *Uv,
			  IRndrColorType Color)
{
    int i,
        DrawContour = FALSE;

    for (i = 0; i < 3; i++) {  /* Check angle of normal with the X/Y/Z axes. */
	IrtRType Angle,
            Scale = Txtr -> tScale[i];

	if (IRIT_APX_EQ(Scale, 0.0))
	    continue;
	Scale = 1.0 / Scale;

	for (Angle = 0; Angle < 90; Angle += Scale) {
	    IrtRType
		CosAngle = cos(IRIT_DEG2RAD(Angle));

	    if (IRIT_APX_EQ_EPS(CosAngle,
				IRIT_FABS(Normal[i]), Txtr -> Width)) {
		DrawContour = TRUE;
		break;
	    }
	}
    }

    if (DrawContour) {
	Color[0] = Txtr -> Color.r;
	Color[1] = Txtr -> Color.g;
	Color[2] = Txtr -> Color.b;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates a texture color that is a function of the curvature of the     M
* surface, getting the "curvature" texture.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, normal at the point.                                         M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureCurvature, texture, curvature                                     M
*****************************************************************************/
void TextureCurvature(IRndrTextureStruct *Txtr,
		      IrtPtType Point,
		      IrtNrmlType Normal,
		      IrtRType *Uv,
		      IRndrColorType Color)
{
    IrtRType Gauss, Mean, k1, k2;
    CagdVType D1, D2;

    if (AttrGetPtrAttrib(Txtr -> OrigSrf -> U.Srfs -> Attr,
			 "_EvalCurv") == NULL) {
	CagdSrfStruct
	    *GaussSrf = SymbSrfGaussCurvature(Txtr -> OrigSrf -> U.Srfs,
					      FALSE);
	CagdBBoxStruct BBox;

	CagdSrfBBox(GaussSrf, &BBox);
	Txtr -> CrvtrSrfMax = sqrt(IRIT_FABS(BBox.Max[0]));
	CagdSrfFree(GaussSrf);

	SymbEvalSrfCurvPrep(Txtr -> OrigSrf -> U.Srfs, TRUE);
    }

    SymbEvalSrfCurvature(Txtr -> OrigSrf -> U.Srfs, Uv[0], Uv[1], TRUE,
			 &k1, &k2, D1, D2);
    Mean = (k1 + k2) * 0.5;
    Gauss = k1 * k2;

    if (IRIT_APX_EQ(Txtr -> tScale[0], 0.0)) {
	/* Convex in red, concave in green, saddle-like in yellow. */
	if (Gauss < 0) {
	    Color[0] = 1.0;
	    Color[1] = 1.0;
	    Color[2] = 0.0;
	}
	else if (k1 < 0 && k2 < 0) {
	    Color[0] = 1.0;
	    Color[1] = 0.0;
	    Color[2] = 0.0;
	}
	else {
	    Color[0] = 0.0;
	    Color[1] = 1.0;
	    Color[2] = 0.0;
	}
    }
    else if (Txtr -> tScale[0] > 0.0) {
	IrtRType t;

	t = IRIT_FABS(Gauss * Txtr -> tScale[0] / Txtr -> CrvtrSrfMax);
	t = IRIT_BOUND(t, 0.0, 1.0);

	/* Convex in red<->magenta, concave in green<->yellow, saddle-like  */
	/* in cyan-blue.                                                    */
	if (Gauss < 0.0) {
	    Color[0] = 0.0;
	    Color[1] = 1.0 - t;
	    Color[2] = 1.0;
	}
	else if (k1 < 0 && k2 < 0) {
	    Color[0] = 1.0;
	    Color[1] = 0.0;
	    Color[2] = t;
	}
	else {
	    Color[0] = 1.0 - t;
	    Color[1] = 1.0;
	    Color[2] = 0.0;
	}
    }
    else {
	IrtRType t;

	t = IRIT_FABS(Mean * Txtr -> tScale[0] / Txtr -> CrvtrSrfMax);
	t = IRIT_BOUND(t, 0.0, 1.0);

	/* Mean < 0 in red<->magenta, Mean > 0 in green<->yellow. */
	if (Mean < 0) {
	    Color[0] = 1.0;
	    Color[1] = 0.0;
	    Color[2] = t;
	}
	else {
	    Color[0] = t;
	    Color[1] = 1.0;
	    Color[2] = 0.0;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates pertubation on color value at given point in order to get      M
*   "punky" style texture.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:   IN, pointer to the texture structure.                            M
*   Point:  IN, coordinate of the point in object space.                     M
*   Normal: IN, OUT, normal at the point.                                    M
*   Uv:     IN, uv parameteric domain's coordinates, if exists.              M
*   Color:  IN, OUT, color value at the point.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TexturePunky, texture, image warping                                     M
*****************************************************************************/
void TexturePunky(IRndrTextureStruct *Txtr,
		  IrtPtType Point,
		  IrtNrmlType Normal,
		  IrtRType *Uv,
		  IRndrColorType Color)
{
    IRIT_STATIC_DATA NoiseStruct *Dx, *Dy, *Dz;
    IRIT_STATIC_DATA int
	InitNoiseFlag = FALSE;
    int i;
    IrtRType Brightness;
    IrtPtType Pt;

    /* Initial texture setup. */
    if (!InitNoiseFlag) {
        Dx = InitNoise(1);
        Dy = InitNoise(1);
        Dz = InitNoise(1);
        InitNoiseFlag = TRUE;
    }

    Brightness = Txtr -> Brightness;
    IRIT_PT_COPY(Pt, Point);
    Color[RNDR_X_AXIS] = Noise3D(Pt, Dx);
    IRIT_PT_SCALE(Pt, 1.2);
    Color[RNDR_Y_AXIS] = Noise3D(Pt, Dy);
    IRIT_PT_SCALE(Pt, 1.3);
    Color[RNDR_Z_AXIS] = Noise3D(Pt, Dz);

    for (i = RNDR_X_AXIS; i <= RNDR_Z_AXIS; i++) {
        Color[i] = pow(Color[i], Brightness);
        Color[i] = IRIT_BOUND(Color[i], 0, 1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxilary function to function InitTextureParameters                        *
*****************************************************************************/
static int SetParAux(char *p, char *Name, IrtRType Dflt, IrtRType *Dest)
{
    int i = 0;
    char *Temp, Dummy[10];

    if ((Temp = strstr(p, Name)) != NULL)
#ifdef IRIT_DOUBLE
        i = sscanf(Temp, "%s %lf", Dummy, Dest);
#else
        i = sscanf(Temp, "%s %f", Dummy, Dest);
#endif /* IRIT_DOUBLE */

    if (i != 2) {
        *Dest = Dflt;
	return FALSE;
    }
    else
        return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxilary function to function InitTextureParameters                        *
*****************************************************************************/
static int Set2ParAux(char *p,
		      char *Name,
		      IrtRType Dflt1,
		      IrtRType Dflt2,
		      IrtRType *Dest1,
		      IrtRType *Dest2)
{
    int i = 0;
    char *Temp, Dummy[10];

    if ((Temp = strstr(p, Name)) != NULL)
#ifdef IRIT_DOUBLE
        i = sscanf(Temp, "%s %lf %lf", Dummy, Dest1, Dest2);
#else
        i = sscanf(Temp, "%s %f %f", Dummy, Dest1, Dest2);
#endif /* IRIT_DOUBLE */

    if (i != 3) {
        *Dest1 = Dflt1;
        *Dest2 = Dflt2;
	return FALSE;
    }
    else
        return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function InitTextureParameters                       *
*****************************************************************************/
static int Set3ParAux(char *p, char *Name, IrtVecType Dflt, IrtVecType Dest)
{
    int i = 0;
    char *Temp, Dummy[10];

    if ((Temp = strstr(p, Name)) != NULL)
#ifdef IRIT_DOUBLE
        i = sscanf(Temp, "%s %lf %lf %lf",
#else
        i = sscanf(Temp, "%s %f %f %f",
#endif /* IRIT_DOUBLE */
           Dummy, &Dest[RNDR_X_AXIS], &Dest[RNDR_Y_AXIS], &Dest[RNDR_Z_AXIS]);

    if (i != 4) {
        IRIT_VEC_COPY(Dest, Dflt);
	return FALSE;
    }
    else
        return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes texture-specific paramters for volumetric textures.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Txtr:    OUT, IRndrTextureStruct that contains the paramters.            M
*   pString: IN, paramters string, taken from the attribute.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TextureInitParameters                                                    M
*****************************************************************************/
void TextureInitParameters(IRndrTextureStruct *Txtr, const char *pString)
{
    char p[100], *Pars;
    IrtVecType V;
    IrtRType
        PrmTextureType = 0.0;
    IRIT_STATIC_DATA IrtVecType
        DefaultDir = { 1.0, 0.0, 0.0 },
	DefaultOrg = { 0.0, 0.0, 0.0 },
	DefaultClr = { 1.0, 1.0, 1.0 },
	DefaultScl = { 1.0, 1.0, -1.0 },
	DefaultCC1 = { 1.0, 0.0, 0.0 },
	DefaultCC2 = { 0.0, 1.0, 0.0 },
	DefaultCC3 = { 0.0, 0.0, 1.0 };

    /* Get string without the texture name. */
    strcpy(p, pString);
    if ((Pars = strchr(p, ',')) != NULL)
        Pars++;
    else
        Pars = "";

    /* Scan paramters string for special texture paramters. */
    SetParAux(Pars, "d ", 1.0, &Txtr -> Depth);
    SetParAux(Pars, "b ", 0.0, &Txtr -> Brightness);
    SetParAux(Pars, "f ", 5.0, &Txtr -> Frequency);
    Set2ParAux(Pars, "t ", 5.0, 4.0, &Txtr -> TurbulenceScale,
                            &Txtr -> TurbulenceFactor);
    Set2ParAux(Pars, "c ", 10.0, 0.2, &Txtr -> CenterScale,
                            &Txtr -> CenterFactor);
    Set2ParAux(Pars, "w ", 12.0, 0.2, &Txtr -> WaveNPoints,
                            &Txtr -> WaveFactor);
    Set2ParAux(Pars, "r ", 3.0, 0.25, &Txtr -> FiberScale,
                            &Txtr -> FiberFactor);
    Set3ParAux(Pars, "z ", DefaultDir, Txtr -> Dir);
    Set3ParAux(Pars, "o ", DefaultOrg, Txtr -> Org);
    Set3ParAux(Pars, "C ", DefaultClr, V);
    Txtr -> Color.r = (int) (V[0] * 255.0);
    Txtr -> Color.g = (int) (V[1] * 255.0);
    Txtr -> Color.b = (int) (V[2] * 255.0);
    SetParAux(Pars, "W ", 0.1, &Txtr -> Width);
    SetParAux(Pars, "CP ", 0.0, &Txtr -> CheckerPlane);
    Set3ParAux(Pars, "C1 ", DefaultCC1, Txtr -> CheckerColor[0]);
    Set3ParAux(Pars, "C2 ", DefaultCC2, Txtr -> CheckerColor[1]);
    Set3ParAux(Pars, "C3 ", DefaultCC3, Txtr -> CheckerColor[2]);
    GMGenTransMatrixZ2Dir(Txtr -> Mat, Txtr -> Org, Txtr -> Dir, 1.0);
    MatInverseMatrix(Txtr -> Mat, Txtr -> Mat);

    /* Parameters of ptexture. */
    SetParAux(Pars, "A ", 0.0, &Txtr -> PrmAngle);
    SetParAux(Pars, "T ", 0.0, &PrmTextureType);

    if (Set3ParAux(Pars, "S ", DefaultScl, V)) {
        Txtr -> PrmUScale = V[0];
	Txtr -> PrmVScale = V[1];
	Txtr -> PrmWScale = V[2];
    }
    else {
        Set2ParAux(Pars, "S ", 1.0, 1.0,
		   &Txtr -> PrmUScale, &Txtr -> PrmVScale);
	Txtr -> PrmWScale = -1.0;
    }
    /* Is there a request for auto stretch to fill in the entire model? */
    if (Txtr -> PrmUScale == 0 && Txtr -> PrmVScale == 0)
	Txtr -> PrmWScale = -1.0;

    Set3ParAux(Pars, "D ", DefaultDir, Txtr -> PrmDir);
    IRIT_VEC_NORMALIZE(Txtr -> PrmDir);
    Set3ParAux(Pars, "O ", DefaultOrg, Txtr -> PrmOrg);
    switch ((int) PrmTextureType) {
	case 0:
	default:
            Txtr -> PrmTextureType = PTEXTURE_UV_TYPE;
	    break;
	case 1:
	    Txtr -> PrmTextureType = PTEXTURE_SPHERICAL_TYPE;
	    break;
	case 2:
	    Txtr -> PrmTextureType = PTEXTURE_SPHERE_BIJECT_TYPE;
	    break;
	case 3:
	    Txtr -> PrmTextureType = PTEXTURE_CYLINDERICAL_TYPE;
	    break;
	case 4:
	    Txtr -> PrmTextureType = PTEXTURE_PLANAR_TYPE;
	    break;
    }

    GMGenTransMatrixZ2Dir(Txtr -> PrmMat, DefaultOrg, Txtr -> PrmDir, 1.0);
    MatInverseMatrix(Txtr -> PrmMat, Txtr -> PrmMat);
}
