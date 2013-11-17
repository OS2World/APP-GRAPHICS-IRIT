/******************************************************************************
* dtr3d2im.c - dither a 3d bolbs object that mimics 2 orthogonal images.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber,					 July 2010.   *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "user_loc.h"

#include "dither3d_2x2x2_2d.h"
#include "dither3d_3x3x3_2d.h"
#include "dither3d_4x4x4_2d.h"

#define USER_DITHER3D_HEURISTIC_MATCH

#define USER_3DDITHER_MAX_INT		0x3fffffff
#define USER_3DDITHER_PIXEL_COORD(x, y) ((x) + (y) * DitheredRowSize)

typedef struct User3DDitherRowMatchingStruct {
    int *Match1to2;             /* Row1[i] is matched to Row2[Match1to2[i]]. */
    int *Match2to1;             /* Row2[i] is matched to Row1[Match2to1[i]]. */
    IrtRType **PenaltyMatrix;             /* Penalty matrix of the matching. */
    unsigned char *Dither1Level;     /* Dithering level selected for input1. */
    unsigned char *Dither2Level;     /* Dithering level selected for input2. */
    unsigned char *Row1Inten;          /* The two current rows' intensities. */
    unsigned char *Row2Inten;
    int *Row1;		        /* Two rows intensities with error diffused. */
    int *Row2;
    IrtRType *Error1;		  /* Error to diffuse from the previous row. */
    IrtRType *Error2;	          /* Error to diffuse from the previous row. */
} User3DDitherRowMatchingStruct;

/* How much to divide 256 (unsigned char) by, to get the 4, 9, or 16 levels. */
IRIT_STATIC_DATA const int
    DitherIntensityLevels[5] = { 0, 0, 4, 9, 16 };
IRIT_STATIC_DATA const IrtRType
    DitherIntensityDivide[5] = { 0.0, 0.0, 51.01, 25.51, 15.01 };

static void User3dDitherReport(char *Line, IrtRType Progress);
static void User3DDitherHeuristicMatch(int RowSize,
				      int MatchWidth,
				      User3DDitherRowMatchingStruct *RowMatch);
static int User3DDitherGetPenalty(int Inten1, int Inten2, int DitherSize);
static IrtRType User3DDitherAccumPenalty(int RowSize,
					 User3DDitherRowMatchingStruct
					                           *RowMatch);
static IPVertexStruct *User3DDitherGetMicroPixels(
				   int ZLevel,
				   int RowSize,
				   int DitherSize,
				   User3DDitherRowMatchingStruct *RowMatch);
static IPObjectStruct *User3DDitherGetMicroBlobs(
				   int ZLevel,
				   int RowSize,
				   int DitherSize,
				   IrtRType SphereRad,
				   User3DDitherRowMatchingStruct *RowMatch);
static IrtRType User3DDither2Rows(const IrtImgPixelStruct *Row1Pixels,
				  const IrtImgPixelStruct *Row2Pixels,
				  int RowSize,
				  int DitherSize,
				  int MatchWidth,
				  int Negate,
				  User3DSpreadType SpreadMethod,
				  User3DDitherRowMatchingStruct *RowMatch);
static IPVertexStruct *User3DDitherAugmentContrast(IPVertexStruct *Pls,
						    int RowSize,
						    int DitherSize,
						    int MatchWidth,
						    int AugmentContrast);
static IPVertexStruct *User3DDitherAugmentContrastAux1(IrtBType *XCover,
						       IrtBType *YCover,
						       IrtBType *PixelCover,
						       int RowSize,
						       int DitherSize,
						       int MatchWidth,
						       IPVertexStruct **VVec,
						       int VVecLen);
static IPVertexStruct *User3DDitherAugmentContrastAux2(IrtBType *XCover,
						       IrtBType *YCover,
						       IrtBType *PixelCover,
						       int RowSize,
						       int DitherSize,
						       int MatchWidth,
						       IrtRType ZLevel,
						       int AugmentContrast);
static IPObjectStruct *User3DDither2ImagesAux(IrtImgPixelStruct *Image1,
					      IrtImgPixelStruct *Image2,
					      int Width,
					      int Height,
					      int DitherSize,
					      int MatchWidth,
					      int Negate,
					      int AugmentContrast,
					      User3DSpreadType SpreadMethod,
					      IrtRType SphereRad,
					      IrtRType *AccumPenalty);

#if defined(ultrix) && defined(mips)
static int User3DDitherSortZ(VoidPtr P1, VoidPtr P2);
static int User3DDitherSortXY(VoidPtr P1, VoidPtr P2);
#else
static int User3DDitherSortZ(const VoidPtr P1, const VoidPtr P2);
static int User3DDitherSortXY(const VoidPtr P1, const VoidPtr P2);
#endif /* ultrix && mips (no const support) */

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO
static void User3DDitherDebugDump(int RowSize,
				  User3DDitherRowMatchingStruct *RowMatch,
				  int Init);
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorting key function for qsort (see below).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:  Two point to sort out and compute their relative order.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    1, 0, -1 based on the relation between P1 and P2.                *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int User3DDitherSortZ(VoidPtr P1, VoidPtr P2)
#else
static int User3DDitherSortZ(const VoidPtr P1, const VoidPtr P2)
#endif /* ultrix && mips (no const support) */
{
    IPVertexStruct 
        *V1 = *((IPVertexStruct **) P1),
        *V2 = *((IPVertexStruct **) P2);

    return IRIT_SIGN(V1 -> Coord[2] - V2 -> Coord[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorting key function for qsort (see below).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:  Two point to sort out and compute their relative order.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    1, 0, -1 based on the relation between P1 and P2.                *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int User3DDitherSortXY(VoidPtr P1, VoidPtr P2)
#else
static int User3DDitherSortXY(const VoidPtr P1, const VoidPtr P2)
#endif /* ultrix && mips (no const support) */
{
    IPVertexStruct 
        *V1 = *((IPVertexStruct **) P1),
        *V2 = *((IPVertexStruct **) P2);

    return IRIT_SIGN(V1 -> Coord[0] + V1 -> Coord[1] -
		     V2 -> Coord[0] - V2 -> Coord[1]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Statuc report of progress.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:     Report progress.                                               *
*   Progress: A number between zero to one showing the progress of the work. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void User3dDitherReport(char *Line, IrtRType Progress)
{
#ifdef USER_3D_UPDATE_PROGRESS
    IRIT_STATIC_DATA IrtRType
        CrntProgress = 0.0;

    if (Progress >= 0.0)
        CrntProgress = Progress;

    fprintf(stderr, "BFrom2Img (%2d%%): %s                \r",
	    ((int) (CrntProgress * 100.0)), Line);
#endif /* USER_3D_UPDATE_PROGRESS */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct an optimal match (minimal penalty) between pixels in Row1 and  *
* Row2.  Penalty is zero between Row1[i] and Row2[j] if there is a valid 3D  *
* dithering matrix between intensities Row1[i] and Row[j].  Otherwise,	     *
* penalty is following the penalty matrix of the respective dithering size.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Row1Pixels, Row2Pixels:   Two rows of two input images to match.         *
*   RowSize:     Length of vectors Row1 and Row2.                            *
*   DitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.              *
*   MatchWidth:  Width to allow matching: between Row1[i] to Row2[i +/- k],  *
*                                                            k < MatchWidth. *
*   Negate:      TRUE to negate the images.				     *
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random *
*                spread to use.						     *
*   RowMatch:    All slots of this structure are to be updated by this func. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:      Resulting minimal penalty.                                *
*****************************************************************************/
static IrtRType User3DDither2Rows(const IrtImgPixelStruct *Row1Pixels,
				  const IrtImgPixelStruct *Row2Pixels,
				  int RowSize,
				  int DitherSize,
				  int MatchWidth,
				  int Negate,
				  User3DSpreadType SpreadMethod,
				  User3DDitherRowMatchingStruct *RowMatch)
{
    int i, j, p;
    IrtRType Div, Penalty;
    const IrtImgPixelStruct *Pxl1, *Pxl2;

    assert(DitherSize >= 2 && DitherSize <= 4);

    /* Computes actual levels as defined by the dithering matrices, taking  */
    /* into account error propagation from the previous row as follows:     */
    /*   Every pixel p[i][j] propagates its error to the next row to pixels */
    /* (p[i+1][j-1], p[i+1][j], p[i+1][j+1]) in ratios of (1/3, 5/9, 1/9).  */
    Div = DitherIntensityDivide[DitherSize];
    for (i = 0, Pxl1 = Row1Pixels, Pxl2 = Row2Pixels;
	 i < RowSize;
	 i++, Pxl1++, Pxl2++) {
        RowMatch -> Row1Inten[i] = (unsigned char)
		     (Pxl1 -> r * 0.3 + Pxl1 -> g * 0.59 + Pxl1 -> b * 0.11);
        RowMatch -> Row2Inten[i] = (unsigned char)
		     (Pxl2 -> r * 0.3 + Pxl2 -> g * 0.59 + Pxl2 -> b * 0.11);
	if (Negate) {
	    RowMatch -> Row1Inten[i] = 255 - RowMatch -> Row1Inten[i];
	    RowMatch -> Row2Inten[i] = 255 - RowMatch -> Row2Inten[i];
	}

        RowMatch -> Row1[i] = (int)
	    ((RowMatch -> Row1Inten[i] + Div * 0.5 + 
	      (i > 0 ? RowMatch -> Error1[i - 1] : 0.0) * 0.33333333333333 +
	      RowMatch -> Error1[i] * 0.5555555555555 +
	      (i < RowSize - 1 ? RowMatch -> Error1[i + 1] : 0.0) * 0.11111111111111) / Div);
        RowMatch -> Row2[i] = (int)
	    ((RowMatch -> Row2Inten[i] + Div * 0.5 +
	      (i > 0 ? RowMatch -> Error2[i - 1] : 0.0) * 0.33333333333333 +
	      RowMatch -> Error2[i] * 0.5555555555555 +
	      (i < RowSize - 1 ? RowMatch -> Error2[i + 1] : 0.0) * 0.11111111111111) / Div);

	RowMatch -> Row1[i] = IRIT_BOUND(RowMatch -> Row1[i],
					 0, DitherIntensityLevels[DitherSize]);
	RowMatch -> Row2[i] = IRIT_BOUND(RowMatch -> Row2[i],
					 0, DitherIntensityLevels[DitherSize]);
    }

    /* Update the penalties of the possible matchings. */
    for (i = 0; i < RowSize; i++) {
        for (j = 0; j < RowSize; j++)
	    RowMatch -> PenaltyMatrix[i][j] = -USER_3DDITHER_MAX_INT;

	for (j = IRIT_MAX(0, i - MatchWidth);
	     j <= IRIT_MIN(RowSize - 1, i + MatchWidth);
	     j++) {
	    Penalty = User3DDitherGetPenalty(RowMatch -> Row1[i],
					     RowMatch -> Row2[j],
					     DitherSize);
	    RowMatch -> PenaltyMatrix[i][j] = IRIT_ABS(Penalty);
	}
    }

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO2
    fprintf(stderr, "\n\nDITHER LEVELS:\n");
    for (i = 0; i < RowSize; i++)
        fprintf(stderr, "%d", RowMatch -> Row1[i]);
    fprintf(stderr, "\n");
    for (i = 0; i < RowSize; i++)
        fprintf(stderr, "%d", RowMatch -> Row2[i]);
    fprintf(stderr, "\n");
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO2 */

#ifdef DEBUG_USER_DITHER3D_DUMP_PENALYY
    fprintf(stderr, "\n\nPENALTY MATRIX:\n   ");
    for (i = 0; i < RowSize; i++)
	fprintf(stderr, "  %2d", RowMatch -> Row1[i]);
    for (i = 0; i < RowSize; i++) {
	fprintf(stderr, "\n%2d)", RowMatch -> Row2[i]);
        for (j = 0; j < RowSize; j++) {
	    fprintf(stderr, " %3.1f", fabs(RowMatch -> PenaltyMatrix[i][j]));
        }
    }
#endif /* DEBUG_USER_DITHER3D_DUMP_PENALYY */

#ifdef USER_DITHER3D_HEURISTIC_MATCH
    /* Use simple non optimal heuristic algorithm to match bi-partite graph. */
    User3DDitherHeuristicMatch(RowSize, MatchWidth, RowMatch);

    Penalty = User3DDitherAccumPenalty(RowSize, RowMatch);
#else
    {
        /* Use Optimal bipartite graph matching. */
        IritBiPrWeightedMatchStruct *Match;

#	ifdef DEBUG       /* Verify optimal is indeed better than heuristic. */
            IrtRType HeuPenalty;

            User3DDitherHeuristicMatch(RowSize, MatchWidth, RowMatch);

	    HeuPenalty = User3DDitherAccumPenalty(RowSize, RowMatch);
#	endif /* DEBUG */

	Match = IritMalloc(sizeof(IritBiPrWeightedMatchStruct) * RowSize);

	User3dDitherReport("Conducting matching", -1.0);

	j = MiscBiPrWeightedMatchBipartite((const IrtRType **) 
					            RowMatch -> PenaltyMatrix,
					   Match, RowSize);
	assert(j == 0);				     /* Successful matching. */

	User3dDitherReport("End conducting matching", -1.0);


	/* Update the local data structures regarding the matching result. */
	for (i = 0; i < RowSize; i++) {
	    RowMatch -> Match1to2[i] = Match[i].m2;
	    RowMatch -> Match2to1[RowMatch -> Match1to2[i]] = i;
	}

	IritFree(Match);

	Penalty = User3DDitherAccumPenalty(RowSize, RowMatch);

#	ifdef DEBUG
	    if (Penalty > HeuPenalty)
	        fprintf(stderr, "dither3d error: Optimal match is worse than heuristic match (%f > %f)!\n",
			Penalty, HeuPenalty);
#	endif /* DEBUG */
    }
#endif /* USER_DITHER3D_HEURISTIC_MATCH */

    /* Compute the RowSize dithering matrices and errors to be used. */
    for (i = 0; i < RowSize; i++) {
        int Inten1 = RowMatch -> Row1[i],
	    Inten2 = RowMatch -> Row2[j = RowMatch -> Match1to2[i]];

	/* Seeks the closest dithering matrix that exists. */
	p = User3DDitherGetPenalty(Inten1, Inten2, DitherSize);

	if (p != 0) {
	    if (Inten1 < Inten2) {
	        if (p < 0) /* Horizontal move. */
		    Inten1 -= p;
		else       /* Vertical move. */
		    Inten2 -= p;
	    }
	    else {
	        if (p < 0) /* Horizontal move. */
		    Inten1 += p;
		else       /* Vertical move. */
		    Inten2 += p;
	    }

	    /* Verify that this position has a valid dithering matrix. */
	    assert(Inten1 >= 0 && Inten1 < RowSize &&
		   Inten2 >= 0 && Inten2 < RowSize &&
		   User3DDitherGetPenalty(Inten1, Inten2, DitherSize) == 0);
	}
	RowMatch -> Dither1Level[i] = Inten1;
	RowMatch -> Dither2Level[j] = Inten2;

	/* And now the intensity error. */
	RowMatch -> Error1[i] = RowMatch -> Row1Inten[i] -
	                        RowMatch -> Dither1Level[i] *
	                                    DitherIntensityDivide[DitherSize];
	RowMatch -> Error2[j] = RowMatch -> Row2Inten[j] -
	                        RowMatch -> Dither2Level[j] *
	                                    DitherIntensityDivide[DitherSize];
    }

    /* Compute and return resulting penalty. */
    return Penalty;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A simple heuristic algorithm to match two images.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   RowSize:     Length of vectors Row1 and Row2.                            *
*   MatchWidth:  Width to allow matching: between Row1[i] to Row2[i +/- k],  *
*                                                            k < MatchWidth. *
*   RowMatch:    All slots of this structure are to be updated by this func. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void User3DDitherHeuristicMatch(int RowSize,
				       int MatchWidth,
				       User3DDitherRowMatchingStruct *RowMatch)
{
    int i, NumIters;

    /* Initialize a trivial identity match of Row1[i] to Row2[i]. */
    for (i = 0; i < RowSize; i++) {
        RowMatch -> Match1to2[i] = i;
	RowMatch -> Dither1Level[i] = RowMatch -> Dither2Level[i] = 255;
    }

    if (MatchWidth > 1) {
        /* Shuffle around the trivial identity matching. */
        for (i = 0; i < RowSize; i++) {
	    int i1i,
	        i1 = (int) IritRandom(0, RowSize - IRIT_EPS);

	    for (i1i = -MatchWidth; i1i <= MatchWidth; i1i++) {
	        int i2 = IRIT_BOUND(i1 + i1i, 0, RowSize - 1),
		    j1 = RowMatch -> Match1to2[i1],
		    j2 = RowMatch -> Match1to2[i2];

		/* See if we can swap i and j. */
		if (IRIT_ABS(j2 - j1) > MatchWidth ||
		    IRIT_ABS(j2 - i1) > MatchWidth ||
		    IRIT_ABS(j1 - i2) > MatchWidth)
		    continue;

		RowMatch -> Match1to2[i1] = j2;
		RowMatch -> Match1to2[i2] = j1;
	    }
	}
    }

    /* Update the reciprocal matched direction. */
    for (i = 0; i < RowSize; i++)
        RowMatch -> Match2to1[RowMatch -> Match1to2[i]] = i;

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO
    User3DDitherDebugDump(RowSize, RowMatch, TRUE);
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO */

    /* Now traverse the data structure and try to improve. */
    for (NumIters = MatchWidth; NumIters-- > 0; ) {
        int i1, i1i, j1, j1j,
	    Swap = FALSE;

	/* First aim from Row1 to Row2. */
        for (i1 = 0; i1 < RowSize; i1++) {
	    for (i1i = -MatchWidth; i1i <= MatchWidth; i1i++) {
	        int i2 = IRIT_BOUND(i1 + i1i, 0, RowSize - 1),
		    j1 = RowMatch -> Match1to2[i1],
		    j2 = RowMatch -> Match1to2[i2];

		/* Make sure new indices are still within MatchWidth: */
		if (IRIT_ABS(j2 - j1) > MatchWidth ||
		    IRIT_ABS(j2 - i1) > MatchWidth ||
		    IRIT_ABS(j1 - i2) > MatchWidth)
		    continue;

		/* Check if we can improve by swapping the edges. */
		if (RowMatch -> PenaltyMatrix[i1][j1] +
		    RowMatch -> PenaltyMatrix[i2][j2] -
		    RowMatch -> PenaltyMatrix[i1][j2] -
		    RowMatch -> PenaltyMatrix[i2][j1] > 0) {
		    RowMatch -> Match1to2[i1] = j2;
		    RowMatch -> Match1to2[i2] = j1;
		    RowMatch -> Match2to1[j1] = i2;
		    RowMatch -> Match2to1[j2] = i1;
		    Swap = TRUE;
		}
	    }
	}

	/* Symmetric aim from Row2 to Row1. */
        for (j1 = 0; j1 < RowSize; j1++) {
	    for (j1j = -MatchWidth; j1j <= MatchWidth; j1j++) {
	        int j2 = IRIT_BOUND(j1 + j1j, 0, RowSize - 1),
		    i1 = RowMatch -> Match2to1[j1],
		    i2 = RowMatch -> Match2to1[j2];

		/* Make sure new indices are still within MatchWidth: */
		if (IRIT_ABS(i2 - i1) > MatchWidth ||
		    IRIT_ABS(j2 - i1) > MatchWidth ||
		    IRIT_ABS(j1 - i2) > MatchWidth)
		    continue;

		/* Check if we can improve by swapping the edges. */
		if (RowMatch -> PenaltyMatrix[i1][j1] +
		    RowMatch -> PenaltyMatrix[i2][j2] -
		    RowMatch -> PenaltyMatrix[i1][j2] -
		    RowMatch -> PenaltyMatrix[i2][j1] > 0) {
		    RowMatch -> Match1to2[i1] = j2;
		    RowMatch -> Match1to2[i2] = j1;
		    RowMatch -> Match2to1[j1] = i2;
		    RowMatch -> Match2to1[j2] = i1;
		    Swap = TRUE;
		}
	    }
	}

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO
	User3DDitherDebugDump(RowSize, RowMatch, FALSE);
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO */

	if (!Swap)
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns the penalty value of using the given two intensities.  A penalty *
* value of zero means a valid dithering matrix exists for this match.        *
*   A positive penalty means horizontal motion of that many steps is         *
* required to get to a valid matrix location.				     *
*   A negative penalty means vertical motion of that many steps is           *
* required to get to a valid matrix location.				     *
*   Note the direction (+/-) of the motion depends on X > Y or vice versa.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Inten1, Inten2: Two intensity levels of two pixels in two different rows.*
*   DitherSize:    2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Penalty between given two intensities.                         *
*****************************************************************************/
static int User3DDitherGetPenalty(int Inten1, int Inten2, int DitherSize)
{
    int p;

    switch (DitherSize) {
        case 2:
	    p = Dither2Imgs3DySize2Penalty[Inten1][Inten2];
	    assert(IRIT_ABS(Dither2Imgs3DySize2Penalty[Inten2][Inten1]) ==
		   IRIT_ABS(p));
	    break;
        case 3:
	    p = Dither2Imgs3DySize3Penalty[Inten1][Inten2];
	    assert(IRIT_ABS(Dither2Imgs3DySize3Penalty[Inten2][Inten1]) ==
		   IRIT_ABS(p));
	    break;
        case 4:
	    p = Dither2Imgs3DySize4Penalty[Inten1][Inten2];
	    assert(IRIT_ABS(Dither2Imgs3DySize4Penalty[Inten2][Inten1]) ==
		   IRIT_ABS(p));
	    break;
        default:
	    p = 0;
	    assert(0);
    }

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the accumulated penalty of this row.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   RowSize:     Length of vectors Row1 and Row2.                            *
*   RowMatch:    Matching slots to be dumped out.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	Accumulated penalty.                                         *
*****************************************************************************/
static IrtRType User3DDitherAccumPenalty(int RowSize,
					 User3DDitherRowMatchingStruct
					                            *RowMatch)
{
    int i;
    IrtRType p;

    for (i = 0, p = 0.0; i < RowSize; i++) {
        int j = RowMatch -> Match1to2[i];

	assert(RowMatch -> PenaltyMatrix[i][j] != -USER_3DDITHER_MAX_INT);

	p += RowMatch -> PenaltyMatrix[i][j];
    }

    return p;
}

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Dump information on the matching process.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   RowSize:     Length of vectors Row1 and Row2.                            *
*   RowMatch:    Matching slots to be dumped out.                            *
*   Init:        TRUE if initialize stage, FALSE if during iterations.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void User3DDitherDebugDump(int RowSize,
				  User3DDitherRowMatchingStruct *RowMatch,
				  int Init)
{
    int i;

    fprintf(stderr, "\nCOMPUTED MATCHING (Penalty = %d) %s:",
	    User3DDitherAccumPenalty(RowSize, RowMatch),
	    Init ? "Initialization" : "");

#ifdef DEBUG_USER_DITHER3D_DUMP_INFO2
    for (i = 0; i < RowSize; i++) {
        int j;

	if (i % 6 == 0)
	    fprintf(stderr, "\n");
	j = RowMatch -> Match1to2[i];
        fprintf(stderr, "%2d> %2d (%2d)  ", i, j, RowMatch -> Match2to1[j]);
	assert(RowMatch -> Match2to1[j] == i);
    }
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO2 */
}
#endif /* DEBUG_USER_DITHER3D_DUMP_INFO */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds small sub pixels' shifts to the micro pixels in the XY direction.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Vrtcs:      Pixels to shift a tad, in place.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: Translated pixels, in place.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DDitherSetXYTranslations                                            M
*****************************************************************************/
IPVertexStruct *User3DDitherSetXYTranslations(IPVertexStruct *Vrtcs)
{
    int i, n, Base;
    IrtRType t;
    IPVertexStruct *V, *VLast, **VVec;

    /* Sort all input vertices according to Z. */
    if ((n = IPVrtxListLen(Vrtcs)) == 0)
	return NULL;
    VVec = IritMalloc(sizeof(IPVertexStruct *) * n);
    for (V = Vrtcs, i = 0; V != NULL; V = V -> Pnext)
        VVec[i++] = V;
    assert(i == n);
    qsort(VVec, n, sizeof(IPVertexStruct *), User3DDitherSortZ);

    /* Translated each Z level alternatingly, in different Z level. */
    t = 0.0;
    Base = 0;
    do {
	for (i = Base ; i < n; i++) {
	    VVec[i] -> Coord[0] += t;
	    VVec[i] -> Coord[1] += t;
	    if (VVec[i] -> Coord[2] - VVec[Base] -> Coord[2] > 0.5)
	        break;
	}

	t = t == 0.0 ? 0.25 : 0.0;
	Base = i;
    }
    while (i < n);
 
    /* Chain the vertices' list back. */
    V = VLast = VVec[0];
    for (i = 1; i < n; i++) {
        VLast -> Pnext = VVec[i];
	VLast = VVec[i];
    }
    VLast -> Pnext = NULL; 

    IritFree(VVec);

    assert(n == IPVrtxListLen(V));

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert the matching result into actual 3D dithering.   Returned is a    *
* list of the center points of the micro-pixels of the dithering matrices.   *
*                                                                            *
* PARAMETERS:                                                                *
*   ZLevel:      Current Z level, or current proceeded pair of rows.         *
*   RowSize:     Size of the rows of the two input images.                   *
*   DitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.              *
*   RowMatch:    All data structure of matching information.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:  List of points, each of which is the center location  *
*                      of a micro-blob in a 3D dithering matrix.	     *
*****************************************************************************/
static IPVertexStruct *User3DDitherGetMicroPixels(
				   int ZLevel,
				   int RowSize,
				   int DitherSize,
				   User3DDitherRowMatchingStruct *RowMatch)
{
    int q;
    IPVertexStruct
        *Vrtcs = NULL;

    for (q = 0; q < RowSize; q++) {
        const unsigned char *p;
        int i, j, k, m, XBase, YBase, ZBase,
	    Inten1 = RowMatch -> Dither1Level[q],
	    Inten2 = RowMatch -> Dither2Level[RowMatch -> Match1to2[q]];
	IPVertexStruct *V;

	/* Fetch the dithering matrix. */
	switch (DitherSize) {
            case 2:
	        p = Dither2Imgs3DSize2Matrices[Inten1][Inten2];
		break;
            case 3:
	        p = Dither2Imgs3DSize3Matrices[Inten1][Inten2];
		break;
            case 4:
	        p = Dither2Imgs3DSize4Matrices[Inten1][Inten2];
		break;
            default:
		p = NULL;
	        assert(0);
	}

#	ifdef DEBUG
        /* Verify the matrix. */
	for (k = m = 0; k < DitherSize; k++) {
	    for (i = 0; i < DitherSize; i++) {
	        for (j = 0; j < DitherSize; j++) {
		    if (p[i + j * DitherSize + k * IRIT_SQR(DitherSize)] != 0) {
		        m++;
			break;
		    }
		}
	    }
	}
	assert(m == Inten1);
	for (k = m = 0; k < DitherSize; k++) {
	    for (j = 0; j < DitherSize; j++) {
	        for (i = 0; i < DitherSize; i++) {
		    if (p[i + j * DitherSize + k * IRIT_SQR(DitherSize)] != 0) {
		        m++;
			break;
		    }
		}
	    }
	}
	assert(m == Inten2);
#	endif /* DEBUG */

	XBase = q * DitherSize;
	YBase = RowMatch -> Match1to2[q] * DitherSize;
	ZBase = ZLevel * DitherSize;

	for (k = m = 0; k < DitherSize; k++) {
	    for (j = 0; j < DitherSize; j++) {
	        for (i = 0; i < DitherSize; i++) {
		    if (p[m++] != 0) {
		        V = IPAllocVertex2(Vrtcs);
			Vrtcs = V;

			V -> Coord[0] = XBase + i;
			V -> Coord[1] = YBase + j;
			V -> Coord[2] = ZBase + k;
		    }
		}
	    }
	}
    }

    return Vrtcs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert the matching result into actual 3D dithering.   Returned is a    *
* list of polygonal blobs of the micro-voxels of the 3D dithering matrices.  *
*                                                                            *
* PARAMETERS:                                                                *
*   ZLevel:      Current Z level, or current proceeded pair of rows.         *
*   RowSize:     Size of the rows of the two input images.                   *
*   DitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.              *
*   SphereRad:   Radius of construct spherical blob.			     *
*   RowMatch:    All data structure of matching information.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  List of spherical blob objects.			     *
*****************************************************************************/
static IPObjectStruct *User3DDitherGetMicroBlobs(
				   int ZLevel,
				   int RowSize,
				   int DitherSize,
				   IrtRType SphereRad,
				   User3DDitherRowMatchingStruct *RowMatch)
{
    IPVertexStruct *Vrtx,
        *Vrtcs = User3DDitherGetMicroPixels(ZLevel, RowSize,
					    DitherSize, RowMatch);
    IPObjectStruct *PTmp,
	*PRetVal = IPGenPOLYObject(IPAllocPolygon(0, NULL, NULL));

    assert(SphereRad > 0);

    for (Vrtx = Vrtcs; Vrtx != NULL; Vrtx = Vrtx -> Pnext) {
        PTmp = PrimGenSPHEREObject(Vrtx -> Coord, SphereRad);
	PRetVal -> U.Pl = IPAppendPolyLists(PTmp -> U.Pl, PRetVal -> U.Pl);
	PTmp -> U.Pl = NULL;
	IPFreeObject(PTmp);
    }

    return PRetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add additional micropixels to improve the contrast behind existing       *
* pixels, in place.				                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Vrtcs:           List of micropixels we current have.  Updated in place. *
*   RowSize:         Size of the rows of the two input images.               *
*   DitherSize:      2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.          *
*   MatchWidth:      How much can we escape off diagonal?                    *
*   AugmentContrast:  Number of micro-pixels to augment the contrast,        *
*                     behind existing pixels.  			             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:    Augmented list of micro pixels.                     *
*                        Returned as a single list of points.                *
*****************************************************************************/
static IPVertexStruct *User3DDitherAugmentContrast(IPVertexStruct *Vrtcs,
						   int RowSize,
						   int DitherSize,
						   int MatchWidth,
						   int AugmentContrast)
{
    int i, j, n, m, Base,
        DitheredRowSize = RowSize * DitherSize;
    IrtBType
        *XCover = IritMalloc(sizeof(IrtBType) * DitheredRowSize),
        *YCover = IritMalloc(sizeof(IrtBType) * DitheredRowSize),
        *PixelCover = IritMalloc(sizeof(IrtBType) * IRIT_SQR(DitheredRowSize));
    IrtRType Dz;
    IPVertexStruct *V, *VLast, **VVec;

    /* Sort all input vertices according to Z. */
    n = IPVrtxListLen(Vrtcs);
    VVec = IritMalloc(sizeof(IPVertexStruct *) * n 
					       * AugmentContrast * DitherSize);
    for (V = Vrtcs, i = 0; V != NULL; V = V -> Pnext)
        VVec[i++] = V;
    assert(i == n);
    qsort(VVec, n, sizeof(IPVertexStruct *), User3DDitherSortZ);

    /* Process each Z level separately. Dz will be our Z level separator.    */
    Dz = (VVec[n - 1] -> Coord[2] - VVec[0] -> Coord[2]) / (DitherSize * 2.0);
    Base = 0;
    m = n;
    do {
	for (i = Base; i < n; i++) {
	    if (VVec[i] -> Coord[2] - VVec[Base] -> Coord[2] > Dz)
	        break;
	}
	i = IRIT_MIN(i, n - 1);

	/* Process micro pixels between Base & i - all in same Z level.      */
	/* Accumulate what is covered in X, Y, and in the XY plane.          */
	IRIT_ZAP_MEM(XCover, sizeof(IrtBType) * DitheredRowSize);
	IRIT_ZAP_MEM(YCover, sizeof(IrtBType) * DitheredRowSize);
	IRIT_ZAP_MEM(PixelCover, sizeof(IrtBType) * IRIT_SQR(DitheredRowSize));

	for (j = Base; j < i; j++) {
	    int x = (int) (VVec[j] -> Coord[0] + 0.5),
	        y = (int) (VVec[j] -> Coord[1] + 0.5);

	    if (x >= 0 && x < DitheredRowSize &&
		y >= 0 && y < DitheredRowSize) {
	        XCover[x]++;
		YCover[y]++;
		PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)]++;
	    }
	}

#ifdef DEBUG_USER_DITHER3D_AUGMENTED_INFO
        {
	    int i, Coverage[11];

	    IRIT_ZAP_MEM(Coverage, sizeof(int) * 11);
	    for (i = 0; i < DitheredRowSize; i++) {
	        if (XCover[i] > 0) {
		    if (XCover[i] < 10)
		        Coverage[XCover[i]]++;
		    else
		        Coverage[10]++;
		}
		if (YCover[i] > 0) {
		    if (YCover[i] < 10)
		        Coverage[YCover[i]]++;
		    else
		        Coverage[10]++;
		}
	    }
	    fprintf(stderr, "Before coverage (Z=%lg): %d %d %d %d %d %d %d %d %d %d %d\n",
		    VVec[Base] -> Coord[2],
		    Coverage[0], Coverage[1], Coverage[2], Coverage[3],
		    Coverage[4], Coverage[5], Coverage[6], Coverage[7],
		    Coverage[8], Coverage[9], Coverage[10]);
        }
#endif /* DEBUG_USER_DITHER3D_AUGMENTED_INFO */

	/* Now for each pair of micro-pixels (x1, y1) and (x2, y2) in the    */
	/* sort list add two new points (x1, y2) and (x2, y1).               */
	V = User3DDitherAugmentContrastAux1(XCover, YCover, PixelCover,
					    RowSize, DitherSize, MatchWidth,
					    &VVec[Base], i - Base);
	for ( ; V != NULL; V = V -> Pnext)
	    VVec[m++] = V;

	/* Add more micro pixels behind existing pixels. */
	V = User3DDitherAugmentContrastAux2(
			XCover, YCover, PixelCover, RowSize, DitherSize,
		        MatchWidth, VVec[Base] -> Coord[2], AugmentContrast);
	for ( ; V != NULL; V = V -> Pnext)
	    VVec[m++] = V;

	Base = i;
    }
    while (Base < n - 1);

    assert(m <= n * AugmentContrast * DitherSize);

    /* Filter duplicates while chaining vertices back into a linked list. */
    qsort(VVec, m, sizeof(IPVertexStruct *), User3DDitherSortXY);

    V = VLast = VVec[0];
    for (i = 1; i < m; i++) {
        if (IRIT_PT_APX_EQ(VLast -> Coord, VVec[i] -> Coord)) {
	    IPFreeVertex(VVec[i]);
	}
	else {
	    VLast -> Pnext = VVec[i];
	    VLast = VVec[i];
	}
    }
    VLast -> Pnext = NULL;

    IritFree(VVec);
    IritFree(XCover);
    IritFree(YCover);
    IritFree(PixelCover);

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create new pixels behind existing pixels to augment the contrast of the  *
* expected result.  New pixels are added by finding close (x1, y1) and       *
* (x2, y2) pixels, adding new (x1, y2) and (x2, y1) pixels.		     *
*   An auxiliary function of User3DDitherAugmentContrast.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   XCover:       What pixels are covered in X and how many times.           *
*   YCover:       What pixels are covered in Y and how many times.           *
*   PixelCover:   Where are the pixels in the XY plane.			     *
*   RowSize:      Size of  a row in the input images.			     *
*   DitherSize:   Dithering width allowed.				     *
*   MatchWidth:   Width to allow matching in a row:			     *
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. *
*   VVec:	  Vector holding all current pixels.                         *
*   VVecLen:	  Length of VVec vector.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:     New pixels to augment existing locations.          *
*****************************************************************************/
static IPVertexStruct *User3DDitherAugmentContrastAux1(IrtBType *XCover,
						       IrtBType *YCover,
						       IrtBType *PixelCover,
						       int RowSize,
						       int DitherSize,
						       int MatchWidth,
						       IPVertexStruct **VVec,
						       int VVecLen)
{
    int j,
        DitheredRowSize = RowSize * DitherSize,
	DitheredMatchWidth = MatchWidth * DitherSize;
    IPVertexStruct *V,
        *VNew = NULL;

    qsort(VVec, VVecLen, sizeof(IPVertexStruct *), User3DDitherSortXY);

    for (j = 0; j + 1 < VVecLen; j++) {
        if (IRIT_ABS(VVec[j] -> Coord[0] -
		     VVec[j + 1] -> Coord[1]) < DitheredMatchWidth) {
	    int x = (int) (VVec[j] -> Coord[0] + 0.5),
	        y = (int) (VVec[j + 1] -> Coord[1] + 0.5);

	    if (x >= 0 && x < DitheredRowSize &&
		y >= 0 && y < DitheredRowSize &&
		PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] == 0) {
	        /* Add a new pixels at (x, y). */
	        PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] = 1;
		assert(XCover[x] > 0 && YCover[y] > 0);
	        XCover[x]++;
		YCover[y]++;

		V = IPAllocVertex2(VNew);
		VNew = V;
		V -> Coord[0] = VVec[j] -> Coord[0];
		V -> Coord[1] = VVec[j + 1] -> Coord[1];
		V -> Coord[2] = VVec[j] -> Coord[2];
	    }
	}

	if (IRIT_ABS(VVec[j + 1] -> Coord[0] -
		     VVec[j] -> Coord[1]) < DitheredMatchWidth) {
	    int x = (int) (VVec[j + 1] -> Coord[0] + 0.5),
	        y = (int) (VVec[j] -> Coord[1] + 0.5);

	    if (x >= 0 && x < DitheredRowSize &&
		y >= 0 && y < DitheredRowSize &&
		PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] == 0) {
	        /* Add a new pixels at (x, y). */
	        PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] = 1;
		assert(XCover[x] > 0 && YCover[y] > 0);
		XCover[x]++;
		YCover[y]++;

		V = IPAllocVertex2(VNew);
		VNew = V;
		V -> Coord[0] = VVec[j + 1] -> Coord[0];
		V -> Coord[1] = VVec[j] -> Coord[1];
		V -> Coord[2] = VVec[j] -> Coord[2];
	    }
	}
    }

    return VNew;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create new pixels behind existing pixels to augment the contrast of the  *
* expected result.  New pixels are added along X-rows that are X-covered and *
* searching for Y-covering, and vice versa.				     *
*   An auxiliary function of User3DDitherAugmentContrast.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   XCover:           What pixels are covered in X and how many times.       *
*   YCover:           What pixels are covered in Y and how many times.       *
*   PixelCover:       Where are the pixels in the XY plane.		     *
*   RowSize:          Size of  a row in the input images.		     *
*   DitherSize:       Dithering width allowed.				     *
*   MatchWidth:       Width to allow matching in a row:			     *
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. *
*   ZLevel:	      Z level of this augmentation.			     *
*   AugmentContrast:  What is the desired level of augmented coverage (how   *
*                     many times each pixel better be covered.)              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:     New pixels to augment existing locations.          *
*****************************************************************************/
static IPVertexStruct *User3DDitherAugmentContrastAux2(IrtBType *XCover,
						       IrtBType *YCover,
						       IrtBType *PixelCover,
						       int RowSize,
						       int DitherSize,
						       int MatchWidth,
						       IrtRType ZLevel,
						       int AugmentContrast)
{
    int i, x, y, x1, y1,
        DitheredRowSize = RowSize * DitherSize,
        DitheredMatchWidth = MatchWidth * DitherSize,
        *XOrder = User3DMicroBlobsCreateRandomVector(DitheredRowSize,
					         USER_3D_SPREAD_RANDOM, TRUE),
        *YOrder = User3DMicroBlobsCreateRandomVector(DitheredRowSize,
					         USER_3D_SPREAD_RANDOM, TRUE),
        *Match = User3DMicroBlobsCreateRandomVector(DitheredMatchWidth * 2,
					         USER_3D_SPREAD_RANDOM, TRUE);
    IPVertexStruct *V,
        *VNew = NULL;

    for (i = 0; i < DitheredRowSize; i++) {
        x = XOrder[i];
	/* If this X pixel is covered but not covered enough, augment: */
	if (XCover[x] > 0 && XCover[x] < AugmentContrast) {
	    for (y1 = 0; y1 < DitheredMatchWidth * 2; y1++) {
		y = IRIT_BOUND(x + Match[y1] - DitheredMatchWidth,
			       0, DitheredRowSize - 1);
		assert(IRIT_ABS(x - y) <= DitheredMatchWidth);

	        if (YCover[y] > 0 &&
		    YCover[y] < AugmentContrast &&
		    PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] == 0) {
		    /* Add a new pixel at (x, y). */
		    PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] = 1;
		    assert(XCover[x] > 0 && YCover[y] > 0);
		    XCover[x]++;
		    YCover[y]++;

		    V = IPAllocVertex2(VNew);
		    VNew = V;
		    V -> Coord[0] = x;
		    V -> Coord[1] = y;
		    V -> Coord[2] = ZLevel;
		}

		if (XCover[x] >= AugmentContrast)
		    break;
	    }
	}

	y = YOrder[i];
	/* If this X pixel is covered but not covered enough, augment: */
	if (YCover[y] > 0 && YCover[y] < AugmentContrast) {
	    for (x1 = 0; x1 < DitheredMatchWidth * 2; x1++) {
		x = IRIT_BOUND(Match[x1] + y - DitheredMatchWidth,
			       0, DitheredRowSize - 1);
		assert(IRIT_ABS(x - y) <= DitheredMatchWidth);

		if (XCover[x] > 0 &&
		    XCover[x] < AugmentContrast &&
		    PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] == 0) {
		    /* Add a new pixel at (x, y). */
		    PixelCover[USER_3DDITHER_PIXEL_COORD(x, y)] = 1;
		    assert(XCover[x] > 0 && YCover[y] > 0);
		    XCover[x]++;
		    YCover[y]++;

		    V = IPAllocVertex2(VNew);
		    VNew = V;
		    V -> Coord[0] = x;
		    V -> Coord[1] = y;
		    V -> Coord[2] = ZLevel;
		}

		if (YCover[y] >= AugmentContrast)
		    break;
	    }
	}
    }

#ifdef DEBUG_USER_DITHER3D_AUGMENTED_INFO
    {
        int i, Coverage[11];

	IRIT_ZAP_MEM(Coverage, sizeof(int) * 11);
	for (i = 0; i < DitheredRowSize; i++) {
	    if (XCover[i] > 0) {
	        if (XCover[i] < 10)
		    Coverage[XCover[i]]++;
		else
		    Coverage[10]++;
	    }
	    if (YCover[i] > 0) {
	        if (YCover[i] < 10)
		    Coverage[YCover[i]]++;
		else
		    Coverage[10]++;
	    }
	}
	fprintf(stderr, "After Coverage (Z=%lg): %d %d %d %d %d %d %d %d %d %d %d\n",
		ZLevel,
		Coverage[0], Coverage[1], Coverage[2], Coverage[3],
		Coverage[4], Coverage[5], Coverage[6], Coverage[7],
		Coverage[8], Coverage[9], Coverage[10]);
    }
#endif /* DEBUG_USER_DITHER3D_AUGMENTED_INFO */

    IritFree(XOrder);
    IritFree(YOrder);
    IritFree(Match);

    return VNew;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Construct an optimal match (minimal penalty) between pixels in Row1 and  *
* Row2.  Penalty is zero between Row1[i] and Row2[j] if there is a valid 3D  *
* dithering matrix between intensities Row1[i] and Row[j].  Otherwise,	     *
* penalty is following the penalty matrix of the respective dithering size.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Image1, Image2:   Two images to match and build 3D dithering for.        *
*   Width, Height:    The size of the two input images.                      *
*   DitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.              *
*   MatchWidth:  Width to allow matching in a row:			     *
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. *
*   RowMatch:    All slots of this structure are to be updated by this func. *
*   Negate:      TRUE to negate the images.				     *
*   AugmentContrast:  Number of micro-pixels to augment the contrast,        *
*                behind existing pixels.  Zero to disable.	             *
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random *
*                spread to use.						     *
*   SphereRad:   Radius of construct spherical blob, zero to return points.  *
*   AccumPenalty:  Returns the accumulated error in the dithering-matching,  *
*		 where zero designates no error.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      resulting minimal penalty.                                     *
*****************************************************************************/
static IPObjectStruct *User3DDither2ImagesAux(IrtImgPixelStruct *Image1,
					      IrtImgPixelStruct *Image2,
					      int Width,
					      int Height,
					      int DitherSize,
					      int MatchWidth,
					      int Negate,
					      int AugmentContrast,
					      User3DSpreadType SpreadMethod,
					      IrtRType SphereRad,
					      IrtRType *AccumPenalty)
{
    int i;
    User3DDitherRowMatchingStruct RowMatch;
    IrtImgPixelStruct *I1, *I2;
    IPObjectStruct *PObj, *PTmp;

    *AccumPenalty = 0.0;

    /* Build the row matching structure. */
    RowMatch.Match1to2 = (int *) IritMalloc(sizeof(int) * Width);
    RowMatch.Match2to1 = (int *) IritMalloc(sizeof(int) * Width);
    RowMatch.Error1 = (IrtRType *) IritMalloc(sizeof(IrtRType) * Width);
    RowMatch.Error2 = (IrtRType *) IritMalloc(sizeof(IrtRType) * Width);
    RowMatch.Row1 = (int *) IritMalloc(sizeof(int) * Width);
    RowMatch.Row2 = (int *) IritMalloc(sizeof(int) * Width);
    RowMatch.Row1Inten = (unsigned char *)
                                   IritMalloc(sizeof(unsigned char) * Width);
    RowMatch.Row2Inten = (unsigned char *)
                                   IritMalloc(sizeof(unsigned char) * Width);
    RowMatch.Dither1Level = (unsigned char *)
                                   IritMalloc(sizeof(unsigned char) * Width);
    RowMatch.Dither2Level = (unsigned char *)
                                   IritMalloc(sizeof(unsigned char) * Width);
    RowMatch.PenaltyMatrix = (IrtRType **)
                                     IritMalloc(sizeof(IrtRType *) * Height);
    for (i = 0; i < Height; i++)
        RowMatch.PenaltyMatrix[i] = (IrtRType *)
	                                IritMalloc(sizeof(IrtRType) * Width);
    IRIT_ZAP_MEM(RowMatch.Error1, sizeof(IrtRType) * Width);
    IRIT_ZAP_MEM(RowMatch.Error2, sizeof(IrtRType) * Width);

    if (SphereRad <= 0.0) {
        /* Accumulate points. */
        PObj = IPGenPOINTLISTObject(IPAllocPolygon(0, NULL, NULL));
    }
    else {
        /* Accumulate spherical blobs. */
        PObj = IPGenPOLYObject(NULL);
    }

    for (i = 0, I1 = Image1, I2 = Image2;
	 i < Height;
	 i++, I1 += Width, I2 += Width) {
        char Line[IRIT_LINE_LEN];

        sprintf(Line, "Handling line %d", i);
        User3dDitherReport(Line, i / (Height - 1.0));

        *AccumPenalty += User3DDither2Rows(I1, I2, Width, DitherSize,
					   MatchWidth, Negate,
					   SpreadMethod, &RowMatch);
        User3dDitherReport("Done matching", -1.0);

	/* Build the geometry (blobs) of this row. */
	if (SphereRad <= 0.0) {
	    IPVertexStruct
	        *Vrtcs = User3DDitherGetMicroPixels(i, Width, DitherSize,
						    &RowMatch); /* Accum. V. */

	    if (Vrtcs != NULL) {
	        if (AugmentContrast > 0)
		    Vrtcs = User3DDitherAugmentContrast(Vrtcs, Width, DitherSize,
							MatchWidth,
							AugmentContrast);

		Vrtcs = User3DDitherSetXYTranslations(Vrtcs);

		PObj -> U.Pl -> PVertex = 
		    IPAppendVrtxLists(Vrtcs, PObj -> U.Pl -> PVertex);
	    }
	}
	else {
	    /* Accumulate spherical blobs. */
	    PTmp = User3DDitherGetMicroBlobs(i, Width, DitherSize, SphereRad,
					     &RowMatch);

	    /* A simple linear list of polygons. */
	    PObj -> U.Pl = IPAppendPolyLists(PTmp -> U.Pl, PObj -> U.Pl);
	    PTmp -> U.Pl = NULL;
	    IPFreeObject(PTmp);
	}
    }

    for (i = 0; i < Height; i++)
        IritFree(RowMatch.PenaltyMatrix[i]);
    IritFree(RowMatch.PenaltyMatrix);
    IritFree(RowMatch.Match2to1);
    IritFree(RowMatch.Match1to2);
    IritFree(RowMatch.Error1);
    IritFree(RowMatch.Error2);
    IritFree(RowMatch.Row1);
    IritFree(RowMatch.Row2);
    IritFree(RowMatch.Row1Inten);
    IritFree(RowMatch.Row2Inten);
    IritFree(RowMatch.Dither1Level);
    IritFree(RowMatch.Dither2Level);

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Build a 3D models consisting of spherical blobs that looks like the 1st  M
* image (gray level) from the XZ plane and like the 2nd image from the YZ    M
* plane.   The entire constructed geometry is confined to a cube world       M
* space of [max(ImageWidth, ImageHeight)]^3.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Image1Name:  Name of 1st image to load.                                  M
*   Image2Name:  Name of 2nd image to load.                                  M
*   DitherSize:  2, 3 or 4 for (2x2), (3x3) or (4x4) dithering.              M
*   MatchWidth:  Width to allow matching in a row:			     M
*                           between pos[i] to pos[i +/- k],  k < MatchWidth. M
*   Negate:      TRUE to negate the images.				     M
*   AugmentContrast:  Number of iterations to add micro-pixels, to augment   M
*                the contrast, behind existing pixels.  Zero to disable.     M
*   SpreadMethod: If allowed (MatchWidth >= RowSize), selects initial random M
*                spread to use.						     M
*   SphereRad:   Radius of construct spherical blob, zero to return points.  M
*   AccumPenalty:  Returns the accumulated error in the dithering-matching,  M
*		 where zero designates no error.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Micro blobs if SphereRad > 0, center points, if = 0.  M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   User3DDither2Images                                                      M
*****************************************************************************/
IPObjectStruct *User3DDither2Images(const char *Image1Name,
				    const char *Image2Name,
				    int DitherSize,
				    int MatchWidth,
				    int Negate,
				    int AugmentContrast,
				    User3DSpreadType SpreadMethod,
				    IrtRType SphereRad,
				    IrtRType *AccumPenalty)
{
    int Img1MaxX, Img1MaxY, Img1Alpha, Img2MaxX, Img2MaxY, Img2Alpha;
    IrtImgPixelStruct
        *Img1 = IrtImgReadImage(Image1Name, &Img1MaxX, &Img1MaxY, &Img1Alpha),
	*Img2 = IrtImgReadImage(Image2Name, &Img2MaxX, &Img2MaxY, &Img2Alpha);
    IPObjectStruct *PObj;

    IritRandomInit(301060);

    if (Img1MaxX != Img2MaxX || Img1MaxY != Img2MaxY) {
	IritFree(Img1);
	IritFree(Img2);
        return NULL;
    }

    PObj = User3DDither2ImagesAux(Img1, Img2, Img1MaxX + 1, Img1MaxY + 1,
				  DitherSize, MatchWidth,
				  Negate, AugmentContrast, SpreadMethod,
				  SphereRad, AccumPenalty);

    IritFree(Img1);
    IritFree(Img2);

    return PObj;
}

#ifdef DEBUG_USER_DITHER_3D

main()
{
    IrtRType Penalty;
    IPObjectStruct *PObj;

    PrimSetResolution(2);

    PObj = User3DDither2Images(//"Herzel150.gif", "BenGurion150.gif",
			       //"Herzel70.gif", "Herzel70.gif",
			       "BenGurion150.gif", "Herzel150.gif",
			       3, 3, 0, 1, 0, USER_3D_SPREAD_RANDOM,
			       0.5, &Penalty);

    IPStdoutObject(PObj, FALSE);
}

#endif /* DEBUG_USER_DITHER_3D */
