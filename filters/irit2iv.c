/*****************************************************************************
* Filter to convert IRIT data files to SGI's Inventor ascii file format.     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, May 1994    *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

#define SIZE_IRIT_EPS	1e-5

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Iv		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Iv	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2Iv l%- 4%- P%- F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!F T%- t%-AnimTime!F z%- DFiles!*s";

IRIT_STATIC_DATA int
    GlblPolygonizeFreeForm = FALSE;

IRIT_STATIC_DATA IrtHmgnMatType CrntViewMat;   /* This is the current view! */

static void DumpDataForIv(IPObjectStruct *PObjects, IrtHmgnMatType CrntViewMat);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void DumpOneObject(FILE *f, IPObjectStruct *PObject);
static void DumpOnePolygon(FILE *f, IPPolygonStruct *PPolygon, int IsPolygon);
static void DumpOneCurve(FILE *f, IPObjectStruct *CrvObj);
static void DumpOneSurface(FILE *f, IPObjectStruct *SrfObj);
static void Irit2IvExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2iv - Read command line and do what is needed...	     M
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
    int Error,
	HasTime = FALSE,
	SrfFineNessFlag = FALSE,
	CrvFineNessFlag = FALSE,
	VerFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat,
			   &GlblPolygonizeFreeForm, &SrfFineNessFlag,
			   &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &CrvFineNessFlag,
			   &IPFFCState.CrvApproxMethod,
			   &IPFFCState.CrvApproxTolSamples,
			   &IPFFCState.Talkative, &HasTime, &CurrentTime,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2IvExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2IvExit(0);
    }

    if (IPFFCState.LinearOnePolyFlag) {
	CagdSetLinear2Poly(CAGD_ONE_POLY_PER_COLIN);
    }
    else
        CagdSetLinear2Poly(CAGD_REG_POLY_PER_LIN);

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2IvExit(1);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2IvExit(1);
    PObjects = IPResolveInstances(PObjects);
    if (HasTime)
	GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    DumpDataForIv(PObjects, CrntViewMat);

    Irit2IvExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for SGI's Inventor formal to stdout.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*   CrntViewMat:  Viewing matrix.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForIv(IPObjectStruct *PObjects, IrtHmgnMatType CrntViewMat)
{
    fprintf(stdout, "#Inventor V1.0 ascii\n\n");
    fprintf(stdout, "Separator {\n    Group {\n");

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, DumpOneTraversedObject);

    fprintf(stdout, "    }\n}\n");
}

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
    IPObjectStruct *PTmp, *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
        PTmp = GMTransformObject(PObj, Mat);
	DumpOneObject(stdout, PTmp);
	IPFreeObject(PTmp);
    }
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
static void DumpOneObject(FILE *f, IPObjectStruct *PObject)
{
    IPPolygonStruct *PList;

    if (!IP_IS_POLY_OBJ(PObject) &&
	!IP_IS_SRF_OBJ(PObject) &&
	!IP_IS_CRV_OBJ(PObject))
	return;

    fprintf(f, "\tSeparator {\n");
    if (IP_VALID_OBJ_NAME(PObject)) {
	fprintf(f, "\t    Label {\n\t\tlabel \"%s\"\n\t    }\n",
		IP_GET_OBJ_NAME(PObject));
    }
    fprintf(f, "\t    Group {\n");

    switch (PObject -> ObjType) {
	case IP_OBJ_POLY:
	    for (PList = PObject -> U.Pl;
		 PList != NULL;
		 PList = PList -> Pnext)
		DumpOnePolygon(f, PList, IP_IS_POLYGON_OBJ(PObject));
	    break;
	case IP_OBJ_CURVE:
	    DumpOneCurve(f, PObject);
	    break;
	case IP_OBJ_SURFACE:
	    DumpOneSurface(f, PObject);
	    break;
	default:
	    break;
    }

    fprintf(f, "\t    }\n\t}\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon, using global Matrix transform CrntViewMat.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump polygon to.  		                     *
*   PPolygon:     Polygon to dump to file f.                                 *
*   IsPolygon:    Is it a polygon or a polyline?                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void DumpOnePolygon(FILE *f, IPPolygonStruct *PPolygon, int IsPolygon)
{
    IPVertexStruct *V,
	*VList = PPolygon -> PVertex;

    if (VList == NULL)
	return;

    if (IsPolygon) {
	int i, NumVrt;

	fprintf(f, "\t\tSeparator {\n\t\t    Coordinate3 {\n\t\t\tpoint [ ");
	for (V = VList, NumVrt = 0; V != NULL; V = V -> Pnext, NumVrt++) {
	    fprintf(f, "%.7g %.7g %.7g",
		    V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	    if (V -> Pnext != NULL)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
	fprintf(f, "\t\t    }\n");

	fprintf(f, "\n\t\t    Normal {\n\t\t\tvector [ ");
	for (V = VList; V != NULL; V = V -> Pnext) {
	    fprintf(f, "%.7g %.7g %.7g",
		    -V -> Normal[0], -V -> Normal[1], -V -> Normal[2]);
	    if (V -> Pnext != NULL)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
	fprintf(f, "\t\t    }\n");

	fprintf(f, "\n\t\t    IndexedFaceSet {\n\t\t\tcoordIndex [ ");
	for (i = 0; i < NumVrt; i++)
	    fprintf(f, "%d, ", i);
	fprintf(f, "-1 ]\n\t\t    }\n\t\t}\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one curve.                                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   CrvObj:	  Curve to dump to file f.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneCurve(FILE *f, IPObjectStruct *CrvObj)
{
    int i,
	NewCrv = FALSE;
    CagdCrvStruct
	*Crv = CrvObj -> U.Crvs;
    CagdRType
	**Points = Crv -> Points;

    if (Crv -> GType == CAGD_CBEZIER_TYPE) {
	Crv = CagdCnvrtBzr2BspCrv(Crv);
	NewCrv = TRUE;
    }

    if (CAGD_IS_RATIONAL_CRV(Crv)) {
	fprintf(f, "\t\tSeparator {\n\t\t    Coordinate4 {\n\t\t\tpoint [ ");
    
	for (i = 0; i < Crv -> Length; i++) {
	    fprintf(f, "%.7g %.7g %.7g %.7g",
		    Points[1] ? Points[1][i] : 0.0,
		    Points[2] ? Points[2][i] : 0.0,
		    Points[3] ? Points[3][i] : 0.0,
		    Points[0] ? Points[0][i] : 1.0);
	    if (i < Crv -> Length - 1)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
    }
    else {
	fprintf(f, "\t\tSeparator {\n\t\t    Coordinate3 {\n\t\t\tpoint [ ");
    
	for (i = 0; i < Crv -> Length; i++) {
	    fprintf(f, "%.7g %.7g %.7g",
		    Points[1] ? Points[1][i] : 0.0,
		    Points[2] ? Points[2][i] : 0.0,
		    Points[3] ? Points[3][i] : 0.0);
	    if (i < Crv -> Length - 1)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
    }
    fprintf(f, "\t\t    }\n");

    fprintf(f, "\t\t    NurbsCurve {\n");
    fprintf(f, "\t\t\tnumControlPoints %d\n", Crv -> Length);
    fprintf(f, "\t\t\tknotVector	[ ");
    for (i = 0; i < Crv -> Length + Crv -> Order; i++) {
	if (i > 0 && i % 5 == 0)
	    fprintf(f, "\n\t\t\t\t");
	fprintf(f, "%.7g%s", Crv -> KnotVector[i], 
		i == Crv -> Length + Crv -> Order - 1 ? " ]\n" : ", ");
    }

    fprintf(f, "\t\t    }\n\t\t}\n");

    if (NewCrv)
	CagdCrvFree(Crv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one surface.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   SrfObj:	  Surface to dump to file f.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneSurface(FILE *f, IPObjectStruct *SrfObj)
{
    int i,
	NewSrf = FALSE;
    CagdSrfStruct
	*Srf = SrfObj -> U.Srfs;
    CagdRType
	**Points = Srf -> Points;

    if (Srf -> GType == CAGD_SBEZIER_TYPE) {
	Srf = CagdCnvrtBzr2BspSrf(Srf);
	NewSrf = TRUE;
    }

    if (CAGD_IS_RATIONAL_SRF(Srf)) {
	fprintf(f, "\t\tSeparator {\n\t\t    Coordinate4 {\n\t\t\tpoint [ ");
    
	for (i = 0; i < Srf -> ULength * Srf -> VLength; i++) {
	    fprintf(f, "%.7g %.7g %.7g %.7g",
		    Points[1] ? Points[1][i] : 0.0,
		    Points[2] ? Points[2][i] : 0.0,
		    Points[3] ? Points[3][i] : 0.0,
		    Points[0] ? Points[0][i] : 1.0);
	    if (i < Srf -> ULength * Srf -> VLength - 1)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
    }
    else {
	fprintf(f, "\t\tSeparator {\n\t\t    Coordinate3 {\n\t\t\tpoint [ ");
    
	for (i = 0; i < Srf -> ULength * Srf -> VLength; i++) {
	    fprintf(f, "%.7g %.7g %.7g",
		    Points[1] ? Points[1][i] : 0.0,
		    Points[2] ? Points[2][i] : 0.0,
		    Points[3] ? Points[3][i] : 0.0);
	    if (i < Srf -> ULength * Srf -> VLength - 1)
		fprintf(f, ",\n\t\t\t\t");
	    else
		fprintf(f, " ]\n");
	}
    }
    fprintf(f, "\t\t    }\n");

    fprintf(f, "\t\t    NurbsSurface {\n");
    fprintf(f, "\t\t\tnumUControlPoints %d\n", Srf -> ULength);
    fprintf(f, "\t\t\tnumVControlPoints %d\n", Srf -> VLength);
    fprintf(f, "\t\t\tuKnotVector	[ ");
    for (i = 0; i < Srf -> ULength + Srf -> UOrder; i++) {
	if (i > 0 && i % 5 == 0)
	    fprintf(f, "\n\t\t\t\t");
	fprintf(f, "%g%s", Srf -> UKnotVector[i], 
		i == Srf -> ULength + Srf -> UOrder - 1 ? " ]\n" : ", ");
    }
    fprintf(f, "\t\t\tvKnotVector	[ ");
    for (i = 0; i < Srf -> VLength + Srf -> VOrder; i++) {
	if (i > 0 && i % 5 == 0)
	    fprintf(f, "\n\t\t\t\t");
	fprintf(f, "%g%s", Srf -> VKnotVector[i], 
		i == Srf -> VLength + Srf -> VOrder - 1 ? " ]\n" : ", ");
    }

    fprintf(f, "\t\t    }\n\t\t}\n");

    if (NewSrf)
	CagdSrfFree(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Iv exit routine.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2IvExit(int ExitCode)
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
