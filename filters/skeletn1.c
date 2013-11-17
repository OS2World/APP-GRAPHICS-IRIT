/*****************************************************************************
* Skeleton for an interface to a parser to read IRIT data files.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Feb 1993    *
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
#include "irit_sm.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "grap_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void PrintData1(IPObjectStruct *PObj, int Indent);
static void PrintData2(IPObjectStruct *PObj, int Indent);
#ifdef USE_VARARGS
static void IFprintf(int Indent, char *va_alist, ...);
#else
static void IFprintf(int Indent, char *Format, ...);
#endif /* USE_VARARGS */
static char *Real2Str(IrtRType R);
static void SkelExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of skeletn1 - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) (argv + 1),
				   argc - 1, TRUE, FALSE)) == NULL)
	SkelExit(1);
    PObjects = IPResolveInstances(PObjects);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    /* Here some useful parameters to play with in tesselating freeforms: */
    IPFFCState.FineNess = 20; /* Resolution of tesselation, larger is finer. */
    IPFFCState.ComputeUV = TRUE;       /* Wants UV coordinates for textures. */
    IPFFCState.FourPerFlat = TRUE;/* 4 polygons per ~flat patch, 2 otherwise.*/
    IPFFCState.LinearOnePolyFlag = TRUE;   /* Linear srf generates one poly. */

    printf("[OBJECT ALL\n");

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, DumpOneTraversedObject);

    printf("]\n");

    return 0;
}

/*****************************************************************************
* AUXILIARY:								     *
* ALL FUNCTIONS BELOW ARE FOR PRINTING THE DATA STRUCTURE IN IRIT FORMAT     *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IPObjectStruct *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
        PrintData1(PObj, 4);
        PrintData2(PObj, 4);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the data from given geometry object.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Object to print.                                             *
*   Indent:     Column of indentation.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintData1(IPObjectStruct *PObj, int Indent)
{
    int i;
    IPPolygonStruct *PPolygon;
    IPVertexStruct *PVertex;
    const IPAttributeStruct
	*Attrs = AttrTraceAttributes(PObj -> Attr, PObj -> Attr);

    if (PObj -> ObjType != IP_OBJ_POLY) {
	IRIT_WARNING_MSG_PRINTF("PrintData1: Non polygonal object \"%s\" detected and ignored\n",
		                IP_GET_OBJ_NAME(PObj));
	return;
    }

    if (Attrs != NULL) {
	IFprintf(Indent, "[OBJECT\n");
	while (Attrs) {
	    IFprintf(Indent + 4, "%s\n", Attr2String(Attrs, TRUE));
	    Attrs = AttrTraceAttributes(Attrs, NULL);
	}
	IFprintf(0, "\t%s\n", IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj)
						      : "NONE");
    }
    else
        IFprintf(Indent, "[OBJECT %s\n",
		 IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "NONE");

    Indent += 4;

    for (PPolygon = PObj -> U.Pl;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext) {
	if (PPolygon -> PVertex == NULL) {
	    IRIT_WARNING_MSG("Dump: Attemp to dump empty polygon, exit\n");
	    SkelExit(1);
	}
	for (PVertex = PPolygon -> PVertex -> Pnext, i = 1;
	     PVertex != PPolygon -> PVertex && PVertex != NULL;
	     PVertex = PVertex -> Pnext, i++);
	IFprintf(Indent, "[%s %d\n",
		 IP_IS_POLYGON_OBJ(PObj)
		     ? "POLYGON"
		     : IP_IS_POINTLIST_OBJ(PObj) ? "POINTLIST" :"POLYLINE",
		 i);

	PVertex = PPolygon -> PVertex;
	do {			     /* Assume at least one edge in polygon! */
	    if (IP_IS_POLYGON_OBJ(PObj)) {
	        float *UVs;

		if ((UVs = AttrGetUVAttrib(PVertex -> Attr, "uvvals")) == NULL)
		    IFprintf(Indent + 4,
			     "[[NORMAL %s %s %s] %s %s %s]\n",
			     Real2Str(PVertex -> Normal[0]),
			     Real2Str(PVertex -> Normal[1]),
			     Real2Str(PVertex -> Normal[2]),
			     Real2Str(PVertex -> Coord[0]),
			     Real2Str(PVertex -> Coord[1]),
			     Real2Str(PVertex -> Coord[2]));
		else
		    IFprintf(Indent + 4,
			     "[[NORMAL %s %s %s] [UVvals \"%s %s\"] %s %s %s]\n",
			     Real2Str(PVertex -> Normal[0]),
			     Real2Str(PVertex -> Normal[1]),
			     Real2Str(PVertex -> Normal[2]),
			     Real2Str(UVs[0]),
			     Real2Str(UVs[1]),
			     Real2Str(PVertex -> Coord[0]),
			     Real2Str(PVertex -> Coord[1]),
			     Real2Str(PVertex -> Coord[2]));
	    }
	    else if (IP_IS_POLYLINE_OBJ(PObj)) {
		IFprintf(Indent + 4, "[%s %s %s]\n",
			 Real2Str(PVertex -> Coord[0]),
			 Real2Str(PVertex -> Coord[1]),
			 Real2Str(PVertex -> Coord[2]));
	    }
	    else if (IP_IS_POINTLIST_OBJ(PObj)) {
		IFprintf(Indent + 4, "[%s %s %s]\n",
			 Real2Str(PVertex -> Coord[0]),
			 Real2Str(PVertex -> Coord[1]),
			 Real2Str(PVertex -> Coord[2]));
	    }

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != PPolygon -> PVertex && PVertex != NULL);
	IFprintf(Indent, "]\n");	       /* Close the polygon. */
    }

    Indent -= 4;
    IFprintf(Indent, "]\n");				/* Close the object. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the data from given geometry object.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Object to print.                                             *
*   Indent:     Column of indentation.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintData2(IPObjectStruct *PObj, int Indent)
{
    int **Pls;
    IPVertexStruct **V;
    IPPolyVrtxIdxStruct *PVIdx;

    if (PObj -> ObjType != IP_OBJ_POLY) {
	IRIT_WARNING_MSG_PRINTF("PrintData2: Non polygonal object \"%s\" detected and ignored\n",
		                IP_GET_OBJ_NAME(PObj));
	return;
    }

    PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, FALSE, 7);

    /* Dump vertices so that each identical vertex shows up once, Vrml style.*/
    IFprintf(Indent, "coord Coordinate {\n");
    Indent += 4;
    IFprintf(Indent, "point [\n");
    for (V = PVIdx -> Vertices; *V != NULL; V++) {
        IFprintf(Indent + 4, "[%s %s %s],\n",
		 Real2Str((*V) -> Coord[0]),
		 Real2Str((*V) -> Coord[1]),
		 Real2Str((*V) -> Coord[2]));
    }

    IFprintf(Indent, "]\n");
    Indent -= 4;
    IFprintf(Indent, "}\n");

    IFprintf(Indent, "coordIndex [\n");
    for (Pls = PVIdx -> Polygons; *Pls != NULL; Pls++) {
        int *Pl = *Pls;

	/* Assume at least one edge in polygon! */		 
	IFprintf(Indent + 4, " ");
	do {
	    IFprintf(0, "%d ", *Pl++);
	}
	while (*Pl >= 0);
	IFprintf(0, "-1,\n");
    }
    IFprintf(Indent, "]\n");

    IPPolyVrtxIdxFree(PVIdx);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Same as printf but with indentation.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:      Column of indentation.                                      *
*   va_alist:    Do "man stdarg".                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IFprintf(int Indent, char *va_alist, ...)
{
    char *Format, Line[IRIT_LINE_LEN_XLONG];
    int i;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IFprintf(int Indent, char *Format, ...)
{
    char Line[IRIT_LINE_LEN_XLONG];
    int i;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    for (i = 0; i < Indent; i++)
	Line[i] = ' ';
    vsprintf(&Line[Indent], Format, ArgPtr);
    va_end(ArgPtr);

    printf(Line);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a real number into a string.					     *
*   The routine maintains 6 different buffers simultanuously so up to 6      *
* calls can be issued from same printf...				     *
*                                                                            *
* PARAMETERS:                                                                *
*   R:          To convert to a string.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     A string representation for R.                               *
*****************************************************************************/
static char *Real2Str(IrtRType R)
{
    IRIT_STATIC_DATA int j, k,
	i = 0;
    IRIT_STATIC_DATA char Buffer[6][IRIT_LINE_LEN_SHORT];

    if (IRIT_FABS(R) < IRIT_EPS)
	R = 0.0;			    /* Round off very small numbers. */

    sprintf(Buffer[i], "%6g", R);

    for (k = 0; !isdigit(Buffer[i][k]) && k < IRIT_LINE_LEN; k++);
    if (k >= IRIT_LINE_LEN) {
	IRIT_WARNING_MSG_PRINTF("Conversion of real number (%f) failed.\n", R);
	SkelExit(1);
    }

    for (j = strlen(Buffer[i]) - 1; Buffer[i][j] == ' ' && j > k; j--);
    if (strchr(Buffer[i], '.') != NULL)
	for (; Buffer[i][j] == '0' && j > k; j--);
    Buffer[i][j+1] = 0;

    j = i;
    i = (i + 1) % 6;
    return Buffer[j];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* SkelExit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SkelExit(int ExitCode)
{
    exit(ExitCode);
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
void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
