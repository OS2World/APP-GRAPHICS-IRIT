/*****************************************************************************
* Module to save IRIT data files as AutoCad DXF files.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

IRIT_STATIC_DATA int
    GlblDXFDumpFreeForms = TRUE;

IRIT_STATIC_DATA FILE
    *OutputFile = NULL;

static void IPDumpDXFTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void IPDumpDXFObject(FILE *f, const IPObjectStruct *PObject);
static void IPDumpDXFPoly(FILE *f,
			  const IPPolygonStruct *PPoly,
			  const char *ObjName,
			  const IPObjectStruct *PObj);
static void IPDumpDXFPolygon(FILE *f,
			     const IPPolygonStruct *PPolygon,
			     const char *ObjName);
static void IPDumpDXFPolyline(FILE *f,
			      const IPPolygonStruct *PPolyline,
			      const char *ObjName);
static void IPDumpDXFPointlist(FILE *f,
			       const IPPolygonStruct *PPointlist,
			       const char *ObjName);
static void IPDumpDXFSrf(FILE *f,
			 const CagdSrfStruct *Srf,
			 const char *ObjName);

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for dxf into DXFFileName (stdout in NULL).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:          To dump into file.                                        *
*   DXFFileName:   Where output should go to, "-" or NULL for stdout.        *
*   DumpFreeForms: TRUE to dump freeforms as splines, FALSE as polys.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if successful, FALSE otherwise.                            *
*****************************************************************************/
int IPDXFSaveFile(const IPObjectStruct *PObj,
		  const char *DXFFileName,
		  int DumpFreeForms)
{
    int i;
    IRIT_STATIC_DATA char *Header[] = {
	"  0",
	"SECTION",
	"  2",
	"HEADER",
	"999",
	"Creator: IRIT solid modeller using irit2dxf filter.",
	"  0",
	"ENDSEC",
	"  0",
	"SECTION",
	"  2",
	"ENTITIES",
	NULL
    };
    IRIT_STATIC_DATA char *Epilog[] = {
	"  0",
	"ENDSEC",
	"  0",
	"EOF",
	NULL
    };
    IrtHmgnMatType UnitMat;
    IPObjectStruct
        *PObject = IPCopyObject(NULL, PObj, TRUE);

    if (DXFFileName != NULL && strncmp(DXFFileName, "-", 1) != 0) {
	if ((OutputFile = fopen(DXFFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", DXFFileName);
	    return FALSE;
	}
    }
    else
	OutputFile = stdout;

    for (i = 0; Header[i] != NULL; i++)
	fprintf(OutputFile, "%s\n", Header[i]);

    MatGenUnitMat(UnitMat);
    GlblDXFDumpFreeForms = DumpFreeForms;
    IPTraverseObjListHierarchy(PObject, UnitMat, IPDumpDXFTraversedObject);

    for (i = 0; Epilog[i] != NULL; i++)
	fprintf(OutputFile, "%s\n", Epilog[i]);

    fclose(OutputFile);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:          Non list object to handle.                                *
*   Mat:           Transformation matrix to apply to this object.            *
*   DumpFreeForms: TRUE to dump freeforms as splines, FALSE as polys.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPDumpDXFTraversedObject(IPObjectStruct *PObj,
				     IrtHmgnMatType Mat)
{
    IPObjectStruct *PObjs;

    if (!GlblDXFDumpFreeForms && IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext)
        IPDumpDXFObject(OutputFile, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to file f.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPDumpDXFObject(FILE *f, const IPObjectStruct *PObject)
{
    IPPolygonStruct *PList;
    CagdSrfStruct *Srfs;
    TrimSrfStruct *TrimSrfs;

    switch (PObject -> ObjType) {
	case IP_OBJ_POLY:
	    PList = PObject -> U.Pl;
	    while (PList) {
		IPDumpDXFPoly(f, PList,
			      IP_VALID_OBJ_NAME(PObject) ?
				             IP_GET_OBJ_NAME(PObject) : "IRIT",
			      PObject);
		PList =	PList -> Pnext;
	    }
	    break;
	case IP_OBJ_CURVE:
	    IRIT_WARNING_MSG_PRINTF(
		"Curve \"%s\" was not dumped - no curve conversion support\n",
		IP_GET_OBJ_NAME(PObject));
	    break;
	case IP_OBJ_SURFACE:
	    Srfs = PObject -> U.Srfs;
	    while (Srfs) {
		IPDumpDXFSrf(f, Srfs, IP_VALID_OBJ_NAME(PObject) ?
			                    IP_GET_OBJ_NAME(PObject) : "IRIT");
	        Srfs = Srfs -> Pnext;
	    }
	    break;
	case IP_OBJ_TRIMSRF:
	    IRIT_WARNING_MSG_PRINTF(
		"Trimmed surface \"%s\" was converted untrimmed\n",
		IP_GET_OBJ_NAME(PObject));
	    TrimSrfs = PObject -> U.TrimSrfs;
	    while (TrimSrfs) {
		IPDumpDXFSrf(f, TrimSrfs -> Srf, IP_VALID_OBJ_NAME(PObject) ?
			                    IP_GET_OBJ_NAME(PObject) : "IRIT");
	        TrimSrfs = TrimSrfs -> Pnext;
	    }
	    break;
	case IP_OBJ_TRIVAR:
	    IRIT_WARNING_MSG_PRINTF(
		"Trivariate \"%s\" cannot be converted and is ignored\n",
		IP_GET_OBJ_NAME(PObject));
	    break;
	case IP_OBJ_TRISRF:
	    IRIT_WARNING_MSG_PRINTF(
		"Triangular surface \"%s\" cannot be converted and is ignored\n",
		IP_GET_OBJ_NAME(PObject));
	    break;
	case IP_OBJ_MULTIVAR:
	    IRIT_WARNING_MSG_PRINTF(
		"Multivariate \"%s\" cannot be converted and is ignored\n",
		IP_GET_OBJ_NAME(PObject));
	    break;
	default:
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon/line. Polygon must be convex.                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PPoly:        Poly to dump to file f.                                    *
*   ObjName:      Name of object (FACE).                                     *
*   PObj:         Object PPoly came from to idetify a polygon from polyline. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void				                                     *
*****************************************************************************/
static void IPDumpDXFPoly(FILE *f,
			  const IPPolygonStruct *PPoly,
			  const char *ObjName,
			  const IPObjectStruct *PObj)
{
    if (IP_IS_POLYGON_OBJ(PObj))
	IPDumpDXFPolygon(f, PPoly, ObjName);
    else if (IP_IS_POLYLINE_OBJ(PObj))
	IPDumpDXFPolyline(f, PPoly, ObjName);
    else if (IP_IS_POINTLIST_OBJ(PObj))
	IPDumpDXFPointlist(f, PPoly, ObjName);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump polygon to.  		                     *
*   PPolygon:     Polygon to dump to file f.                                 *
*   ObjName:      Name of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void				                                     *
*****************************************************************************/
static void IPDumpDXFPolygon(FILE *f,
			     const IPPolygonStruct *PPolygon,
			     const char *ObjName)
{
    int i, j;
    IrtRType *Pts[4];
    IPVertexStruct *VFirst, *V1, *V2, *V3, *VLast,
	*VList = PPolygon -> PVertex;

    if (VList == NULL)
	return;

    /* Make sure the vertex list is not circular, temporarily. */
    if ((VLast = IPGetLastVrtx(VList)) != NULL && VLast -> Pnext == VList)
        VLast -> Pnext = NULL;
    else
        VLast = NULL;

    if (!GMIsConvexPolygon2(PPolygon)) {
	IRIT_STATIC_DATA int Printed = FALSE;

	if (!Printed) {
	    IRIT_WARNING_MSG(
		    "\nWARNING: Non convex polygon(s) might be in data (see CONVEX in IRIT),\n\t\t\t\toutput can be wrong as the result!\n");
	    Printed = TRUE;
	}
    }

    VFirst = VList;
    V1 = VFirst -> Pnext;
    V2 = V1 -> Pnext;
    V3 = V2 -> Pnext;

    while (V2 != NULL) {
	Pts[0] = VFirst -> Coord;
	Pts[1] = V1 -> Coord;
	Pts[2] = V2 -> Coord;
	if (V3 != NULL)
	    Pts[3] = V3 -> Coord;
	else
	    Pts[3] = V2 -> Coord; /* Duplicate fourth pt. */

	if (!IRIT_PT_APX_EQ(Pts[0], Pts[1]) &&
	    !IRIT_PT_APX_EQ(Pts[0], Pts[2]) &&
	    !IRIT_PT_APX_EQ(Pts[1], Pts[2])) {
	    int HiddenEdges = (IP_IS_INTERNAL_VRTX(V1) ? 2 : 0) +
			      (IP_IS_INTERNAL_VRTX(V2) ? 4 : 0);

	    /* The last edge that closes the poly is always hidden since    */
	    /* we artificially introduced it here, unless it is the same as */
	    /* the edge of the original polygon, if it is the last edge.    */
	    if (V3 != NULL &&
		(V3 -> Pnext != NULL || IP_IS_INTERNAL_VRTX(V3)))
		HiddenEdges |= 8;

	    /* The first edge is always hidden since we artificially	    */
	    /* introduced it here, unless it is first edge of orig poly.    */
	    if (V1 == VFirst -> Pnext)
	        HiddenEdges |= IP_IS_INTERNAL_VRTX(VFirst) ? 1 : 0;
	    else
		HiddenEdges |= 1;

	    fprintf(f, "  0\n3DFACE\n  8\n%s\n 70\n%d\n", ObjName, HiddenEdges);

	    for (i = 0; i < 4; i++)
		for (j = 0; j < 3; j++)
		    fprintf(f, " %1d%1d\n%-10.7f\n",
			    j + 1, i, Pts[i][j]);
	}

	if (V3 == NULL) {
	    V1 = V2;
	    V2 = V2 -> Pnext;
	}
	else {
	    V1 = V3;
	    V2 = V3 -> Pnext;
	    if (V2 != NULL)
		V3 = V2 -> Pnext;
	    else
	        V3 = NULL;
	}
    }


    /* Recover circular vertex list if was so. */
    if (VLast != NULL)
        VLast -> Pnext = VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polyline.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump polyline to.  		                     *
*   PPolyline:    Polyline to dump to file f.                                *
*   ObjName:      Name of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void				                                     *
*****************************************************************************/
static void IPDumpDXFPolyline(FILE *f,
			      const IPPolygonStruct *PPolyline,
			      const char *ObjName)
{
    IrtRType *Pts;
    IPVertexStruct
	*VList = PPolyline -> PVertex;

    if (VList == NULL)
	return;

    fprintf(f, "  0\nPOLYLINE\n  8\n%s\n 70\n8\n", ObjName);

    for ( ; VList != NULL; VList = VList -> Pnext) {
	Pts = VList -> Coord;
	fprintf(f, "  0\nVERTEX\n  8\n%s\n 70\n32\n 10\n%f\n 20\n%f\n 30\n%f\n",
		ObjName, Pts[0], Pts[1], Pts[2]);
    }
    fprintf(f, "  0\nSEQEND\n  8\n%s\n", ObjName);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one point list.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump point list to.  		                     *
*   PPointlist:   Point list to dump to file f.                              *
*   ObjName:      Name of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void				                                     *
*****************************************************************************/
static void IPDumpDXFPointlist(FILE *f,
			      const IPPolygonStruct *PPointlist,
			      const char *ObjName)
{
    IrtRType *Pts;
    IPVertexStruct
	*VList = PPointlist -> PVertex;

    if (VList == NULL)
	return;

    for ( ; VList != NULL; VList = VList -> Pnext) {
	Pts = VList -> Coord;
	fprintf(f, "  0\nPOINT\n  8\n%s\n 10\n%f\n 20\n%f\n 30\n%f\n",
		ObjName, Pts[0], Pts[1], Pts[2]);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one surface.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump polygon to.  		                     *
*   Srf:          Surface to dump into f as a DXF.                           *
*   ObjName:      Name of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPDumpDXFSrf(FILE *f,
			 const CagdSrfStruct *Srf,
			 const char *ObjName)
{
    int i;
    CagdRType **E3Points;
    CagdSrfStruct *E3Srf;

    if (CAGD_IS_BEZIER_SRF(Srf)) {
	fprintf(f, "  0\nPOLYLINE\n  8\n%s\n 70\n16\n 73\n%d\n 74\n%d\n 75\n8\n",
		ObjName, Srf -> VLength, Srf -> ULength);
    }
    else if (CAGD_IS_BSPLINE_SRF(Srf)) {
	if (Srf -> UOrder == Srf -> VOrder &&
	    (Srf  -> UOrder == 3 || Srf  -> UOrder == 4)) {
	    /* We do ignore the knot vector issue that DXF does not even */
	    /* mention - is it assuming uniform float!? open!?           */
	    fprintf(f, "  0\nPOLYLINE\n  8\n%s\n 70\n16\n 73\n%d\n 74\n%d\n 75\n%d\n",
		    ObjName, Srf -> ULength, Srf -> ULength,
		    Srf  -> UOrder == 3 ? 5 : 6);
	}
	else {
	    IRIT_WARNING_MSG("DXF files supports only biquadratic or bicubic Bspline surfaces\n");
	    IRIT_WARNING_MSG_PRINTF("\tfound surface \"%s\" of order %d by %d and this surface is ignored\n",
		                    ObjName, Srf -> UOrder, Srf -> VOrder);
	    return;
	}
    }
    else {
	IRIT_WARNING_MSG("Only Bezier or bspline surfaces can be converted.\n");
	return;
    }

    if (CAGD_IS_RATIONAL_PT(Srf -> PType)) {
	IRIT_WARNING_MSG_PRINTF("Rational surface \"%s\" coerced to Euclidean due to DXF inability\n",
		                ObjName);
    }

    /* Dumps the control points: */
    E3Srf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
    E3Points = E3Srf -> Points;

    for (i = 0; i < E3Srf -> ULength * E3Srf -> VLength; i++) {
	fprintf(f, "  0\nVERTEX\n  8\n%s\n 70\n16\n 10\n%f\n 20\n%f\n 30\n%f\n",
		ObjName, E3Points[1][i], E3Points[2][i], E3Points[3][i]);
    }
    fprintf(f, "  0\nSEQEND\n  8\n%s\n", ObjName);

    CagdSrfFree(E3Srf);
}
