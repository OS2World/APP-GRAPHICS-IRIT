/******************************************************************************
* Texture.h - header file for texture processing in the RNDR library.         *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef IRNDR_TEXTURE_H
#define IRNDR_TEXTURE_H

#include "rndr_loc.h"

#define TEXTURE_TYPE_NONE    0
#define TEXTURE_TYPE_PROC    1
#define TEXTURE_TYPE_RSTR    2
#define TEXTURE_TYPE_SRF     3
#define TEXTURE_TYPE_BUMP    8     /* Can be combined with any of the above. */

#define STEXTURE_FUNC_NONE    0
#define STEXTURE_FUNC_SQRT    1
#define STEXTURE_FUNC_ABS     2

#define PTEXTURE_UV_TYPE               0
#define PTEXTURE_SPHERICAL_TYPE        1
#define PTEXTURE_SPHERE_BIJECT_TYPE    2
#define PTEXTURE_CYLINDERICAL_TYPE     3
#define PTEXTURE_PLANAR_TYPE           4

/* Texture image: its size and 2D array of pixels. */
typedef struct IRndrImageStruct {
    int xSize, ySize, Alpha;
    union {
        IrtImgPixelStruct *RGB;
        IrtImgRGBAPxlStruct *RGBA;
    } U;
} IRndrImageStruct;

struct IRndrTextureStruct;

typedef void (*TextureFuncType)(struct IRndrTextureStruct *,
				IrtPtType,
				IrtNrmlType,
				IrtRType *,
				IRndrColorType);

typedef struct IRndrTextureStruct {
    int Type;                                  /* Procedural/Raster/Surface. */
    char *TextureFileName;

    IPObjectStruct *OrigSrf;      /* Reference to original geometry surface. */
    CagdPType OrigSrfParamDomain[2];   /* Parametric domain of original srf. */

    /* Raster image texture. */
    IRndrImageStruct *PrmImage; /* Ptr to texture image if exists, NULL else.*/
    IrtRType PrmUMin, PrmUMax, PrmVMin, PrmVMax;       /* Parametric domain. */
    IrtRType PrmUScale, PrmVScale, PrmWScale;    /* Scale in U/V of texture. */
    IrtRType PrmAngle;     /* Angle of rotation of texture around main axis. */
    int PrmTextureType;  /* Type of param. texture: regular, spherical, etc. */
    IrtVecType PrmDir;       /* Direction used in some parametric texturing. */
    IrtVecType PrmOrg;    /* Origin point used in some parametric texturing. */
    IrtHmgnMatType PrmMat;      /* Parametric texture transformation matrix. */

    /* Surface style texture. */
    CagdSrfStruct *Srf;         /* A surface to evaluate for texture value.  */
    CagdPType SrfParamDomain[2];                /* Parametric domain of srf. */
    IRndrImageStruct *SrfScale;     /* To map the value of srf into a color. */
    IrtRType SrfScaleMinMax[2];  /* Values to be mapped to srfScale extrema. */
    int SrfFunc;     /* If surface value should be piped through a function. */

    /* Curvature style textures. */
    IrtRType CrvtrSrfMax;	         /* Bounding value on the curvature. */

    /* Procedure/volumetric type texture. */
    TextureFuncType vTexture;
    IrtHmgnMatType Mat;                    /* Texture transformation matrix. */
    IrtRType Width;              /* Width used in some volumetric texturing. */
    IrtRType Depth;              /* Width used in some volumetric texturing. */
    IrtRType Brightness;    /* Brightness used in some volumetric texturing. */
    IrtRType Frequency;      /* Frequency used in some volumetric texturing. */
    IrtImgPixelStruct Color;     /* Color used in some volumetric texturing. */
    IrtVecType tScale;                          /* Volumetric texture scale. */
    IrtVecType Dir;          /* Direction used in some volumetric texturing. */
    IrtVecType Org;       /* Origin point used in some volumetric texturing. */
    /* Parameters specific for "wood" volumetric texture. */
    IrtRType CenterScale;
    IrtRType CenterFactor;
    IrtRType WaveNPoints;
    IrtRType WaveFactor;
    IrtRType FiberScale;
    IrtRType FiberFactor;
    /* Parameters specific for "marble" volumetric texture. */
    IrtRType TurbulenceScale;
    IrtRType TurbulenceFactor;
    /* Parameters specific for "checker" volumetric texture. */
    IRndrColorType CheckerColor[3];                     /* Checker's colors. */
    IrtRType CheckerPlane; /* Flag for whether it's a plane checker or a 3D. */
} IRndrTextureStruct;

IrtImgPixelStruct *TextureImageGetPixel(IRndrTextureStruct *Txtr,
					IRndrImageStruct *i,
					IrtPtType p,
					IrtRType v,
					IrtRType u,
					IPPolygonStruct *Poly);

/* Structures for noise generation, used for Solid Texture Mapping.          */

/* NoiseStruct is used for noise in a 3D space. */
typedef struct NoiseStruct {
    IrtRType Lattice[RNDR_MAX_NOISE][RNDR_MAX_NOISE][RNDR_MAX_NOISE];
    IrtRType Scale;
} NoiseStruct;

/* SmoothNoiseStruct is used when a linear interpolation of noise values     */
/* isn't good enough. The values are interpolated by a cubic interpolation   */
/* but this is good only for 1D noise. (i.e. noise = f(x)   )                */
typedef struct SmoothNoiseStruct {
    int n;
    IrtRType Min, Max;
    IrtRType Coefs[RNDR_MAX_NOISE - 1][4];
} SmoothNoiseStruct;

/* We make use of Texture name to Texture functions mapping table.           */
/* That data structure describes a single entry.                             */
typedef struct ProcIRndrTextureStruct {
    char *Name;             /* Procedural volumetric texture symbolic name. */
    TextureFuncType vTexture;
} ProcIRndrTextureStruct;

/*-----------------------------*/
void TextureMarble(IRndrTextureStruct *Txtr,
		   IrtPtType Point,
		   IrtNrmlType Normal,
		   IrtRType *Uv,
		   IRndrColorType Color);
void TextureWood(IRndrTextureStruct *Txtr,
		 IrtPtType Point,
		 IrtNrmlType Normal,
		 IrtRType *Uv,
		 IRndrColorType Color);
void TextureCamouf(IRndrTextureStruct *Txtr,
		   IrtPtType Point,
		   IrtNrmlType Normal,
		   IrtRType *Uv,
		   IRndrColorType Color);
void TextureBumpOrange(IRndrTextureStruct *Txtr,
		       IrtPtType Point,
		       IrtNrmlType Normal,
		       IrtRType *Uv,
		       IRndrColorType Color);
void TextureBumpChocolate(IRndrTextureStruct *Txtr,
			  IrtPtType Point,
			  IrtNrmlType Normal,
			  IrtRType *Uv,
			  IRndrColorType Color);
void TextureChecker(IRndrTextureStruct *Txtr,
		    IrtPtType Point,
		    IrtNrmlType Normal,
		    IrtRType *Uv,
		    IRndrColorType Color);
void TextureContour(IRndrTextureStruct *Txtr,
		    IrtPtType Point,
		    IrtNrmlType Normal,
		    IrtRType *Uv,
		    IRndrColorType Color);
void TextureCurvature(IRndrTextureStruct *Txtr,
		      IrtPtType Point,
		      IrtNrmlType Normal,
		      IrtRType *Uv,
		      IRndrColorType Color);
void TextureContourNormal(IRndrTextureStruct *Txtr,
			  IrtPtType Point,
			  IrtNrmlType Normal,
			  IrtRType *Uv,
			  IRndrColorType Color);
void TexturePunky(IRndrTextureStruct *Txtr,
		  IrtPtType Point,
		  IrtNrmlType Normal,
		  IrtRType *Uv,
		  IRndrColorType Color);
void TextureInitParameters(IRndrTextureStruct *Txtr, const char *pString);

/* Mapping of predefined textures names to functions. */
IRIT_GLOBAL_DATA_HEADER ProcIRndrTextureStruct ProcTextures[];

#endif /* IRNDR_TEXTURE_H */
