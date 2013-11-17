/*****************************************************************************
* Module to read OBJ files into IRIT data.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai                             Ver 1.0, July 2009  *
*****************************************************************************/

#include <ctype.h>

#include "irit_sm.h"
#include "iritprsr.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "attribut.h"
#include "geom_lib.h"

#ifdef __WINNT__
#include <direct.h>
#endif

#ifdef __UNIX__
#include <unistd.h>
#endif /* __UNIX__ */

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include "objirlst.h"

#define IP_O2I_LINE_LEN  IRIT_LINE_LEN_MAX_CFG
IRIT_STATIC_DATA const char
    IP_O2I_CONTINUE_NEXT_LINE = '\\',
    IP_O2I_IN_BODY_TOKENS[] = " PARM TRIM HOLE SCRV SP END ",
    /* Adding MTLLIB so that material can be referenced before being loaded. */
    IP_O2I_VECTORS_TOKENS[] = " V VT VN VP MTLLIB ",
    IP_O2I_CURV2_VECTORS_TOKENS[] = 
			 " V VT VN VP CURV2 CSTYPE DEG BMAT STEP ",
    IP_O2I_MTL_ATTRIB[] = "_IPO2IMtl",
    IP_O2I_NOT_SUPPORTED_PREFIX[] = "IPO2I_",
    IP_O2I_ATTRIB_SP[] = "IPO2I_SP",
    IP_O2I_ATTRIB_SCRV[] = "IPO2I_SCRV",
    IP_O2I_ATTRIB_RES[] = "IPO2I_RES";

IRIT_STATIC_DATA const int
    IP_O2I_READ_ALL_VALUES = -1;

IRIT_STATIC_DATA const IrtRType
    IP_O2I_SMOOT_THRESHOLD = 45;


/* Matrix for convertion from CIE XYZ to RGB. */
IRIT_STATIC_DATA const double
    IPO2I_CIE2RGB_MAT[3][3] = 
        { { 2.565619565, -1.16698913, -0.39851087 },
	  { -1.022094734, 1.978260607, 0.043820979 },
	  { 0.074697994, -0.251850614, 1.176799544 } };

typedef enum IPO2IPolyTypeType {
    IP_O2I_POLY_NONE = 0,
    IP_O2I_POINTS,
    IP_O2I_POLYLINES,
    IP_O2I_POLYGONES
} IPO2IPolyTypeType;

typedef enum IPO2ICrvSrfTypeType {
    IP_O2I_NONE,
    IP_O2I_BEZIER,
    IP_O2I_BMATRIX,
    IP_O2I_BSPLINE,
    IP_O2I_CARDINAL,
    IP_O2I_TAYLOR
} IPO2ICrvSrfTypeType;

typedef enum IPO2IReturnNum {
    IP_O2I_NO_MORE_LINES,
    IP_O2I_READ_TOKEN,
    IP_O2I_LINE_END
} IPO2IReturnNum;

typedef enum IPO2IReadingStage {
    IP_O2I_VECTORS_STAGE,
    IP_O2I_CURV2_STAGE,
    IP_O2I_REST_STAGE,
    IP_O2I_MTL_STAGE
} IPO2IReadingStage;

typedef enum IPO2IResultType {
    IP_O2I_RESULT_ARRAY,
    IP_O2I_RESULT_LIST
} IPO2IResultType;

typedef struct IPO2ILoadFileDataStruct {
    /* Support for all file types. */
    int LineNum;
    char *FileName, *DirPath;
    FILE *File;
    IPO2IReadingStage Stage;
    char *CurrLine;
    char *NextToken; /* After call to NextToken it will point to the start   */
                     /* of the next token if there are any more tokens       */
                     /* (including tokens in next line if the current line   */
                     /* ends with IP_O2I_CONTINUE_NEXT_LINE). It is NULL     */
                     /* when it gets to end of line or at the initialization */
                     /* of IPO2IData.                                        */
    char *LastToken; /* Whenever IPO2INextToken is called with given Token   */
                     /* buffer, LastToken will point to that buffer.         */
    int ConNextLine; /* If true, IP_O2I_CONTINUE_NEXT_LINE appeard at the end*/
                     /* of this line and we continue read tokens from next   */
                     /* line (Only for IPO2INextToken).                      */
    int EndOfLine;/* We are at end of line. If ConNextLine is TRUE this flag */
                  /* is FALSE.                                               */
    int GoNextLine; /* Force NextpToken to read all the current line and all */
                    /* the next line if ConNextLine is TRUE before reading   */
                    /* tokens. Turn this flag to TRUE in order to read next  */
                    /* line.                                                 */

    /* Support for obj files. */
    int Rat, DegU, DegV, StepU, StepV, Res[2];
    IPO2ICrvSrfTypeType CrvSrfType;
    IPO2IListStruct UMatrix, VMatrix, VVec, VtVec, VpVec, VnVec, Curv2List,
	GroupsList, Lines, Mtls, SGroups;
    IPObjectStruct *Geom;

    /* Support for mtl files. */
    IPO2IMtlStruct Mtl;
} IPO2ILoadFileDataStruct;

/* Parsing function. Parse the following tokens in IPO2IData according to    */
/* certain statement. Call IPO2IErrorAndExit in case of an error.            */
typedef void (*IPO2IParseFuncType)(void);

/* Parsing function. Parse the following tokens in the current map statement */
/* (Given by MapToken) at the current mtl file. Call IPO2IErrorAndExit in    */
/* case of an error.                                                         */
typedef void (*IPO2IParseMtlMapOptFuncType)(char *MapToken);

/* This function interpret the given token. The values are written in Result.*/
/* The function should expect a compatible type of Result and a compatible   */
/*   type of Params.                                                         */
/* Number is the number of this token in the read tokens.                    */
/* It calls IPO2IErrorAndExit in case of wrong format of Token.              */
typedef void (*IPO2IInterpValueFuncType)(char *Token,
                                         void *Result, 
                                         void *Params,
                                         int Number);

typedef struct IPO2IParseStatementsStruct {
    char *Statement;
    IPO2IParseFuncType Func;
} IPO2IParseStatementsStruct;

typedef struct IPO2IParseMtlMapOptStruct {
    char *Statement;
    IPO2IParseMtlMapOptFuncType Func;
} IPO2IParseMtlMapOptStruct;

typedef struct IPO2ICstypeOptsStruct {
    char *Statement;
    IPO2ICrvSrfTypeType Type;
} IPO2ICstypeOptsStruct;

IRIT_STATIC_DATA IPO2ICstypeOptsStruct IPO2ICstypeOpts[] = {
    { "bmatrix",  IP_O2I_BMATRIX },
    { "bezier",   IP_O2I_BEZIER },
    { "bspline",  IP_O2I_BSPLINE },
    { "cardinal", IP_O2I_CARDINAL },
    { "taylor",   IP_O2I_TAYLOR }
};

IRIT_STATIC_DATA jmp_buf IPO2ILongJumpBuffer;              /* Used in error. */
/* Error message used by IPO2IErrorAndExit. Should be similar to this:       */
/* "Error in obj file \"%s\" line %d. %s. Exiting.\n"                        */
/* First %s is file name, %d is line number. Second %s is the specific error */
/* message from the program (Notice that there's a period and \n after it    */
/* which isn't given in the specific message itself                          */
IRIT_STATIC_DATA char *IPO2IOtherErrorMsg;
/* The same as IPO2IOtherErrorMsg but used in IPO2IErrorAndExit.             */
IRIT_STATIC_DATA char *IPO2IOtherWarningMsg;
IRIT_STATIC_DATA IPO2ILoadFileDataStruct *IPO2IData;
/* A counter to help create unique name to each object. */
IRIT_STATIC_DATA int IPO2IUniqueNum;
/* Whether to display warning messages or not. */
IRIT_STATIC_DATA int IPO2IWarningMsgs;
/* Whether to use white diffuse for material with textures. */
IRIT_STATIC_DATA int IPO2IWhiteDiffuseTexture;
/* Whether to ignore transperency when it's full transperency. */
IRIT_STATIC_DATA int IPO2IIgnoreFullTransp;
/* Whether to force smoothing when no s statement exists. */
IRIT_STATIC_DATA int IPO2IForceSmoothing;

static IPObjectStruct *IPO2IOBJLoadFile(const char *OBJFileName, 
                                        int WarningMsgs,
                                        int WhiteDiffuseTexture,
                                        int IgnoreFullTransp,
                                        int ForceSmoothing);
static void IPO2ICIEToRGB(IrtVecType RGBColor, IrtVecType CIEColor);
static void IPO2IParseV(void);
static void IPO2IParseVt(void);
static void IPO2IParseVn(void);
static void IPO2IParseVp(void);
static void IPO2IParseCstype(void);
static void IPO2IParseDeg(void);
static void IPO2IParseBmat(void);
static void IPO2IParseStep(void);
static void IPO2IFinalizeGeometry() ;
static void IPO2IAddToGeom(IPObjectStruct *PObj);
static void IPO2IParsePLF(void);
static void IPO2IAddPtsToCrvSrf(CagdRType ** CrvSrfPts, 
                                IPO2IListStruct *Pts, 
                                IPO2IListStruct *PtsList,
                                int Dim,
                                int Rat);
static void IPO2IParseCurvCurv2(void);
static void IPO2IParseSurfBody(IPO2IListStruct *ParmU, 
                               IPO2IListStruct *ParmV, 
                               TrimCrvStruct **TrimCrvList,
                               IPAttributeStruct **Attrs,
                               int *HasTopLvlTrim);
static void IPO2IParseSurf(void);
static void IPO2IParseBspBzp(void);
static void IPO2IStatementNotSupported(void);
static void IPO2MtlOptNotSupported(char *Statement, char *Option);
static void IPO2IParseUsemtl(void);
static char *IPO2ISplitPath(char *FullPath);
static char *IPO2ICombinePaths(const char *Path1, const char *Path2);
static char *IPO2IGetCurPath(void);
static int IPO2IInitMtlFile(IPO2ILoadFileDataStruct *Data);
static void IPO2IFinishMtlFile(IPO2ILoadFileDataStruct *Data) ;
static void IPO2IAddMaterial(IPO2IListStruct *Mtls, IPO2IMtlStruct *Mtl);
static void IPO2IParseMtllib(void);
static void IPO2IParseGO(void);
static void IPO2IParseS(void);
static void IPO2IParseRes(void);
static char *IPO2IStrToIrit(char *Str);
static void IPO2IParseCall(void);
static int IPO2IParse(void);
static void IPO2IResetDataForRereading(void);
static int IPO2IInitLoadFile(const char *OBJFileName);
static void IPO2IFinishLoadFile(void);
static int IPO2ICheckReference(IPO2IListStruct *List, int *Number);
static int IPO2IReadLine(void);
static int IPO2INextToken2(char *Token, int SetTokenRead);
static int IPO2INextToken(char *Token);
static char *IPO2IReadToEndOfLine(void);
static void IPO2IInterpInts(char *Token, void *Result, void *Params, int Number);
static void IPO2IInterpInt(char *Token, void *Result, void *Params, int Number);
static void IPO2IInterpReal(char *Token, void *Result, void *Params, int Number);
static int IPO2IReadTokens(int Min, 
                           int Max,
                           void *Result,
                           void *Params,
                           IPO2IInterpValueFuncType InterpFunct);
static void IPO2IParseNewmtl(void);
static void IPO2IParseKadsTf(void);
static void IPO2IParseD(void);
static void IPO2IParseIllum(void);
static void IPO2IParseNs(void);
static void IPO2IParseNi(void);
static void IPO2IParseBlenuv(char *MapToken);
static void IPO2IParseBm(char *MapToken);
static void IPO2IParseBoost(char *MapToken);
static void IPO2IParseCc(char *MapToken);
static void IPO2IParseClamp(char *MapToken);
static void IPO2IParseImfchan(char *MapToken);
static void IPO2IParseMm(char *MapToken);
static void IPO2IParseMap(void);
static void IPO2IParseTexres(char *MapToken);
static void IPO2IParseOst(char *MapToken);
static void IPO2IParseMapAat(void);
static void IPO2IParseRefl(void);
static void IPO2IParseSharpness(void);
#ifdef USE_VARARGS
static void IPO2IErrorAndExit(const char *va_alist, ...);
static void IPO2IWarning(const char *va_alist, ...);
#else
static void IPO2IErrorAndExit(const char *Format, ...);
static void IPO2IWarning(const char *Format, ...);
#endif /* USE_VARARGS */

IRIT_STATIC_DATA const IPO2IParseStatementsStruct IPO2IParseStatements[] = 
{   
    {"v", IPO2IParseV},
    {"vt", IPO2IParseVt},
    {"vn", IPO2IParseVn},
    {"vp", IPO2IParseVp},
    {"cstype", IPO2IParseCstype},
    {"deg", IPO2IParseDeg},
    {"bmat", IPO2IParseBmat},
    {"step", IPO2IParseStep},
    {"p", IPO2IParsePLF},
    {"l", IPO2IParsePLF},
    {"f", IPO2IParsePLF},
    {"curv", IPO2IParseCurvCurv2},
    {"curv2", IPO2IParseCurvCurv2},
    {"surf", IPO2IParseSurf},
    {"con", IPO2IStatementNotSupported},
    {"g", IPO2IParseGO},
    {"s", IPO2IParseS},
    {"mg", IPO2IStatementNotSupported},
    {"o", IPO2IParseGO},
    {"bevel", IPO2IStatementNotSupported},
    {"c_interp", IPO2IStatementNotSupported},
    {"d_interp", IPO2IStatementNotSupported},
    {"lod", IPO2IStatementNotSupported},
    {"usemtl", IPO2IParseUsemtl},
    {"mtllib", IPO2IParseMtllib},
    {"res", IPO2IParseRes},
    {"usemap", IPO2IStatementNotSupported},
    {"maplib", IPO2IStatementNotSupported},
    {"shadow_obj", IPO2IStatementNotSupported},
    {"trace_obj", IPO2IStatementNotSupported},
    {"ctech", IPO2IStatementNotSupported},
    {"stech", IPO2IStatementNotSupported},
    {"cmd", IPO2IStatementNotSupported},
    {"call", IPO2IParseCall},
    {"bzp", IPO2IParseBspBzp},
    {"bsp", IPO2IParseBspBzp},
    {"cdc", IPO2IStatementNotSupported},
    {"cdp", IPO2IStatementNotSupported}
};

IRIT_STATIC_DATA const IPO2IParseStatementsStruct IPO2IParseMtlStatements[] = 
{   
    {"newmtl", IPO2IParseNewmtl},
    {"Ka", IPO2IParseKadsTf},
    {"Kd", IPO2IParseKadsTf},
    {"Ks", IPO2IParseKadsTf},
    {"Tf", IPO2IParseKadsTf},
    {"d", IPO2IParseD},
    {"illum", IPO2IParseIllum},
    {"Ns", IPO2IParseNs},
    {"Ni", IPO2IParseNi},
    {"map_Kd", IPO2IParseMap},
    {"map_aat", IPO2IParseMapAat},
    {"refl", IPO2IParseRefl},
    {"sharpness", IPO2IParseSharpness},
    {"map_Ka", IPO2IParseMap},
    {"map_Ks", IPO2IParseMap},
    {"map_Ns", IPO2IParseMap},
    {"map_d", IPO2IParseMap},
    {"disp", IPO2IParseMap},
    {"decal", IPO2IParseMap},
    {"bump", IPO2IParseMap}
};

IRIT_STATIC_DATA const IPO2IParseMtlMapOptStruct IPO2IParseMtlMapOpts[] = 
{   
    {"-blenu", IPO2IParseBlenuv},
    {"-blenv", IPO2IParseBlenuv},
    {"-bm", IPO2IParseBm},
    {"-boost", IPO2IParseBoost},
    {"-cc", IPO2IParseCc},
    {"-clamp", IPO2IParseClamp},
    {"-imfchan", IPO2IParseImfchan},
    {"-mm", IPO2IParseMm},
    {"-o", IPO2IParseOst},
    {"-s", IPO2IParseOst},
    {"-t", IPO2IParseOst},
    {"-texres", IPO2IParseTexres}
};

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Read an OBJ file into IRIT data structure.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   OBJFileName:         Name of OBJ file. NULL means standard input.        M
*   WarningMsgs:         Weather to display warning messages or not.         M
*   WhiteDiffuseTexture: When material has texture and RGB (0,0,0) give it   M
*                        RGB (1,1,1).                                        M
*   IgnoreFullTransp:    Full transperency of material is ignored.           M
*   ForceSmoothing:      If the s statement isn't given use smoothing for    M
*                        all the polygons.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Read OBJ DATA or NULL if error.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOBJSaveFile                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOBJLoadFile                                                            M
*****************************************************************************/
IPObjectStruct *IPOBJLoadFile(const char *OBJFileName, 
                              int WarningMsgs, 
                              int WhiteDiffuseTexture,
                              int IgnoreFullTransp,
                              int ForceSmoothing)
{
    IPO2IOtherErrorMsg = "Error in obj file \"%s\" line %d. %s. Exiting.\n";
    IPO2IOtherWarningMsg = "Obj file \"%s\" line %d. %s.\n";

    return IPO2IOBJLoadFile(OBJFileName, WarningMsgs, WhiteDiffuseTexture, 
			    IgnoreFullTransp, ForceSmoothing);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an OBJ file into IRIT data structure.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   OBJFileName:   Name of OBJ file. NULL means standard input.              *
*   WarningMsgs: Whtether to display warning messages or not.                *
*   WhiteDiffuseTexture: When material has texture and RGB (0,0,0) give it   *
*                        RGB (1,1,1).                                        *
*   IgnoreFullTransp: Full transperency of material is ignored.              *
*   ForceSmoothing:   If the s statement isn't given use smoothing for all   *
*                     the polygons.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Read OBJ DATA or NULL if error.                       *
*****************************************************************************/
static IPObjectStruct *IPO2IOBJLoadFile(const char *OBJFileName, 
                                        int WarningMsgs,
                                        int WhiteDiffuseTexture,
                                        int IgnoreFullTransp,
                                        int ForceSmoothing)
{
    IPObjectStruct
        *Res = NULL;
    IPO2ILoadFileDataStruct Data;

    IPO2IWarningMsgs = WarningMsgs;
    IPO2IWhiteDiffuseTexture = WhiteDiffuseTexture;
    IPO2IIgnoreFullTransp = IgnoreFullTransp;
    IPO2IForceSmoothing = ForceSmoothing;
    IPO2IData = &Data;

    if (!IPO2IInitLoadFile(OBJFileName))
        return NULL;

    if (setjmp(IPO2ILongJumpBuffer)== 0) {
        IPO2IParse();
        Res = Data.Geom;
        Data.Geom = NULL;
    }
    IPO2IFinishLoadFile();
    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert CIE XYZ color to RGB color.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   RGBColor:   The output RGB color. Allowed to be the same variable as     *
*               CIEColor.                                                    *
*   CIEColor:   The input CIE XYZ color.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2ICIEToRGB(IrtVecType RGBColor, IrtVecType CIEColor) 
{
    int i;
    IrtVecType Res;

    for (i = 0; i < 3; i++) {
        Res[i] = IPO2I_CIE2RGB_MAT[i][0] * CIEColor[0] +
		 IPO2I_CIE2RGB_MAT[i][1] * CIEColor[1] +
		 IPO2I_CIE2RGB_MAT[i][2] * CIEColor[2];
	Res[i] = IRIT_BOUND(Res[i], 0.0, 1.0);
    }    
    IRIT_PT_COPY(RGBColor, Res);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a v point.                                                          *
*   If the stage isn't IP_O2I_VECTORS_STAGE just increase the v index and    *
*   return.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseV(void)
{
    IrtRType
        Temp[4] = { 0, 0, 0, 1 };

    IPO2IData -> VVec.Curr++;
    if (IPO2IData -> Stage != IP_O2I_VECTORS_STAGE)
        return;
    if (IPO2IReadTokens(3, 4, Temp, NULL, IPO2IInterpReal) == 4)
        IRIT_PT_SCALE(Temp, Temp[3]);
    _IPO2IAddElement(&IPO2IData -> VVec, Temp);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a vt point.                                                         *
*   If the stage isn't IP_O2I_VECTORS_STAGE just increase the vt index and   *
*   return.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseVt(void)
{
    IrtRType
        Temp[3] = { 0, 0, 0 };

    IPO2IData -> VtVec.Curr++;
    if (IPO2IData -> Stage != IP_O2I_VECTORS_STAGE)
        return;
    IPO2IReadTokens(1, 3, Temp, NULL, IPO2IInterpReal);
    _IPO2IAddElement(&IPO2IData -> VtVec, Temp);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a vn point.                                                         *
*   If the stage isn't IP_O2I_VECTORS_STAGE just increase the vn index and   *
*   return.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseVn(void)
{
    IrtRType Temp[3];

    IPO2IData -> VnVec.Curr++;
    if (IPO2IData -> Stage != IP_O2I_VECTORS_STAGE)
        return;
    IPO2IReadTokens(3, 3, Temp, NULL, IPO2IInterpReal);
    IRIT_PT_SCALE(Temp, -1);
    _IPO2IAddElement(&IPO2IData -> VnVec, Temp);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a vp point.                                                         *
*   If the stage isn't IP_O2I_VECTORS_STAGE just increase the vp index and   *
*   return.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseVp(void)
{
    /* The second parameter doesn't have default, yet we assume the file is  */
    /* correct, otherwise it's the problem of the user.                      */
    IrtRType
        Temp[3] = {0, 0, 1};

    IPO2IData -> VpVec.Curr++;
    if (IPO2IData -> Stage != IP_O2I_VECTORS_STAGE)
        return;
    if (IPO2IReadTokens(1, 3, Temp, NULL, IPO2IInterpReal) == 3) {
        Temp[0] *= Temp[2];
        Temp[1] *= Temp[2];
    }
    _IPO2IAddElement(&IPO2IData -> VpVec, Temp);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a cstype statement.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseCstype(void)
{
    char Token[IRIT_LINE_LEN + 1];

    IPO2IData -> Rat = FALSE;

    while (TRUE) {
        int i, Len;

        if (IPO2INextToken(Token) == IP_O2I_LINE_END)
            IPO2IErrorAndExit("Expecting more parameters");
        if (stricmp(Token, "rat") == 0) {
            if  (IPO2IData -> Rat == TRUE) 
                IPO2IErrorAndExit("The token \"rat\" appears more than once");
            IPO2IData -> Rat = TRUE;
            continue;
        }
        Len = sizeof(IPO2ICstypeOpts) / sizeof (IPO2ICstypeOptsStruct) - 1;
        for (i = 0; i <= Len - 1; i++)
            if (stricmp(Token, IPO2ICstypeOpts[i].Statement) == 0) {
                IPO2IData -> CrvSrfType = IPO2ICstypeOpts[i].Type;
                return;
            }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a deg statement.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseDeg(void)
{
    int Res, Temp[2];

    Res = IPO2IReadTokens(1, 2, Temp, NULL, IPO2IInterpInt);
    IPO2IData -> DegU = Temp[0];
    if (Res == 2)
        IPO2IData -> DegV = Temp[1];
    if ((Temp[0] < 1) || ((Temp[1] < 1) && (Res == 2)))
        IPO2IErrorAndExit("Degree must be positive");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a bmat statement.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseBmat(void)
{
    int Res, u, Len;
    char Token[IRIT_LINE_LEN+1];
    IPO2IListStruct *List;
    IPO2IResultType
        ListResultType = IP_O2I_RESULT_LIST;

    /* Reading u or v. */
    if (IPO2INextToken(Token) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    if (stricmp(Token, "u") == 0) {
        u = TRUE;
        Len = IPO2IData -> DegU + 1;
        List = &IPO2IData -> UMatrix;
    }
    else if (stricmp(Token, "v") == 0) {
        u = FALSE;
        Len = IPO2IData -> DegV + 1;
        List = &IPO2IData -> VMatrix;
    }
    else {
        IPO2IErrorAndExit("Expecting \"u\" or \"v\".\n");
	u = FALSE;
	Len = -1;
	List = NULL;
    }

    if ((u && (IPO2IData -> DegU == -1)) || ((IPO2IData -> DegV == -1))) 
        IPO2IErrorAndExit("\"bmat\" statement is given before a corresponding "
                          "\"deg\" statement");
    /* Reading the matrix itself. */
    _IPO2IClearList(List, FALSE);
    Res = IPO2IReadTokens(Len, Len, List, &ListResultType, IPO2IInterpInt);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a step statement.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseStep(void)
{
    int Res, Temp[2];

    Res = IPO2IReadTokens(1, 2, Temp, NULL, IPO2IInterpInt);
    IPO2IData -> StepU = Temp[0];
    if (Res == 2)
        IPO2IData -> StepV = Temp[1];

    if ((Temp[1] < 0) || (Temp[0] < 0))
        IPO2IErrorAndExit("Step must be non negative");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Go over all the geometry and do:                                         *
*   * Remove empty objects.                                                  *
*   * Replace IP_O2I_MTL_ATTRIB attribute with the attribute of the relevant *
*     material.                                                              *
*   * Do smoothing of polygons in smoothing groups.                          *
*   * Add all groups to the root gropu IPO2IData -> Geom                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IFinalizeGeometry(void) 
{
    int i, Mtl;
    IPO2IListStruct
        *List = &IPO2IData -> SGroups;

    /* Implementing the smoothing group. It must be done before adding the   */
    /* to geometry because adding to geometry will destroy the linked lists  */
    /* used for the smoothing groups.                                        */
    for (i = 0; i <= List -> Len - 1; i++) 
        if (List -> U.Groups[i] -> U.Pl != NULL)
            GMBlendNormalsToVertices(List -> U.Groups[i] -> U.Pl, 
                IP_O2I_SMOOT_THRESHOLD);

    /* Remove empty objects, set the material and gather all objects into  . */
    /* one linked list under one IPObjectStruct.                             */
    List = &IPO2IData -> GroupsList;
    for (i = 0; i <= List -> Len -1; i++) {
        IPObjectStruct
	    *PObj = List -> U.Groups[i];
        IPObjectStruct **PObjList;

        if (PObj -> U.Lst.PObjList == NULL) {
            IPFreeObject(PObj);
            List -> U.Groups[i] = List -> U.Groups[List -> Len - 1];
            List -> Len--;
            i--;
            continue;
        }
        /* All groups beside IPO2IData -> Geom added to IPO2IData -> Geom.   */
        if (i > 0) 
            IPListObjectAppend(IPO2IData -> Geom, PObj);
        for (PObjList = PObj ->U.Lst.PObjList; (*PObjList) != NULL; PObjList++) {
            IPObjectStruct **PObj2, 
                **PObj3 = (*PObjList) -> U.Lst.PObjList;

            /* Replacing matrial index with material's attributes. */
            Mtl = AttrGetIntAttrib((*PObjList) -> Attr, IP_O2I_MTL_ATTRIB);
            if (Mtl != IP_ATTR_BAD_INT) {
                const char ** OldValidLIst;
                IPAttributeStruct *AttrCpy1, *AttrCpy2;

                AttrFreeOneAttribute(&(*PObjList) -> Attr, IP_O2I_MTL_ATTRIB);
                if (Mtl != -1) {
                    OldValidLIst = AttrCopyValidAttrList(NULL);
                    AttrCpy2 = AttrCpy1 = 
                        AttrCopyAttributes(IPO2IData -> Mtls.U.Mtl[Mtl].Attr);
                    AttrCopyValidAttrList(OldValidLIst);
                    for(; AttrCpy1 -> Pnext != NULL; AttrCpy1 = AttrCpy1 -> Pnext);
                    AttrCpy1 -> Pnext = (*PObjList) -> Attr;
                    (*PObjList) -> Attr = AttrCpy2;
                }
            }
            /* Gathering all objects into one linked list. */
            if ((*PObjList) -> ObjType != IP_OBJ_LIST_OBJ)
                continue;
            /* Setting the type of the gathered linked list. */
            (*PObjList) -> ObjType = 
                (*PObjList) -> U.Lst.PObjList[0] -> ObjType;
            if ((*PObjList) -> ObjType == IP_OBJ_POLY)
                (*PObjList) -> Tags = ((*PObjList) -> Tags & 0xFC) 
                    | (*PObjList) -> U.Lst.PObjList[0] -> Tags;
            /* Creating the linked list. */
            (*PObjList) -> U.Pl = NULL;
            for (PObj2 = PObj3; (*PObj2) != NULL; PObj2++) {
                switch ((*PObj2) -> ObjType) {
                    case IP_OBJ_POLY:
                        (*PObj2) -> U.Pl -> Pnext = (*PObjList) -> U.Pl;
                        (*PObjList) -> U.Pl = (*PObj2) -> U.Pl;
                        break;
                    case IP_OBJ_CURVE:
                        (*PObj2) -> U.Crvs -> Pnext = (*PObjList) -> U.Crvs;
                        (*PObjList) -> U.Crvs = (*PObj2) -> U.Crvs;
                        break;
                    case IP_OBJ_SURFACE:
                        (*PObj2) -> U.Srfs -> Pnext = (*PObjList) -> U.Srfs;
                        (*PObjList) -> U.Srfs = (*PObj2) -> U.Srfs;
                        break;
                    case IP_OBJ_TRIMSRF:
                        (*PObj2) -> U.TrimSrfs -> Pnext = 
                            (*PObjList) -> U.TrimSrfs;
                        (*PObjList) -> U.TrimSrfs = (*PObj2) -> U.TrimSrfs;
                        break;
		    default:
		        assert(0);
		        break;
                }
            }
            IritFree(PObj3);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add the object inside PObj to the geometry lists. Put it in the list     *
*   of the same type of geometric objects (polygones, polylines, polypoints, *
*   surfaces, trimmed surfaces or curvs) and the same material.              *
*   If necessary create the new list for that geometric object and material  *
*   and temporary mark it with the index of the material.                    *
*   If PObj contain special curves or special points it gets a list of       *
*   itself.                                                                  *
*   Also, if PObj is a Polygon, add it to its SGroup (If it has one).        *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj: The object to update with material and add to current group.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IAddToGeom(IPObjectStruct *PObj) 
{
    /* We put PObj in two list:                                              */
    /* 1. IPObjectStruct with list of polygones with all the polygones       */
    /*    polylines and points which require smoothing. We put here only the */
    /*    inner geometric of PObj and only for polygones/polylines/points.   */
    /* 2. The geometric list. There are 4 levels for this list.              */
    /*    First level: IPO2IData -> GroupList, an IPObjectStruct list        */
    /*    containing additional list from the second level.                  */
    /*    It also contains third level list for object without obj group or  */
    /*    object name.							     */
    /*    Second level: IPObjectStruct list representing one group name or   */
    /*    object name in the obj file. Each one contains a list of           */
    /*    IPObjectStruct from the third level.                               */
    /*    Third level: IPObjectStruct list representing all the objects from */
    /*    the same type (surface, curve, etc...) and the same material and   */
    /*    the same object/group name represented by their parent list from   */
    /*    second level.                                                      */
    /*    Unique third level: Objects with v2.1 surface or with special      */
    /*    curves/points are held in this level as seperate objects and not   */
    /*    inside a list.                                                     */
    /*    Forth level: The objects in each third level list.                 */
    /*                                                                       */
    /*    The material attributes are held in the third level.               */
    /*    The name of the group/object is held in the third level.           */
    /*    The tags for polygone/polyline/points are held in the forth level. */
    /*    In objects of the unique third level all the above are held in the */
    /*    third level.                                                       */
    /*    The type of the objects held in the third level isn't kept in the  */
    /*    third level. It can be found by checking one of its objects in the */
    /*    forth level.                                                       */

    IPObjectStruct
	*PList2 = NULL,
        *PList = IPO2IData -> GroupsList.U.Groups[IPO2IData -> GroupsList.Curr];
    int i, Len;

    /* Adding PObj to its smoothing group. */
    if ((IPO2IData -> SGroups.Curr != -1) && 
        (PObj -> ObjType ==  IP_OBJ_POLY)) {       
        IPObjectStruct *SPobj = 
            IPO2IData -> SGroups.U.Groups[IPO2IData -> SGroups.Curr];

        PObj -> U.Pl -> Pnext = SPobj -> U.Pl;
        SPobj -> U.Pl = PObj -> U.Pl;
    }

    /* Adding PObj to the geometry. */

    /* Materials with special points/curves or with surfaces from v2.1       */
    /* get their own object.                                                 */
    if ((AttrGetStrAttrib(PObj -> Attr, IP_O2I_ATTRIB_SP) != NULL) ||
        (AttrGetObjAttrib(PObj -> Attr, IP_O2I_ATTRIB_SCRV) != NULL) ||
        (AttrGetStrAttrib(PObj -> Attr, IP_O2I_ATTRIB_RES) != NULL)) {
        /* Giving unique name. */
        char Name[IRIT_LINE_LEN_VLONG];

        sprintf(Name, "%s_Element%d", IP_GET_OBJ_NAME(PList), IPO2IUniqueNum++);
        IP_SET_OBJ_NAME2(PObj,Name);
        AttrSetIntAttrib(&PObj -> Attr, IP_O2I_MTL_ATTRIB, 
            IPO2IData -> Mtls.Curr);
        IPListObjectAppend(PList, PObj);
        return;
    }

    Len = IPListObjectLength(PList);
    /* Looking inside the relevant group a list with the same object type    */
    /* (if it's polygone we need to check for polygone/polyline/pointslist)  */
    /* then we also check that it's the same material (because rgb color can */
    /* be given in irit only in the IPObjectStruct level and not in the      */
    /* inner geometric object level).                                        */
    for (i=0; i <= Len -1; i++) {
        IPObjectStruct *PObj2;
        
        PList2 = IPListObjectGet(PList, i);
        if (PList2 -> ObjType != IP_OBJ_LIST_OBJ)
            continue;
        PObj2 = PList2 -> U.Lst.PObjList[0];
        if ((PObj2 -> ObjType == PObj -> ObjType) &&
            ((PObj -> ObjType != IP_OBJ_POLY) || 
            ((PObj2 -> Tags & 0x03) == (PObj -> Tags & 0x03))) &&
            (AttrGetIntAttrib(PList2 -> Attr, IP_O2I_MTL_ATTRIB) == 
            IPO2IData -> Mtls.Curr))
            break;     
    }

    if (i == Len) {                              /* Need to create new list. */
        char Name[IRIT_LINE_LEN_VLONG];

        PList2 = IPGenLISTObject(NULL);
        /* Giving unique name. */
        sprintf(Name, "%s_Element%d", IP_GET_OBJ_NAME(PList), IPO2IUniqueNum++);
        IP_SET_OBJ_NAME2(PList2,Name);
        IPListObjectAppend(PList, PList2);
        AttrSetIntAttrib(&PList2 -> Attr, IP_O2I_MTL_ATTRIB, 
            IPO2IData -> Mtls.Curr);
    }
    IPListObjectAppend(PList2, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an p, l or f statement.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParsePLF(void) 
{
    IPO2IListStruct List;
    IPPolygonStruct *Poly;
    IPObjectStruct * PObj;
    IPO2IPolyTypeType PolyType;
    int i,
        Pattern[] = { -1, TRUE, FALSE, FALSE, FALSE },
        PolyCirc = IPSetPolyListCirc(FALSE);

    IPSetPolyListCirc(PolyCirc);
    switch (IPO2IData -> LastToken[0] & ~0x20) {
        case 'L':
            PolyType = IP_O2I_POLYLINES;
            break;
        case 'F':
            PolyType = IP_O2I_POLYGONES;
            break;
        case 'P':
            PolyType = IP_O2I_POINTS;
            break;
        default:
	    assert(0);
            PolyType = IP_O2I_POLY_NONE;
            break;
	    
    }

    _IPO2IInitList(&List,IP_O2I_IVEC_3);
    if (PolyType == IP_O2I_POLYGONES) {
        Pattern[2] = TRUE;
        Pattern[3] = TRUE;
        Pattern[4] = TRUE;
        IPO2IReadTokens(3, IP_O2I_READ_ALL_VALUES, &List,
			Pattern, IPO2IInterpInts);
    }
    else if (PolyType == IP_O2I_POLYLINES) {
        IPO2IReadTokens(2, IP_O2I_READ_ALL_VALUES, &List,
			Pattern, IPO2IInterpInts);
        Pattern[2] = TRUE;
    }
    else
        IPO2IReadTokens(1, IP_O2I_READ_ALL_VALUES, &List,
			Pattern, IPO2IInterpInts);
    Poly = IPAllocPolygon(0, NULL, NULL);

    for (i = 0; i <= List.Len - 1; i++) {
        int *Temp = List.U.IVec[i];

        /* In this point we are accepting the posibility that                */
        /* IPO2ICheckReference will call IPO2IErrorAndExit and we won't be   */
        /* able to free the memory of Poly and its vertices.                 */
        IPO2ICheckReference(&IPO2IData -> VVec, &Temp[0]);
        Poly -> PVertex = IPAllocVertex2(Poly -> PVertex);        
        IRIT_PT_COPY(Poly -> PVertex -> Coord, 
                     IPO2IData -> VVec.U.RPts4[Temp[0]-1]);
        if ((Pattern[0] == 2) || (Pattern[0] == 3)) {
            IrtRType * Vt;
            
            IPO2ICheckReference(&IPO2IData -> VtVec, &Temp[1]);
            Vt = IPO2IData -> VtVec.U.RPts3[Temp[1]-1];
            AttrSetUVAttrib(&Poly -> PVertex -> Attr, "uvvals", Vt[0], Vt[1]);
        }
        if ((PolyType == IP_O2I_POLYGONES) && (Pattern[0] >= 3)) {
            IPO2ICheckReference(&IPO2IData -> VnVec, &Temp[2]);
            IRIT_PT_COPY(Poly -> PVertex -> Normal, 
			 IPO2IData -> VnVec.U.RPts3[Temp[2]-1]);
            IP_SET_NORMAL_VRTX(Poly -> PVertex);
        }
    }

    if (PolyCirc) {
        IPVertexStruct
	    *VLast = IPGetLastVrtx(Poly -> PVertex);

	/* Make the vertex list circular. */
	VLast -> Pnext = Poly -> PVertex;
    }

    PObj = IPGenPOLYObject(Poly);
    if (PolyType == IP_O2I_POLYLINES)
        IP_SET_POLYLINE_OBJ(PObj);
    else if (PolyType == IP_O2I_POINTS)
        IP_SET_POINTLIST_OBJ(PObj);
    else {
        if (IP_HAS_NORMAL_VRTX(PObj -> U.Pl -> PVertex)) {
            IrtPtType Pt;

            IRIT_PT_ADD(Pt, PObj -> U.Pl -> PVertex -> Coord, 
			PObj -> U.Pl -> PVertex -> Normal);
            IPUpdatePolyPlane2(PObj -> U.Pl, Pt);
        }
        else
            IPUpdatePolyPlane(PObj -> U.Pl);
    }
    IPO2IAddToGeom(PObj);
    _IPO2IFreeList(&List,TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Put the given points Pts in the curve or surface points database.        *
*   Pts may contain indexes to the points in PtsList or (If PtsList is NULL) *
*   the points themselves as list of pairs, triples or quadruplets.          *
*   When PtsList ins't NULL Pts may be a list of IP_O2I_I or a list of       *
*   IP_O2I_IVEC_3/4 (In the later case the indexes will be taken from the    *
*   first element in each tripple).                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvSrfPts : The database of the curve or surface.                        *
*   Pts       : The points to put in the curve or surface.                   *
*   PtsList   : The list containing the points pointed by Pts. If it's NULL  *
*               then Pts contains the points themselves and not indexes.     *
*   Dim       : The dimension of the points (How many axis. 2 or 3).         *
*   Rat       : Whether this is rational curve or not (Has weight or not).   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IAddPtsToCrvSrf(CagdRType **CrvSrfPts, 
                                IPO2IListStruct *Pts, 
                                IPO2IListStruct *PtsList,
                                int Dim,
                                int Rat) 
{
    int i;

    for (i = 0; i <= Pts -> Len - 1; i++) {
        IPO2IReal4Type *Pt;

        if (PtsList == NULL)
            Pt = &Pts -> U.RPts4[i];
        else {
            int *index = Pts -> Type == IP_O2I_I ? &Pts -> U.i[i]
					         : &Pts -> U.IVec[i][0];

            if (PtsList -> Type == IP_O2I_RPT_4)
                Pt = &PtsList -> 
                    U.RPts4[IPO2ICheckReference(PtsList, index ) - 1];
            else
                Pt = (IPO2IReal4Type*)&PtsList -> 
                    U.RPts3[IPO2ICheckReference(PtsList, index ) - 1];
        }
        CrvSrfPts[1][i] = (*Pt)[0];
        CrvSrfPts[2][i] = (*Pt)[1];
        if (Dim == 3)
            CrvSrfPts[3][i] = (*Pt)[2];
        if (Rat)
            CrvSrfPts[0][i] = (*Pt)[Dim];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a curv or curv2 statement.                                          *
*   If it's curv2 statement and the stage isn't IP_O2I_CURV2_STAGE just      *
*   increase the curv2 index and return.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseCurvCurv2(void)
{
    IrtRType u[2];
    IPO2IListStruct v, Parm;
    IPO2IResultType
        IPO2IResultList = IP_O2I_RESULT_LIST;
   CagdCrvStruct *Crv;
    char
        Statement[6] = "curv";
    IPAttributeStruct
        *Attr = NULL;
    int Dim = 3,
	Curv = (stricmp(IPO2IData -> LastToken, "curv") == 0);

    if (IPO2IData -> DegU == -1)
        IPO2IErrorAndExit("\"%s\" statement is given before a \"deg u\" "
                          "statement ", Statement);
    if (!Curv) {
        IPO2IData -> Curv2List.Curr++;
        if (IPO2IData -> Stage != IP_O2I_CURV2_STAGE)
            return;
        strcat(Statement, "2");
        Dim = 2;
    }

    /* Read u0 and u1. */
    if (Curv)
       IPO2IReadTokens(2, 2, u, NULL, IPO2IInterpReal);

    /* Read control points. */
    _IPO2IInitList(&v, IP_O2I_I);
    IPO2IReadTokens(0, IP_O2I_READ_ALL_VALUES, &v, &IPO2IResultList, 
		    IPO2IInterpInt);

    /* Read body statements. */
    _IPO2IInitList(&Parm, IP_O2I_R);
    while (TRUE) {
        int Res;
        char Token[IRIT_LINE_LEN + 1];

        IPO2IData -> GoNextLine = TRUE;
        Res = IPO2INextToken(Token);
        if (Res == IP_O2I_NO_MORE_LINES)    /* Finished reading the input. */
            IPO2IErrorAndExit("Expecting \"end\" statement");

        if (stricmp(Token, "sp") == 0) {
            char *Temp;
            const char *Temp2;
            IPO2IListStruct Sp;
            int i;

            _IPO2IInitList(&Sp, IP_O2I_I);
            IPO2IReadTokens(1, IP_O2I_READ_ALL_VALUES, &Sp, &IPO2IResultList, 
			    IPO2IInterpInt);

            /* Taking 30 chars for a number which is much more than enough.*/
            Temp = IritMalloc(30 * Sp.Len - 1);
            Temp[0] = '\0';
            for (i = 0; i <= Sp.Len - 1; i++) {
                IPO2ICheckReference(&IPO2IData -> VpVec, &Sp.U.i[i]);
                sprintf(Temp + strlen(Temp), "%f, ",
			IPO2IData -> VpVec.U.RPts3[Sp.U.i[i] - 1][0]);
            }
            Temp[strlen(Temp) - 2] = '\0';
            if ((Temp2 = AttrGetStrAttrib(Attr, IP_O2I_ATTRIB_SP)) != NULL) {
                char
		    *Temp3 = IritMalloc((unsigned int)
				         (strlen(Temp) + strlen(Temp2) + 1));

                sprintf(Temp3, "%s, %s", Temp2, Temp);
                IritFree(Temp);
                Temp = Temp3;
            }
            AttrSetStrAttrib(&Attr, IP_O2I_ATTRIB_SP, Temp);
            IritFree(Temp);
        }
        else if (stricmp(Token, "parm") == 0) {
            Res = IPO2INextToken(Token);
            if ((Res == IP_O2I_NO_MORE_LINES) || (Res == IP_O2I_LINE_END))
                /* Finished reading the input or the line. */
                IPO2IErrorAndExit("Expecting more parameters");

            /* I'm assuming the file format is correct and therefore Token   */
            /* can only be "u". No need to check it.                         */

            /* A new statement override the previous one. No adding to it.   */
            _IPO2IClearList(&Parm, TRUE);
            IPO2IReadTokens(1, IP_O2I_READ_ALL_VALUES,
			    &Parm, &IPO2IResultList, IPO2IInterpReal);
        }
        else if (stricmp(Token, "end") == 0)
            break;
        else
            IPO2IErrorAndExit("The statement \"%s\" isn't known (or not "
                              "expected here)", Token);
    }
    if (Parm.Len == 0)
        IPO2IErrorAndExit("Expecting a \"parm\" statement");

    /* Create the curve. */
    switch (IPO2IData -> CrvSrfType) {
        case IP_O2I_NONE:
            IPO2IErrorAndExit("\"%s\" statement appears before a \"cstype\" "
                              "statement", Statement);
        default:
	    Crv = NULL;
	    break;
        case IP_O2I_BEZIER:
            /* We don't use the parameter value, but we leave here the       */
            /* condition.                                                    */
            if (((v.Len - 1) % IPO2IData -> DegU != 0) || 
                ((v.Len - 1) / IPO2IData -> DegU + 1 != Parm.Len))
                IPO2IErrorAndExit("Expecting number of global parameters (%d) - 1"
                                  "to equal number of control points (%d) - 1 "
                                  "divided by the degree (%d) plus 1", Parm.Len, 
                                  v.Len, IPO2IData -> DegU);
            Crv = BzrCrvNew(v.Len, CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, Dim));
            break;
        case IP_O2I_BSPLINE:            
            if (v.Len != Parm.Len - IPO2IData -> DegU - 1)
                IPO2IErrorAndExit("Expecting number of control points (%d) to "
                                  "equal number of knot vectors (%d) minus the"
                                  " degree (%d) minus 1", v.Len, Parm.Len, 
                                  IPO2IData -> DegU);
            Crv = BspCrvNew(v.Len, IPO2IData -> DegU + 1,/* Order = deg + 1. */
                CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, Dim));
            IRIT_GEN_COPY(Crv -> KnotVector, Parm.U.r, 
                sizeof(IrtRType) * Parm.Len);
            break;
        case IP_O2I_TAYLOR:
            /* We don't use the parameter value, but we leave here the       */
            /* condition.                                                    */
            if (((v.Len) % IPO2IData -> DegU != 0) ||
                ((v.Len) / (IPO2IData -> DegU + 1) + 1 != Parm.Len))
                IPO2IErrorAndExit("Expecting number of global parameters (%d) - 1"
                                  "to equal number of control points (%d) "
                                  "divided by the degree (%d) plus 1, plus 1", 
                                  Parm.Len, v.Len, IPO2IData -> DegU);
            Crv = PwrCrvNew(v.Len, CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, Dim));
            break;
        case IP_O2I_BMATRIX:
        case IP_O2I_CARDINAL:
            IPO2IErrorAndExit("\"bmatrix\", \"cardinal\" and are not "
                              "supported");
            assert("\"bmatrix\" and \"cardinal\" are not supported" == 0);
            return;
    }

    /* Connect previous attributes to the new crv. */
    Crv -> Attr = Attr;

    /* Copy the control points to the curve. */
    if (Curv) {
        /* Add param domain values. */
        AttrSetRealAttrib(&Crv -> Attr, "BspDomainMin", u[0]);
	AttrSetRealAttrib(&Crv -> Attr, "BspDomainMax", u[1]);
        /* Add control points to curve. */
        IPO2IAddPtsToCrvSrf(Crv -> Points, &v, &IPO2IData -> VVec, Dim,
            IPO2IData -> Rat);
        /* Add curve to the geometry. */
        IPO2IAddToGeom(IPGenCRVObject(Crv));
    }
    else {
        /* Add control points to curve. */
        IPO2IAddPtsToCrvSrf(Crv -> Points, &v, &IPO2IData -> VpVec, Dim, 
            IPO2IData -> Rat);
        /* Add curve to the list of curv2. */
        _IPO2IAddElement(&IPO2IData -> Curv2List, Crv);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read the body statement of a surf statement.                             *
*   All the parameters are for results purposes .                            *
*                                                                            *
* PARAMETERS:                                                                *
*   ParmU, ParmV:  The values of the last parm statements for u and v (List  *
*                  of type IP_O2I_R).                                        *
*   TrimCrvList:   Structure to hold the trimming curves.                    *
*   Attrs:         Attributes which held special points/curves that are      *
*                  added to the surface.                                     *
*   HasTopLvlTrim: Whether one of the trimming curves is a top level         *
*                  trimming curve or a default one should be used.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseSurfBody(IPO2IListStruct *ParmU, 
                               IPO2IListStruct *ParmV, 
                               TrimCrvStruct **TrimCrvList,
                               IPAttributeStruct **Attrs,
                               int *HasTopLvlTrim)
{    
    *HasTopLvlTrim = -1;
    while (TRUE) {
        IPO2IResultType
            IPO2IResultList = IP_O2I_RESULT_LIST;
        int Res;
        char Token[IRIT_LINE_LEN+1];

        IPO2IData -> GoNextLine = TRUE;
        if (IPO2INextToken(Token) == IP_O2I_NO_MORE_LINES)
            /* Finished reading the input. */
            IPO2IErrorAndExit("Expecting \"end\" statement");
        if (stricmp(Token, "sp") == 0) {
            char *Temp;
            const char *Temp2;
            IPO2IListStruct Sp;
            int i;

            _IPO2IInitList(&Sp, IP_O2I_I);
            IPO2IReadTokens(1, IP_O2I_READ_ALL_VALUES, &Sp, &IPO2IResultList, 
                IPO2IInterpInt);
            /* Taking 60 chars for a number which is much more than enough.*/
            Temp = IritMalloc(60 * Sp.Len - 1);
            Temp[0] = '\0';
            for (i = 0; i <= Sp.Len - 1; i++) {
                IPO2ICheckReference(&IPO2IData -> VpVec, &Sp.U.i[i]);
                sprintf(Temp + strlen(Temp), "(%f, %f), ",
                    IPO2IData -> VpVec.U.RPts3[Sp.U.i[i] - 1][0],
                    IPO2IData -> VpVec.U.RPts3[Sp.U.i[i] - 1][1]);
            }
            Temp[strlen(Temp) - 2] = '\0';
            if ((Temp2 = AttrGetStrAttrib(*Attrs, IP_O2I_ATTRIB_SP)) != NULL) {
                char
		    *Temp3 = IritMalloc((unsigned int)
					   (strlen(Temp) + strlen(Temp2) + 3));

                sprintf(Temp3, "%s, %s", Temp2, Temp);
                IritFree(Temp);
                Temp = Temp3;
            }
            AttrSetStrAttrib(Attrs, IP_O2I_ATTRIB_SP, Temp);
            IritFree(Temp);
        }
        else if (stricmp(Token, "parm") == 0) {
            IPO2IListStruct *Parm;

            Res = IPO2INextToken(Token);
            if ((Res == IP_O2I_NO_MORE_LINES) || (Res == IP_O2I_LINE_END))
                /* Finished reading the input or the line. */
                IPO2IErrorAndExit("Expecting more parameters");
            if (stricmp(Token, "u") == 0)
                Parm = ParmU;
            else if (stricmp(Token, "v") == 0)
                Parm = ParmV;
            else {
                IPO2IErrorAndExit("Expecting the token \"%s\" to be \"u\" or "
                                  "\"v\"", Token);
		Parm = NULL;
	    }

            /* A new statement override the previous one. No adding to it. */
            _IPO2IClearList(Parm, TRUE);
            IPO2IReadTokens(1, IP_O2I_READ_ALL_VALUES, Parm, &IPO2IResultList, 
			    IPO2IInterpReal);
        }
        else if ((stricmp(Token, "trim") == 0) || 
            (stricmp(Token, "hole") == 0)) {
            TrimCrvSegStruct
	        *TrimCrvSegList = NULL;

            if (*HasTopLvlTrim == -1) {
                if (stricmp(Token, "trim") == 0) 
                    *HasTopLvlTrim = TRUE;
                else
                    *HasTopLvlTrim = FALSE;
            }
            if (IPO2IData -> Curv2List.Len == 0)
                IPO2IErrorAndExit("The \"%s\" statement requires \"curv2\" statement in the file",
                                   Token);
            do {                
                IrtRType
		    u[2] = { 0, 1 };
                int CrvIndex = -1;
                TrimCrvSegStruct *CrvSeg;
                CagdCrvStruct *Crv;

                IPO2IReadTokens(2, 2, u, NULL, IPO2IInterpReal);
                IPO2IReadTokens(1, 1, &CrvIndex, NULL, IPO2IInterpInt);
                IPO2ICheckReference(&IPO2IData -> Curv2List, &CrvIndex);
                Crv = CagdCrvCopy(IPO2IData -> Curv2List.U.Curv2s[CrvIndex - 1]);
                AttrSetRealAttrib(&Crv -> Attr, "BspDomainMin", u[0]);
                AttrSetRealAttrib(&Crv -> Attr, "BspDomainMax", u[1]);
                CrvSeg = TrimCrvSegNew(Crv, NULL);
                CrvSeg -> Pnext = TrimCrvSegList;
                TrimCrvSegList = CrvSeg;
                if (Crv != CrvSeg -> UVCrv)
                    CagdCrvFree(Crv);
            }
            while (!IPO2IData -> EndOfLine);
            if (TrimCrvSegList != NULL) {
                TrimCrvStruct
		    *TrimCrv = TrimCrvNew(TrimCrvSegList);

                TrimCrv -> Pnext = *TrimCrvList;
                *TrimCrvList = TrimCrv;
            }
        }
        else if (stricmp(Token, "scrv") == 0) {
            IPObjectStruct 
                *CrvList;

            if ((CrvList = AttrGetObjAttrib(*Attrs, IP_O2I_ATTRIB_SCRV)) 
                == NULL) 
                CrvList = IPGenLISTObject(NULL);                
            if (IPO2IData -> Curv2List.Len == 0)
                IPO2IErrorAndExit("The \"scrv\" statement requires \"curv2\" statement in the file");
            do {                
                IrtRType
		    u[2] = { 0, 1 };
                int CrvIndex = -1;
                CagdCrvStruct *Crv;

                IPO2IReadTokens(2, 2, u, NULL, IPO2IInterpReal);
                IPO2IReadTokens(1, 1, &CrvIndex, NULL, IPO2IInterpInt);
                IPO2ICheckReference(&IPO2IData -> Curv2List, &CrvIndex);
                Crv = CagdCrvCopy(IPO2IData -> Curv2List.U.Curv2s[CrvIndex - 1]);
                AttrSetRealAttrib(&Crv -> Attr, "BspDomainMin", u[0]);
                AttrSetRealAttrib(&Crv -> Attr, "BspDomainMax", u[1]);
                IPListObjectAppend(CrvList, IPGenCRVObject(Crv));
            }
            while (!IPO2IData -> EndOfLine);
            if (AttrGetObjAttrib(*Attrs, IP_O2I_ATTRIB_SCRV) == NULL)
                AttrSetObjAttrib(Attrs, IP_O2I_ATTRIB_SCRV, CrvList, FALSE);
        }
        else if (stricmp(Token, "end") == 0)
            break;
        else
            IPO2IErrorAndExit("The statement \"%s\" isn't known (or not "
                              "expected here)", Token);
    }

    if ((ParmV -> Len == 0) || (ParmU -> Len == 0))
        IPO2IErrorAndExit("Expecting both a \"parm u\" and a \"parm v\" "
                          "statements");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a surf statement.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseSurf(void)
{
    IrtRType St[4];
    IPO2IListStruct v, ParmU, ParmV;
    CagdSrfStruct *Srf;
    TrimCrvStruct *TrimCrvList = NULL;
    int HasTopLvlTrim,
	Pattern[] = {-1, TRUE, TRUE, TRUE, TRUE};
    IPAttributeStruct
        *Attr = NULL;

    if (IPO2IData -> DegV == -1)
        IPO2IErrorAndExit("a surf statement is given before a \"deg u v\" "
                          "statement ");
    /* Read s0, s1, t0 and t1. */
    IPO2IReadTokens(4, 4, St, NULL, IPO2IInterpReal);

    /* Read control points. */
    _IPO2IInitList(&v, IP_O2I_IVEC_3);

    /* We don't check the outcome pattern because we use only the first  */
    /* vector. The second and third vectors are texture and normals and  */
    /* they aren't used because surfaces have natural texture            */
    /* parameterization and normals.                                     */
    IPO2IReadTokens(0, IP_O2I_READ_ALL_VALUES, &v,
		    Pattern, IPO2IInterpInts); 

    /* Read body statements. */
    _IPO2IInitList(&ParmU, IP_O2I_R);
    _IPO2IInitList(&ParmV, IP_O2I_R);
    /* Read the surface body statements. */
    IPO2IParseSurfBody(&ParmU, &ParmV, &TrimCrvList, &Attr, 
		       &HasTopLvlTrim);

    /* Create the surface. */
    switch (IPO2IData -> CrvSrfType) {
        case IP_O2I_NONE:
            IPO2IErrorAndExit("\"surf\" statement appears before a \"cstype\" "
                              "statement");
        default:
	    Srf = NULL;
	    break;
        case IP_O2I_BEZIER:
	{
            int LenU, LenV;

            LenU = (ParmU.Len - 1) * IPO2IData -> DegU + 1;
            LenV = (ParmV.Len - 1) * IPO2IData -> DegV + 1;
            /* We don't use the parameter value, but we leave here the       */
            /* condition.                                                    */
            if (v.Len != LenU * LenV) 
                IPO2IErrorAndExit("Expecting number of control points (%d) to "
                                  "be %d", v.Len, LenU * LenV);
            Srf = BzrSrfNew(LenU, LenV, CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, 3));
            break;
        }
        case IP_O2I_BSPLINE:
	{
            int LenU, LenV;

            LenU = ParmU.Len - IPO2IData -> DegU - 1;
            LenV = ParmV.Len - IPO2IData -> DegV - 1;
            if (v.Len != LenU * LenV)
                IPO2IErrorAndExit("Expecting number of control points (%d) to "
                                  "be %d", v.Len, LenU * LenV);

            /* Order = deg + 1. */
            Srf = BspSrfNew(LenU, LenV, IPO2IData -> DegU + 1, 
			    IPO2IData -> DegV + 1,
			    CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, 3));
            IRIT_GEN_COPY(Srf -> UKnotVector, ParmU.U.r, 
			  sizeof(IrtRType) * ParmU.Len);
            IRIT_GEN_COPY(Srf -> VKnotVector, ParmV.U.r, 
			  sizeof(IrtRType) * ParmV.Len);
            break;
        }
        case IP_O2I_TAYLOR:
	{
            int LenU, LenV;

            LenU = (ParmU.Len - 1) * (IPO2IData -> DegU + 1);
            LenV = (ParmV.Len - 1) * (IPO2IData -> DegV + 1);

            /* We don't use the parameter value, but we leave here the       */
            /* condition.                                                    */
            if (v.Len != LenU * LenV) 
                IPO2IErrorAndExit("Expecting number of control points (%d) to "
                                  "be %d", v.Len, LenU * LenV);
            Srf = PwrSrfNew(LenU, LenV, CAGD_MAKE_PT_TYPE(IPO2IData -> Rat, 3));
            break;
        }
        case IP_O2I_BMATRIX:
        case IP_O2I_CARDINAL:
            IPO2IWarning("\"bmatrix\" and \"cardinal\" are not supported");
            assert("\"bmatrix\" and \"cardinal\" are not supported" == 0);
            return;
    }
    Srf -> Attr = Attr;

    /* Add param domain to the surface. */
    AttrSetUVAttrib(&Srf -> Attr, "BspDomainMin", St[0], St[2]);
    AttrSetUVAttrib(&Srf -> Attr, "BspDomainMax", St[1], St[3]);

    /* Copy the control points to the curve. */
    IPO2IAddPtsToCrvSrf(Srf -> Points, &v, &IPO2IData -> VVec, 3, 
        IPO2IData -> Rat);

    /* Add the surface or trimmed surface to the geometry. */
    if (TrimCrvList == NULL) 
        IPO2IAddToGeom(IPGenSRFObject(Srf));
    else 
        IPO2IAddToGeom(IPGenTRIMSRFObject(
			       TrimSrfNew(Srf, TrimCrvList, HasTopLvlTrim)));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a bsp or bzp statement.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseBspBzp(void)
{
    IrtRType
	St[2] = {0, 1};
    int Vec[16], i, j,
        Bsp = (stricmp(IPO2IData -> LastToken, "bsp") == 0);
    IPO2IListStruct v;
    CagdSrfStruct *Srf;
    char AttrValue[IRIT_LINE_LEN + 1];

    IPO2IReadTokens(16, 16, Vec, NULL, IPO2IInterpInt); 
    _IPO2IInitList(&v, IP_O2I_IVEC_3);
    /* Arranging the vectors according to version 3.0 order. */
    for (i = 4; i >= 1; i--)
        for (j = (i - 1) * 4; j <= i * 4 - 1; j++)
            _IPO2IAddElement(&v, &Vec[j]);

    /* Create the surface. */
    if (Bsp) {
        /* Order = deg + 1. */
        Srf = BspSrfNew(4, 4, 4, 4, CAGD_MAKE_PT_TYPE(FALSE, 3));
	BspKnotUniformOpen(8, 4, Srf -> UKnotVector);
	BspKnotUniformOpen(8, 4, Srf -> VKnotVector);
    }
    else 
        Srf = BzrSrfNew(4, 4, CAGD_MAKE_PT_TYPE(FALSE, 3));

    /* Add limits of the domain to the surface. */
    AttrSetUVAttrib(&Srf -> Attr, "BspDomainMin", St[0], St[0]);
    AttrSetUVAttrib(&Srf -> Attr, "BspDomainMax", St[1], St[1]);

    /* Add the res statement's values as attribute. */
    if ((IPO2IData -> Res[0] != -1) && (IPO2IData -> Res[1] != -1)) {
        sprintf(AttrValue, "%d, %d", IPO2IData -> Res[0], IPO2IData -> Res[1]);
        AttrSetStrAttrib(&Srf -> Attr, IP_O2I_ATTRIB_RES, AttrValue);
    }

    /* Copy the control points to the curve. */
    IPO2IAddPtsToCrvSrf(Srf -> Points, &v, &IPO2IData -> VVec, 3, FALSE);
    IPO2IAddToGeom(IPGenSRFObject(Srf));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print warning that statement isn't supported and return.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IStatementNotSupported() 
{
    IPO2IWarning("The \"%s\" statement isn't supported", 
		 IPO2IData -> LastToken);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print warning that option of that satement isn't supported and return.   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2MtlOptNotSupported(char *Statement, char *Option) 
{
    IPO2IWarning("The \"%s\" option in the statement \"%s\" isn't supported",
		 Option, Statement);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a usemtl statement.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseUsemtl(void)
{
    char Token[IRIT_LINE_LEN + 1];
    int i;

    if (IPO2INextToken(Token) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    for (i = 0; i <= IPO2IData -> Mtls.Len - 1; i++)
        if (stricmp(Token, IPO2IData -> Mtls.U.Mtl[i].Name) == 0) {
            IPO2IData -> Mtls.Curr = i;
            return;
        }
    IPO2IWarning("Unknown material \"%s\". Ignore it", Token);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split FileName to the path of the directory and the name of the file.    *
*   FullPath is changed to the file name (Notice that it mustn't be const    *
*   array). The returned value is the directory path.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   FullPath: The name of the file to split its path.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *: The path of FullPath's directory.                                *
*****************************************************************************/
static char *IPO2ISplitPath(char *FullPath)
{
    char *C1, *C3,
        *Ret = IritStrdup(FullPath);

#ifdef __WINNT__
    char *C2;

    C1 = strrchr(Ret, '/');
    C2 = strrchr(Ret, '\\');
    if (C1 > C2)
        C3 = C1 + 1;
    else if (C2 > C1) 
        C3 = C2 + 1;
    else 
        C3 = Ret;
#else
    C1 = strrchr(Ret, '/');
    if (C1 != NULL) 
        C3 = C1 + 1;
    else 
        C3 = Ret;
#endif
    strcpy(FullPath, C3);
    *(C3) = '\0';
    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add Path2 to Path1. If Path2 is absolute the result will be only Path2.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Path1: The path to add Path2 to.                                         *
*   Path2: The path to add to Path1.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *: The combined path or NULL if FileName is NULL.                   *
*****************************************************************************/
static char *IPO2ICombinePaths(const char *Path1, const char *Path2)
{
    char *Ret;
    int Len = (int) strlen(Path1);

#ifdef __WINNT__
    if ((Path2[0] == '\\') ||
	(Path2[1] == ':') ||
	((Len = (int) (strlen(Path1))) == 0))
        return IritStrdup(Path2);

    Ret = IritMalloc((int) (strlen(Path1) + strlen(Path2) + 2));
    strcpy(Ret, Path1);
    if ((Path1[Len - 1] != '\\') && (Path1[Len - 1] != '/'))
        strcat(Ret, "\\");
#else
    if ((Path1[0] == '/') || (Path1[1] == ':') || ((Len = strlen(Path1)) == 0))
        return IritStrdup(Path2);
    Ret = IritMalloc(strlen(Path1) + strlen(Path2) + 2);
    strcpy(Ret, Path1);
    if (Path1[Len - 1] != '/')
        strcat(Ret, "/");
#endif
    strcat(Ret, Path2);
    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Return the current path. The path were the program was activated from.   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *: The combined path or NULL if FileName is NULL.                   *
*****************************************************************************/
static char *IPO2IGetCurPath(void)
{
    char Buffer[IRIT_LINE_LEN_MAX_CFG + 1], *Ret;

#ifdef __WINNT__
    Ret = _getcwd(Buffer, IRIT_LINE_LEN_MAX_CFG);
#else
    Ret = getcwd(Buffer, IRIT_LINE_LEN_MAX_CFG);
#endif
    if (Ret == NULL)                /* In case of error using relative path. */
        Buffer[0] = '\0';
    return IritStrdup(Ret);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reading the name of the file from an mtl statement and then initialize   *
*   data for reading mtl file.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:     IPO2ILoadFileDataStruct to contain information about the file. *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE Initialized new mtl file. FALSE no mtl files were found in the *
*        mtllib statement.                                                   *
*****************************************************************************/
static int IPO2IInitMtlFile(IPO2ILoadFileDataStruct *Data) 
{
    char Token[IRIT_LINE_LEN+1], *FilePath;
    int Res;

    Res = IPO2INextToken(Token);
    if ((Res == IP_O2I_NO_MORE_LINES) || (Res == IP_O2I_LINE_END))
        return FALSE;

    /* Initialize data. */
    Data -> LineNum = 0;
    Data -> CurrLine = NULL;
    Data -> NextToken = NULL;
    Data -> ConNextLine = FALSE;
    Data -> Stage = IP_O2I_MTL_STAGE;
    Data -> EndOfLine = FALSE;
    Data -> GoNextLine = TRUE;
    Data -> Mtl.Attr = NULL;
    Data -> Mtl.Name = NULL;
    Data -> FileName = IritStrdup(Token);
    Data -> DirPath = IPO2ICombinePaths(IPO2IData -> DirPath,         
        IPO2ISplitPath(Data -> FileName));
    IPO2IData = Data; 
    /* Open the file. */
    /* Must be after the initialization because when the longjmp is catched  */
    /* it expects IPO2IData to belong to the mtl file and to be initialized. */
    FilePath = IPO2ICombinePaths(IPO2IData -> DirPath, IPO2IData -> FileName);
    if ((Data -> File = fopen(FilePath, "r")) == NULL) {
        if (IPO2IWarningMsgs)
            IRIT_WARNING_MSG_PRINTF("Cannot open MTL file \"%s\", "
                                    "ignoring it.\n", FilePath);
        IritFree(FilePath);
        longjmp(IPO2ILongJumpBuffer, 1);
    }
    else
        IRIT_WARNING_MSG_PRINTF("Start parsing mtl file \"%s\".\n", FilePath);
    IritFree(FilePath);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Release resources after reading an mtl file.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Data: The IPO2ILoadFileDataStruct of the obj file which ordered the      *
*         parsing of this mtl file.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IFinishMtlFile(IPO2ILoadFileDataStruct *Data) 
{
    IritFree(IPO2IData -> FileName);
    IritFree(IPO2IData -> CurrLine);
    IPO2IData = Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add the matrerial Mtl into the list of materials Mtls. If Mtl is emtpy   *
*   material do nothing.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mtls : List of materials.                                                *
*   Mtl :  The material to add.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IAddMaterial(IPO2IListStruct *Mtls, IPO2IMtlStruct *Mtl) 
{
    int Rgb[3];

    /* In case when material has texture and a Kd 0,0,0 */
    if (AttrGetRGBColor(Mtl -> Attr, &Rgb[0], &Rgb[1], &Rgb[2]) &&
        (AttrGetStrAttrib(Mtl -> Attr, "ptexture") != NULL) &&
        (Rgb[0] == 0) && (Rgb[1] == 0) && (Rgb[2] == 0)) {
        if (IPO2IWhiteDiffuseTexture)
            AttrSetRGBColor(&Mtl -> Attr, 255, 255, 255); 
        else
            IPO2IWarning("Recieved a material with a texture and a Kd value (0,0,0). That is probably a mistake");
    }
    if (Mtl -> Attr != NULL) 
    _IPO2IAddElement(Mtls, Mtl);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtllib statement.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseMtllib(void)
{
    IPO2ILoadFileDataStruct Data, 
        *OrigData = IPO2IData;
    int Res = FALSE; 

    if (IPO2IData -> Stage != IP_O2I_VECTORS_STAGE)
        return;
    if (IPO2IData -> EndOfLine)
        IPO2IErrorAndExit("Expecting more parameters. Ignoring this \"mtllib\" "
                          "statement");
    do {
        jmp_buf LongJumpBuffer;
        char
	    *SaveErrMsg = IPO2IOtherErrorMsg,
	    *SaveWarningMsg = IPO2IOtherWarningMsg;

        memcpy(LongJumpBuffer, IPO2ILongJumpBuffer, sizeof(jmp_buf));
        IPO2IOtherErrorMsg = "Error in mtl file \"%s\" line %d. %s. Stop parsing "
                             "the file.\n";
        IPO2IOtherWarningMsg = "Mtl file \"%s\" line %d. %s.\n";
        if (setjmp(IPO2ILongJumpBuffer) == 0) {
            /* Start reading new mtl file. */
            Res = IPO2IInitMtlFile(&Data);
            while (Res) {
	        char Token[IRIT_LINE_LEN + 1], Aux[IRIT_LINE_LEN + 3];
                int i, 
                    Len = sizeof(IPO2IParseMtlStatements) / 
                          sizeof (IPO2IParseStatementsStruct);

                IPO2IData -> GoNextLine = TRUE;
                if (IPO2INextToken(Token) == IP_O2I_NO_MORE_LINES)
                {
                    IPO2IAddMaterial(&OrigData -> Mtls, &IPO2IData -> Mtl);
                    break;
                }
                sprintf(Aux, " %s ", Token);
                IritStrUpper(Aux);
                for (i = 0; i <= Len - 1; i++) {
                    if (stricmp(IPO2IParseMtlStatements[i].Statement, Token) 
                        == 0) {
                        if (i == 0)
                            IPO2IAddMaterial(&OrigData -> Mtls, &IPO2IData -> Mtl);
                        IPO2IParseMtlStatements[i].Func();
                        break;
                    }
                    /* First function is newmtl, therefore when ever getting */
                    /* here, either has material name or started material    */
                    /* statements with statement other than newmtl.          */
                    if (IPO2IData -> Mtl.Name == NULL)
                        IPO2IErrorAndExit("First material had no name");
                }
                if (i == Len)
                    IPO2IErrorAndExit("Can't recognise the statement \"%s\"", 
                                      Token);
            }
        }
        IPO2IFinishMtlFile(OrigData);
        memcpy(IPO2ILongJumpBuffer, LongJumpBuffer, sizeof(jmp_buf));
        IPO2IOtherErrorMsg = SaveErrMsg;
        IPO2IOtherWarningMsg = SaveWarningMsg;
        /* Finished reading all mtl files. */
        if (IPO2IData -> EndOfLine)
            break;
    }
    while(Res);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a Res statement.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseRes(void)
{
    IPO2IReadTokens(2, 2, IPO2IData -> Res, NULL, IPO2IInterpInt);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Transform the given string to an acceptable Irit name.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Str: String to transform.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   char: The transformed string.                                            *
*****************************************************************************/
static char *IPO2IStrToIrit(char *Str) 
{
    int i;

    if (Str == NULL)
        return NULL;
    for (i = 0; i <= ((int)strlen(Str)) - 1; i++) 
	if (!(isalpha(Str[i]) || isdigit(Str[i]) || (Str[i] == '_')))
            Str[i] = '_';
    if (isdigit(Str[0]))
        Str[0] = '_';
    return Str;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a g or o statement (Treated the same).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseGO(void)
{
    char Token[IRIT_LINE_LEN + 1];
    IPObjectStruct *Group;
    int i;

    if (IPO2INextToken(Token) == IP_O2I_LINE_END)
        return; /* Specifications require name but examples seems to use it  */
                /* without a name.                                           */
    IPO2IStrToIrit(Token);
    for (i = 0; i <= IPO2IData -> GroupsList.Len - 1; i++)
        if (stricmp(Token, 
                    IP_GET_OBJ_NAME(IPO2IData -> GroupsList.U.Groups[i])) 
                    == 0) {
            IPO2IData -> GroupsList.Curr = i;
            return ;
        }
    Group = IPGenListObject(Token,NULL,NULL);
    _IPO2IAddElement(&IPO2IData -> GroupsList, Group);
    IPO2IData -> GroupsList.Curr = IPO2IData -> GroupsList.Len - 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a s statement.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseS(void)
{
    char Token[IRIT_LINE_LEN + 1];
    IPObjectStruct *Group;
    int i, 
        Num = -1;

    if (IPO2INextToken(Token) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    sscanf(Token, "%i", &Num);
    if ((Num == 0) || (stricmp(Token, "off") == 0)) {
        IPO2IData -> SGroups.Curr = -1;
        return;
    }
    if (Num == -1)
        IPO2IErrorAndExit("Expecting integer value");
    sprintf(Token, "%d", Num);
    for (i = 0; i <= IPO2IData -> SGroups.Len - 1; i++)
        if (stricmp(Token, 
                    IP_GET_OBJ_NAME(IPO2IData -> SGroups.U.Groups[i])) 
                    == 0) {
            IPO2IData -> SGroups.Curr = i;
            return ;
        }
    Group = IPGenPolyObject(Token, NULL, NULL);
    _IPO2IAddElement(&IPO2IData -> SGroups, Group);
    IPO2IData -> SGroups.Curr = IPO2IData -> SGroups.Len - 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read a call statement.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseCall(void)
{
    /* There isn't support for mod files or files with parameters. */
    int Res, Len;
    char Token[IRIT_LINE_LEN+1],
	*SaveMsg = IPO2IOtherErrorMsg;
    IPO2ILoadFileDataStruct
        *Data = IPO2IData;
    IPObjectStruct *PObj;

    Res = IPO2INextToken(Token);
    if (Res == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    if (Res == IP_O2I_READ_TOKEN)        
        IPO2IErrorAndExit("Calling files with parameters isn't supported");
    Len = (int) strlen(Token);
    if ((Len < 4) || (stricmp(Token + Len - 4, ".obj") != 0))
        IPO2IErrorAndExit("Supports only calls to \"obj\" files");

    /* Changing error message. In case of error ignoring the called file   */
    /* without exiting. Continue to parse the rest of the caliing file.    */
    IPO2IOtherErrorMsg = "Error in obj file \"%s\" line %d. %s. Ignoring the "
                         "file.\n";
    PObj = IPO2IOBJLoadFile(Token, IPO2IWarningMsgs, IPO2IWhiteDiffuseTexture, 
        IPO2IIgnoreFullTransp, IPO2IForceSmoothing);
    _IPO2IAddElement(&Data -> GroupsList, PObj);
    IPO2IOtherErrorMsg = SaveMsg;
    IPO2IData = Data;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read OBJ file into IRIT data using the given input function.             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int IPO2IParse(void)
{
    while (TRUE) {
        char Token[IRIT_LINE_LEN + 1], Aux[IRIT_LINE_LEN + 3];
        int i, Res;

        IPO2IData -> GoNextLine = TRUE;
        Res = IPO2INextToken(Token);
        if (Res == IP_O2I_NO_MORE_LINES) {
            if (IPO2IData -> Stage == IP_O2I_REST_STAGE) {
                /* Finished reading the input. */
                IPO2IFinalizeGeometry();
                return TRUE;
            }
            IPO2IResetDataForRereading();
            IPO2IData -> Stage++; 
            continue;
        }

        /* Checking that no inner body keywords appear here. */
        sprintf(Aux, " %s ", Token);
        IritStrUpper(Aux);
        if (/* Parsing only vectors in vectors stage. */
            ((IPO2IData -> Stage == IP_O2I_VECTORS_STAGE) &&
            (strstr(IP_O2I_VECTORS_TOKENS, Aux) == NULL)) ||
            /* Parsing only vectors in vectors and curv2 in this stage. */
            ((IPO2IData -> Stage == IP_O2I_CURV2_STAGE) &&
            (strstr(IP_O2I_CURV2_VECTORS_TOKENS, Aux) == NULL)) ||
            /* Inner body statements have no effect here. */
            strstr(IP_O2I_IN_BODY_TOKENS, Aux)) {
	}
        else {
            int Len = sizeof(IPO2IParseStatements) / 
                sizeof (IPO2IParseStatementsStruct);
            for (i = 0; i <= Len - 1; i++) {
                if (stricmp(IPO2IParseStatements[i].Statement, Token) == 0) {
                    IPO2IParseStatements[i].Func();
                    break;
                }
            }
            if (i == Len)
                IPO2IErrorAndExit("Can't recognise the statement \"%s\"", 
                                  Token);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reset the required fields in IPO2IData in order to start reading again   *
*   the file.                                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IResetDataForRereading() 
{
    IPO2IData -> LineNum = 0;
    IPO2IData -> Res[0] = -1;
    IPO2IData -> Res[1] = -1;
    IPO2IData -> Rat = -1;
    IPO2IData -> VVec.Curr= -1;
    IPO2IData -> VtVec.Curr = -1;
    IPO2IData -> VpVec.Curr = -1;
    IPO2IData -> VnVec.Curr = -1;
    IPO2IData -> Curv2List.Curr = -1;
    IPO2IData -> GroupsList.Curr = 0;
    if (IPO2IForceSmoothing)
        IPO2IData -> SGroups.Curr = 0;
    else
        IPO2IData -> SGroups.Curr = -1;
    IPO2IData -> DegU = -1;
    IPO2IData -> DegV = -1;
    IPO2IData -> StepU = -1;
    IPO2IData -> StepV = -1;
    IPO2IData -> CrvSrfType = IP_O2I_NONE;
    IPO2IData -> CurrLine = NULL;
    IPO2IData -> NextToken = NULL;
    IPO2IData -> ConNextLine = FALSE;
    IPO2IData -> EndOfLine = FALSE;
    IPO2IData -> GoNextLine = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   open OBJFileName.                                                        *
*   Initialise the variables required for loading a file in IPO2IData        *
*                                                                            *
* PARAMETERS:                                                                *
*   OBJFileName:   The file to open. NULL means standard input.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE if can't open OBJFileName (Variables are not initialized).    *
*****************************************************************************/
static int IPO2IInitLoadFile(const char *OBJFileName)
{
    /* Open the file. */
    IPO2IData -> FileName = _IPO2ITrimWhiteSpaces(OBJFileName);
    if (IPO2IData -> FileName == NULL) {
        IPO2IData -> File = stdin;
        IPO2IData -> FileName = IritStrdup("<standard input>");
        IPO2IData -> DirPath  = IritMalloc(sizeof(char));
        IPO2IData -> DirPath[0] = '\0';
    }
    else if ((IPO2IData -> File = fopen(IPO2IData -> FileName, "r")) == NULL) {
        IRIT_WARNING_MSG_PRINTF("Cannot open OBJ file \"%s\", exiting.\n", 
                                IPO2IData -> FileName);
        return FALSE;
    }
    else 
        IPO2IData -> DirPath = IPO2ICombinePaths(IPO2IGetCurPath(), 
				       IPO2ISplitPath(IPO2IData -> FileName));
    _IPO2IInitList(&(IPO2IData -> UMatrix), IP_O2I_R);
    _IPO2IInitList(&(IPO2IData -> VMatrix), IP_O2I_R);
    _IPO2IInitList(&(IPO2IData -> VVec), IP_O2I_RPT_4);
    _IPO2IInitList(&(IPO2IData -> VtVec), IP_O2I_RPT_3);
    _IPO2IInitList(&(IPO2IData -> VpVec), IP_O2I_RPT_3);
    _IPO2IInitList(&(IPO2IData -> VnVec), IP_O2I_RPT_3);
    _IPO2IInitList(&(IPO2IData -> Curv2List), IP_O2I_CURV2);
    _IPO2IInitList(&(IPO2IData -> GroupsList), IP_O2I_GROUPS);
    _IPO2IInitList(&(IPO2IData -> Lines), IP_O2I_STRING);
    _IPO2IInitList(&(IPO2IData -> Mtls), IP_O2I_MTL);
    _IPO2IInitList(&(IPO2IData -> SGroups), IP_O2I_GROUPS);
    if (IPO2IForceSmoothing) {
        IPObjectStruct
	    *Group = IPGenPolyObject("0", NULL, NULL);

        _IPO2IAddElement(&IPO2IData -> SGroups, Group);
        IPO2IData -> SGroups.Curr = 0;
    }
    IPO2IData -> Geom = IPGenListObject("default", NULL, NULL);
    _IPO2IAddElement(&IPO2IData -> GroupsList, IPO2IData -> Geom);
    IPO2IData -> Stage = IP_O2I_VECTORS_STAGE;
    IPO2IResetDataForRereading();
    IPO2IUniqueNum = 0;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Release resources after loading a file.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IFinishLoadFile(void)
{
    fclose(IPO2IData -> File);
    IritFree(IPO2IData -> FileName);
    IritFree(IPO2IData -> DirPath);
    _IPO2IFreeList(&(IPO2IData -> UMatrix), TRUE);
    _IPO2IFreeList(&(IPO2IData -> VMatrix), TRUE);
    _IPO2IFreeList(&(IPO2IData -> VVec), TRUE);
    _IPO2IFreeList(&(IPO2IData -> VtVec), TRUE);
    _IPO2IFreeList(&(IPO2IData -> VpVec), TRUE);
    _IPO2IFreeList(&(IPO2IData -> VnVec), TRUE);
    _IPO2IFreeList(&(IPO2IData -> Curv2List), TRUE);
    _IPO2IFreeList(&(IPO2IData -> GroupsList), FALSE);
    _IPO2IFreeList(&(IPO2IData -> Lines), TRUE);
    _IPO2IFreeList(&(IPO2IData -> Mtls), TRUE);
    _IPO2IFreeList(&(IPO2IData -> SGroups), FALSE);
    IPFreeObject(IPO2IData -> Geom);
    IritFree(IPO2IData -> CurrLine);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifies that Number references to an existing element in List. Call     *
*   IPO2IErrorAndExit if not. If Number is relative reference (negative      *
*   nubmer) changes it to an absolute reference first.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   List:   A IPO2IListStruct which Number points to its element.            *
*   Number: An index to element in List(Starting from 1, Minus to relative). *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: The final value of *Number.                                         *
*****************************************************************************/
static int IPO2ICheckReference(IPO2IListStruct *List, int *Number)
{
    if (*Number > List -> Len)
        IPO2IErrorAndExit("The reference \"%d\" points to non existing element "
                          "(There are only %d elements)", *Number, List -> Len);
    if (*Number == 0) 
        IPO2IErrorAndExit("A reference can't be 0");
    if (*Number < 0) {
        int Temp = List -> Curr + 2 + *Number;

        if (Temp <= 0) 
            IPO2IErrorAndExit("The reference \"%d\" points to non-positive"
                              " index (%d).\n", *Number, Temp);
        *Number = Temp;
    }
    return *Number;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reads a line from file or from the lines stored in IPO2IData.            *
*   If IPO2IData -> Stage is IP_O2I_VECTORS_STAGE reads from file and saves  *
*   lines in IPO2IData. If it's IP_O2I_MTL_STAGE, reads from file but        *
*   doesn't save. Otherwise read lines from IPO2IData. Ignores lines         *
*   starting with '#', empty lines and lines that contain only               *
*   IP_O2I_CONTINUE_NEXT_LINE. Remove IP_O2I_CONTINUE_NEXT_LINE from end of  *
*   lines and set IPO2IData -> ConNextLine to TRUE. The read line is stored  *
*   in  IPO2IData -> CurrLine.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE There aren't any more lines (Line wasn't read).               *
*        TRUE  A line was read. There might be more lines.                   *
*****************************************************************************/
static int IPO2IReadLine(void) 
{
    char
        *LongLine = NULL;

    while (TRUE) {
        if ((IPO2IData -> Stage == IP_O2I_VECTORS_STAGE) ||
            (IPO2IData -> Stage == IP_O2I_MTL_STAGE)) {
	    char Line[IP_O2I_LINE_LEN + 1], *FinalLine;

            if (fgets(Line, IP_O2I_LINE_LEN, IPO2IData -> File) == NULL) {
                if (feof(IPO2IData -> File)) 
                    /* End of file and no data was read. CurrLine keeps the  */
                    /* last line of the file and no increasing LineNum.      */
                    return FALSE;
                else {
                    /* Error reading from file. */
                    IPO2IErrorAndExit("Error reading the file");
                }
            }
            IPO2IData -> LineNum++;
            if (feof(IPO2IData -> File))
                /* Stopped reading when got to end of file. */
                {}
            else if (Line[strlen(Line)-1] == '\n')
                /* Stopped reading when got to end of line. */
                Line[strlen(Line)-1] = '\0';
            else {
                /* The line is too long. Keep it and read the rest. */
                IPO2IData -> CurrLine = NULL;
                _IPO2IStrcat(&LongLine, Line);
                continue;
            }
            if (LongLine != NULL) 
                FinalLine = _IPO2IStrcat(&LongLine, Line);
            else
                FinalLine = Line;

            if (IPO2IData -> Stage == IP_O2I_VECTORS_STAGE)
                _IPO2IAddElement(&IPO2IData -> Lines, FinalLine);
            IritFree(IPO2IData -> CurrLine);
            IPO2IData -> CurrLine = _IPO2ITrimWhiteSpaces(FinalLine);
            IritFree(LongLine);
            LongLine = NULL;
        }
        else {
            IPO2IData -> LineNum++;
            if (IPO2IData -> LineNum > IPO2IData -> Lines.Len)
                return FALSE;
            IritFree(IPO2IData -> CurrLine);
            IPO2IData -> CurrLine = IritStrdup( IPO2IData -> 
                Lines.U.Strings[IPO2IData -> LineNum - 1]);
        }
        IPO2IData -> NextToken = IPO2IData -> CurrLine;

        /* Ignores remarks and empty lines. */
        if ((IPO2IData -> CurrLine[0] == '#') || 
            (strlen(IPO2IData -> CurrLine) == 0))
            continue;
        if ((IPO2IData -> CurrLine[0] == IP_O2I_CONTINUE_NEXT_LINE) &&
            (strlen(IPO2IData -> CurrLine) == 1)) {
            continue; /* Line containinig only IP_O2I_CONTINUE_NEXT_LINE is  */
                      /* meaningless.                                        */
        }
        if (IPO2IData -> CurrLine[strlen(IPO2IData -> CurrLine) - 1] ==
            IP_O2I_CONTINUE_NEXT_LINE) {
            int i;

            /* Removing the IP_O2I_CONTINUE_NEXT_LINE character. */
            for (i = (int) (strlen(IPO2IData -> CurrLine) - 2);
		 (i >= 0) && (strchr(IP_O2I_WHITE_SPACE,
				     IPO2IData -> CurrLine[i]) != NULL);
		 i--);
            if (i == -1) 
                /* The line contains only IP_O2I_CONTINUE_NEXT_LINE. */
                continue;
            IPO2IData -> CurrLine[i + 1] = '\0';
            IPO2IData -> ConNextLine = TRUE;
        }
        else 
            IPO2IData -> ConNextLine = FALSE;
        return TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read next token from IPO2IData until it gets to the end of line (In      *
*   which case it doesn't read any token and return IP_O2I_LINE_END). If the *
*   line ends with IP_O2I_CONTINUE_NEXT_LINE it keeps reading from the next  *
*   line as if the line wasn't ended. Comments and blanks line are already   *
*   ignored in ReadLine.                                                     *
*   Special flags in IPO2IData:                                              *
*       EndOfLine  - mark that last reading got to the end of line.          *
*       GoNextLine - set this flag to TRUE in order to start reading next    *
*                    line.                                                   *
*   If can't read any more lines from file returns IP_O2I_NO_MORE_LINES.     *
*                                                                            *
* PARAMETERS:                                                                *
*   Token:        Buffer for the read token.                                 *
*   SetTokenRead: FALSE - after reading the token don't set it as read. Next *
*                 reading will read the same token.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: IP_O2I_READ_TOKEN    A token was read. There are more tokens in the *
*                             line.                                          *
*        IP_O2I_LINE_END      No token was read. There aren't more tokens in *
*                             the line but there might be more lines.        *
*        IP_O2I_NO_MORE_LINES No token was read. There are no more tokens    *
*                             and no more lines (May appear only if          *
*                             GoNextLine was set to TRUE before the call).   *
*****************************************************************************/
static int IPO2INextToken2(char *Token, int SetTokenRead)
{
    if ((IPO2IData -> EndOfLine == TRUE) && (IPO2IData -> GoNextLine == FALSE))
        return IP_O2I_LINE_END;
    else 
        IPO2IData -> EndOfLine = FALSE;
    IPO2IData -> LastToken = Token;

    while(TRUE) {
        char *Temp, *CurrToken, RemovedSpace;
 
        if ((IPO2IData -> NextToken == NULL) || (IPO2IData -> GoNextLine)) {
            int ReadNewLine = FALSE;

            if (IPO2IData -> GoNextLine && IPO2IData -> ConNextLine) 
                ReadNewLine = TRUE;
            if (!IPO2IReadLine())
                return IP_O2I_NO_MORE_LINES;
            if (ReadNewLine) 
                continue;
            IPO2IData -> GoNextLine = FALSE;
        }
        CurrToken = IPO2IData -> NextToken;
        Temp = strpbrk(CurrToken, IP_O2I_WHITE_SPACE);
        if (Temp == NULL) {
            /* Empty lines are not getting here so no check for empty line. */
            /* The token is the last token in the line. */
            if (strlen(CurrToken) > IRIT_LINE_LEN) 
                /* The token is too long. */
                IPO2IErrorAndExit("Token \"%s\" is too long", CurrToken);
            strcpy(Token, CurrToken);
            if (SetTokenRead) {
                IPO2IData -> NextToken = NULL;
                if (IPO2IData -> ConNextLine) {
                    if (!IPO2IReadLine())
                        IPO2IData -> EndOfLine = TRUE;
                }
                else
                    IPO2IData -> EndOfLine = TRUE;
            }
            return IP_O2I_READ_TOKEN;
        }
        RemovedSpace = Temp[0];
        Temp[0] = '\0';
        /* The token is too long. */
        if (strlen(CurrToken) > IRIT_LINE_LEN) 
            IPO2IErrorAndExit("Token \"%s\" is too long", CurrToken);
        strcpy(Token, CurrToken);
        if (SetTokenRead) 
            /* Advancing to next token */
            for (IPO2IData -> NextToken = Temp + 1; 
                strchr(IP_O2I_WHITE_SPACE, *IPO2IData -> NextToken) 
                != NULL; IPO2IData -> NextToken++);
        else
            Temp[0] = RemovedSpace;
        return IP_O2I_READ_TOKEN;        
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read next token from IPO2IData. See IPO2INextToken2 for more details.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Token: Buffer for the read token.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: IP_O2I_READ_TOKEN    A token was read. There are more tokens in the *
*                             line.                                          *
*        IP_O2I_LINE_END      No token was read. There aren't more tokens in *
*                             the line but there might be more lines.        *
*        IP_O2I_NO_MORE_LINES No token was read. There are no more tokens    *
*                             and no more lines (May appear only if          *
*                             GoNextLine was set to TRUE before the call).   *
*****************************************************************************/
static int IPO2INextToken(char *Token)
{
    return IPO2INextToken2(Token, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read from next current till the end of line (Including next lines if the *
*   current line ends with IP_O2I_CONTINUE_NEXT_LINE).                       *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *: The line in new allocated memory.                                *
*****************************************************************************/
static char *IPO2IReadToEndOfLine() {

    char Token[IRIT_LINE_LEN + 1], *Line;

    if (IPO2IData -> EndOfLine)
        return IritStrdup("");
    Line = IritStrdup(IPO2IData -> NextToken);
    while (IPO2IData -> ConNextLine) {
        char 
            *CurrLine = IritStrdup(IPO2IData -> CurrLine);

        while (stricmp(CurrLine, IPO2IData -> CurrLine) == 0)
            IPO2INextToken(Token);
        _IPO2IStrcat(&Line, " ");
        _IPO2IStrcat(&Line, IPO2IData -> NextToken);
        IritFree(CurrLine);
    }
    return Line;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Interpret the given Token as 1, 2 or 3 integers in the pattern #, #/#,   *
*   #/#/#, #//# (#// and #/#/ are also accepted as equivalent to # and #/#). *
*                                                                            *
* PARAMETERS:                                                                *
*   Token: The token to interpret.                                           *
*   Result: Expected to point to IPO2IListStruct of type IP_O2I_IVEC_3.      *
*   Params: Array of 5 integers. If number 0 is -1 then numbers 1 to 4 are   *
*           corresponding to pattern 1 to 4 above. Their value is TRUE if    *
*           this pattern is allowed, then the read pattern is written to     *
*           cell number 0. If number 0 isn't -1 it means that previous       *
*           values were in that pattern and only that pattern is aacepted.   *
*           If unaccepted pattern is recieved call IPO2IErrorAndExit.        *
*   Number: The number of this token in the overall read tokens.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IInterpInts(char *Token, void *Result, void *Params, int Number)
{
    int Res,
	Pattern = -1,
        *Prms = (int*)Params;
    IPO2IListStruct
        *Buf = (IPO2IListStruct*) Result;
    IPO2IInt3Type Vec;

    Res = sscanf(Token, "%d/%d/%d", &Vec[0], &Vec[1], &Vec[2]);
    if (Res == 3)
        Pattern = 3;
    else if (Res == 2)
        Pattern = 2;
    else {
        Res = sscanf(Token, "%d//%d", &Vec[0], &Vec[2]);
        if (Res == 2)
            Pattern = 4;
        else if (Res == 1)
            Pattern = 1;
        else
            IPO2IErrorAndExit("The token \"%s\" is expected to point to v/vt/vn"
                              " vector", Token);
    }
    assert(Pattern != -1);

    if (Prms[0] == -1) {
        if (Prms[Pattern])
            Prms[0] = Pattern;
        else 
            IPO2IErrorAndExit("The token \"%s\" isn't in the expected format",
                              Token);
    }
    else if (Prms[0] != Pattern) 
        IPO2IErrorAndExit("The format of the token \"%s\" isn't compatible "
                          "with the format of previous tokens", Token);
    _IPO2IAddElement(Buf, Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Interpret the given Token as one integer. Ignore additional "grabage"    *
*   after the integer. If can't parse integer call IPO2IErrorAndExit.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Token:  The token to interpret.                                          *
*   Result: Expected to be an array of integers or IPO2IListStruct of type   *
*           IP_O2I_I (depends on Params).                                    *
*   Params: Expecting IPO2IResultType. NULL will be interpreted as           *
*           IP_O2I_RESULT_ARRAY.                                             *
*   Number: The number of this token in the overall read tokens (starting    *
*           from 0).                                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IInterpInt(char *Token, void *Result, void *Params, int Number)
{
    int Temp;

    if (sscanf(Token,"%d", &Temp) == 0)  
        IPO2IErrorAndExit("The  token \"%s\" is expected to be an integer", 
                          Token);
    if ((Params == NULL) || 
        (*(IPO2IResultType*)Params == IP_O2I_RESULT_ARRAY)) {
        int *Buf = Result;

        Buf[Number] = Temp;
    }
    else {
        IPO2IListStruct *List = Result;

        _IPO2IAddElement(List, &Temp);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Interpret the given Token as one IrtRType. Ignore additional "grabage"   *
*   after the IrtRType. If can't parse float call IPO2IErrorAndExit.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Token:  The token to interpret.                                          *
*   Result: Expected to be an array of IrtRType or IPO2IListStruct of type   *
*           IP_O2I_R (depends on Params).                                    *
*   Params: Expecting IPO2IResultType. NULL will be interpreted as           *
*           IP_O2I_RESULT_ARRAY.                                             *
*   Number: The number of this token in the overall read tokens (starting    *
*           from 0).                                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IInterpReal(char *Token, void *Result, void *Params, int Number)
{
    double Temp;

    if (sscanf(Token,"%lf", &Temp) == 0)
        IPO2IErrorAndExit("The  token \"%s\" is expected to be a real value", 
                          Token);
    if ((Params == NULL) || 
        (*(IPO2IResultType*)Params == IP_O2I_RESULT_ARRAY)) {
        IrtRType
	    *Buf = Result;

        Buf[Number] = Temp;
    }
    else {
        IPO2IListStruct
	    *List = Result;

        _IPO2IAddElement(List, &Temp);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read values from the line. Number of values may be between Min to        *
*   Max. If there are too few values call IPO2IErrorAndExit. InterpFunc is   *
*   responsible for calling IPO2IErrorAndExit in case of wrong format. Extra *
*   values are ignored.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Min:      Minimum values to read.                                        *
*   Max:      Maximum values to read. IP_O2I_READ_ALL_VALUES to read all     *
*             available values.                                              *
*   Result:   Read the values into this buffer.                              *
*   Params:   Params to Interp, its type depends on Interp.                  *
*   Interp:   Function that will interp the value and call ErrorAndExit if   *
*             it's in the wrong format. It will put the results in Results.  * 
*                                                                            *
* RETURN VALUE:                                                              *
*   int: Number of read values.                                              *
*****************************************************************************/
static int IPO2IReadTokens(int Min, 
                           int Max,
                           void *Result,
                           void *Params,
                           IPO2IInterpValueFuncType InterpFunc)
{
    int i, Res;

    if (Max == 0)
        return 0;
    if (Max == IP_O2I_READ_ALL_VALUES)
        Max = IRIT_MAX_INT;
    for (i = 0; i <= Max - 1; i++) {
        char Token[IRIT_LINE_LEN + 1];

        Res = IPO2INextToken(Token);
        if ((Res == IP_O2I_LINE_END) || (Res == IP_O2I_NO_MORE_LINES)) {
            if (i < Min) 
                IPO2IErrorAndExit("Expected at least %d values", Min);
            return i;
        }
        InterpFunc(Token, Result, Params, i);
    }
    /* Additional values are ignored. */
    return Max;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's newmtl statement.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseNewmtl(void)
{
    char Token[IRIT_LINE_LEN + 1];

    if (IPO2INextToken(Token) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    IPO2IData -> Mtl.Name = IritStrdup(Token);
    IPO2IData -> Mtl.Attr = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's Ka, Kd, Ks or Tf statements.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseKadsTf(void)
{
    char Token1[IRIT_LINE_LEN + 1], Token2[IRIT_LINE_LEN + 1],
        Temp[IRIT_LINE_LEN + 1], AttrName[IRIT_LINE_LEN + 1],
        *Statement = IPO2IData -> LastToken;
    IrtRType Params[3], Avg;

    if (IPO2INextToken2(Token1, FALSE) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    if (stricmp(Token1, "spectral") * stricmp(Token1, "xyz") == 0) {
        if (stricmp(Token1, "spectral") == 0) {
            IrtRType
	        Factor = 1.0;

            if (IPO2INextToken(Token2) == IP_O2I_LINE_END)
                IPO2IErrorAndExit("Expecting more parameters");
            IPO2IReadTokens(0, 1, &Factor, NULL, IPO2IInterpInt);
            sprintf(AttrName, "%s%s", IP_O2I_NOT_SUPPORTED_PREFIX, Statement);
            sprintf(Temp, "spectral %s %f", Token2, Factor);
            AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Temp);
            IPO2IWarning("The \"spectral\" option of the statement \"%s\"isn't "
			 "supported", Statement);
            return;
        }
        IPO2INextToken(Token1);                       /* Removing the token. */
    }
    if (IPO2INextToken2(Token2, FALSE) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    /* There is either one RGB/CIE value for all or three     */
    /* values.                                                */
    if (sscanf(Token2, "%lf", &Params[0]) != 1)
        IPO2IErrorAndExit("The  token \"%s\" is expected to "
                          "be a real", Token2);
    /* Only one value for all values. */
    if (IPO2IData -> EndOfLine) {
        Params[2] = Params[1] = Params[0];
    }
    else
        IPO2IReadTokens(3, 3, &Params, NULL, IPO2IInterpReal);
    Avg = (Params[0] + Params[1] + Params[2])/3;
    if (stricmp(Token1, "xyz") == 0)
        IPO2ICIEToRGB(Params, Params);
    sprintf(Temp, "%s %s %s", _IPReal2Str(Params[0]), 
        _IPReal2Str(Params[1]), _IPReal2Str(Params[2]));
    sprintf(AttrName, "%s%s", IP_O2I_NOT_SUPPORTED_PREFIX, Statement);
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Temp);
    if (stricmp(Statement, "Tf") == 0) {
        if (IRIT_APX_EQ(Params[0], 1) && IRIT_APX_EQ(Params[1], 1) && 
            IRIT_APX_EQ(Params[2], 1)) {
            if (IPO2IIgnoreFullTransp) 
                Avg = 0;
            else
                IPO2IWarning("Recieved a Tf value (1,1,1). A complete transparent material is meaningless");
        }
        AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "transp", Avg);
    }
    else if (stricmp(Statement, "Kd") == 0) {
        AttrSetRGBColor(&IPO2IData -> Mtl.Attr, (int)(Params[0]*255 + 0.5), 
                        (int)(Params[1]*255 + 0.5), 
                        (int)(Params[2]*255 + 0.5));
    }
    else if (stricmp(Statement, "Ks") == 0) 
        AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "Specular", Avg);                     
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's d statement.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseD(void)
{
    IrtRType d;
    char Token[IRIT_LINE_LEN + 1], AttrName[IRIT_LINE_LEN + 1];

    if (IPO2INextToken2(Token, FALSE) == IP_O2I_LINE_END)
        IPO2IErrorAndExit("Expecting more parameters");
    if (stricmp(Token, "-halo") == 0) {
        sprintf(AttrName, "%sd_halo", IP_O2I_NOT_SUPPORTED_PREFIX);
        AttrSetIntAttrib(&IPO2IData -> Mtl.Attr, AttrName, 1);
        IPO2INextToken(Token);                        /* Removing the token. */
    }
    IPO2IReadTokens(1, 1, &d, NULL, IPO2IInterpReal);
    sprintf(AttrName, "%sd", IP_O2I_NOT_SUPPORTED_PREFIX);
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, AttrName, d);
    if (IRIT_APX_EQ(d, 0)) {
        if (IPO2IIgnoreFullTransp) 
            d = 1;
        else
            IPO2IWarning("Recieved a d value 0. A complete transparent material is meaningless");
    }
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "transp", 1 - d);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's illum statement.                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseIllum(void)
{
    int Illum;
    char AttrName[IRIT_LINE_LEN + 1];

    IPO2IStatementNotSupported();
    IPO2IReadTokens(1, 1, &Illum, NULL, IPO2IInterpInt);
    sprintf(AttrName, "%sillum", IP_O2I_NOT_SUPPORTED_PREFIX);
    AttrSetIntAttrib(&IPO2IData -> Mtl.Attr, AttrName, Illum);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's Ns statement.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseNs(void)
{
    IrtRType Ns;

    IPO2IReadTokens(1, 1, &Ns, NULL, IPO2IInterpReal);
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "srf_cosine", Ns);
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "specpow", Ns);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's Ni statement.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseNi(void)
{
    IrtRType Ni;

    IPO2IReadTokens(1, 1, &Ni, NULL, IPO2IInterpReal);
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, "ior", Ni);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -Blenu or -Blenv option.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseBlenuv(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], Token[IRIT_LINE_LEN + 1],
        *Statement = IPO2IData -> LastToken;
    
    IPO2MtlOptNotSupported(MapToken, Statement);
    if (IPO2INextToken(Token) == IP_O2I_LINE_END) 
        IPO2IErrorAndExit("Expected more parameters");
    sprintf(AttrName, "%s%s_%s", IP_O2I_NOT_SUPPORTED_PREFIX, 
        MapToken, Statement + 1); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Token);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -bm option.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseBm(char *MapToken) 
{
    IrtRType Mult;
    char AttrName[IRIT_LINE_LEN + 1];

    if (stricmp(MapToken, "bump") != 0)
        IPO2IErrorAndExit("Only \"bump\" statement accept \"-bm\" option");
    IPO2MtlOptNotSupported(MapToken, "-bm");
    IPO2IReadTokens(1, 1, &Mult, NULL, IPO2IInterpReal);
    sprintf(AttrName, "%s%s_bm", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, AttrName, Mult);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -boost option.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseBoost(char *MapToken) 
{
    IrtRType Value;
    char AttrName[IRIT_LINE_LEN + 1];

    IPO2MtlOptNotSupported(MapToken, "-boost");
    IPO2IReadTokens(1, 1, &Value, NULL, IPO2IInterpReal);
    sprintf(AttrName, "%s%s_boost", IP_O2I_NOT_SUPPORTED_PREFIX, 
        MapToken); 
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, AttrName, Value);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -clamp option.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseCc(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], Token[IRIT_LINE_LEN + 1];
    
    if (stricmp(MapToken, "map_Ka") * stricmp(MapToken, "map_Kd") *
        stricmp(MapToken, "map_Ks") != 0)
        IPO2IErrorAndExit("Only \"map_Ka\", \"map_Kd\", and \"map_ks\""
                          " statements accept \"-cc\" option");
    IPO2MtlOptNotSupported(MapToken, "-cc");
    if (IPO2INextToken(Token) == IP_O2I_LINE_END) 
        IPO2IErrorAndExit("Expected more parameters");
    sprintf(AttrName, "%s%s_cc", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Token);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -cc option.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseClamp(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], Token[IRIT_LINE_LEN + 1];
    
    IPO2MtlOptNotSupported(MapToken, "-clamp");
    if (IPO2INextToken(Token) == IP_O2I_LINE_END) 
        IPO2IErrorAndExit("Expected more parameters");
    sprintf(AttrName, "%s%s_clamp", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Token);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -imfchan option.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseImfchan(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], Token[IRIT_LINE_LEN + 1];
    
    if (stricmp(MapToken, "map_Ns") * stricmp(MapToken, "map_d") *
        stricmp(MapToken, "decal") * stricmp(MapToken, "disp") * 
        stricmp(MapToken, "bump") != 0)
        IPO2IErrorAndExit("Only \"map_Ns\", \"map_d\", \"decal\", \"disp\","
                          " and \"bump\" statements accept \"-imfchan\""
                          " option");
    IPO2MtlOptNotSupported(MapToken, "-imfchan");
    if (IPO2INextToken(Token) == IP_O2I_LINE_END) 
        IPO2IErrorAndExit("Expected more parameters");
    sprintf(AttrName, "%s%s_imfchan", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, Token);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -mm option.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseMm(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], Token[IRIT_LINE_LEN + 1],
         ParamsStr[IRIT_LINE_LEN + 1];
    IrtRType
        Params[2] = { 0, 1 };

    IPO2MtlOptNotSupported(MapToken, "-mm");
    IPO2IReadTokens(1, 1, &Params[0], NULL, IPO2IInterpReal);
    if((IPO2INextToken2(Token, FALSE) != IP_O2I_LINE_END) &&
       (sscanf(Token, "%lf", &Params[1]) == 1))
        IPO2INextToken(Token);                  /* Remove the token. */
    sprintf(ParamsStr, "%f %f", Params[0], Params[1]);
    sprintf(AttrName, "%s%s_mm", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, ParamsStr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -o, -s, or -t option.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseOst(char *MapToken) 
{
    char AttrName[IRIT_LINE_LEN + 1], ParamsStr[IRIT_LINE_LEN + 1],
         Token[IRIT_LINE_LEN + 1],
         *Statement = IPO2IData -> LastToken;
    IrtRType Params[3];
    int i, 
        Count = 1;

    IPO2MtlOptNotSupported(MapToken, Statement);
    IPO2IReadTokens(1, 1, &Params[0], NULL, IPO2IInterpReal);
    if((IPO2INextToken2(Token, FALSE) != IP_O2I_LINE_END) &&
       (sscanf(Token, "%lf", &Params[1]) == 1)) {
        Count++;
        IPO2INextToken(Token);                  /* Remove the token. */
    }
    if((IPO2INextToken2(Token, FALSE) != IP_O2I_LINE_END) &&
       (sscanf(Token, "%lf", &Params[2]) == 1)) {
        Count++;
        IPO2INextToken(Token);                  /* Remove the token. */
    }

    /* -o or -t default 0,0,0. */
    if (stricmp(Statement, "-o") * 
        stricmp(Statement, "-t") == 0) {
        for(i = Count; i <= 2; i++)
            Params[i] = 0;
    }
    /* -s default 1,1,1. */
    else {
        for(i = Count; i <= 2; i++)
            Params[i] = 1;
    }
    sprintf(ParamsStr, "%f %f %f", Params[0], Params[1], Params[2]);
    sprintf(AttrName, "%s%s_%s", IP_O2I_NOT_SUPPORTED_PREFIX, 
        MapToken, Statement + 1); 
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, ParamsStr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement's -texres option.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   MapToken: The type of the map analyzed.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseTexres(char *MapToken) 
{
    IrtRType Value;
    char AttrName[IRIT_LINE_LEN + 1];

    IPO2MtlOptNotSupported(MapToken, "-texres");
    IPO2IReadTokens(1, 1, &Value, NULL, IPO2IInterpReal);
    sprintf(AttrName, "%s%s_texres", IP_O2I_NOT_SUPPORTED_PREFIX, MapToken); 
    AttrSetRealAttrib(&IPO2IData -> Mtl.Attr, AttrName, Value);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map statement.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseMap(void)
{  
    char
        *MapName = IPO2IData -> LastToken;
    int GotTexture = FALSE;

    while (TRUE) {
        IPAttributeStruct
	    **Attr = &IPO2IData -> Mtl.Attr;
        char Token[IRIT_LINE_LEN + 1];
        int i,
            Len = sizeof(IPO2IParseMtlMapOpts) /
                  sizeof(IPO2IParseMtlMapOptStruct);

        if (IPO2INextToken2(Token, FALSE) == IP_O2I_LINE_END) {
            if (GotTexture)
                return;
            IPO2IErrorAndExit("Expecting texture/image file");
        }
        if (Token[0] != '-') {
            char *Temp, *Texture;

            /* Token not starting with '-' is the file name. Notice that it  */
            /* doesn't support file names that split over two lines.         */
            char AttrName[IRIT_LINE_LEN + 1];

            /* Only map_Kd is supported for now. */
            if (stricmp(MapName, "map_Kd") != 0) {
                IPO2IData -> LastToken = MapName;
                IPO2IStatementNotSupported();
                sprintf(AttrName, "%s%s_", IP_O2I_NOT_SUPPORTED_PREFIX, MapName);
            }
            else {
                AttrName[0] = '\0';
                IPO2IWarning("Reference to texture file \"%s\"", 
                    IPO2IData -> NextToken);
            }
            strcat(AttrName, "ptexture");
            Texture = IPO2IData -> NextToken;
            for (Temp = Texture; (*Temp != '\0') && !((*Temp == '-') &&
                (strchr(IP_O2I_WHITE_SPACE, *(Temp - 1)) != NULL)); Temp++);
            if (*Temp != '\0') {
                IPO2IData -> NextToken = Temp;
                for (Temp--; strchr(IP_O2I_WHITE_SPACE, *(Temp - 1)) != NULL; 
                    Temp--);
                *Temp = '\0';
                AttrSetStrAttrib(Attr, AttrName, Texture);
                GotTexture = TRUE;
            }
            else {
                AttrSetStrAttrib(Attr, AttrName, Texture);
                return;
            }
        }
        IPO2INextToken(Token);
        for (i = 0; i <= Len - 1; i++) {
            if (stricmp(IPO2IParseMtlMapOpts[i].Statement, Token) == 0) {
                IPO2IParseMtlMapOpts[i].Func(MapName);
                break;
            }
        }
        if (i == Len)
            IPO2IErrorAndExit("Unknown option \"%s\" for %s", Token, 
                MapName);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's map_Kd statement.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseMapAat(void)
{
    char AttrName[IRIT_LINE_LEN + 1];

    sprintf(AttrName, "%smap_aat", IP_O2I_NOT_SUPPORTED_PREFIX);
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, IPO2IReadToEndOfLine());
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's refl statement.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseRefl(void)
{
    /* The syntax isn't exactly clear, therefore I just copy all the line. */
    char AttrName[IRIT_LINE_LEN + 1];

    sprintf(AttrName, "%srefl", IP_O2I_NOT_SUPPORTED_PREFIX);
    AttrSetStrAttrib(&IPO2IData -> Mtl.Attr, AttrName, IPO2IReadToEndOfLine());
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Read an mtl's sharpness statement.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IPO2IParseSharpness(void)
{
    int Sharpness;
    char AttrName[IRIT_LINE_LEN + 1];

    IPO2IStatementNotSupported();
    IPO2IReadTokens(1, 1, &Sharpness, NULL, IPO2IInterpInt);
    sprintf(AttrName, "%ssharpness", IP_O2I_NOT_SUPPORTED_PREFIX);
    AttrSetIntAttrib(&IPO2IData -> Mtl.Attr, AttrName, Sharpness);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the message (printf style) using IRIT_WARNING_MSG and then do      *
*   longjmp back to IPOBJLoadFile. The message is prefixed with the message  *
*   at IPO2IOtherErrorMsg (Which include file name and line number). Add at  *
*   the end of the message period, "exiting." and new line character.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Format:   The fromat of the string.                                      *
*   ...:      Parameters for the format.                                     *
*   va_alist: The format of the string and parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IPO2IErrorAndExit(const char *va_alist, ...)
{
    char *Format, *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IPO2IErrorAndExit(const char *Format, ...)
{
    char *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    sprintf(Format2, IPO2IOtherErrorMsg, IPO2IData -> FileName, 
        IPO2IData -> LineNum, Format);
    IRIT_VSPRINTF(p, Format2, ArgPtr);
    IRIT_WARNING_MSG(p);

    va_end(ArgPtr);

    longjmp(IPO2ILongJumpBuffer, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the message (printf style) using IRIT_WARNING_MSG and return.      *
*   The message is prefixed with the message at IPO2IOtherWarningMsg         *
*   (Which include file name and line number). Add at the end of the message *
*   new line character.                                                      *
*   Print the message only if IPO2IWarningMsgs is TRUE.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Format:   The fromat of the string.                                      *
*   ...:      Parameters for the format.                                     *
*   va_alist: The format of the string and parameters.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
#ifdef USE_VARARGS
static void IPO2IWarning(const char *va_alist, ...)
{
    char *Format, *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
static void IPO2IWarning(const char *Format, ...)
{
    char *p, Format2[IRIT_LINE_LEN + 1];
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    if (!IPO2IWarningMsgs) 
        return;
    sprintf(Format2, IPO2IOtherWarningMsg, IPO2IData -> FileName, 
        IPO2IData -> LineNum, Format);
    IRIT_VSPRINTF(p, Format2, ArgPtr);
    IRIT_WARNING_MSG(p);

    va_end(ArgPtr);
}
