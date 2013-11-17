/*****************************************************************************
* Convert IRIT data (.itd) files back to IRIT .irt files.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Sep 1998    *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */
#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "triv_lib.h"
#include "trim_lib.h"
#include "trng_lib.h"
#include "mvar_lib.h"
#include "mdl_lib.h"
#include "misc_lib.h"

#define IP_CNV_SRF_RVRS_ENG_EPS	1e-3
#define IP_VRTX_SAME_EPS	IRIT_EPS
#define IHT_VERTEX_KEY(Pt)	(Pt[0] * 0.301060 + \
				 Pt[1] * 0.050964 + \
				 Pt[2] * 0.161188)

IRIT_STATIC_DATA int
    GlblIdenticalVertexAttribMask = 0;

#ifdef USE_VARARGS
static void IPCnvPrintf(char *va_alist, ...);
#else
static void IPCnvPrintf(char *Format, ...);
#endif /* USE_VARARGS */

static void IPCnvOnePolygon(const char *Indent, IPPolygonStruct *PPolygon);
static void IPCnvOneCurve(const char *Indent,
			  const char *Name,
			  CagdCrvStruct *Crv);
static void IPCnvOneSurface(const char *Indent,
			    const char *Name,
			    CagdSrfStruct *Srf);
static void IPCnvOneTrimmedSrf(const char *Indent,
			       const char *Name,
			       TrimSrfStruct *TrimSrf);
static void IPCnvOneTrivar(const char *Indent,
			   const char *Name,
			   TrivTVStruct *Trivar);
static void IPCnvOneTriSrf(const char *Indent,
			   const char *Name,
			   TrngTriangSrfStruct *TriSrf);
static void IPCnvOneModel(const char *Indent,
			  const char *Name,
			  MdlModelStruct *Mdl);
static void IPCnvOneMultiVar(const char *Indent,
			     const char *Name,
			     MvarMVStruct *MultiVar);
static void IPCnvCtlPt(const char *Indent, 
		       CagdPointType PType,
		       IrtRType **Points,
		       int Index);
static void IPCnvKnotVector(const char *Indent, 
			    IrtRType *KnotVector,
			    CagdBType Periodic,
			    int Len,
			    int Order);
static const char *IPCnvAssignName(const char *Name);
static CagdCrvStruct *IPCnvLeastSquaresFitCrv(CagdCrvStruct *Crv);
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2);
static int *IPCnvPolyVrtxNeighborsAux(IPPolyVrtxIdxStruct *PVIdx,
				      int VIdx,
				      int Ring);
static int IPCnvPolyVrtxNeighborsInsV(int **NV, int *CrntNV, int NewIdx);
static void IPCnvAddVertNeighbAux(IPPolyVrtxIdxStruct *PVIdx,
				  int **NV,
				  unsigned int VIdx,
				  int RingOrigin);
IRIT_STATIC_DATA char
    IPCnvDelimitChar = ';';
IRIT_STATIC_DATA int
    GlblNameCount = 1,
    GlblCompactListDump = FALSE,
    GlblDumpAssignName = TRUE,
    GlblLeastSquaresMinLenFit = 50,
    GlblLeastSquaresPercent = 30,
    GlblPolyNum = 1,
    GlblCrntNV = 0;
IRIT_STATIC_DATA IrtRType
    GlblLeastSquaresMaxError = 0.01;
IRIT_STATIC_DATA IPPrintFuncType
    IPCnvPrintFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the given Objects to .irt style.  Output goes to the function   M
* IPCnvPrintFunc which echos the lines, one at a time.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:   To convert to .irt style.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvSetPrintFunc, IPCnvSetDelimitChar, IPCnvDataToIritOneObject         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvDataToIrit                                                          M
*****************************************************************************/
void IPCnvDataToIrit(const IPObjectStruct *PObjects)
{
    for ( ; PObjects != NULL; PObjects = PObjects -> Pnext)
	IPCnvDataToIritOneObject("", PObjects, 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the printing function to call if needs to redirect printing of dat to M
* irt conversions.  Called (indirectly) by IPCnvDataToIrit.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CnvPrintFunc:   A function that gets a single string it should print.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPrintFuncType:   Old value of this state.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvSetPrintFunc, files                                                 M
*****************************************************************************/
IPPrintFuncType IPCnvSetPrintFunc(IPPrintFuncType CnvPrintFunc)
{
    IPPrintFuncType
	OldVal = IPCnvPrintFunc;

    IPCnvPrintFunc = CnvPrintFunc;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits using least squares, a new curve to the input curve with only       M
* Percent percents control points.  A curve with be least squares fitted     M
* if it has more than MinLenFit control points.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   MinLenFit:   Minimum number of control point to attempt a fit.           M
*   Percent:     Percent of number of control points to fit to.              M
*   MaxError:    maximum allowed error (in maximum norm).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old percent value.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvSetLeastSquaresFit                                                  M
*****************************************************************************/
int IPCnvSetLeastSquaresFit(int MinLenFit, int Percent, IrtRType MaxError)
{
    int OldVal = GlblLeastSquaresPercent;

    GlblLeastSquaresMinLenFit = MinLenFit;
    GlblLeastSquaresPercent = Percent;
    GlblLeastSquaresMaxError = MaxError;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the delimiting chararacter.  Typically ';' but can be ':' as well.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Delimit:   The character to consider as an expression delimiting char.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   char:   Old delimiting character.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvSetDelimitChar, files                                               M
*****************************************************************************/
char IPCnvSetDelimitChar(char Delimit)
{
    char
	OldVal = IPCnvDelimitChar;

    IPCnvDelimitChar = Delimit;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the way list objects are dumped - TRUE for a single list, FALSE for   M
* separated objects that are grouped into a list at the end.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CompactList:   TRUE for compact list, FALSE for separate entities..      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Old state of compact list dump.	                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvSetCompactList, files                                               M
*****************************************************************************/
int IPCnvSetCompactList(int CompactList)
{
    int OldVal = GlblCompactListDump;

    GlblCompactListDump = CompactList;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If TRUE objects are dumped with an assignment to their own name.         M
* Otherwise, just the geometry is dumped with no assignment.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   DumpAssignName:   TRUE to dump assignment, FALSE for no assignment.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Old state of dump assignments.	                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvSetDumpAssignName, files                                            M
*****************************************************************************/
int IPCnvSetDumpAssignName(int DumpAssignName)
{
    int OldVal = GlblDumpAssignName;

    GlblDumpAssignName = DumpAssignName;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Same as printf but for this dat to irt conversion only.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   va_alist:   Do "man stdarg".                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IPCnvPrintf(char *va_alist, ...)
{
    char *Format, *p;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IPCnvPrintf(char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IPCnvPrintFunc(p);

    va_end(ArgPtr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts one object PObject to .irt style                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Indent:       Level of indentation, as white spaces string               M
*   PObj:         Object to convert to .irt style.                           M
*   Level:        Nesting level of this object.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit, IPCnvSetPrintFunc                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvDataToIritOneObject                                                 M
*****************************************************************************/
void IPCnvDataToIritOneObject(const char *Indent,
			      const IPObjectStruct *PObj,
			      int Level)
{
    int i, j;
    char NewIndent[IRIT_LINE_LEN];
    CagdRType *R;
    CagdSrfStruct *Srf;
    CagdCrvStruct *Crv;
    TrimSrfStruct *TrimSrf;
    TrivTVStruct *Trivar;
    MvarMVStruct *MultiVar;
    TrngTriangSrfStruct *TriSrf;
    MdlModelStruct *Mdl;
    IPPolygonStruct *Pl;
    IPObjectStruct *PTmp,
        *PObject = IPCopyObject(NULL, PObj, TRUE);

    if (!IP_VALID_OBJ_NAME(PObject))
	IP_SET_OBJ_NAME(PObject, "NoName%d", GlblNameCount++);

    sprintf(NewIndent, "%s    ", Indent);

    if (PObject -> ObjType != IP_OBJ_LIST_OBJ)
	IPCnvPrintf("\n%s# %s\n", Indent, IP_GET_OBJ_NAME(PObject));

    switch (PObject -> ObjType) {
	case IP_OBJ_LIST_OBJ:
	    if (GlblCompactListDump) {
	        int OldDelim = IPCnvDelimitChar,
		    OldAssignName = GlblDumpAssignName;
 
		IPCnvPrintf("\n%s# %s\n", Indent, IP_GET_OBJ_NAME(PObject));
	        IPCnvPrintf("%s%slist(\n", Indent,
			    IPCnvAssignName(IP_GET_OBJ_NAME(PObject)));

		GlblDumpAssignName = FALSE;

	        for (i = 0;
		     (PTmp = IPListObjectGet(PObject, i)) != NULL;
		     i++) {
		    if (IPListObjectGet(PObject, i + 1) == NULL)
		        IPCnvDelimitChar = ' ';
		    else
		        IPCnvDelimitChar = ',';
		    IPCnvDataToIritOneObject(NewIndent, PTmp, Level + 1);
		}

		GlblDumpAssignName = OldAssignName;
	        IPCnvDelimitChar = OldDelim;

	        IPCnvPrintf(")%c\n", IPCnvDelimitChar);
	    }
	    else {
	        for (i = 0; (PTmp = IPListObjectGet(PObject, i)) != NULL; i++)
		    IPCnvDataToIritOneObject(Indent, PTmp, Level + 1);

		if (!IP_VALID_OBJ_NAME(PObject))
		    IP_SET_OBJ_NAME(PObject, "NoName%d", GlblNameCount++);

		IPCnvPrintf("\n%s# %s\n", Indent, IP_GET_OBJ_NAME(PObject));
		IPCnvPrintf("%s%slist( ", Indent,
			    IPCnvAssignName(IP_GET_OBJ_NAME(PObject)));

		for (j = 0;
		     (PTmp = IPListObjectGet(PObject, j)) != NULL;
		     j++) {
		    if (!IP_VALID_OBJ_NAME(PTmp))
		        IP_SET_OBJ_NAME(PTmp, "NoName%d", GlblNameCount++);

		    IPCnvPrintf("%s%s", j == 0 ? "" : "\t\t", IP_GET_OBJ_NAME(PTmp));
		    if (j == i - 1)
		        IPCnvPrintf(" )%c\n", IPCnvDelimitChar);
		    else
		        IPCnvPrintf(",\n");
		}
	    }
	    break;
	case IP_OBJ_POLY:
	    Pl = PObject -> U.Pl;
	    if (Pl -> Pnext == NULL) {
		/* One polygon only. */
	        IPCnvPrintf("%s%s\n%s    poly( list( ", Indent,
			    IPCnvAssignName(IP_GET_OBJ_NAME(PObject)), Indent);

		IPCnvOnePolygon(NewIndent, Pl);

		IPCnvPrintf(" ), %s )%c\n",
			    IP_IS_POLYLINE_OBJ(PObject) ? "true" : "false",
			    IPCnvDelimitChar);
	    }
	    else {
	        i = GlblPolyNum;
		IPCnvPrintf("%s%smergePoly( list(\n", Indent, 
			    IPCnvAssignName(IP_GET_OBJ_NAME(PObject)));
		for ( ; Pl != NULL; Pl = Pl -> Pnext) {
		    IPCnvPrintf("%spoly( list( ", NewIndent);

		    IPCnvOnePolygon(NewIndent, Pl);

		    IPCnvPrintf(" ), %s )%s",
				IP_IS_POLYLINE_OBJ(PObject) ? "true" : "false",
				Pl -> Pnext == NULL ? "" : ",\n");
		}
		IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    }
	    break;
	case IP_OBJ_NUMERIC:
	    IPCnvPrintf("%s%s%s%c\n", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			_IPReal2Str(PObject -> U.R), IPCnvDelimitChar);
	    break;
	case IP_OBJ_POINT:
	    IPCnvPrintf("%s%spoint(%s, %s, %s)%c\n", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			_IPReal2Str(PObject -> U.Pt[0]),
			_IPReal2Str(PObject -> U.Pt[1]),
			_IPReal2Str(PObject -> U.Pt[2]),
			IPCnvDelimitChar);
	    break;
	case IP_OBJ_VECTOR:
	    IPCnvPrintf("%s%svector(%s, %s, %s)%c\n", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			_IPReal2Str(PObject -> U.Vec[0]),
			_IPReal2Str(PObject -> U.Vec[1]),
			_IPReal2Str(PObject -> U.Vec[2]),
			IPCnvDelimitChar);
	    break;
	case IP_OBJ_PLANE:
	    IPCnvPrintf("%s%splane(%s, %s, %s, %s)%c\n", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			_IPReal2Str(PObject -> U.Plane[0]),
			_IPReal2Str(PObject -> U.Plane[1]),
			_IPReal2Str(PObject -> U.Plane[2]),
			_IPReal2Str(PObject -> U.Plane[3]),
			IPCnvDelimitChar);
	    break;
	case IP_OBJ_MATRIX:
	    IPCnvPrintf("%s%shomomat( list( ", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)));
	    for (i = 0; i < 4; i++) {
		IPCnvPrintf("\n%slist( ", NewIndent);
	        for (j = 0; j < 4; j++) {
		    IPCnvPrintf(i == 3 && j == 3 ? "%s) ) )"
						 : (j == 3 ? "%s ),"
						           : "%s, "),
				_IPReal2Str((*PObject -> U.Mat)[i][j]));
		}
	    }
	    IPCnvPrintf("%c\n", IPCnvDelimitChar);
	    break;
	case IP_OBJ_INSTANCE:
	    IPCnvPrintf("%s%sinstance( \"%s\", homomat( list( ", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			PObject -> U.Instance -> Name);
	    for (i = 0; i < 4; i++) {
		IPCnvPrintf("\n%slist( ", NewIndent);
	        for (j = 0; j < 4; j++) {
		    IPCnvPrintf(i == 3 && j == 3 ? "%s ) ) ) )"
					 	 : (j == 3 ? "%s ),"
						           : "%s, "),
			      _IPReal2Str(PObject -> U.Instance -> Mat[i][j]));
		}
	    }
	    IPCnvPrintf("%c\n", IPCnvDelimitChar);
	    break;
	case IP_OBJ_CURVE:
	    for (Crv = PObject -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext)
		IPCnvOneCurve(Indent, IP_GET_OBJ_NAME(PObject), Crv);
	    break;
	case IP_OBJ_SURFACE:
	    for (Srf = PObject -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext)
		IPCnvOneSurface(Indent, IP_GET_OBJ_NAME(PObject), Srf);
	    break;
	case IP_OBJ_TRIMSRF:
	    for (TrimSrf = PObject -> U.TrimSrfs;
		 TrimSrf != NULL;
		 TrimSrf = TrimSrf -> Pnext)
		IPCnvOneTrimmedSrf(Indent, IP_GET_OBJ_NAME(PObject), TrimSrf);
	    break;
	case IP_OBJ_TRIVAR:
	    for (Trivar = PObject -> U.Trivars;
		 Trivar != NULL;
		 Trivar = Trivar -> Pnext)
		IPCnvOneTrivar(Indent, IP_GET_OBJ_NAME(PObject), Trivar);
	    break;
	case IP_OBJ_MULTIVAR:
	    for (MultiVar = PObject -> U.MultiVars;
		 MultiVar != NULL;
		 MultiVar = MultiVar -> Pnext)
		IPCnvOneMultiVar(Indent, IP_GET_OBJ_NAME(PObject), MultiVar);
	    break;
	case IP_OBJ_TRISRF:
	    for (TriSrf = PObject -> U.TriSrfs;
		 TriSrf != NULL;
		 TriSrf = TriSrf -> Pnext)
		IPCnvOneTriSrf(Indent, IP_GET_OBJ_NAME(PObject), TriSrf);
	    break;
	case IP_OBJ_MODEL:
	    for (Mdl = PObject -> U.Mdls;
		 Mdl != NULL;
		 Mdl = Mdl -> Pnext)
		IPCnvOneModel(Indent, IP_GET_OBJ_NAME(PObject), Mdl);
	    break;
	case IP_OBJ_STRING:
	    IPCnvPrintf("%s%s\"%s\"%c\n", Indent,
			IPCnvAssignName(IP_GET_OBJ_NAME(PObject)),
			PObject -> U.Str, IPCnvDelimitChar);
	    break;
	case IP_OBJ_CTLPT:
	    R = PObject -> U.CtlPt.Coords;
	    IPCnvPrintf("%s%s", Indent, IPCnvAssignName(IP_GET_OBJ_NAME(PObject)));
	    IPCnvCtlPt("", PObject -> U.CtlPt.PtType, &R, -1);
	    IPCnvPrintf("\n");
	    break;
	default:
	    break;
    }

    if (GlblDumpAssignName) {
        const IPAttributeStruct
	    *Attrs = AttrTraceAttributes(PObject -> Attr, PObject -> Attr);

        IPCnvDataToIritAttribs(Indent, IP_GET_OBJ_NAME(PObject), Attrs);
    }

    IPCnvPrintf(Level == 0 ? "\n\n" : (Level == 1 ? "\n" : ""));

    IPFreeObject(PObject);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts attributes PObject to .irt style                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Indent:       Level of indentation, as white spaces string.              M
*   ObjName:      Name of object these attributes are for.                   M
*   Attr:         Attributes to convert to irit scripting format.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvDataToIrit, IPCnvSetPrintFunc		                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvDataToIritAttribs                                                   M
*****************************************************************************/
void IPCnvDataToIritAttribs(const char *Indent,
			    const char *ObjName,
			    const IPAttributeStruct *Attr)
{
    while (Attr) {
        if (Attr -> Type == IP_ATTR_OBJ) {
	    char Name[IRIT_LINE_LEN], *AttrObjName;

	    if (IP_VALID_OBJ_NAME(Attr -> U.PObj))
	        AttrObjName = IP_GET_OBJ_NAME(Attr -> U.PObj);
	    else {
	        sprintf(Name, "NoName%d", GlblNameCount);
		AttrObjName = Name;
	    }

	    IPCnvDataToIritOneObject(Indent, Attr -> U.PObj, 2);
	    IPCnvPrintf("%sattrib(%s, \"%s\", %s)%c\n", Indent,
			ObjName, AttrGetAttribName(Attr),
			AttrObjName, IPCnvDelimitChar);
	}
	else {
	    IPCnvPrintf("%sattrib(%s, \"%s\", %s)%c\n", Indent,
			ObjName, AttrGetAttribName(Attr),
			Attr2String(Attr, FALSE), IPCnvDelimitChar);
	}
	Attr = AttrTraceAttributes(Attr, NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one polygon to .irt style.	                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   PPolygon:     Polygon to convert.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOnePolygon(const char *Indent, IPPolygonStruct *PPolygon)
{
    IPVertexStruct
	*V = PPolygon -> PVertex,
	*VFirst = V;
    char Indent2[IRIT_LINE_LEN];

    sprintf(Indent2, "%s            ", Indent);

    do {
        IPCnvPrintf("%svector( %s, %s, %s )%s", V == VFirst ? "" : Indent2,
		    _IPReal2Str(V -> Coord[0]),
		    _IPReal2Str(V -> Coord[1]),
		    _IPReal2Str(V -> Coord[2]),
		    (V  -> Pnext == VFirst || V  -> Pnext == NULL) ?
								  "" : ",\n");
	V = V -> Pnext;
    }
    while (V != VFirst && V != NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one curve to .irt style.	                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of curve to convert to .irt style.                    *
*		  If NULL, assumed to be a part of high level geometry such  *
*		  as a trimmed surface.					     *
*   Crv:   	  Geometry to convert.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneCurve(const char *Indent,
			  const char *Name,
			  CagdCrvStruct *Crv)
{
    int i;
    char NewIndent[IRIT_LINE_LEN], Indent2[IRIT_LINE_LEN];

    sprintf(NewIndent, "%s    ", Indent);
    sprintf(Indent2, "%s          ", Indent);

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    IPCnvPrintf("%s%scbezier(\n", Indent,
			Name ? IPCnvAssignName(Name) : "");
	    break;
	case CAGD_CBSPLINE_TYPE:
	    IPCnvPrintf("%s%scbspline( %d,\n", Indent,
			Name ? IPCnvAssignName(Name) : "",
			Crv -> Order);
	    break;
	case CAGD_CPOWER_TYPE:
	    IPCnvPrintf("%s%scpower(\n", Indent,
			Name ? IPCnvAssignName(Name) : "");
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNSUPPORT_CRV_TYPE);
	    return;
    }

    /* Print the control polygon: */
    IPCnvPrintf("%s    list( ", Indent);
    for (i = 0; i < Crv -> Length; i++) {
	IPCnvCtlPt(i == 0 ? "" : Indent2, Crv -> PType, Crv -> Points, i);
	if (i < Crv -> Length - 1)
	    IPCnvPrintf(",\n");
    }

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CPOWER_TYPE:
	    if (Name != NULL)
	        IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    else
	        IPCnvPrintf(" ) )");
	    break;
	case CAGD_CBSPLINE_TYPE:
	    IPCnvPrintf(" ),\n");
	    IPCnvKnotVector(NewIndent, Crv -> KnotVector, Crv -> Periodic,
			    CAGD_CRV_PT_LST_LEN(Crv), Crv -> Order);
	    if (Name != NULL)
	        IPCnvPrintf(" )%c\n", IPCnvDelimitChar);
	    else
	        IPCnvPrintf(" )");
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one surface to .irt style.	                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of surface to convert to .irt style.   	             *
*		  If NULL, assumed to be a part of high level geometry such  *
*		  as a trimmed surface.					     *
*   Srf:   	  Geometry to convert.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneSurface(const char *Indent,
			    const char *Name,
			    CagdSrfStruct *Srf)
{
    int i, j, k;
    char NewIndent[IRIT_LINE_LEN];
    CagdVType ExtDir;
    CagdCrvStruct *Crv1, *Crv2;

    sprintf(NewIndent, "%s    ", Indent);

    /* Check if it is a bilinear surface and dump as such, if so. */
    if (Srf -> UOrder == 2 && Srf -> VOrder == 2 &&
	Srf -> ULength == 2 && Srf -> ULength == 2 &&
	(Srf -> GType == CAGD_SBEZIER_TYPE ||
	 (Srf -> GType == CAGD_SBSPLINE_TYPE &&
	  BspIsKnotUniform(CAGD_SRF_UPT_LST_LEN(Srf),
			   Srf -> UOrder,
			   Srf -> UKnotVector) == CAGD_END_COND_OPEN &&
	  BspIsKnotUniform(CAGD_SRF_VPT_LST_LEN(Srf),
			   Srf -> VOrder,
			   Srf -> VKnotVector) == CAGD_END_COND_OPEN))) {
        IPCnvPrintf("%s%sRuledSrf(\n", Indent,
		    Name ? IPCnvAssignName(Name) : "");
	IPCnvCtlPt(NewIndent, Srf -> PType, Srf -> Points, 0);
        IPCnvPrintf(" + ");
	IPCnvCtlPt("", Srf -> PType, Srf -> Points, 1);
	IPCnvPrintf(",\n");
	IPCnvCtlPt(NewIndent, Srf -> PType, Srf -> Points, 2);
        IPCnvPrintf(" + ", Indent);
	IPCnvCtlPt("", Srf -> PType, Srf -> Points, 3);
	if (Name != NULL)
	    IPCnvPrintf(" )%c\n", IPCnvDelimitChar);
	else
	    IPCnvPrintf(" )");
	return;        
    }
    else if ((k = SymbIsExtrusionSrf(Srf, &Crv1, ExtDir,
				      IP_CNV_SRF_RVRS_ENG_EPS)) != 0) {
	IPCnvPrintf("%s%s%sExtrude(\n", Indent,
		    Name ? IPCnvAssignName(Name) : "",
		    k == 1 ? "SReverse( " : "");
	IPCnvOneCurve(NewIndent, NULL, Crv1);
	IPCnvPrintf(",\n%svector( %s, %s, %s ), 0", NewIndent,
		    _IPReal2Str(ExtDir[0]),
		    _IPReal2Str(ExtDir[1]), 
		    _IPReal2Str(ExtDir[2]));
	if (Name != NULL)
	    IPCnvPrintf(" )%s%c\n", k == 1 ? " )" : "", IPCnvDelimitChar);
	else
	    IPCnvPrintf(" )%s", k == 1 ? " )" : "");
	return;        
    }
    else if ((k = SymbIsRuledSrf(Srf, &Crv1, &Crv2,
				 IP_CNV_SRF_RVRS_ENG_EPS)) != 0) {
	IPCnvPrintf("%s%s%sRuledSrf(\n", Indent,
		    Name ? IPCnvAssignName(Name) : "",
		    k == 1 ? "SReverse( " : "");
	IPCnvOneCurve(NewIndent, NULL, Crv1);
	IPCnvPrintf(",\n%s", NewIndent);
	IPCnvOneCurve(NewIndent, NULL, Crv2);
	if (Name != NULL)
	    IPCnvPrintf(" )%s%c\n", k == 1 ? " )" : "", IPCnvDelimitChar);
	else
	    IPCnvPrintf(" )");
	return;        
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    IPCnvPrintf("%s%ssbezier(\n", Indent,
			Name ? IPCnvAssignName(Name) : "");
	    break;
	case CAGD_SBSPLINE_TYPE:
	    IPCnvPrintf("%s%ssbspline( %d, %d, \n", Indent,
			Name ? IPCnvAssignName(Name) : "",
			Srf -> UOrder,
			Srf -> VOrder);
	    break;
	case CAGD_SPOWER_TYPE:
	    IPCnvPrintf("%s%sspower(\n", Indent,
			Name ? IPCnvAssignName(Name) : "");
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNSUPPORT_SRF_TYPE);
	    return;
    }

    /* Print the control mesh: */
    for (i = k = 0; i < Srf -> VLength; i++) {
	for (j = 0; j < Srf -> ULength; j++) {
	    IPCnvPrintf("%s%s", Indent,
			i == 0 && j == 0 ? "    list( " : "          ");
	    IPCnvPrintf("%s", j == 0 ? "list( " : "      ");
	    IPCnvCtlPt("", Srf -> PType, Srf -> Points, k++);
	    IPCnvPrintf("%s", j == Srf -> ULength - 1 ? " )" : ",\n" );
	}
	if (i < Srf -> VLength - 1)
	    IPCnvPrintf(",\n");
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SPOWER_TYPE:
	    if (Name != NULL)
	        IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    else
	        IPCnvPrintf(" ) )");
	    break;
	case CAGD_SBSPLINE_TYPE:
	    IPCnvPrintf(" ),\n%s    list( ", Indent);
	    IPCnvKnotVector("", Srf -> UKnotVector, Srf -> UPeriodic,
			    CAGD_SRF_UPT_LST_LEN(Srf), Srf -> UOrder);
	    IPCnvPrintf(",\n%s          ", Indent);
	    IPCnvKnotVector("", Srf -> VKnotVector, Srf -> VPeriodic,
			    CAGD_SRF_VPT_LST_LEN(Srf), Srf -> VOrder);
	    if (Name != NULL)
	        IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    else
	        IPCnvPrintf(" ) )");
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one trimmed surface to .irt style.	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of trimmed surface to convert to .irt style.          *
*   TrimSrf:  	  Geoemrty to convert.				             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneTrimmedSrf(const char *Indent,
			       const char *Name,
			       TrimSrfStruct *TrimSrf)
{
    CagdCrvStruct *TrimCrv, *TrimCrvs;
    char NewIndent[IRIT_LINE_LEN], NewIndent2[IRIT_LINE_LEN];

    /* Examine for a degenerate trimmed surface that covers the entire       */
    /* domain and convert to a regular surface if so.		             */
    if (TrimSrfTrimCrvAllDomain(TrimSrf)) {
        IPCnvOneSurface(Indent, Name, TrimSrf -> Srf);
        return;
    }

    TrimSrf = TrimAffineTransTrimSrf(TrimSrf, 0.0, 1.0, 0.0, 1.0);
    TrimCrvs = TrimGetTrimmingCurves(TrimSrf, TRUE, TRUE);

    sprintf(NewIndent, "%s    ", Indent);
    sprintf(NewIndent2, "%s        ", Indent);

    IPCnvPrintf("%s%strimsrf(\n", Indent, IPCnvAssignName(Name));
    IPCnvOneSurface(NewIndent, NULL, TrimSrf -> Srf);
    IPCnvPrintf(",\n%slist(\n", NewIndent);
    for (TrimCrv = TrimCrvs; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext) {
        if (TrimCrv -> Length > GlblLeastSquaresMinLenFit) {
	    CagdCrvStruct
		*FitCrv = IPCnvLeastSquaresFitCrv(TrimCrv);

	    IPCnvOneCurve(NewIndent2, NULL, FitCrv);
	    CagdCrvFree(FitCrv);
	}
	else
	    IPCnvOneCurve(NewIndent2, NULL, TrimCrv);

	if (TrimCrv -> Pnext == NULL)
	    IPCnvPrintf(" ),\n%strue )%c\n", NewIndent, IPCnvDelimitChar);
	else
	    IPCnvPrintf(",\n");
    }

    CagdCrvFreeList(TrimCrvs);
    TrimSrfFree(TrimSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one trivariate function to .irt style.	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of trivariate to convert to .irt style.               *
*   Trivar:   	  Geometry to convert.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneTrivar(const char *Indent,
			   const char *Name,
			   TrivTVStruct *Trivar)
{
    int i, j, k, l;
    char NewIndent[IRIT_LINE_LEN];

    sprintf(NewIndent, "%s    ", Indent);

    switch (Trivar -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    IPCnvPrintf("%s%stbezier(\n", Indent, IPCnvAssignName(Name));
	    break;
	case TRIV_TVBSPLINE_TYPE:
	    IPCnvPrintf("%s%stbspline( %d, %d, %d,\n", Indent,
			IPCnvAssignName(Name),
		    Trivar -> UOrder, Trivar -> VOrder, Trivar -> WOrder);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNSUPPORT_TV_TYPE);
	    return;
    }

    /* Print the control mesh: */
    for (k = l = 0; k < Trivar -> WLength; k++) {
	for (j = 0; j < Trivar -> VLength; j++) {
	    for (i = 0; i < Trivar -> ULength; i++) {
		IPCnvPrintf("%s%s", NewIndent,
			    i == 0 && j == 0 && k == 0 ? "list( "
						       : "      ");
		IPCnvPrintf("%s", j == 0 && i == 0 ? "list( " : "      ");
		IPCnvPrintf("%s", i == 0 ? "list( " : "      ");
		IPCnvCtlPt("", Trivar -> PType, Trivar -> Points, l++);
		IPCnvPrintf("%s", i == Trivar -> ULength - 1 ? " )" : ",\n" );
	    }
	    IPCnvPrintf("%s", j == Trivar -> VLength - 1 ? " )" : ",\n" );
	}
	if (k < Trivar -> WLength - 1)
	    IPCnvPrintf(",\n");
    }

    switch (Trivar -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    break;
	case TRIV_TVBSPLINE_TYPE:
	    IPCnvPrintf(" ),\n%s    list( ", Indent);
	    IPCnvKnotVector("", Trivar -> UKnotVector, Trivar -> UPeriodic,
			    TRIV_TV_UPT_LST_LEN(Trivar), Trivar -> UOrder);
	    IPCnvPrintf(",\n%s          ", Indent);
	    IPCnvKnotVector("", Trivar -> VKnotVector, Trivar -> VPeriodic,
			    TRIV_TV_VPT_LST_LEN(Trivar), Trivar -> VOrder);
	    IPCnvPrintf(",\n%s          ", Indent);
	    IPCnvKnotVector("", Trivar -> WKnotVector, Trivar -> WPeriodic,
			    TRIV_TV_WPT_LST_LEN(Trivar), Trivar -> WOrder);
	    IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one multivariate function to .irt style.	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of multivariate to convert to .irt style.             *
*   Multivar:  	  Geometry to convert.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneMultiVar(const char *Indent,
			     const char *Name,
			     MvarMVStruct *MultiVar)
{
    char AuxName[IRIT_LINE_LEN];
    CagdCrvStruct *Crv;
    CagdSrfStruct *Srf;
    TrivTVStruct *TV;

    sprintf(AuxName, "%sAux", Name);

    switch (MultiVar -> Dim) {
	case 1:
	    Crv = MvarMVToCrv(MultiVar);
	    IPCnvOneCurve(Indent, AuxName, Crv);
	    CagdCrvFree(Crv);
	    IPCnvPrintf("%s%scoerce( %s, multivar_type )%c\nfree( %s )%c\n",
			Indent,
			IPCnvAssignName(Name), AuxName, IPCnvDelimitChar,
			AuxName, IPCnvDelimitChar);
	    break;
	case 2:
	    Srf = MvarMVToSrf(MultiVar);
	    IPCnvOneSurface(Indent, AuxName, Srf);
	    CagdSrfFree(Srf);
	    IPCnvPrintf("%s%scoerce( %s, multivar_type )%c\nfree( %s )%c\n",
			Indent,
			IPCnvAssignName(Name), AuxName, IPCnvDelimitChar,
			AuxName, IPCnvDelimitChar);
	    break;
	case 3:
	    TV = MvarMVToTV(MultiVar);
	    IPCnvOneTrivar(Indent, AuxName, TV);
	    TrivTVFree(TV);
	    IPCnvPrintf("%s%scoerce( %s, multivar_type )%c\nfree( %s )%c\n",
			Indent,
			IPCnvAssignName(Name), AuxName, IPCnvDelimitChar,
			AuxName, IPCnvDelimitChar);
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one triangular surface to .irt style.	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of triangular surface to convert to .irt style.       *
*   Trivar:   	  Geometry to convert.			                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvOneTriSrf(const char *Indent,
			   const char *Name,
			   TrngTriangSrfStruct *TriSrf)
{
    int i;
    char NewIndent[IRIT_LINE_LEN], Indent2[IRIT_LINE_LEN];

    sprintf(NewIndent, "%s    ", Indent);
    sprintf(Indent2, "%s          ", Indent);

    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	    IPCnvPrintf("%s%stsbezier( %d,\n", Indent,
			IPCnvAssignName(Name), TriSrf -> Length);
	    break;
	case TRNG_TRISRF_BSPLINE_TYPE:
	    IPCnvPrintf("%s%stsbspline( %d %d,\n", Indent,
			IPCnvAssignName(Name), TriSrf -> Order,
			TriSrf -> Length);
	    break;
	case TRNG_TRISRF_GREGORY_TYPE:
	    IPCnvPrintf("%s%stsgregory( %d,\n", Indent,
			IPCnvAssignName(Name), TriSrf -> Length);
	    break;
	default:
	    IP_FATAL_ERROR(IP_ERR_UNSUPPORT_TRNG_TYPE);
	    return;
    }

    /* Print the control mesh: */
    IPCnvPrintf("%slist( ", NewIndent);
    for (i = 0; i < TRNG_TRISRF_MESH_SIZE(TriSrf); i++) {
	IPCnvCtlPt(i > 0 ? Indent2 : "",
		   TriSrf -> PType, TriSrf -> Points, i);
	IPCnvPrintf("%s", i == TRNG_TRISRF_MESH_SIZE(TriSrf) - 1 ? " )"
								 : ",\n" );
    }

    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	case TRNG_TRISRF_GREGORY_TYPE:
	    IPCnvPrintf(" )%c\n", IPCnvDelimitChar);
	    break;
	case TRNG_TRISRF_BSPLINE_TYPE:
	    IPCnvPrintf(",\n%s    list( ", Indent);
	    IPCnvKnotVector("", TriSrf -> KnotVector, FALSE,
			    TriSrf -> Length, TriSrf -> Order);
	    IPCnvPrintf(" ) )%c\n", IPCnvDelimitChar);
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Convert one model object.		                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   Name:	  Name of model geometry to convert to .irt style.           *
*   Mdl:          Geometry to convert.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPCnvOneModel(const char *Indent,
			  const char *Name,
			  MdlModelStruct *Mdl)
{
    IP_FATAL_ERROR(IP_ERR_NOT_SUPPORT_CNVRT_IRT);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one control point.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   PType:	  Type of control point.				     *
*   Points:	  The coefficients of the control points (possibly from a    *
*		  curve or surface).					     *
*   Index:   	  Into Points. -1 denotes Points are in fact (IrtRType *).   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void IPCnvCtlPt(const char *Indent,
		       CagdPointType PType,
		       IrtRType **Points,
		       int Index)
{
    int i,
	IsRational = CAGD_IS_RATIONAL_PT(PType),
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);

    /* Eliminate trailing zeros by decreasing MaxCoord. */
    if (Index == -1) {
	IrtRType
	    *Pt = *Points;

	while (MaxCoord > 1 && IRIT_APX_EQ(Pt[MaxCoord], 0.0))
	    MaxCoord--;
    }
    else if (Index > 0) {   /* Leave Index 0 to make sure we recover PType. */
	while (MaxCoord > 1 && IRIT_APX_EQ(Points[MaxCoord][Index], 0.0))
	    MaxCoord--;
    }

    IPCnvPrintf("%sctlpt( ", Indent);
    IPCnvPrintf(IsRational ? "P%d" : "E%d", MaxCoord);

    if (Index == -1) {
	IrtRType
	    *Pt = *Points;

	for (i = !IsRational; i <= MaxCoord; i++)
	    IPCnvPrintf(", %s", _IPReal2Str(Pt[i]));
    }
    else {
	for (i = !IsRational; i <= MaxCoord; i++)
	    IPCnvPrintf(", %s", _IPReal2Str(Points[i][Index]));
    }

    IPCnvPrintf(Index == -1 ? " );" : " )");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts one knot vector.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Indent:       Level of indentation.                                      *
*   KnotVector:   Coefficients of knot vector.                               *
*   Periodic:	  TRUE if this direction is periodic.                        *
*   Len:          Length of control mesh/point in this direction	     *
*   Order:        Of Bspline function in this direction.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPCnvKnotVector(const char *Indent,
			    IrtRType *KnotVector,
			    CagdBType Periodic,
			    int Len,
			    int Order)
{
    int i,
	Length = Len + Order;

    IPCnvPrintf("%slist( ", Indent);

    switch (BspIsKnotUniform(Len, Order, KnotVector)) {
        case CAGD_END_COND_OPEN:
            IPCnvPrintf("kv_open ");
	    break;
	case CAGD_END_COND_FLOAT:
	    IPCnvPrintf(Periodic ? "kv_periodic " : "kv_float ");
	    break;
	case CAGD_END_COND_GENERAL:
	default:
	    if (BspIsKnotDiscontUniform(Len, Order, KnotVector) ==
							    CAGD_END_COND_OPEN)
	        IPCnvPrintf("kv_disc_open ");
	    else {
	        for (i = 0; i < Length; i++) {
		    IPCnvPrintf("%s", _IPReal2Str(KnotVector[i]));
		    if (i < Length - 1)
		        if (i > 0 && (i & 7) == 0)
			    IPCnvPrintf(",\n\t\t\t\t");
			else
			    IPCnvPrintf(", ");
		    else
		        IPCnvPrintf(" ");
		}
	    }
	    break;
    }

    IPCnvPrintf(")");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates assignment to the given name if required.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Name:   Name to assigne to.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:                                                                  *
*****************************************************************************/
static const char *IPCnvAssignName(const char *Name)
{
    static char Assignment[IRIT_LINE_LEN_VLONG];

    if (GlblDumpAssignName) {
	sprintf(Assignment, "%s = ", Name);
	return Assignment;
    }
    else
        return "";
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Least squares fit a given curve and reduce it as a result.               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:     Curve to least squares fit.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  Least squares reduced curve.                           *
*****************************************************************************/
static CagdCrvStruct *IPCnvLeastSquaresFitCrv(CagdCrvStruct *Crv)
{
    CagdPointType
	PType = Crv -> PType;
    int Periodic = Crv -> Periodic,
	Len = (int) (Crv -> Length * GlblLeastSquaresPercent / 100.0),
	SampLen = Len * 10,
	PtSize = (CAGD_NUM_OF_PT_COORD(PType) + 1) * sizeof(CagdRType);
    CagdRType TMin, TMax, t, dt, *R, Err, MaxError;
    CagdPType Pt1, Pt2;
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;
    CagdCrvStruct *FitCrv;

    CagdCrvDomain(Crv, &TMin, &TMax);
    dt = (TMax - TMin) / (SampLen + 1 );

    /* Sample the curve. */
    for (t = TMin; t <= TMax + dt * 0.5; t += dt) {
	if (t > TMax)
	    t = TMax;

	R = CagdCrvEval(Crv, t);

	if (CtlPtList == NULL)
	    CtlPtList = CtlPt = CagdCtlPtNew(PType);
	else {
	    CtlPt -> Pnext = CagdCtlPtNew(PType);
	    CtlPt = CtlPt -> Pnext;
	}

	/* Copy the point: */
	IRIT_GEN_COPY(CtlPt -> Coords, R, PtSize);
	CtlPt -> PtType = PType;
    }

    if (!Periodic) {
	/* Lets see if first point equal last point. */
        R = CagdCrvEval(Crv, TMin);
	CagdCoerceToE3(Pt1, &R, -1, PType);
        R = CagdCrvEval(Crv, TMax);
	CagdCoerceToE3(Pt2, &R, -1, PType);
	Periodic = IRIT_PT_APX_EQ(Pt1, Pt2);
    }

    FitCrv = BspCrvInterpPts2(CtlPtList, Crv -> Order, Len,
			      CAGD_UNIFORM_PARAM, Periodic);
    if (Periodic) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtPeriodic2FloatCrv(FitCrv);

	CagdCrvFree(FitCrv);

	FitCrv = CagdCnvrtFloat2OpenCrv(TCrv);
	CagdCrvFree(TCrv);
    }

    BspKnotAffineTransOrder2(FitCrv -> KnotVector, FitCrv -> Order,
			     FitCrv -> Order + FitCrv -> Length,
			     TMin, TMax);
    MaxError = 0.0;

    /* Sample the curve. */
    for (t = TMin, CtlPt = CtlPtList;
	 t <= TMax + dt * 0.5;
	 CtlPt = CtlPt -> Pnext, t += dt) {
	if (t > TMax)
	    t = TMax;

	R = CagdCrvEval(FitCrv, t);

	Err = IRIT_SQR(R[1] - CtlPt -> Coords[1]) +
	      IRIT_SQR(R[2] - CtlPt -> Coords[2]);

	if (MaxError < Err)
	    MaxError = Err;
    }

    CagdCtlPtFreeList(CtlPtList);

    if (MaxError < GlblLeastSquaresMaxError)
        return FitCrv;
    else {
	CagdCrvFree(FitCrv);
	return CagdCrvCopy(Crv);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare two vertices if same or not.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   VP1, VP2:   Two vertices to compare.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0, +1 if V1 less, equal, greater than V2.                  *
*****************************************************************************/
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2)
{
    int i;
    IPVertexStruct
	*V1 = (IPVertexStruct *) VP1,
	*V2 = (IPVertexStruct *) VP2;
    IrtRType
	*Coord1 = V1 -> Coord,
	*Coord2 = V2 -> Coord;

    if (IRIT_PT_APX_EQ_EPS(Coord1, Coord2, IP_VRTX_SAME_EPS)) {
	if (GlblIdenticalVertexAttribMask & 0x01) {  /* Compare normals. */
	  if (IP_HAS_NORMAL_VRTX(V1) &&
	      IP_HAS_NORMAL_VRTX(V2) &&
	      !IRIT_PT_APX_EQ_EPS(V1 -> Normal, V2 -> Normal, IP_VRTX_SAME_EPS)) {
	        for (i = 0; i < 3; i++) {
		    if (V1 -> Normal[i] < V2 -> Normal[i])
		        return -1;
		    else if (V1 -> Normal[i] > V2 -> Normal[i])
		        return 1;
		}
	    }
	}
	if (GlblIdenticalVertexAttribMask & 0x02) {  /* Compare uvvals. */
	    float
		*Uv1 = AttrGetUVAttrib(V1 -> Attr, "uvvals"),
		*Uv2 = AttrGetUVAttrib(V2 -> Attr, "uvvals");

	    if (Uv1 != NULL &&
		Uv2 != NULL &&
		!IRIT_PT_APX_EQ_E2_EPS(Uv1, Uv2, IP_VRTX_SAME_EPS)) {
	        for (i = 0; i < 2; i++) {
		    if (Uv1[i] < Uv2[i])
		        return -1;
		    else if (Uv1[i] > Uv2[i])
		        return 1;
		}
	    }
	}

	if (GlblIdenticalVertexAttribMask & 0x04) {  /* Compare rgb. */
	    int RGB1[3], RGB2[3];

	    if (AttrGetRGBColor(V1 -> Attr, &RGB1[0], &RGB1[1], &RGB1[2]) &&
		AttrGetRGBColor(V2 -> Attr, &RGB2[0], &RGB2[1], &RGB2[2]) &&
		!IRIT_PT_APX_EQ_EPS(RGB1, RGB2, IP_VRTX_SAME_EPS)) {
	        for (i = 0; i < 3; i++) {
		    if (RGB1[i] < RGB2[i])
		        return -1;
		    else if (RGB1[i] > RGB2[i])
		        return 1;
		}	        
	    }		
	}

	return 0;
    }
    else {
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
* DESCRIPTION:                                                               M
*   Given a vertex and a mesh in IPPolyVrtxIdxStruct format, find neigbors   M
* upto the prescribed maximal distrance/ring.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PVIdx:      The input mesh to look at.  Assumed that was constructed     M
*		using IPCnvPolyToPolyVrtxIdxStruct with CalcPPolys TRUE.     M
*   VIdx:       The index of the source vertex, zero based.		     M
*   Ring:       maximal topological distance from VIdx.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int *:    A -1 terminated vector holding the indices of neighboring      M
*	      vertices to VIdx, with topological distance of up to Ring.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvPolyToPolyVrtxIdxStruct                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvPolyVrtxNeighbors	                                             M
*****************************************************************************/
int *IPCnvPolyVrtxNeighbors(IPPolyVrtxIdxStruct *PVIdx, int VIdx, int Ring)
{
    GlblCrntNV = 0;

    return IPCnvPolyVrtxNeighborsAux(PVIdx, VIdx, Ring);
}

/*****************************************************************************
* AUXILIARY:                                                                 *
*   Auxiliary function of IPCnvPolyVrtxNeighbors.			     *
*****************************************************************************/
static int *IPCnvPolyVrtxNeighborsAux(IPPolyVrtxIdxStruct *PVIdx,
				      int VIdx,
				      int Ring)
{
    static int 
        *NV = NULL;
    unsigned int
        LastIterBegin = 0,
        CurTopDist = 1;

    if (!PVIdx -> TriangularMesh) {
	IP_FATAL_ERROR(IP_ERR_NEIGH_SEARCH);
        NV[GlblCrntNV] = -1;
	return NV;
    }

    if (Ring <= 0 || PVIdx -> PPolys == NULL) {
        NV[GlblCrntNV] = -1;
	return NV;
    }
	
    VIdx = IRIT_ABS(VIdx);

    IPCnvAddVertNeighbAux(PVIdx, &NV, VIdx, VIdx);
    while (--Ring) {
        unsigned int 
	    i = LastIterBegin,
	    LastIterEnd = GlblCrntNV;

	LastIterBegin = GlblCrntNV;
	++CurTopDist;
	for (; i < LastIterEnd; ++i)
	    IPCnvAddVertNeighbAux(PVIdx, &NV, NV[i], VIdx);
    }
	
    NV[GlblCrntNV] = -1;
    return NV;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function adds a vertex neighbors to the neighbors list.             *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:      The input mesh to look at.  Assumed that was constructed     *
*		using IPCnvPolyToPolyVrtxIdxStruct with CalcPPolys TRUE.     *
*   NV:         The current vector of neighboring vertices.                  *
*   VIdx:       The index of the source vertex, zero based.                  *
*   RingOrigin: The index of the ring source vertex, zero based.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPCnvAddVertNeighbAux(IPPolyVrtxIdxStruct *PVIdx,
				  int **NV,
				  unsigned int VIdx,
				  int RingOrigin)
{
    IPPolyPtrStruct *Pls;

    /* Go over all polygons that uses vertex index VIdx. */
    for (Pls = PVIdx -> PPolys[VIdx]; Pls != NULL; Pls = Pls -> Pnext) {
        IPPolygonStruct
	    *Pl = Pls -> Poly;
	IPVertexStruct
	    *VHead = Pl -> PVertex,
	    *V = VHead;

	do {
	    int Idx = AttrGetIntAttrib(V -> Attr, "_VIdx");
	    
	    Idx = IRIT_ABS(Idx) - 1;
	    
	    if (Idx != RingOrigin) 
	      IPCnvPolyVrtxNeighborsInsV(NV, &GlblCrntNV, Idx);
	    V = V -> Pnext;
	}
	while (V != NULL && V != VHead);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts a new neighbor into the vector of neighboring vertices.          *
*                                                                            *
* PARAMETERS:                                                                *
*   NV:     The current vector of neighboring vertices.                      *
*   CrntNV: The currently inserted NV.                                       *
*   NewIdx: New index to insert.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   FALSE if NewIdx already in NV, TRUE if new and inserted.          *
*****************************************************************************/
static int IPCnvPolyVrtxNeighborsInsV(int **NV, int *CrntNV, int NewIdx)
{
    static int
	MaxNV = 0;
    int i, *NVPtr;

    /* Make sure we have a proper vector size. */
    if (*CrntNV + 1 >= MaxNV) {
	if (*NV == NULL) {
	    MaxNV = 100;
	    *NV = (int *) IritMalloc(sizeof(int) * MaxNV);
	}
	else {
	    int NewMax = MaxNV * 2;

	    *NV = (int *) IritRealloc(*NV, sizeof(int) * MaxNV,
				      sizeof(int) * NewMax);
	    MaxNV = NewMax;
	}
    }
    NVPtr = *NV;

    for (i = 0; i < *CrntNV; i++) {
	if (NVPtr[i] == NewIdx)
	    return FALSE;
    }

    NVPtr[(*CrntNV)++] = NewIdx;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Process a given polygonal model into a vertex list with each polygon     M
* having indices into the vertex list.  All list are zero based.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        A polygonal mesh to convert to PolyIdx structure.           M
*   CalcPPolys:  TRUE if a polygon pointer list is to be calculated, FALSE   M
*                otherwise.						     M
*   AttribMask:  Sets what attributes to consider when comparing for	     M
*		 identical vertices -					     M
*		 Bit 0 - normals should be identical in identical vertices.  M
*		 Bit 1 - uvvals should be identical in identical vertices.   M
*		 Bit 2 - rgb should be identical in identical vertices.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolyVrtxIdxStruct *:    The polygonal mesh as PolyIdx struct.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCnvPolyVrtxNeighbors	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCnvPolyToPolyVrtxIdxStruct                                             M
*****************************************************************************/
IPPolyVrtxIdxStruct *IPCnvPolyToPolyVrtxIdxStruct(const IPObjectStruct *PObj,
						  int CalcPPolys,
						  int AttribMask)
{
    int n, *VIdxPtr, **PIdx, NumVertices;
    IrtRType Min, Max;
    IPVertexStruct *PVertex, **VIdx;
    IPPolygonStruct *PPolygon;
    GMBBBboxStruct 
	*BBox = GMBBComputeBboxObject(PObj);
    IritHashTableStruct *IHT;
    IPPolyVrtxIdxStruct *PVIdx;

    GlblIdenticalVertexAttribMask = AttribMask;

    /* Create a hash table to hold vertices and detect identities. */
    Min = IRIT_MIN(IRIT_MIN(BBox -> Min[0], BBox -> Min[1]), BBox -> Min[2]);
    Max = IRIT_MAX(IRIT_MAX(BBox -> Max[0], BBox -> Max[1]), BBox -> Max[2]);
    IHT = IritHashTableCreate(Min, Max, IP_VRTX_SAME_EPS,
			      IPPolyListLen(PObj -> U.Pl));

    /* Insert vertices into hash table, only one of each identical vertex. */
    for (PPolygon = PObj -> U.Pl, n = 0;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext) {
        AttrSetIntAttrib(&PPolygon -> Attr, "_PIdx", n++);

	PVertex = PPolygon -> PVertex;
	do {
	    IritHashTableInsert(IHT, PVertex, CmpTwoVertices,
				IHT_VERTEX_KEY(PVertex -> Coord), FALSE);

	    /* Make sure no old data is kept here... */
	    AttrFreeOneAttribute(&PVertex -> Attr, "_VIdx");

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPolygon -> PVertex);
    }

    /* Accumulate vertices so that each identical vertex shows up only once. */
    NumVertices = 0;
    for (PPolygon = PObj -> U.Pl;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext) {
	/* Assume at least one edge in polygon! */		 
        PVertex = PPolygon -> PVertex;
	do {
	    IPVertexStruct *V;
	    int Idx;

	    if ((V = (IPVertexStruct *)
		    IritHashTableFind(IHT, PVertex, CmpTwoVertices,
				      IHT_VERTEX_KEY(PVertex -> Coord)))
	       							     == NULL) {
	        IP_FATAL_ERROR(IP_ERR_VRTX_HASH_FAILED);
	    }
	    Idx = AttrGetIntAttrib(V -> Attr, "_VIdx");

	    if (IP_ATTR_IS_BAD_INT(Idx)) {
		/* It is a new vertex - number it. */
		AttrSetIntAttrib(&V -> Attr, "_VIdx", Idx = ++NumVertices);
	    }

	    if (PVertex != V) {
	        /* Keep (negative) index of old, already visited, vertex. */
		AttrSetIntAttrib(&PVertex -> Attr, "_VIdx", -Idx);
	    }

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPolygon -> PVertex);
    }

    IritHashTableFree(IHT);

    /* Allocate the structure and copy the vertices in. */
    PVIdx = IPPolyVrtxIdxNew(NumVertices, IPPolyListLen(PObj -> U.Pl));
    VIdx = PVIdx -> Vertices;
    PIdx = PVIdx -> Polygons;
    PVIdx -> PObj = PObj;

#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPolyVrtxIndx1, FALSE) {
	    IRIT_ZAP_MEM(VIdx,
			 sizeof(IPVertexStruct *) * (PVIdx -> NumVrtcs + 1));
	    IRIT_ZAP_MEM(PIdx, sizeof(int *) * (PVIdx -> NumPlys + 1));
	}
    }
#   endif /* DEBUG */

    for (PPolygon = PObj -> U.Pl, n = 0;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext, n++) {
	/* Assume at least one edge in polygon! */		 
        PVertex = PPolygon -> PVertex;
	do {
	    int Idx = AttrGetIntAttrib(PVertex -> Attr, "_VIdx");

	    n++;

	    if (Idx > 0)
	        VIdx[Idx - 1] = PVertex;

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPolygon -> PVertex);
    }
    VIdx[PVIdx -> NumVrtcs] = NULL;

    /* Allocate the polygons as indices into vertices. */
    PVIdx -> _AuxVIndices = (int *) IritMalloc(sizeof(int) * n);
    PVIdx -> TriangularMesh = TRUE;

    for (PPolygon = PObj -> U.Pl, n = 0, VIdxPtr = PVIdx -> _AuxVIndices;
	 PPolygon != NULL;
	 PPolygon = PPolygon -> Pnext) {
        /* Assume at least one edge in polygon! */
        int NumVrtcs = IPVrtxListLen(PVertex = PPolygon -> PVertex);

	if (NumVrtcs > 3)
	    PVIdx -> TriangularMesh = FALSE;

	PIdx[n++] = VIdxPtr;

        PVertex = PPolygon -> PVertex;
	do {
	    int Idx = (int) AttrGetIntAttrib(PVertex -> Attr, "_VIdx");

	    *VIdxPtr++ = IRIT_ABS(Idx) - 1;

	    PVertex = PVertex -> Pnext;
	}
	while (PVertex != NULL && PVertex != PPolygon -> PVertex);
	*VIdxPtr++ = -1;
    }
    PIdx[n++] = NULL;

#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPolyVrtxIndx2, FALSE) {
	    int i;

	    for (i = 0; i < PVIdx -> NumVrtcs; i++) {
	        if (VIdx[i] == NULL)
		    IRIT_INFO_MSG_PRINTF(
			    "POLYVRTX: Failed to assign a vertex %d\n", i);
	    }
	    if (VIdx[PVIdx -> NumVrtcs] != NULL)
	        IRIT_INFO_MSG_PRINTF(
			"POLYVRTX: Expecting terminating NULL vertex\n");

	    for (i = 0; i < PVIdx -> NumPlys; i++) {
	        if (PIdx[i] == NULL)
		    IRIT_INFO_MSG_PRINTF(
			    "POLYVRTX: Failed to assign a polygon %d\n", i);
	    }
	    if (PIdx[PVIdx -> NumPlys] != NULL)
	        IRIT_INFO_MSG_PRINTF(
			"POLYVRTX: Expecting terminating NULL polygon\n");
	}
    }
#   endif /* DEBUG */

    /* Calculate PPolys. */
    if (CalcPPolys) {
        int VIndex;
        IPPolygonStruct *P;
        IPVertexStruct *V;
        IPPolyPtrStruct *PPoly;

        PVIdx -> PPolys =
	    IritMalloc(sizeof(IPPolyPtrStruct *) * (PVIdx -> NumVrtcs + 1));
	IRIT_ZAP_MEM(PVIdx -> PPolys,
		sizeof(IPPolyPtrStruct *) * (PVIdx -> NumVrtcs + 1));

        for (P = PObj -> U.Pl; P != NULL; P = P -> Pnext) {
            V = P -> PVertex;
	    do {
		VIndex = AttrGetIntAttrib(V -> Attr, "_VIdx");
		VIndex = IRIT_ABS(VIndex) - 1;
                PPoly = IritMalloc(sizeof(IPPolyPtrStruct));
                PPoly -> Poly = P;
                PPoly -> Pnext = PVIdx -> PPolys[VIndex];
                PVIdx -> PPolys[VIndex] = PPoly;
		V = V -> Pnext;
            }
	    while (V != NULL && V != P -> PVertex);
        }
    }

    return PVIdx;
}
