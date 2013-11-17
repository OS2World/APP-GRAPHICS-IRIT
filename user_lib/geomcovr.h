/*****************************************************************************
* GeomCovr.h - Header file for Calculating Geometric Covering.               *
******************************************************************************
* Written by Nadav Shragai, February 2010.                                   *
*****************************************************************************/
#ifndef GEOM_COVER_H
#define GEOM_COVER_H

#include "irit_sm.h"
#include "iritprsr.h"

typedef struct UserGCRgbToGrayStruct {
    IrtImgPixelStruct Rgb;
    MiscISCPixelType Gray;
} UserGCRgbToGrayStruct;

typedef struct UserGCObsPtEffectiveDistanceParamStruct {
    union {
        struct {
            IrtRType Efficiency;
        } Uniform;
        struct {
            IrtRType Step;
            IrtRType Near;
            IrtRType Far;
        } Step;
    };
} UserGCObsPtEffectiveDistanceParamStruct;

/* The effective of OP as a function of distance from the OP. */
typedef IrtRType (*UserGCObsPtEffectiveDistanceFuncType) (
    UserGCObsPtEffectiveDistanceParamStruct *Params,
    IrtRType Distance);

typedef struct UserGCObsPtTypeStruct {
    IrtRType ZAngle;                      /* Angle in degrees in the Z axis. */
    IrtRType XYAngle;                   /* Angle in degrees in the XY plane. */
    UserGCObsPtEffectiveDistanceFuncType DistanceDecay;
    UserGCObsPtEffectiveDistanceParamStruct DistanceDecayParam;
} UserGCObsPtTypeStruct;

typedef struct UserGCObsPtSuggestionStruct {
    IrtPtType ObsPt; /* If it's USER_GC_INF_VEC then the ObsPt is at         */
                     /* infinity.                                            */
    IrtVecType Direction;
} UserGCObsPtSuggestionStruct;

typedef struct UserGCObsPtGroupTypeStruct {
    UserGCObsPtTypeStruct ObsPtType;
    IrtRType Weight; /* When weight is non negative the best arrangement is  */
                     /* chosen as the arrangement who minimizes the overall  */
                     /* weight.                                              */
    /* Limit the amount of this ObsPt in the final arrangement.              */
    /* (-INF,0]: no limit.                                                   */
    /* (0,1]: percent of total number of ObsPts.                             */
    /* (1,INF): number of this ObsPt. (0 ObsPt is actually the same as 0     */
    /*          percent).                                                    */
    IrtRType MinAmount, MaxAmount;
    /* The suggested positions for the obesrvations points = obeservation    */
    /* space. Null terminated vector.                                        */
    UserGCObsPtSuggestionStruct **Suggestions;
} UserGCObsPtGroupTypeStruct;

typedef enum UserGCSetCoverAlgorithmType {
    USER_GC_GREEDY,
    USER_GC_EXHAUSTIVE,
    USER_GC_EXACT,
    USER_GC_ALGORITHM_NUM
} UserGCSetCoverAlgorithmType;

typedef struct UserGCSetCoverParams {
    union {
        struct {
            int BestChosenAmount; /* How many best choices the greedy check. */
        } Greedy;
        struct {
            IrtRType CoverLimit;
            int SizeLimit;
        } Exhaustive;
        struct {
            int SizeLimit;
        } Exact;
    };
    UserGCSetCoverAlgorithmType Algorithm;
} UserGCSetCoverParams;

/* In both functions FileName is the name without the extension. */
typedef IrtImgPixelStruct *(*UserGCLoadRgbImageFromFileFuncType)
						    (const char* FileName, 
						     int *Width, 
						     int *Height);
typedef int (*UserGCSaveRgbImageToFileFuncType)(const char* FileName, 
                                                IrtImgPixelStruct* VisMap, 
                                                int Width, 
                                                int Height);

typedef struct UserGCSolvingParams {
    int VisMapWidth;
    int VisMapHeight;
    IrtRType VisMapTanAng;
    IrtRType VisMapCriticAR;
    int Srf2PlgFineness;
    UserGCSetCoverParams SetCoverParams;
} UserGCSolvingParams;

typedef struct UserGCDebugParams {
    /* Don't do the set cover, display all the visibility maps. */
    int DisableSetCover;
    /* Store the final processed geometic object and obstacle object before  */
    /* observation points influence as ibd files.                            */
    int StoreObjectsBeforeOPs;
    /* For each observation point save the object after the OP's influence   */
    /* and after the transformations to the rendering space.                 */
    int StoreObjectsForOPs;
    /* Don't delete the visiblity map from disk after creation. */
    int StoreVisMap;
    /* Don't create visibility maps. Load them from disk. */
    int LoadVisMaps;
    /* Save images as ppm instead of PNG. */
    int SaveVisMapAsPPM;
    /* The function to load and save images. Must be compatible.             */
    /* If any of the load or save functions are NULL, they are both replaced */
    /* by default funtions for PPM image.                                    */
    /* LoadImageFunc must allocate memory using IritMalloc.                  */
    UserGCLoadRgbImageFromFileFuncType LoadImageFunc;
    UserGCSaveRgbImageToFileFuncType SaveImageFunc;
} UserGCDebugParams;

typedef struct UserGCProblemDefinitionStruct {
    UserGCObsPtGroupTypeStruct **ObsPtsGroups;     /* NULL terminated vector.*/
    IPObjectStruct *GeoObj;
    IPObjectStruct *Obstacles;
    UserGCSolvingParams SolvingParams;
    UserGCDebugParams DebugParams;
    IrtRType CoverAmount;
} UserGCProblemDefinitionStruct;

typedef struct UserGCSolutionIndexStruct {
    int ObsPtGroupIndex, /* The indexes of the visibility map in the groups  */
        SuggestionIndex, /* and suggestions.                                 */
        VisMapIndex; /* The index of the visibility map in the visibility map*/
                     /* list.                                                */
} UserGCSolutionIndexStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern const int 
    USER_GC_NOT_USED;
extern const IrtPtType 
    USER_GC_INF_VEC;
extern const UserGCRgbToGrayStruct 
    USER_GC_COLOR_TO_GRAY_MAP[];
extern const int 
    USER_GC_COLOR_TO_GRAY_MAP_SIZE;

/* Distance functions. */
IrtRType UserGCObsPtDistanceStepFunc(
			    UserGCObsPtEffectiveDistanceParamStruct *Params,
			    IrtRType Distance);
IrtRType UserGCObsPtDistanceUnifromFunc(
			    UserGCObsPtEffectiveDistanceParamStruct *Params,
			    IrtRType Distance);

int UserGCSolveGeoProblem(UserGCProblemDefinitionStruct *Problem,
                          UserGCSolutionIndexStruct *** SolutionOps,
                          IrtRType *CoverPercentage,
                          IrtImgPixelStruct ***VisMaps);
void UserGCCreateViewMatrix(UserGCObsPtSuggestionStruct *Op, 
                            IrtHmgnMatType ViewMat);
MiscISCPixelType *UserGCFlatVisMap(int VisMapWidth,
				   int VisMapHeight,
				   IrtImgPixelStruct *Pixels,
				   MiscISCPixelType *VisMap,
				   const UserGCRgbToGrayStruct *RgbToGrayMap,
				   int SizeOfRgbToGraymap);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GEOM_COVER_H */
