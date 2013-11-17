/******************************************************************************
* ImgStCvr.c images' set cover computations.                                  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Eyal Guthmann, December 2009                                     *
* Rewritten by Nadav Shragai July 2011                                        *
******************************************************************************/

/* This file contain few algorithm to solve a set cover problem of pictures. */
/* Using this unit:                                                          */
/*   1. Call MiscISCNewCalculator to recieve MiscISCCalculatorPtrType (Calc  */
/* for short) which is the context of this unit. Every function used in this */
/* unit is used with that context.                                           */
/*   2. Call MiscISCSetImageToCover to set the image needed to be covered.   */
/*   3. Call MiscISCAddPicture to add a picture which may cover the image.   */
/*      Call it for each picture.                                            */
/*   4. Call the algorithm you want to use in order to solve the set cover.  */
/* currently (July 2011) there are 3 algorithms: MiscISCCalculateExact,      */
/* MiscISCCalculateExhaustive and MiscISCCalculateGreedy.                    */
/*   5. Call MiscISCFreeCalculator to free all the calculator inner          */
/* memory allocations.                                                       */
/*   Section 2 above may be done before/after or in the middle of Section 3. */
/*                                                                           */
/*   The set cover problem is the problem of covering a given set with       */
/* minimum number of given sub sets.                                         */
/*   Here the set is called image and is given in section 2 above. The sub   */
/* sets are called pictures and are given in section 3 above. When working   */
/* B&W this description is precise. When working with gray we extend the     */
/* problem by adding magnitude of how much pixel in the image is needed to be*/
/* and how much pixel in a picture can cover.                                */
/*   The distinction between image and picture is done throughout this file  */
/* in order to ease the understanding of the algorithm.                      */
/* Both the image and the pictures must                                      */
/* have the same dimensions. Both are given as array of type MiscISCPixelType*/
/* and the size of the image (width*height). The semantic for both is        */
/* similar. For image, pixel with value 0 means that pixel is                */
/* not part of the image - the set to cover. In B&W, value 1 means that pixel*/
/* is part of the image. In gray, value greater than 0 means how much this   */
/* pixel is need to be covered. For pictures, pixel with value 0 menas that  */
/* pixel isn't part of the picture - the covering sub set and can't cover the*/
/* image.                                                                    */
/* In B&W, value 1 means that the picture contains this pixel. In gray, value*/
/* greater than 0 means how much this picture can cover this pixel.          */
/*                                                                           */
/* shrinking and zipping                                                     */
/* ---------------------                                                     */
/* In B&W, in order to speed the process we do 2 operations:                 */
/* shrinking - For each pixel in the image we create a pixel vector which    */
/* contains for each picture 0 if it contains this pixel or 1 if not. If     */
/* there are two identical pixel vectors, it's obvious that their            */
/* contribution to the final result is the same. Each picture which contains */
/* one pixel contains also the other and each picture which doesn't contain  */
/* one pixel, doesn't contains the other either. So we can use just one the  */
/* pixel vector instead of both. Shrinking is the operation of eliminating   */
/* all the similar pixel vectors and staying only with unique pixel vectors. */
/* Also, pixels which aren't contained in the image (no need to cover them)  */
/* or pixels which aren't contained in any picture are removed. We are left  */
/* with the same number of pictures but each picture is smaller and          */
/* represents only the relevant pixels.                                      */
/* zipping - Instead of using one byte for each pixel we are using one bit.  */
/* We use MiscISCUnitType which is x bytes long in order to keep the status  */
/* of 8x pixels.                                                             */
/*                                                                           */
/* In order to ease the understanding of the algorithm, we use the           */
/* terminology unprocessed before variables used for the pictures before     */
/* shrinking and zipping. For example, UnprocessedPictures is the array of   */
/* the pictures given by the user. Pictures is the array of the pictures     */
/* after shrinking and zipping.                                              */
/*                                                                           */


#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "irit_sm.h"
#include "misc_loc.h"
#include "time.h"

#define MISC_ISC_MAX_GRAY_COVER                     255
#define MISC_ISC_EPSILON                            0.01
#define MISC_ISC_PIXAL_HASH_TO_PIXEL_SIZE_RATIO     2

/* These messages must be compatbile to MiscISCErrorsType. */
IRIT_STATIC_DATA const char 
    *ERROR_MESSAGES[] = {
        "Unknown error occured",
        "Error allocating memory for creating the calculator.\n",
        "Error allocating memory",
        "Attempt to free memory that wasn't allocated using MiscISCMalloc",
        "The unit type MiscISCUnitType is bigger than expected",
        "Got illegal values in MiscISCSetImageToCover",
        "Can't add more pictures. Either reached maximum number of pictures or the calculater has already started the calculation",
        "Given picture isn't black and white",
        "Got NULL in one of the parameters in MiscISCHashUniquePixelsArray.",
        "Failed adding element to hashtable in MiscISCHashUniquePixelsArray.",
        "Got NULL in one of the parameters in MiscISCShrinkPictures.",
        "Got Calc NULL in MiscISCShrinkCalculator.",
        "Error creating hash table in MiscISCShrinkCalculator.",
        "Calc isn't ready in MiscISCUpdateGrayCoverability",
        "Already moved to compute phase. Can't do that again.",
        "Got ToCover NULL in MiscISCFindSplitIndex",
        "Logic error. Couldn't find previous index in MiscISCFindSplitIndex",
        "Got illgeal values in MiscISCExpendSolutionNode",
        "Got illgeal values in MiscISCSearchExact",
        "Can't call MiscISCInitExactorArray unless in compute phase",
        "Got illegal values in MiscISCCalculateExactAux",
        "Got illegal values in MiscISCCalculateExhaustiveAux",
        "Got illgeal values in MiscISCSearchExhaustive",
        "Can't activate the calculator without any covering pictures",
        "No covering pictures were added. Can't activate the calculator",
        "Can't activate the calculator twice",
    };

typedef enum MiscISCErrorsType {
    /* This message should never be sent through longjmp (since it's 0). */
    MISC_ISC_UNKNOWN_ERROR, 
    MISC_ISC_ERROR_CREATING_CALCULATOR,
    MISC_ISC_ERROR_ALLOCATING_MEMORY,
    MISC_ISC_MEMORY_WAS_NOT_ALLOCATED_WITH_MISC_ISC_MALLOC,
    MISC_ISC_UNIT_TYPE_BIGGER_THAN_EXPECTED,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SET_IMAGE_TO_COVER,
    MISC_ISC_TO_MANY_PICTURES_ADDED,
    MISC_ISC_PICTURE_NOT_BNW,
    MISC_ISC_NULL_IN_MISC_ISC_HASH_UNIQUE_PIXEL_ARRAY,
    MISC_ISC_ERROR_ADDING_ELEMENT_TO_HASHTABLE,
    MISC_ISC_NULL_IN_MISC_ISC_SHRINK_PICTURES,
    MISC_ISC_NULL_IN_MISC_ISC_SHRINK_CALCULATOR,
    MISC_ISC_ERROR_CREATING_HASH_TABLE,
    MISC_ISC_CALC_IS_NOT_READY_IN_MISC_ISC_UPDATE_GRAY_COVERABILITY,
    MISC_ISC_ALREADY_IN_COMPUTE_PHASE,
    MISC_ISC_NULL_IN_MISC_ISC_FIND_SPLIT_INDEX,
    MISC_ISC_LOGIC_ERROR_IN_MISC_ISC_FIND_SPLIT_INDEX,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_EXPEND_SOLUTION_NODE,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SEARCH_EXACT,
    MISC_ISC_MISC_ISC_INIT_EXACTOR_ARRAY_NOT_IN_COMPUTE_PHASE,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_CALCULATE_EXACT_AUX,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_CALCULATE_EXHAUSTIVE_AUX,
    MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SEARCH_EXHAUSTIVE_AUX,
    MISC_ISC_NO_COVERING_PICTURES_WERE_ADDED,
    MISC_ISC_ATTEMP_TO_ACTIVATE_CALCULATOR_TWICE
} MiscISCErrorsType;

/* Calculator will group pixels into this kind of unit. */
typedef unsigned long MiscISCUnitType;

/******************************************************************************
* DESCRIPTION:                                                                *
*   Return a value for a given picture. The greedy algorithm will choose the  *
* next picture to be the picture whichs return the highest value by this      *
* function. Sematic depents on the specific function.                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:        The calculator.                                              *
*   Picture:     The picture. Sematic depents on the specific function.       *
*   ToCover:     The pixels still requiring cover. Sematic depends on the     *
*                specific function.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCImageSizeType: The amount of uncovered pixel which can be covered  *
*                         by the given picture.                               *
******************************************************************************/
typedef MiscISCImageSizeType
    (*MiscISCGreedyGetPictureValueFuncType)(MiscISCCalculatorPtrType Calc,
                                            void *Picture,
                                            void *ToCover);

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates one pixel in the total Picture left to cover.                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Sematic depents on the specific*
*            function.                                                        *
*   Pixel:   The index of the pixel to update.                                *
*   Covered: TRUE - set the pixel as covered. FALSE - set the pixel as not    *
*            covered.                                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
typedef void (*MiscISCUpdatePixelInToCoverFuncType)(void *ToCover,
                                                    MiscISCImageSizeType Pixel,
                                                    int Covered);

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates the part of the total image left to cover after using the given   *
* picture.                                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   NewPicture:  The covering picture. Sematic depents on the specific        *
*                function.                                                    *
*   ToCover:     The pixels still requiring cover. Sematic depents on the     *
*                specific function.                                           *
*   SizeInBytes: The amount of bytes used by the picture.                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
typedef void (*MiscISCUpdateToCoverFuncType)(void *Picture,
                                             void *ToCover,
                                             MiscISCImageSizeType SizeInBytes);

/******************************************************************************
* DESCRIPTION:                                                                *
*   Checks if the given pixel need to be covered in the remaining cover.      *
* Return how much is still needed to be covered.                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Sematic depents on the specific*
*            function.                                                        *
*   Pixel:   The pixel's index.                                               *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: How much is still needed to be covered.                             *
******************************************************************************/
typedef int (*MiscISCNeedToCoverFuncType)(void *ToCover, 
                                          MiscISCImageSizeType Pixel);


typedef struct MiscISCFreeMemoryStruct {
    struct MiscISCFreeMemoryStruct *PNext;
    void *MemoryToFree;
} MiscISCFreeMemoryStruct;

typedef enum MiscISCUsePictureTypeType {
    MISC_ISC_USED = 1,
    MISC_ISC_NOT_USED = 0
} MiscISCUsePictureTypeType;

typedef struct MiscISCCalculatorStruct {
    /* Functions used in the algorithm with different behavior for B&W vs    */
    /* gray.                                                                 */
    MiscISCGreedyGetPictureValueFuncType GreedyGetPictureValue;
    MiscISCUpdatePixelInToCoverFuncType UpdatePixelInToCover;
    MiscISCUpdateToCoverFuncType UpdateToCover;
    MiscISCNeedToCoverFuncType NeedToCover;

    /* This should hold list of pointers to all the allocated memory. That   */
    /* way, if a longjmp is made, the memory can be freed.                   */
    MiscISCFreeMemoryStruct *MemoryToFree;
    /* Keep the stuck state for jumping. */
    jmp_buf MiscISCJmpTarget;

    int MaxPictures;
    int NumPictures;
    /* If true, the calculater was already moved to compute phase.           */
    int ComputePhase; 
    MiscISCColorTypeEnum ColorType;
    /* The size of the input unprocessed pictures. */
    MiscISCImageSizeType UnprocessedImageSize;
    /* The size of the processed pictures. The pictures which actually used  */
    /* in the set cover algorithms.                                          */
    MiscISCImageSizeType ImageSize;
    /* The size of the processed pictures in bytes. */
    MiscISCImageSizeType ImageSizeInBytes;
    /* The input uncompressed pictures. */
    MiscISCPixelType **UnprocessedPictures;
    /* The processed pictures. The pictures which actually used in the set   */
    /* cover algorithms.                                                     */
    void **Pictures;

    /* In B&W when pictures are processed they are shrinked so that each     */
    /* pixel in the processed pictures represents several pixels in the      */
    /* unprocessed picture. This array (size of ImageSize) holds for each    */
    /* pixel in the unprocessed image how much pixels it represents in the   */
    /* unprocessed picture.                                                  */
    MiscISCImageSizeType *PixelsAmount;
    /* Number of pixels in the unprocessed picture that need to be covered.  */
    MiscISCImageSizeType NumOfPixeslToCover;
    /* Describes the user's request of which part of unprocessed image to    */
    /* cover. See MiscISCSetImageToCover for the semantic of this array.     */
    /* Used only in the configuration phase.                                 */
    MiscISCPixelType *UnprocessedRequiredCover;
    /* Hold which part of the processed image need to be covered.            */
    /* The same semantic as UnprocessedRequiredCover. Used only in the       */
    /* computation phase.                                                    */
    MiscISCPixelType *RequiredCover;

    /* Parameters defining the algorithm to use for the solution. */
    int UseSizeLimit; /* Whether to limit the size of the solution. Relevant */
                      /* only for algorithms which use it.                   */
    int SizeLimit; /* If UseSizeLimit is TRUE, the algorithm will stop       */
                   /* searching the current branch of the tree when reaching */
                   /* SizeLimit size of the solution (continuing searching   */
                   /* would have enlarged the size of the solution).         */
    int UseCoverLimit; /* Whether to limit the search algorithm by the cover */
                       /* size. Relevant only for algorithms which use it.   */
    IrtRType CoverLimit; /* If UseCoverLimit is TRUE, the algorithm will stop*/
                         /* searching the current branch of the tree         */
                         /* (That emount of cover is enough for us, no need  */
                         /* to add more pictures to the solution).           */
                         /* The values are in (0,1).                         */
    /* Number of pixels in the unprocessed image need to be covered. Equal   */
    /* to CoverLimt*NumOfPixeslToCover.                                      */
    MiscISCImageSizeType CoverLimitInPixels;

    int SolutionSize;                            /* The found solution size. */
    MiscISCImageSizeType SolutionCover; /* The amount of pixels in the       */
                                        /* unprocessed image covered by the  */
                                        /* found solution.                   */
    int FoundSolution; /* TRUE iff we have already found some kind of        */
                       /* solution.                                          */
    int FoundFullSolution; /* TRUE iff the current found solution is a full  */
                           /* solution (as oppose to solution of             */
                           /* UseCoverLimit).                                */
    /* Both of the following hold the solution. */
    /* The first one holds indices for the solving pictures by the order they*/
    /* are intended to be given to the user.                                 */
    /* The second one holds array the size of number of pictures and for each*/
    /* picture tells whether it appears in the solution or not (obviously    */
    /* this solution doesn't hold any order for the solving pictures).       */
    /* Only one of them should exist (the other one is  NULL).               */
    /* SolutionByIndex is the output solution and therefore should not be    */
    /* freed by the calculator, it should be freed by the user.              */
    int *SolutionByIndex; 
    MiscISCUsePictureTypeType *Solution; 


    /* Auxilary data used internally. */
    int *MinFinder;
    int *VisibilitySetsSizes;
    IrtRType *SelectionRanks;
    IrtRType *TotalSelectionRanks;
    /* Can have value MISC_ISC_USED(1) which mean the picture is used.       */
    /* MISC_ISC_NOT_USED(0) which mean the picture is not used.              */   
    MiscISCUsePictureTypeType *UnusedPictures;
} MiscISCCalculatorStruct;

typedef struct MiscISCSltnNodeStruct *MiscISCSltnNodePtrType;
typedef struct MiscISCSltnNodeStruct {
    int Size; /* Number of pictures in the current combination. */
    MiscISCImageSizeType Cover; /* The number of pixels in the uprocessed    */
                                /* image covered by the current combination. */
    int PicNumber;          /* The index of the picture which is added here. */
    IrtRType SelectionRank;  /* The selection rank of the picture added here.*/
} MiscISCSltnNodeStruct;

static void MiscISCLongJmpCond(MiscISCCalculatorPtrType Calc, 
                               int Cond, 
                               MiscISCErrorsType MsgCode);
static int MiscISCHandleError(MiscISCCalculatorPtrType Calc,
                              int SetjmpReturnValue, 
                              char *FunctionName);
static void *MiscISCMalloc(MiscISCCalculatorPtrType Calc, unsigned int Size);
static void *MiscISCMallocAndSet(MiscISCCalculatorPtrType Calc, 
                                 unsigned int Size, 
                                 int Val);
static void MiscISCFree(MiscISCCalculatorPtrType Calc, void *P);
static unsigned long MiscISCArrayHashCallback(void *Arr, 
                                              MiscISCImageSizeType SizeInBytes, 
                                              unsigned int KeySizeBits);
static void *MiscISCHashArrayCopyCallback(void *Arr, 
                                          MiscISCImageSizeType SizeInBytes);
static void MiscISCHashFreeCallback(void *Arr);
static int MiscISCArrayCompCallBack(void *Arr1,
                                    void *Arr2,
                                    MiscISCImageSizeType SizeInbytes);
static int MiscISCGetZipFactor(MiscISCCalculatorPtrType Calc);
static MiscISCImageSizeType MiscISCGetZipSize(MiscISCCalculatorPtrType Calc,
                                              MiscISCImageSizeType Size);
static MiscISCUnitType MiscISCGetHighBitMask();
static int MiscISCIsZeroArray(void *Arr, MiscISCImageSizeType SizeInBytes);
static void *MiscISCArrayCopy(MiscISCCalculatorPtrType Calc, 
                              void *OutputArr,
                              void *InputArr, 
                              MiscISCImageSizeType SizeInBytes);
static MiscISCImageSizeType MiscISCBnWGreedyPicValue(
                                                  MiscISCCalculatorPtrType Calc,
                                                  void *Picture,
                                                  void *ToCover);
static MiscISCImageSizeType MiscISEGrayGreedyPictureValue(
                                                  MiscISCCalculatorPtrType Calc,
                                                  void *Picture,
                                                  void *ToCover);
static void MiscISCBnWUpdatePixelInToCoverFuncType(void *ToCover,
                                                   MiscISCImageSizeType Pixel,
                                                   int Covered);
static void MiscISCGrayWUpdatePixelInToCoverFuncType(void *ToCover,
                                                     MiscISCImageSizeType Pixel,
                                                     int Covered);
static void MiscISCBnWUpdateCover(void *NewPicture,
                                  void *ToCover,
                                  MiscISCImageSizeType Size);
static void MiscISCGrayUpdateCover(void *NewPicture,
                                   void *ToCover,
                                   MiscISCImageSizeType Size);
static int MiscISCBnWNeedToCover(void *ToCover, MiscISCImageSizeType Pixel);
static int MiscISCGrayNeedToCover(void *ToCover, MiscISCImageSizeType Pixel);
static void MiscISCFreeAllocatedMemoryOnError(MiscISCCalculatorPtrType Calc);
static void MiscISCHashUniquePixelsArray(MiscISCCalculatorPtrType Calc,
                                         MiscHashPtrType PixelHash,
                                         MiscISCImageSizeType *NeededPixels,
                                         MiscISCImageSizeType *TotalUsedPixels);
static void MiscISCShrinkPictures(MiscISCCalculatorPtrType Calc,
                                  MiscISCImageSizeType *NeededPixels,
                                  MiscISCImageSizeType TotalUsedPixels);
static void MiscISCShrinkCalculator(MiscISCCalculatorPtrType Calc);
static MiscISCUnitType *MiscISCZipPicture(MiscISCCalculatorPtrType Calc,
                                          MiscISCPixelType *Picture,
                                          MiscISCImageSizeType Size);
static void MiscISCZipCalculator(MiscISCCalculatorPtrType Calc);
static void MiscISCUpdateGrayCoverability(MiscISCCalculatorPtrType Calc);
static void MiscISCMoveCalcToComputePhase(MiscISCCalculatorPtrType Calc);
static int MiscISCCompareSelectionRanks(const void *s1, const void *s2);
static IrtRType MiscISCGetSelectionRank(MiscISCCalculatorPtrType Calc,
                                        int PictureIndex);
static IrtRType MiscISCGetTotalSelectionRank(MiscISCCalculatorPtrType Calc,
                                             int PixelIndex);
static MiscISCSltnNodePtrType MiscISCCreateNewSolutionNode(
                                          MiscISCCalculatorPtrType Calc,
                                          int Size, 
                                          MiscISCImageSizeType Cover,
                                          int PicNumber, 
                                          IrtRType SelectionRank);
static MiscISCSltnNodePtrType MiscISCCreateChildSltnNode(
                                          MiscISCCalculatorPtrType Calc,
                                          void *ToCover,
                                          MiscISCSltnNodePtrType SolutionNode,
                                          int PictureIndex);
static void MiscISCFindSplitIndex(MiscISCCalculatorPtrType Calc, 
                                  void *ToCover,
                                  MiscISCImageSizeType *SplitPixelIndex);
static void MiscISCExpendSolutionNode(MiscISCCalculatorPtrType Calc, 
                                      MiscISCSltnNodePtrType SolutionNode, 
                                      void *ToCover,
                                      MiscISCSltnNodePtrType **Children,
                                      int *NumChildren,
                                      MiscISCImageSizeType *SplitPixelIndex);
static void MiscISCSearchExact(MiscISCCalculatorPtrType Calc,
                               MiscISCSltnNodePtrType SolutionNode,
                               void *ToCover);
static void MiscISCFreeExactorArray(MiscISCCalculatorPtrType Calc);
static void MiscISCInitExactorArray(MiscISCCalculatorPtrType Calc);
static void MiscISCCalculateExactAux(MiscISCCalculatorPtrType Calc,
                                     int **SolutionByIndex,
                                     int *SolutionSize,
                                     IrtRType *CoverPart);
static int MiscISCStopSearch(MiscISCCalculatorPtrType Calc,
                             void *ToCover,
                             int Size,
                             MiscISCImageSizeType Cover);
static void MiscISCCalculateExhaustiveAux(MiscISCCalculatorPtrType Calc,
                                     int **SolutionByIndex,
                                     int *SolutionSize,
                                     IrtRType *CoverPart);
static void MiscISCSearchExhaustive(MiscISCCalculatorPtrType Calc,
                                    int StartTime,
                                    long *Leaves,
                                    void *ToCover,
                                    int Size,
                                    MiscISCImageSizeType Cover,
                                    int LastPicIndex);
static MiscISCImageSizeType MiscISCGetCoverAmount(
                                           MiscISCCalculatorPtrType Calc, 
                                           void *ToCover); 
static IrtRType MiscISCGetCoverPart(MiscISCCalculatorPtrType Calc, 
                                    MiscISCImageSizeType Cover);
static void MiscISCCalculateGreedyAux(MiscISCCalculatorPtrType Calc,
                                      int **SolutionByIndex,
                                      int *SolutionSize,
                                      IrtRType *CoverPart);

/******************************************************************************
* DESCRIPTION:                                                                *
*   If Cond is FALSE, long jump with the given message to MiscISCJmpTarget.   *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:    The calculator.                                                  *
*   Cond:    If TRUE, the function does nothing. If FALSE, longjmp with the   *
*            code message to MiscISCJmpTarget.                                *
*   MsgCode: Message code to send with the longjmp.                           *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCLongJmpCond(MiscISCCalculatorPtrType Calc, 
                               int Cond, 
                               MiscISCErrorsType MsgCode) 
{
    if (!Cond)
        longjmp(Calc -> MiscISCJmpTarget, MsgCode);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Check whether SetjmpResult (which is the return value of setjmp) is       *
* different from 0. If so, handle error message, free all calculator memory   *
* (including temporary memory) and return FALSE. Otherwise returns TRUE.      *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*   SetjmpReturnValue: The return value of setjmp call.                       *
*   FunctionName:      The name of the function for error message.            *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: FALSE if SetjmpResult is different from 0. Otherwise TRUE.          *
******************************************************************************/
static int MiscISCHandleError(MiscISCCalculatorPtrType Calc, 
                              int SetjmpReturnValue, 
                              char *FunctionName)
{
    switch (SetjmpReturnValue) {
        case 0:
            return TRUE;
        default: {
            char *Msg;
            int MsgCode;
            
            if (SetjmpReturnValue < 0)
                MsgCode = 0;
            else 
                MsgCode = SetjmpReturnValue;

            Msg = IritMalloc(strlen(ERROR_MESSAGES[MsgCode]) + 1 + 
                             IRIT_LINE_LEN);

            sprintf(Msg, "%s. Entry point %%s.\n", ERROR_MESSAGES[MsgCode]);
            IRIT_WARNING_MSG_PRINTF(Msg, FunctionName);
            IritFree(Msg);
            MiscISCFreeAllocatedMemoryOnError(Calc);
            return FALSE;
        }
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Allocate memory. longjmp to Calc -> MiscISCJmpTarget if fails.            *
*   On the way, keep the pointer in global structure in order to be freed in  *
* case of an error.                                                           *
*   If Calc is NULL, the memory isn't added to the list of allocated memory.  *
* Also, failure doesn't cause longjmp but return of NULL (because the longjmp *
* is expected to be inside Calc which is NULL and therefore inaccesible).     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator. If it's NULL don't add it to the list of allocated  *
*         memory and error return NULL instead of longjmp.                    *
*   Size: The size of memory to allocate.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   void*: The allocated memory.                                              *
******************************************************************************/
static void *MiscISCMalloc(MiscISCCalculatorPtrType Calc, unsigned int Size)
{
    void
        *Res = NULL;
    MiscISCFreeMemoryStruct **AllocateNext;

    Res = IritMalloc(Size);
    if (Calc == NULL) 
        return Res;
    MiscISCLongJmpCond(Calc, Res != NULL, MISC_ISC_ERROR_ALLOCATING_MEMORY);   

    if (Calc -> MemoryToFree == NULL) {
        AllocateNext = &Calc -> MemoryToFree;
    }
    else {
        for (AllocateNext = &Calc -> MemoryToFree; (*AllocateNext)!= NULL; 
            AllocateNext = &(*AllocateNext) -> PNext);
    }
    (*AllocateNext) = IritMalloc(sizeof(MiscISCFreeMemoryStruct));
    MiscISCLongJmpCond(Calc, (*AllocateNext) != NULL, 
        MISC_ISC_ERROR_ALLOCATING_MEMORY);
    (*AllocateNext) -> PNext = NULL;
    (*AllocateNext) -> MemoryToFree = Res;
    return Res;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Allocate memory and then setmem it to Val. longjmp to                    *
* MiscISCJmpTarget if fails.                                                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*   Size: The size of memory to allocate.                                     *
*   Val:  The value to set all bytes.                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*   void*: The allocated memory.                                              *
******************************************************************************/
static void *MiscISCMallocAndSet(MiscISCCalculatorPtrType Calc, 
                                 unsigned int Size, 
                                 int Val)
{
    void
        *Res = MiscISCMalloc(Calc, Size);

    memset(Res, Val, Size);
    return Res;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Free memory.                                                              *
*   On the way, remove the pointer from global structure.                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*   P: The memory to free. NULL will be ignored.                              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCFree(MiscISCCalculatorPtrType Calc, void *p) {

  MiscISCFreeMemoryStruct *Temp,
        **MemoryToFree = &Calc -> MemoryToFree;

    if (p == NULL)
        return;
    for (MemoryToFree = &Calc -> MemoryToFree; 
        (*MemoryToFree != NULL) && ((*MemoryToFree) -> MemoryToFree != p) ; 
         MemoryToFree = &((*MemoryToFree) -> PNext)); 
    MiscISCLongJmpCond(Calc, *MemoryToFree != NULL,
        MISC_ISC_MEMORY_WAS_NOT_ALLOCATED_WITH_MISC_ISC_MALLOC);
    Temp = *MemoryToFree;
    *MemoryToFree = (*MemoryToFree) -> PNext;
    IritFree(Temp -> MemoryToFree);
    IritFree(Temp);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Hashes an array.                                                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   Arr:           The array.                                                 *
*   SizeInBytes: The size in bytes of the array.                              *
*   KeySizeBits:   The key's size in bits.                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*    unsigned long: The hash value.                                           *
*                                                                             *
* SEE ALSO:                                                                   *
*                                                                             *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCArrayHashCallback                                                 *
******************************************************************************/
static unsigned long MiscISCArrayHashCallback(void *Arr, 
                                              MiscISCImageSizeType SizeInBytes, 
                                              unsigned int KeySizeBits)
{
    char 
        *CharArray = (char *) Arr;
    unsigned long i, j, Step, 
        Ret = 0;

    if (SizeInBytes <= KeySizeBits) {
        for (i = 0; i < SizeInBytes; ++i) {
            Ret <<= 1;
            Ret |= (CharArray[i] & 0x01);
        }

        return Ret;
    }

    Step = SizeInBytes / KeySizeBits;
    for (i = 0, j = 0; i < KeySizeBits; ++i, j += Step) {
        Ret <<= 1;
        Ret |= (CharArray[j] & 0x01);
    }

    return Ret;
}


/******************************************************************************
* DESCRIPTION:                                                                *
*   Creates a copy of the given array. Used for hash method.                  *
*   Uses IritMalloc and not MiscISCMalloc so it isn't protected.              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Arr:         The array to copy.                                           *
*   SizeInBytes: The size in bytes of the array.                              *
*                                                                             *
* RETURN VALUE:                                                               *
*   void *: The copy of the array.                                            *
******************************************************************************/
static void *MiscISCHashArrayCopyCallback(void *Arr, 
                                          MiscISCImageSizeType SizeInBytes)
{
    void 
        *Ret = IritMalloc(SizeInBytes);

    memcpy(Ret, Arr, SizeInBytes);
    return Ret;
} 

/******************************************************************************
* DESCRIPTION:                                                                *
*   A free call back of the hash function.                                    *
*   Use IritFree and not MiscISCFree so it isn't protected.                   *
*                                                                             *
* PARAMETERS:                                                                 *
*   Arr: Pointer to hash table element to free.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCHashFreeCallback(void *Arr)
{
    IritFree(Arr);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Compares 2 Arrays. Used in the hash table.                                *
*                                                                             *
* PARAMETERS:                                                                 *
*   Arr1:        An array to compare.                                         *
*   Arr2:        An array to compare.                                         *
*   SizeInBytes: The size in bytes of the array.                              *
*                                                                             *
* RETURN VALUE:                                                               *
*    int:         0 if equal.                                                 *
******************************************************************************/
static int MiscISCArrayCompCallBack(void *Arr1, 
                                    void *Arr2, 
                                    MiscISCImageSizeType SizeInBytes)
{
    MiscISCUnitType 
        *UA1 = (MiscISCUnitType *) Arr1, 
        *UA2 = (MiscISCUnitType *) Arr2;
    MiscISCImageSizeType 
        i, ArrSizeUnits;
    unsigned int j, TailSize;
    char *Tail1, *Tail2;

    ArrSizeUnits = SizeInBytes / sizeof(MiscISCUnitType);
    TailSize = SizeInBytes % sizeof(MiscISCUnitType);

    for (i = 0; i < ArrSizeUnits; ++i) {
        if (UA1[i] != UA2[i]) {
            return 1;
        }
    }
    Tail1 = (char *) (UA1 + ArrSizeUnits);
    Tail2 = (char *) (UA2 + ArrSizeUnits);

    for (j = 0; j < TailSize; ++j) {
        if (Tail1[j] != Tail2[j])
            return 1;
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Returns the Log2 of the number of bits in MiscISCUnitType.               *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: Log2 of MiscISCUnitType size in bits.                               *
******************************************************************************/
static int MiscISCGetZipFactor(MiscISCCalculatorPtrType Calc)
{
    switch (sizeof(MiscISCUnitType)) {
        case 1:
            return 3;
        case 2:
            return 4;
        case 4:
            return 5;
        case 8:
            return 6;
        default:
            MiscISCLongJmpCond(Calc, FALSE, 
                MISC_ISC_UNIT_TYPE_BIGGER_THAN_EXPECTED);
            /* The exception above will prevent the execution of this line.  */
            /* It's here just for the compiler's warning.                    */
            return FALSE;
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Returns the number of MiscISCUnitType needed to store a picture of the   *
* given size when each bit of a MiscISCUnitType stores a pixel (used in B&W   *
* calculators).                                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*   Size: The number of pixels in the picture.                                *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCImageSizeType: The number of MiscISCUnitType needed to store a     *
*   picture of the given size when Each bit of a MiscISCUnitType stores a     *
*   pixel.                                                                    *
******************************************************************************/
static MiscISCImageSizeType MiscISCGetZipSize(MiscISCCalculatorPtrType Calc,
                                              MiscISCImageSizeType Size)
{
    int ZipFactor;
    MiscISCImageSizeType ZipSize;

    ZipFactor = MiscISCGetZipFactor(Calc);
    ZipSize = Size >> ZipFactor;
    if ((ZipSize << ZipFactor) < Size) {
        ZipSize++;
    }

    return ZipSize;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Returns a BitMask for a MiscISCUnitType where only the high bit is set.  *
*                                                                             *
* PARAMETERS:                                                                 *
*   None                                                                      *
*                                                                             *
* RETURN VALUE:                                                               *
*    MiscISCUnitType: A mask with only the high bit set.                      *
******************************************************************************/
static MiscISCUnitType MiscISCGetHighBitMask()
{
    return ((MiscISCUnitType) 1) << ((sizeof(MiscISCUnitType) << 3) - 1);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Checks if the given array is zero.                                       *
*                                                                             *
* PARAMETERS:                                                                 *
*    Arr:         The array.                                                  *
*    SizeInBytes: The size of the array in bytes.                             *
*                                                                             *
* RETURN VALUE:                                                               *
*    int:         TRUE if zero, FALSE otherwize.                              *
******************************************************************************/
static int MiscISCIsZeroArray(void *Arr, MiscISCImageSizeType SizeInBytes)
{
    MiscISCImageSizeType i, ArrSizeUnits;
    MiscISCUnitType 
        *UnitArr = (MiscISCUnitType*)Arr;
    char *Tail;
    unsigned int j, TailSize;

    ArrSizeUnits = SizeInBytes / sizeof(MiscISCUnitType);
    TailSize = SizeInBytes % sizeof(MiscISCUnitType);

    for (i = 0; i < ArrSizeUnits; ++i) {
        if (UnitArr[i] != 0)
            return FALSE;
    }

    Tail = (char *) (UnitArr + ArrSizeUnits);

    for (j = 0; j < TailSize; ++j) {
        if (Tail[j] != 0)
            return FALSE;
    }

    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Copy InputArr to OutputArr. If OutputArr is NULL allocate new memory and  *
* copy InputArr to that new allocated memory.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator.                                                     *
*   OutputArr:   The location to copy the array to. If NULL, allocate new     *
*                memory for the new array.                                    *
*   InputArr:    The array to copy.                                           *
*   SizeInBytes: The array's size.                                            *
*                                                                             *
* RETURN VALUE:                                                               *
*   void *:      The copy of the array.                                       *
******************************************************************************/
static void *MiscISCArrayCopy(MiscISCCalculatorPtrType Calc, 
                              void *OutputArr,
                              void *InputArr, 
                              MiscISCImageSizeType SizeInBytes)
{
    if (OutputArr == NULL)
        OutputArr = MiscISCMalloc(Calc, SizeInBytes);;
    memcpy(OutputArr, InputArr, SizeInBytes);

    return OutputArr;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Return the number of uncovered pixels in ToCover wich can be covered by   *
*   Picture. The greedy algorithm will choose the next picture to be the      *
*   picture which returns the highest value by this function.                 *
*   Used only for B&W pictures.                                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:        The calculator.                                              *
*   Picture:     The picture when each bit represents a pixel.                *
*   ToCover:     The pixels still requiring cover. Each bit represents a      *
*                pixel.                                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCImageSizeType: The amount of uncovered pixel which can be covered  *
*                         by the given picture.                               *
******************************************************************************/
static MiscISCImageSizeType MiscISCBnWGreedyPicValue(
                                           MiscISCCalculatorPtrType Calc,
                                           void *Picture,
                                           void *ToCover)
{
    MiscISCImageSizeType i, Cover;

    Cover = 0;   
    for (i = 0; i <= Calc -> ImageSize - 1; i++) {
        if ((Calc -> NeedToCover(Picture, i) == 1) &&
            (Calc -> NeedToCover(ToCover, i) == 1)) {
            Cover += Calc -> PixelsAmount[i];
        }
    }
    return Cover;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Calculate the number of "cover unit" in ToCover which can be covered by   *
*   Picture. E.g: Pixel X in ToCover has value 8 and Pixel X in Picture has   *
*   value 12. Therefore, Picture has 8 "cover unit" for this pixel. This      *
*   function sum the "cover unit" for all the pixels of ToCover.              *
*   The greedy algorithm will choose the next picture to be the picture which *
*   returns the highest value by this function.                               *
*   Used only for gray pictures.                                              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:        The calculator.                                              *
*   Picture:     The picture when each MiscISCPixelType represents a pixel.   *
*   ToCover:     The pixels still requiring cover. Each MiscISCPixelType      *
*                represents a pixel.                                          *   
*   SizeInBytes: The amount of bytes used by the picture (when each bit       *
*                MiscISCPixelType represents one pixel).                      *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCImageSizeType: The amount of uncovered pixel which can be covered  *
*                         by the given picture.                               *
******************************************************************************/
static MiscISCImageSizeType MiscISEGrayGreedyPictureValue(
                                            MiscISCCalculatorPtrType Calc,
                                            void *Picture,
                                            void *ToCover)
{
    MiscISCImageSizeType i, Size, 
        Ret = 0;
    MiscISCPixelType 
        *PictureInner = (MiscISCPixelType *) Picture,
        *ToCoverInner = (MiscISCPixelType *) ToCover;

    Size = Calc -> ImageSizeInBytes / sizeof(MiscISCPixelType);

    for (i = 0; i < Size; ++i) 
        Ret += ((PictureInner[i]) > (ToCoverInner[i]) ? (ToCoverInner[i])
            : (PictureInner[i]));
    return Ret;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates one pixel in the total image left to cover.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Each bit represents a pixel.   *
*   Pixel:   The index of the pixel to update.                                *
*   Covered: TRUE - set the pixel as covered. FALSE - set the pixel as not    *
*            covered.                                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCBnWUpdatePixelInToCoverFuncType(void *ToCover,
                                                   MiscISCImageSizeType Pixel,
                                                   int Covered)
{
    MiscISCImageSizeType Cell, Offset;
    char 
        *Arr = (char *) ToCover;

    Cell  = Pixel / (sizeof(char) << 3);
    Offset  = Pixel % (sizeof(char) << 3);
    
    if (Covered)
        Arr[Cell] |= (1 << Offset);
    else
        Arr[Cell] &= (~(1 << Offset));
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates one pixel in the total image left to cover.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Each MiscISCPixelType          *
*            represents a pixel.                                              *   
*   Pixel:   The index of the pixel to update.                                *
*   Covered: TRUE - set the pixel as covered. FALSE - set the pixel as not    *
*            covered.                                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCGrayWUpdatePixelInToCoverFuncType(void *ToCover,
                                                     MiscISCImageSizeType Pixel,
                                                     int Covered)
{
    MiscISCPixelType 
        *Arr = (MiscISCPixelType* ) ToCover;
    
    Arr[Pixel] = !!Covered;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates the part of the total image left to cover after using the         *
* given picture. For B&W pictures.                                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   NewPicture:  The covering picture when each bit represents a pixel.       *
*   ToCover:     The pixels still requiring cover. Each bit represents a      *
*                pixel.                                                       *
*   SizeInBytes: The amount of bytes used by the picture (when each bit       *
*                represents one pixel).                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCBnWUpdateCover(void *NewPicture,
                                  void *ToCover,
                                  MiscISCImageSizeType SizeInBytes)
{
    MiscISCImageSizeType i, Size;
    MiscISCUnitType 
        *PicInner = (MiscISCUnitType *) NewPicture,
        *CoverInner = (MiscISCUnitType *) ToCover;

    Size = SizeInBytes / sizeof(MiscISCUnitType);

    for (i = 0; i < Size; ++i) 
        CoverInner[i] &= ~(PicInner[i]);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Updates the part of the total image left to cover after using the         *
*   given picture. For gray pictures.                                         *
*                                                                             *
*   NewPicture:  The covering picture when each MiscISCPixelType represents   *
*                a pixel.                                                     *
*   ToCover:     The pixels still requiring cover. Each MiscISCPixelType      *
*                represents a pixel.                                          *   
*   SizeInBytes: The amount of bytes used by the picture (when each           *
*                MiscISCPixelType represents one pixel).                      *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCGrayUpdateCover(void *NewPicture,
                                   void *ToCover,
                                   MiscISCImageSizeType SizeInBytes)
{
    MiscISCImageSizeType i, Size;
    MiscISCPixelType 
        *PicInner = (MiscISCPixelType*)NewPicture,
        *CoverInner = (MiscISCPixelType*)ToCover;

    Size = SizeInBytes / sizeof(MiscISCPixelType);

    for (i = 0; i < Size; ++i) {
        CoverInner[i] = 
            CoverInner[i] < PicInner[i] ? 0 : CoverInner[i] - PicInner[i];
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Checks if the given pixel need to be covered in the image left to cover.  *
* Return how much is still needed to be covered (which is 1 if cover needed or*
* 0 if cover isn't needed).                                                   *
*   Used in B&W pictures.                                                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Each bit represents a pixel.   *   
*   Pixel:   The pixel's index.                                               *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: How much is still needed to be covered.                             *
******************************************************************************/
static int MiscISCBnWNeedToCover(void *ToCover, MiscISCImageSizeType Pixel)
{
    MiscISCImageSizeType Cell, Offset;
    char 
        *Arr = (char *) ToCover;

    Cell  = Pixel / (sizeof(char) << 3);
    Offset  = Pixel % (sizeof(char) << 3);
    
    return (Arr[Cell] >> Offset) & 0x01;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Checks if the given pixel need to be covered in the image left to cover. *
* Returns how much cover is still needed.                                     *
* Used in gray pictures.                                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   ToCover: The pixels still requiring cover. Each MiscISCPixelType          *
*            represents a pixel.                                              *   
*   Pixel:   The pixel's index.                                               *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: How much is still needed to cover.                                  *
******************************************************************************/
static int MiscISCGrayNeedToCover(void *ToCover, MiscISCImageSizeType Pixel) 
{
    MiscISCPixelType 
        *Arr = (MiscISCPixelType* ) ToCover;
    
    return Arr[Pixel];
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Creates a new calculator.                                                 M
*                                                                             *
* PARAMETERS:                                                                 M
*   MaxPictures: The maximal number of pictures the calculator will hold.     M
*   ImageSize:   The number of pixels in each picture.                        M
*   ColorType:   Are the pictures in black & white or gray.                   M
*                                                                             *
* RETURN VALUE:                                                               M
*   MiscISCCalculatorPtrType:   The new calculator or NULL if fails.          M
*                                                                             *
* SEE ALSO:                                                                   M
*                                                                             *
* KEYWORDS:                                                                   M
*    MiscISCNewCalculator                                                     M
******************************************************************************/
MiscISCCalculatorPtrType MiscISCNewCalculator(int MaxPictures,
                                              MiscISCImageSizeType ImageSize,
                                              MiscISCColorTypeEnum ColorType)
{
    MiscISCCalculatorPtrType 
        Calc = NULL;
    jmp_buf TempJmpTarget;
    int SetjmpReturn;

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(TempJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, "MiscISCNewCalculator")) {
        IritFree(Calc);
        return NULL;
    }

    /* Since MiscISCMalloc recieve NULL it doesn't longjmp on error, instead */
    /* it returns NULL.                                                      */
    Calc = (MiscISCCalculatorPtrType) 
        MiscISCMalloc(NULL, sizeof(struct MiscISCCalculatorStruct));
    if (Calc == NULL)
        longjmp(TempJmpTarget, MISC_ISC_ERROR_CREATING_CALCULATOR);
    IRIT_GEN_COPY(Calc -> MiscISCJmpTarget, TempJmpTarget, sizeof(jmp_buf));

    if (ColorType == MISC_ISC_BNW) {
        Calc -> GreedyGetPictureValue = MiscISCBnWGreedyPicValue;
        Calc -> UpdatePixelInToCover = MiscISCBnWUpdatePixelInToCoverFuncType;
        Calc -> UpdateToCover = MiscISCBnWUpdateCover;
        Calc -> NeedToCover = MiscISCBnWNeedToCover;
    } 
    else {
        assert(ColorType == MISC_ISC_GRAY);
        Calc -> GreedyGetPictureValue = MiscISEGrayGreedyPictureValue;
        Calc -> UpdatePixelInToCover = MiscISCGrayWUpdatePixelInToCoverFuncType;
        Calc -> UpdateToCover = MiscISCGrayUpdateCover;
        Calc -> NeedToCover = MiscISCGrayNeedToCover;
    }

    /* Must set to NULL before any memory allocation. */
    Calc -> MemoryToFree = NULL; 
    
    /* Calc -> jmp_buf was already given initial value above. */
    Calc -> MaxPictures = MaxPictures;
    Calc -> NumPictures = 0;
    Calc -> ComputePhase = FALSE;
    Calc -> ColorType = ColorType;
    Calc -> UnprocessedImageSize = ImageSize;
    Calc -> ImageSize = -1;
    Calc -> ImageSizeInBytes = -1;
    Calc -> UnprocessedPictures = (MiscISCPixelType **)
        MiscISCMalloc(Calc, sizeof(void *) * MaxPictures);
    Calc -> Pictures = NULL;

    Calc -> PixelsAmount = NULL;
    Calc -> NumOfPixeslToCover = -1;
    Calc -> UnprocessedRequiredCover = NULL;
    Calc -> RequiredCover = NULL;

    Calc -> UseSizeLimit = FALSE;
    Calc -> SizeLimit = -1;
    Calc -> UseCoverLimit = FALSE;
    Calc -> CoverLimit = -1;
    Calc -> CoverLimitInPixels = -1;

    Calc -> SolutionSize = -1;
    Calc -> SolutionCover = -1;
    Calc -> FoundSolution = FALSE;
    Calc -> FoundFullSolution = FALSE;
    Calc -> SolutionByIndex = NULL; 
    Calc -> Solution = NULL; 

    Calc -> MinFinder = NULL;
    Calc -> VisibilitySetsSizes = NULL;
    Calc -> SelectionRanks = NULL;
    Calc -> TotalSelectionRanks = NULL;
    Calc -> UnusedPictures = NULL;

    return Calc;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Called in case of an error to free all the current allocated memory.      *
* Do nothing if Calc is NULL or if the memory was already freed.              *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc: The calculator. If NULL, assume Calc wasn't created yet and return  *
*         without doing anything.                                             *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
*                                                                             *
* SEE ALSO:                                                                   *
*                                                                             *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCFreeAllocatedMemoryOnError                                        *
******************************************************************************/
static void MiscISCFreeAllocatedMemoryOnError(MiscISCCalculatorPtrType Calc)
{
    MiscISCFreeMemoryStruct *MemoryToFree;

    if ((Calc == NULL) || (Calc -> MemoryToFree == NULL))
        return;

    for (MemoryToFree = Calc -> MemoryToFree; MemoryToFree != NULL;) {
        MiscISCFreeMemoryStruct *Temp;

        IritFree(MemoryToFree -> MemoryToFree);
        Temp = MemoryToFree;
        MemoryToFree = MemoryToFree -> PNext;
        IritFree(Temp);
    }
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Frees memory allocated for the calculator.                                M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc: The calculator.                                                     M
*                                                                             *
* RETURN VALUE:                                                               M
*   void                                                                      M
*                                                                             *
* SEE ALSO:                                                                   M
*                                                                             M
*                                                                             *
* KEYWORDS:                                                                   M
*   MiscISCFreeCalculator                                                     M
******************************************************************************/
void MiscISCFreeCalculator(MiscISCCalculatorPtrType Calc)
{
    /* All memory was freed because of an error. No need to free it again. */
    if (Calc -> MemoryToFree != NULL) {
        int i;
        
        MiscISCFree(Calc, Calc -> UnprocessedRequiredCover);
        MiscISCFree(Calc, Calc -> RequiredCover);
        if (Calc -> UnprocessedPictures) {
            for (i = 0; i < Calc -> NumPictures; ++i) {
                MiscISCFree(Calc, Calc -> UnprocessedPictures[i]);
            }
            MiscISCFree(Calc, Calc -> UnprocessedPictures);
        }
        if ((Calc -> Pictures != NULL) &&
            (Calc -> Pictures != (void**) Calc -> UnprocessedPictures)) {
            for (i = 0; i < Calc -> NumPictures; ++i) {
                MiscISCFree(Calc, Calc -> Pictures[i]);
            }
            MiscISCFree(Calc, Calc -> Pictures);
        }
        MiscISCFree(Calc, Calc -> PixelsAmount);
        MiscISCFree(Calc, Calc -> Solution);
        /* We don't free Calc -> SolutionByIndex because it is output for the*/
        /* user and should be freed by the user.                             */
    }
    IritFree(Calc);
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Set the image to be covered. A pixel with value 0 means that the pixel    M
* isn't required to be covered. A pixel with value 1 means that the pixel is  M
* required to be covered. If this function isn't called than all the pixel areM
* required to be covered.                                                     M
*   The function makes an inner copy of RequiredCover (that copy will be freedM
* when the calculator is freed). The user may free or change RequiredCover's  M
* memory without harming the process.                                         M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc:         The calculator to add the picture to.                       M
*   RequiredCover: The cover picture to add.                                  M
*                                                                             *
* RETURN VALUE:                                                               M
*   int: FALSE if error occured.                                              M
*                                                                             *
* SEE ALSO:                                                                   M
*                                                                             *
* KEYWORDS:                                                                   M
*    MiscISCSetImageToCover                                                   M
******************************************************************************/
int MiscISCSetImageToCover(MiscISCCalculatorPtrType Calc, 
                           MiscISCPixelType *RequiredCover)
{
    int SetjmpReturn;

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(Calc -> MiscISCJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, "MiscISCSetImageToCover")) 
        return FALSE;

    MiscISCLongJmpCond(Calc, (Calc != NULL) && (Calc -> RequiredCover == NULL), 
        MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SET_IMAGE_TO_COVER);

    Calc -> UnprocessedRequiredCover = MiscISCArrayCopy(Calc, NULL, 
        RequiredCover, Calc -> UnprocessedImageSize * sizeof(MiscISCPixelType));

    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Adds a picture to the calculator. Copies the picture (will be freed       M
* when the calculator is freed).                                              M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc:    The calculator to add the picture to.                            M
*   Picture: The picture to add.                                              M
*                                                                             *
* RETURN VALUE:                                                               M
*   int: FALSE if error occured.                                              M
*                                                                             *
* SEE ALSO:                                                                   M
*                                                                             M
*                                                                             *
* KEYWORDS:                                                                   M
*   MiscISCAddPicture                                                         M
******************************************************************************/
int MiscISCAddPicture(MiscISCCalculatorPtrType Calc, MiscISCPixelType *Picture)
{
    MiscISCImageSizeType SizeBytes, i;
    MiscISCPixelType *InnerPicture;
    int SetjmpReturn;

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(Calc -> MiscISCJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, "MiscISCAddPicture")) 
        return FALSE;

    MiscISCLongJmpCond(Calc, (Calc -> NumPictures != Calc -> MaxPictures) &&
        (!Calc -> ComputePhase), MISC_ISC_TO_MANY_PICTURES_ADDED);

    SizeBytes = Calc -> UnprocessedImageSize * sizeof(MiscISCPixelType);
    InnerPicture = MiscISCArrayCopy(Calc, NULL, Picture, SizeBytes);
    if (Calc -> ColorType == MISC_ISC_BNW)
        for (i=0; i < SizeBytes; i++) {
            MiscISCLongJmpCond(Calc, (InnerPicture[i] == 0) || 
                (InnerPicture[i] == 1), MISC_ISC_PICTURE_NOT_BNW);
        }

    Calc -> UnprocessedPictures[Calc -> NumPictures] = InnerPicture;
    Calc -> NumPictures++;

    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used only in B&W calculators. Part of moving to computing phase in        *
* MiscISCMoveCalcToComputePhase.                                              *
*   Do section 2 described in MiscISCMoveCalcToComputePhase.                  *
*   For section 1, find all the relevat pixel.                                *
*   Uses the given hash table in order to recognise unique pixels.            *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:            IN OUT, The Calculator.                                 *
*    PixelHash:       IN, The hash table for the pixel. The elemnts of the    *
*                     table are arrays the size of number of pictures. Each   *
*                     element represents for one pixel which pictures cover   *
*                     it and which not.                                       *
*    NeededPixels:    OUT, array in which each cell corresponds to a pixel in *
*                     the image. Value of 0 means the pixel won't be used in  *
*                     the final image. Any other value means the pixel will   *
*                     be used in the final pictures. That value also tell how *
*                     many identical pixels (all those pixel were covered by  *
*                     the pictures in the same way) were in the unprocessed   *
*                     pictures.                                               *
*    TotalUsedPixels: OUT, parameter. The number of used pixels in            *
*                     NeededPixels.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCHashUniquePixelsArray(MiscISCCalculatorPtrType Calc,
                                         MiscHashPtrType PixelHash,
                                         MiscISCImageSizeType *NeededPixels,
                                         MiscISCImageSizeType *TotalUsedPixels)
{
    MiscISCImageSizeType i, PixelSizeInBytes,
        BadPixels = 0,
        IgnoredPixels = 0;
    MiscISCPixelType *CurrPixel;
    int j, Curr;
    long *PixelIndex;

    MiscISCLongJmpCond(Calc, (NeededPixels != NULL) &&
        (TotalUsedPixels != NULL), 
        MISC_ISC_NULL_IN_MISC_ISC_HASH_UNIQUE_PIXEL_ARRAY);

    CurrPixel = MiscISCMallocAndSet(Calc, 
        sizeof(MiscISCPixelType)*Calc -> NumPictures, 0);
    *TotalUsedPixels = 0;    
    PixelSizeInBytes = (MiscISCImageSizeType) (Calc -> NumPictures  *
                                                    sizeof(MiscISCPixelType));
    for (i = 0; i < Calc -> UnprocessedImageSize; ++i) {
        /* If UnprocessedRequiredCover exists, 0 marks pixel which isn't     */
        /* required to be coverd and 1 marks pixel which is required to be   */
        /* covered.                                                          */
        /* If UnprocessedRequiredCover doesn't exist, all pixels are required*/
        /* to be covered.                                                    */
        if (Calc -> UnprocessedRequiredCover != NULL && 
            Calc -> UnprocessedRequiredCover[i] == 0) {
            IgnoredPixels++;
            continue;
        }
        for (j = 0; j < Calc -> NumPictures; ++j) {
            CurrPixel[j] = (Calc -> UnprocessedPictures)[j][i];
        }
        if (MiscISCIsZeroArray(CurrPixel, PixelSizeInBytes)) {
            /* Zero pixel cannot be covered. */
            ++BadPixels;
            continue;
        }
        PixelIndex = MiscHashGetElementAuxData(PixelHash, CurrPixel, 
            PixelSizeInBytes);
        if (PixelIndex != NULL) {
            /* Duplicate pixel. */
            NeededPixels[*PixelIndex]++;
            continue;
        }
        MiscISCLongJmpCond(Calc, 
            MiscHashAddElement(PixelHash, CurrPixel, PixelSizeInBytes) != -1, 
            MISC_ISC_ERROR_ADDING_ELEMENT_TO_HASHTABLE);
        PixelIndex = MiscHashGetElementAuxData(PixelHash, CurrPixel, 
            PixelSizeInBytes);
        /* The auxilary data of the element in the hash table is its index in*/
        /* the NeededPixels array.                                           */
        (*PixelIndex) = i; 
        NeededPixels[i] = 1;
        (*TotalUsedPixels)++;
    }
    Calc -> NumOfPixeslToCover = Calc -> UnprocessedImageSize - IgnoredPixels;
    Calc -> SolutionCover = Calc -> NumOfPixeslToCover - BadPixels;

    Calc -> PixelsAmount = (MiscISCImageSizeType*)MiscISCMalloc(Calc, 
        *TotalUsedPixels*sizeof(MiscISCImageSizeType));
    Curr = 0;
    for (i=0; i<= Calc -> UnprocessedImageSize - 1; i++) {
        if (NeededPixels[i] == 0)
            continue;
        Calc -> PixelsAmount[Curr] = NeededPixels[i];
        Curr++;
    }
    MiscISCFree(Calc, CurrPixel);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used only in B&W calculators. Part of moving to computing phase in        *
* MiscISCMoveCalcToComputePhase.                                              *
*   For section 1 of MiscISCMoveCalcToComputePhase, prepare the pictures for  *
* the calculatioin by taking from Calc -> UnprocessedPictures only the pixels *
* given by NeededPixels and set Calc -> Pictures to hold the new pictures.    *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:            The Calculator.                                         *
*    TotalUsedPixels: IN, see MiscISCHashUniquePixelsArray.                   *
*    NeededPixels:    IN, see MiscISCHashUniquePixelsArray.                   *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCShrinkPictures(MiscISCCalculatorPtrType Calc,
                                  MiscISCImageSizeType *NeededPixels,
                                  MiscISCImageSizeType TotalUsedPixels)
{
    int i;

    MiscISCLongJmpCond(Calc, (NeededPixels != NULL),
        MISC_ISC_NULL_IN_MISC_ISC_SHRINK_PICTURES);

    Calc -> Pictures = MiscISCMallocAndSet(Calc, 
        sizeof(void*)* Calc -> NumPictures, 0);

    for (i = 0; i < Calc -> NumPictures; ++i) {
        MiscISCImageSizeType j;
        int loc = 0;
        MiscISCPixelType 
            **Picture = (MiscISCPixelType**)&(Calc -> Pictures[i]);

        *Picture = 
            MiscISCMalloc(Calc, sizeof(MiscISCPixelType) * TotalUsedPixels);

        for (j = 0; j < Calc -> UnprocessedImageSize; ++j) {
            if (NeededPixels[j] == 0)
                continue;
            (*Picture)[loc] = Calc -> UnprocessedPictures[i][j];
            ++loc;
        }
        MiscISCFree(Calc, Calc -> UnprocessedPictures[i]);
    }
    Calc -> ImageSize = TotalUsedPixels;
    MiscISCFree(Calc, Calc -> UnprocessedPictures);
    Calc -> UnprocessedPictures = NULL;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used only in B&W calculators. Part of moving to computing phase in        *
* MiscISCMoveCalcToComputePhase.                                              *
*   Responsbile for the B&W part of the explenations in                       *
* MiscISCMoveCalcToComputePhase.                                              *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc: The Calculator.                                                    *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCShrinkCalculator(MiscISCCalculatorPtrType Calc)
{
    MiscISCImageSizeType 
        *NeededPixels = NULL;
    MiscISCImageSizeType 
        TotalUsedPixels = 0;
    MiscHashPtrType
        PixelHash = NULL;

    MiscISCLongJmpCond(Calc, Calc != NULL, 
        MISC_ISC_NULL_IN_MISC_ISC_SHRINK_CALCULATOR);

    NeededPixels = MiscISCMallocAndSet(Calc, 
        sizeof(MiscISCImageSizeType) * Calc -> UnprocessedImageSize, 0);   

    PixelHash = MiscHashNewHash(Calc -> UnprocessedImageSize  *
                                    MISC_ISC_PIXAL_HASH_TO_PIXEL_SIZE_RATIO,
                                MiscISCArrayHashCallback,
                                MiscISCHashArrayCopyCallback,
                                MiscISCHashFreeCallback,
                                MiscISCArrayCompCallBack);
    MiscISCLongJmpCond(Calc, PixelHash != NULL, MISC_ISC_ERROR_CREATING_HASH_TABLE);
    MiscISCHashUniquePixelsArray(Calc, PixelHash, NeededPixels,
                                 &TotalUsedPixels);
    MiscISCShrinkPictures(Calc, NeededPixels, TotalUsedPixels);

    MiscHashFreeHash(PixelHash);
    MiscISCFree(Calc, NeededPixels);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Used in B&W calculators. Groups the pixels of one pictures into          *
* MiscISCUnitType (one bit per pixel).                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The Calculator.                                          *
*   Picture:          The picture.                                            *
*   Size:             The picture's size.                                     *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCUnitType*: The zipped picture.                                     *
******************************************************************************/
static MiscISCUnitType *MiscISCZipPicture(MiscISCCalculatorPtrType Calc,
                                          MiscISCPixelType *Picture,
                                          MiscISCImageSizeType Size)
{
    int ZipFactor, Grouping, j;
    MiscISCImageSizeType ZipSize, i, Loc;
    MiscISCUnitType HighBitMask, 
        *ZipPicture = NULL;
    MiscISCPixelType 
        *LocalPicture = (MiscISCPixelType*) Picture;

    ZipFactor = MiscISCGetZipFactor(Calc);
    ZipSize = MiscISCGetZipSize(Calc, Size);
    HighBitMask = MiscISCGetHighBitMask();
    Grouping = 1U << ZipFactor;
    ZipPicture = (MiscISCUnitType*) 
        MiscISCMalloc(Calc, sizeof(MiscISCUnitType) * ZipSize);

    Loc = 0;
    for (i = 0; i < ZipSize; i++) {
        ZipPicture[i] = 0;
        for (j = 0; j < Grouping && Loc < Size; j++, Loc++) {
            ZipPicture[i] >>= 1;
            if (LocalPicture[Loc] == 1) {
                ZipPicture[i] |= HighBitMask;
            }
        }
        for (; j < Grouping; j++) {
            ZipPicture[i] >>= 1;
            ZipPicture[i] |= HighBitMask;
        }
    }
    return ZipPicture;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*    Used in B&W calculators. Groups the pixels in all the pictures into      *
* MiscISCUnitType (one bit per pixel).                                        *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc: The Calculator.                                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCZipCalculator(MiscISCCalculatorPtrType Calc)
{
    int i;
    MiscISCImageSizeType ZipSize;

    for (i = 0; i < Calc -> NumPictures; i++) {
        MiscISCUnitType
	    *ZippedPicture = 
	        MiscISCZipPicture(Calc, Calc -> Pictures[i],
				  Calc -> ImageSize);

        MiscISCFree(Calc, Calc -> Pictures[i]);
        Calc -> Pictures[i] = ZippedPicture;
    }
    ZipSize = MiscISCGetZipSize(Calc, Calc -> ImageSize);
    Calc -> RequiredCover = (MiscISCPixelType*) MiscISCMallocAndSet(Calc, 
                                     sizeof(MiscISCUnitType) * ZipSize, 0xFF);
    Calc -> ImageSizeInBytes = ZipSize * sizeof(MiscISCUnitType);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used only in gray calculators. Part of moving to computing phase in       *
* MiscISCMoveCalcToComputePhase.                                              *
*   Responsible for all the operations specified in                           *
* MiscISCMoveCalcToComputePhase under gray calculators.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc: The Calculator.                                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscISCUpdateGrayCoverability(MiscISCCalculatorPtrType Calc)
{
    MiscISCImageSizeType i, 
        CoverableSum = 0, 
        TotalSum = 0;
    char
        *CanCover = NULL;
    int j,
        *PixelsSum = NULL;

    MiscISCLongJmpCond(Calc, (Calc != NULL) &&
        (Calc -> ColorType == MISC_ISC_GRAY) && 
        (Calc -> UnprocessedRequiredCover != NULL),
        MISC_ISC_CALC_IS_NOT_READY_IN_MISC_ISC_UPDATE_GRAY_COVERABILITY);
    
    CanCover = (char *) MiscISCMallocAndSet(Calc, sizeof(char) * 
        Calc -> UnprocessedImageSize, 0);
    PixelsSum = (int *) MiscISCMallocAndSet(Calc, sizeof(int) * 
        Calc -> UnprocessedImageSize, 0);
   
    for (j = 0; j < Calc -> NumPictures; ++j) {
        for (i = 0; i < Calc -> UnprocessedImageSize; ++i) {
            if (CanCover[i]) {
                continue;
            }
            PixelsSum[i] += 
                ((MiscISCPixelType **) (Calc -> UnprocessedPictures))[j][i];
            if (PixelsSum[i] >= (Calc -> UnprocessedRequiredCover)[i]) {
                CanCover[i] = 1;
            }
        }
    }

    Calc -> RequiredCover = (MiscISCPixelType*) MiscISCMalloc(Calc, 
        sizeof(MiscISCPixelType) * Calc -> UnprocessedImageSize);
    for (i = 0; i < Calc -> UnprocessedImageSize; ++i) {
        TotalSum += (Calc -> UnprocessedRequiredCover)[i];
        if (!CanCover[i]) {
            Calc -> RequiredCover[i] = PixelsSum[i];
        }
        else {
            Calc -> RequiredCover[i] = Calc -> UnprocessedRequiredCover[i];
        }
        CoverableSum += Calc -> RequiredCover[i];
    }
    Calc -> NumOfPixeslToCover = TotalSum;
    Calc -> SolutionCover = CoverableSum;

    MiscISCFree(Calc, PixelsSum);
    MiscISCFree(Calc, CanCover);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used before any search algorithm begins.                                  *
*   Processing all the input of the calculator and creating from it the data  *
* requried for the calculation. Therefore, after calling this function new    *
* pictures can't be added and UnprocessedRequiredCover can't be set.          *
*                                                                             *
*   More specificaly:                                                         *
*   In B&W calculators:                                                       *
*   1. Prepare the processed pictures that will be used in the actual         *
* calculation and the processed RequiredCover.                                *
* The processed pictures are created by taking from the original pictures only*
* the following pixels:                                                       *
*     * Only pixels that can actually be covered by one of the pictures.      *
*     * The pixels in the pictures are grouped such that all the pixels in    *
* each group are covered by exactly the same pictures. In each group all the  *
* pixel inforce the same constraints and therefore are redundant. Thus, from  *
* each such group only the first pixel is taken to the final pictures.        *
*   2. Set Calc -> SolutionCover to the total possible pixels in the          *
* unprocessed image which can be covered by pictures.                         *
*                                                                             *
*   In gray calculators:                                                      *
*   1. Set Calc -> SolutionCover to the total possible cover by the calculator*
*   2. Set Calc -> RequiredCover to the sum of what all the Pictures can cover*
*      out of the unprocessed required cover (Calc->UnprocessedRequiredCover).*
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:            The Calculator.                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCMoveCalcToComputePhase(MiscISCCalculatorPtrType Calc)
{
    MiscISCLongJmpCond(Calc, !Calc -> ComputePhase, 
        MISC_ISC_ALREADY_IN_COMPUTE_PHASE);
    Calc -> ComputePhase = TRUE;
    if (Calc -> ColorType == MISC_ISC_BNW) {
        MiscISCShrinkCalculator(Calc);
        MiscISCZipCalculator(Calc);        
    }
    else {
        /* Calc -> ColorType == MISC_ISC_GRAY. */
        if (Calc -> UnprocessedRequiredCover == NULL) {
            /* If no limitation was given of what to cover, we cover all. For*/
            /* that we create Calc -> UnprocessedRequiredCover with maximum  */
            /* value in each pixel.                                          */
            MiscISCImageSizeType i;

            Calc -> UnprocessedRequiredCover = (MiscISCPixelType *) 
                MiscISCMalloc(Calc, sizeof(MiscISCPixelType) * 
                Calc -> UnprocessedImageSize);
            /* Not using memset here because we don't assume                 */
            /* sizeof(MiscISCPixelType) == 1.                                */
            for (i = 0; i < Calc -> UnprocessedImageSize; ++i) {
                ((MiscISCPixelType *) Calc -> UnprocessedRequiredCover)[i] = 
                    MISC_ISC_MAX_GRAY_COVER;
            }
        }
        MiscISCUpdateGrayCoverability(Calc);
        Calc -> ImageSize = Calc -> UnprocessedImageSize;
        Calc -> ImageSizeInBytes = Calc -> ImageSize * sizeof(MiscISCPixelType);
        Calc -> Pictures = (void **) Calc -> UnprocessedPictures;
    }
    if (Calc -> UseCoverLimit)
        Calc -> CoverLimitInPixels = (MiscISCImageSizeType)
            (Calc -> CoverLimit * Calc -> NumOfPixeslToCover);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm.                              *
*   Compare the selection rank of 2 given solutions.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*    s1:  A solution.                                                         *
*    s2:  A solution.                                                         *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: retuen -1 if s1 > s2 and 1 otherwise.                               *
******************************************************************************/
static int MiscISCCompareSelectionRanks(const void *s1, const void *s2) 
{
    return (*(MiscISCSltnNodePtrType *) s1) -> SelectionRank > 
           (*(MiscISCSltnNodePtrType *) s2) -> SelectionRank ? -1 : 1;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm.                              *
*   Calculates the selection rank for the given picture.                      *
* The selection rank is the weighted sum of all the pixel covered by the      *
* picture. The weight of each pixel is 1/(num of pictures covering the pixel).*
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:         The calculator.                                            *
*    PictureIndex: The picture's index.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*    IrtRType:     The selection rank.                                        *
******************************************************************************/
static IrtRType MiscISCGetSelectionRank(MiscISCCalculatorPtrType Calc,
                                        int PictureIndex) 
{
    
    IrtRType 
        Ret = 0.0;
    MiscISCImageSizeType i;

    if (Calc -> SelectionRanks[PictureIndex] > -MISC_ISC_EPSILON) {
        return Calc -> SelectionRanks[PictureIndex];
    }

    for (i = 0; i < Calc -> ImageSize; ++i) {
        if (Calc -> NeedToCover(Calc -> Pictures[PictureIndex], i))
            Ret += 1.0 / ((IrtRType) Calc -> VisibilitySetsSizes[i]);
    }
    return Ret;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm.                              *
*   Calculates the total selection rank for the given pixel.                  *
*   The total selection rank is the sum of the selection ranks (see           *
* MiscISCGetSelectionRank) of all the pictures covering the pixel.            *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:       The calculator.                                              *
*    PixelIndex: The pixel's index.                                           *
*                                                                             *
* RETURN VALUE:                                                               *
*    IrtRType:   The total selection rank.                                    *
******************************************************************************/
static IrtRType MiscISCGetTotalSelectionRank(MiscISCCalculatorPtrType Calc,
                                             int PixelIndex) 
{
    
    IrtRType 
        Ret = 0.0;
    int i;

    if (Calc -> TotalSelectionRanks[PixelIndex] > -MISC_ISC_EPSILON) {
        return Calc -> TotalSelectionRanks[PixelIndex];
    }

    for (i = 0; i < Calc -> NumPictures; ++i) {
        if (Calc -> NeedToCover(Calc -> Pictures[i], PixelIndex))
            Ret += MiscISCGetSelectionRank(Calc, i);
    }
    return Ret;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm. Creates a solution(a node) in*
* the search tree.                                                            *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The Calculator.                                          *
*   Size:            The size of the solution.                                *
*   Cover:           The number of pixels in the original image               *
*                    covered by the solution.                                 *
*   PicNumber:       The picture index of the solution.                       *
*   SelectionRank:   The selection rank of the picture.                       *
*                                                                             *
* RETURN VALUE:                                                               *
*    MiscISCSltnNodePtrType: The solution.                                    *
******************************************************************************/
static MiscISCSltnNodePtrType MiscISCCreateNewSolutionNode(
                                              MiscISCCalculatorPtrType Calc,
                                              int Size, 
                                              MiscISCImageSizeType Cover,
                                              int PicNumber, 
                                              IrtRType SelectionRank) 
{
    MiscISCSltnNodePtrType Ret;

    Ret = MiscISCMalloc(Calc, sizeof(struct MiscISCSltnNodeStruct));
    Ret -> Size = Size;
    Ret -> Cover = Cover;
    Ret -> PicNumber = PicNumber;
    Ret -> SelectionRank = SelectionRank;

    return Ret;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm.                              *
*   Create a child solution node for the given SolutionNode.                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   ToCover:         IN, the image left to cover.                             *
*   SolutionNode:    OUT, the solution node parent of the created child       *
*                    solution.                                                *
*   PictureIndex:    IN, The picture index of the child solution.             *
*                                                                             *
* RETURN VALUE:                                                               *
*    MiscISCSltnNodePtrType: The solution node.                               *
******************************************************************************/
static MiscISCSltnNodePtrType MiscISCCreateChildSltnNode(
                                          MiscISCCalculatorPtrType Calc,
                                          void *ToCover,
                                          MiscISCSltnNodePtrType SolutionNode,
                                          int PictureIndex) 
{
    MiscISCImageSizeType
        Cover = SolutionNode -> Cover + 
            Calc -> GreedyGetPictureValue(Calc, Calc -> Pictures[PictureIndex], 
            ToCover);

    return MiscISCCreateNewSolutionNode(Calc, SolutionNode -> Size+1, Cover, 
        PictureIndex, MiscISCGetSelectionRank(Calc, PictureIndex));
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the MiscISCCalculateExact algorithm.                              *
*   Find the split pixel index (see MiscISCExpendSolutionNode).               *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:            The calculator.                                         *
*    ToCover:         IN, The image left to cover.                            *
*    SplitPixelIndex: OUT, the split index.                                   *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCFindSplitIndex(MiscISCCalculatorPtrType Calc, 
                                  void *ToCover,
                                  MiscISCImageSizeType *SplitPixelIndex) 
{
    MiscISCImageSizeType j, k, PrevMinIndex, 
        NumMin = 1;
    int MinVal;
    IrtRType MaxTotalSelectionRank;

    MiscISCLongJmpCond(Calc, ToCover != NULL,
        MISC_ISC_NULL_IN_MISC_ISC_FIND_SPLIT_INDEX);

    for (k = 0; k < Calc -> ImageSize; ++k) {
        if (Calc -> NeedToCover(ToCover, k) > 0) {
            break;
        }
    }

    MiscISCLongJmpCond(Calc, k != Calc -> ImageSize, 
        MISC_ISC_LOGIC_ERROR_IN_MISC_ISC_FIND_SPLIT_INDEX);

    PrevMinIndex = k; 
    MinVal = Calc -> VisibilitySetsSizes[k];
    Calc -> MinFinder[k] = -1;

    for (j = k+1; j < Calc -> ImageSize; ++j) {
        if (Calc -> NeedToCover(ToCover, j) == 0) {
            continue;
        }
        if (Calc -> VisibilitySetsSizes[j] == MinVal) {
            NumMin++;
            Calc -> MinFinder[j] = PrevMinIndex;
            PrevMinIndex = j;
        }
        if (Calc -> VisibilitySetsSizes[j] < MinVal) {
            NumMin = 1;
            Calc -> MinFinder[j] = -1;
            PrevMinIndex = j;
            MinVal = Calc -> VisibilitySetsSizes[j];
        }
    }

    *SplitPixelIndex = PrevMinIndex;

    if (NumMin == 1)
        return;

    MaxTotalSelectionRank = 
                MiscISCGetTotalSelectionRank(Calc, PrevMinIndex);
    PrevMinIndex = Calc -> MinFinder[PrevMinIndex];

    while (PrevMinIndex != -1) {
        IrtRType
            CurrTotalSelectionRank =
                            MiscISCGetTotalSelectionRank(Calc, PrevMinIndex);

        if (CurrTotalSelectionRank > MaxTotalSelectionRank) {
            *SplitPixelIndex = PrevMinIndex;
        }
        PrevMinIndex = Calc -> MinFinder[PrevMinIndex];
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Used in the exact algorithm.                                              *
*   Create children solution nodes for SolutionNode. The children are all the *
* pixels which cover the split pixel found by MiscISCFindSplitIndex. This is  *
* the pixel covered by the smallest number of pictures. If there are several  *
* pixels with the same number of pictcures covering them MiscISCFindSplitIndex*
* will return the one with the largest total selection rank (see              *
* MiscISCGetTotalSelectionRank).                                              *
*   The children are sort by the total selection rank of the pictures they    *
* represents. The next step of the exact algorithm will use those children by *
* that order.                                                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:            IN, the calculator.                                     *
*    SolutionNode:    IN, the solution node to expand.                        *
*    ToCover:         IN, the image left to cover.                            *
*    Children:        OUT, the children solutions.                            *
*    NumChildren:     OUT, the number of children.                            *
*    SplitPixelIndex: OUT, the split pixel index.                             * 
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCExpendSolutionNode(MiscISCCalculatorPtrType Calc, 
                                      MiscISCSltnNodePtrType SolutionNode, 
                                      void *ToCover,
                                      MiscISCSltnNodePtrType **Children,
                                      int *NumChildren,
                                      MiscISCImageSizeType *SplitPixelIndex) 
{

    MiscISCSltnNodePtrType *ChildrenInner;
    int i, 
        NumChildrenInner = 0;

    MiscISCLongJmpCond(Calc, (NumChildren != NULL) && (Children != NULL) && 
        (ToCover != NULL), 
        MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_EXPEND_SOLUTION_NODE);

    MiscISCFindSplitIndex(Calc, ToCover,  SplitPixelIndex);
    /* Calc -> VisibilitySetsSizes[SplitPixelIndex] is an upper bound on the */
    /* number of childrens.                                                  */
    ChildrenInner = (MiscISCSltnNodePtrType *) 
        MiscISCMallocAndSet(Calc, sizeof(MiscISCSltnNodePtrType) * 
                            Calc -> VisibilitySetsSizes[*SplitPixelIndex], 0);
    for (i = 0; i < Calc -> NumPictures; ++i) {
        if ((Calc -> UnusedPictures[i] == MISC_ISC_NOT_USED) 
            && Calc -> NeedToCover(Calc -> Pictures[i],*SplitPixelIndex)) {
            ChildrenInner[NumChildrenInner] = MiscISCCreateChildSltnNode(
                Calc, ToCover, SolutionNode, i);
            NumChildrenInner++;
        }
    }

    qsort(ChildrenInner, NumChildrenInner, sizeof(MiscISCSltnNodePtrType),
          MiscISCCompareSelectionRanks);
    *Children = ChildrenInner;
    *NumChildren = NumChildrenInner;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   The main search function of the exact algorithm. It recursively choose    *
* children for the tree and try to find the best cover.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc:             The calculator.                                        *
*    SolutionNode:     The solution node to start the search from.            *
*    ToCover:          The image left to cover.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*    int:              0 found. -1 didn't find.                               *
******************************************************************************/
static void MiscISCSearchExact(MiscISCCalculatorPtrType Calc,
                               MiscISCSltnNodePtrType SolutionNode,
                               void *ToCover) 
{

    MiscISCSltnNodePtrType 
        *Children = NULL;
    int NumChildren, i, j,
        Size = SolutionNode -> Size;
    MiscISCImageSizeType
        Cover = SolutionNode -> Cover,
        SplitPixelIndex;
    void *ToCoverOfChild;

    MiscISCLongJmpCond(Calc, (ToCover != NULL) && (Calc -> Pictures != NULL),
        MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SEARCH_EXACT);

    /* Check whether to stop the search and save the combination as a        */
    /* solution. Notice that altough MiscISCStopSearch consider the          */
    /* possiblity of CoverLimit, that possiblity should never be active from */
    /* in MiscISCSearchExact.                                                */
    if (MiscISCStopSearch(Calc, ToCover, Size, Cover))
        return;

    MiscISCExpendSolutionNode(Calc, SolutionNode, ToCover, &Children, 
        &NumChildren, &SplitPixelIndex);
    ToCoverOfChild = MiscISCMalloc(Calc, Calc -> ImageSizeInBytes);    
    for (i = 0; i <= NumChildren - 1; ++i) {
        /* Update database with the new node. */
        memcpy(ToCoverOfChild, ToCover, Calc -> ImageSizeInBytes);
        Calc -> UpdateToCover(Calc -> Pictures[Children[i] -> PicNumber], 
            ToCoverOfChild, Calc -> ImageSizeInBytes);
        Calc -> UnusedPictures[Children[i] -> PicNumber] = MISC_ISC_USED;

        /* Go down to the new node. */
        MiscISCSearchExact(Calc, Children[i], ToCoverOfChild);
        /* Update database that we returned from the previous node. */
        Calc -> UnusedPictures[Children[i] -> PicNumber] = MISC_ISC_NOT_USED;
        /* The last check may have found a new full solution one step down   */
        /* the tree and therefore we can't find better solution in the       */
        /* current branch.                                                   */
        if ((Calc -> FoundFullSolution) && (Size == Calc -> SolutionSize - 1))
            break;
        MiscISCFree(Calc, Children[i]);
    }
    for (j = i; j < NumChildren; ++j) {
        MiscISCFree(Calc, Children[j]);
    }
    MiscISCFree(Calc, Children);
    MiscISCFree(Calc, ToCoverOfChild);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Free the data allocated in MiscISCInitExactorArray.                       *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc: The calculator.                                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCFreeExactorArray(MiscISCCalculatorPtrType Calc) 
{
    MiscISCFree(Calc, Calc -> UnusedPictures);
    MiscISCFree(Calc, Calc -> TotalSelectionRanks);
    MiscISCFree(Calc, Calc -> SelectionRanks);
    MiscISCFree(Calc, Calc -> MinFinder);
    MiscISCFree(Calc, Calc -> VisibilitySetsSizes);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Allocate and initializes necessary variables inside Calc. Used in         *
* MiscISCCalculateExact and MiscISCCalculateExhaustive (only few variables are*
* used in this algorithm).                                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*    Calc: The calculator.                                                    *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
******************************************************************************/
static void MiscISCInitExactorArray(MiscISCCalculatorPtrType Calc) 
{
    int i;
    MiscISCImageSizeType j;

    MiscISCLongJmpCond(Calc, Calc -> ComputePhase, 
        MISC_ISC_MISC_ISC_INIT_EXACTOR_ARRAY_NOT_IN_COMPUTE_PHASE);

    Calc -> UnusedPictures = (MiscISCUsePictureTypeType *) MiscISCMalloc(
        Calc, sizeof(*Calc -> UnusedPictures) * (Calc -> NumPictures + 1));
    Calc -> TotalSelectionRanks = (IrtRType *) MiscISCMalloc(
        Calc, sizeof(IrtRType)  * Calc -> ImageSize);
    Calc -> SelectionRanks = (IrtRType *) MiscISCMalloc(
        Calc, sizeof(IrtRType)  * Calc -> NumPictures);
    Calc -> MinFinder = (int *) MiscISCMalloc(
        Calc, sizeof(int) * Calc -> ImageSize);
    Calc -> VisibilitySetsSizes = (int *) MiscISCMallocAndSet(
        Calc, sizeof(int) * Calc -> ImageSize, 0);

    for (i = 0; i < Calc -> NumPictures; ++i) {
        for (j = 0; j < Calc -> ImageSize; ++j) {
            if (Calc -> NeedToCover(Calc -> Pictures[i], j))
                Calc -> VisibilitySetsSizes[j]++;
        }
        Calc -> SelectionRanks[i] = -1.0;
        Calc -> UnusedPictures[i] = MISC_ISC_NOT_USED;
    }
    
    for (j = 0; j < Calc -> ImageSize; ++j) {
        Calc -> TotalSelectionRanks[j] = -1.0;
    }
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Calculates the cover. Uses an exact exponential Algorithm.                M
*   Each MiscISCCalculatorPtrType can be used only once with any search       M
* algorithm.                                                                  M
*   Calling this funcion will move the calculator to compute phase.           M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc:            The calculator.                                          M
*   SizeLimit:       IN, if this value is greater than 0 the algorithm will   M
*                    stop adding pictures when getting to that size of        M
*                    pictures combinations and revert to search in other      M
*                    branches. This will speed the algorithm though may not   M
*                    find any solution at all in which case the combination   M
*                    with the best cover will be returned.                    M
*   SolutionByIndex: OUT, The solution as indices of pictures.                M
*   SolutionSize:    OUT, The size of the solution (size of SolutionByIndex). M
*   CoverPart:       OUT, The part of the uprocessed  picture covered by the  M
*                    solution.                                                M
*                                                                             *
* RETURN VALUE:                                                               M
*   int: FALSE if error occured.                                              M
*                                                                             *
* SEE ALSO:                                                                   M
*    MiscISCCalculateApprox                                                   M
*                                                                             *
* KEYWORDS:                                                                   M
*    MiscISCCalculateExact, MiscISCCalculateExactAux                          M
*****************************************************************************/
int MiscISCCalculateExact(MiscISCCalculatorPtrType Calc,
                          int SizeLimit,
                          int **SolutionByIndex,
                          int *SolutionSize,
                          IrtRType *CoverPart)
{
    int SetjmpReturn;

    if (SizeLimit > 0) {
        Calc -> UseSizeLimit = TRUE;
        Calc -> SizeLimit = SizeLimit;
    }

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(Calc -> MiscISCJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, 
        "MiscISCCalculateExact")) 
        return FALSE;
    MiscISCCalculateExactAux(Calc, SolutionByIndex, SolutionSize, CoverPart);
    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Implementation for MiscISCCalculateExact.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   SolutionByIndex: See MiscISCCalculateExact.                               *
*   SolutionSize:    See MiscISCCalculateExact.                               *
*   CoverPart:       See MiscISCCalculateExact.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   int: FALSE if error occured.                                              *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCCalculateExactAux, MiscISCCalculateExact                          *
*****************************************************************************/
static void MiscISCCalculateExactAux(MiscISCCalculatorPtrType Calc,
                                     int **SolutionByIndex,
                                     int *SolutionSize,
                                     IrtRType *CoverPart)
{
    MiscISCSltnNodePtrType 
        InitialSolutionNode = NULL;

    MiscISCLongJmpCond(Calc, Calc != NULL, 
        MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_CALCULATE_EXACT_AUX);

    /* Notice that Calc->SolutionByIndex, Calc->SolutionSize and            */
    /* Calc->SolutionCover are set inside MiscISCCalculateGreedySolutionAux.*/
    MiscISCCalculateGreedyAux(Calc, SolutionByIndex, SolutionSize, 
        CoverPart);
    MiscISCInitExactorArray(Calc);

    /* The actual call to solve the problem. */
    InitialSolutionNode = MiscISCCreateNewSolutionNode(Calc, 0, 0, -1, -1.0);
    MiscISCSearchExact(Calc, InitialSolutionNode, Calc -> RequiredCover);

    /* An exact solution was found. Need to convert it from Calc -> Solution */
    /* to Calc -> SolutionByIndex.                                           */
    if (!Calc -> SolutionByIndex) {
        int i,
            Curr = 0;

        for (i = 0; i < Calc -> NumPictures; ++i) {
            if (Calc -> Solution[i] == MISC_ISC_USED) {
                (*SolutionByIndex)[Curr] = i;
                Curr++;
            }
        }
        *SolutionSize = Calc -> SolutionSize;
        *CoverPart = MiscISCGetCoverPart(Calc, Calc -> SolutionCover);
    }    

    MiscISCFree(Calc, InitialSolutionNode);
    MiscISCFreeExactorArray(Calc);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Decide whether to stop the search and whether to save the current         *
* combination of pictures as a solution. The decision relies on               *
* Calc -> UseSizeLimit, Calc -> UseCoverLimit and other parameters of Calc and*
* the current found solution.                                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:         The Calculator.                                             *
*   ToCover:      The image left to cover.                                    *
*   Size:         The size of the current combination.                        *
*   Cover:        How many pixels of the unprocessed image are covered by the *
*                 current combination of pictures.                            *
*                                                                             *
* RETURN VALUE:                                                               *
*    int: TRUE if the search should be stopped.                               *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCStopSearch                                                        *
******************************************************************************/
static int MiscISCStopSearch(MiscISCCalculatorPtrType Calc,
                             void *ToCover,
                             int Size,
                             MiscISCImageSizeType Cover) {

    int StopFullSolutionFound, StopReachFullSolutionSize, StopSizeLimit,
        StopCoverLimit;

    StopFullSolutionFound = MiscISCIsZeroArray(ToCover, 
        Calc -> ImageSizeInBytes);
    StopReachFullSolutionSize = 
        Calc -> FoundFullSolution && (Size + 1 == Calc -> SolutionSize);
    StopSizeLimit = ((Calc -> UseSizeLimit) && (Size == Calc -> SizeLimit))||
        (Size == Calc -> SolutionSize); 
    StopCoverLimit = 
        (Calc -> UseCoverLimit) && (Cover > Calc -> CoverLimitInPixels);
    
    /* We stop the search on the current branch if one of the following      */
    /* conditions has been reached.                                          */
    if (
        /* We found a full solution. */
        StopFullSolutionFound ||
        /* We reached size of found solution - 1 which means we can't find   */
        /* better solution.                                                  */
        StopReachFullSolutionSize ||
        /* The solution is limited to maximum size of pictures and we reached*/
        /* it. Or, we have already found the size of some solution (using the*/
        /* (gready algorithm prior to using this algorithm or by earlier     */
        /* stages of this algorithm) and we reached that size which means we */
        /* can't find better solution.                                       */
        StopSizeLimit ||
        /* The user may set a CoverLimit which any cover above it is         */
        /* acceptable as a solution. The current combination passes that     */
        /* amount.                                                           */
        StopCoverLimit) {
        /* We save the new solution instead of the previous one if: */
        if (
            /* There wasn't any previous solution. */
            (!Calc -> FoundSolution) ||
            /* We found new full solution. The new solution's size must be   */
            /* smaller than any previous one (otherwise we would have broken */
            /* the search earlier).                                          */
            StopFullSolutionFound ||
            /* The new solution covers more. The new solution must be equal  */
            /* or smaller than the previous one (otherwisw we would have     */
            /* broken the search earlier). Also, if we have already found a  */
            /* full solution this condition will obviously fail for any non  */
            /* full solution (because Calc -> SolutionCover is 1).           */
            (Cover > Calc -> SolutionCover)||
            /* The new solution is smaller. StopCoverLimit make sure that we */
            /* indeed stopped with a new solution found and not for other    */
            /* condition. StopFullSolutionFound and StopSizeLimit for smaller*/
            /* size were already handled in the previous conditions.         */
            /* StopCoverLimit with the same size but bigger cover was also   */
            /* handled in previous condition.                                */
            (StopCoverLimit && (Size < Calc -> SolutionSize))) {
                if (Calc -> SolutionByIndex != NULL) {
                    MiscISCFree(Calc, Calc -> SolutionByIndex);
                    Calc -> SolutionByIndex = NULL;
                }
                Calc -> Solution = (MiscISCUsePictureTypeType *)
                    MiscISCArrayCopy(Calc, Calc -> Solution, 
				     Calc -> UnusedPictures, 
				     Calc -> NumPictures *
				         sizeof(*Calc -> UnusedPictures));
                Calc -> SolutionSize = Size;
                Calc -> SolutionCover = Cover;
                /* Debug message about the progress. */
		if (Calc -> UseCoverLimit) {
                    if (!Calc -> FoundSolution) /* First time. */
                        printf("Requested Cover: %d pixels (%.7g of the "
                        "image)\n", (int) Calc -> CoverLimitInPixels, 
                        Calc -> CoverLimit);
                    printf("Found cover: %d pixels (%.7g of the image) "
			   "%d pictures.\n", 
			   (int) Cover, MiscISCGetCoverPart(Calc, Cover),
			   Calc -> SolutionSize);
                }
        }

        Calc -> FoundSolution = TRUE;
        if (StopFullSolutionFound)
            Calc -> FoundFullSolution = TRUE;
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Calculates the cover. Uses an exhaustive exponential Algorithm.           M
*   Each MiscISCCalculatorPtrType can be used only once with any search       M
* algorithm.                                                                  M
*   Calling this funcion will move the calculator to compute phase.           M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc:            The calculator.                                          M
*   CoverLimit:      IN, if this value is in (0,1), the algorithm will        M
*                    consider a combination of pictures which covers          M
*                    CoverLimit part of the image as a valid solution.        M
*   SizeLimit:       IN, if this value is greater than 0 the algorithm will   M
*                    stop adding pictures when getting to that size of        M
*                    pictures combinations and revert to search in other      M
*                    branches. This will speed the algorithm though may not   M
*                    find any solution at all in which case the combination   M
*                    with the best cover will be returned.                    M
*   SolutionByIndex: OUT, The solution as indices of pictures.                M
*   SolutionSize:    OUT, The size of the solution (size of SolutionByIndex). M
*   CoverPart:       OUT, The part of the unprocessed pictured covered by the M
*                    solution.                                                M
*                                                                             *
* RETURN VALUE:                                                               M
*   int: FALSE if error occured.                                              M
*                                                                             *
* SEE ALSO:                                                                   M
*    MiscISCCalculateApprox                                                   M
*                                                                             *
* KEYWORDS:                                                                   M
*    MiscISCCalculateExhaustive, MiscISCCalculateExact                        M
******************************************************************************/
int MiscISCCalculateExhaustive(MiscISCCalculatorPtrType Calc,
                               IrtRType CoverLimit,
                               int SizeLimit,
                               int **SolutionByIndex,
                               int *SolutionSize,
                               IrtRType *CoverPart)
{
    int SetjmpReturn;

    if ((CoverLimit > 0 ) && (CoverLimit < 1)) {
        Calc -> UseCoverLimit = TRUE;
        Calc -> CoverLimit = CoverLimit;
    }
    if (SizeLimit > 0) {
        Calc -> UseSizeLimit = TRUE;
        Calc -> SizeLimit = SizeLimit;
    }

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(Calc -> MiscISCJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, 
			    "MiscISCCalculateExhaustive")) 
        return FALSE;
    MiscISCCalculateExhaustiveAux(Calc, SolutionByIndex, SolutionSize,
        CoverPart);
    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Implementation for MiscISCCalculateExact.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   SolutionByIndex: See MiscISCCalculateExact.                               *
*   SolutionSize:    See MiscISCCalculateExact.                               *
*   CoverPart:       See MiscISCCalculateExact.                               *
*                                                                             *
* RETURN VALUE:                                                               *
*   int: FALSE if error occured.                                              *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCCalculateExactAux, MiscISCCalculateExact                          *
*****************************************************************************/
static void MiscISCCalculateExhaustiveAux(MiscISCCalculatorPtrType Calc,
                                          int **SolutionByIndex,
                                          int *SolutionSize,
                                          IrtRType *CoverPart)
{ 
    long Combinations;

    MiscISCLongJmpCond(Calc, Calc != NULL, 
		MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_CALCULATE_EXHAUSTIVE_AUX);

    /* Notice that Calc->SolutionByIndex, Calc->SolutionSize and             */
    /* Calc->SolutionCover are set inside MiscISCCalculateGreedySolutionAux. */
    MiscISCCalculateGreedyAux(Calc, SolutionByIndex, SolutionSize, 
        CoverPart);
    MiscISCInitExactorArray(Calc);

    /* The actual call to solve the problem. */
    Combinations = 0;
    MiscISCSearchExhaustive(Calc, (int) time(NULL), &Combinations, 
        Calc -> RequiredCover, 0, 0, -1);

    /* An exhaustive solution was found. Need to convert it from             */
    /* Calc -> Solution to Calc -> SolutionByIndex.                          */
    if (!Calc -> SolutionByIndex) {
        int i,
            Curr = 0;

        *SolutionByIndex = 
            (int *) MiscISCMalloc(Calc, Calc -> NumPictures * sizeof(int));
        for (i = 0; i < Calc -> NumPictures; ++i) {
            if (Calc -> Solution[i] == MISC_ISC_USED) {
                (*SolutionByIndex)[Curr] = i;
                Curr++;
            }
        }
        *SolutionSize = Calc -> SolutionSize;
        *CoverPart = MiscISCGetCoverPart(Calc, Calc -> SolutionCover);

    }
    MiscISCFreeExactorArray(Calc);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   The main search function of the MiscISCCalculateExhaustive algorithm. It  *
* recursively choose children for the tree and try to find the best cover.    *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:         The Calculator.                                             *
*   StartTime:    The time the search started (used for debug messages about  *
*                 the progress).                                              *
*   Combinations: Number of Combinations checked by now (used for debug       *
*                 messages about the progress).                               *
*   ToCover:      The image left to cover.                                    *
*   Size:         The size of the current combination.                        *
*   Cover:        How many pixels of the unprocessed image are covered by the *
*                 current combination of pictures.                            *
*   LastPicIndex: The index of the last picture added. Next picture will be   *
*                 searched only in pictures with bigger index in order to     *
*                 prevent repetitions.                                        *
*                                                                             *
* RETURN VALUE:                                                               *
*    void                                                                     *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCSearchExhaustive                                                  *
******************************************************************************/
static void MiscISCSearchExhaustive(MiscISCCalculatorPtrType Calc,
                                    int StartTime,
                                    long *Combinations,
                                    void *ToCover,
                                    int Size,
                                    MiscISCImageSizeType Cover,
                                    int LastPicIndex)
{
    int i;
    void *ToCoverOfChild;

    MiscISCLongJmpCond(Calc, (ToCover != NULL) && (Calc -> Pictures != NULL),
        MISC_ISC_ILLEGAL_VALUES_IN_MISC_ISC_SEARCH_EXHAUSTIVE_AUX);

    /* Debug message about the progress. */
    (*Combinations)++;
    if (*Combinations % 100000 == 0) {
        int TimePassed = (int) time(NULL) - StartTime;

        printf("Number of combinations checked: %d TimeFromStart: %s\n",
	       (int) *Combinations, MiscISCGetTimeStamp(TimePassed, NULL, 0));
    }

    /* Check whether to stop the search and save the combination as a        */
    /* solution.                                                             */
    if (MiscISCStopSearch(Calc, ToCover, Size, Cover))
        return;

    ToCoverOfChild = MiscISCMalloc(Calc, Calc -> ImageSizeInBytes);    
    for (i = LastPicIndex + 1; i <= Calc -> NumPictures - 1; ++i) {
        MiscISCImageSizeType CoverOfChild;

        /* Update database with the new picture. */
        memcpy(ToCoverOfChild, ToCover, Calc -> ImageSizeInBytes);
        Calc -> UpdateToCover(Calc -> Pictures[i], ToCoverOfChild, 
            Calc -> ImageSizeInBytes);
        CoverOfChild = Cover + Calc -> GreedyGetPictureValue(Calc, 
            Calc -> Pictures[i], ToCover); 
        Calc -> UnusedPictures[i] = MISC_ISC_USED;

        /* Go down to the new node. */
        MiscISCSearchExhaustive(Calc, StartTime, Combinations, ToCoverOfChild, 
            Size+1, CoverOfChild, i);
        /* Update database that we returned from the previous node. */
        Calc -> UnusedPictures[i] = MISC_ISC_NOT_USED;

        /* The last check may have found a new full solution one step down   */
        /* the tree and therefore we can't find better solution in the       */
        /* current branch.                                                   */
        if ((Calc -> FoundFullSolution) && (Size == Calc -> SolutionSize - 1))
            break;
    }
    MiscISCFree(Calc, ToCoverOfChild);
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Return a time stamp HH:MM:SS from the given Time.                         *
*                                                                             *
* PARAMETERS:                                                                 *
*   Time:       The time to print.                                            *
*   TimeString: If not NULL, print the time to this string (at least 9 char   *
*               long.                                                         *
*   TimeZone:   How many hours to add for the time zone difference.           *
*                                                                             *
* RETURN VALUE:                                                               *
*   char *: The printed string.                                               *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCGetTimeStamp                                                      *
******************************************************************************/
char *MiscISCGetTimeStamp(int Time, char *TimeString, int TimeZone)
{
    IRIT_STATIC_DATA char StaticStr[9];
    char *Str;

    if (TimeString != NULL)
        Str = TimeString;
    else 
        Str = StaticStr;

    sprintf(Str, "%02d:%02d:%02d", 
	    (int) (((Time/3600) + TimeZone) % 24), (int) ((Time/60) % 60), 
	    (int) (Time % 60));

    return Str;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Calculates the number of pixels covered by ToCover in the unprocessed     *
* image.                                                                      *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   ToCover:         IN, the image left to cover.                             *
*                                                                             *
* RETURN VALUE:                                                               *
*   MiscISCImageSizeType: The number of pixels covered by ToCover in the      *
*                         unprocessed image.                                  *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCCalculateGreedySolutionAux, MiscISCCalculateGreedySolution        *
******************************************************************************/
static MiscISCImageSizeType MiscISCGetCoverAmount(
                                           MiscISCCalculatorPtrType Calc, 
                                           void *ToCover) 
{
    MiscISCImageSizeType i,
        Cover = 0;

    if (Calc -> ColorType == MISC_ISC_BNW) {
        for (i = 0; i <= Calc -> ImageSize - 1; i++) {
            if (!Calc -> NeedToCover(ToCover, i))
                Cover += Calc -> PixelsAmount[i];
        }
    }
    else { /* MISC_ISC_GRAY */
        for (i = 0; i <= Calc -> ImageSize - 1; i++) {
            if (!Calc -> NeedToCover(ToCover, i))
                Cover++;
        }
    }
    return Cover;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Calculates the part of the picture covered in the unprocessed image given *
* the number of pixels covered in the uncompressed image.                     *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   Cover:           The number of pixels covered by ToCover in the           *
*                    unprocessed image.                                       *
*                                                                             *
* RETURN VALUE:                                                               *
*   IrtRType: The part of the picture covered by ToCover in the unprocessed   *
*             image.                                                          *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCCalculateGreedySolutionAux, MiscISCCalculateGreedySolution        *
******************************************************************************/
static IrtRType MiscISCGetCoverPart(MiscISCCalculatorPtrType Calc, 
                                    MiscISCImageSizeType Cover) 
{
    /* Calculate the output CovetPart. */
    return 1.0 * Cover / Calc -> NumOfPixeslToCover;
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Calculates the cover using quick and not optimal greedy algorithm. In     M
* each step the algorithm choose the next picture to be the picture which     M
* covers the maximum number of pixel out of the original unprocessed picture. M
*   If Calc -> UseCoverLimit is TRUE, the algorithm's search will stop when   M
* reaching cover the size of Calc -> CoverLimit and return the solution.      M
*   Each MiscISCCalculatorPtrType can be used only once with any search       M
* algorithm.                                                                  M
*   Calling this funcion will move the calculator to compute phase.           M
*                                                                             *
* PARAMETERS:                                                                 M
*   Calc:            The calculator.                                          M
*   SolutionByIndex: OUT, The solution as indices of pictures by the order    M
*                    they were added to the solution.                         M
*   SolutionSize:    OUT, The size of the solution (size of SolutionByIndex). M
*   CoverPart:       OUT, The part of the unprocessed image covered by the    M
*                    solution.                                                M
*                                                                             *
* RETURN VALUE:                                                               M
*   int: FALSE if error occured.                                              M
*                                                                             *
* KEYWORDS:                                                                   M
*    MiscISCCalculateGreedy, MiscISCCalculateGreedyAux                        M
******************************************************************************/
int MiscISCCalculateGreedy(MiscISCCalculatorPtrType Calc,
                           int **SolutionByIndex,
                           int *SolutionSize,
                           IrtRType *CoverPart) 
{
    int SetjmpReturn;

    /* Error catching using longjmp. */
    SetjmpReturn = setjmp(Calc -> MiscISCJmpTarget);
    if (!MiscISCHandleError(Calc, SetjmpReturn, 
        "MiscISCCalculateGreedy")) 
        return FALSE;
    MiscISCCalculateGreedyAux(Calc, SolutionByIndex, SolutionSize,
			      CoverPart);
    return TRUE;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Implementation of MiscISCCalculateGreedy.                                 *
*                                                                             *
* PARAMETERS:                                                                 *
*   Calc:            The calculator.                                          *
*   SolutionByIndex: See MiscISCCalculateGreedySolution.                      *
*   SolutionSize:    See MiscISCCalculateGreedySolution.                      *
*   CoverPart:       See MiscISCCalculateGreedySolution.                      *
*                                                                             *
* RETURN VALUE:                                                               *
*   int: FALSE if error occured.                                              *
*                                                                             *
* KEYWORDS:                                                                   *
*    MiscISCCalculateGreedyAux, MiscISCCalculateGreedy                        *
******************************************************************************/
static void MiscISCCalculateGreedyAux(MiscISCCalculatorPtrType Calc,
                                      int **SolutionByIndex,
                                      int *SolutionSize,
                                      IrtRType *CoverPart)
{
    int i,
        BestPicture = 0; 
    MiscISCImageSizeType 
        TotalCover = 0,
        BestCover = 0;
    MiscISCUsePictureTypeType *Solution;
    MiscISCPixelType *RequiredCover;

    MiscISCLongJmpCond(Calc, Calc -> NumPictures > 0, 
        MISC_ISC_NO_COVERING_PICTURES_WERE_ADDED);
    MiscISCLongJmpCond(Calc, !Calc -> ComputePhase, 
        MISC_ISC_ATTEMP_TO_ACTIVATE_CALCULATOR_TWICE);

    MiscISCMoveCalcToComputePhase(Calc);
    /* Since the greedy algorithm may be called by other algorithm, we don't */
    /* alter Calc -> RequiredCover.                                          */
    RequiredCover = MiscISCArrayCopy(Calc, NULL, Calc -> RequiredCover, 
        Calc -> ImageSizeInBytes);
    *SolutionByIndex = 
        (int *) MiscISCMalloc(Calc, Calc -> NumPictures * sizeof(int));
    Solution = (MiscISCUsePictureTypeType*) MiscISCMallocAndSet(
        Calc, sizeof(MiscISCUsePictureTypeType) * Calc -> NumPictures, 
        0); /* setmem to 0 will make MISC_ISC_NOT_USED at all cells. */
 
    *SolutionSize = 0;
    while(TRUE){
        BestCover = 0;
        /* Find best next picture to add. */
        for (i = 0; i <= Calc -> NumPictures - 1; ++i) {
            MiscISCImageSizeType TempCover;

            TempCover = Calc -> GreedyGetPictureValue(Calc, 
                Calc -> Pictures[i], RequiredCover);
            if (TempCover > BestCover) {
                BestPicture = i;
                BestCover = TempCover;
            }
        }
        TotalCover += BestCover;
        if ((BestCover == 0) || (TotalCover > Calc -> CoverLimitInPixels))
            break;
        Calc -> UpdateToCover(Calc -> Pictures[BestPicture], 
            RequiredCover, Calc -> ImageSizeInBytes);
        Solution[BestPicture] = MISC_ISC_USED;
        (*SolutionByIndex)[*SolutionSize] = BestPicture;
        (*SolutionSize)++;
    }

    Calc -> SolutionByIndex = *SolutionByIndex;
    Calc -> SolutionSize = *SolutionSize;
    Calc -> SolutionCover = MiscISCGetCoverAmount(Calc,  
        RequiredCover);
    *CoverPart = MiscISCGetCoverPart(Calc, Calc -> SolutionCover);

    MiscISCFree(Calc, Solution);
    MiscISCFree(Calc, RequiredCover);
}
