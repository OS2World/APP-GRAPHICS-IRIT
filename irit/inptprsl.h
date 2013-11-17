/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   General, local to module, definitions for the Input Parser module.	     *
*   Note this module actually consists of InptPrsr/InptEval/OverLoad modules.*
*****************************************************************************/

#ifndef	INPT_PRSR_LH
#define	INPT_PRSR_LH

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Every time, we evaluate an aggregare (if/for/sufnction) we jump by level. */
#define IP_EVAL_BASE_LEVEL	1024
#define IP_EVAL_BASE_MASK	1023
#define IP_EVAL_NEXT_LEVEL(Lvl) (((Lvl) & ~IP_EVAL_BASE_MASK) + \
				                            IP_EVAL_BASE_LEVEL)

#define IP_INIT_PARSER_STACK	100	     /* Depth of expression nesting. */

#define IP_EXPR_MAX_STRING_LEN	10000

typedef enum {
    ZERO_EXPR =     0,
    POLY_EXPR =     0x00000001,
    NUMERIC_EXPR =  0x00000002,
    POINT_EXPR	=   0x00000004,
    VECTOR_EXPR	=   0x00000008,
    PLANE_EXPR	=   0x00000010,
    MATRIX_EXPR	=   0x00000020,
    CURVE_EXPR =    0x00000040,
    SURFACE_EXPR =  0x00000080,
    STRING_EXPR	=   0x00000100,
    OLST_EXPR =     0x00000200,
    CTLPT_EXPR =    0x00000400,
    TRIMSRF_EXPR =  0x00000800,
    TRIVAR_EXPR =   0x00001000,
    INSTANCE_EXPR = 0x00002000,
    TRISRF_EXPR =   0x00004000,
    MODEL_EXPR =    0x00008000,
    MULTIVAR_EXPR = 0x00010000,

    POLY_CURVE_EXPR = POLY_EXPR | CURVE_EXPR,
    POLY_LIST_EXPR = POLY_EXPR | OLST_EXPR,
    GEOM_EXPR =     POINT_EXPR | VECTOR_EXPR | POLY_EXPR | CURVE_EXPR |
		    SURFACE_EXPR | CTLPT_EXPR | TRIMSRF_EXPR | TRIVAR_EXPR |
		    INSTANCE_EXPR | TRISRF_EXPR | MODEL_EXPR | MULTIVAR_EXPR,
    OLST_GEOM_EXPR = OLST_EXPR | GEOM_EXPR,
    ANY_EXPR =      POLY_EXPR | NUMERIC_EXPR | POINT_EXPR | VECTOR_EXPR |
    		    PLANE_EXPR | MATRIX_EXPR | CURVE_EXPR | SURFACE_EXPR |
		    STRING_EXPR | OLST_EXPR | CTLPT_EXPR | TRIMSRF_EXPR |
		    TRIVAR_EXPR | MULTIVAR_EXPR | INSTANCE_EXPR |
		    TRISRF_EXPR | MODEL_EXPR,
    FREE_FORM_EXPR = CURVE_EXPR | SURFACE_EXPR | TRIMSRF_EXPR | TRIVAR_EXPR |
		    TRISRF_EXPR | MULTIVAR_EXPR,

    NO_EXPR =       0x10000000,
    ERROR_EXPR =    0x20000000
} IritExprType;

/* See also IPObjectStruct structure for the different object possible: */
#define IS_POLY_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_POLY)
#define IS_NUM_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_NUMERIC)
#define IS_PT_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_POINT)
#define IS_VEC_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_VECTOR)
#define IS_PLANE_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_PLANE)
#define IS_CTLPT_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_CTLPT)
#define IS_MAT_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_MATRIX)
#define IS_STR_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_STRING)
#define IS_OLST_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_LIST_OBJ)
#define IS_CRV_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_CURVE)
#define IS_SRF_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_SURFACE)
#define IS_TRIMSRF_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_TRIMSRF)
#define IS_TRIVAR_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_TRIVAR)
#define IS_INSTNC_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_INSTANCE)
#define IS_TRISRF_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_TRISRF)
#define IS_MODEL_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_MODEL)
#define IS_MULTIVAR_NODE(Node)	((Node) -> PObj -> ObjType == IP_OBJ_MULTIVAR)

/*****************************************************************************
* a block of command separated by a colon can have assignments followed by   *
* use of variables. We would like to know that when we test for validity.    *
*****************************************************************************/
#define TO_BE_ASSIGNED_TAG	   0x10
#define IS_TO_BE_ASSIGN_OBJ(Obj)   ((Obj) -> Tags & TO_BE_ASSIGNED_TAG)
#define SET_TO_BE_ASSIGN_OBJ(Obj)  ((Obj) -> Tags |= TO_BE_ASSIGNED_TAG)
#define RST_TO_BE_ASSIGN_OBJ(Obj)  ((Obj) -> Tags &= ~TO_BE_ASSIGNED_TAG))

/*****************************************************************************
* Function entry table looks like this (for table see InptPrsr.c module):    *
*****************************************************************************/
#define IP_FUNC_NAME_LEN	31		    /* 10 + NULL terminator. */
#define IP_FUNC_MAX_PARAM	12     /* Update # Param cases in inptevl1.c */
#define IP_ANY_PARAM_NUM	127

typedef struct FuncTableType {
    char FuncName[IP_FUNC_NAME_LEN];
    int FuncToken;
    char *CFuncName;
    void (*Func)();
    IrtBType NumOfParam;
    IritExprType ParamObjType[IP_FUNC_MAX_PARAM];
    IritExprType RetType;
} FuncTableType;

typedef struct NumFuncTableType {
    char FuncName[IP_FUNC_NAME_LEN];
    int FuncToken;
    char *CFuncName;
    double (*Func)();
    IrtBType NumOfParam;
    IritExprType ParamObjType[IP_FUNC_MAX_PARAM];
    IritExprType RetType;
} NumFuncTableType;

typedef struct ObjFuncTableType {
    char FuncName[IP_FUNC_NAME_LEN];
    int FuncToken;
    char *CFuncName;
    IPObjectStruct *(*Func)();
    IrtBType NumOfParam;
    IritExprType ParamObjType[IP_FUNC_MAX_PARAM];
    IritExprType RetType;
} ObjFuncTableType;

typedef struct GenFuncTableType {
    char FuncName[IP_FUNC_NAME_LEN];
    int FuncToken;
    char *CFuncName;
    void (*Func)();
    IrtBType NumOfParam;
    IritExprType ParamObjType[IP_FUNC_MAX_PARAM];
    IritExprType RetType;
} GenFuncTableType;

typedef struct ConstantTableType {
    char FuncName[IP_FUNC_NAME_LEN];
    IrtRType Value;
} ConstantTableType;

typedef struct UserDefinedFuncDefType {
    struct UserDefinedFuncDefType *Pnext;
    char FuncName[IP_FUNC_NAME_LEN];
    int IsFunction;
    int NumParams;
    IPObjectStruct *Params;
    IPObjectStruct *LocalVars;
    struct ParseTree *Body;
} UserDefinedFuncDefType;

/* The followings are defined in the InptEval.c module and are globals so   */
/* InptPrsr.c module will be able to access them...			    */
IRIT_GLOBAL_DATA_HEADER NumFuncTableType NumFuncTable[];
IRIT_GLOBAL_DATA_HEADER int NumFuncTableSize;
IRIT_GLOBAL_DATA_HEADER ObjFuncTableType ObjFuncTable[];
IRIT_GLOBAL_DATA_HEADER int ObjFuncTableSize;
IRIT_GLOBAL_DATA_HEADER GenFuncTableType GenFuncTable[];
IRIT_GLOBAL_DATA_HEADER int GenFuncTableSize;
IRIT_GLOBAL_DATA_HEADER ConstantTableType ConstantTable[];
IRIT_GLOBAL_DATA_HEADER int ConstantTableSize;
IRIT_GLOBAL_DATA_HEADER UserDefinedFuncDefType *UserDefinedFuncList;

/*****************************************************************************
* Tokens used in the expression	to tree	conversion and tree definition.	     *
*****************************************************************************/

#define	IP_TKN_ERROR  0
#define IP_TKN_NONE   1

/* Warning - changing the order of these constants, needs updating the order */
/* of them, in the tables in the begining of InptPrsr.c & OverLoad.c modules.*/

typedef enum {
    IP_USERFUNCDEF = 90,	       /* Function and procedure definition. */
    IP_USERPROCDEF,
    IP_USERINSTDEF
} UserDefinedFuncType;

#define IP_USER_FUNC			0
#define IP_USERFUNC_MAX_PARAM		20
#define IP_MAX_PARAM			IRIT_MAX(IP_USERFUNC_MAX_PARAM, \
					         IP_FUNC_MAX_PARAM)
#define IP_IS_USER_FUNCTION(Token)	((Token) >= IP_USERFUNCDEF && \
					 (Token) <= IP_USERINSTDEF)

typedef enum {				   /* Real value returned functions. */
    IP_ACOS = 100,
    IP_ASIN,
    IP_ATAN2,
    IP_ATAN,
    IP_COS,
    IP_EXP,
    IP_ABS,
    IP_FLOOR,
    IP_FMOD,
    IP_POWER,
    IP_LN,
    IP_LOG,
    IP_SIN,
    IP_SQRT,
    IP_TAN,
    IP_CPOLY,
    IP_AREA,
    IP_VOLUME,
    IP_TIME,
    IP_SIZEOF,
    IP_MESHSIZE,
    IP_THISOBJ,
    IP_RANDOM,
    IP_CLNTEXEC,
    IP_DSTPTLN,
    IP_DSTPTPLN,
    IP_DSTLNLN,
    IP_ZCOLLIDE,

    IP_REAL_VAL_LAST
} RealValueFuncType;

#define IP_NUM_FUNC		100
#define IP_NUM_FUNC_OFFSET	100
#define IP_NUM_FUNC_END		199
#define IP_IS_NUM_FUNCTION(Token)	((Token) >= IP_NUM_FUNC && \
					 (Token) <= IP_NUM_FUNC_END)

typedef enum {				       /* Object returned Functions. */
    IP_POINT = 200,
    IP_VECTOR,
    IP_PLANE,
    IP_CTLPT,
    IP_ROTX,
    IP_ROTY,
    IP_ROTZ,
    IP_ROTVEC,
    IP_ROTZ2V2,
    IP_ROTZ2V,
    IP_ROTV2V,
    IP_MAP3PT2EQL,
    IP_TRANS,
    IP_SCALE,
    IP_BOX,
    IP_GBOX,
    IP_CONE,
    IP_CON2,
    IP_CYLIN,
    IP_SPHERE,
    IP_TORUS,
    IP_CIRCPOLY,
    IP_POLY,
    IP_POLYHOLES,
    IP_CROSSEC,
    IP_SURFREV,
    IP_SURFREVAXS,
    IP_SURFREV2,
    IP_SURFREVAX2,
    IP_SURFPREV,
    IP_EXTRUDE,
    IP_LIST,
    IP_LOAD,
    IP_CONVEX,
    IP_SPOWER,
    IP_CPOWER,
    IP_SBEZIER,
    IP_CBEZIER,
    IP_SBSPLINE,
    IP_CBSPLINE,
    IP_SEVAL,
    IP_CEVAL,
    IP_STANGENT,
    IP_CTANGENT,
    IP_PNORMAL,
    IP_PATTRIB,
    IP_SNORMAL,
    IP_TSNORMAL,
    IP_CNORMAL,
    IP_MDIVIDE,
    IP_TDIVIDE,
    IP_SDIVIDE,
    IP_CDIVIDE,
    IP_MREGION,
    IP_TREGION,
    IP_SREGION,
    IP_CREGION,
    IP_MREFINE,
    IP_TREFINE,
    IP_SREFINE,
    IP_CREFINE,
    IP_MRAISE,
    IP_TRAISE,
    IP_SRAISE,
    IP_CRAISE,
    IP_CREDUCE,
    IP_CSURFACE,
    IP_CMESH,
    IP_NTH,
    IP_NREF,
    IP_GPOLYGON,
    IP_GPOLYLINE,
    IP_GPOINTLIST,
    IP_CIRCLE,
    IP_PCIRCLE,
    IP_CSPIRAL,
    IP_CHELIX,
    IP_CSINE,
    IP_ARC,
    IP_ARC360,
    IP_RULEDSRF,
    IP_RULEDTV,
    IP_BOOLSUM,
    IP_BOOLONE,
    IP_TBOOLSUM,
    IP_TBOOLONE,
    IP_SFROMCRVS,
    IP_SINTPCRVS,
    IP_SWEEPSRF,
    IP_SWPSCLSRF,
    IP_OFFSET,
    IP_AOFFSET,
    IP_ILOFFSET,
    IP_LOFFSET,
    IP_MOFFSET,
    IP_TOFFSET,
    IP_SSHELL,
    IP_COERCE,
    IP_CEDITPT,
    IP_SEDITPT,
    IP_TEDITPT,
    IP_MERGEPOLY,
    IP_MERGEPLLN,
    IP_CMORPH,
    IP_PMORPH,
    IP_SMORPH,
    IP_TMORPH,
    IP_BZR2BSP,
    IP_BSP2BZR,
    IP_SMERGE,
    IP_MMERGE,
    IP_CDERIVE,
    IP_SDERIVE,
    IP_TDERIVE,
    IP_MDERIVE,
    IP_TSDERIVE,
    IP_CINTEG,
    IP_SINTEG,
    IP_SNRMLSRF,
    IP_CNRMLCRV,
    IP_SYMBPROD,
    IP_SYMBDPROD,
    IP_SYMBCPROD,
    IP_SYMBIPROD,
    IP_SYMBSUM,
    IP_SYMBDIFF,
    IP_HOMOMAT,
    IP_PROJMAT,
    IP_RFLCTMAT,
    IP_MATPOSDIR,
    IP_MATDECOMP,
    IP_MATDECOMP2,
    IP_MATRECOMP,
    IP_CINFLECT,
    IP_CCRVTR,
    IP_CFNCRVTR,
    IP_SCRVTR,
    IP_SRADCRVTR,
    IP_CIEXTREME,
    IP_PCRVTR,
    IP_PIMPRTNC,
    IP_SGAUSS,
    IP_SMEAN,
    IP_SUMBILIC,
    IP_CREPARAM,
    IP_SREPARAM,
    IP_TREPARAM,
    IP_MREPARAM,
    IP_EVOLUTE,
    IP_CZEROS,
    IP_CEXTREMES,
    IP_PPINTER,
    IP_CCINTER,
    IP_CARRANGMNT,
    IP_CARNGMNT2,
    IP_BELTCURVE,
    IP_SSINTER,
    IP_SSINTR2,
    IP_MVINTER,
    IP_MVCONTACT,
    IP_NIL,
    IP_COORD,
    IP_COMPOSE,
    IP_DECOMPOSE,
    IP_PRISA,
    IP_CRVPTDST,
    IP_CRVLNDST,
    IP_SRFPTDST,
    IP_SRFLNDST,
    IP_ADAPISO,
    IP_PDOMAIN,
    IP_LINTERP,
    IP_PINTERP,
    IP_CINTERP,
    IP_SINTERP,
    IP_CMULTIRES,
    IP_GETLINE,
    IP_FFEXTREME,
    IP_TRIMSRF,
    IP_TRMSRFS,
    IP_STRIMSRF,
    IP_CTRIMSRF,
    IP_TBEZIER,
    IP_TBSPLINE,
    IP_TSBEZIER,
    IP_TSBSPLINE,
    IP_TSGREGORY,
    IP_TEVAL,
    IP_TSEVAL,
    IP_TEXTGEOM,
    IP_STRIVAR,
    IP_SMESH,
    IP_TINTERP,
    IP_CLNTREAD,
    IP_CLNTCRSR,
    IP_MOMENT,
    IP_TFROMSRFS,
    IP_TINTPSRFS,
    IP_TVREV,
    IP_TVPREV,
    IP_SFOCAL,
    IP_HERMITE,
    IP_BLHERMITE,
    IP_BLSHERMITE,
    IP_FFMATCH,
    IP_CONTOUR,
    IP_PRINTER,
    IP_SRINTER,
    IP_PPINCLUDE,
    IP_CPINCLUDE,
    IP_PLN3PTS,
    IP_PTPTLN,
    IP_PTLNPLN,
    IP_PTSLNLN,
    IP_PTSCRCR,
    IP_TNSCRCR,
    IP_PT3BARY,
    IP_FFSPLIT,
    IP_FFMERGE,
    IP_FFPTTYPE,
    IP_FFGTYPE,
    IP_FFORDER,
    IP_FFMSIZE,
    IP_FFMESH,
    IP_FFKNTVEC,
    IP_FFCTLPTS,
    IP_FFPOLES,
    IP_CNVXHULL,
    IP_MSCONE,
    IP_MSCIRC,
    IP_MSSPHERE,
    IP_CRVPTTAN,
    IP_CRV2TANS,
    IP_CRC2CRVTAN,
    IP_SRF3TANS,
    IP_INSTANCE,
    IP_CANGLEMAP,
    IP_CVIEWMAP,
    IP_CVISIBLE,
    IP_SVISIBLE,
    IP_GETATTR,
    IP_PTHMSPR,
    IP_PDECIMATE,
    IP_FFPTDIST,
    IP_CENVOFF,
    IP_CBISECTOR2D,
    IP_CBISECTOR3D,
    IP_CALPHASECTOR,
    IP_SBISECTOR,
    IP_MBISECTOR,
    IP_MTRISECTOR,
    IP_CVORONOICELL,
    IP_SPRBISECT,
    IP_BSCTPLNPT,
    IP_BSCTCYLPT,
    IP_BSCTCONPT,
    IP_BSCTSPRPT,
    IP_BSCTTRSPT,
    IP_BSCTPLNLN,
    IP_BSCTCONLN,
    IP_BSCTSPRLN,
    IP_BSCTSPRPL,
    IP_BSCTCYLPL,
    IP_BSCTCONPL,
    IP_BSCTCONCON,
    IP_BSCTCONCN2,
    IP_BSCTCONSPR,
    IP_BSCTCYLSPR,
    IP_BSCTSPRSPR,
    IP_BSCTTRSSPR,
    IP_BSCTCYLCYL,
    IP_BSCTCONCYL,
    IP_SKEL2DINT,
    IP_SKELNDINT,
    IP_BBOX,
    IP_ORTHOTOMC,
    IP_BOUNDARY,
    IP_SILHOUETTE,
    IP_SASPCTGRPH,
    IP_SSILINFL,
    IP_POLARSIL,
    IP_ISOCLINE,
    IP_TCRVTR,
    IP_MRCHCUBE,
    IP_TRIANGL,
    IP_MAXEDGELEN,
    IP_PTS2PLLN,
    IP_COVERPT,
    IP_COVERISO,
    IP_TVLOAD,
    IP_DUALITY,
    IP_ALGSUM,
    IP_SWUNGASUM,
    IP_MPROMOTE,
    IP_MREVERSE,
    IP_MFROMMV,
    IP_MFROMMESH,
    IP_MEVAL,
    IP_MZERO,
    IP_MUNIVZERO,
    IP_SPARABOLC,
    IP_RRINTER,
    IP_GGINTER,
    IP_SREVERSE,
    IP_CONICSEC,
    IP_ELLIPSE3PT,
    IP_QUADRIC,
    IP_CNC2QUAD,
    IP_IMPLCTRANS,
    IP_IRITSTATE,
    IP_ISGEOM,
    IP_SMOOTHNRML,
    IP_FIXPLNRML,
    IP_FIXPLGEOM,
    IP_BLOSSOM,
    IP_KNOTCLEAN,
    IP_KNOTREMOVE,
    IP_PKTRI3CRCS,
    IP_PKTRI6CRCS,
    IP_RAYTRAPS,
    IP_CMOEBIUS,
    IP_SMOEBIUS,
    IP_RFLCTLN,
    IP_CAREA,
    IP_SVOLUME,
    IP_SMOMENTS,
    IP_TVZRJACOB,
    IP_CRVKERNEL,
    IP_SRFKERNEL,
    IP_CRVDIAMTR,
    IP_SACCESS,
    IP_SFLECNODAL,
    IP_SRF2TANS,
    IP_NRMLCONE,
    IP_PTREGISTER,
    IP_GETNAME,
    IP_SDDMMAP,
    IP_MVEXPLICIT,
    IP_ANALYFIT,
    IP_PLANECLIP,
    IP_SRFFFORM,
    IP_CCRVTREVAL,
    IP_SCRVTREVAL,
    IP_SASYMPEVAL,
    IP_CARCLEN,
    IP_TEXTWARP,
    IP_SRAYCLIP,
    IP_PPROPFTCH,
    IP_FITPMODEL,
    IP_UVPOLY,
    IP_MPOWER,
    IP_MBEZIER,
    IP_MBSPLINE,
    IP_CBIARCS,
    IP_SPLITLST,
    IP_SETCOVER,
    IP_DIST2FF,
    IP_FFRIGIDSIM,
    IP_FFCMPCRVS,
    IP_ANIMEVAL,
    IP_CUBICCRVS,
    IP_QUADCRVS,
    IP_CBSP_FIT,
    IP_ANTIPODAL,
    IP_SELFINTER,
    IP_PTS2PLYS,
    IP_CCRVTR1PT,
    IP_MINDIST2FF,
    IP_HAUSDORFF,
    IP_NCCNTRPATH,
    IP_NCPCKTPATH,
    IP_UVECSONSPR,
    IP_MFROM2IMG,
    IP_MFROM3IMG,
    IP_BFROM2IMG,
    IP_BFROM3IMG,
    IP_RULEDFIT,
    IP_CSRFPROJ,
    IP_C2RECTRGN,

    IP_OBJ_VAL_LAST
} ObjValueFuncType;

#define IP_OBJ_FUNC1		200
#define IP_OBJ_FUNC2		300
#define IP_OBJ_FUNC3		400
#define IP_OBJ_FUNC4		500
#define IP_OBJ_FUNC_OFFSET	200
#define IP_OBJ_FUNC_END		599
#define IP_IS_OBJ_FUNCTION(Token)	(Token >= IP_OBJ_FUNC1 && \
					 Token <= IP_OBJ_FUNC_END)

typedef enum {		   /* General Functions/No value returned functions. */
    IP_EXIT = 600,
    IP_RESET,
    IP_VIEWOBJ,
    IP_VIEWSET,
    IP_CHDIR,
    IP_INCLUDE,
    IP_SAVE,
    IP_FREE,
    IP_FNFREE,
    IP_IF,
    IP_FOR,
    IP_WHILE,
    IP_HELP,
    IP_VARLIST,
    IP_SYSTEM,
    IP_LOGFILE,
    IP_COLOR,
    IP_AWIDTH,
    IP_ADWIDTH,
    IP_SNOC,
    IP_ATTRIB,
    IP_ATTRPROP,
    IP_ATTRVPROP,
    IP_CPATTR,
    IP_RMATTR,
    IP_FFCOMPAT,
    IP_MSLEEP,
    IP_PRINTF,
    IP_PRINTFILE,
    IP_ERROR,
    IP_CLNTWRITE,
    IP_CLNTCLOSE,
    IP_SETNAME,
    IP_EXEC,
    IP_IQUERY,
    IP_INSERTPOLY,
    IP_EXAMPLEFUNC,
    IP_IDYNMEM,

    IP_GEN_VAL_LAST
} GenValueFuncType;

#define IP_GEN_FUNC		600
#define IP_GEN_FUNC_OFFSET	600
#define IP_GEN_FUNC_END		699
#define IP_IS_GEN_FUNCTION(Token)	((Token) >= IP_GEN_FUNC && \
					 (Token) <= IP_GEN_FUNC_END)

#define IP_IS_FUNCTION(Token)           ((Token) >= 90 && \
					 (Token) < IP_GEN_FUNC_END)

#define IP_IS_NO_PARAM_FUNC(Token)	((Token) == IP_EXIT || \
					 (Token) == IP_RESET || \
					 (Token) == IP_VARLIST || \
					 (Token) == IP_NIL)

typedef enum {						       /* Operators. */
    IP_TKN_PLUS = 800,
    IP_TKN_MINUS,
    IP_TKN_MULT,
    IP_TKN_DIV,
    IP_TKN_POWER,
    IP_TKN_UNARMINUS,
    IP_TKN_EQUAL,
    IP_TKN_COMMA,
    IP_TKN_COLON,
    IP_TKN_SEMICOLON,
    IP_TKN_CMP_EQUAL,
    IP_TKN_CMP_NOTEQUAL,
    IP_TKN_CMP_LSEQUAL,
    IP_TKN_CMP_GTEQUAL,
    IP_TKN_CMP_LESS,
    IP_TKN_CMP_GREAT,
    IP_TKN_BOOL_AND,
    IP_TKN_BOOL_OR,
    IP_TKN_BOOL_NOT,
    IP_TKN_LASTOPER = IP_TKN_BOOL_NOT,

    IP_TKN_OPENPARA,					     /* Paranthesis. */
    IP_TKN_CLOSPARA,

    IP_TKN_NUMBER,					    /* Numeric Data. */
    IP_TKN_PARAMETER,				 /* Point on new/old object. */
    IP_TKN_STRING,	     /* Sequence of characters within double quotes. */

    IP_TKN_START,
    IP_TKN_END
} GenericTokenType;

#define IP_OPERATORS		800
#define IP_OPERATORS_OFFSET	800
#define IP_IS_AN_OPERATOR(Token)	((Token) >= IP_OPERATORS && \
					 (Token) <= IP_TKN_LASTOPER)

#define IP_PRSR_MAX_TOKEN	1000              /* Upper bound for tokens. */

typedef enum {
    IRIT_INPUT_SOURCE_KBD,
    IRIT_INPUT_SOURCE_FILE,
    IRIT_INPUT_SOURCE_LINE_QUEUE
} IritInputSourceType;

IRIT_GLOBAL_DATA_HEADER char IPGlblCharData[];/* Used for both parse & eval. */
IRIT_GLOBAL_DATA_HEADER int InptPrsrLastToken;
IRIT_GLOBAL_DATA_HEADER InptPrsrEvalErrType IPGlblEvalError;/* From EvalTree.*/
IRIT_GLOBAL_DATA_HEADER IritInputSourceType IritInputSource;

/*****************************************************************************
*   The local function (static) prototypes:				     *
*   Note that if DEBUG is defined for the preprocessor, few more function    *
* become available:							     *
*   Also note that some of the routines are defined globals as both the      *
* InptPrsr.c and InptEval.c modules needs them.				     *
*****************************************************************************/

ParseTree *ExprMalloc(void);
void ExprFree(ParseTree *Ptr);
void UpdateCharError(char *StrMsg, int Token, ParseTree *Node);
ParseTree *InptPrsrGenInputParseTree(void);
int InptPrsrSetEchoSource(int EchoSource);
IritExprType InptPrsrTypeCheck(ParseTree *Root, int Level);   /* Type check. */
ParseTree *InptPrsrEvalTree(ParseTree *Root, int Level);   /* Evaluate tree. */
void InptPrsrFreeTree(ParseTree *Root);			   /* Free all tree. */
void InptPrsrPrintTree(const ParseTree *Root, char *Str, int StrLen);
ParseTree *InptPrsrCopyTree(ParseTree *Root);			 /* Copy it. */
IritExprType InptPrsrObjType2Expr(ParseTree *Root, IPObjectStruct *PObj);
ParseTree *InptEvalFetchParameter(ParseTree *Root, int i, int n);
int InptEvalCountNumParameters(ParseTree *Root);

char *InptPrsrTypeToStr(IritExprType Type);
void InptEvalPrintHelp(const char *HelpHeader);
ParseTree *InptEvalCompareObject(ParseTree *Root,
				 ParseTree *Left,
			         ParseTree *Right,
				 InptPrsrEvalErrType *IError,
    				 char *CError);
void InptEvalIfCondition(ParseTree *Cond,
			 ParseTree *CondTrue,
			 ParseTree *CondFalse);
void InptEvalForLoop(ParseTree *PStart,
		     ParseTree *PInc,
		     ParseTree *PEnd,
		     ParseTree *PBody);
void InptEvalWhileLoop(ParseTree *PCond, ParseTree *PBody);
IPObjectStruct *InptEvalMVExplicit(IrtRType *Dim, const char *Expr);
void InptPrsrMarkToBeAssigned(ParseTree *Root);
IPObjectStruct *InptEvalGenObjectList(ParseTree *PObjParams);
IPObjectStruct *InptEvalCtlPtFromParams(ParseTree *PObjParams);
void InptEvalDefineFunc(ParseTree *FuncDef);
ParseTree *InptEvalUserFunc(ParseTree *Root, ParseTree *Params[]);
int InptEvalFetchParameters(ParseTree *Root,
			    FuncTableType *FuncTable,
			    int NumParams,
			    int Level,
			    ParseTree *Params[],
			    VoidPtr ParamPtrs[]);
int IritEvalFuncParamMismatch(ParseTree *Root);
void InptEvalPropagateDependencies(IPObjectStruct *PObj, ParseTree *Root);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* INPT_PRSR_LH */
