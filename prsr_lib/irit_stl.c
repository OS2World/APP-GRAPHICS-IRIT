/*****************************************************************************
* Module to save IRIT data into an STL files.		        	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, May 1998   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <setjmp.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

#define SAME_VRTX_DEF_EPS 1e-4

#define IHT_VERTEX_KEY(Pt)	(Pt[0] * 0.301060 + \
				 Pt[1] * 0.050964 + \
				 Pt[2] * 0.161188)

IRIT_STATIC_DATA int
    GlblRegularTriang = TRUE,
    GlblMultiObjSplit = 0;
IRIT_STATIC_DATA IrtRType
    GlblSameVrtxEps = SAME_VRTX_DEF_EPS;

static int CmpTwoVertices(VoidPtr V1, VoidPtr V2);
static void MakeVerticesIdentical(IPObjectStruct *PObj);
static void DumpDataForStl(IPObjectStruct *PObjects,
			   const char *OutFileName,
			   int Messages,
			   int Level);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the epsilon euqality of two vertices, to be considered the same.    M
*                                                                            *
* PARAMETERS:                                                                M
*   SameVrtxEps:   New epsilon.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:     Old epsilon.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSTLSaveFile                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSTLSaveSetVrtxEps                                                      M
*****************************************************************************/
IrtRType IPSTLSaveSetVrtxEps(IrtRType SameVrtxEps)
{
    IrtRType
	OldVal = GlblSameVrtxEps;

    GlblSameVrtxEps = SameVrtxEps;
    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dumps IRIT object as an STL file.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:          IritObject to dump as STL file.                           M
*   CrntViewMat:   The current viewing matrix to apply to the object.        M
*   RegularTriang: Should we regularized the triagles before dumping them?   M
*   MultiObjSplit: 0 to save everything as one large file,		     M
*		   1 to save every IRIT object as a separate STL object,     M
*		   2 to save every IRIT object as a separate STL object in   M
*		     a separated file.					     M
*   STLFileName:   Name of STL file, "-" or NULL for stdout.		     M
*   Messages:      TRUE for warning messages.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if succesful, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSTLLoadFile, IPSTLSaveSetVrtxEps                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSTLSaveFile                                                            M
*****************************************************************************/
int IPSTLSaveFile(const IPObjectStruct *PObj,
		  IrtHmgnMatType CrntViewMat,
		  int RegularTriang,
		  int MultiObjSplit,
		  const char *STLFileName,
		  int Messages)
{
    int OldRefCountState = IPSetCopyObjectReferenceCount(FALSE);
    IPObjectStruct
        *PTmp = IPCopyObject(NULL, PObj, TRUE);

    PTmp -> Pnext = NULL;
    IPSetCopyObjectReferenceCount(OldRefCountState);

    if (!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(PObj, "RegularTriang")))
        RegularTriang = AttrGetObjectIntAttrib(PObj, "RegularTriang");
    if (!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(PObj, "MultiObjSplit")))
        MultiObjSplit = AttrGetObjectIntAttrib(PObj, "MultiObjSplit");

    IPTraverseObjListHierarchy(PTmp, CrntViewMat, IPMapObjectInPlace);
    PTmp = IPResolveInstances(PTmp);

    GlblRegularTriang = RegularTriang;
    GlblMultiObjSplit = MultiObjSplit;

    DumpDataForStl(PTmp, STLFileName, Messages, 0);

    IPFreeObject(PTmp);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare two vertices if same or not.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2:   Two vertices to compare.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0, +1 if V1 less, equal, greater than V2.                  *
*****************************************************************************/
static int CmpTwoVertices(VoidPtr V1, VoidPtr V2)
{
    IrtRType
	*Coord1 = (IrtRType *) V1,
	*Coord2 = (IrtRType *) V2;

    if (IRIT_PT_APX_EQ(Coord1, Coord2))
	return 0;
    else {
        int i;

        for (i = 0; i < 3; i++) {
	    if (Coord1[i] < Coord2[i])
	        return -1;
	    else if (Coord1[i] > Coord2[i])
	        return 1;
	}

	return 0; /* Should never get here. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make sure all epsilon like vertices are made identically the same.       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Polygonal object to process.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MakeVerticesIdentical(IPObjectStruct *PObj)
{
    IrtRType Min, Max;
    GMBBBboxStruct *BBox;
    IritHashTableStruct *IHT;
    IPVertexStruct *PVertex;
    IPPolygonStruct *PPoly;

    if (PObj == NULL || PObj -> U.Pl == NULL)
	return;

    BBox = GMBBComputeBboxObject(PObj);
    Min = IRIT_MIN(IRIT_MIN(BBox -> Min[0], BBox -> Min[1]), BBox -> Min[2]);
    Max = IRIT_MAX(IRIT_MAX(BBox -> Max[0], BBox -> Max[1]), BBox -> Max[2]);

    /* Create the hash table. */
    IHT = IritHashTableCreate(Min, Max, IRIT_EPS,
			      IPPolyListLen(PObj -> U.Pl));

    /* Insert all vertices to the hash table.  Similar vertices upto      */
    /* the epsilon prescribed in CmpTwoVertices are inserted once only.   */
    for (PPoly = PObj -> U.Pl;
	 PPoly != NULL;
	 PPoly = PPoly -> Pnext) {
        PVertex = PPoly -> PVertex;
	do {
	    IritHashTableInsert(IHT, PVertex -> Coord, CmpTwoVertices,
				IHT_VERTEX_KEY(PVertex -> Coord), FALSE);

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPoly -> PVertex);
    }

    /* Find duplicated entries amd kae them identical. */
    for (PPoly = PObj -> U.Pl;
	 PPoly != NULL;
	 PPoly = PPoly -> Pnext) {
        PVertex = PPoly -> PVertex;
	do {
	    VoidPtr Data;

	    if ((Data = IritHashTableFind(IHT, PVertex -> Coord,
					  CmpTwoVertices,
					  IHT_VERTEX_KEY(PVertex -> Coord)))
								  != NULL &&
		Data != (VoidPtr) PVertex -> Coord) {
	        /* It is an entry that is similar to this one.  Coerce   */
	        /* them both to be identically the same.		 */
		IRIT_PT_COPY(PVertex -> Coord, Data);
	    }

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPoly -> PVertex);
    }
    
    IritHashTableFree(IHT);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for Stl to OutFileName.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         To dump into file.                                         *
*   OutFileName:  Name of output STL file.			             *
*   Messages:     TRUE to print more error messages.			     *
*   Level:        Of recursion, 0 for top level.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForStl(IPObjectStruct *PObj,
			   const char *OutFileName,
			   int Messages,
			   int Level)
{
    IRIT_STATIC_DATA FILE
	*f = NULL;
    char
	*Name = IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "irit2stl";
    int i;
    IPPolygonStruct *PPoly;
    IPObjectStruct *PObjReg, *PObjRegTris, *PPObj;

    if (Level == 0 && GlblMultiObjSplit < 2) {
	if (OutFileName != NULL && strncmp(OutFileName, "-", 1) != 0) {
	    if ((f = fopen(OutFileName, "w")) == NULL) {
	        IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n",
					OutFileName);
		return;
	    }
	}
	else
	    f = stdout;

	if (GlblMultiObjSplit == 0)
	    fprintf(f, "solid %s\n", Name);
    }

    /* Process freeform geometry into polys. */
    if (IP_IS_FFGEOM_OBJ(PObj))
        PPObj = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else if (IP_IS_GEOM_OBJ(PObj))
        PPObj = PObj;
    else if (IP_IS_OLST_OBJ(PObj)) {
        IPObjectStruct *PTmp;
	int j = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PObj, j++)) != NULL)
	    DumpDataForStl(PTmp, OutFileName, Messages, Level + 1);

	if (Level == 0 && GlblMultiObjSplit < 2) {
	    if (GlblMultiObjSplit == 0)
	        fprintf(f, "endsolid\n\n");

	    if (f != stdout)
	        fclose(f);
	}
	return;
    }
    else
        return;

    if (!IP_IS_POLY_OBJ(PPObj) || !IP_IS_POLYGON_OBJ(PPObj)) {
        if (Messages)
	    IRIT_WARNING_MSG_PRINTF("Non polygonal object \"%s\" ignored.\n",
				    Name);
	return;
    }

    if (GlblMultiObjSplit >= 2) {
        IRIT_STATIC_DATA int
	    FileCount = 1;
	char OutFileNameIndex[IRIT_LINE_LEN_LONG];
	    
	if (OutFileName != NULL) {
	    char
	        *p = strchr(OutFileName, '.');

	    if (p != NULL)
	        *p = 0;
	    sprintf(OutFileNameIndex, "%s%d.stl", OutFileName, FileCount++);

	    if ((f = fopen(OutFileNameIndex, "w")) == NULL) {
	        IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n",
					OutFileNameIndex);
		return;
	    }
	}
	else
	    f = stdout;

	if (Messages)	
	    IRIT_INFO_MSG_PRINTF("Processing object \"%s\" into file \"%s\".\n",
			      Name, f == stdout ? "stdout" : OutFileNameIndex);
    }
    else {
        if (Messages)	
	    IRIT_INFO_MSG_PRINTF("Processing object \"%s\" into file \"%s\".\n",
				 Name, f == stdout ? "stdout" : OutFileName);
    }
    if (GlblMultiObjSplit > 0)
        fprintf(f, "solid %s\n", Name);

    if (GlblRegularTriang) {
        PObjReg = GMRegularizePolyModel(PPObj, TRUE);
	PObjRegTris = GMConvertPolysToTriangles(PObjReg);
	IPFreeObject(PObjReg);
    }
    else
        PObjRegTris = GMConvertPolysToTriangles(PPObj);

    if (PPObj != PObj)
        IPFreeObject(PPObj);

    MakeVerticesIdentical(PObjRegTris);

    /* Dump the geometry out. */
    for (PPoly = PObjRegTris -> U.Pl;
	 PPoly != NULL;
	 PPoly = PPoly -> Pnext) {
        IPVertexStruct *V;

	if (IRIT_PT_APX_EQ_ZERO_EPS(PPoly -> Plane, IRIT_EPS))
	    IRIT_WARNING_MSG("Warning: zero normals detected!\n");

	fprintf(f, "  facet normal %9.6f %9.6f %9.6f\n    outer loop\n",
		-PPoly -> Plane[0],
		-PPoly -> Plane[1],
		-PPoly -> Plane[2]);
	for (i = 0, V = PPoly -> PVertex;
	     i < 3 && V != NULL;
	     i++, V = V -> Pnext) {
	    fprintf(f, "      vertex %9.6f %9.6f %9.6f\n",
		    V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	}
	fprintf(f, "    endloop\n  endfacet\n");
    }

    IPFreeObject(PObjRegTris);

    if (GlblMultiObjSplit > 0)
        fprintf(f, "endsolid\n\n");

    if (GlblMultiObjSplit >= 2 && f != stdout)
	fclose(f);

    if (Level == 0 && GlblMultiObjSplit < 2) {
	if (GlblMultiObjSplit == 0)
	    fprintf(f, "endsolid\n\n");

        if (f != stdout)
	    fclose(f);
    }
}
