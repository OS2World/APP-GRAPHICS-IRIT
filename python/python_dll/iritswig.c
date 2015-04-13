/*****************************************************************************
*   Main module of "Irit" - the 3d (not only polygonal) solid modeller.      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Usage:								     *
*   Irit [-t] [-g] [-i] [-z] [file[.irt]]				     *
*									     *
* Written by:  Gershon Elber				Ver 3.0, Apr. 1990   *
*****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "program.h"
#include "misc_lib.h"
#include "ctrl-brk.h"
#include "dosintr.h"
#include "inptprsg.h"
#include "inptprsl.h"
#include "iritprsr.h"
#include "objects.h"
#include "geom_lib.h"
#include "grap_lib.h"
#include "bool_lib.h"
#include "trim_lib.h"
#include "triv_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */
#ifdef __WINNT__
#include <direct.h>
#endif /* __WINNT__ */

#ifdef DJGCC
#include "intr_lib.h"
#include "intr_gr.h"
#endif /* DJGCC */
#ifdef __UNIX__
#if defined (__FreeBSD__)
#include <term.h>
IRIT_STATIC_DATA struct termios GlblOrigTermio;
#else
#include <termio.h>
IRIT_STATIC_DATA struct termio GlblOrigTermio;
#endif /* !__FreeBSD__ */
#endif /* __UNIX__ */

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA char
*VersionStr = "Irit		Version 10.0, Gershon Elber,\n\
	      (C) Copyright 1989-2009 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA char
*VersionStr = "Irit	        " IRIT_VERSION
",	Gershon Elber,	" __DATE__ ",   " __TIME__ "\n"
IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA char
*CtrlStr = "Irit [-t] [-g] [-q] [-i] [-z] [file[.irt]]";

IRIT_STATIC_DATA char
*GlblPrgmHeader = "                Irit - the not only polygonal solid modeller";

IRIT_STATIC_DATA char
    *GlblCopyRight  =
#ifdef OS2GCC
     IRIT_COPYRIGHT ", OS2 " IRIT_VERSION ",   " __DATE__;
#elif AMIGA
     IRIT_COPYRIGHT ", Amiga " IRIT_VERSION ",   " __DATE__;
#elif __WINNT__
     IRIT_COPYRIGHT ", Windows " IRIT_VERSION ", " __DATE__;
#elif __MACOSX__
     IRIT_COPYRIGHT ", Mac " IRIT_VERSION ", " __DATE__;
#elif __FreeBSD__
     IRIT_COPYRIGHT ", FreeBSD " IRIT_VERSION ", " __DATE__;
#elif defined(sun) || defined(SUN4)
     IRIT_COPYRIGHT ", SUN4 " IRIT_VERSION ", " __DATE__;
#elif sgi
     IRIT_COPYRIGHT ", SGI " IRIT_VERSION ", " __DATE__;
#elif LINUX386
     IRIT_COPYRIGHT ", Linux " IRIT_VERSION ", " __DATE__;
#elif __CYGWIN__
     IRIT_COPYRIGHT ", Cygwin " IRIT_VERSION ", " __DATE__;
#elif __UNIX__
     IRIT_COPYRIGHT ", Unix " IRIT_VERSION ", " __DATE__;
#elif NO_CONCAT_STR
     "(C) Copyright 1989-2009 Gershon Elber,  Unix  Version 10";
#else
     IRIT_COPYRIGHT ", Unknown " IRIT_VERSION ", " __DATE__;
#endif

IRIT_STATIC_DATA char
*GlblAuthorName = "                         Written by Gershon Elber";

IRIT_STATIC_DATA int
    GlblKeepClientsCursorEvents = FALSE,
    GlblQuietMode = FALSE;

/* Init CWD to recover on exit.*/
IRIT_STATIC_DATA char GlblCrntWorkingDir[IRIT_LINE_LEN];

IRIT_STATIC_DATA IPObjectStruct
    *GlblLastClientCursorEvent = NULL;        /* Cursor events from clients. */

IRIT_GLOBAL_DATA IPObjectStruct
    *GlblObjList = NULL;		   /* All objects defined on system. */

IRIT_GLOBAL_DATA jmp_buf GlblLongJumpBuffer;	  /* Used in error recovery. */

IRIT_GLOBAL_DATA int
    GlblInterpProd	 = TRUE;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
    { "DoGraphics",	"-t", (VoidPtr) &GlblDoGraphics,     IC_BOOLEAN_TYPE },
    { "BspProdMethod",	"",   (VoidPtr) &GlblBspProdMethod,  IC_INTEGER_TYPE },
    { "PointLength",	"",   (VoidPtr) &GlblPointLenAux,    IC_INTEGER_TYPE },
    { "LineEdit",       "",   (VoidPtr) &GlblLineEditControl,IC_STRING_TYPE },
    { "StartFile",	"",   (VoidPtr) &GlblStartFileName,  IC_STRING_TYPE },
    { "LogFile",	"",   (VoidPtr) &GlblLogFileName,    IC_STRING_TYPE },
    { "FloatFrmt",	"",   (VoidPtr) &GlblFloatFormat,    IC_STRING_TYPE },
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

#define TRUE  1
#define FALSE 0
typedef int BOOL;

#define IRIT_TO_SWIG_LINE_LENGTH 500

static void Interact(void);
static void ValidateVariables(void);
static void PrintInptPrsrError(void);
static void ConvertData2Irt(IPObjectStruct *PObj);
static void IritBoolFatalError(BoolFatalErrorType ErrID);

static void Irit2SwigNumeric(FILE* h_file, FILE* c_file, FILE* i_file);
static void Irit2SwigIPObject(FILE* h_file, FILE* c_file, FILE* i_file);
static void Irit2SwigProcedures();
static void PrintNumericFuncDeclaration(FILE* file, int nIndex);
static void PrintNumericFuncDefinition(FILE* file, int nIndex);
static void PrintIPObjectFuncDeclaration(FILE* file, int nIndex, BOOL bToIFile);
static void PrintIPObjectFuncDefinition(FILE* file, int nIndex);
static void PrintProceduresFuncDeclaration(FILE* file, int nIndex, BOOL bToIFile);
static void PrintProceduresFuncDefinition(FILE* file, int nIndex);

static void Irit2SwigAddons(FILE *i_file, FILE *input_file);
static int SkipNumeric(char* FuncName);

#ifdef DEBUG
static void AllFatalErrorTrap(void);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of IRIT - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
void main(int argc, char **argv)
{
    FILE* h_file = fopen("iritpy_interface.h", "w");
    FILE* c_file = fopen("iritpy_interface.c", "w");
    FILE* i_file = fopen("irit.i", "w");
    FILE* inp_file1 = fopen("irit_to_swig1.in", "r");
    FILE* inp_file2 = fopen("python_link.h", "r");
    FILE* inp_file3 = fopen("irit_to_swig2.in", "r");

    Irit2SwigAddons(i_file, inp_file1);

    fprintf(c_file, "#include \"iritpy_interface.h\"\n\n");
    fprintf(c_file, "#include \"attribut.h\"\n");
    fprintf(c_file, "#include \"dosintr.h\"\n");
    fprintf(c_file, "#include \"program.h\"\n");
    fprintf(c_file, "#include \"inptprsg.h\"\n\n");
    fprintf(c_file, "#include \"inptprsl.h\"\n\n");

    fprintf(h_file, "#ifndef _IRITPY_INTERFACE_H_\n");
    fprintf(h_file, "#define _IRITPY_INTERFACE_H_\n\n");
    fprintf(h_file, "#include \"grap_lib.h\"\n");
    fprintf(h_file, "#include \"ext_lib.h\"\n");
    fprintf(h_file, "#include \"objects.h\"\n");
    fprintf(h_file, "#include \"freeform.h\"\n");
    fprintf(h_file, "#include \"bsc_geom.h\"\n");

    fprintf(h_file, "\n\n/* Numeric returning function: */\n");
    fprintf(i_file, "\n\n/* Numeric returning function: */\n");
    Irit2SwigNumeric(h_file, c_file, i_file);

    fprintf(h_file, "\n\n/* IPObject return functions: */\n");
    fprintf(i_file, "\n\n/* IPObject return functions: */\n");
    Irit2SwigIPObject(h_file, c_file, i_file);

    fprintf(h_file, "\n\n/* Procedures: */\n");
    fprintf(i_file, "\n\n/* Procedures: */\n");
    Irit2SwigProcedures(h_file, c_file, i_file);

    Irit2SwigAddons(i_file, inp_file2);
    Irit2SwigAddons(i_file, inp_file3);

    fprintf(h_file, "\n#endif\n");

    fclose(h_file);
    fclose(c_file);
    fclose(i_file);
    fclose(inp_file1);
    fclose(inp_file2);
}

void ExecOneLine(char *Line)
{
}

void IritExit0(void)
{
}

void IritIdleFunction(int MiliSeconds)
{
}

void IritPrintOneString(const char *Line)
{
    fprintf(stderr, "%s\n", Line);
}

void DefaultFPEHandler(int Type)
{
}


void IritExit(int ExitCode)
{
}

IPObjectStruct *SetIritState(char *Name, IPObjectStruct *Data)
{
    return NULL;
}

IPObjectStruct *IritPickCrsrClientEvent(IrtRType *RWaitTime)
{
    return NULL;
}

char *GetParamFromType(IritExprType Param)
{
    switch (Param) {
	case 0:
	    return "void";
	case NUMERIC_EXPR:
	    return "IrtRType";
	case POINT_EXPR | VECTOR_EXPR:
	    return "IrtPtType";
	case POINT_EXPR:
	    return "IrtPtType";
	case VECTOR_EXPR:
	    return "IrtVecType";
	case PLANE_EXPR:
	    return "IrtPlnType";
	case STRING_EXPR:
	    return "char *";
	case CTLPT_EXPR: /* Fall through. */
	case MATRIX_EXPR:
	case CURVE_EXPR: 
	case SURFACE_EXPR:         
	case OLST_EXPR: 
	case TRIMSRF_EXPR: 
	case TRIVAR_EXPR: 
	case INSTANCE_EXPR: 
	case TRISRF_EXPR: 
	case MULTIVAR_EXPR: 
	case POLY_CURVE_EXPR: 
	case OLST_GEOM_EXPR: 
	case ANY_EXPR:
	    return "IPObjectStruct*"; 
	default:
	    return "IPObjectStruct*"; /* all the |(or) cases */
    }
}


void Irit2SwigNumeric(FILE* file_h, FILE* file_c, FILE* file_i)
{
    int i;

    for (i = 0; i < NumFuncTableSize; i++) {
	if (SkipNumeric(NumFuncTable[i].FuncName)) {
	    continue;
	}

	/* Special case:
	Can't use sizeof, there id a python function by that name. */
	if (strcmp("SIZEOF", NumFuncTable[i].FuncName) == 0)  {
	    fprintf(file_i, "%%rename(%s) Py%s;\n", "SizeOf", _strlwr(NumFuncTable[i].FuncName));
	}
	else {
	    fprintf(file_i, "%%rename(%s) Py%s;\n", _strlwr(NumFuncTable[i].FuncName), _strlwr(NumFuncTable[i].FuncName));
	}
		
	PrintNumericFuncDeclaration(file_i, i);
	fprintf(file_i, ";\n\n");

	PrintNumericFuncDeclaration(file_h, i);
	fprintf(file_h, ";\n");

	PrintNumericFuncDeclaration(file_c, i);
	fprintf(file_c, "\n");

	PrintNumericFuncDefinition(file_c, i);
	fprintf(file_c, "\n");

    }

}

void PrintNumericFuncDeclaration(FILE* file, int nIndex)
{
    int j;

    /* Special case:
    Discrepancy between parameter number 2 (should be IrtRType by the rules). */
    if (strcmp("meshsize", NumFuncTable[nIndex].FuncName) == 0) {
	fprintf(file, "IrtRType Pymeshsize(IPObjectStruct* Param1, IrtRType* Param2)");
	return;
    }
    /* Special case:
    Discrepancy between parameters number 2 and 3 (should be IrtRType by the rules). */
    else if (strcmp("zcollide", NumFuncTable[nIndex].FuncName) == 0) {
	fprintf(file, "IrtRType Pyzcollide(IPObjectStruct* Param1, IPObjectStruct* Param2, IrtRType* Param3, IrtRType* Param4)");
	return;
    }
    
    fprintf(file, "IrtRType ");
    fprintf(file, "Py");
    fprintf(file, _strlwr(NumFuncTable[nIndex].FuncName));
    fprintf(file, "(");
    for (j = 0; j < NumFuncTable[nIndex].NumOfParam - 1; j++) {
	fprintf(file, GetParamFromType((NumFuncTable[nIndex].ParamObjType)[j]));
	fprintf(file, " Param%d", j + 1);
	fprintf(file, ", ");
    }
    if ((NumFuncTable[nIndex].ParamObjType)[j] != 0) {
	fprintf(file, GetParamFromType((NumFuncTable[nIndex].ParamObjType)[j]));
	fprintf(file, " Param%d", j + 1);
    }

    fprintf(file, ")");    
}

void PrintNumericFuncDefinition(FILE* file, int nIndex)
{
    int j;

    fprintf(file, "{\n");
    fprintf(file, "\treturn ");
    fprintf(file, NumFuncTable[nIndex].CFuncName);
    fprintf(file, "(");
    for (j = 0; j < NumFuncTable[nIndex].NumOfParam - 1; j++) {
	fprintf(file, "Param%d", j + 1);
	fprintf(file, ", ");
    }
    fprintf(file, "Param%d", j + 1);
    fprintf(file, ");\n}\n");    
}



void Irit2SwigIPObject(FILE* file_h, FILE* file_c, FILE* file_i)
{
    int i;

    for (i = 0; i < ObjFuncTableSize; i++) {
	/* Skip those functions.
	We implement them in python. */
	if ((strcmp("CTLPT", ObjFuncTable[i].FuncName) == 0) ||
	    (strcmp("LIST", ObjFuncTable[i].FuncName) == 0)) {
	    continue;
	}
	if (strcmp("CONTOUR", ObjFuncTable[i].FuncName) == 0) {
	    fprintf(file_i, "%%rename(Wrap%s) PyWrap%s;\n", _strlwr(ObjFuncTable[i].FuncName),
		_strlwr(ObjFuncTable[i].FuncName));
	}
	else {
	    fprintf(file_i, "%%rename(%s) Py%s;\n", _strlwr(ObjFuncTable[i].FuncName),
		_strlwr(ObjFuncTable[i].FuncName));
	}
	PrintIPObjectFuncDeclaration(file_i, i, TRUE);
	fprintf(file_i, ";\n\n");

	PrintIPObjectFuncDeclaration(file_h, i, FALSE);
	fprintf(file_h, ";\n");

	PrintIPObjectFuncDeclaration(file_c, i, FALSE);
	fprintf(file_c, "\n");

	PrintIPObjectFuncDefinition(file_c, i);
	fprintf(file_c, "\n");
    }
}

void PrintIPObjectFuncDeclaration(FILE* file, int nIndex, BOOL bToIFile)
{
    int j;

    /* Special case:
       contour is function which can get 2-4 parameters, so it should have 
       a wrapper Wrapcontour. */
    if (strcmp("contour", ObjFuncTable[nIndex].FuncName) == 0) {
	fprintf(file, "IPObjectStruct* PyWrapcontour(IPObjectStruct* Param1, IPObjectStruct* Param2, IPObjectStruct* Param3, IPObjectStruct* Param4)");
	return;
    }

    fprintf(file, "IPObjectStruct* ");
    fprintf(file, "Py");
    fprintf(file, _strlwr(ObjFuncTable[nIndex].FuncName));
    fprintf(file, "(");
    for (j = 0; j < ObjFuncTable[nIndex].NumOfParam - 1; j++) {
	fprintf(file, GetParamFromType((ObjFuncTable[nIndex].ParamObjType)[j]));
	if ((ObjFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && bToIFile)
	    fprintf(file, "* INPUT");
	else if ((ObjFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && !bToIFile)
	    fprintf(file, "* Param%d", j + 1);
	else
	    fprintf(file, " Param%d", j + 1);
	fprintf(file, ", ");
    }
    if ((ObjFuncTable[nIndex].ParamObjType)[j] != 0) {
	fprintf(file, GetParamFromType((ObjFuncTable[nIndex].ParamObjType)[j]));
	if ((ObjFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && bToIFile)
	    fprintf(file, "* INPUT");
	else if ((ObjFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && !bToIFile)
	    fprintf(file, "* Param%d", j + 1);
	else
	    fprintf(file, " Param%d", j + 1);    	
    }
    fprintf(file, ")");    
}

void PrintIPObjectFuncDefinition(FILE* file, int nIndex)
{
    int j = 0;
    int NumOfLoops;

    /* Special case: contour is function which can get 2-4 parameters. */
    if (strcmp("contour", ObjFuncTable[nIndex].FuncName) == 0) 
	        fprintf(file, "{\n\treturn ContourFreeform(Param1, Param2 -> U.Plane, Param3, Param4);\n}\n");
    else {
	  NumOfLoops = ObjFuncTable[nIndex].NumOfParam;

	  fprintf(file, "{\n");
	  fprintf(file, "\treturn ");
	  fprintf(file, ObjFuncTable[nIndex].CFuncName);
	  fprintf(file, "(");
	  for (j = 0; j < NumOfLoops - 1; j++) {
	      fprintf(file, "Param%d", j + 1);
	      fprintf(file, ", ");
	  }
	  if (NumOfLoops != 0) {
	      fprintf(file, "Param%d", j + 1);
	  }

	  fprintf(file, ");\n}\n");
      }
}

void Irit2SwigProcedures(FILE* file_h, FILE* file_c, FILE* file_i)
{
    int i;

    for (i = 0; i < GenFuncTableSize; i++) {
	/* Skip translating those functions, because we use python version. */
	if ((strcmp("IF", GenFuncTable[i].FuncName) == 0) ||
	    (strcmp("FOR", GenFuncTable[i].FuncName) == 0) ||
	    (strcmp("WHILE", GenFuncTable[i].FuncName) == 0) ||
	    (strcmp("EXEC", GenFuncTable[i].FuncName) == 0) ||
	    (strcmp("IDYNMEM", GenFuncTable[i].FuncName) == 0) ||
	    (strcmp("IQUERY", GenFuncTable[i].FuncName) == 0)) {
	    continue;
	}


	fprintf(file_i, "%%rename(%s) Py%s;\n", _strlwr(GenFuncTable[i].FuncName),
	    _strlwr(GenFuncTable[i].FuncName));
	PrintProceduresFuncDeclaration(file_i, i, TRUE);
	fprintf(file_i, ";\n\n");

	PrintProceduresFuncDeclaration(file_h, i, FALSE);
	fprintf(file_h, ";\n");

	PrintProceduresFuncDeclaration(file_c, i, FALSE);
	fprintf(file_c, "\n");

	PrintProceduresFuncDefinition(file_c, i);
	fprintf(file_c, "\n");
    }      
}

void PrintProceduresFuncDeclaration(FILE* file, int nIndex, BOOL bToIFile)
{
    int j;

    fprintf(file, "void ");
    fprintf(file, "Py");		
    fprintf(file, _strlwr(GenFuncTable[nIndex].FuncName));
    fprintf(file, "(");
    for (j = 0; j < GenFuncTable[nIndex].NumOfParam - 1; j++) {
	fprintf(file, GetParamFromType((GenFuncTable[nIndex].ParamObjType)[j]));
	if ((GenFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && bToIFile)
	    fprintf(file, "* INPUT");
	else if ((GenFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && !bToIFile)
	    fprintf(file, "* Param%d", j + 1);
	else
	    fprintf(file, " Param%d", j + 1);
	fprintf(file, ", ");
    }
    if ((GenFuncTable[nIndex].ParamObjType)[j] != 0) {
	fprintf(file, GetParamFromType((GenFuncTable[nIndex].ParamObjType)[j]));
	if ((GenFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && bToIFile)
	    fprintf(file, "* INPUT");
	else if ((GenFuncTable[nIndex].ParamObjType)[j] == NUMERIC_EXPR && !bToIFile)
	    fprintf(file, "* Param%d", j + 1);
	else
	    fprintf(file, " Param%d", j + 1);
    }
    fprintf(file, ")");    
}

void PrintProceduresFuncDefinition(FILE* file, int nIndex)
{
    int j = 0;

    fprintf(file, "{\n");
    fprintf(file, "\t");
    fprintf(file, GenFuncTable[nIndex].CFuncName);
    fprintf(file, "(");
    for (j = 0; j < GenFuncTable[nIndex].NumOfParam - 1; j++) {
	fprintf(file, "Param%d", j + 1);
	fprintf(file, ", ");
    }
    if (GenFuncTable[nIndex].NumOfParam != 0) {
	fprintf(file, "Param%d", j + 1);
    }

    fprintf(file, ");\n}\n");    
}


void Irit2SwigAddons(FILE *i_file, FILE *input_file)
{
    char line[IRIT_TO_SWIG_LINE_LENGTH];

    while (fgets(line, IRIT_TO_SWIG_LINE_LENGTH, input_file) != NULL) {
	fprintf(i_file, "%s", line);
    }
}

/* Skip translating those functions, because we use python math library. */
int SkipNumeric(char* FuncName)
{    
    if ((strcmp("ACOS", FuncName) == 0) ||
	(strcmp("ASIN", FuncName) == 0) ||
	(strcmp("ATAN2", FuncName) == 0) ||
	(strcmp("ATAN", FuncName) == 0) ||
	(strcmp("COS", FuncName) == 0) ||
	(strcmp("EXP", FuncName) == 0) ||
	(strcmp("ABS", FuncName) == 0) ||
	(strcmp("FLOOR", FuncName) == 0) ||
	(strcmp("FMOD", FuncName) == 0) ||
	(strcmp("POWER", FuncName) == 0) ||
	(strcmp("LN", FuncName) == 0) ||
	(strcmp("LOG", FuncName) == 0) ||
	(strcmp("SIN", FuncName) == 0) ||
	(strcmp("SQRT", FuncName) == 0) ||
	(strcmp("TAN", FuncName) == 0)) {
	return 1;
    }
	
    return 0;
}
