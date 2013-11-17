/*****************************************************************************
* Module to read STL files into IRIT data.		        	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, May 2005   *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "misc_lib.h"

#ifdef __WINNT__
#include <io.h>
#include <fcntl.h>
#endif /* __WINNT__ */

#define STL_REASONABLE_NUM_OF_POLYS 100000000

#define PT_FLT2DBL_COPY(DstPt, SrcPt) \
	DstPt[0] = SrcPt[0]; \
	DstPt[1] = SrcPt[1]; \
	DstPt[2] = SrcPt[2];

IRIT_STATIC_DATA int
    GlblStlLineNum = 0;

static void EndianSwapFloats(float *RP, int n);
static void EndianSwapLongInts(long int *IP, int n);
static void Stl2IritAbort(char *Str);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Read STL file into IRIT data.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   STLFileName:     Name of STL file.					     M
*   BinarySTL:       TRUE if input STL is a binary file. 		     M
*   EndianSwap:      Swap non char-long entities (little vs. big binaries    M
*		     endings).					             M
*   NormalFlip:      Flip normal directions if TRUE.			     M
*   Messages:        1 for error messages,				     M
*                    2 to include warning messages,			     M
*		     3 to include informative messages.			     M
*                    4 to include dump of IRIT objects.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Read STL DATA or NULL if error.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSTLSaveFile                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSTLLoadFile                                                            M
*****************************************************************************/
IPObjectStruct *IPSTLLoadFile(const char *STLFileName,
			      int BinarySTL,
			      int EndianSwap,
			      int NormalFlip,
			      int Messages)
{
    char *ReadAttr, Line[IRIT_LINE_LEN_VLONG];
    int i,
	PolyCirc = IPSetPolyListCirc(FALSE);
    FILE *StlFile;
    IPObjectStruct *PObj;

    GlblStlLineNum = 0;
    IPSetPolyListCirc(PolyCirc);

#   ifdef __WINNT__
        ReadAttr = BinarySTL ? "rb" : "r";
#   else
	ReadAttr = "r";
#   endif /* __WINNT__ */

    if (STLFileName == NULL || strcmp(STLFileName, "-") == 0) {
	StlFile = stdin;
#	if defined(__WINNT__)
	    if (BinarySTL)
	        _setmode(_fileno(stdin), _O_BINARY);        /* Binary mode. */
#	endif /* __WINNT__ */
    }
    else if ((StlFile = fopen(STLFileName, ReadAttr)) == NULL) {
	sprintf(Line,
		"Cannot open STL file \"%s\", exit", STLFileName);
	Stl2IritAbort(Line);
    }

    /* Skip the header. */
    if (BinarySTL) {
        long int Len;

	fread(&Line, 80, 1, StlFile);
	Line[80] = 0;

	fread(&Len, sizeof(long int), 1, StlFile);
	if (EndianSwap)
	    EndianSwapLongInts(&Len, 1);

	IRIT_INFO_MSG_PRINTF("%d triangles in STL binary file\n", Len);
    }
    else {
        /* Skip until a first facet is found. */
        do {
	    fgets(Line, IRIT_LINE_LEN_VLONG - 1, StlFile);
	    GlblStlLineNum++;
	}
	while (!feof(StlFile) && !strstr(Line, "facet"));
    }

    /* Read triangles and dump them immediately. */
    PObj = IPGenPolyObject("STL", NULL, NULL);
    if (feof(StlFile)) {
        IRIT_WARNING_MSG("Empty STL file, can be a binary/text STL problem?\n");
        return PObj;
    }

    do {
	char Pad[2];
        float Normal[3], Points[3][3];

        if (BinarySTL) {
	    fread(Normal, sizeof(float), 3, StlFile);
	    if (EndianSwap)
	        EndianSwapFloats(&Normal[0], 3);
	    fread(Points, sizeof(float), 9, StlFile);
	    if (EndianSwap)
	        EndianSwapFloats(&Points[0][0], 9);
	    fread(Pad, sizeof(char), 2, StlFile);
	}
	else {
	    /* Parse the normal. */
	    sscanf(Line, " facet normal  %f %f %f\n",
		   &Normal[0], &Normal[1], &Normal[2]);

	    /* Sync on the first vertex. */
	    do {
	        fgets(Line, IRIT_LINE_LEN_VLONG - 1, StlFile);
	        GlblStlLineNum++;
	    }
	    while (!feof(StlFile) && !strstr(Line, "vertex"));
	    if (feof(StlFile))
		break;

	    sscanf(Line, " vertex %f %f %f\n",
		   &Points[0][0], &Points[0][1], &Points[0][2]);

	    /* Sync on the second vertex. */
	    fgets(Line, IRIT_LINE_LEN_VLONG - 1, StlFile);
	    sscanf(Line, " vertex %f %f %f\n",
		   &Points[1][0], &Points[1][1], &Points[1][2]);

	    /* Sync on the third vertex. */
	    fgets(Line, IRIT_LINE_LEN_VLONG - 1, StlFile);
	    sscanf(Line, " vertex %f %f %f\n",
		   &Points[2][0], &Points[2][1], &Points[2][2]);

	    /* Skip until next facet is found. */
	    while (!strstr(fgets(Line, IRIT_LINE_LEN_VLONG - 1, StlFile),
			   "facet") &&
		   !strstr(Line, "normal") &&
		   !feof(StlFile));
	}

	if (!feof(StlFile)) {
	    IrtVecType V1, V2, V;
	    IPVertexStruct
		*Vrtx3 = IPAllocVertex2(NULL),
		*Vrtx2 = IPAllocVertex2(Vrtx3),
		*Vrtx1 = IPAllocVertex2(Vrtx2);

	    PObj -> U.Pl = IPAllocPolygon(0, Vrtx1, PObj -> U.Pl);

	    /* Estimate normal for this triangle. */
	    for (i = 0; i < 3; i++) {
	        V1[i] = Points[1][i] - Points[0][i];
		V2[i] = Points[2][i] - Points[1][i];
	    }
	    GMVecCrossProd(V, V1, V2);

	    if (IRIT_DOT_PROD(V, V) > IRIT_UEPS) {
	        if (!NormalFlip ||
		    (V[0] * Normal[0] +
		     V[1] * Normal[1] +
		     V[2] * Normal[2] > 0)) {
		    /* Flip the vertices order to flip the normal... */
		    PT_FLT2DBL_COPY(Vrtx1 -> Coord, Points[0]);
		    PT_FLT2DBL_COPY(Vrtx2 -> Coord, Points[2]);
		    PT_FLT2DBL_COPY(Vrtx3 -> Coord, Points[1]);
		}
		else {
		    PT_FLT2DBL_COPY(Vrtx1 -> Coord, Points[0]);
		    PT_FLT2DBL_COPY(Vrtx2 -> Coord, Points[1]);
		    PT_FLT2DBL_COPY(Vrtx3 -> Coord, Points[2]);
		}
	    }
	    IPUpdatePolyPlane(PObj -> U.Pl);
	    IPUpdateVrtxNrml(PObj -> U.Pl, PObj -> U.Pl -> Plane);

	    if (PolyCirc)
	        Vrtx3 -> Pnext = Vrtx1;
	}
    }
    while (!feof(StlFile));

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n float type numbers, in place.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   RP:   A pointer to the vector of floats.                                 *
*   n:    Number of entities in RP.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapFloats(float *RP, int n)
{
    int i, j, k;
    char
	*c = (char *) RP;

    for (i = 0; i < n; i++) {
	for (j = 0, k = sizeof(float) - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += sizeof(float);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Swaps a vector of n integer type numbers, in place.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   IP:   A pointer to the vector of integers.                               *
*   n:    Number of entities in IP.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EndianSwapLongInts(long int *IP, int n)
{
    int i, j, k;
    char
	*c = (char *) IP;

    for (i = 0; i < n; i++) {
	for (j = 0, k = sizeof(long int) - 1; j < k; j++, k--)
	    IRIT_SWAP(char, c[j], c[k]);

	c += sizeof(long int);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Stl2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:       Error message.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Stl2IritAbort(char *Str)
{
    IRIT_WARNING_MSG_PRINTF("\n%s, STL line %d\n", Str, GlblStlLineNum);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dummy function to link at debugging time.                               *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*****************************************************************************/
static void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
