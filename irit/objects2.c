/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle the objects list - fetch, insert, delete etc...	     *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "program.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_lib.h"
#include "objects.h"
#include "freeform.h"

IRIT_STATIC_DATA int
    GlblDumpLvl = DEFAULT_DUMPLVL;

static void UpdateLoadedHierarchy(IPObjectStruct *PObj, IPObjectStruct *Root);
static void PrintIritObjectAux(IPObjectStruct *PObj, int Indent);
static const char *GetDataFileType(const char *FileName);
static int MatGenMatGeneric(IPObjectStruct *LstObjList, IrtHmgnMatType Mat);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets dumping level values for variables and expression's results.          M
*                                                                            *
* PARAMETERS:                                                                M
*   DumpLvl:  New dump level.				                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Old value.                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetDumpLevel                                                             M
*****************************************************************************/
int SetDumpLevel(int DumpLvl)
{
    int OldVal = GlblDumpLvl;

    GlblDumpLvl = DumpLvl;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Print some usefull info on the given object.			     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To print out.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrintIritObject                                                          M
*****************************************************************************/
void PrintIritObject(IPObjectStruct *PObj)
{
    IPSetPrintFunc(IritPutStr2);
    PrintIritObjectAux(PObj, 0);
    IPSetPrintFunc(NULL);
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of PrintIritObject                                      *
*****************************************************************************/
static void PrintIritObjectAux(IPObjectStruct *PObj, int Indent)
{
    int i,
	Count = PObj -> Count;
    char CIndent[IRIT_LINE_LEN_VLONG],
	*p = CIndent,
	*Name =  IP_VALID_OBJ_NAME(PObj) ? IP_GET_OBJ_NAME(PObj) : "NONE";

    for (i = 0; i < Indent; i++)
        *p++ = ' ';
    *p = 0;

    switch (PObj -> ObjType) {
	case IP_OBJ_UNDEF:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Undefined type",
				   CIndent, Name, Count);
	    break;
	case IP_OBJ_POLY:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Poly      type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x04)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_NUMERIC:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Numeric   type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_POINT:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Point     type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_VECTOR:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Vector    type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_PLANE:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Plane     type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_CTLPT:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - CtlPt     type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_MATRIX:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Matrix    type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_INSTANCE:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Instance  type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x01)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_STRING:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - String    type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x1)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_LIST_OBJ:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Object List type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x20) {
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    }
	    else if (GlblDumpLvl & 0x10) {
	        IPObjectStruct *PObjTmp;

	        for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
		    if (PObjTmp == PObj)
		        IPFatalError(IP_ERR_LIST_CONTAIN_SELF);
		    else
		        PrintIritObjectAux(PObjTmp, Indent + 4);
		}
	    }
	    break;
	case IP_OBJ_CURVE:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Curve     type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_SURFACE:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Surface   type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_TRIMSRF:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Trimmed Srf type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_TRIVAR:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Trivariate type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_MODEL:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Model type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_MULTIVAR:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Multivariate type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	case IP_OBJ_TRISRF:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF4("%s%-10s (%d) - Triangular Surface type",
				   CIndent, Name, Count);
	    if (GlblDumpLvl & 0x02)
	        IPPutObjectToFile2(NULL, PObj, Indent);
	    break;
	default:
	    if (!(GlblDumpLvl & 0x100))
		IRIT_WNDW_FPRINTF5("%s%-10s (%d) - Obj type error, type = %d",
				   CIndent, Name, Count, PObj -> ObjType);
	    break;
    }

    if ((GlblDumpLvl & 0x40) && PObj -> Dpnds != NULL) {
        char Line[IRIT_LINE_LEN_VLONG];
	IPODDependsStruct *Dpnd;
	IPODParamsStruct *Prm;

	IRIT_WNDW_FPRINTF3(
	    "%sDependency structure of \"%s\", EvalExpr equals:",
	    CIndent, IP_GET_OBJ_NAME(PObj));
	IRIT_WNDW_FPRINTF3("%s%s", CIndent, PObj -> Dpnds -> EvalExpr);

	sprintf(Line, IRIT_EXP_STR("%sDependencies: "), CIndent);
	for (Dpnd = PObj -> Dpnds -> ObjDepends;
	     Dpnd != NULL;
	     Dpnd = Dpnd -> Pnext) {
	    strcat(Line, Dpnd -> Name);
	    if (Dpnd -> Pnext != NULL)
	        strcat(Line, ", ");
	}
	IRIT_WNDW_PUT_STR(Line);

	sprintf(Line, IRIT_EXP_STR("%sParameters (%d): "),
		CIndent, PObj -> Dpnds -> NumParams);
	for (Prm = PObj -> Dpnds -> ObjParams;
	     Prm != NULL;
	     Prm = Prm -> Pnext) {
	    strcat(Line, Prm -> Name);
	    if (Prm -> Pnext != NULL)
	        strcat(Line, ", ");
	}
	IRIT_WNDW_PUT_STR(Line);
	IRIT_WNDW_PUT_STR("");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Gets a string description of the object type.			     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Object to get a string description on.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:      A string describing PObj.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetObjectTypeAsString                                                    M
*****************************************************************************/
char *GetObjectTypeAsString(IPObjectStruct *PObj)
{
    if (PObj == NULL)
	return "Unknown";

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    return (IP_IS_POLYGON_OBJ(PObj) ? "Polygons"
				      : (IP_IS_POLYLINE_OBJ(PObj) ? "Polylines"
							          : "Points"));
	case IP_OBJ_NUMERIC:
	    return "Numeric";
	case IP_OBJ_POINT:
	    return "Point";
	case IP_OBJ_VECTOR:
	    return "Vector";
	case IP_OBJ_PLANE:
	    return "Plane";
	case IP_OBJ_CTLPT:
	    return "Control Point";
	case IP_OBJ_MATRIX:
	    return "Matrix";
	case IP_OBJ_INSTANCE:
	    return "Instance";
	case IP_OBJ_STRING:
	    return "String";
	case IP_OBJ_LIST_OBJ:
	    return "List Object";
	case IP_OBJ_CURVE:
	    return "Curve";
	case IP_OBJ_SURFACE:
	    return "Surface";
	case IP_OBJ_TRIMSRF:
	    return "TrimSrf";
	case IP_OBJ_TRIVAR:
	    return "Trivar";
	case IP_OBJ_MODEL:
	    return "Model";
	case IP_OBJ_MULTIVAR:
	    return "Multivar";
	case IP_OBJ_TRISRF:
	    return "TriSrf";
	case IP_OBJ_UNDEF:
	default:
	    return "Undefined";
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Print some useful information on a list of objects.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      List of objects to print out.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrintIritObjectList                                                      M
*****************************************************************************/
void PrintIritObjectList(IPObjectStruct *PObj)
{
    IritDBValidateVariables();

    IRIT_WNDW_PUT_STR("");

    while (PObj != NULL) {
	PrintIritObject(PObj);
	PObj = PObj -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Coerce an object to a new object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        Object to coerce.                                           M
*   RNewType:    New type for PObj.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The newly coerced object.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CoerceIritObjectTo                                                       M
*****************************************************************************/
IPObjectStruct *CoerceIritObjectTo(IPObjectStruct *PObj, IrtRType *RNewType)
{
    int NewType = IRIT_REAL_TO_INT(*RNewType),
        PObjIsNew = FALSE;
    CagdSrfStruct *Srf;
    TrivTVStruct *TV;
    IPObjectStruct
	*NewObj = NULL;

    if (PObj -> ObjType == IP_OBJ_LIST_OBJ) {
	int i, j;
	IPObjectStruct *PObjTmp;

	NewObj = IPGenLISTObject(NULL);
	for (i = j = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++) {
	    IPObjectStruct
	        *PObjTmpNew = CoerceIritObjectTo(PObjTmp, RNewType);

	    if (PObjTmpNew != NULL)
	        IPListObjectInsert(NewObj, j++, PObjTmpNew);
	}
	IPListObjectInsert(NewObj, j++, NULL);

	return NewObj;
    }

    switch (NewType) {
	case FF_POWER_TYPE:
	    if ((PObj -> ObjType == IP_OBJ_CURVE &&
		 PObj -> U.Crvs -> GType == CAGD_CBSPLINE_TYPE) ||
		(PObj -> ObjType == IP_OBJ_SURFACE &&
		 PObj -> U.Srfs -> GType == CAGD_SBSPLINE_TYPE) ||
		(PObj -> ObjType == IP_OBJ_MULTIVAR &&
		 PObj -> U.MultiVars -> GType == MVAR_BSPLINE_TYPE)) {
		PObj = CnvrtBsplineToBezier(PObj);
		PObjIsNew = TRUE;
	    }
	    if ((PObj -> ObjType == IP_OBJ_CURVE &&
		 PObj -> U.Crvs -> GType == CAGD_CBEZIER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_SURFACE &&
		 PObj -> U.Srfs -> GType == CAGD_SBEZIER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_MULTIVAR &&
		 PObj -> U.MultiVars -> GType == MVAR_BEZIER_TYPE))
		NewObj = CnvrtBezierToPower(PObj);
	    break;

	case FF_BEZIER_TYPE:
	    if ((PObj -> ObjType == IP_OBJ_CURVE &&
		 PObj -> U.Crvs -> GType == CAGD_CPOWER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_SURFACE &&
		 PObj -> U.Srfs -> GType == CAGD_SPOWER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_MULTIVAR &&
		 PObj -> U.MultiVars -> GType == MVAR_POWER_TYPE))
		NewObj = CnvrtPowerToBezier(PObj);
	    else if (PObj -> ObjType == IP_OBJ_TRISRF &&
		     PObj -> U.TriSrfs -> GType == TRNG_TRISRF_GREGORY_TYPE)
		NewObj = CnvrtGregoryToBezier(PObj);
	    else if ((PObj -> ObjType == IP_OBJ_CURVE &&
		      PObj -> U.Crvs -> GType == CAGD_CBSPLINE_TYPE) ||
		     (PObj -> ObjType == IP_OBJ_SURFACE &&
		      PObj -> U.Srfs -> GType == CAGD_SBSPLINE_TYPE) ||
		     (PObj -> ObjType == IP_OBJ_TRIVAR &&
		      PObj -> U.Trivars -> GType == TRIV_TVBSPLINE_TYPE) ||
		     (PObj -> ObjType == IP_OBJ_MULTIVAR &&
		      PObj -> U.MultiVars -> GType == MVAR_BSPLINE_TYPE))
	        NewObj = CnvrtBsplineToBezier(PObj);
	    break;
	case FF_BSPLINE_TYPE:
	    if ((PObj -> ObjType == IP_OBJ_CURVE &&
		 PObj -> U.Crvs -> GType == CAGD_CPOWER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_SURFACE &&
		 PObj -> U.Srfs -> GType == CAGD_SPOWER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_MULTIVAR &&
		 PObj -> U.MultiVars -> GType == MVAR_POWER_TYPE)) {
	        PObj = CnvrtPowerToBezier(PObj);
		PObjIsNew = TRUE;
	    }
	    if ((PObj -> ObjType == IP_OBJ_CURVE &&
		 PObj -> U.Crvs -> GType == CAGD_CBEZIER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_SURFACE &&
		 PObj -> U.Srfs -> GType == CAGD_SBEZIER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_TRIVAR &&
		 PObj -> U.Trivars -> GType == TRIV_TVBEZIER_TYPE) ||
		(PObj -> ObjType == IP_OBJ_MULTIVAR &&
		 PObj -> U.MultiVars -> GType == MVAR_BEZIER_TYPE))
		NewObj = CnvrtBezierToBspline(PObj);
	    break;
	case KV_UNIFORM_PERIODIC:
	    IRIT_WNDW_PUT_STR("Conversion to periodic is not supported.");
	    return NULL;
	case KV_UNIFORM_FLOAT:
	    if (IP_IS_CRV_OBJ(PObj)) {
		if (CAGD_IS_PERIODIC_CRV(PObj -> U.Crvs)) {
		    NewObj =
		        IPGenCRVObject(CagdCnvrtPeriodic2FloatCrv(PObj -> U.Crvs));
		}
		else if (CAGD_IS_BSPLINE_CRV(PObj -> U.Crvs) &&
			 !BspCrvHasOpenEC(PObj -> U.Crvs)) {
		    NewObj = IPGenCRVObject(CagdCrvCopy(PObj -> U.Crvs));
		}
		else {
		    IRIT_WNDW_PUT_STR("Conversion to float legal only from periodic.");
		    return NULL;
		}
	    }
	    else if (IP_IS_SRF_OBJ(PObj)) {
		if (CAGD_IS_PERIODIC_SRF(PObj -> U.Srfs)) {
		    Srf = CagdCnvrtPeriodic2FloatSrf(PObj -> U.Srfs);
		    NewObj = IPGenSRFObject(Srf);
		}
		else if (CAGD_IS_BSPLINE_SRF(PObj -> U.Srfs) &&
			 !BspSrfHasOpenEC(PObj -> U.Srfs)) {
		    NewObj = IPGenSRFObject(CagdSrfCopy(PObj -> U.Srfs));
		}
		else {
		    IRIT_WNDW_PUT_STR("Conversion to float legal only from periodic.");
		    return NULL;
		}
	    }
	    else if (IP_IS_TRIVAR_OBJ(PObj)) {
		if (TRIV_IS_PERIODIC_TV(PObj -> U.Trivars)) {
		    TV = TrivCnvrtPeriodic2FloatTV(PObj -> U.Trivars);
		    NewObj = IPGenTRIVARObject(TV);
		}
		else if (TRIV_IS_BSPLINE_TV(PObj -> U.Trivars) &&
			 !TrivBspTVHasOpenEC(PObj -> U.Trivars)) {
		    NewObj = IPGenTRIVARObject(TrivTVCopy(PObj -> U.Trivars));
		}
		else {
		    IRIT_WNDW_PUT_STR("Conversion to float legal only from periodic.");
		    return NULL;
		}
	    }
	    else if (IP_IS_MVAR_OBJ(PObj)) {
	        if (MVAR_IS_BSPLINE_MV(PObj -> U.MultiVars)) {
		    NewObj = IPGenMULTIVARObject(
			      MvarCnvrtPeriodic2FloatMV(PObj -> U.MultiVars));
		}
		else {
		    IRIT_WNDW_PUT_STR("Conversion to float legal only from B-spline rep.");
		    return NULL;
		}
	    }
	    break;
	case KV_UNIFORM_OPEN:
	    if (IP_IS_CRV_OBJ(PObj)) {
		if (CAGD_IS_BEZIER_CRV(PObj -> U.Crvs) ||
		    (CAGD_IS_BSPLINE_CRV(PObj -> U.Crvs) &&
		     BspCrvHasOpenEC(PObj -> U.Crvs))) {
		    NewObj = IPGenCRVObject(CagdCrvCopy(PObj -> U.Crvs));
		}
		else {
		    NewObj = IPGenCRVObject(BspCrvOpenEnd(PObj -> U.Crvs));
		}
	    }
	    else if (IP_IS_SRF_OBJ(PObj)) {
		if (CAGD_IS_BEZIER_SRF(PObj -> U.Srfs) ||
		    (CAGD_IS_BSPLINE_SRF(PObj -> U.Srfs) &&
		     BspSrfHasOpenEC(PObj -> U.Srfs))) {
		    NewObj = IPGenSRFObject(CagdSrfCopy(PObj -> U.Srfs));
		}
		else {
		    NewObj = IPGenSRFObject(BspSrfOpenEnd(PObj -> U.Srfs));
		}
	    }
	    else if (IP_IS_TRIVAR_OBJ(PObj)) {
		if (TRIV_IS_BEZIER_TV(PObj -> U.Trivars) ||
		    (TRIV_IS_BSPLINE_TV(PObj -> U.Trivars) &&
		     TrivBspTVHasOpenEC(PObj -> U.Trivars))) {
		    NewObj = IPGenTRIVARObject(TrivTVCopy(PObj -> U.Trivars));
		}
		else {
		    NewObj = IPGenTRIVARObject(TrivTVOpenEnd(PObj -> U.Trivars));
		}
	    }
	    else if (IP_IS_MVAR_OBJ(PObj)) {
	        if (MVAR_IS_BEZIER_MV(PObj -> U.MultiVars)) {
		    NewObj = IPGenMULTIVARObject(
					     MvarMVCopy(PObj -> U.MultiVars));
		}
		else {
		    NewObj = IPGenMULTIVARObject(
			          MvarCnvrtFloat2OpenMV(PObj -> U.MultiVars));
		}
	    }
	    break;
	case IP_OBJ_CURVE:
	    if (IP_IS_MVAR_OBJ(PObj)) {
		if (PObj -> U.MultiVars -> Dim == 1)
		    NewObj = IPGenCRVObject(MvarMVToCrv(PObj -> U.MultiVars));
		else
		    IRIT_WNDW_PUT_STR("Multivariate is not a univariate.");
	    }
	    break;
	case IP_OBJ_SURFACE:
	    if (IP_IS_MVAR_OBJ(PObj)) {
		if (PObj -> U.MultiVars -> Dim == 2)
		    NewObj = IPGenSRFObject(MvarMVToSrf(PObj -> U.MultiVars));
		else
		    IRIT_WNDW_PUT_STR("Multivariate is not a bivariate.");
	    }
	    break;
	case IP_OBJ_TRIVAR:
	    if (IP_IS_MVAR_OBJ(PObj)) {
		if (PObj -> U.MultiVars -> Dim == 3)
		    NewObj = IPGenTRIVARObject(MvarMVToTV(PObj -> U.MultiVars));
		else
		    IRIT_WNDW_PUT_STR("Multivariate is not a trivariate.");
	    }
	    break;
	case IP_OBJ_MODEL:
	    {
		MdlModelStruct
		    *Mdl = NULL;

		if (IP_IS_SRF_OBJ(PObj)) {
		    Mdl = MdlCnvrtSrf2Mdl(PObj -> U.Srfs);
		}
		else if (IP_IS_TRIMSRF_OBJ(PObj)) {
		    Mdl = MdlCnvrtTrimmedSrf2Mdl(PObj -> U.TrimSrfs);
		}
		else {
		    IRIT_WNDW_PUT_STR("Only a (trimmed) surface can be coerced into a model.");
		}

		NewObj = Mdl == NULL ? NULL : IPGenMODELObject(Mdl);
	    }
	    break;
	case IP_OBJ_TRIMSRF:
	    {
		TrimSrfStruct
		    *TSrf = NULL;
		IPObjectStruct *TrimSrfObj;

		if (IP_IS_SRF_OBJ(PObj)) {
		    TSrf = TrimSrfNew(PObj -> U.Srfs, NULL, FALSE);
		    NewObj = IPGenTRIMSRFObject(TSrf);
		}
		else if (IP_IS_MODEL_OBJ(PObj)) {
		    if ((TSrf = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls)) != NULL) {
			int i = 0;

			NewObj = IPGenLISTObject(NULL);
			while (TSrf != NULL) {
			    TrimSrfObj = IPGenTRIMSRFObject(TSrf);
			    TSrf = TSrf -> Pnext;
			    TrimSrfObj -> U.TrimSrfs -> Pnext = NULL;
			    IPListObjectInsert(NewObj, i++, TrimSrfObj);
			}
			IPListObjectInsert(NewObj, i, NULL);
		    }		    
		}
		else {
		    IRIT_WNDW_PUT_STR("Only a surface or a model may be coerced into a trimmed surface.");
		}
	    }
	    break;
	case IP_OBJ_MULTIVAR:
	    switch (PObj -> ObjType) {
	        case IP_OBJ_CURVE:
		    NewObj = IPGenMULTIVARObject(MvarCrvToMV(PObj -> U.Crvs));
		    break;
		case IP_OBJ_SURFACE:
		    NewObj = IPGenMULTIVARObject(MvarSrfToMV(PObj -> U.Srfs));
		    break;
		case IP_OBJ_TRIVAR:
		    NewObj = IPGenMULTIVARObject(MvarTVToMV(PObj -> U.Trivars));
		    break;
		default:
		    IRIT_WNDW_PUT_STR("Invalid source for coercion to multivariate.");
		    break;
	    }
	    break;
	default:
	    NewObj = IPCoerceObjectTo(PObj, NewType);
	    break;
    }

    if (PObjIsNew)
        IPFreeObject(PObj);

    if (NewObj == NULL)
	IRIT_WNDW_PUT_STR("Invalid coercion requested.");
    return NewObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Isolates the file type associated with this file name.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:   Input file name to isolate its file type.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   const char *:  Detected file type, statically allocated, or "???" if not *
*                  found.				                     *
*****************************************************************************/
static const char *GetDataFileType(const char *FileName)
{
    static char FTypeStr[IRIT_LINE_LEN_VLONG], *FType;

    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 5);

    strncpy(FTypeStr, FileName, IRIT_LINE_LEN_VLONG - 5);
    FType = strrchr(FTypeStr, '.');

    if ((FType = strrchr(FTypeStr, '.')) != NULL &&
	(stricmp(FType, ".Z") == 0 || stricmp(FType, ".gz") == 0)) {
        *FType = 0;
	FType = strrchr(FTypeStr, '.');
    }

    return FType == NULL ? "???" : FType + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Saves an object in a data file.				     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   Where to save PObj.                                          M
*   PObj:       Object to save in a file.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SaveObjectInFile                                                         M
*****************************************************************************/
void SaveObjectInFile(const char *FileName, IPObjectStruct *PObj)
{
    if (FileName != NULL) {
        int Handler,
	    Iges = FALSE,
	    GCode = FALSE,
	    STL = FALSE,
	    OBJ = FALSE,
	    Vrml = FALSE;
	char FullFileName[IRIT_LINE_LEN_VLONG];
	const char
	    *FType = GetDataFileType(FileName);

	strcpy(FullFileName, FileName);
	if (stricmp(FType, "wrl") == 0) {
	    /* Vrml data file is requested. */
	    Vrml = TRUE;
	}
	else if (stricmp(FType, "igs") == 0 || stricmp(FType, "iges") == 0) {
	    /* Iges data file is requested. */
	    Iges = TRUE;
	}
	else if (stricmp(FType, "stl") == 0) {
	    /* STL data file is requested. */
	    STL = TRUE;
	}
	else if (stricmp(FType, "obj") == 0) {
	    /* OBJ data file is requested. */
	    OBJ = TRUE;
	}
	else if (stricmp(FType, "nc") == 0 ||
		 stricmp(FType, "cnc") == 0 ||
		 stricmp(FType, "gcode") == 0) {
	    /* GCode/CNC data file is requested. */
	    GCode = TRUE;
	}
	else if (stricmp(FType, IRIT_COMPRESSED_DATA_FILE) == 0) {
            /* Irit compressed data file is requested. */
        }
	else if (stricmp(FType, IRIT_BINARY_DATA_FILE) == 0) {
	    /* Irit binary data file is requested. */
	}
	else if (stricmp(FType, IRIT_TEXT_DATA_FILE) != 0 &&
		 stricmp(FType, IRIT_MATRIX_DATA_FILE) != 0) {
	    /* Irit text data file is requested. */
	    strcat(FullFileName, ".");
	    strcat(FullFileName, IRIT_TEXT_DATA_FILE);
	}

	if (Iges) {
	    IrtHmgnMatType UnitMat;

	    MatGenUnitMat(UnitMat);

	    IPIgesSaveFile(PObj, UnitMat, FullFileName, TRUE);
	}
	else if (STL) {
	    IrtHmgnMatType UnitMat;

	    MatGenUnitMat(UnitMat);

	    IPSTLSaveFile(PObj, UnitMat, TRUE, 1, FullFileName, TRUE);
	}
	else if (OBJ) {
	    IPOBJSaveFile(PObj, FullFileName, TRUE, FALSE);
	}
	else if (GCode) {
	    IrtHmgnMatType UnitMat;

	    MatGenUnitMat(UnitMat);

	    IPNCGCodeSaveFile(PObj, UnitMat, FullFileName, TRUE,
			      AttrGetObjectIntAttrib(PObj, "NCUnits"));
	}
	else {
	    if (Vrml)
		Handler = IPOpenVrmlFile(FullFileName, FALSE,
					 GetResolution(TRUE));
	    else
		Handler = IPOpenDataFile(FullFileName, FALSE, FALSE);
	    if (Handler >= 0) {
		IPPutObjectToHandler(Handler, PObj);
		IPCloseStream(Handler, TRUE);
	    }
	    else {
		IRIT_WNDW_FPRINTF2("Failed to open file \"%s\" for write.",
				   FileName);
	    }
	}
    }
    else {
	/* The output will go to IRIT_WNDW_PUT_STR2: */
	IPSetPrintFunc(IritPutStr2);
	IPPutObjectToFile(NULL, PObj, FALSE);
	IPSetPrintFunc(NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Loads an object(s) from a data file.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:    Where to read object(s) from.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Object(s) read from file FileName.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   LoadObjectFromFile                                                       M
*****************************************************************************/
IPObjectStruct *LoadObjectFromFile(const char *FileName)
{
    if (FileName != NULL) {
        int Handler,
	    GCode = FALSE,
	    Iges = FALSE,
	    Obj = FALSE,
	    Stl = FALSE;
        char FullFileName[IRIT_LINE_LEN_VLONG];
	const char
	    *FType = GetDataFileType(FileName);

	strcpy(FullFileName, FileName);

	if (stricmp(FType, "igs") == 0 || stricmp(FType, "iges") == 0) {
	    /* Iges data file is requested. */
	    Iges = TRUE;
	}
	else if (stricmp(FType, "stl") == 0) {
	    /* STL data file is requested. */
	    Stl = TRUE;
	}
	else if (stricmp(FType, "obj") == 0) {
	    /* STL data file is requested. */
	    Obj = TRUE;
	}
	else if (stricmp(FType, "nc") == 0 ||
		 stricmp(FType, "cnc") == 0 ||
		 stricmp(FType, "gcode") == 0) {
	    /* CNC G-code data file is requested. */
	    GCode = TRUE;
	}
	else if (stricmp(FType, IRIT_TEXT_DATA_FILE) != 0 &&
		 stricmp(FType, IRIT_BINARY_DATA_FILE) != 0 &&
                 stricmp(FType, IRIT_COMPRESSED_DATA_FILE) != 0 &&
		 stricmp(FType, IRIT_MATRIX_DATA_FILE) != 0) {
	    strcat(FullFileName, ".");
	    strcat(FullFileName, IRIT_TEXT_DATA_FILE);
	}

	if (Iges) {
	    return IPIgesLoadFile(FullFileName, FALSE, TRUE, FALSE, 1);
	}
	else if (Stl) {
	    return IPSTLLoadFile(FullFileName, FALSE, FALSE, FALSE, FALSE);
	}
	else if (Obj) {
	    return IPOBJLoadFile(FullFileName, FALSE, TRUE, TRUE, TRUE);
	}
	else if (GCode) {
	    return IPNCGCodeLoadFile(FullFileName, FALSE, FALSE);
	}
	else {
	    if (GlblFlatLoadMode)
	        IPSetFlattenObjects(TRUE);

	    if ((Handler = IPOpenDataFile(FullFileName, TRUE, FALSE)) >= 0) {
	        IPObjectStruct
		    *PObj = IPGetObjects(Handler);

		IPCloseStream(Handler, TRUE);

		if (PObj == NULL) {
		    const char *ErrorMsg;

		    if (IPHasError(&ErrorMsg)) {
		        IRIT_WNDW_PUT_STR("Data file parsing error:");
			IRIT_WNDW_PUT_STR(ErrorMsg);
		    }
		}
		else {
		    if (GlblFlatLoadMode) {
		        IPSetFlattenObjects(FALSE);

			PObj = IPObjLnkListToListObject(PObj);
		    }
		    else {
		        UpdateLoadedHierarchy(PObj, PObj);
			PObj -> Count = 0;
		    }

		    return PObj;
		}
	    }
	}
    }

    IRIT_NON_FATAL_ERROR2("LOAD: Failed to load file \"%s\".\n", FileName);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Updates the hierarchy of loaded object.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:     Current, local, top of hierarchy.                              *
*   Root:     Global, root, toop of hierarchy.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateLoadedHierarchy(IPObjectStruct *PObj, IPObjectStruct *Root)
{
    if (PObj == NULL)
	return;

    if (IP_IS_OLST_OBJ(PObj)) {
	int i;
	IPObjectStruct *PObjTmp;

	for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++)
	    UpdateLoadedHierarchy(PObjTmp, Root);
    }

    if (PObj != Root && IP_VALID_OBJ_NAME(PObj)) {
	IPObjectStruct *OldPObj;

	if ((OldPObj = IritDBGetObjByName(IP_GET_OBJ_NAME(PObj))) != NULL)
	    IritDBDeleteObject(OldPObj, TRUE);
	IritDBInsertObject(PObj, FALSE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Error handler for loading and saving of files.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorMsg:   Returns description of error if found one.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if error found, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   LoadSaveObjectParseError                                                 M
*****************************************************************************/
int LoadSaveObjectParseError(const char **ErrorMsg)
{
    if (!IPHasError(ErrorMsg)) {
	*ErrorMsg = "";
	return FALSE;
    }
    else
	return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a viewing transformation matrix for a viewer at Pos, looking   M
* at Dir and upper direction of UpDir.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos:       Location of viewer.                                           M
*   Dir:       Direction of view of viewer.                                  M
*   UpDir:     Upper direction of view.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A matrix object with the transformation matrix.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenMatObjectPosDir                                                       M
*****************************************************************************/
IPObjectStruct *GenMatObjectPosDir(IrtPtType Pos,
				   IrtVecType Dir,
				   IrtVecType UpDir)
{
    IrtHmgnMatType Mat;

    if (GMMatFromPosDir(Pos, Dir, UpDir, Mat))
        return IPGenMATObject(Mat);
    else {
	IRIT_WNDW_PUT_STR("Matrix: Given directions are parallel!");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Decompose a matrix into rotation, scaling and translation factors, in    M
* this order.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Matrix to decompose into rot/trans/scale.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of three objects: scaling vector, translation  M
*		       vector and rotation matrix.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GetMatTransRecomp                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetMatTransDecomp                                                        M
*****************************************************************************/
IPObjectStruct *GetMatTransDecomp(IPObjectStruct *Mat)
{
    IrtHmgnMatType RotMat;
    GMQuatType q;
    IrtVecType S, T;
    IPObjectStruct
        *DecomObj = IPGenLISTObject(NULL);

    GMMatrixToTransform(*(Mat -> U.Mat), S, q, T);
    GMQuatToMat(q, RotMat);

    IPListObjectInsert(DecomObj, 0, IPGenVECObject(&S[0], &S[1], &S[2]));
    IPListObjectInsert(DecomObj, 1, IPGenVECObject(&T[0], &T[1], &T[2]));
    IPListObjectInsert(DecomObj, 2, IPGenMATObject(RotMat));
    IPListObjectInsert(DecomObj, 3, NULL);

    return DecomObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Decomposes a (4x4) homogeneous matrix into the vector:                   M
* (RotX, RotY, RotZ, Scale, TransX, TransY, TransZ)			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:     A homogeneous matrix to decompose.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of decomposed factors.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GetMatTransRecomp                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetMatTransDecomp2                                                       M
*****************************************************************************/
IPObjectStruct *GetMatTransDecomp2(IPObjectStruct *Mat)
{
    int i, n;
    IrtVecType Vecs[8];
    IrtHmgnMatType RotMat;
    CagdRType Factors[7];
    IPObjectStruct *RetList, *PTmp;

    MatRotateFactorMatrix(*Mat -> U.Mat, RotMat);
    if ((n = GMQuatMatrixToAngles(RotMat, Vecs)) == 0) {
	IRIT_VEC_RESET(Factors);      /* Failed to compute rotation factors. */
    }
    else
        IRIT_VEC_COPY(Factors, Vecs[0]);

    Factors[3] = MatScaleFactorMatrix(*Mat -> U.Mat);

    MatTranslateFactorMatrix(*Mat -> U.Mat, &Factors[4]);
    
    RetList = IPGenLISTObject(NULL);
    for (i = 0; i < 7; i++) {
        PTmp = IPGenNUMValObject(Factors[i]);
	IPListObjectAppend(RetList, PTmp);
    }

    return RetList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Composes a (4x4) homogeneous matrix from the vector:                     M
* (RotX, RotY, RotZ, Scale, TransX, TransY, TransZ)			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatFactorObj:   The list of 7 factors.  Transformations are applied in   M
*                   order.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A homogeneous matrix                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GetMatTransDecomp                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetMatTransRecomp                                                        M
*****************************************************************************/
IPObjectStruct *GetMatTransRecomp(IPObjectStruct *MatFactors)
{
    int i;
    IrtHmgnMatType Mat, M;
    CagdRType Factors[7];

    if (!IP_IS_OLST_OBJ(MatFactors))
        return NULL;

    for (i = 0; i < 7; i++) {
        IPObjectStruct
	    *PTmp = IPListObjectGet(MatFactors, i);
	if (!IP_IS_NUM_OBJ(PTmp)) {
	    IRIT_WNDW_FPRINTF2("MatRecomp: expected a list of 7 real factors");
	    return NULL;
	}

	Factors[i] = PTmp -> U.R;
    }

    MatGenMatRotX1(Factors[0], Mat);
    MatGenMatRotY1(Factors[1], M);
    MatMultTwo4by4(Mat, Mat, M);
    MatGenMatRotZ1(Factors[2], M);
    MatMultTwo4by4(Mat, Mat, M);
    MatGenMatUnifScale(Factors[3], M);
    MatMultTwo4by4(Mat, Mat, M);
    MatGenMatTrans(Factors[4], Factors[5], Factors[6], M);
    MatMultTwo4by4(Mat, Mat, M);

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to construct a matrix out of a list of four lists of four numbers. M
*                                                                            *
* PARAMETERS:                                                                M
*   LstObjList:   A list of four lists of four numbers.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenMatObjectGeneric                                                      M
*****************************************************************************/
IPObjectStruct *GenMatObjectGeneric(IPObjectStruct *LstObjList)
{
    IrtHmgnMatType Mat;

    /* Generate the transformation matrix */
    if (MatGenMatGeneric(LstObjList, Mat))
	return IPGenMATObject(Mat);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a matrix that projects 3D objects to the Projection Plane     M
* ProjPlane, having the eye at EyePos.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:       The plane to project the objects onto.                      M
*   EyeDir:      The direction to the eye.                                   M
*   EyeInfinity: Zero for eye at infinity, EyeDir is eye position divided    M
*		 by EyeInfinity otherwise.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Constructed projection matrix.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatProjectionMat                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenMatProjectionMat                                                      M
*****************************************************************************/
IPObjectStruct *GenMatProjectionMat(IrtPlnType Plane,
				    IrtVecType EyeDir,
				    IrtRType *EyeInfinity)
{
    IrtRType EyePos[4];
    IrtHmgnMatType Mat;

    IRIT_VEC_COPY(EyePos, EyeDir);
    EyePos[3] = *EyeInfinity;

    GMGenProjectionMat(Plane, EyePos, Mat);

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a matrix that reflects 3D objects around the prescribed       M
* reflection Plane RflctPlane.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:       The plane to reflect the objects according to.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Constructed projection matrix.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatProjectionMat                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenMatReflectionMat                                                      M
*****************************************************************************/
IPObjectStruct *GenMatReflectionMat(IrtPlnType Plane)
{
    IrtHmgnMatType Mat;

    GMGenReflectionMat(Plane, Mat);

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to generate a 4*4 matrix  by specifying all its 16 coefficients.   *
*                                                                            *
* PARAMETERS:                                                                *
*   LstObjList:   A list of four lists of four numbers.                      *
*   Mat:          Where to save the constructed matrix.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if successful, FALSE otherwise.                       *
*****************************************************************************/
static int MatGenMatGeneric(IPObjectStruct *LstObjList, IrtHmgnMatType Mat)
{
    int i, j;
    IPObjectStruct *Row, *Col;

    MatGenUnitMat(Mat);                             /* Make it unit matrix, */

    if (!IP_IS_OLST_OBJ(LstObjList)) {
	IRIT_WNDW_PUT_STR("Matrix: Not object list object!");
	return FALSE;
    }

    for (i = 0; i < 4; i++) {
	if ((Row = IPListObjectGet(LstObjList, i)) == NULL) {
	    IRIT_WNDW_PUT_STR("Matrix: Four rows expected, found less");
	    return FALSE;
	}
	if (!IP_IS_OLST_OBJ(Row)) {
	    IRIT_WNDW_PUT_STR("None list object found in list");
	    return FALSE;
	}

	for (j = 0; j < 4; j++) {
	    if ((Col = IPListObjectGet(Row, j)) == NULL) {
		IRIT_WNDW_PUT_STR("Matrix: Four columns expected, found less.");
		return FALSE;
	    }

	    if (!IP_IS_NUM_OBJ(Col)) {
		IRIT_WNDW_PUT_STR("Numeric value expected.");
		return FALSE;
	    }

	    Mat[i][j] = Col -> U.R;
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a BOX primitive for IRIT.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:          Low end corner of BOX.                                      M
*   WidthX:      Width of BOX (X axis).                                      M
*   WidthY:      Depth of BOX( Y axis).                                      M
*   WidthZ:      Height of BOX( Z axis).                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A BOX primitive.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBOXObject                                                             M
*****************************************************************************/
IPObjectStruct *GenBOXObject(IrtVecType Pt,
			     IrtRType *WidthX,
			     IrtRType *WidthY,
			     IrtRType *WidthZ)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenBOXObject(Pt, *WidthX, *WidthY, *WidthZ)) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to BOX primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a GBOX primitive for IRIT.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:                Low end corner of GBOX.				     M
*   Dir1, Dir2, Dir3:  Three independent directional vectors to define GBOX. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A GBOX primitive.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenGBOXObject                                                            M
*****************************************************************************/
IPObjectStruct *GenGBOXObject(IrtVecType Pt,
			      IrtVecType Dir1,
			      IrtVecType Dir2,
			      IrtVecType Dir3)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenGBOXObject(Pt, Dir1, Dir2, Dir3)) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to GBOX primitive\n");

    return PObj;

}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a CONE primitive for IRIT.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Center location of Base of CONE.                             M
*   Dir:        Direction and distance from Pt to apex of CONE.              M
*   R:          Radius of Base of the cone.                                  M
*   Bases:      0 for none, 1 for bottom, 2 for top, 3 for both.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A CONE primitive.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenCONEObject                                                            M
*****************************************************************************/
IPObjectStruct *GenCONEObject(IrtVecType Pt,
			      IrtVecType Dir,
			      IrtRType *R,
			      IrtRType *Bases)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenCONEObject(Pt, Dir, *R, (int) (*Bases))) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to CONE primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a CONE2 primitive for IRIT.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:      Center location of Base of CON2.                                M
*   Dir:     Direction and distance from Pt to center of other base of CON2. M
*   R1, R2:  Two base radii of the truncated CON2                            M
*   Bases:      0 for none, 1 for bottom, 2 for top, 3 for both.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A CONE2 primitive.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenCONE2Object                                                           M
*****************************************************************************/
IPObjectStruct *GenCONE2Object(IrtVecType Pt,
			       IrtVecType Dir,
			       IrtRType *R1,
			       IrtRType *R2,
			       IrtRType *Bases)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenCONE2Object(Pt, Dir, *R1, *R2, (int) (*Bases))) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to CONE2 primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a CYLINder primitive for IRIT.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:         Center location of Base of CYLINder.                         M
*   Dir:        Direction and distance from Pt to other base of cylinder.    M
*   R:          Radius of Base of the cylinder.                              M
*   Bases:      0 for none, 1 for bottom, 2 for top, 3 for both.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A CYLIN primitive.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenCYLINObject                                                           M
*****************************************************************************/
IPObjectStruct *GenCYLINObject(IrtVecType Pt,
			       IrtVecType Dir,
			       IrtRType *R,
			       IrtRType *Bases)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenCYLINObject(Pt, Dir, *R, (int) (*Bases))) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to CYLIN primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a SPHERE primitive for IRIT.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:   Center location of SPHERE.                                     M
*   R:        Radius of sphere.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A SPHERE primitive.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSPHEREObject                                                          M
*****************************************************************************/
IPObjectStruct *GenSPHEREObject(IrtVecType Center, IrtRType *R)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenSPHEREObject(Center, *R)) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to SPHERE primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a TORUS primitive for IRIT.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:  Center location of the TORUS primitive.                         M
*   Normal:  Normal to the major plane of the torus.                         M
*   Rmajor:  Major radius of torus.                                          M
*   Rminor:  Minor radius of torus.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A TOURS primitive.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTORUSObject                                                           M
*****************************************************************************/
IPObjectStruct *GenTORUSObject(IrtVecType Center,
			       IrtVecType Normal,
			       IrtRType *Rmajor,
			       IrtRType *Rminor)
{
    IPObjectStruct *PObj;

    PrimSetResolution(GetResolution(TRUE));

    if ((PObj = PrimGenTORUSObject(Center, Normal, *Rmajor, *Rminor)) == NULL)
	IRIT_WNDW_PUT_STR("Invalid parameters to TORUS primitive\n");

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a POLYDISK primitive for IRIT.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   N:         Normal to the plane this disk included in.                    M
*   T:         A translation factor of the center of the disk.               M
*   R:         Radius of teh disk.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A POLYDISK primitive.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPOLYDISKObject                                                        M
*****************************************************************************/
IPObjectStruct *GenPOLYDISKObject(IrtVecType N, IrtVecType T, IrtRType *R)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenPOLYDISKObject(N, T, *R);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a POLYGON primitive for IRIT.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:     List of vertices/points to construct as a polygon/line.    M
*   RIsPolyline:   If TRUE, make a polyline, otherwise a polygon.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A POLYGON primitive.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPOLYGONObject                                                         M
*****************************************************************************/
IPObjectStruct *GenPOLYGONObject(IPObjectStruct *PObjList,
				 IrtRType *RIsPolyline)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenPOLYGONObject(PObjList, IRIT_REAL_PTR_TO_INT(RIsPolyline));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an object from a list of polys.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:      List of polygonal objects.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A single object containing all polygons in all       M
*                       provided objects, by a simple union.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenObjectFromPolyList                                                    M
*****************************************************************************/
IPObjectStruct *GenObjectFromPolyList(IPObjectStruct *PObjList)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenObjectFromPolyList(PObjList);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Marges polyline in the given list of polylines. Polyliness are merged if   M
* they share an end point.			                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:      List of polyline objects.                                 M
*   Eps:	   Tolerance to consider two end points the same.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Merged list of polylines.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenObjectFromPolyList                                                    M
*****************************************************************************/
IPObjectStruct *GenObjectFromPolylineList(IPObjectStruct *PObj,
					  IrtRType *Eps)
{
    int i;
    IPPolygonStruct *Pl,
	*PlHead = NULL;
    IPObjectStruct *PPl;

    if (IP_IS_OLST_OBJ(PObj)) {
        for (i = 0; (PPl = IPListObjectGet(PObj, i++)) != NULL; ) {
	    if (!IP_IS_POLY_OBJ(PPl)) {
	        IRIT_NON_FATAL_ERROR("Non poly object found in list.  Aborted.");
		IPFreePolygonList(PlHead);
		return NULL;
	    }
	    Pl = IPCopyPolygonList(PPl -> U.Pl);
	    PlHead = IPAppendPolyLists(Pl, PlHead);
	}
    }
    else if (IP_IS_POLY_OBJ(PObj)) {
        PlHead = IPCopyPolygonList(PObj -> U.Pl);
    }
    else {
        IRIT_NON_FATAL_ERROR("Expecting either a poly or a list object.  Aborted.");
	return NULL;
    }

    PlHead = GMMergePolylines(PlHead, *Eps);

    return IPGenPOLYLINEObject(PlHead);}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Inserts a new polygons to existing polygonal object, in place.             M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:      New polygon to insert into PPolys object.                    M
*   PPolys:     Existing polygon object to insert PPoly into, in place.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if succesful, FALSE otherwise.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InsertPolyToPoly                                                         M
*****************************************************************************/
int InsertPolyToPoly(IPObjectStruct *PPoly, IPObjectStruct *PPolys)
{
    IPPolygonStruct *Pl;

    if (!IP_IS_POLY_OBJ(PPoly) || !IP_IS_POLY_OBJ(PPolys))
	return FALSE;

    Pl = IPCopyPolygonList(PPoly -> U.Pl);
    IPGetLastPoly(Pl) -> Pnext = PPolys -> U.Pl;
    PPolys -> U.Pl = Pl;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Not supported.                                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:                                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:                                                        *
*****************************************************************************/
IPObjectStruct *GenCROSSECObject(IPObjectStruct *PObj)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenCROSSECObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a SURFREV primitive for IRIT.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming a surface of revolution.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A SURFREV primitive.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSURFREVObject                                                         M
*****************************************************************************/
IPObjectStruct *GenSURFREVObject(IPObjectStruct *Cross)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenSURFREVObject(Cross);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a SURFREVAXS primitive for IRIT.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Axis forming a surface of revolution.    M
*   Axis:      Axis of rotation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A SURFREVAXS primitive.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSURFREVAxisObject                                                     M
*****************************************************************************/
IPObjectStruct *GenSURFREVAxisObject(IPObjectStruct *Cross,
				     IrtVecType Axis)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenSURFREVAxisObject(Cross, Axis);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a SURFREV2 primitive for IRIT.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming a surface of revolution.  M
*   StartAngle, EndAngle:  angles of portion of surface of revolution.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A SURFREV2 primitive.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSURFREV2Object                                                        M
*****************************************************************************/
IPObjectStruct *GenSURFREV2Object(IPObjectStruct *Cross,
				  IrtRType *StartAngle,
				  IrtRType *EndAngle)
{
    PrimSetResolution(GetResolution(TRUE));

    if (IRIT_APX_EQ_EPS(*StartAngle, *EndAngle, IRIT_UEPS)) {
	IRIT_NON_FATAL_ERROR("Start angle must be different than end angle.");
	return NULL;
    }
    return PrimGenSURFREV2Object(Cross, *StartAngle, *EndAngle);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a SURFREV primitive for IRIT.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Axis forming a surface of revolution.    M
*   StartAngle, EndAngle:  angles of portion of surface of revolution.	     M
*   Axis:      Axis of rotation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A SURFREV primitive.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSURFREV2AxisObject                                                    M
*****************************************************************************/
IPObjectStruct *GenSURFREV2AxisObject(IPObjectStruct *Cross,
				      IrtRType *StartAngle,
				      IrtRType *EndAngle,
				      IrtVecType Axis)
{
    PrimSetResolution(GetResolution(TRUE));

    return PrimGenSURFREV2AxisObject(Cross, *StartAngle, *EndAngle, Axis);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates an EXTRUDE primitive for IRIT.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To extrude in direction Dir.                                  M
*   Dir:       Direction and magnitude of extrusion.                         M
*   RBases:    Which, if any, of the bases to include in the extrusion.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An EXTRUDE primitive.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenEXTRUDEObject                                                         M
*****************************************************************************/
IPObjectStruct *GenEXTRUDEObject(IPObjectStruct *Cross,
				 IrtVecType Dir,
				 IrtRType *RBases)
{
    if (IP_IS_OLST_OBJ(Cross)) {
        int i;
        IPObjectStruct *PTmp,
	    *PRetVal = IPGenLISTObject(NULL);

	for (i = 0; (PTmp = IPListObjectGet(Cross, i)) != NULL; i++) {
	    IPListObjectAppend(PRetVal,
			       GenEXTRUDEObject(PTmp, Dir, RBases));
	}
	return PRetVal;
    }
    else if (IP_IS_SRF_OBJ(Cross)) {
	CagdVecStruct Vec;
        TrivTVStruct *TV;

	IRIT_VEC_COPY(Vec.Vec, Dir);
	if ((TV = TrivExtrudeTV(Cross -> U.Srfs, &Vec)) == NULL)
	    return NULL;

	return IPGenTRIVARObject(TV);	
    }
    else {
        PrimSetResolution(GetResolution(TRUE));

	return PrimGenEXTRUDEObject(Cross, Dir, IRIT_REAL_PTR_TO_INT(RBases));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Data reduces/Decimate a given polygonal model.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolyObj:      To data reduce/Decimate.                                  M
*   RDecimType:    TRUE for numeric (0 to 1) decimation threshold, FALSE for M
8		   exact polygonal budgeting.		                     M
*   RThreshold:    Number of reduction passes.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Decimated/Data reduced polygonal object.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenDecimatedObject, data reduction, decimation                           M
*****************************************************************************/
IPObjectStruct *GenDecimatedObject(IPObjectStruct *PPolyObj,
				   IrtRType *RDecimType,
				   IrtRType *RThreshold)
{
    IPObjectStruct *PRed;
    VoidPtr Hds = HDSCnvrtPObj2QTree(PPolyObj, 25);

    if (IRIT_REAL_TO_INT(*RDecimType))
	PRed = HDSThreshold(Hds, *RThreshold);
    else
        PRed = HDSTriBudget(Hds, IRIT_REAL_TO_INT(*RThreshold));

    HDSFreeQTree(Hds);

    return PRed;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimates curvature properties at vertices of the given polygonal model. M
* Model is assumed to consist of triangles only and is regular.              M
*   Each vertex will be assigned "KCurv" and "HCurv" attributes with the     M
* estimated Gaussian and Mean curvatures, respectively.                      M
*   Estimation is performed via a paraboloid fit near a vertex to            M
* RNumOfRings around the estimated vertex.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolyObj:     Polygonal model to estimate the curvature at its vertices. M
*   RNumOfRings:  Number of neighborhood rings for the poarabolid fit.       M
*   RCubicFit:    TRUE for a cubic fit, FALSE for quadratic fit.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A copy of PPOlyObj with curvature attributes set.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlCrvtrSetCurvatureAttr                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyCurvatureApprox                                                      M
*****************************************************************************/
IPObjectStruct *PolyCurvatureApprox(IPObjectStruct *PPolyObj,
				    IrtRType *RNumOfRings,
				    IrtRType *RCubicFit)
{
    int OldFit = GMPlCrvtrSetFitDegree(IRIT_REAL_TO_INT(*RCubicFit));

    PPolyObj = IPCopyObject(NULL, PPolyObj, FALSE);

    GMPlCrvtrSetCurvatureAttr(PPolyObj -> U.Pl, IRIT_REAL_TO_INT(*RNumOfRings),
			      TRUE);

    GMPlCrvtrSetFitDegree(OldFit);

    return PPolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimates importance properties at vertices of given polygonal model.    M
* Model is assumed to consist of triangles only and is regular.              M
*   Each vertex will be assigned a "SilImp" attribute with the estimated     M
* importance of the vertex.				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPolyObj:    Polygonal model to estimate the importance at its vertices. M
*   RGetRange:   TRUE to extract the edge ranges of the importance map.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A copy of PPolyObj with importance attributes set.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlSilImportanceAttr                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyImportanceApprox                                                     M
*****************************************************************************/
IPObjectStruct *PolyImportanceApprox(IPObjectStruct *PPolyObj,
				     IrtRType *RGetRange)
{
    IPPolygonStruct *PlRange;

    PPolyObj = IPCopyObject(NULL, PPolyObj, FALSE);

    GMPlSilImportanceAttr(PPolyObj -> U.Pl);

    if (!IRIT_APX_EQ(*RGetRange, 0.0)) {
	PlRange = GMPlSilImportanceRange(PPolyObj -> U.Pl);

	IPFreeObject(PPolyObj);

	return IPGenPOLYLINEObject(PlRange);
    }
    else
	return PPolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Metamorphosis of two polygonal objects.                                  M
*   The two polygonal objects must be equal topologically - same number of   M
* polygons and same number of vertices in each corresponding polygon.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2:   Two polygons to blend.                                       M
*   t:          Blending factor.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     Blended polyhedra.                                 M
*                                                                            *
* SEE ALSO:    GMPolygonalMorphosis	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoPolysMorphing                                                         M
*****************************************************************************/
IPObjectStruct *TwoPolysMorphing(IPObjectStruct *Pl1,
				 IPObjectStruct *Pl2,
				 IrtRType *t)
{
    return IPGenPOLYObject(GMPolygonalMorphosis(Pl1 -> U.Pl, Pl2 -> U.Pl, *t));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Derive properties as curves on polygonal models - isophotes, curvature   M
* curves (parabolic curves, etc.), etc.					     M
*                                                                            *
* PARAMETERS:                 						     M
*   PObj:      A polygonal model.                                            M
*   RPropType: Property type - 1 for isophotes, 2 for Gaussian curvature,    M
*	       3 for Mean curvature.					     M
*   PropParam: A list object holding the parameters of the properties:	     M
*              0 Attribute   - list( AttrName, AttrValue )		     M
*              1 Isophotes   - list( ViewDir, InclinationAngle )	     M
*	       2 Gauss crvtr - list( NumRingCrvtrAprx, CrvtrVal )	     M
*	       3 Mean crvtr  - list( NumRingCrvtrAprx, CrvtrVal )	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     Polylines on the polygonal model possessing the    M
*			  property value.				     M
*                                                                            *
* SEE ALSO:    GMPolyPropFetchIsophotes, GMPolyPropFetchCurvature	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolyPropFetch                                                            M
*****************************************************************************/
IPObjectStruct *PolyPropFetch(IPObjectStruct *PObj,
			      IrtRType *RPropType,
			      IPObjectStruct *PropParam)
{
    IPObjectStruct *PParam1, *PParam2;
    IPPolygonStruct
	*PlRes = NULL;

    switch (IRIT_REAL_PTR_TO_INT(RPropType)) {
	default:
	case 0:
	    PParam1 = IPListObjectGet(PropParam, 0);
	    PParam2 = IPListObjectGet(PropParam, 1);
	    if (IP_IS_STR_OBJ(PParam1) && IP_IS_NUM_OBJ(PParam2)) {
	        PlRes = GMPolyPropFetchAttribute(PObj -> U.Pl,
						 PParam1 -> U.Str,
						 PParam2 -> U.R);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Property params of \"list(AttrName, AttrValue)\" expected.");
	    }
            break;
	case 1:
	    PParam1 = IPListObjectGet(PropParam, 0);
	    PParam2 = IPListObjectGet(PropParam, 1);
	    if (IP_IS_VEC_OBJ(PParam1) && IP_IS_NUM_OBJ(PParam2)) {
	        PlRes = GMPolyPropFetchIsophotes(PObj -> U.Pl,
						 PParam1 -> U.Vec,
						 PParam2 -> U.R);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Property params of \"list(ViewDir, InclinationAngle)\" expected.");
	    }
            break;
	case 2:
	    PParam1 = IPListObjectGet(PropParam, 0);
	    PParam2 = IPListObjectGet(PropParam, 1);
	    if (IP_IS_NUM_OBJ(PParam1) && IP_IS_NUM_OBJ(PParam2)) {
	        PlRes = GMPolyPropFetchCurvature(PObj -> U.Pl, 0,
						 IRIT_REAL_TO_INT(PParam1 -> U.R),
						 PParam2 -> U.R);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Property params of \"list(NumRingCrvtrAprx, CrvtrVal)\" expected.");
	    }
            break;
	case 3:
	    PParam1 = IPListObjectGet(PropParam, 0);
	    PParam2 = IPListObjectGet(PropParam, 1);
	    if (IP_IS_NUM_OBJ(PParam1) && IP_IS_NUM_OBJ(PParam2)) {
	        PlRes = GMPolyPropFetchCurvature(PObj -> U.Pl, 1,
						 IRIT_REAL_TO_INT(PParam1 -> U.R),
						 PParam2 -> U.R);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Property params of \"list(NumRingCrvtrAprx, CrvtrVal)\" expected.");
	    }
            break;
    }

    return IPGenPOLYLINEObject(PlRes);
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Primitive shape fitting, for cloud of points/polys describing the shape. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Poly object to fit a primitive shape to.                       M
*   FitType:  Type of shape to fit to: plane, cone, torus, etc.  See         M
*	      GMFittingModelType in geom_lib.h.				     M
*   Tol:      Of fitted shape.					  	     M
*   NumIters: Number of iterations to randomly fit and test for outliers.    M
*	      A 100 is a good start.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list object of numeric values with the median     M
*	      distance error from the point cloud points to fitted shape     M
*             as first parameter in the list, followed by:		     M
*               FitType is GM_FIT_PLANE:    A, B, C, D of plane equation.    M
*               FitType is GM_FIT_SPHERE:   Xcntr, Ycntr, Zcntr, Radius.     M
*               FitType is GM_FIT_CYLINDER: Xcntr, Ycntr, Zcntr,             M
*                                           Xdir, Ydir, Zdir, Radius.	     M
*		FitType is GM_FIT_CIRCLE:   Xcntr, Ycntr, Radius.	     M
*		FitType is GM_FIT_CONE:     Xcntr, Ycntr, Zcntr,             M
*                                           Xdir, Ydir, Zdir, Radius.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMFitObjectWithOutliers                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   FitPrim2PolyModel                                                        M
*****************************************************************************/
IPObjectStruct *FitPrim2PolyModel(IPObjectStruct *PObj,
				  IrtRType *FitType,
				  IrtRType *Tol,
				  IrtRType *NumIters)
{
    int i, n;
    IrtRType
	ModelExtParams[GM_FIT_MODEL_MAX_PARAM];
    IrtRType
        Result = GMFitObjectWithOutliers(PObj -> U.Pl, IRIT_REAL_TO_INT(*FitType),
					 ModelExtParams, *Tol,
					 IRIT_REAL_TO_INT(*NumIters));
    IPObjectStruct
        *RetObj = IPGenLISTObject(NULL);

    IPListObjectInsert(RetObj, 0, IPGenNUMValObject(Result));
    switch ((GMFittingModelType) IRIT_REAL_TO_INT(*FitType)) {
	case GM_FIT_OTHER:
        default:
            IRIT_NON_FATAL_ERROR("Invalid shape to fit to requested.");
            break;
	case GM_FIT_PLANE:
	    n = 4;
            break;
	case GM_FIT_SPHERE:
	    n = 4;
            break;
	case GM_FIT_CYLINDER:
	    n = 7;
            break;
	case GM_FIT_CIRCLE:
	    n = 3;
            break;
	case GM_FIT_CONE:
	    n = 7;
            break;
    }

    for (i = 0; i < n; i++)
        IPListObjectInsert(RetObj, i + 1,
			   IPGenNUMValObject(ModelExtParams[i]));

    IPListObjectInsert(RetObj, i + 1, NULL);

    return RetObj;
}
