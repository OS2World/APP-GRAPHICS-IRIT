/*****************************************************************************
* Module to read OBJ files into IRIT data.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai                          Ver 1.0, October 2009  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "objirlst.h"
#include "attribut.h"

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

const IPO2IInt3Type IP_I2O_VEC_INIT = {-1,-1,-1};
const int IP_I2O_SCALAR_INIT = -1;
IRIT_STATIC_DATA const char
    IP_I2O_SUB_TRIM_ATTRIB[] = "_subTrims";

typedef int IPI2OInt2Type[2];

typedef enum IPI2OCrvSrfTypeType {
    IP_I2O_NONE,
    IP_I2O_BEZIER,
    IP_I2O_BSPLINE
} IPI2OCrvSrfTypeType;

typedef enum IPI2OPtTypesType {
    IP_I2O_V,
    IP_I2O_VN,
    IP_I2O_VP,
    IP_I2O_VT
} IPI2OPtTypesType;

typedef struct IPI2OSaveFileDataStruct {
    char *FileName;
    FILE *ObjFile, *MtlFile;
    /* Current material information. */
    char *Map_Kd;
    IPO2IInt3Type Kd;
    IrtRType Tf, Ks, Ns, Ni;
    /* Current surface/curve type information. */
    IPI2OCrvSrfTypeType CrvSrfType;
    int Rat, DegU, DegV;
} IPI2OSaveFileDataStruct;

/* A counter to help create unique name to each material. */
IRIT_STATIC_DATA int IPI2OUniqueNum;
/* Whether to display warning messages or not. */
IRIT_STATIC_DATA int IPI2OWarningMsgs;
/* Whether each polygon use its own vertices or gather similar vertices      */
/* together.                                                                 */
IRIT_STATIC_DATA int IPI2OUniqueVertices;
IRIT_STATIC_DATA jmp_buf IPI2OLongJumpBuffer;              /* Used in error. */
IRIT_STATIC_DATA IPI2OSaveFileDataStruct *IPI2OData;

static void IPI2OFinishSaveFile();
static void IPI2OInitSaveFile(const char *OBJFileName);
static void IPI2OWriteVnpt(IPI2OPtTypesType Type, 
                           const IrtRType *Vertex, 
                           int Rat,
                           int WeightFirst);
static void IPI2OWriteVnpt2(IPI2OPtTypesType Type, 
                            IrtRType V1, 
                            IrtRType V2, 
                            IrtRType V3, 
                            IrtRType V4, 
                            int Rat,
                            int WeightFirst);
static void IPI2OWritePoly(const IPPolygonStruct *Pl, 
                           const IPObjectStruct *PObj);
static void IPI2OWritePolyList(IPPolyVrtxIdxStruct *PVIdx, 
                               const IPObjectStruct *PObj);
static void IPI2OWriteSrfCrvPts(CagdRType * const *CrvSrfPts, 
                                IPI2OPtTypesType PtStatement,
                                int NumOfPts, 
                                int Rat);
static void IPI2OWriteCrv(const CagdCrvStruct *Crv, int TrimmingCurv);
static void IPI2OWriteSrf(const CagdSrfStruct *Srf, TrimSrfStruct *TrimSrfs);
static void IPI2OWritePt(const IrtPtType Pt);
static void IPI2ONewGeometricObject(const IPObjectStruct *PObj);
static void IPI2OWriteData(const IPObjectStruct *PObj);
static void IPI2ONewSrfCrvParams(IPI2OCrvSrfTypeType CrvSrfType, 
                                 int Rat, 
                                 int DegU, 
                                 int DegV);
static void IPI2OUsemtl();
static void IPI2ONewMtl(const IPObjectStruct *PObj);
static int IPI2OCmpTextures(const char *Text1, const char* Text2);

#ifdef USE_VARARGS
static void IPI2OErrorAndExit(const char *va_alist, ...);
static void IPI2OWarning(const char *va_alist, ...);
#else
static void IPI2OErrorAndExit(const char *Format, ...);
static void IPI2OWarning(const char *Format, ...);
#endif /* USE_VARARGS */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Convert IRIT data structure to an OBJ file.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:           IRIT data structure to convert.                          M
*   OBJFileName:    Name of OBJ file to write the result to.                 M
*   WarningMsgs:    Whether to display warning messages or not.              M
*   UniqueVertices: If true, don't gather identical vertices to one vector.  M
*                   Each polygon uses its own vertices.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if succeeded.                                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOBJLoadFile                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOBJSaveFile                                                            M
*****************************************************************************/
int IPOBJSaveFile(const IPObjectStruct *PObj, 
                  const char *OBJFileName,
                  int WarningMsgs,
                  int UniqueVertices) 
{
    IPI2OSaveFileDataStruct Data;

    IPI2OData = &Data;
    IPI2OWarningMsgs = WarningMsgs;
    IPI2OUniqueVertices = UniqueVertices;
    
    if (setjmp(IPI2OLongJumpBuffer) == 0) {
        IPI2OInitSaveFile(OBJFileName);

        IPI2OWriteData(PObj);

        IPI2OFinishSaveFile();
	return TRUE;
    }
    else {
        IPI2OFinishSaveFile();
        return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Release resources after saving an obj file.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OFinishSaveFile()
{
    if (IPI2OData -> ObjFile != NULL)
	fclose(IPI2OData -> ObjFile);
    if (IPI2OData -> MtlFile != NULL)
	fclose(IPI2OData -> MtlFile);
    if (IPI2OData -> FileName != NULL)
	IritFree(IPI2OData -> FileName);
    if (IPI2OData -> Map_Kd != NULL)
	IritFree(IPI2OData -> Map_Kd);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   open OBJFileName.                                                        *
*   Initialize the variables required for saving.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   OBJFileName: Name of OBJ file to save to. NULL means standard output.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OInitSaveFile(const char *OBJFileName)
{
    /* Open the file. */
    IPI2OData -> FileName = _IPO2ITrimWhiteSpaces(OBJFileName);
    if (IPI2OData -> FileName == NULL) {
        IPI2OData -> ObjFile = stdout;  
        IPI2OData -> FileName = IritStrdup("<standard input>");
    }
    else if ((IPI2OData -> ObjFile = fopen(IPI2OData -> FileName, "w")) == NULL) 
        IPI2OErrorAndExit("Cannot open OBJ file \"%s\", exiting.\n", 
                                IPI2OData -> FileName);
    IPI2OData -> MtlFile = NULL;
    IPI2OUniqueNum = 0;
    IPI2OData -> Map_Kd = NULL;
    IRIT_GEN_COPY(IPI2OData -> Kd, IP_I2O_VEC_INIT, sizeof(IPO2IInt3Type));
    IPI2OData -> Tf = IP_I2O_SCALAR_INIT;
    IPI2OData -> Ks = IP_I2O_SCALAR_INIT;
    IPI2OData -> Ns = IP_I2O_SCALAR_INIT;
    IPI2OData -> Ni = IP_I2O_SCALAR_INIT;
    IPI2OData -> CrvSrfType = IP_I2O_NONE;
    IPI2OData -> Rat = IP_I2O_SCALAR_INIT;
    IPI2OData -> DegU = IP_I2O_SCALAR_INIT;
    IPI2OData -> DegV = IP_I2O_SCALAR_INIT;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the given vertex according to the given type.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Type:        The type of the vertex (v, vn, vt or vp).                   *
*   Vertex:      The coordinates of the vertex.                              *
*   Rat:         Whether this is a rational vertex. (Relevant only for v and *
*                vp).                                                        *
*   WeightFirst: TRUE if the first element of Vertex is the weight. FALSE if *
*                if the last vertex is the weight.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteVnpt(IPI2OPtTypesType Type, 
                           const IrtRType *Vertex, 
                           int Rat,
                           int WeightFirst)
{
    char *TypeString;
    IrtRType v[4], w;

    if (!WeightFirst) {
        v[1] = Vertex[0];
        v[2] = Vertex[1];
        v[3] = Vertex[2];
        if (Rat) {
            if (Type == IP_I2O_VP)
                v[0] = Vertex[2];
            else if (Type == IP_I2O_V)
                v[0] = Vertex[3];
            else
                Rat = FALSE;
        }
    }
    else
        memcpy(v, Vertex, 4 * sizeof(IrtRType));

    w = Rat ? 1/v[0] : 1;
    switch (Type) {
        case IP_I2O_V: 
            TypeString = "v";
            break;
        case IP_I2O_VN: 
            TypeString = "vn";
            /* Insert normal direction to be compatible with OBJ format. */
            IRIT_PT_SCALE(v + 1, -1 );
            break;
        case IP_I2O_VP: 
            TypeString = "vp";
            break;
        case IP_I2O_VT: 
            TypeString = "vt";
            break;
        default:
            TypeString = "";
            assert(0);
    }
    fprintf(IPI2OData -> ObjFile, "%s %s %s", TypeString, 
            _IPReal2Str(w*v[1]), _IPReal2Str(w*v[2]));
    if ((Type == IP_I2O_V) || (Type == IP_I2O_VN)) 
        fprintf(IPI2OData -> ObjFile, " %s", _IPReal2Str(w*v[3]));
    if (Rat) 
        fprintf(IPI2OData -> ObjFile, " %s", _IPReal2Str(v[0]));
    fprintf(IPI2OData -> ObjFile, "\n");
}
 
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the given vertex according to the given type.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Type:        The type of the vertex (v, vn, vt or vp).                   *
*   V1, V2, V3:  The coordinates of the vertex.                              *
*   Rat:         Whether this is a rational vertex. (Relevant only for v and *
*                vp).                                                        *
*   WeightFirst: TRUE if the first element of Vertex is the weight. FALSE if *
*                if the last vertex is the weight.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteVnpt2(IPI2OPtTypesType Type, 
                            IrtRType V1, 
                            IrtRType V2, 
                            IrtRType V3, 
                            IrtRType V4, 
                            int Rat,
                            int WeightFirst)
{
    IrtRType v[4];

    v[0] = V1;
    v[1] = V2;
    v[2] = V3;
    v[3] = V4;

    IPI2OWriteVnpt(Type, v, Rat, WeightFirst);
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the polygons/polylines/points in Pl to the Obj file.               *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:   The polygons/polylines/point to write.                             *
*   PObj: The IPObjectStruct that contains Pl.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWritePoly(const IPPolygonStruct *Pl, 
                           const IPObjectStruct *PObj)
{
    int i,
        v = 0, 
        n = 0, 
        t = 0;
    IPVertexStruct
        *Pv = Pl -> PVertex;

    do {
        float *UV;

        v--;
        IPI2OWriteVnpt(IP_I2O_V, Pv -> Coord, FALSE, FALSE);
        if ((IP_IS_POLYGON_OBJ(PObj) || IP_IS_POLYLINE_OBJ(PObj)) &&
            (UV = AttrGetUVAttrib(Pv -> Attr, "uvvals")) != NULL) {
            IrtRType Temp[2];

	    Temp[0] = UV[0];
	    Temp[1] = UV[1];

            t--;
            IPI2OWriteVnpt(IP_I2O_VT, Temp, FALSE, FALSE);
        }
        if (IP_IS_POLYGON_OBJ(PObj) && IP_HAS_NORMAL_VRTX(Pv)) {
            n--;
            IPI2OWriteVnpt(IP_I2O_VN, Pv -> Normal, FALSE, FALSE);
        }

	Pv = Pv -> Pnext;
    }
    while (Pv != NULL && Pv != Pl -> PVertex);

    if (IP_IS_POLYGON_OBJ(PObj))
        fprintf(IPI2OData -> ObjFile, "f");
    else if (IP_IS_POLYLINE_OBJ(PObj))
        fprintf(IPI2OData -> ObjFile, "l");
    else if (IP_IS_POINTLIST_OBJ(PObj))
        fprintf(IPI2OData -> ObjFile, "p");
    else
        assert(0);

    for (i = v; i < 0; i++) {
        /* Starting new line each 10 vectors. */
        if ((((i - v) % 10 ) == 0) && (v != i))
            fprintf(IPI2OData -> ObjFile, "\\\n\t");

        /* Print the vector itself. */
        fprintf(IPI2OData -> ObjFile, " %i", i);

        /* All vertices have texture vector. Print texture vector. */
        if (t == v) 
            fprintf(IPI2OData -> ObjFile, "/%i", i);

        /* All vertices have normals. Print normal vector.*/
        if (n == v) {
            if (t != v)
                fprintf(IPI2OData -> ObjFile, "/");
            fprintf(IPI2OData -> ObjFile, "/%i", i);
        }
    }
    fprintf(IPI2OData -> ObjFile, "\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the polygons/polylines/points in Pl to the Obj file.               *
*                                                                            *
* PARAMETERS:                                                                *
*   PVidx: The polygons/polylines/points to write.                           *
*   PObj:  The IPObjectStruct that contains Pl.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWritePolyList(IPPolyVrtxIdxStruct *PVIdx, 
                               const IPObjectStruct *PObj)
{
    int i,
	n = 0, 
        t = 0;
    IPVertexStruct
        **Pv = PVIdx -> Vertices;

    /* Holds for the normals and textrue the index of their vector. Index    */
    /* 0 is the first normal/texture vector that was written and counting up */
    /* from there. Therefore, in order to achieve the right index for the obj*/
    /* file, one need to reduce number of normal/texture vectors from this   */
    /* index.                                                                */
    IPI2OInt2Type
        *Indices = malloc(PVIdx -> NumVrtcs * sizeof(IPI2OInt2Type));

    /* Print all vertices and print their index */
    for (i = 0; Pv[i] != NULL; i++) {
        IPI2OWriteVnpt(IP_I2O_V, Pv[i] -> Coord, FALSE, FALSE);
    }
    for (i = 0; Pv[i] != NULL; i++) {
        float *UV;

        if ((UV = AttrGetUVAttrib(Pv[i] -> Attr, "uvvals")) != NULL) {
            IrtRType Temp[2];

	    Temp[0] = UV[0];
	    Temp[1] = UV[1];

            IPI2OWriteVnpt(IP_I2O_VT, Temp, FALSE, FALSE);
            Indices[i][0] = t;
            t++;
        }
    }
    for (i = 0; Pv[i] != NULL; i++) {
        if (IP_HAS_NORMAL_VRTX(Pv[i])) {
            IPI2OWriteVnpt(IP_I2O_VN, Pv[i] -> Normal, FALSE, FALSE);
            Indices[i][1] = n;
            n++;
        }
    }

    /* Print each polygon/polyline/points list. */
    for (i = 0; i <= PVIdx -> NumPlys - 1; i++) {
        int *Pl;
        int j = 0,
            v1 = 0,
            n1 = 0,
            t1 = 0;

        if (IP_IS_POLYGON_OBJ(PObj))
            fprintf(IPI2OData -> ObjFile, "f");
        else if (IP_IS_POLYLINE_OBJ(PObj))
            fprintf(IPI2OData -> ObjFile, "l");
        else if (IP_IS_POINTLIST_OBJ(PObj))
            fprintf(IPI2OData -> ObjFile, "p");
        else
            assert(0);

        /* Counting number of vertices/normal/uvvals in this polygon. */
        for (Pl = PVIdx -> Polygons[i]; *Pl != -1; Pl++) {
            v1++;
            if (AttrGetUVAttrib(Pv[*Pl] -> Attr, "uvvals") != NULL)
                t1++;
            if (IP_HAS_NORMAL_VRTX(Pv[*Pl]))
                n1++;
        }
        for (Pl = PVIdx -> Polygons[i]; *Pl != -1; Pl++, j++) {
            /* Starting new line each 10 vectors. */
            if (((i % 10 ) == 0) && (i != 0))
                fprintf(IPI2OData -> ObjFile, "\\\n\t");
            /* Print the vector itself. */
            fprintf(IPI2OData -> ObjFile, " %i", *Pl - PVIdx -> NumVrtcs);
            /* All vertices have texture vector. Print texture vector. */
            if (t1 == v1) 
                fprintf(IPI2OData -> ObjFile, "/%i", Indices[*Pl][0] - t);
            /* All vertices have normals. Print normal vector. */
            if (n1 == v1) {
                if (t1 != v1)
                    fprintf(IPI2OData -> ObjFile, "/");
                fprintf(IPI2OData -> ObjFile, "/%i", Indices[*Pl][1] - n);
            }
        }
        fprintf(IPI2OData -> ObjFile, "\n");
    }
    free(Indices);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the points used by the surface/curve.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvSrfPts:   The points to be written.                                   *
*   PtStatement: The type of the points (In this context it's v or vp).      *
*   NumOfPts:    Number of points to write.                                  *
*   Rat:         Whether the points are rational.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteSrfCrvPts(CagdRType * const *CrvSrfPts, 
                                IPI2OPtTypesType PtStatement,
                                int NumOfPts, 
                                int Rat)
{
    int i;

    for (i = 0; i <= NumOfPts - 1; i++) {
        IPI2OWriteVnpt2(PtStatement, Rat ? CrvSrfPts[0][i] : 1, CrvSrfPts[1][i],
            CrvSrfPts[2][i], (PtStatement == IP_I2O_V) ? CrvSrfPts[3][i] : 0, 
            Rat, TRUE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the Curves in Crvs to the Obj file.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:         The curves to write.                                       *
*   TrimmingCurv: Whether it's trimming curve which use parametric points.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   NULL                                                                     *
*****************************************************************************/
static void IPI2OWriteCrv(const CagdCrvStruct *Crv, int TrimmingCurv)
{
    int Rat, DegU, VecIndex;
    CagdRType u[2];
    CagdCrvStruct
        *TempCrv = NULL;
    IPI2OPtTypesType PtType;
    char *CrvStatement;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) <= 1) {
        IPI2OWarning("Encountered unsupported element: CURVE with E1 or P1 points");
        return;
    }
    Rat = CAGD_IS_RATIONAL_CRV(Crv);
    /* Curve with dimension different from 3 is transformed to 3 dimension  */
    /* curve.                                                               */
    if ((CAGD_NUM_OF_PT_COORD(Crv -> PType) != 3) && !TrimmingCurv) {
        if (Rat)
            TempCrv = CagdCoerceCrvTo(Crv, CAGD_PT_P3_TYPE, FALSE);
        else
            TempCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);
        Crv = TempCrv;
    }
    /* Trimming curve with dimension different from 2 is transformed to 2    */
    /* dimension curve.                                                      */
    else if ((CAGD_NUM_OF_PT_COORD(Crv -> PType) != 2) && TrimmingCurv) {
        if (Rat)
            TempCrv = CagdCoerceCrvTo(Crv, CAGD_PT_P2_TYPE, FALSE);
        else
            TempCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
        Crv = TempCrv;
    }
    /* Transform power curves to bezier curves. */
    if (Crv -> GType == CAGD_CPOWER_TYPE) {
        CagdCrvStruct *TempCrv2 = TempCrv;

        TempCrv = CagdCnvrtPwr2BzrCrv(Crv);
        CagdCrvFree(TempCrv2);
        Crv = TempCrv;
    }
    /* Transform periodic bspline curves to not periodic. */
    else if ((Crv -> GType == CAGD_CBSPLINE_TYPE) && 
        (CAGD_IS_PERIODIC_CRV(Crv))) {
        CagdCrvStruct *TempCrv2 = TempCrv;

        TempCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
        CagdCrvFree(TempCrv2);
        Crv = TempCrv;
    }
    if (TrimmingCurv) {
        PtType = IP_I2O_VP;
        CrvStatement = "curv2";
    }
    else {
        PtType = IP_I2O_V;
        CrvStatement = "curv";
    }

    /* Printing all the vectors required for the curve. */
    IPI2OWriteSrfCrvPts(Crv -> Points, PtType, Crv -> Length, Rat);

    /* Printing curve type, rational and degree statements if necessary. */
    switch (Crv -> GType) {
        case CAGD_CBEZIER_TYPE: {
            DegU = Crv -> Length - 1;
            IPI2ONewSrfCrvParams(IP_I2O_BEZIER, Rat, DegU, IP_I2O_SCALAR_INIT);
            break;
        }
        case CAGD_CBSPLINE_TYPE: {
            DegU = Crv -> Order - 1;
            IPI2ONewSrfCrvParams(IP_I2O_BSPLINE, Rat, DegU, IP_I2O_SCALAR_INIT);
            break;
        }
        default:
	    DegU = 0;
            assert(0);  
    }

    /* Printing the curve statement. */
    fprintf(IPI2OData -> ObjFile, CrvStatement);
    /* Printing the parametric ranges unless it's trimming curve. */
    CagdCrvDomain(Crv, &u[0], &u[1]);
    if (!TrimmingCurv)
        fprintf(IPI2OData -> ObjFile, " %s %s", _IPReal2Str(u[0]), 
        _IPReal2Str(u[1]));

    /* Printing the control points. */
    for (VecIndex = -Crv -> Length; VecIndex <= -1; VecIndex++) {
        if (((Crv -> Length + VecIndex) % 10 == 0) && 
            (Crv -> Length + VecIndex != 0))
            /* Starting new line each 10 vectors. */
            fprintf(IPI2OData -> ObjFile, " \\\n\t");
        fprintf(IPI2OData -> ObjFile, " %i", VecIndex);
    }
    fprintf(IPI2OData -> ObjFile, "\n");
    switch (Crv -> GType) {
        case CAGD_CBEZIER_TYPE:
            fprintf(IPI2OData -> ObjFile, "parm u %s %s\n", _IPReal2Str(u[0]), 
                _IPReal2Str(u[1]));
            break;

        /* Printing the knot vectors. */
        case CAGD_CBSPLINE_TYPE: {
            int i,
                Knots = DegU + Crv -> Length + 1;

            fprintf(IPI2OData -> ObjFile, "parm u");
            for (i = 0; i <= Knots - 1; i++) {
                if ((i % 10 == 0) && (i != 0))
                    /* Starting new line each 10 vectors. */
                    fprintf(IPI2OData -> ObjFile, " \\\n\t");
                fprintf(IPI2OData -> ObjFile, " %s", 
                    _IPReal2Str(Crv -> KnotVector[i]));
            }
            fprintf(IPI2OData -> ObjFile, "\n");
            break;
	}
        default:
            assert(0);  
    }
    fprintf(IPI2OData -> ObjFile, "end\n");

    CagdCrvFree(TempCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the surfaces in srfs or TrimSrfs to the Obj file.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Srfs:     The surface to write or NULL if TrimSrfs contains a trimmed    *
*             surface.                                                       *
*   TrimSrfs: The trimmed surface to write or NULL if Srf contains a surface.*
*             If both Srfs and TrimSrfs aren't NULL then Srfs is ignored.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteSrf(const CagdSrfStruct *Srf, TrimSrfStruct *TrimSrfs) 
{

    int Rat, DegU, DegV, VecIndex, CtlPtNum, i ,j,
        TrimCrvsNum = 0;
    CagdRType Min[2], Max[2], *UVals, *VVals;
    CagdSrfStruct *TempSrf = NULL;

    if (TrimSrfs != NULL) {
        TrimSrfs = TrimSrfCopy(TrimSrfs);
        Srf = TrimSrfs -> Srf;
    }
    if (CAGD_NUM_OF_PT_COORD(Srf -> PType) <= 1) {
        IPI2OWarning("Encountered unsupported element: SURFACE with E1 or P1 points");
        return;
    }
    Rat = CAGD_IS_RATIONAL_SRF(Srf);
    /* surface with dimension different from 3 is transformed to 3           */
    /* dimension surface.                                                    */
    if (CAGD_NUM_OF_PT_COORD(Srf -> PType) != 3) {
            if (Rat)
                TempSrf = CagdCoerceSrfTo(Srf, CAGD_PT_P3_TYPE, FALSE);
            else
                TempSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
        Srf = TempSrf;
    }
    /* Transform power surfaces to bezier surfaces. */
    if (Srf -> GType == CAGD_SPOWER_TYPE) {
        CagdSrfStruct
	    *TempSrf2 = TempSrf;

        TempSrf = CagdCnvrtPwr2BzrSrf(Srf);
        Srf = TempSrf;
        CagdSrfFree(TempSrf2);
    }
    /* Transform periodic bspline surfaces to not periodic. */
    else if ((Srf -> GType == CAGD_SBSPLINE_TYPE) && 
        (CAGD_IS_UPERIODIC_SRF(Srf) || CAGD_IS_VPERIODIC_SRF(Srf))){
        CagdSrfStruct
	    *TempSrf2 = TempSrf;

        TempSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
        Srf = TempSrf;
        CagdSrfFree(TempSrf2);
    }

    /* Transform the trimming curves to a format which determine when one    */
    /* curve contains another one. Also, print all the trimming curves to    */
    /* the obj file (curv2 elements).                                        */
    if (TrimSrfs != NULL) {
        TrimCrvStruct
	    *TrimCrvList = TrimSrfs -> TrimCrvList;

        TrimSrfs -> TrimCrvList =     
            TrimChainTrimmingCurves2Loops(TrimCrvList);
        TrimCrvFreeList(TrimCrvList);
        if ((TrimSrfs -> TrimCrvList == NULL) || 
            !TrimClassifyTrimmingLoops(&TrimSrfs -> TrimCrvList)) {
            IPI2OWarning("Error classifing the trimming curves of the surface. Printing the surface without trimmings");
            TrimSrfFree(TrimSrfs);
            TrimSrfs = NULL;
        }
        else {
            TrimCrvStruct
	        *TrimCrv = TrimSrfs -> TrimCrvList;
            int i;

            for(i = 0; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext, i++) {
                TrimCrvStruct *SubTrimCrv;
    
                IPI2OWriteCrv(TrimCrv -> TrimCrvSegList -> UVCrv, TRUE);
                SubTrimCrv = (TrimCrvStruct *) AttrGetPtrAttrib(TrimCrv -> Attr,
                    IP_I2O_SUB_TRIM_ATTRIB);
                for (; SubTrimCrv != NULL; SubTrimCrv = SubTrimCrv -> Pnext) {
                    i++;
                    IPI2OWriteCrv(SubTrimCrv -> TrimCrvSegList -> UVCrv, TRUE);
                }
            }
            TrimCrvsNum = i;
        }
    }

    /* Printing all the vectors required for the surface. */
    IPI2OWriteSrfCrvPts(Srf -> Points, IP_I2O_V,
			Srf -> ULength * Srf -> VLength, Rat);

    /* If there's a texture, printing all the texture vectors required for   */
    /* the surface.                                                          */
    UVals = CagdSrfNodes(Srf, CAGD_CONST_U_DIR);
    VVals = CagdSrfNodes(Srf, CAGD_CONST_V_DIR);
    if (IPI2OData -> Map_Kd != NULL) {
        for (i = 0; i <= Srf -> VLength - 1; i++)
            for (j = 0; j <= Srf -> ULength - 1; j++)
                IPI2OWriteVnpt2(IP_I2O_VT, UVals[j], VVals[i], 0, 0, FALSE, FALSE);
    }

    /* Printing normal vectors. */
    for (i = 0; i <= Srf -> VLength - 1; i++)
        for (j = 0; j <= Srf -> ULength - 1; j++) { 
            CagdVecStruct *Vec;

            Vec = CagdSrfNormal(Srf, UVals[j], VVals[i], TRUE);
            IPI2OWriteVnpt(IP_I2O_VN, Vec -> Vec, FALSE, FALSE);
        }
    
    /* Printing surface type, rational and degree statements if necessary. */
    switch (Srf -> GType) {
        case CAGD_SBEZIER_TYPE: {
            DegU = Srf -> ULength - 1;
            DegV = Srf -> VLength - 1;
            IPI2ONewSrfCrvParams(IP_I2O_BEZIER, Rat, DegU, DegV);
            break;
        }
        case CAGD_SBSPLINE_TYPE: {
            DegU = Srf -> UOrder - 1;
            DegV = Srf -> VOrder - 1;
            IPI2ONewSrfCrvParams(IP_I2O_BSPLINE, Rat, DegU, DegV);
            break;
        }
        default:
	    DegU = DegV = 0;
            assert(0);  
    }

    /* Printing the surface statement. */
    CagdSrfDomain(Srf, &Min[0], &Max[0], &Min[1], &Max[1]);
    fprintf(IPI2OData -> ObjFile, "surf");
    fprintf(IPI2OData -> ObjFile, " %s %s %s %s", _IPReal2Str(Min[0]), 
	    _IPReal2Str(Max[0]), _IPReal2Str(Min[1]), _IPReal2Str(Max[1]));

    /* Printing the control points. */
    CtlPtNum = Srf -> ULength * Srf ->VLength;
    for (VecIndex = -CtlPtNum ; VecIndex <= -1; VecIndex++) {
        if (((CtlPtNum + VecIndex) % 10 == 0) && 
            (CtlPtNum + VecIndex != 0))
            /* Starting new line each 10 vectors. */
            fprintf(IPI2OData -> ObjFile, " \\\n\t");
        if (IPI2OData -> Map_Kd != NULL) /* Print texture vertices as well. */
            fprintf(IPI2OData -> ObjFile, " %i/%i/%i", VecIndex, VecIndex, 
                VecIndex);
        else /* Don't print texture vertices. */
            fprintf(IPI2OData -> ObjFile, " %i//%i", VecIndex, VecIndex);
    }

    fprintf(IPI2OData -> ObjFile, "\n");
    switch (Srf -> GType) {
        case CAGD_SBEZIER_TYPE: {
            fprintf(IPI2OData -> ObjFile, "parm u %s %s\n",
		    _IPReal2Str(Min[0]),
                _IPReal2Str(Max[0]));
            fprintf(IPI2OData -> ObjFile, "parm v %s %s\n",
		    _IPReal2Str(Min[1]),
                _IPReal2Str(Max[1]));
            break;
        }
        case CAGD_SBSPLINE_TYPE: {             /* Printing the knot vectors. */
            int i,
                KnotsU = DegU + Srf -> ULength + 1,
                KnotsV = DegV + Srf -> VLength + 1;

            fprintf(IPI2OData -> ObjFile, "parm u");
            for (i = 0; i <= KnotsU - 1; i++) {
                if ((i % 10 == 0) && (i != 0))
                    /* Starting new line each 10 vectors. */
                    fprintf(IPI2OData -> ObjFile, " \\\n\t");
                fprintf(IPI2OData -> ObjFile, " %s", 
                    _IPReal2Str(Srf -> UKnotVector[i]));
            }
            fprintf(IPI2OData -> ObjFile, "\n");
            fprintf(IPI2OData -> ObjFile, "parm v");
            for (i = 0; i <= KnotsV - 1; i++) {
                if ((i % 10 == 0) && (i != 0))
                    /* Starting new line each 10 vectors. */
                    fprintf(IPI2OData -> ObjFile, " \\\n\t");
                fprintf(IPI2OData -> ObjFile, " %s", 
                    _IPReal2Str(Srf -> VKnotVector[i]));
            }
            fprintf(IPI2OData -> ObjFile, "\n");
            break;
        }
        default:
            assert(0);  
    }

    /* Printing the trimming curves inside the surface (trim and hole        */
    /* statements.                                                           */
    if (TrimSrfs != NULL) {
        TrimCrvStruct
	    *TrimCrv = TrimSrfs -> TrimCrvList;

        for(i = 0; TrimCrv != NULL; TrimCrv = TrimCrv -> Pnext, i++) {
            TrimCrvStruct *SubTrimCrv;
            CagdRType  u[2];

            CagdCrvDomain(TrimCrv -> TrimCrvSegList -> UVCrv, &u[0], &u[1]);
            fprintf(IPI2OData -> ObjFile, "trim %s %s %d\n", _IPReal2Str(u[0]),
                _IPReal2Str(u[1]), -TrimCrvsNum + i);
            SubTrimCrv = (TrimCrvStruct *) AttrGetPtrAttrib(TrimCrv -> Attr,
                IP_I2O_SUB_TRIM_ATTRIB);
            for (; SubTrimCrv != NULL; SubTrimCrv = SubTrimCrv -> Pnext) {
                i++;
                CagdCrvDomain(SubTrimCrv -> TrimCrvSegList -> UVCrv, &u[0], 
                    &u[1]);
                fprintf(IPI2OData -> ObjFile, "hole %s %s %d\n", 
                    _IPReal2Str(u[0]), _IPReal2Str(u[1]), -TrimCrvsNum + i);
            }
        }
    }
    fprintf(IPI2OData -> ObjFile, "end\n");

    if (TrimSrfs!=NULL)
        TrimSrfFree(TrimSrfs);
    CagdSrfFree(TempSrf);
    IritFree(UVals);
    IritFree(VVals);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the given point.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt: The point to write.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWritePt(const IrtPtType Pt) 
{
    IPI2OWriteVnpt(IP_I2O_V, Pt, FALSE, FALSE);
    fprintf(IPI2OData -> ObjFile, "P -1\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the given control point as a point.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   CtlPt: The control point to write.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteCtlPt(const CagdCtlPtStruct *CtlPt) 
{
    int i, j;

    for (i = 0; CtlPt != NULL; CtlPt = CtlPt -> Pnext, i++) {
        IrtPtType Pt;
        IrtRType 
            *Pt2 = (IrtRType*)CtlPt -> Coords;

        CagdCoerceToE3(Pt, &Pt2, -1, CtlPt -> PtType);
        IPI2OWriteVnpt(IP_I2O_V, Pt, FALSE, FALSE);
    }

    fprintf(IPI2OData -> ObjFile, "P");
    for (j = -i; j <= -1; j++) {
        if ((((j - i) % 10 ) == 0) && (j != i))
            fprintf(IPI2OData -> ObjFile, "\\\n\t");
        fprintf(IPI2OData -> ObjFile, " %d", j);
    }
    fprintf(IPI2OData -> ObjFile, "\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle issues when starting new geometric object.                        *
*   Start new object.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: The new geometric object.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2ONewGeometricObject(const IPObjectStruct *PObj)
{
    fprintf(IPI2OData -> ObjFile, "o %s_%d\n", IP_GET_OBJ_NAME(PObj), 
                                               IPI2OUniqueNum++);
    /* Add useMtl statement if necessary. */
    IPI2ONewMtl(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Write the data in PObj to the Obj file.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: The object to write to file.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OWriteData(const IPObjectStruct *PObj)
{
    switch (PObj -> ObjType) {
        case IP_OBJ_LIST_OBJ: {
	    int i;
            IPObjectStruct *PTmp;

            for (i = 0; (PTmp = IPListObjectGet(PObj, i)) != NULL; i++)
                IPI2OWriteData(PTmp);
            break;
        }
        case IP_OBJ_POLY: {
            if (IP_IS_POLYSTRIP_OBJ(PObj)) {
                IPI2OWarning("Encountered unsupported element: POLYSTRIP");
                break;
            }
            IPI2ONewGeometricObject(PObj);
            if (IPI2OUniqueVertices) {
                IPPolygonStruct
		    *Pl = PObj -> U.Pl;

                for ( ;Pl != NULL; Pl = Pl -> Pnext)
                    IPI2OWritePoly(Pl, PObj);
            }
            else {
                IPPolyVrtxIdxStruct *PVIdx;

                PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, 0, 1 | 2);
                IPI2OWritePolyList(PVIdx, PObj);
            }
            break;
        }
        case IP_OBJ_CURVE: {
            CagdCrvStruct
	        *Crvs = PObj -> U.Crvs;

            IPI2ONewGeometricObject(PObj);
            for( ; Crvs != NULL; Crvs = Crvs -> Pnext)
                IPI2OWriteCrv(Crvs, FALSE);
            break;
        }
        case IP_OBJ_SURFACE: {
            CagdSrfStruct
	        *Srfs = PObj -> U.Srfs;

            IPI2ONewGeometricObject(PObj);
            for( ; Srfs != NULL; Srfs = Srfs -> Pnext)
                IPI2OWriteSrf(Srfs, NULL);
            break;
        }
        case IP_OBJ_TRIMSRF: {
            TrimSrfStruct
	        *TrimSrfs = PObj -> U.TrimSrfs;

            IPI2ONewGeometricObject(PObj);
            for( ; TrimSrfs != NULL; TrimSrfs = TrimSrfs -> Pnext)
                IPI2OWriteSrf(NULL, TrimSrfs);
            break;
        }
        case IP_OBJ_POINT: {
            IPI2ONewGeometricObject(PObj);
            if (ATTR_OBJ_ATTR_EXIST(PObj, "LIGHT_SOURCE")) {
                IPI2OWarning("Encountered unsupported element: POINT as LIGHT_SOURCE");
                break;
            }
            IPI2OWritePt(PObj -> U.Vec);
            break;
        }
        case IP_OBJ_VECTOR: {
            IPI2ONewGeometricObject(PObj);
            IPI2OWritePt(PObj -> U.Pt);
            break;
        }
        case IP_OBJ_CTLPT: {
            IPI2ONewGeometricObject(PObj);
            IPI2OWriteCtlPt(&PObj -> U.CtlPt);
            break;
        }
        case IP_OBJ_NUMERIC: {
            IPI2OWarning("Encountered unsupported element: NUMBER");
            break;
        }
        case IP_OBJ_PLANE: {
            IPI2OWarning("Encountered unsupported element: IP_OBJ_PLANE");
            break;
        }
        case IP_OBJ_MATRIX: {
            IPI2OWarning("Encountered unsupported element: MATRIX");
            break;
        }
        case IP_OBJ_STRING: {
            IPI2OWarning("Encountered unsupported element: STRING");
            break;
        }
        case IP_OBJ_TRIVAR: {
            IPI2OWarning("Encountered unsupported element: TRIVAR");
            break;
        }
        case IP_OBJ_INSTANCE: {
            IPI2OWarning("Encountered unsupported element: INSTANCE");
            break;
        }
        case IP_OBJ_TRISRF: {
            IPI2OWarning("Encountered unsupported element: TRISRF");
            break;
        }
        case IP_OBJ_MODEL: {
            IPI2OWarning("Encountered unsupported element: IP_OBJ_MODEL");
            break;
        }
        case IP_OBJ_MULTIVAR: {
            IPI2OWarning("Encountered unsupported element: MULTIVAR");
            break;
        }
        default:
            IPI2OWarning("Encountered unknown element recognized as ObjType %d", 
                PObj -> ObjType);
            break;
    }

}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check whether it's required to print a statement for any of the given    *
*   and if it required, print it.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: An object to check its material.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2ONewSrfCrvParams(IPI2OCrvSrfTypeType CrvSrfType, 
                                 int Rat, 
                                 int DegU, 
                                 int DegV)
{
    if ((CrvSrfType != IPI2OData -> CrvSrfType) || (Rat != IPI2OData -> Rat)) {
        char *Cstype;

        switch (CrvSrfType) {
            case IP_I2O_BEZIER:
                Cstype = "bezier";
                break;
            case IP_I2O_BSPLINE:
                Cstype = "bspline";
                break;
            default:
		Cstype = NULL;
                assert(0);
        }

        fprintf(IPI2OData -> ObjFile, "cstype %s%s\n",
		Rat == TRUE ? "rat " : "", Cstype);
    }

    if ((DegU != IPI2OData -> DegU) || (DegV != IPI2OData -> DegV)) {
        if (DegV != IP_I2O_SCALAR_INIT) 
            fprintf(IPI2OData -> ObjFile, "deg %d %d\n", DegU, DegV);
        else 
            fprintf(IPI2OData -> ObjFile, "deg %d\n", DegU);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add usemtl statement with the current material. If needed add mtllib     *
*   statement as well and create the mtl file.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   None								     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2OUsemtl(void)
{
    char MtlName[10];
    char Texture[IRIT_LINE_LEN_LONG];
    double Scale[3];
    int Flip;

    if (IPI2OData -> MtlFile == NULL) {
        char FileName[IRIT_LINE_LEN + 1];
        int Len = (int) (strlen(IPI2OData -> FileName) - 3);

        strcpy(FileName, IPI2OData -> FileName);
        if ((Len <= 0) || 
            (stricmp(FileName + Len - 1, ".obj") != 0)) 
            strcat(FileName,".mtl");
        else
            strcpy(FileName + Len - 1, ".mtl");
        if ((IPI2OData -> MtlFile = fopen(FileName, "w")) == NULL) 
            IPI2OErrorAndExit("Cannot open mtl file \"%s\", exiting.\n", 
                              FileName);    
        fprintf(IPI2OData -> ObjFile, "mtllib %s\n", FileName);
    }
    sprintf(MtlName, "Mtl_%d", IPI2OUniqueNum++);
    fprintf(IPI2OData -> ObjFile, "usemtl %s\n", MtlName);


    fprintf(IPI2OData -> MtlFile, "newmtl %s\n", MtlName);
    if (IPI2OData -> Map_Kd != NULL) {
        IrtImgParsePTextureString(IPI2OData -> Map_Kd, Texture,  Scale, &Flip);
        fprintf(IPI2OData -> MtlFile, "map_Kd %s\n", Texture);
    }
    if (IRIT_GEN_CMP(IP_I2O_VEC_INIT, IPI2OData -> Kd, 
        sizeof(IPO2IInt3Type)) != 0)
        fprintf(IPI2OData -> MtlFile, "Kd %s %s %s\n", 
            _IPReal2Str(IPI2OData -> Kd[0]/255.0),
            _IPReal2Str(IPI2OData -> Kd[1]/255.0), 
            _IPReal2Str(IPI2OData -> Kd[2]/255.0));
    if (!IRIT_APX_EQ(IPI2OData -> Tf, IP_I2O_SCALAR_INIT))
        fprintf(IPI2OData -> MtlFile, "Tf %s %s %s\n", 
            _IPReal2Str(IPI2OData -> Tf), _IPReal2Str(IPI2OData -> Tf), 
            _IPReal2Str(IPI2OData -> Tf));
    if (!IRIT_APX_EQ(IPI2OData -> Ks, IP_I2O_SCALAR_INIT))
        fprintf(IPI2OData -> MtlFile, "Ks %s %s %s\n", 
            _IPReal2Str(IPI2OData -> Ks), _IPReal2Str(IPI2OData -> Ks), 
            _IPReal2Str(IPI2OData -> Ks));
    if (!IRIT_APX_EQ(IPI2OData -> Ns, IP_I2O_SCALAR_INIT))
        fprintf(IPI2OData -> MtlFile, "Ns %s\n", _IPReal2Str(IPI2OData -> Ns));
    if (!IRIT_APX_EQ(IPI2OData -> Ni, IP_I2O_SCALAR_INIT))
        fprintf(IPI2OData -> MtlFile, "Ni %s\n", _IPReal2Str(IPI2OData -> Ni));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check whether a new material is required for PObj. If so, update the     *
*   current material and add usemtl statement.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: An object to check its material.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPI2ONewMtl(const IPObjectStruct *PObj)
{
    int NewMtl = FALSE;
    IPO2IInt3Type Rgb;
    const char *Ptexture;
    IPAttributeStruct
        *Attr = PObj -> Attr;
    IrtRType Transp, Specular, Specpow, Refract;

    /* ptexture to map_Kd. */
    Ptexture = AttrGetStrAttrib(Attr,"ptexture");
    if (!IPI2OCmpTextures(Ptexture, IPI2OData -> Map_Kd)) {
        NewMtl = TRUE;
        IritFree(IPI2OData -> Map_Kd);
        IPI2OData -> Map_Kd = _IPO2ITrimWhiteSpaces(Ptexture);
    }
    /* RGB/color to Kd. */
    if (AttrGetRGBColor2(Attr, NULL, &Rgb[0], &Rgb[1], &Rgb[2])) {
        if (IRIT_GEN_CMP(Rgb, IPI2OData -> Kd, sizeof(IPO2IInt3Type)) != 0) {
            NewMtl = TRUE;
            IRIT_GEN_COPY(IPI2OData -> Kd, Rgb, sizeof(IPO2IInt3Type));
        }
    }
    else if (IRIT_GEN_CMP(IP_I2O_VEC_INIT, IPI2OData -> Kd, 
        sizeof(IPO2IInt3Type)) != 0) {
        NewMtl = TRUE;
        IRIT_GEN_COPY(IPI2OData -> Kd, IP_I2O_VEC_INIT, 
            sizeof(IPO2IInt3Type));
    }
    /* transp to Tf and d. */
    if ((Transp = AttrGetRealAttrib(Attr, "transp")) != IP_ATTR_BAD_REAL) {
        if (!IRIT_APX_EQ(Transp, IPI2OData -> Tf)) {
            NewMtl = TRUE;
            IPI2OData -> Tf = Transp;
        }        
    }
    else if (!IRIT_APX_EQ(IPI2OData -> Tf, IP_I2O_SCALAR_INIT)) {
        NewMtl = TRUE;
        IPI2OData -> Tf = IP_I2O_SCALAR_INIT;
    }
    /* Specular to Ks. */
    if ((Specular = AttrGetRealAttrib(Attr, "Specular")) != IP_ATTR_BAD_REAL) {
        if (!IRIT_APX_EQ(Specular, IPI2OData -> Ks)) {
            NewMtl = TRUE;
            IPI2OData -> Ks = Specular;
        }        
    }
    else if (!IRIT_APX_EQ(IPI2OData -> Ks, IP_I2O_SCALAR_INIT)) {
        NewMtl = TRUE;
        IPI2OData -> Ks = IP_I2O_SCALAR_INIT;
    }
    /* srf_cosine/specpow to Ns. */
    if ((Specpow = AttrGetRealAttrib(Attr, "Specpow")) == IP_ATTR_BAD_REAL) 
        Specpow = AttrGetRealAttrib(Attr, "srf_cosine");
    if (Specpow != IP_ATTR_BAD_REAL) {
        if (!IRIT_APX_EQ(Specpow, IPI2OData -> Ns)) {
            NewMtl = TRUE;
            IPI2OData -> Ns = Specpow;
        }        
    }
    else if (!IRIT_APX_EQ(IPI2OData -> Ns, IP_I2O_SCALAR_INIT)) {
        NewMtl = TRUE;
        IPI2OData -> Ns = IP_I2O_SCALAR_INIT;
    }
    /* Refract to Ni. */
    if ((Refract = AttrGetRealAttrib(Attr, "ior")) != IP_ATTR_BAD_REAL) {
        if (!IRIT_APX_EQ(Refract, IPI2OData -> Ni)) {
            NewMtl = TRUE;
            IPI2OData -> Ni = Refract;
        }        
    }
    else if (!IRIT_APX_EQ(IPI2OData -> Ni, IP_I2O_SCALAR_INIT)) {
        NewMtl = TRUE;
        IPI2OData -> Ni = IP_I2O_SCALAR_INIT;
    }
    if (NewMtl)
        IPI2OUsemtl();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare the two textures, taking in consideration only the parameters    *
*   relevant to obj file.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Text1, Text2: The two textures strings to compare.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE if the textures are equals.                                    *
*****************************************************************************/
static int IPI2OCmpTextures(const char *Text1, const char* Text2)
{
    char Name1[IRIT_LINE_LEN_LONG], Name2[IRIT_LINE_LEN_LONG];
    IrtRType Scale[3];
    int Flip;

    IrtImgParsePTextureString(Text1, Name1, Scale, &Flip);
    IrtImgParsePTextureString(Text2, Name2, Scale, &Flip);
    if ((Text2 == NULL) && (Text1 == NULL))
        return TRUE;
    if ((Text1 == NULL) || (Text2 == NULL))
        return FALSE;
    return stricmp(Name1, Name2) == 0;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the message (printf style) using IRIT_WARNING_MSG and then do      *
*   longjmp back to IPOBJSaveFile. Add at the end of the message period,     *
*   "Exiting." and new line character.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Format:   The format of the string.                                      *
*   ...:      Parameters for the format.                                     *
*   va_alist: The format of the string and parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IPI2OErrorAndExit(const char *va_alist, ...)
{
    char *Format, *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IPI2OErrorAndExit(const char *Format, ...)
{
    char *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    sprintf(Format2, "%s. Exiting.\n", Format);
    IRIT_VSPRINTF(p, Format2, ArgPtr);
    IRIT_WARNING_MSG(p);

    va_end(ArgPtr);

    longjmp(IPI2OLongJumpBuffer, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the message (printf style) using IRIT_WARNING_MSG and return.      *
*   Add at the end of the message period and new line character.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Format:   The format of the string.                                      *
*   ...:      Parameters for the format.                                     *
*   va_alist: The format of the string and parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IPI2OWarning(const char *va_alist, ...)
{
    char *Format, *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IPI2OWarning(const char *Format, ...)
{
    char *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    if (!IPI2OWarningMsgs) 
        return;
    sprintf(Format2, "%s.\n", Format);
    IRIT_VSPRINTF(p, Format2, ArgPtr);
    IRIT_WARNING_MSG(p);

    va_end(ArgPtr);
}
