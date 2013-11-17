/*****************************************************************************
* GeomCovr.c - Calculating Geometric Covering.                               *
******************************************************************************
* Written by Nadav Shragai, February 2010.                                   *
*****************************************************************************/

#include <setjmp.h>
#include <time.h>

#include "geom_lib.h"
#include "rndr_lib.h"

#include "geomcovr.h"

const int 
    USER_GC_NOT_USED = -1;
const IrtPtType 
    USER_GC_INF_VEC = { IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY };
const int USERG_GC_TIME_ZONE = 3;

/* Set the transformation of RGB map to gray map. The first three nubmers   */
/* are the RGB origin value and the forth is the gray destination value.    */
const UserGCRgbToGrayStruct USER_GC_COLOR_TO_GRAY_MAP[] = 
    {{{0, 125, 0}, 1}, 
     {{255, 0, 0}, 0}, 
     {{255, 255, 0}, 0},
     {{255, 255, 255}, 0}};
const int 
    USER_GC_COLOR_TO_GRAY_MAP_SIZE = sizeof(USER_GC_COLOR_TO_GRAY_MAP) / 
                                     sizeof (UserGCRgbToGrayStruct);
IRIT_STATIC_DATA const UserGCRgbToGrayStruct USER_GC_VISMAP_TO_MAPPED_MAP[] = 
    {{{0, 125, 0}, 1}, 
     {{255, 0, 0}, 1}, 
     {{255, 255, 0}, 1},
     {{255, 255, 255}, 0}};
IRIT_STATIC_DATA const int 
    USER_GC_VISMAP_TO_MAPPED_MAP_SIZE = sizeof(USER_GC_VISMAP_TO_MAPPED_MAP) / 
				        sizeof (UserGCRgbToGrayStruct);
IRIT_STATIC_DATA const char 
    *USER_GC_VISMAP_FILE_NAME_BASE = "VisMap",
    *USER_GC_PROCESSED_GEO_OBJ_NAME = "ProcessedGeoObj",
    *USER_GC_PROCESSED_OBSTACLES_NAME = "ProcessedObstacles";

/* Variables for the view map image creation (UserGCProblemDefinitionStruct  */
/* can't be given to the relevant functions).                                */
IRIT_STATIC_DATA IrtImgPixelStruct 
    *UserGCVisMap = NULL;
IRIT_STATIC_DATA int UserGCVisMapLine;
IRIT_STATIC_DATA int UserGCVisMapSize[2];

/* For exiting inner function and returning without a solution. */
IRIT_STATIC_DATA jmp_buf UserGCStartOfProcess;

/* Number of times to attempt saving or loading image. */
IRIT_STATIC_DATA const int
    USER_GC_NUMBER_FILE_ATTEMPTS = 5;

static void SaveObjectAndMatrix(char* FileName, 
				IrtHmgnMatType Mat, 
				IPObjectStruct *PObj);
static void UserGCCreatePrspMatrix(UserGCObsPtGroupTypeStruct *OpGroup, 
                                   IrtHmgnMatType **PrspMat);
static void UserGCScanObjects(IRndrPtrType Rend,
                              IPObjectStruct *Scene,
                              int PicIndex,
                              int SaveObject);
static IrtImgPixelStruct *UserGCCreateVisMap(UserGCProblemDefinitionStruct *Problem,     
					     IPObjectStruct *Scene, 
					     UserGCObsPtSuggestionStruct *Op,
					     IrtHmgnMatType PrspMat,
					     int PicIndex);
static int UserGCCreateVisMaps(UserGCProblemDefinitionStruct *Problem, 
                               IrtImgPixelStruct ***VisMaps);
static int UserGCSolveSetcover(UserGCProblemDefinitionStruct *Problem, 
                               IrtImgPixelStruct **VisMaps,
                               int **SolutionByIndex,
                               int *SolutionSize,
                               IrtRType *CoverPart);
static UserGCSolutionIndexStruct **UserGCSetSolutionSuggestion(
					    UserGCObsPtGroupTypeStruct **OpGroups,
					    int SuggestionNum,
					    int *SolutionOpsIndex,
					    int SolutionOpsNum,
					    IrtImgPixelStruct **VisMaps);
static void UserGCPrepareScene(UserGCProblemDefinitionStruct *Problem);
static IPObjectStruct *UserGCPrepareObjAux(IPObjectStruct* PObj, 
                                           void *Param);
static IPObjectStruct *UserGCPrepareObj(IPObjectStruct *PObj, 
                                        int MapWidth, 
                                        int MapHeight);
static int UserGCGetOPsNum(UserGCProblemDefinitionStruct *Problem);
static IrtImgImageType UserGCImgWriteSetType(const char *ImageType);
static int UserGcImgWriteOpenFile(const char **argv,
                                  const char *FName,
                                  int Alpha,
                                  int XSize,
                                  int YSize);
static void UserGCImgWritePutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels);
static void UserGCImgWriteCloseFile(void);
static FILE* UserGCOpenFile(const char *FileName, int ForWriting, char* ErrMsg);
static int UserGCSavePpmImageToFile(const char* FileName, 
                                    IrtImgPixelStruct* VisMap, 
                                    int Width, 
                                    int Height);
static IrtImgPixelStruct *UserGCLoadPpmImageFromFile(const char* FileName, 
                                                     int *Width, 
                                                     int *Height);
static void UserGCSaveVisMap(int Index, 
                             IrtImgPixelStruct* VisMap, 
                             int MapWidth, 
                             int MapHeightm,
                             UserGCSaveRgbImageToFileFuncType SaveImageFunc);
static IrtImgPixelStruct *UserGCLoadVisMap(
                            int Index, 
                            int Width, 
                            int Height,
                            UserGCLoadRgbImageFromFileFuncType LoadImageFunc);
static void UserGCDeleteVisMap(int StartIndex, int EndIndex, int PPMImage);
static int UserGCSaveObjectToFile(const char* FileName, 
                                  IPObjectStruct* PObj,
                                  int IsBinary,
                                  int AllTheList);
static IPObjectStruct *UserGCLoadObjectFromFile(const char* FileName, 
                                                int IsBinary);
static void UserGCSaveProcessedObjects(IPObjectStruct *GeoObj,
                                       IPObjectStruct *Obstacles);
static IPObjectStruct *UserGCLoadProcessedObjects(IPObjectStruct **GeoObj,
                                                  IPObjectStruct **Obstacles);
static void UserGCDeleteProcessedObjects();

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Distance function. Step function. Return Params -> Step.Near for distanceM
*   less or equal to Params -> Step.Step or Params.Step.Far for distance     M
*   greater than Params -> Step.Step.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Params:   The parameters defnining this function.                        M
*   Distance: The distnace for which the observation point efficiently is    M
*             evaluated.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The value calculated for the given distance.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserGCObsPtEffectiveDistanceParamStruct,                                 M
*   UserGCObsPtEffectiveDistanceFuncType                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCObsPtDistanceStepFunc                                              M
*****************************************************************************/
IrtRType UserGCObsPtDistanceStepFunc(
			       UserGCObsPtEffectiveDistanceParamStruct *Params,
			       IrtRType Distance)
{
    if (Distance > Params -> Step.Step)
        return Params -> Step.Far;
    else 
        return Params -> Step.Near;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Distance function. Return Params -> Uniform.Value.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Params:   The parameters defnining this function.                        M
*   Distance: The distnace for which the observation point efficiently is    M
*             evaluated.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: The value calculated for the given distance.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserGCObsPtEffectiveDistanceParamStruct,                                 M
*   UserGCObsPtEffectiveDistanceFuncType                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCObsPtDistanceUnifromFunc                                           M
*****************************************************************************/
IrtRType UserGCObsPtDistanceUnifromFunc(
			    UserGCObsPtEffectiveDistanceParamStruct *Params,
			    IrtRType Distance)
{
    return Params -> Uniform.Efficiency;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Open the given file for reading or writing. Tries                        *
* USER_GC_NUMBER_FILE_ATTEMPTS times, each time shows the given error        *
* message. If IRIT_WARNING_MSG_PRINTF activates function which wait for user *
* input, this will allow the user to fix the problem.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:   The name of the file.                                        *
*   ForWriting: TRUE to open the file for writing. FALSE for reading.        *
*   ErrMsg:     The message to display if error occurs. It should be phrased *
*               so that this function could add the name of the file directly*
*               after it. e.g.: "Error opening file".                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   FILE*: The opened file or NULL if failed.                                *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCOpenFile                                                           *
*****************************************************************************/
static FILE* UserGCOpenFile(const char *FileName, int ForWriting, char* ErrMsg)
{
    FILE *File;
    int i;

    for (i = 1; i<=USER_GC_NUMBER_FILE_ATTEMPTS; i++) {
        if ((File = fopen(FileName, ForWriting ? "wb" : "rb")) == NULL) {
            IRIT_WARNING_MSG_PRINTF(
                "%s \"%s\".\nAttempt %d out of %d.", ErrMsg, FileName, i,
                USER_GC_NUMBER_FILE_ATTEMPTS);
        }
        else 
            return File;
    }
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saving image in a ppm 6 format.                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName: The name of the file to save (without an extension).           M
*   VisMap:   The visibility map to save.                                    M
*   Width:    Width of the map.                                              M
*   Height:   Height of the map.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: FALSE if failed saving (Error message is produced by the function). M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCSavePpmImageToFile, UserGCLoadPpmImageFromFile                     M
*****************************************************************************/
int UserGCSavePpmImageToFile(const char* FileName, 
                             IrtImgPixelStruct* VisMap, 
                             int Width, 
                             int Height)
{
    char FullFileName[IRIT_LINE_LEN_VLONG];
    int i;

    sprintf(FullFileName, "%s.ppm", FileName);

    IrtImgWriteSetType("ppm6");
    if (!IrtImgWriteOpenFile(NULL, FullFileName, FALSE, Width, Height)) {
        IRIT_WARNING_MSG_PRINTF("Can't open for writing file %s.\n", FileName);
        return FALSE;
    }
    for (i = 0; i <= Height - 1; i++)
        IrtImgWritePutLine(NULL, VisMap + i * Width);
    IrtImgWriteCloseFile();
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Loading image from a file into format of MiscISCPixelType.               M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName: IN, The name of the file to load.                              M
*   Width:    OUT, The width of the image.                                   M
*   Height:   OUT, The height of the image.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtImgPixelStruct *: The loaded visibility map or NULL if failed (Error  M
*                        message is produced by the function).               M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCLoadPpmImageFromFile, UserGCSavePpmImageToFile                     M
*****************************************************************************/
IrtImgPixelStruct *UserGCLoadPpmImageFromFile(const char* FileName, 
                                              int *Width, 
                                              int *Height)
{
    int Alpha;
    IrtImgPixelStruct 
        *Res;
    char FullFileName[IRIT_LINE_LEN_VLONG];

    sprintf(FullFileName, "%s.ppm", FileName);
    Alpha = FALSE;
    Res = IrtImgReadImage(FullFileName, Width, Height, &Alpha);
    (*Width)++;
    (*Height)++;
    if (Res == NULL)
        return NULL;
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Saving visibilty map with the given index to disk.                       *
*   If an error occur it longjmp to UserGCStartOfProcess.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Index:         The index of the visibility map.                          *
*   VisMap:        The visibility map to save.                               *
*   MapWidth:      The width of the image.                                   *
*   MapHeight:     The height of the image.                                  *
*   SaveImageFunc: Function for saving the image.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveVisMap, UserGCLoadVisMap, UserGCDeleteVisMap                   *
*****************************************************************************/
static void UserGCSaveVisMap(int Index, 
                             IrtImgPixelStruct* VisMap, 
                             int MapWidth, 
                             int MapHeight,
                             UserGCSaveRgbImageToFileFuncType SaveImageFunc)
{
    char FileName[IRIT_LINE_LEN];

    sprintf(FileName, "%s%d", USER_GC_VISMAP_FILE_NAME_BASE, Index);
    if (!SaveImageFunc(FileName, VisMap, MapWidth, MapHeight))
        longjmp(UserGCStartOfProcess, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loading visibilty map with the given index from disk.                    *
* where UserGCSaveVisMap saved it earlier.                                   *
*   If an error occur it longjmp to UserGCStartOfProcess.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Index:         The index of the visibility map.                          *
*   Width:         The width of the image.                                   *
*   Height:        The height of the image.                                  *
*   LoadImageFunc: Function for loading the image.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtImgPixelStruct: The loaded visibility map.                            *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveVisMap, UserGCLoadVisMap, UserGCDeleteVisMap                   *
*****************************************************************************/
static IrtImgPixelStruct *UserGCLoadVisMap(
                        int Index, 
                        int Width, 
                        int Height,
                        UserGCLoadRgbImageFromFileFuncType LoadImageFunc)
{
    IrtImgPixelStruct * Res;
    int LocalWidth, LocalHeight;
    char FileName[IRIT_LINE_LEN];

    sprintf(FileName, "%s%d", USER_GC_VISMAP_FILE_NAME_BASE, Index);
    Res = LoadImageFunc(FileName, &LocalWidth, &LocalHeight);
    if (!Res)
        longjmp(UserGCStartOfProcess, 1);
    if ((LocalWidth != Width) || (LocalHeight != Height)) {
        IRIT_WARNING_MSG_PRINTF("Width(%d) or height(%d) of the image in the "
            "file \"%s\" are incompatible. \n", LocalWidth, LocalHeight, 
            FileName);
        IritFree(Res);
        longjmp(UserGCStartOfProcess, 1);
    }
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deleting all visibilty maps at the given index range from disk.          *
*   No error produced if any of the files doesn't exist.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   StartIndex, EndIndex: The visibility maps indices range to delete.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveVisMap, UserGCLoadVisMap, UserGCDeleteVisMap                   *
*****************************************************************************/
static void UserGCDeleteVisMap(int StartIndex, int EndIndex, int PPMImage)
{
    char FileName[IRIT_LINE_LEN];
    int i;

    for (i = StartIndex; i<=EndIndex; i++) {
        sprintf(FileName, "%s%d.%s", USER_GC_VISMAP_FILE_NAME_BASE, i,
            PPMImage ? "ppm" : "png");
        remove(FileName);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Saving an object to file.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:   The name of the file (without extension, the extension .ibd  *
*               or .itd will added depneding on IsBinary).                   *
*   PObj:       The object to save. If NULL, an empty IP_OBJ_STRING object   *
*               file is created (an ugly patch to mark empty file).          *
*   IsBinary:   Whether to save it as a binary file.                         *
*   AllTheList: To save all the list starting with PObj.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if failed saving (Error message is produced by the function.  *
*        The function try to save for USER_GC_NUMBER_FILE_ATTEMPTS times).   *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveObjectToFile, UserGCLoadObjectFromFile                         *
*****************************************************************************/
static int UserGCSaveObjectToFile(const char* FileName, 
                                  IPObjectStruct* PObj,
                                  int IsBinary,
                                  int AllTheList)
{
    FILE *Out;
    int FilterDegen;  
    char Name[IRIT_LINE_LEN];
    IPObjectStruct 
        *PObj2 = PObj;

    if (IsBinary) {
        sprintf(Name, "%s.ibd", FileName);
    }
    else {
        sprintf(Name, "%s.itd", FileName);
    }

    Out = UserGCOpenFile(Name, TRUE, "Can't open for writing file");
    if (!Out)
        return FALSE;

    FilterDegen = IPSetFilterDegen(FALSE);
    if (PObj == NULL)
        PObj2 = IPGenSTRObject("EmptyObject");
    if (AllTheList) {
        IPObjectStruct *PObj3;
        
        for (PObj3 = PObj; PObj3 != NULL; PObj3 = PObj3 -> Pnext)
            IPPutObjectToFile(Out, PObj3, IsBinary);
    }
    else
        IPPutObjectToFile(Out, PObj2, IsBinary);
    if (PObj == NULL)
        IPFreeObject(PObj2);
    IPSetFilterDegen(FilterDegen);
    fclose(Out);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loading an object from file.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName: The name of the file.                                          *
*   IsBinary: Whether to load a binary file.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if failed loading (Error message is produced by the function. *
*        The function try to load for USER_GC_NUMBER_FILE_ATTEMPTS times).   *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveObjectToFile, UserGCLoadObjectFromFile                         *
*****************************************************************************/
static IPObjectStruct *UserGCLoadObjectFromFile(const char* FileName, 
                                                int IsBinary)
{
    FILE *In;
    IPObjectStruct *PObj;
    int FlattenObjects, PropagateAttrs;
    IPProcessLeafObjType ProcessLeafFunc;

    In = UserGCOpenFile(FileName, FALSE, "Can't open for reading file");
    if (!In)
        return NULL;
    
    /* Disable every thing. */
    FlattenObjects = IPSetFlattenObjects(FALSE);
    PropagateAttrs = IPSetPropagateAttrs(FALSE);
    ProcessLeafFunc = IPSetProcessLeafFunc(NULL);

    PObj = IPGetObjects(IPOpenStreamFromFile(In, TRUE, IsBinary, FALSE, FALSE));

    IPSetProcessLeafFunc(ProcessLeafFunc);
    IPSetPropagateAttrs(PropagateAttrs);
    IPSetFlattenObjects(FlattenObjects);
    
    if (!PObj) 
        IRIT_WARNING_MSG_PRINTF("Can't read file \"%s\".", FileName);
    fclose(In);

    if(IP_IS_STR_OBJ(PObj)) { /* Mark empty object. */
        IPFreeObject(PObj);
        PObj = IPGenPOLYObject(NULL);
    }
    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Saving both geometric object and obstacle object to disk so they can     *
* later be loaded using UserGCLoadProcessedObjects.                          *
*   If an error occur it longjmp to UserGCStartOfProcess.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   GeoObj:    The geometric object to save.                                 *
*   Obstacles: The obstacles to save.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveProcessedObjects, UserGCLoadProcessedObjects,                  *
*   UserGCDeleteProcessedObjects                                             *
*****************************************************************************/
static void UserGCSaveProcessedObjects(IPObjectStruct *GeoObj,
                                       IPObjectStruct *Obstacles)
{
    if (!UserGCSaveObjectToFile(USER_GC_PROCESSED_GEO_OBJ_NAME, GeoObj, TRUE, 
				FALSE))
        longjmp(UserGCStartOfProcess, 1);
    if (!UserGCSaveObjectToFile(USER_GC_PROCESSED_OBSTACLES_NAME, 
				Obstacles, TRUE, FALSE))
        longjmp(UserGCStartOfProcess, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loading both geometric object and obstacle object from disk.             *
* (Those file were supposed to be saved earlier by UserGCSaveProcessedObject)*
*   If an error occur it longjmp to UserGCStartOfProcess.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   GeoObj:    OUT, The loaded geometric object. If NULL the loaded objects  *
*              are returned by the result of this function.                  *
*   Obstacles: OUT, The loaded obstacles. If NULL, it's ignore.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct*: NULL or if GeoObj or Obstacles are NULL, one object     *
*                    containing GeoObj's polygons followed by Obstacles's    *
*                    polygons.                                               *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveProcessedObjects, UserGCLoadProcessedObjects,                  *
*   UserGCDeleteProcessedObjects                                             *
*****************************************************************************/
static IPObjectStruct *UserGCLoadProcessedObjects(IPObjectStruct **GeoObj,
                                                  IPObjectStruct **Obstacles)
{
    char FileName[IRIT_LINE_LEN];
    IPObjectStruct *PObj1, *PObj2;

    sprintf(FileName, "%s.ibd", USER_GC_PROCESSED_GEO_OBJ_NAME);
    PObj1 = UserGCLoadObjectFromFile(FileName, TRUE);
    if (!PObj1)
        longjmp(UserGCStartOfProcess, 1);

    sprintf(FileName, "%s.ibd", USER_GC_PROCESSED_OBSTACLES_NAME);
    PObj2 = UserGCLoadObjectFromFile(FileName, TRUE);
    if (!PObj2) {
        IPFreeObject(*GeoObj);
        longjmp(UserGCStartOfProcess, 1);
    }

    if (GeoObj) {
        *GeoObj = PObj1;
        if(Obstacles)
            *Obstacles = PObj2;
        return NULL;
    }
    else {
        IPPolygonStruct *Pl = PObj1 -> U.Pl;

        if (Pl == NULL) {
            PObj1 -> U.Pl = PObj2 -> U.Pl;
        }
        else {
            for (; Pl -> Pnext != NULL; Pl = Pl -> Pnext);
            Pl -> Pnext = PObj2 -> U.Pl;
        }
        PObj2 -> U.Pl = NULL;
        IPFreeObject(PObj2);
        return PObj1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deleting the geometric object and obstacles files.                       *
* (Those file were supposed to be saved earlier by UserGCSaveProcessedObject)*
*   No error is reported in case of an error.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCSaveProcessedObjects, UserGCLoadProcessedObjects,                  *
*   UserGCDeleteProcessedObjects                                             *
*****************************************************************************/
static void UserGCDeleteProcessedObjects()
{
    char FileName[IRIT_LINE_LEN];

    sprintf(FileName, "%s.ibd", USER_GC_PROCESSED_GEO_OBJ_NAME);
    remove(FileName);
    sprintf(FileName, "%s.ibd", USER_GC_PROCESSED_OBSTACLES_NAME);
    remove(FileName);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Saving the given matrix together with the object.                        *
*   Used for debug needs.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName: The name of the file to save.                                  *
*   Mat:      The matrix to save.                                            *
*   PObj:     The object to save.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveObjectAndMatrix(char* FileName, 
				IrtHmgnMatType Mat, 
				IPObjectStruct *PObj) {

    FILE *Out;
    int i,j;
    int FilterDegen;

    if ((Out = fopen(FileName, "w")) == NULL)
        assert("can't open file" == 0);
    if (Mat != NULL) {
        fprintf(Out,"[OBJECT VIEW_MAT\n\t[MATRIX");
        for(i = 0; i<=3; i++) {
            fprintf(Out, "\n\t\t");
            for(j = 0; j<=3; j++)
                fprintf(Out,"%-16.14lf ", Mat[i][j]);
        }
        fprintf(Out,"]\n]\n");
    }
    FilterDegen = IPSetFilterDegen(FALSE);
    IPPutObjectToFile(Out, PObj, FALSE);
    IPSetFilterDegen(FilterDegen);
    fclose(Out);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepare prespective matrix for the given observation point group type.   *
*   The prespective matrix is used in order to consider how wide is the      *
*   openning of the observation points.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   OpGroup:    in - The type of the observation points group that this      *
*               prespective matrix will be used for.                         *
*   PrspMat:    out -Prespective matrix to be used when evaluating the scene *
*               visibility map.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserGCCreatePrspMatrix(UserGCObsPtGroupTypeStruct *OpGroup, 
                                   IrtHmgnMatType **PrspMat) 
{
    /* todo: prespective matrix isn't supported yet. */
    *PrspMat = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepare view matrix for the given observation point. Using this matrix   *
*   any scene can be modifed to look the way it will be seen from the given  *
*   observation point.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Op:         in - The observation point from which the scene will be seen.*
*   ViewMat:    out - View matrix which set modification of the scene so it  *
*               will be seen from different location or/and direction.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void UserGCCreateViewMatrix(UserGCObsPtSuggestionStruct *Op, 
                           IrtHmgnMatType ViewMat) 
{
    IrtPtType Dir;   
    IrtHmgnMatType MatY, MatX;
    int UseMatY = TRUE;

    if (!IRIT_PT_APX_EQ(Op -> ObsPt, USER_GC_INF_VEC)) {
        MatGenMatTrans(-Op -> ObsPt[0], -Op -> ObsPt[1], -Op -> ObsPt[2], 
		       ViewMat);
    }
    else
        MatGenUnitMat(ViewMat);

    /* Rotating around the Y axis so the direction will be on the ZY plane. */
    IRIT_PT_COPY(Dir, Op -> Direction);
    Dir[1] = 0;
    if (IRIT_VEC_LENGTH(Dir) > IRIT_UEPS) {
        IRIT_PT_NORMALIZE(Dir);
        MatGenMatRotY(-Dir[2], Dir[0], MatY);
    }
    else
        UseMatY = FALSE;
    /* Rotating around the X axis so the direction will be at direction     */
    /* (0,0,-1).                                                            */
    IRIT_PT_COPY(Dir, Op -> Direction);
    Dir[2] = -sqrt(Dir[0]*Dir[0] + Dir[2]*Dir[2]);
    Dir[0] = 0;
    IRIT_PT_NORMALIZE(Dir);
    MatGenMatRotX(-Dir[2], -Dir[1], MatX);
    if (UseMatY)
        MatMultTwo4by4(ViewMat, ViewMat, MatY);
    MatMultTwo4by4(ViewMat, ViewMat, MatX);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scane all the polygons in the given scene into the visibility map.       *
*   The object inside Scene is modified. The vertices get their final values *
*   for the rendering. Additional changes may be involved as well. Don't use *
*   Scene again unless you know what you are doing (free it instead).        *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:       The render context.                                          *
*   Scene:      The scene to scan. Assuming it contain objects only in the   *
*               Scene->Pnext list and not the Scene->U.Lst list.             *
*   PicIndex:   The index of the picture created. For debug purposes only.   *
*   SaveObject: Whether to save the object before rendering. For debug       *
*               purposes only.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserGCScanObjects(IRndrPtrType Rend,
                              IPObjectStruct *Scene,
                              int PicIndex,
                              int SaveObject)
{
    /* todo: what about support for polystrip. */
    if (IP_IS_POLY_OBJ(Scene) && IP_IS_POLYGON_OBJ(Scene)) {
        if (Scene -> U.Pl != NULL) {
            IPPolygonStruct *Poly;

            IRndrBeginObject(Rend, Scene, FALSE);

            /* Debug issue. Saving the object. */
            if (SaveObject) {
                char Name[30];

                sprintf(Name, "ProcessedObject%d.itd", PicIndex);
                SaveObjectAndMatrix(Name, NULL, Scene);
            }
            for (Poly = Scene -> U.Pl;
                 Poly != NULL;
                 Poly = Poly -> Pnext) {
                IRndrPutTriangle(Rend, Poly);
            }
            IRndrEndObject(Rend);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create visibility map to the given scene from the given observation      *
*   point. Scene is freed during the process.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Problem:    The geometric covering problem to be solved (Contains the    *
*               the dimensions of the visibility map).                       *
*   Scene:      The scene to scan.                                           *
*   Op:         The observation point from which the scene will be seen.     *
*   PrspMat:    Prespective matrix to be used when evaluating the scene      *
*               visibility map.                                              *
*   PicIndex:   The index of the picture created. For debug purposes only.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtImgPixelStruct*: The visibility map of the given scene from the given *
*                       observation point.                                   *
*****************************************************************************/
static IrtImgPixelStruct *UserGCCreateVisMap(
				       UserGCProblemDefinitionStruct *Problem,
				       IPObjectStruct *Scene, 
				       UserGCObsPtSuggestionStruct *Op,
				       IrtHmgnMatType PrspMat,
				       int PicIndex)
{
    IrtHmgnMatType ViewMat;
    IRndrPtrType Rend;
    /* Actually, since I'm using vismap the background color is overwritten */
    /* in IRndrInitialize and it's meaningless here.                        */
    IRndrColorType
        BackGround = {255, 255, 255};
    IrtImgPixelStruct *Result;

    /* Notice that I don't initialize background color because for          */
    /* visibility map it's initialized inside IRndrInitialized.             */
    Rend = IRndrInitialize(Problem -> SolvingParams.VisMapWidth,
                           Problem -> SolvingParams.VisMapHeight,
                           1, FALSE, TRUE, TRUE,/* todo: decide whether to  */
                                            /* use backface culling or not. */
                           BackGround , 0.0, TRUE);
    /* Set callback function that write the image to a buffer instead of    */
    /* a file.                                                              */
    IRndrSaveFileCB(Rend, UserGCImgWriteSetType, UserGcImgWriteOpenFile, 
		    UserGCImgWritePutLine, UserGCImgWriteCloseFile);
    IRndrSetShadeModel(Rend, IRNDR_SHADING_NONE);
    UserGCCreateViewMatrix(Op, ViewMat);
    IRndrSetViewPrsp(Rend, ViewMat, PrspMat, NULL);
    IRndrVisMapEnable(Rend, Scene, 1);   
    /* Setting the TanAng, CriticAr and UvDilation. Must mbe after the      */
    /* initialization in IRndrVisMapEnable.                                 */
    IRndrVisMapSetTanAngle(Rend,
	      cos((90 - Problem -> SolvingParams.VisMapTanAng) * (M_PI/180)));
    IRndrVisMapSetCriticAR(Rend, 
			   Problem -> SolvingParams.VisMapCriticAR);
    UserGCScanObjects(Rend, Scene, PicIndex, 
		      Problem -> DebugParams.StoreObjectsForOPs);
    IRndrVisMapScan(Rend);
    IPFreeObject(Scene);
    IRndrSaveFileVisMap(Rend, ".", "dummy.ppm", "ppm");
    IRndrDestroy(Rend);
    Result = UserGCVisMap;
    UserGCVisMap = NULL;
    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create visibility map of the scene for each observation point suggestion *
* and add it to the calculator. The scene is loaded using                    *
* UserGCLoadProcessedObjects.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Problem:    The geometric covering problem to be solved (Contains the    *
*               required observation points and other parameters).           *
*   VisMaps:    OUT, The created visibility maps. NULL terminated.           *
*                                                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if error occured.                                             *
*****************************************************************************/
static int UserGCCreateVisMaps(UserGCProblemDefinitionStruct *Problem, 
                               IrtImgPixelStruct ***VisMaps)
{
    int i, PicsNum;

    /* If we don't load visibility map we need to create them. */
    if (!Problem -> DebugParams.LoadVisMaps) {
        IPObjectStruct 
            *OriginScene = UserGCLoadProcessedObjects(NULL, NULL);
        UserGCObsPtGroupTypeStruct **OpGroup;
        IPObjectStruct *Scene; 
        int Index = -1;

        /* Creating all visibility maps, each visibility map is created,    */
        /* saved to disk and then freed. This is in order to reduce memory  */
        /* use during the creation of the rest of the visibility maps.      */
        for (OpGroup = Problem -> ObsPtsGroups; *OpGroup != NULL; OpGroup++) {
            UserGCObsPtSuggestionStruct **Op;
            IrtHmgnMatType *PrspMat;
            IrtImgPixelStruct *VisMap;

            UserGCCreatePrspMatrix(*OpGroup, &PrspMat);
            for (Op = (*OpGroup) -> Suggestions; *Op != NULL; Op++) {
                Index++;

                Scene = IPCopyObject(NULL, OriginScene, FALSE);
                printf("%s Creating image %d.\n", 
		       MiscISCGetTimeStamp((int) time(NULL), NULL, 
					   USERG_GC_TIME_ZONE), Index);
                VisMap = UserGCCreateVisMap(Problem,
					    Scene, *Op, *PrspMat, Index);
                /* In case of longjmp we will have memory leak of VisMap.   */
                /* We can live with that.                                   */
                UserGCSaveVisMap(Index, VisMap, 
				 Problem -> SolvingParams.VisMapWidth, 
				 Problem -> SolvingParams.VisMapHeight,
				 Problem -> DebugParams.SaveImageFunc);
                IritFree(VisMap);
            }
        }
        IPFreeObject(OriginScene);
    }

    PicsNum = UserGCGetOPsNum(Problem);
    /* Loading all visibility maps.                                         */
    (*VisMaps) = (IrtImgPixelStruct **)
        IritMalloc((PicsNum + 1) * sizeof(MiscISCPixelType*));
    (*VisMaps)[0] = NULL; 
    for (i = 0; i<= PicsNum - 1; i++) {
        printf("%s Loading image %d\n", 
	       MiscISCGetTimeStamp((int) time(NULL), NULL,
				   USERG_GC_TIME_ZONE), i);
        (*VisMaps)[i] = UserGCLoadVisMap(i, 
				     Problem -> SolvingParams.VisMapWidth, 
				     Problem -> SolvingParams.VisMapHeight,
				     Problem -> DebugParams.LoadImageFunc);
        /* Needed in case of longjmp to terminate the visiblity maps        */
        /* created till now.                                                */
        (*VisMaps)[i+1] = NULL; 
    }

    if (!Problem -> DebugParams.StoreVisMap)
        UserGCDeleteVisMap(0, PicsNum - 1, 
			   Problem -> DebugParams.SaveVisMapAsPPM);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the smallest set of pictures in VisMaps which cover the entire      *
* image. Pixels with empty/degenerated color are subtracted from the image so*
* that the image required to be cover doesn't include them.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Problem:    The geometric covering problem to be solved.                 *
*   VisMaps:         IN, The visibility maps. NULL terminated. If doesn't    *
*   SolutionByIndex: OUT, The solution as indices of pictures by the order   *
*                    they were added to the solution.                        *
*   SolutionSize:    OUT, The size of the solution (size of SolutionByIndex).*
*   CoverPart:       OUT, The part of the image covered by the solution.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if error occured.                                             *
*****************************************************************************/
static int UserGCSolveSetcover(UserGCProblemDefinitionStruct *Problem, 
                               IrtImgPixelStruct **VisMaps,
                               int **SolutionByIndex,
                               int *SolutionSize,
                               IrtRType *CoverPart)
{
    MiscISCCalculatorPtrType Calc;
    int PicsNum, i,
        Res = FALSE;
    IrtImgPixelStruct **VisMap;
    MiscISCPixelType 
        *GrayVisMap = NULL; 

    PicsNum = UserGCGetOPsNum(Problem);
    /* Creating the calculator. */
    Calc = MiscISCNewCalculator(PicsNum, 
				Problem -> SolvingParams.VisMapWidth *
				Problem -> SolvingParams.VisMapHeight,
				MISC_ISC_BNW);

    if (!Calc)
        return FALSE;

    /* Adding all the pictures to the calculator. */
    for (i = 0, VisMap = VisMaps; (*VisMap) != NULL; VisMap++, i++) {
        /* Turn the visibility map to black and white in order to add it    */
        /* to the calculator.                                               */
        GrayVisMap = UserGCFlatVisMap(Problem -> SolvingParams.VisMapWidth, 
				      Problem -> SolvingParams.VisMapHeight,
				      VisMaps[i], GrayVisMap,
				      USER_GC_COLOR_TO_GRAY_MAP, 
				      USER_GC_COLOR_TO_GRAY_MAP_SIZE);
        MiscISCAddPicture(Calc, GrayVisMap);
    }

    /* Setting the image to be covered in the set cover problem.            */
    /* That image is a map with zeroes in all unmapped or degenerated       */
    /* location .                                                           */
    GrayVisMap = UserGCFlatVisMap(Problem -> SolvingParams.VisMapWidth, 
				  Problem -> SolvingParams.VisMapHeight,
				  VisMaps[0], GrayVisMap,
				  USER_GC_VISMAP_TO_MAPPED_MAP, 
				  USER_GC_VISMAP_TO_MAPPED_MAP_SIZE);
    MiscISCSetImageToCover(Calc, GrayVisMap);
    IritFree(GrayVisMap);

    /* Solving the set cover. */
    printf("%s Starts solving the set cover.\n", 
	   MiscISCGetTimeStamp((int) time(NULL), NULL, USERG_GC_TIME_ZONE));

    switch (Problem -> SolvingParams.SetCoverParams.Algorithm) {
        case USER_GC_GREEDY: {
            Res = MiscISCCalculateGreedy(Calc, SolutionByIndex, SolutionSize, 
				         CoverPart);
            break;
        }
        case USER_GC_EXHAUSTIVE: {
            Res = MiscISCCalculateExhaustive(Calc, 
                Problem -> SolvingParams.SetCoverParams.Exhaustive.CoverLimit,
                Problem -> SolvingParams.SetCoverParams.Exhaustive.SizeLimit,
                SolutionByIndex, SolutionSize, CoverPart);
            break;
        }
        case USER_GC_EXACT: {
            break;
        }
        default: {
            assert("Unknown algorithm type." == 0);
        }
    }

    printf("%s Finished solving the set cover.\n", 
	   MiscISCGetTimeStamp((int) time(NULL), NULL, USERG_GC_TIME_ZONE));

    /* Freeing the calculator. */
    MiscISCFreeCalculator(Calc);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Return the indices of Suggestions in OpGroups wich appear in the         *
* solution and their visiblity maps.                                         *
*   The visibility map of each observation point points to one of VisMaps'   *
* visibility map and therefore shouldn't be freed.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   OpGroups:         The original suggestions used for solving the problem. *
*   SuggestionNum:    Number of suggesstions in OpGropus.                    *
*   SolutionOpsIndex: Array with the indices of the suggestions which        *
*                     appears in the solution. The order of the suggestion   *
*                     (for which the index relay to) is starting from the    *
*                     suggestions in the first observation group of          *
*                     OpGroups and continue with the suggestions of the next *
*                     observation group of OpGroups and son on.              *
*   SolutionOpsNum:   Number of observation points solving the problem       *
*                     (which is the number of indices in SolutionOpsIndex)   *
*   VisMaps:          An array with the visibility maps of all the           *
*                     observation points suggested by OpGroups in an order   *
*                     compatible with SolutionOpsIndex.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserGCSolutionIndexStruct**: null terminated array of indices of         *
*       Suggestions in OpGroups which appears in the solution. Each elemnt   *
*       in the array contains the index of the observation point group, the  *
*       index of the suggestion in that observation point group and the      *
*       correspoinding visibility map inside VisMaps.                        *
*****************************************************************************/
static UserGCSolutionIndexStruct **UserGCSetSolutionSuggestion(
				       UserGCObsPtGroupTypeStruct **OpGroups,
				       int SuggestionNum,
				       int *SolutionOpsIndex,
				       int SolutionOpsNum,
				       IrtImgPixelStruct **VisMaps)
{
    int i,
        SuggestionIndex = 0, 
        ObsPtGroupIndex = 0,
        *SuggestionIndices = (int *) IritMalloc(sizeof(int*)*SuggestionNum),
        *ObsPtGroupIndices = (int *) IritMalloc(sizeof(int*)*SuggestionNum);
    UserGCSolutionIndexStruct **Res
        = IritMalloc((SolutionOpsNum + 1) * sizeof(UserGCSolutionIndexStruct*));

    Res[SolutionOpsNum] = NULL;
    for (i = 0; i <= SuggestionNum - 1; i++, SuggestionIndex++) {
        if (OpGroups[ObsPtGroupIndex] -> Suggestions[SuggestionIndex] == NULL){
            ObsPtGroupIndex++;
            SuggestionIndex = 0;
        }
        ObsPtGroupIndices[i] = ObsPtGroupIndex;
        SuggestionIndices[i] = SuggestionIndex;
    }

    for (i = 0; i<=SolutionOpsNum - 1; i++) {
        assert((SolutionOpsIndex[i] <= SuggestionNum) &&
            "The solution contains solutions that doesn't appear in the "
            "original problem");
        Res[i] = IritMalloc(sizeof(UserGCSolutionIndexStruct));
        Res[i] -> ObsPtGroupIndex = ObsPtGroupIndices[SolutionOpsIndex[i]];
        Res[i] -> SuggestionIndex = SuggestionIndices[SolutionOpsIndex[i]];
        Res[i] -> VisMapIndex = SolutionOpsIndex[i];        
    }
    IritFree(SuggestionIndices);
    IritFree(ObsPtGroupIndices);
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Preparing the scene to be rendered.                                      *
*   The objects stored in Problem->GeoObj and Problem->Obstacles are         *
* destroyed and mustn't be accessed again.                                   *
*   Both objects are going through the processing mentioned in               *
* UserGCPrepareObj. The new created objects are saved to disk using          *
* UserGCSaveProcessedObject and can be later accessed using                  *
* UserGCLoadProcessedObject. Problem->GeoObj and Problem->Obstacles are set  *
* to NULL.                                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Problem: IN OUT, The geometric covering problem to be solved.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct*: The prepared scene.                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCPrepareScene                                                       *
*****************************************************************************/
static void UserGCPrepareScene(UserGCProblemDefinitionStruct *Problem)
{
    /* Prespective matrix isn't supported yet. todo: support it.            */
    /* Add here transformation of prespective matrix.                       */

    Problem -> GeoObj = UserGCPrepareObj(Problem -> GeoObj, 
        Problem -> SolvingParams.VisMapWidth, 
        Problem -> SolvingParams.VisMapHeight);

    if (Problem -> Obstacles != NULL)
        Problem -> Obstacles = UserGCPrepareObj(Problem -> Obstacles, 0, 0);
    /* If it do longjmp, GeoObj and Obstacles will be freed at the setjmp. */
    UserGCSaveProcessedObjects(Problem -> GeoObj, Problem -> Obstacles);

    IPFreeObject(Problem -> GeoObj);
    IPFreeObject(Problem -> Obstacles);
    Problem -> GeoObj = NULL;
    Problem -> Obstacles = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Auxilary function for UserGCPrepareObjAux. Used in IPForEachObj2 in       *
*  order to combine all polygons object to one polygon object.               *
*  If PObj isn't a polygon, free it and returns NULL.                        *
*  When using this function with IPForEachObj2 the returned IPObjectStruct   *
*  is empty IPObjectStruct.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:  The polygons to add to Param.                                     *
*   Param: A polygon IPObjectStruct to add PObj -> U.Pl to.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct: Will return NULL.                                       *
*                                                                            *
* SEE ALSO:                                                                  *
*   UserGCPrepareObjAux, UserGCTravelObjFunc, UserGCTravelObjs.              *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserGCPrepareObjAux                                                      *
*****************************************************************************/
static IPObjectStruct *UserGCPrepareObjAux(IPObjectStruct* PObj, 
                                           void *Param)
{
    IPPolygonStruct *Pl;

    Pl = IPGetLastPoly(((IPObjectStruct*)Param) -> U.Pl);
    if (Pl == NULL)
        ((IPObjectStruct*)Param) -> U.Pl = PObj -> U.Pl;
    else{
        Pl -> Pnext = PObj -> U.Pl;
    }
    PObj -> U.Pl = NULL;
    IPFreeObject(PObj);
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepares an object which means:                                          *
*     * Unites all polygon objects into one polygon object.                  *
*     * Change the uv values of all objects so they won't collide with each  *
*       other's uv spaces.                                                   *
*   The returned object may differ from PObj. PObj mustn't be accessed again *
*   after calling this function.                                             *
*   The function works under the assumption that an IPProcessFreeForm        *
*   function is defined which converts surfaces to polygons.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:                The object to prepare. The object should already    *
*                        contain only object of type polygon connected only  *
*                        by pnext (not by U.Lst). This object can't be used  *
*                        anymore after this function returns. Use the return *
*                        object instead.                                     *
*   MapWidth, MapHeight: The dimension of the visibility map. Used in order  *
*                        to arrange the UV domain of all PObj's objects. If  *
*                        any of them is 0, the function won't handle the UV  *
*                        domain of the objects.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  The prepared scene.                                   *
*****************************************************************************/
static IPObjectStruct *UserGCPrepareObj(IPObjectStruct *PObj, 
                                        int MapWidth, 
                                        int MapHeight)
{
    IPObjectStruct *PObjList;

    /* Arrange the UV values of all objects in PObj in to one non           */
    /* overlapping UV space.                                                */
    if ((MapWidth != 0) && (MapHeight != 0))
        IRndrVisMapPrepareUVValuesOfGeoObj(PObj, MapWidth, MapHeight);
    /* Goes over all the objects in the list (using Pnext) and unites all   */
    /* polygon objects into one polygon object.                             */
    PObjList = IPGenPOLYObject(NULL);
    PObj = IPForEachObj2(PObj, UserGCPrepareObjAux, PObjList);
    IPFreeObjectList(PObj);
    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Return number of observation points in the given problem.                *
*                                                                            *
* PARAMETERS:                                                                *
*    Problem:   The problem to count its observation points.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: The number of observation points in the problem.                    *
*****************************************************************************/
static int UserGCGetOPsNum(UserGCProblemDefinitionStruct *Problem) 
{
    UserGCObsPtGroupTypeStruct **OpGroup;
    int Count = 0;

    for (OpGroup = Problem -> ObsPtsGroups; *OpGroup != NULL; OpGroup++) {
        UserGCObsPtSuggestionStruct **Op;

        for (Op = (*OpGroup) -> Suggestions; *Op != NULL; Op++) 
            Count++;
    }
    return Count;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Solve the geometric covering problem given by Problem and return the     M
*   solving observation points.                                              M
*   The geometric object and obastacles of Problem are going through further M
*   processing and becoming compatible with the objects sent to the          M
*   visibility map generator.                                                M
*   The function may fail in which case errors will be anounced to the user  M
*   and it returns FALSE.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Problem:         IN, The geometric covering problem to be solved.        M
*   SolutionOps:     OUT, Null terminated array of indices of Suggestions in M
*                    ObsPtsGroups -> ObsPtsGroups which appears in the       M
*                    solution. Each elemnt in the array contains the index ofM
*                    the observation point group, the index of the suggestionM
*                    in that observation point group and the the index of theM
*                    corresponding visibility map in VisMaps.                M
*   CoverPart:       OUT, parameter. Will hold the part of the cover picture M
*                    that we succeeded to cover.                             M
*   VisMaps:         OUT - Will hold the visibility maps for all the         M
*                    observation points. NULL terminated.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: Return FALSE if encoutntered error.                                 *
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCSolveGeoProblem                                                    M
*****************************************************************************/
int UserGCSolveGeoProblem(UserGCProblemDefinitionStruct *Problem,
                          UserGCSolutionIndexStruct *** SolutionOps,
                          IrtRType *CoverPart,
                          IrtImgPixelStruct ***VisMaps)
{
    int Res,
        PicsNum = UserGCGetOPsNum(Problem), 
        *SolutionOpsIndex = NULL, 
        SolutionOpsNum = 0;

    if (setjmp(UserGCStartOfProcess)) {
        IrtImgPixelStruct **VisMap;


        if (!Problem -> DebugParams.StoreVisMap)
            UserGCDeleteVisMap(0, PicsNum - 1, 
                Problem -> DebugParams.SaveVisMapAsPPM);
        UserGCDeleteProcessedObjects();
        IPFreeObject(Problem -> GeoObj);
        IPFreeObject(Problem -> Obstacles);
        for (VisMap = *VisMaps; *VisMap != NULL; VisMap++)
            IritFree(*VisMap);
        IritFree(*VisMaps);
        return FALSE;
    }

    if ((Problem -> DebugParams.LoadImageFunc == NULL) ||
        (Problem -> DebugParams.SaveImageFunc == NULL)) {
        Problem -> DebugParams.LoadImageFunc = UserGCLoadPpmImageFromFile;
        Problem -> DebugParams.SaveImageFunc = UserGCSavePpmImageToFile;
    }

    /* Create the scene for render. */
    UserGCPrepareScene(Problem);
    /* Create all the visiblity maps. (Must use VisMaps itself in case of    */
    /* logjmp).                                                              */
    UserGCCreateVisMaps(Problem, VisMaps);

    /* Solving the set cover. */
    if (!Problem -> DebugParams.DisableSetCover) {
        Res = UserGCSolveSetcover(Problem, *VisMaps, &SolutionOpsIndex, 
            &SolutionOpsNum, CoverPart);
    }
    else {
        int i;
        SolutionOpsNum = PicsNum;
        SolutionOpsIndex = IritMalloc(sizeof(int) * SolutionOpsNum);
        for (i = 0; i <= SolutionOpsNum - 1; i++)
            SolutionOpsIndex[i] = i;
        Res = TRUE;
    }

    if (Res) {
        /* Creating the result from the solutions of the set cover. */
        *SolutionOps = UserGCSetSolutionSuggestion(Problem -> ObsPtsGroups, 
            PicsNum, SolutionOpsIndex, SolutionOpsNum, *VisMaps);
        IritFree(SolutionOpsIndex);
    }
    else {
        *SolutionOps = (UserGCSolutionIndexStruct**) 
            IritMalloc(sizeof(UserGCSolutionIndexStruct*));
        (*SolutionOps)[0] = NULL;
    }
    UserGCLoadProcessedObjects(&Problem -> GeoObj, &Problem -> Obstacles);
    if (!Problem -> DebugParams.StoreObjectsBeforeOPs) {
        UserGCDeleteProcessedObjects();
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Image type has no meaning since the image is written to a buffer.        *
*   Therefore this function just return an arbitrary image type.             *
*   It just retuns IRIT_IMAGE_PPM3_TYPE.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   ImageType:  A string describing the image type.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtImgImageType:  Returns IRIT_IMAGE_PPM3_TYPE as the detected type.     *
*****************************************************************************/
static IrtImgImageType UserGCImgWriteSetType(const char *ImageType)
{
    return IRIT_IMAGE_PPM3_TYPE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepare the variables required for keeping the image in a buffer.        *
*   The first three parameters aren't used. They are part of the required    *
*   signature for the use of this function.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   argv:     Pointer to the name of this program. Not used.                 *
*   FName:    Filename to open. Not used.                                    *
*   Alpha:    Do we have aan alpha channel. Not used.                        *
*   XSize:    X dimension of the image.                                      *
*   YSize:    Y dimension of the image.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if successful, FALSE otherwise. It never fails unless malloc  *
*         fails.                                                             *
*****************************************************************************/
static int UserGcImgWriteOpenFile(const char **argv,
                                  const char *FName,
                                  int Alpha,
                                  int XSize,
                                  int YSize)
{
    /* The semantic is that in order to use the created image (after it      */
    /* finished), one must either set UserGCVisMap to NULL or copy its       */
    /* content. Therefore, freeing it here, either frees a NULL pointer or   */
    /* frees a previously unused or copied image.                            */

    IritFree(UserGCVisMap);
    UserGCVisMap = 
        (IrtImgPixelStruct *) IritMalloc(XSize * YSize * 
					 sizeof(IrtImgPixelStruct));
    UserGCVisMapSize[0] = XSize;
    UserGCVisMapSize[1] = YSize;
    UserGCVisMapLine = 0;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Store a line of pixels to the image buffer. Ignore the alpha channel.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Alpha:  array of alpha values.                                           *
*   Pixels: array of color pixels.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserGCImgWritePutLine(IrtBType *Alpha, IrtImgPixelStruct *Pixels)
{
    int i,
        Start = UserGCVisMapLine * UserGCVisMapSize[0];
    IrtImgPixelStruct
        *VisMap = UserGCVisMap + Start;

    for (i = 0; i <= UserGCVisMapSize[0] - 1; i++) {
        IRIT_GEN_COPY(&VisMap[i], &Pixels[i], sizeof(IrtImgPixelStruct));
    }
    UserGCVisMapLine++;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   No action is actually required to close the buffer image so this is just *
*   a dummy function.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserGCImgWriteCloseFile(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Turns an RGB visibilty maps into a gray map using RgbToGrayMap.          M
*                                                                            *
* PARAMETERS:                                                                M
*   VisMapWidth:        The width of the visibility map.                     M 
*   VisMapHeight:       The height of the visibility map.                    M
*   Pixels:             The map.                                             M
*   VisMap:             Create the map in this buffer. If NULL, allocates    M
*                       new buffer.                                          M
*   RgbToGrayMap:       Map from RGB color to gray color.                    M
*   SizeOfRgbToGraymap: Size of RgbToGrayMap.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MiscISCPixelType *: A gray visibility map.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserGCFlatVisMap                                                         M
*****************************************************************************/
MiscISCPixelType *UserGCFlatVisMap(int VisMapWidth,
				   int VisMapHeight,
				   IrtImgPixelStruct *Pixels,
				   MiscISCPixelType *VisMap,
				   const UserGCRgbToGrayStruct *RgbToGrayMap,
				   int SizeOfRgbToGraymap)
{
    int i, j;

    if (VisMap == NULL)
        VisMap = (MiscISCPixelType*) IritMalloc(VisMapWidth * VisMapHeight *
            sizeof(MiscISCPixelType));    

    for (i = 0; i <= VisMapWidth * VisMapHeight - 1; i++) {
        for (j = 0; j <= SizeOfRgbToGraymap - 1; j++) {
            if ((Pixels[i].r == RgbToGrayMap[j].Rgb.r) &&
                (Pixels[i].g == RgbToGrayMap[j].Rgb.g) &&
                (Pixels[i].b == RgbToGrayMap[j].Rgb.b)) {
                VisMap[i] = RgbToGrayMap[j].Gray;
                break;
            }
        }
    }
    return VisMap;
}
