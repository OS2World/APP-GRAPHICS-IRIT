/******************************************************************************
* Handles searching routines.                                                 *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*					  Written by Gershon Elber, Sep. 2001 *
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_loc.h"

/* #define DEBUG_MAIN_SEARCH_TEST   - To test this file in stand alone mode. */
#ifdef DEBUG_MAIN_SEARCH_TEST
#define IritMalloc malloc
#define IritFree free
#endif /* DEBUG_MAIN_SEARCH_TEST */

#define IRIT_SRCH2D_MAX_DIM	256

typedef struct IritSearch2DElementStruct {
    struct IritSearch2DElementStruct *Pnext;
    IrtRType X, Y;
    char Data[1];  /* We allocate more than one char, as need, to keep data. */
} IritSearch2DElementStruct;

typedef struct IritSearch2DStruct {
    int GridXSize, GridYSize, DataSize;
    IrtRType XMin, XMax, YMin, YMax, DxInv, DyInv, Tol;
    IritSearch2DElementStruct ***Grid;
} IritSearch2DStruct;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize the search data structure.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   XMin, XMax, YMin, YMax:  Dimensions of 2D domain.                        M
*   Tol:                     Tolerance of expected search.	             M
*   DataSize:		  Size, in bytes, of expected data elements to keep. M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    An internal auxiliary structure to be provided to the        M
*		insertion and searching routines, NULL if error.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSearch2DInsertElem, IritSearch2DFindElem, IritSearch2DFree           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSearch2DInit                                                         M
*****************************************************************************/
VoidPtr IritSearch2DInit(IrtRType XMin,
			 IrtRType XMax,
			 IrtRType YMin,
			 IrtRType YMax,
			 IrtRType Tol,
			 int DataSize)
{
    int XSize, YSize, i;
    IritSearch2DElementStruct ***Grid;
    IritSearch2DStruct
	*Search2D = (IritSearch2DStruct *)
				       IritMalloc(sizeof(IritSearch2DStruct));

    Search2D -> XMin = XMin;
    Search2D -> XMax = XMax;
    Search2D -> YMin = YMin;
    Search2D -> YMax = YMax;
    Search2D -> Tol = Tol;
    Search2D -> DataSize = DataSize;

    if (XMax <= XMin || YMax <= YMin) {
        IritFree(Search2D);
	return NULL;
    }

    XSize = (int) ((XMax - XMin) / Tol);
    XSize = IRIT_BOUND(XSize, 1, IRIT_SRCH2D_MAX_DIM);
    Search2D -> GridXSize = XSize;

    YSize = (int) ((YMax - YMin) / Tol);
    YSize = IRIT_BOUND(YSize, 1, IRIT_SRCH2D_MAX_DIM);
    Search2D -> GridYSize = YSize;

    Search2D -> DxInv = XSize / (XMax - XMin);
    Search2D -> DyInv = YSize / (YMax - YMin);

    Search2D -> Grid = Grid = (IritSearch2DElementStruct ***)
		     IritMalloc(YSize * sizeof(IritSearch2DElementStruct **));
    Grid[0] = (IritSearch2DElementStruct **)
	      IritMalloc(XSize * YSize * sizeof(IritSearch2DElementStruct *));

    IRIT_ZAP_MEM(Grid[0], XSize * YSize * sizeof(IritSearch2DElementStruct *));

    for (i = 1; i < YSize; i++) {
        Grid[i] = &Grid[i - 1][XSize];
    }

    return Search2D;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free auxiliary data structure aiding in 2D search.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   S2D:  Internal data structure, aiding search, to be freed.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSearch2DInsertElem, IritSearch2DFindElem, IritSearch2DInit           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSearch2DFree                                                         M
*****************************************************************************/
void IritSearch2DFree(VoidPtr S2D)
{
    int j, i;
    IritSearch2DStruct
	*Search2D = (IritSearch2DStruct *) S2D;

    for (j = 0; j < Search2D -> GridYSize; j++) {
	for (i = 0; i < Search2D -> GridXSize; i++) {
	    IritSearch2DElementStruct
	        *Elements = Search2D -> Grid[j][i];

	    while (Elements != NULL) {
	        IritSearch2DElementStruct
		    *NextElements = Elements -> Pnext;

		IritFree(Elements);
		Elements = NextElements;
	    }
	}
    }

    IritFree(Search2D -> Grid[0]);
    IritFree(Search2D -> Grid);
    IritFree(Search2D);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert a new element into the data structure.  No test is made if a      M
* similar element already exists.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   S2D:          Internal data structure, aiding search.		     M
*   XKey, YKey:   The new 2D coordinates to insert.			     M
*   Data:         A pointer to the data to save with this 2D coordinate key. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSearch2DFree, IritSearch2DFindElem, IritSearch2DInit                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSearch2DInsertElem                                                   M
*****************************************************************************/
void IritSearch2DInsertElem(VoidPtr S2D,
			    IrtRType XKey,
			    IrtRType YKey,
			    VoidPtr Data)
{
    IritSearch2DStruct
	*Search2D = (IritSearch2DStruct *) S2D;
    int x = (int) ((XKey - Search2D -> XMin) * Search2D -> DxInv),
        y = (int) ((YKey - Search2D -> YMin) * Search2D -> DyInv);
    IritSearch2DElementStruct
	*Element = (IritSearch2DElementStruct *)
		IritMalloc(sizeof(IritSearch2DElementStruct) +
			   Search2D -> DataSize);

    x = IRIT_BOUND(x, 0, Search2D -> GridXSize - 1);
    y = IRIT_BOUND(y, 0, Search2D -> GridYSize - 1);

    IRIT_GEN_COPY(Element -> Data, Data, Search2D -> DataSize);
    Element -> X = XKey;
    Element -> Y = YKey;

    IRIT_LIST_PUSH(Element, Search2D -> Grid[y][x]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Looks for an existing element in the data structure.  If found an        M
* element within prescribed tolerance, Data is updated with the saved data   M
*                                                                            *
* PARAMETERS:                                                                M
*   S2D:         Internal data structure, aiding search.		     M
*   XKey, YKey:  The 2D coordinates to search for.			     M
*   Data:        A pointer to the location to copy the found data into.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if element was found, FALSE otherwise.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSearch2DFree, IritSearch2DInsertElem, IritSearch2DInit               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSearch2DFindElem                                                     M
*****************************************************************************/
int IritSearch2DFindElem(VoidPtr S2D,
			 IrtRType XKey,
			 IrtRType YKey,
			 VoidPtr Data)
{
    VoidPtr FoundData;
    IritSearch2DStruct
	*Search2D = (IritSearch2DStruct *) S2D;
    int x1, y1, x2, y2,
        x = (int) ((XKey - Search2D -> XMin) * Search2D -> DxInv),
        y = (int) ((YKey - Search2D -> YMin) * Search2D -> DyInv);
    IrtRType MinSqrDist;

    /* Locate the local neighboorhood we must examine, taking into account */
    /* tolerance deviation (in the order of one cell width).		   */
    x1 = x - 1;
    y1 = y - 1;
    x2 = x + 1;
    y2 = y + 1;

    x1 = IRIT_BOUND(x1, 0, Search2D -> GridXSize - 1);
    y1 = IRIT_BOUND(y1, 0, Search2D -> GridYSize - 1);
    x2 = IRIT_BOUND(x2, 0, Search2D -> GridXSize - 1);
    y2 = IRIT_BOUND(y2, 0, Search2D -> GridYSize - 1);

    FoundData = NULL;
    MinSqrDist = IRIT_SQR(Search2D -> Tol);
    for (y = y1; y <= y2; y++) {
	for (x = x1; x <= x2; x++) {
	    IritSearch2DElementStruct *El,
		*Elements = Search2D -> Grid[y][x];

	    for (El = Elements; El != NULL; El = El -> Pnext) {
	        IrtRType
		    SqrDist = IRIT_SQR(XKey - El -> X) +
			      IRIT_SQR(YKey - El -> Y);

		if (MinSqrDist > SqrDist) {
		    MinSqrDist = SqrDist;
		    FoundData = El -> Data;
		}
	    }
	}
    }

    if (FoundData) {
        IRIT_GEN_COPY(Data, FoundData, Search2D -> DataSize);
	return TRUE;
    }
    else
        return FALSE;
}

#ifdef DEBUG_MAIN_SEARCH_TEST

/* Stand alone VC++ compilation line.
cl -DDEBUG_SEARCH_TEST_MAIN -DIRIT_HAVE_URT_RLE -DIRIT_HAVE_GIF_LIB  -Zi -Od  -D__WINNT__ -D__OPENGL__ -D_X86_=1 -DWIN32 -D_MT -DRAND -W3 -nologo -Ic:\c\urt\include -Ic:\c\giflib\lib -DDEBUG_IRIT_MALLOC -I. -Ic:\irit\irit\include -Fosearch.obj search.c
*/

#define NUM_ITEMS 100000

IrtRType DbgRandom(IrtRType Min, IrtRType Max)
{
    long R = rand();

    return Min + (Max - Min) * ((double) (R & RAND_MAX)) / ((double) RAND_MAX);
}
n
void main()
{
    int i;
    static IrtRType TestArray[NUM_ITEMS * 2][2], Data[2];
    VoidPtr S2D = IritSearch2DInit(0, 10, 0, 10, 0.001, sizeof(IrtRType) * 2);

    /* Insert (half) the data in. */
    for (i = 0; i < NUM_ITEMS * 2; i++) {
	TestArray[i][0] = DbgRandom(0, 10);
	TestArray[i][1] = DbgRandom(0, 10);
	if (i < NUM_ITEMS)
	    IritSearch2DInsertElem(S2D, TestArray[i][0], TestArray[i][1],
				   TestArray[i]);
    }

    /* Now lets see if we can find it. */
    IRIT_INFO_MSG("Searching items that must be inside...\n");
    for (i = 0; i < NUM_ITEMS; i++) {
	if (!IritSearch2DFindElem(S2D, TestArray[i][0], TestArray[i][1], Data))
	    IRIT_WARNING_MSG_PRINTF("Failed to find item %d\n", i);
	else {
	    if (TestArray[i][0] != Data[0] || TestArray[i][1] != Data[1])
	        IRIT_WARNING_MSG_PRINTF("Found item %d not identical\n", i);
	}
    }

    IRIT_INFO_MSG("\nSearching items that are not inside...\n");
    for (i = NUM_ITEMS; i < NUM_ITEMS * 2; i++) {
	if (IritSearch2DFindElem(S2D, TestArray[i][0], TestArray[i][1], Data))
	    IRIT_INFO_MSG_PRINTF("Found item %d: [%f %f], key = [%f %f]\n",
		       i, TestArray[i][0], TestArray[i][1], Data[0], Data[1]);
    }

    IRIT_INFO_MSG("\nDone.\n");
}

#endif /* DEBUG_MAIN_SEARCH_TEST */

