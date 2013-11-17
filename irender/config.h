#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <irit_sm.h>

#define X_AXIS        0
#define Y_AXIS        1
#define Z_AXIS        2
#define W_AXIS        3

#define RED_CLR        0
#define GREEN_CLR    1
#define BLUE_CLR    2
#define ALPHA_CLR    3

#define NEAREST_Z     IRIT_INFNTY
#define FAREST_Z    -IRIT_INFNTY
#define NEAR_THAN     >
#define FARTHER_THAN     <

#define MALLOC(type, n)         (type *) IritMalloc(sizeof(type) * (n))
#define FREE(p)            IritFree(p)

/* Configuration attributes common to all polygons in the Object.            */
/* We use it to store otherwise unefficient accessed attributes.             */
/* GLOBAL configuration variables.                                           */
typedef struct GlobalOptionsStruct {
    int       XSize, YSize;                      /* Size of the final image. */
    IrtRType  Ambient;          /* Ambient color fraction in the final color.*/
    IrtPtType BackGround;                   /* Background of the image color.*/
    int       BackFace;        /* Flag directing to remove back faced flats. */

    int       Srf2PlgOptimal;  /* Optimal argument of IritSurfaces2Polygons. */
    IrtRType  Srf2PlgFineness;/* Fineness of surface to polygons tesselation.*/
    SymbCrvApproxMethodType Crv2PllMethod;   /* Arg of IritCurves2Polylines. */
    int       Crv2PllSamples;     /* Number of samples to approximate curve. */

    int       HasTime;             /* If TRUE, we compute animation at Time. */
    IrtRType  Time;            /* If HasTime, Time is set to animation time. */

    int       ShadeModel;                 /* Type of shading model to apply. */
    int       Polylines; /* Flag directing to convert polylines to polygons. */
    IrtRType  PllMinW;                           /* Polylines minimal width. */
    IrtRType  PllMaxW;                           /* Polylines maximal width. */
    int       DrawPoints;	         /* Should we render points as well? */
    IrtRType  PointDfltRadius; /* Points are rendered as sphere of this rad. */
    int       Shadows;     /* Flag directing to apply shadows determination. */
    int       Transp;         /* Flag directing to apply transperancy model. */
    char      *FilterName;         /* Name of the antialias filter, or NULL. */
    int       ZDepth;    /* Flag directing to output image in zdepth format. */
    int       Stencil;  /* Flag directing to output image in stencil format. */
    int       VisMap;   /* Flag directing to output image in visible format. */
    int       NormalReverse;       /* Flag directing to reverse every vertex */
                                     /* and plane normals before processing. */
    int       NFiles;         /* Number of file names passed to the program. */
    char      **Files;               /* Array of pointers to the file names. */
    int       PPMStyleP3;                      /* By default P6, P3 if TRUE. */
    int       Verbose;                    /* Print some diagnostic messages. */
    char      *OutFileName;       /* Name of output file, NULL if to stdout. */
    const char *FileType;                             /* Type of output file */
    IrtRType  ZNear;                                   /*Near clipping plane */
    IrtRType  ZFar;                                     /*Far clipping plane */
    int	      NPRRendering;          /* TRUE if we seek NPR style rendering. */
    IrtRType  NPRSilWidth;          /* NPR rendering - width of silhouettes. */
    IrtPtType NPRSilColor;          /* NPR rendering - color of silhouettes. */
    int       NPRClrQuant;  /* NPR rendering - # of colors to quantize into. */
} GlobalOptionsStruct;

IRIT_GLOBAL_DATA_HEADER GlobalOptionsStruct Options;

void GetOptions(int argc, char *argv[]);
void InitOptions(void);
void GetConfig(char *argv[]);
#endif
