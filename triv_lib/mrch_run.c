/******************************************************************************
* Mrch_Run.c - An interface to an implementation of marching cubes algorithm. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*						Gershon Elber, Dec 1992.      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "mrchcube.h"
#include "triv_loc.h"

#ifdef __WINNT__
#include <fcntl.h>
#include <io.h>
#endif /* __WINNT__ */

typedef enum {
    INPUT_ASCII = 1,
    INPUT_INTEGER,
    INPUT_LONG,
    INPUT_BYTE,
    INPUT_IRIT_FLOAT,
    INPUT_IRIT_DOUBLE
} ScalarFormatType;

IRIT_STATIC_DATA IrtPtType
    GlblCubeDim = { 1.0, 1.0, 1.0 };
IRIT_STATIC_DATA int
    GlblSkipInputData = 1,
    GlblDataWidth = 100,
    GlblDataDepth = 100,
    GlblDataHeight = 100;
IRIT_STATIC_DATA ScalarFormatType
    GlblInputFormat = INPUT_ASCII;
IRIT_STATIC_DATA CagdRType
    *GlblLayerOne = NULL,
    *GlblLayerTwo = NULL;

static CagdRType GetOneScalar(FILE *Fin);
static MCCubeCornerScalarStruct *GetCubeFile(FILE *f, int FirstTime);
static MCCubeCornerScalarStruct *GetCubeTV(CagdRType *TVPoints, int FirstTime);
static int GetLayerFromImage(CagdRType *Layer, IPObjectStruct *ImageNameObj);
static MCCubeCornerScalarStruct *GetCubeImageFile(IPObjectStruct *ImageFiles,
						  int FirstTime);
static void EstimateGradient(MCCubeCornerScalarStruct *CCS);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract a polygonal iso-surface out of volumetric data file.             M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   Containing the volumetric data.                              M
*   DataType:   Type of scalar value in volume.  Can be one of:              M
*		  1 - Regular float or int ascii (separated by white spaces).V
*		  2 - Two bytes short integer.				     V
*		  3 - Four bytes long integer.				     V
*		  4 - One byte (char) integer.				     V
*		  5 - Four bytes float.					     V
*		  6 - Eight bytes double.				     V
*   CubeDim:	Width, height, and depth of a single cube, in object space   M
*		coordinates.						     M
*   Width:      Of volumetric data set.                                      M
*   Height:     Of volumetric data set.                                      M
*   Depth:      Of volumetric data set.                                      M
*   SkipFactor: Typically 1.  For 2, only every second sample is considered  M
*		and for i, only every i'th sample is considered, in all axes.M
*   IsoVal:     At which to extract the iso-surface.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A polygonal approximation of the iso-surface at IsoVal M
*		computed using the marching cubes algorithm.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCThresholdCube, MCExtractIsoSurface2, MCExtractIsoSurface3              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCExtractIsoSurface                                                      M
*****************************************************************************/
IPObjectStruct *MCExtractIsoSurface(const char *FileName,
				    int DataType,
				    IrtPtType CubeDim,
				    int Width,
				    int Height,
				    int Depth,
				    int SkipFactor,
				    CagdRType IsoVal)
{
    FILE *f;
    MCCubeCornerScalarStruct *CCS;
    IPPolygonStruct
	*AllPolys = NULL;
    int IsCirc = IPSetPolyListCirc(FALSE);

    IPSetPolyListCirc(IsCirc);	      /* Restore state, now that we know it. */

    if ((f = fopen(FileName, "r")) == NULL) {
        TRIV_FATAL_ERROR(TRIV_ERR_FAIL_READ_FILE);
        return NULL;
    }

#if defined(__OS2GCC__)
    if (DataType != INPUT_ASCII)
	setmode(fileno(f), O_BINARY);    /* Make sure it is in binary mode. */
#endif /* __OS2GCC__ */
#if defined(__WINNT__)
    if (DataType != INPUT_ASCII)
	_setmode(_fileno(f), _O_BINARY); /* Make sure it is in binary mode. */
#endif /* __WINNT__ */


    IRIT_PT_COPY(GlblCubeDim, CubeDim);
    GlblDataWidth = Width;
    GlblDataHeight = Height;
    GlblDataDepth = Depth;
    GlblSkipInputData = SkipFactor;
    GlblInputFormat = (ScalarFormatType) DataType;

    GlblLayerOne = (CagdRType *) IritMalloc(sizeof(CagdRType) * GlblDataWidth *
							       GlblDataHeight);
    GlblLayerTwo = (CagdRType *) IritMalloc(sizeof(CagdRType) * GlblDataWidth *
							       GlblDataHeight);
    GetCubeFile(f, TRUE);                          /* Initialize local info. */

    while ((CCS = GetCubeFile(f, FALSE)) != NULL) {
	MCPolygonStruct
	    *MCPolys = MCThresholdCube(CCS, IsoVal);

	while (MCPolys != NULL) {
	    int i;
	    MCPolygonStruct
		*MCPolyTmp = MCPolys -> Pnext;

	    /* Convert to regular irit polygons. */
	    for (i = 2; i < MCPolys -> NumOfVertices - 1; i++) {
	        int j;
	        IPVertexStruct
		    *V3 = IPAllocVertex2(NULL),
		    *V2 = IPAllocVertex2(V3),
		    *V1 = IPAllocVertex2(V2);
		IPPolygonStruct
		    *Pl = IPAllocPolygon(0, V1, AllPolys);

		AllPolys = Pl;

		for (j = 0; j < 3; j++) {
		    V1 -> Coord[j] = MCPolys -> V[0][j];
		    V2 -> Coord[j] = MCPolys -> V[i - 1][j];
		    V3 -> Coord[j] = MCPolys -> V[i][j];

		    V1 -> Normal[j] = MCPolys -> N[0][j];
		    V2 -> Normal[j] = MCPolys -> N[i - 1][j];
		    V3 -> Normal[j] = MCPolys -> N[i][j];
		}
		IP_SET_NORMAL_VRTX(V1);
		IP_SET_NORMAL_VRTX(V2);
		IP_SET_NORMAL_VRTX(V3);

		if (IsCirc)
		    V3 -> Pnext = V1;
		IPUpdatePolyPlane(Pl);
		if (IRIT_DOT_PROD(V1 -> Normal, Pl -> Plane) < 0) {
		    IRIT_PT_SCALE(Pl -> Plane, -1.0);
		    Pl -> Plane[3] *= -1.0;
		}
	    }

	    IritFree(MCPolys);

	    MCPolys = MCPolyTmp;
	}
    }

    IritFree(GlblLayerOne);
    IritFree(GlblLayerTwo);

    fclose(f);

    if (AllPolys != NULL)
        return IPGenPOLYObject(AllPolys);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract a polygonal iso-surface out of a trivariate function.            M
*                                                                            *
* PARAMETERS:                                                                M
*   CTV:	The trivariate to compute an iso surface approximation for,  M
*   Axis:	of the trivariate to handle, 1 for X, 2 for Y, etc.	     M
*   TrivarNormals:   If TRUE normal are computed using gradient of the       M
*		trivariate, if FALSE no normal are estimated.		     M
*   CubeDim:	Width, height, and depth of a single cube, in object space   M
*		coordinates.						     M
*   SkipFactor: Typically 1.  For 2, only every second sample is considered  M
*		and for i, only every i'th sample is considered, in all axes.M
*   SamplingFactor:  Additional relative sampling to apply to TV.	     M
*	        If SamplingFactor set to 1.0, the trivariate is sampled      M
*		ULength * VLength * WLength.  Otherwise, the samplings are   M
*		(ULength * SamplingFactor) * (VLength * SamplingFactor) *    M
*	        (WLength * SamplingFactor).				     M
*   IsoVal:     At which to extract the iso-surface.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A polygonal approximation of the iso-surface at IsoVal M
*		computed using the marching cubes algorithm.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCThresholdCube, MCExtractIsoSurface, MCExtractIsoSurface3               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCExtractIsoSurface2                                                     M
*****************************************************************************/
IPObjectStruct *MCExtractIsoSurface2(const TrivTVStruct *CTV,
				     int Axis,
				     CagdBType TrivarNormals,
				     IrtPtType CubeDim,
				     int SkipFactor,
				     CagdRType SamplingFactor,
				     CagdRType IsoVal)
{
    MCCubeCornerScalarStruct *CCS;
    TrivTVStruct *TVGradient[3];
    IPPolygonStruct
	*AllPolys = NULL;
    int i, j, k,
	IsCirc = IPSetPolyListCirc(FALSE);
    CagdRType *TVPoints, *R, UCorrection, VCorrection, WCorrection;
    TrivTVStruct *TV;

    SamplingFactor = IRIT_BOUND(SamplingFactor, IRIT_EPS, IRIT_INFNTY);

    IPSetPolyListCirc(IsCirc);	      /* Restore state, now that we know it. */

    if (Axis < 1 || Axis > CAGD_MAX_PT_COORD || CTV -> Points[Axis] == NULL) {
	TRIV_FATAL_ERROR(TRIV_ERR_INVALID_AXIS);
	return NULL;
    }

    if (TRIV_IS_BEZIER_TV(CTV))
        TV = TrivCnvrtBzr2BspTV(CTV);
    else
        TV = TrivTVCopy(CTV);

    GlblDataWidth = (int) (TV -> ULength * SamplingFactor);
    GlblDataHeight = (int) (TV -> VLength * SamplingFactor);
    GlblDataDepth = (int) (TV -> WLength * SamplingFactor);

    UCorrection = GlblDataWidth / (GlblDataWidth - 1.0);
    VCorrection = GlblDataHeight / (GlblDataHeight - 1.0);
    WCorrection = GlblDataDepth / (GlblDataDepth - 1.0);

    /* Make sure the domain is consistent with the data size. */
    BspKnotAffineTransOrder2(TV -> UKnotVector, TV -> UOrder,
			     TV -> ULength + TV -> UOrder,
			     0.0, IRIT_EPS + GlblDataWidth - 1.0);
    BspKnotAffineTransOrder2(TV -> VKnotVector, TV -> VOrder,
			     TV -> VLength + TV -> VOrder,
			     0.0, IRIT_EPS + GlblDataHeight - 1.0);
    BspKnotAffineTransOrder2(TV -> WKnotVector, TV -> WOrder,
			     TV -> WLength + TV -> WOrder,
			     0.0, IRIT_EPS + GlblDataDepth - 1.0);

    if (TrivarNormals &&
	((TVGradient[0] = TrivTVDeriveScalar(TV, TRIV_CONST_U_DIR)) == NULL ||
	 (TVGradient[1] = TrivTVDeriveScalar(TV, TRIV_CONST_V_DIR)) == NULL ||
	 (TVGradient[2] = TrivTVDeriveScalar(TV, TRIV_CONST_W_DIR)) == NULL)) {
	TrivTVFree(TV);
	return NULL;
    }

    for (i = 0; i < 3; i++) {
	if (CubeDim[i] < IRIT_UEPS)
	    CubeDim[i] = IRIT_UEPS;
    }

    IRIT_PT_COPY(GlblCubeDim, CubeDim);
    GlblSkipInputData = SkipFactor;

    /* Evaluate the trivariate on the given Axis, in place. */
    TVPoints = R = (CagdRType *) IritMalloc(GlblDataWidth *
					    GlblDataHeight *
					    GlblDataDepth * sizeof(CagdRType));
    for (k = 0; k < GlblDataDepth; k++)
        for (j = 0; j < GlblDataHeight; j++)
	    for (i = 0; i < GlblDataWidth; i++) {
	        CagdRType
		    *TVR = TrivTVEval(TV, i, j, k);

		if (TRIV_IS_RATIONAL_TV(TV)) {
		    *R++ = TVR[Axis] / TVR[0];
		}
		else
		    *R++ = TVR[Axis];
	    }

    GetCubeTV(TVPoints, TRUE);                     /* Initialize local info. */

    while ((CCS = GetCubeTV(TVPoints, FALSE)) != NULL) {
	MCPolygonStruct
	    *MCPolys = MCThresholdCube(CCS, IsoVal);

	while (MCPolys != NULL) {
	    MCPolygonStruct
		*MCPolyTmp = MCPolys -> Pnext;

	    /* Convert to regular irit polygons. */
	    for (i = 2; i < MCPolys -> NumOfVertices - 1; i++) {
	        int j;
	        IPVertexStruct
		    *V3 = IPAllocVertex2(NULL),
		    *V2 = IPAllocVertex2(V3),
		    *V1 = IPAllocVertex2(V2);
		IPPolygonStruct
		    *Pl = IPAllocPolygon(0, V1, AllPolys);

		AllPolys = Pl;

		for (j = 0; j < 3; j++) {
		    V1 -> Coord[j] = MCPolys -> V[0][j];
		    V2 -> Coord[j] = MCPolys -> V[i - 1][j];
		    V3 -> Coord[j] = MCPolys -> V[i][j];
		    
		    V1 -> Normal[j] = MCPolys -> N[0][j];
		    V2 -> Normal[j] = MCPolys -> N[i - 1][j];
		    V3 -> Normal[j] = MCPolys -> N[i][j];
		}

		if (IsCirc)
		    V3 -> Pnext = V1;
		IPUpdatePolyPlane(Pl);

		if (IRIT_DOT_PROD(V1 -> Normal, Pl -> Plane) < 0) {
		    Pl -> PVertex = V1 = IPReverseVrtxList2(V1);
		    V2 = V1 -> Pnext;
		    V3 = V2 -> Pnext;
		    IRIT_PT_SCALE(Pl -> Plane, -1.0);
		    Pl -> Plane[3] *= -1.0;
		}

		if (TrivarNormals) {
		    IPVertexStruct *V;

		    /* Compute the gradients for the three vertices. */
		    for (j = 0, V = V1; j < 3; j++, V = V -> Pnext) {
			int l;

			for (l = 0; l < 3; l++) {
			    CagdRType
			        *R = TrivTVEval(TVGradient[l], 
						V -> Coord[0] / CubeDim[0],
						V -> Coord[1] / CubeDim[1],
						V -> Coord[2] / CubeDim[2]);

			    V -> Normal[l] = R[1];
			}
			IRIT_PT_NORMALIZE(V -> Normal);
			IP_SET_NORMAL_VRTX(V);
		    }
		}
		else {
		    IP_RST_NORMAL_VRTX(V1);
		    IP_RST_NORMAL_VRTX(V2);
		    IP_RST_NORMAL_VRTX(V3);
		}
	    }

	    IritFree(MCPolys);

	    MCPolys = MCPolyTmp;
	}
    }

    IritFree(TVPoints);

    if (TrivarNormals) {
	TrivTVFree(TVGradient[0]);
	TrivTVFree(TVGradient[1]);
	TrivTVFree(TVGradient[2]);
    }
    TrivTVFree(TV);

    if (AllPolys != NULL) {
        IrtHmgnMatType Mat;
	IPObjectStruct *PObj;

	MatGenMatScale(UCorrection / SamplingFactor,
		       VCorrection / SamplingFactor,
		       WCorrection / SamplingFactor, Mat);
	PObj = IPGenPOLYObject(GMTransformPolyList(AllPolys, Mat, TRUE));
	IPFreePolygonList(AllPolys);

	return PObj;
    }
    else {
	IRIT_WARNING_MSG("Empty iso surface resulted.\n");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract a polygonal iso-surface out of a stack of images.                M
*                                                                            *
* PARAMETERS:                                                                M
*   ImageList:  List of image file names.	                             M
*   CubeDim:	Width, height, and depth of a single cube, in object space   M
*		coordinates.						     M
*   SkipFactor: Typically 1.  For 2, only every second sample is considered  M
*		and for i, only every i'th sample is considered, in all axes.M
*   IsoVal:     At which to extract the iso-surface.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A polygonal approximation of the iso-surface at IsoVal M
*		computed using the marching cubes algorithm.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCThresholdCube, MCExtractIsoSurface, MCExtractIsoSurface2               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCExtractIsoSurface3                                                     M
*****************************************************************************/
IPObjectStruct *MCExtractIsoSurface3(IPObjectStruct *ImageList,
				     IrtPtType CubeDim,
				     int SkipFactor,
				     CagdRType IsoVal)
{
    int XMax, YMax, Depth, Alpha,
	IsCirc = IPSetPolyListCirc(FALSE);
    MCCubeCornerScalarStruct *CCS;
    IPPolygonStruct
	*AllPolys = NULL;
    IPObjectStruct *ImageNameObj;
    IrtImgPixelStruct *RGB;

    IPSetPolyListCirc(IsCirc);	      /* Restore state, now that we know it. */

    /* Read the first image to get the dimensions of the volume. */
    ImageNameObj = IPListObjectGet(ImageList, 0);
    if (IP_IS_STR_OBJ(ImageNameObj) &&
	(RGB = IrtImgReadImage(ImageNameObj -> U.Str,
			       &XMax, &YMax, &Alpha)) != NULL) {
    }
    else {
        TRIV_FATAL_ERROR(TRIV_ERR_FAIL_READ_FILE);
        return NULL;
    }
    IritFree(RGB);
    Depth = IPListObjectLength(ImageList);

    IRIT_PT_COPY(GlblCubeDim, CubeDim);
    GlblDataWidth = XMax + 1;
    GlblDataHeight = YMax + 1;
    GlblDataDepth = Depth;
    GlblSkipInputData = SkipFactor;

    GlblLayerOne = (CagdRType *) IritMalloc(sizeof(CagdRType) * GlblDataWidth *
							       GlblDataHeight);
    GlblLayerTwo = (CagdRType *) IritMalloc(sizeof(CagdRType) * GlblDataWidth *
							       GlblDataHeight);
    GetCubeImageFile(ImageList, TRUE);             /* Initialize local info. */

    while ((CCS = GetCubeImageFile(ImageList, FALSE)) != NULL) {
	MCPolygonStruct
	    *MCPolys = MCThresholdCube(CCS, IsoVal);

	while (MCPolys != NULL) {
	    int i;
	    MCPolygonStruct
		*MCPolyTmp = MCPolys -> Pnext;

	    /* Convert to regular irit polygons. */
	    for (i = 2; i < MCPolys -> NumOfVertices - 1; i++) {
	        int j;
	        IPVertexStruct
		    *V3 = IPAllocVertex2(NULL),
		    *V2 = IPAllocVertex2(V3),
		    *V1 = IPAllocVertex2(V2);
		IPPolygonStruct
		    *Pl = IPAllocPolygon(0, V1, AllPolys);

		AllPolys = Pl;

		for (j = 0; j < 3; j++) {
		    V1 -> Coord[j] = MCPolys -> V[0][j];
		    V2 -> Coord[j] = MCPolys -> V[i - 1][j];
		    V3 -> Coord[j] = MCPolys -> V[i][j];

		    V1 -> Normal[j] = MCPolys -> N[0][j];
		    V2 -> Normal[j] = MCPolys -> N[i - 1][j];
		    V3 -> Normal[j] = MCPolys -> N[i][j];
		}
		IP_SET_NORMAL_VRTX(V1);
		IP_SET_NORMAL_VRTX(V2);
		IP_SET_NORMAL_VRTX(V3);

		if (IsCirc)
		    V3 -> Pnext = V1;
		IPUpdatePolyPlane(Pl);
		if (IRIT_DOT_PROD(V1 -> Normal, Pl -> Plane) < 0) {
		    IRIT_PT_SCALE(Pl -> Plane, -1.0);
		    Pl -> Plane[3] *= -1.0;
		}
	    }

	    IritFree(MCPolys);

	    MCPolys = MCPolyTmp;
	}
    }

    IritFree(GlblLayerOne);
    IritFree(GlblLayerTwo);

    if (AllPolys != NULL)
        return IPGenPOLYObject(AllPolys);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get one scalar value from input stream.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:   Input file to read from.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   One read scalar value.                                      *
*****************************************************************************/
static CagdRType GetOneScalar(FILE *Fin)
{
    short i;
    long l;
    float f;
    double d;
    CagdRType r;

    if (feof(Fin))
	return IRIT_INFNTY;

    switch (GlblInputFormat) {
	case INPUT_ASCII:
	    if (fscanf(Fin, "%lf", &r) != 1)
		return IRIT_INFNTY;
	    break;
	case INPUT_INTEGER:
	    i = getc(Fin);
	    r = getc(Fin) * 256.0 + i;
	    break;
	case INPUT_LONG:
	    if (fread(&l, 4, 1, Fin) != 4)
		return IRIT_INFNTY;
	    r = l;
	    break;
	case INPUT_BYTE:
	    r = getc(Fin);
	    break;
	case INPUT_IRIT_FLOAT:
	    if (fread(&f, 4, 1, Fin) != 4)
		return IRIT_INFNTY;
	    r = f;
	    break;
	case INPUT_IRIT_DOUBLE:
	    if (fread(&d, 8, 1, Fin) != 8)
		return IRIT_INFNTY;
	    r = d;
	    break;
	default:
	    IRIT_WARNING_MSG("Input format requested not supported.\n");
	    return IRIT_INFNTY;
    }

    return r;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads input file and returns one cube at a time.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FirstTime:    TRUE for first time, FALSE otherwise.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCCubeCornerScalarStruct *:                                              *
*****************************************************************************/
static MCCubeCornerScalarStruct *GetCubeFile(FILE *f, int FirstTime)
{
    IRIT_STATIC_DATA MCCubeCornerScalarStruct CCS;
    IRIT_STATIC_DATA int
	LayerNumber = -1,
	LayerCountX = -1,
	LayerCountY = 0;
    int i, j;
    CagdRType *p;

    if (FirstTime) {
	/* Initialize the CCS constant data */
	CCS.CubeDim[0] = GlblCubeDim[0];
	CCS.CubeDim[1] = GlblCubeDim[1];
	CCS.CubeDim[2] = GlblCubeDim[2];
	LayerNumber = -GlblSkipInputData;
	LayerCountX = -1;
	LayerCountY = 0;

	for (p = GlblLayerTwo, i = 0; i < GlblDataWidth * GlblDataHeight; i++)
	    if ((*p++ = GetOneScalar(f)) == IRIT_INFNTY)
	        return NULL;

	return NULL;
    }

    if (LayerCountX == -1) {		             /* Read the next layer. */
        LayerNumber += GlblSkipInputData;
        if (LayerNumber >= GlblDataDepth - GlblSkipInputData)
	    return NULL;                                            /* Done! */

	IRIT_SWAP(CagdRType *, GlblLayerOne, GlblLayerTwo);/*Swap two layers.*/

	for (j = 0; j < GlblSkipInputData; j++) { /* And get the next layer. */
	    for (p = GlblLayerTwo, i = 0;
		 i < GlblDataWidth * GlblDataHeight;
		 i++)
	        if ((*p++ = GetOneScalar(f)) == IRIT_INFNTY)
		    return NULL;
	}

	LayerCountX = 0;
	LayerCountY = 0;
    }

    CCS.Vrtx0Lctn[0] = LayerCountX * GlblCubeDim[0] / GlblSkipInputData;
    CCS.Vrtx0Lctn[1] = LayerCountY * GlblCubeDim[1] / GlblSkipInputData;
    CCS.Vrtx0Lctn[2] = LayerNumber * GlblCubeDim[2] / GlblSkipInputData;
    CCS.Corners[0] = GlblLayerOne[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[1] = GlblLayerOne[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[2] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[3] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];
    CCS.Corners[4] = GlblLayerTwo[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[5] = GlblLayerTwo[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[6] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[7] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];

    EstimateGradient(&CCS);

    LayerCountX += GlblSkipInputData;
    if (LayerCountX >= GlblDataWidth - GlblSkipInputData) {
	LayerCountY += GlblSkipInputData;
	LayerCountX = 0;
	if (LayerCountY >= GlblDataHeight - GlblSkipInputData)
	    LayerCountX = -1;                                /* No more data */
    }

    return &CCS;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads inout file and returns one cube at a time.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   TVPoints:	  The points on the trivariate from which to sample data.    *
*   FirstTime:    TRUE for first time, FALSE otherwise.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCCubeCornerScalarStruct *:                                              *
*****************************************************************************/
static MCCubeCornerScalarStruct *GetCubeTV(CagdRType *TVPoints, int FirstTime)
{
    IRIT_STATIC_DATA MCCubeCornerScalarStruct CCS;
    IRIT_STATIC_DATA int
	LayerNumber = -1,
	LayerCountX = -1,
	LayerCountY = 0;
    int j;

    if (FirstTime) {
	/* Initialize the CCS constant data */
	CCS.CubeDim[0] = GlblCubeDim[0];
	CCS.CubeDim[1] = GlblCubeDim[1];
	CCS.CubeDim[2] = GlblCubeDim[2];
	LayerNumber = -GlblSkipInputData;
	LayerCountX = -1;
	LayerCountY = 0;

	GlblLayerTwo = TVPoints;

	return NULL;
    }

    if (LayerCountX == -1) {		             /* Read the next layer. */
        LayerNumber += GlblSkipInputData;
        if (LayerNumber >= GlblDataDepth - GlblSkipInputData)
	    return NULL;                                            /* Done! */

	GlblLayerOne = GlblLayerTwo;     /* Swap the two layers and advance. */
	for (j = 0; j < GlblSkipInputData; j++)   /* And get the next layer. */
	    GlblLayerTwo = GlblLayerTwo + GlblDataWidth * GlblDataHeight;

	LayerCountX = 0;
	LayerCountY = 0;
    }

    CCS.Vrtx0Lctn[0] = LayerCountX * GlblCubeDim[0] / GlblSkipInputData;
    CCS.Vrtx0Lctn[1] = LayerCountY * GlblCubeDim[1] / GlblSkipInputData;
    CCS.Vrtx0Lctn[2] = LayerNumber * GlblCubeDim[2] / GlblSkipInputData;
    CCS.Corners[0] = GlblLayerOne[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[1] = GlblLayerOne[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[2] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[3] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];
    CCS.Corners[4] = GlblLayerTwo[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[5] = GlblLayerTwo[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[6] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[7] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];

    EstimateGradient(&CCS);

    LayerCountX += GlblSkipInputData;
    if (LayerCountX >= GlblDataWidth - GlblSkipInputData) {
	LayerCountY += GlblSkipInputData;
	LayerCountX = 0;
	if (LayerCountY >= GlblDataHeight - GlblSkipInputData)
	    LayerCountX = -1;                                /* No more data */
    }

    return &CCS;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetch one image as one layer in the volume.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Layer:        The layer were to place the read image.		     *
*   ImageNameObj: File name of the image to read.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if succesful, FALSE otherwise.                               *
*****************************************************************************/
static int GetLayerFromImage(CagdRType *Layer, IPObjectStruct *ImageNameObj)
{
    int i, j, XMax, YMax, Alpha;
    IrtImgPixelStruct *RGB, *Prgb;
    CagdRType
	*R = Layer;

    if (!IP_IS_STR_OBJ(ImageNameObj))
        return FALSE;

    if ((RGB = IrtImgReadImage(ImageNameObj -> U.Str,
			       &XMax, &YMax, &Alpha)) != NULL) {
    }

    if (XMax + 1 != GlblDataWidth || YMax + 1 != GlblDataHeight) {
        IritFree(RGB);
        return FALSE;
    }

    /* Convert the image to grey levels. */
    for (Prgb = RGB, i = 0; i <= XMax; i++) {
        for (j = 0; j <= YMax; j++, Prgb++) {
	    *R++ = Prgb -> r * 0.3 + Prgb -> g * 0.59 + Prgb -> b * 0.11;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads input file and returns one cube at a time.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageList:    List of image file names to read in.                       *
*   FirstTime:    TRUE for first time, FALSE otherwise.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCCubeCornerScalarStruct *:                                              *
*****************************************************************************/
static MCCubeCornerScalarStruct *GetCubeImageFile(IPObjectStruct *ImageFiles,
						  int FirstTime)
{
    IRIT_STATIC_DATA MCCubeCornerScalarStruct CCS;
    IRIT_STATIC_DATA int
	LayerNumber = 0,
	LayerCountX = -1,
	LayerCountY = 0;

    if (FirstTime) {
	/* Initialize the CCS constant data */
	CCS.CubeDim[0] = GlblCubeDim[0];
	CCS.CubeDim[1] = GlblCubeDim[1];
	CCS.CubeDim[2] = GlblCubeDim[2];
	LayerNumber = 0;
	LayerCountX = -1;
	LayerCountY = 0;

	GetLayerFromImage(GlblLayerTwo,
			  IPListObjectGet(ImageFiles, LayerNumber));

	return NULL;
    }

    if (LayerCountX == -1) {		             /* Read the next layer. */
        LayerNumber += GlblSkipInputData;
        if (LayerNumber >= GlblDataDepth - GlblSkipInputData)
	    return NULL;                                            /* Done! */

	IRIT_SWAP(CagdRType *, GlblLayerOne, GlblLayerTwo);/*Swap two layers.*/

	if (!GetLayerFromImage(GlblLayerTwo,
			       IPListObjectGet(ImageFiles, LayerNumber)))
	    return NULL;

	LayerCountX = 0;
	LayerCountY = 0;
    }

    CCS.Vrtx0Lctn[0] = LayerCountX * GlblCubeDim[0] / GlblSkipInputData;
    CCS.Vrtx0Lctn[1] = LayerCountY * GlblCubeDim[1] / GlblSkipInputData;
    CCS.Vrtx0Lctn[2] = LayerNumber * GlblCubeDim[2] / GlblSkipInputData;
    CCS.Corners[0] = GlblLayerOne[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[1] = GlblLayerOne[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[2] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[3] = GlblLayerOne[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];
    CCS.Corners[4] = GlblLayerTwo[LayerCountY * GlblDataWidth + LayerCountX];
    CCS.Corners[5] = GlblLayerTwo[LayerCountY * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[6] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX + GlblSkipInputData];
    CCS.Corners[7] = GlblLayerTwo[(LayerCountY + GlblSkipInputData) * GlblDataWidth +
			          LayerCountX];

    EstimateGradient(&CCS);

    LayerCountX += GlblSkipInputData;
    if (LayerCountX >= GlblDataWidth - GlblSkipInputData) {
	LayerCountY += GlblSkipInputData;
	LayerCountX = 0;
	if (LayerCountY >= GlblDataHeight - GlblSkipInputData)
	    LayerCountX = -1;                                /* No more data */
    }

    return &CCS;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Estimates normals based on first order differencing.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   CCS:       A single cube to estimate normals to its vertices.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EstimateGradient(MCCubeCornerScalarStruct *CCS)
{
    CCS -> GradientX[0] =
	CCS -> GradientX[1] =
	    CCS -> Corners[1] - CCS -> Corners[0];
    CCS -> GradientX[2] =
	CCS -> GradientX[3] =
	    CCS -> Corners[2] - CCS -> Corners[3];
    CCS -> GradientX[4] =
	CCS -> GradientX[5] =
	    CCS -> Corners[5] - CCS -> Corners[4];
    CCS -> GradientX[6] =
	CCS -> GradientX[7] =
	    CCS -> Corners[6] - CCS -> Corners[7];
    
    CCS -> GradientY[0] =
	CCS -> GradientY[3] =
	    CCS -> Corners[3] - CCS -> Corners[0];
    CCS -> GradientY[1] =
	CCS -> GradientY[2] =
	    CCS -> Corners[2] - CCS -> Corners[1];
    CCS -> GradientY[4] =
	CCS -> GradientY[7] =
	    CCS -> Corners[7] - CCS -> Corners[4];
    CCS -> GradientY[5] =
	CCS -> GradientY[6] =
	    CCS -> Corners[6] - CCS -> Corners[5];
    
    CCS -> GradientZ[0] =
	CCS -> GradientZ[4] =
	    CCS -> Corners[4] - CCS -> Corners[0];
    CCS -> GradientZ[1] =
	CCS -> GradientZ[5] =
	    CCS -> Corners[5] - CCS -> Corners[1];
    CCS -> GradientZ[2] =
	CCS -> GradientZ[6] =
	    CCS -> Corners[6] - CCS -> Corners[2];
    CCS -> GradientZ[3] =
	CCS -> GradientZ[7] =
	    CCS -> Corners[7] - CCS -> Corners[3];

    CCS -> HasGradient = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Loads a volumetric data set as a trivariate function of prescribed       M
* orders.  Uniform open end conditions are created for it.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:    To load the trivariate data set from.                       M
*   DataType:    Type of scalar value in volume data file.  Can be one of:   M
*		   1 - Regular ascii (separated by while spaces.             V
*		   2 - Two bytes short integer.				     V
*		   3 - Four bytes long integer.				     V
*		   4 - One byte (char) integer.				     V
*		   5 - Four bytes float.				     V
*		   6 - Eight bytes double.				     V
*   VolSize:     Dimensions of trivariate volume.                            M
*   Orders:      Orders of the three axis of the volume (in U, V, and W).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  Loaded trivariate, or NULL if error.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivLoadVolumeIntoTV                                                     M
*****************************************************************************/
TrivTVStruct *TrivLoadVolumeIntoTV(const char *FileName,
				   int DataType,
				   IrtVecType VolSize,
				   IrtVecType Orders)
{
    int i;
    FILE *f;
    TrivTVStruct
        *TV = TrivBspTVNew((int) VolSize[0],
			   (int) VolSize[1],
			   (int) VolSize[2],
			   (int) Orders[0],
			   (int) Orders[1],
			   (int) Orders[2],
			   CAGD_PT_E1_TYPE);
    CagdRType
	*R = TV -> Points[1];

    BspKnotUniformOpen(TV -> ULength, TV -> UOrder, TV -> UKnotVector);
    BspKnotUniformOpen(TV -> VLength, TV -> VOrder, TV -> VKnotVector);
    BspKnotUniformOpen(TV -> WLength, TV -> WOrder, TV -> WKnotVector);

    BspKnotAffineTrans2(TV -> UKnotVector, TV -> ULength + TV -> UOrder,
			0.0, TV -> ULength - 1.0);
    BspKnotAffineTrans2(TV -> VKnotVector, TV -> VLength + TV -> VOrder,
			0.0, TV -> VLength - 1.0);
    BspKnotAffineTrans2(TV -> WKnotVector, TV -> WLength + TV -> WOrder,
			0.0, TV -> WLength - 1.0);
    
    if ((f = fopen(FileName, "r")) == NULL) {
	TrivTVFree(TV);
	TRIV_FATAL_ERROR(TRIV_ERR_READ_FAIL);
	return NULL;
    }

#if defined(__OS2GCC__)
    if (DataType != INPUT_ASCII)
	setmode(fileno(f), O_BINARY);    /* Make sure it is in binary mode. */
#endif /* __OS2GCC__ */
#if defined(__WINNT__)
    if (DataType != INPUT_ASCII)
	_setmode(_fileno(f), _O_BINARY); /* Make sure it is in binary mode. */
#endif /* __WINNT__ */

    GlblInputFormat = (ScalarFormatType) DataType;

    for (i = TV -> ULength * TV -> VLength * TV -> WLength; i > 0; i--) {
	if ((*R++ = GetOneScalar(f)) == IRIT_INFNTY) {
	    TrivTVFree(TV);
	    fclose(f);
	    TRIV_FATAL_ERROR(TRIV_ERR_READ_FAIL);
	    return NULL;
	}
    }

    fclose(f);

    return TV;
}
