/******************************************************************************
* MarchCube.h - An implementation of the marching cube algorithm.	      *
*									      *
*                                                                             *
*                     7            K           6                              *
*                      ***********************                                *
*                    * +                   * *                                *
*                L *   +                 *   *              Vertices 0 - 7    *
*                *     +     I         * J   *              Edges    A - L    *
*            4 *********************** 5     *                                *
*              *       +             *       *                                *
*              *       +             *       * G                              *
*              *       + H           *       *                                *
*              *       +             *       *                                *
*              *       +             * F     *                                *
*            E *       +       C     *       *                                *
*              *       ++++++++++++++*+++++++* 2                              *
*              *   D + 3             *     *                                  *
*              *   +                 *   * B                                  *
*              * +                   * *                                      *
*              ***********************                                        *
*             0           A           1                                       *
*                                                                             *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 92.					      *
******************************************************************************/

#ifndef MARCH_CUBE_H
#define MARCH_CUBE_H

#include "irit_sm.h"
#include "triv_lib.h"

#define MC_VRTX_0 0
#define MC_VRTX_1 1
#define MC_VRTX_2 2
#define MC_VRTX_3 3
#define MC_VRTX_4 4
#define MC_VRTX_5 5
#define MC_VRTX_6 6
#define MC_VRTX_7 7
#define MC_VRTX_NONE 999

#define MC_EDGE_A 0
#define MC_EDGE_B 1
#define MC_EDGE_C 2
#define MC_EDGE_D 3
#define MC_EDGE_E 4
#define MC_EDGE_F 5
#define MC_EDGE_G 6
#define MC_EDGE_H 7
#define MC_EDGE_I 8
#define MC_EDGE_J 9
#define MC_EDGE_K 10
#define MC_EDGE_L 11
#define MC_EDGE_NONE 999

typedef struct _MCInterStruct { /* Used internally to hold intersection pts. */
    CagdPType _Nrml;
    CagdPType _Pos;
    int _HighV;
} _MCInterStruct;

typedef struct MCCubeCornerScalarStruct {
    CagdPType Vrtx0Lctn;			  /* Lowest corner position. */
    CagdPType CubeDim; 				    /* Width, Depth, Height. */
    CagdRType Corners[8];			/* Scalar values of corners. */
    CagdRType GradientX[8];		    /* Optional gradient at corners. */
    CagdRType GradientY[8];
    CagdRType GradientZ[8];
    CagdBType HasGradient;		       /* True if Gradient? are set. */

    /* Used internally. */
    CagdBType _Intersect;
    CagdPType _VrtxPos[8];
    struct _MCInterStruct _Inter[12];
} MCCubeCornerScalarStruct;

typedef struct MCPolygonStruct {
    struct MCPolygonStruct *Pnext;		        /* To next in chain. */
    int NumOfVertices;
    CagdPType V[13];
    CagdPType N[13];
} MCPolygonStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MCPolygonStruct *MCThresholdCube(MCCubeCornerScalarStruct *CCS,
				 CagdRType Threshold);

IPObjectStruct *MCExtractIsoSurface(const char *FileName,
				    int DataType,
				    IrtPtType CubeDim,
				    int Width,
				    int Height,
				    int Depth,
				    int SkipFactor,
				    CagdRType IsoVal);
IPObjectStruct *MCExtractIsoSurface2(const TrivTVStruct *TV,
				     int Axis,
				     CagdBType TrivarNormals,
				     IrtPtType CubeDim,
				     int SkipFactor,
				     CagdRType SamplingFactor,
				     CagdRType IsoVal);
IPObjectStruct *MCExtractIsoSurface3(IPObjectStruct *ImageList,
				     IrtPtType CubeDim,
				     int SkipFactor,
				     CagdRType IsoVal);
TrivTVStruct *TrivLoadVolumeIntoTV(const char *FileName,
				   int DataType,
				   IrtVecType VolSize,
				   IrtVecType Orders);

CagdBType MCImprovePointOnIsoSrfPrelude(const TrivTVStruct *TV);
void MCImprovePointOnIsoSrfPostlude(void);
int MCImprovePointOnIsoSrf(IrtPtType Pt,
			   const IrtPtType CubeDim,
			   CagdRType IsoVal,
			   CagdRType Tolerance,
			   CagdRType AllowedError);
CagdCrvStruct *TrivCoverIsoSurfaceUsingStrokes(TrivTVStruct *TV,
					       int NumStrokes,
					       int StrokeType,
					       CagdPType MinMaxPwrLen,
					       CagdRType StepSize,
					       CagdRType IsoVal,
					       CagdVType ViewDir);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /*  MARCH_CUBE_H */
