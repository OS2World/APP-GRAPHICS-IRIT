/*****************************************************************************
*   General routines to	configuration file reading:			     *
* The config file (same name as program, type .cfg) must have th following   *
* format: one variable setup per line, each line consists of two params.     *
* The first is variable name, which is text string without white spaces.     *
* The second param. contains its value, which might be boolean (TRUE/FALSE), *
* integer, or real type.						     *
*   The main routine should get a structure consists of 3 elements per	     *
* variable: the string to match, the variable to save the data read from     *
* config file, and the type (Boolean, Integer, Real), for type checking.     *
* See config.h for exact definition of this data structure.		     *
*									     *
* Version 0.2 - adding String Type.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.2, Jan. 1989    *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_loc.h"

#define TAB	9

/* Some fatal errors that cause this module to die... */
typedef enum {
    ERR_ONLY_NAME,
    ERR_BOOL_EXPECTED,
    ERR_INT_EXPECTED,
    ERR_REAL_EXPECTED,
    ERR_STRING_EXPECTED,
    ERR_MULTI_STR_EXPECTED,
    ERR_NOT_EXISTS
} CagdFatalErrorType;

IRIT_STATIC_DATA const char *ConfigPath;

static int IritConfigGetLine(char *Line, FILE *f, int *LineCount);
static void UpdateVariable(char *Line,
			   const IritConfigStruct *SetUp,
			   int NumVar,
			   FILE *f,
			   int *LineCount);
static void PrintConfigError(int ErrorNum, int LineCount);
static FILE *FindFile(const char *PrgmName,
		      const char *FileName,
		      char **FoundFileName);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the current configuration data structure contents.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   SetUp:     Configuration data based.                                     M
*   NumVar:    Number of entries on configuration data base.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritConfigPrint, configuration, cfg files                                M
*****************************************************************************/
void IritConfigPrint(const IritConfigStruct *SetUp, int NumVar)
{
    int i, j;
    char *p, **pp;

    IRIT_INFO_MSG("\nCurrent defaults:\n\n");

    for (i = 0; i < NumVar; i++) {
	char VarName[IRIT_LINE_LEN];

	if (SetUp[i].SomeInfo != NULL && strlen(SetUp[i].SomeInfo) > 0)
	    sprintf(VarName, "%s [%s]", SetUp[i].VarName, SetUp[i].SomeInfo);
	else
	    sprintf(VarName, SetUp[i].VarName);

        switch (SetUp[i].VarType) {
	    case IC_BOOLEAN_TYPE:
		if (* ((int *) SetUp[i].VarData))
		    IRIT_INFO_MSG_PRINTF("\t%-20s = TRUE\n", VarName);
		else
		    IRIT_INFO_MSG_PRINTF("\t%-20s = FALSE\n", VarName);
		break;
	    case IC_INTEGER_TYPE:
		IRIT_INFO_MSG_PRINTF("\t%-20s = %d\n", VarName,
					*((int *) SetUp[i].VarData));
		break;
	    case IC_REAL_TYPE:
		IRIT_INFO_MSG_PRINTF("\t%-20s = %g\n", VarName,
					*((IrtRType *) SetUp[i].VarData));
		break;
	    case IC_STRING_TYPE:
		p = *((char **) SetUp[i].VarData);
		IRIT_INFO_MSG_PRINTF("\t%-20s = \"%s\"\n", VarName,
					p == NULL ? "" : p);
		break;
	    case IC_MULTI_STR_TYPE:
		pp = *((char ***) SetUp[i].VarData);
		IRIT_INFO_MSG_PRINTF("\t%-20s = {\n", VarName);
		if (pp != NULL) {
		    for (j = 0; pp[j] != NULL; j++)
		        IRIT_INFO_MSG_PRINTF("\t    \"%s\"\n", pp[j]);
		}
		IRIT_INFO_MSG("\t}\n");
		break;
	    default:
	        assert(0);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetches the next line from the configuration file, or signals EOF.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      Where to place the next line in.                              *
*   f:         File to read the enxt line from.				     *
*   LineCount: To update  with the current line number of f.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if valid data is return, FALSE if EOF.                     *
*****************************************************************************/
static int IritConfigGetLine(char *Line, FILE *f, int *LineCount)
{
    int i, InQuotes;

    do {
        fgets(Line, IRIT_LINE_LEN_MAX_CFG, f);
	(*LineCount)++;

	if (feof(f))
	    return FALSE;

	i = 0;	/* Delete all the part after the ; (The comment) if was any. */
	InQuotes = FALSE;
	while (Line[i] != 0) {
	    if (Line[i] == '"')
	        InQuotes = !InQuotes;
	    if (!InQuotes && Line[i] == ';')
	        break;
	    i++;
	}
	if (Line[i])
	    Line[i] = 0;

	i = 0;			   /* Now test if that line is empty or not: */
	while (Line[i] != 0 && Line[i] <= ' ')
	    i++;				       /* Skip white spaces. */
    }
    while (Line[i] == 0);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main routine of configuration file handling.				     M
*  Gets the program name, PrgmName, and the configuration data base that     M
*  defines the acceptable variables, Setup, with Numvarentries.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PrgmName:  Name of program that uses this data base.                     M
*   SetUp:     Configuration data based.                                     M
*   NumVar:    Number of entries on configuration data base.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:  Name of config file read, or NULL if error.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritConfig, configuration, cfg files                                     M
*****************************************************************************/
const char *IritConfig(const char *PrgmName,
		       const IritConfigStruct *SetUp,
		       int NumVar)
{
    int LineCount = 0;
    char CfgName[IRIT_LINE_LEN_VLONG], Line[IRIT_LINE_LEN_MAX_CFG],
        *Cptr, *FoundName;
    FILE *f;

    assert(strlen(PrgmName) < IRIT_LINE_LEN_VLONG - 5);

    /* Try the full name, with '.cfg' type. */
    strncpy(CfgName, PrgmName, IRIT_LINE_LEN_VLONG - 5);
    Cptr = strrchr(CfgName, '.');
    if (Cptr != NULL)
	*Cptr = 0;					 /* Delete old type. */
    strcat(CfgName, ".cfg");		    /* And add the config file type. */

    if ((f = FindFile(PrgmName, CfgName, &FoundName)) == NULL) {
	/* Try only file name, with '.cfg' type. */
        int i = (int) strlen(PrgmName) - 1;	 /* Skip the full path name: */
	while (i 
	       && PrgmName[i] != '\\'
	       && PrgmName[i] != '/'
	       && PrgmName[i] != ':')
	    i--;
	if (i)
	    i++;

        strncpy(CfgName, &PrgmName[i], IRIT_LINE_LEN_VLONG - 5);
	Cptr = strrchr(CfgName, '.');
	if (Cptr != NULL)
	    *Cptr = 0;					 /* Delete old type. */
	strcat(CfgName, ".cfg");

	if ((f = FindFile(PrgmName, CfgName, &FoundName)) == NULL) {
	    sprintf(Line, "File \"%s\"", CfgName);
	    MISC_FATAL_ERROR(MISC_ERR_CONFIG_FILE_NO_FOUND, Line);
	    return NULL;
	}
    }

    while (IritConfigGetLine(Line, f, &LineCount))
        UpdateVariable(Line, SetUp, NumVar, f, &LineCount);

    fclose(f);

    return FoundName;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to interpret the input Line and update the appropriate variable  *
* in SetUp data structure. NumVar holds number of entries in SetUp Table.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:       Single line from a configuration file to interpret.          *
*   SetUp:      Configuration data based.                                    *
*   NumVar:     Number of entries on configuration data base.                *
*   f:		File we are reading from.				     *
*   LineCount:  Line number in f.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateVariable(char *Line,
			   const IritConfigStruct *SetUp,
			   int NumVar,
			   FILE *f,
			   int *LineCount)
{
    int i, j, MaxNumStrs, NumStrs;
    char VarName[IRIT_LINE_LEN_LONG], VarData[IRIT_LINE_LEN_MAX_CFG],
	*StrStart, *StrEnd, *p, *NewStr, **MultiStr;
    IrtRType Dummy;

    i = j = 0;
    while (Line[i] > ' ') {			  /* Copy the Variable name: */
	VarName[i] = Line[i];
	i++;
    }
    VarName[i] = 0;

    while (Line[i] != 0 && Line[i] <= ' ')
        i++;
    if (Line[i] == 0)
	PrintConfigError(ERR_ONLY_NAME, *LineCount);

    while (Line[i] >= ' ' || Line[i] == TAB)	  /* Copy the Variable data: */
	VarData[j++] = Line[i++];
    VarData[j] = 0;

    for (i = 0; i < NumVar; i++) {
        if (strcmp(VarName, SetUp[i].VarName) == 0) {
	    switch (SetUp[i].VarType) {
	        case IC_BOOLEAN_TYPE:
		    if (strnicmp(VarData, "True", 4) == 0 ||
			strnicmp(VarData, "False", 5) == 0)
		        *((int *) SetUp[i].VarData) =
			                   (strnicmp(VarData, "True", 4) == 0);
		    else
		        PrintConfigError(ERR_BOOL_EXPECTED, *LineCount);
		    return;
		case IC_INTEGER_TYPE:
		    if (sscanf(VarData, "%d", (int *) SetUp[i].VarData) != 1)
		        PrintConfigError(ERR_INT_EXPECTED, *LineCount);
		    return;
		case IC_REAL_TYPE:
#ifdef IRIT_DOUBLE
		    if (sscanf(VarData, "%lf", &Dummy) != 1) {
#else
		    if (sscanf(VarData, "%f", &Dummy) != 1) {
#endif /* IRIT_DOUBLE */
			PrintConfigError(ERR_REAL_EXPECTED, *LineCount);
			return;
		    }
		    *((IrtRType *) SetUp[i].VarData) = Dummy;
		    return;
		case IC_STRING_TYPE:
		    if ((StrStart = strchr(VarData, '"')) != NULL &&
			(StrEnd = strrchr(VarData, '"')) != NULL &&
			StrEnd != StrStart) {
			assert(StrEnd - StrStart < IRIT_LINE_LEN_MAX_CFG - 2);
			NewStr = IritMalloc(1 + ((unsigned int)
							(StrEnd - StrStart)));
			j = 0;
			while (++StrStart != StrEnd)
			    NewStr[j++] = *StrStart;
			NewStr[j] = 0;
			*((char **) SetUp[i].VarData) = NewStr;
		    }
		    else
			PrintConfigError(ERR_STRING_EXPECTED, *LineCount);
		    return;
		case IC_MULTI_STR_TYPE:
		    if ((p = strchr(VarData, '{')) != NULL) {
		        /* Fetch the multiple strings. */
		        MaxNumStrs = 2;
			NumStrs = 0;
		        MultiStr = (char **)
			    IritMalloc(sizeof(char *) * MaxNumStrs);
			p = &p[1];

			/* Get the strings. */
			do {
			    if ((StrStart = strchr(p, '"')) != NULL &&
				(StrEnd = strchr(&StrStart[1], '"')) != NULL) {
			        NewStr = IritMalloc(1 + ((unsigned int)
							(StrEnd - StrStart)));
				j = 0;
				while (++StrStart != StrEnd)
				    NewStr[j++] = *StrStart;
				NewStr[j] = 0;
				MultiStr[NumStrs++] = NewStr;
				p = &StrEnd[1];

				/* Realloc if need more space for str-vec. */
				if (NumStrs >= MaxNumStrs - 1) {
				    MultiStr = IritRealloc(MultiStr,
					      sizeof(char *) * MaxNumStrs,
					      sizeof(char *) * MaxNumStrs * 2);
				    MaxNumStrs *= 2;
				}
			    }
			    else {
			        if (!IritConfigGetLine(VarData, f, LineCount)) {
				    PrintConfigError(ERR_MULTI_STR_EXPECTED,
						     *LineCount);
				    return;
				}
				p = VarData;
			    }
			}
			while (strchr(p, '"') != NULL ||
			       strchr(p, '}') == NULL);

			MultiStr[NumStrs] = NULL;
			*((char ***) SetUp[i].VarData) = MultiStr;
		    }
		    else
			PrintConfigError(ERR_MULTI_STR_EXPECTED, *LineCount);
		    return;
		default:
		    assert(0);
	    }
	}
    }

    PrintConfigError(ERR_NOT_EXISTS, *LineCount);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the given configuration into a file.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:  File to save configuration at.		                     M
*   SetUp:     Configuration data based.                                     M
*   NumVar:    Number of entries on configuration data base.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if saved successfully, FALSE otherwise.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritConfigSave, configuration, cfg files                                 M
*****************************************************************************/
int IritConfigSave(const char *FileName,
		   const IritConfigStruct *SetUp,
		   int NumVar)
{
    int i, j;
    char *p, **pp;
    FILE *f;

    if ((f = fopen(FileName, "w")) == NULL)
        return FALSE;

    fprintf(f, ";\n;File saved by IritConfigSave\n;\n");
    for (i = 0; i < NumVar; i++) {
        fprintf(f, "%s\t", SetUp[i].VarName);
        switch (SetUp[i].VarType) {
	    case IC_BOOLEAN_TYPE:
	        fprintf(f, "%s\n",
			(*((int *) SetUp[i].VarData)) ? "TRUE" : "FALSE");
	        break;
	    case IC_INTEGER_TYPE:
	        fprintf(f, "%d\n", *((int *) SetUp[i].VarData));
	        break;
	    case IC_REAL_TYPE:
	        fprintf(f, "%f\n", *((IrtRType *) SetUp[i].VarData));
	        break;
	    case IC_STRING_TYPE:
	        p = *((char **) SetUp[i].VarData);
	        fprintf(f, "\"%s\"\n", p == NULL ? "" : p);
	        break;
	    case IC_MULTI_STR_TYPE:
	        pp = *((char ***) SetUp[i].VarData);
	        fprintf(f, "{\n");
		if (pp != NULL) {
		    for (j = 0; pp[j] != NULL; j++)
		        fprintf(f, "\t\"%s\"\n", pp[j]);
		}
	        fprintf(f, "}\n");
	        break;
	    default:
	        assert(0);
	}
    }

    fclose(f);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to print fatal configuration file error, and die.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrorNum:   Error number.                                                *
*   LineCount:  Line where error had occured, in configuration file.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintConfigError(int ErrorNum, int LineCount)
{
    char Line[IRIT_LINE_LEN_VLONG];

    sprintf(Line, IRIT_EXP_STR("[%s, ln %d]: "),
	    ConfigPath, LineCount);

    switch (ErrorNum) {
	case ERR_ONLY_NAME:
	    strcat(Line, IRIT_EXP_STR("Only Name found."));
	    break;
        case ERR_BOOL_EXPECTED:
	    strcat(Line, IRIT_EXP_STR("Boolean type expected."));
	    break;
	case ERR_INT_EXPECTED:
	    strcat(Line, IRIT_EXP_STR("Integer type expected."));
	    break;
	case ERR_REAL_EXPECTED:
	    strcat(Line, IRIT_EXP_STR("Real type expected."));
	    break;
	case ERR_STRING_EXPECTED:
	    strcat(Line, IRIT_EXP_STR("String (within \") type expected."));
	    break;
	case ERR_MULTI_STR_EXPECTED:
	    strcat(Line, IRIT_EXP_STR("Multi string type expected."));
	    break;
	case ERR_NOT_EXISTS:
	    strcat(Line, IRIT_EXP_STR("No such set up option."));
	    break;
        default:
	    strcat(Line, IRIT_EXP_STR("Undefined config file error."));
    }

    MISC_FATAL_ERROR(MISC_ERR_CONFIG_ERROR, Line);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to search for a file and open it according to attribute at all   *
* directories defined by PATH environment variable and current dir.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PrgmName:       Name of program that uses the configuration file.        *
*   FileName:       Name of configuration file.                              *
*   FoundFileName:  If config file was found, updated to the found name,     *
*                                                                            *
* RETURN VALUE:                                                              *
*   FILE *:    Open file is found, NULL otherwise                            *
*****************************************************************************/
static FILE *FindFile(const char *PrgmName,
		      const char *FileName,
		      char **FoundFileName)
{
    IRIT_STATIC_DATA char FullPath[IRIT_LINE_LEN_LONG];
    const char *FPath;
    FILE *f;

    *FoundFileName = NULL;
    ConfigPath = searchpath(FileName);

#if defined(AMIGA) && !defined(__SASC)
    if ((f = fopen(FPath = ConfigPath, "r")) != NULL ||
	(f = fopen(FPath = FileName, "r")) != NULL) {
#else
    if ((f = fopen(FPath = ConfigPath, "rt")) != NULL ||
	(f = fopen(FPath = FileName, "rt")) != NULL) {
#endif /* defined(AMIGA) && !defined(__SASC) */
        strncpy(FullPath, FPath, IRIT_LINE_LEN_LONG - 1);
        *FoundFileName = FullPath;
	return f;
    }

    return NULL;
}

#ifdef DEBUG_MAIN_CONFIG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Simple test routine.                                                     *
*****************************************************************************/
void main(int argc, char **argv)
{
    IRIT_STATIC_DATA int i, Test1, Test2, Test3, Test4;
    IRIT_STATIC_DATA double Test5, Test6;
    IRIT_STATIC_DATA char *Test7, *Test8, **Test9, **Test10;
    IritConfigStruct SetUp[] =
	{ { "TestOne",   "-1", (VoidPtr) &Test1, IC_BOOLEAN_TYPE },
	  { "TestTwo",   "-2", (VoidPtr) &Test2, IC_BOOLEAN_TYPE },
	  { "Test333",   "-3", (VoidPtr) &Test3, IC_INTEGER_TYPE },
	  { "Testing4",  "-4", (VoidPtr) &Test4, IC_INTEGER_TYPE },
	  { "TestReal5", "-5", (VoidPtr) &Test5, IC_REAL_TYPE },
	  { "TestReal6", "-6", (VoidPtr) &Test6, IC_REAL_TYPE },
	  { "String7",	 "-7", (VoidPtr) &Test7, IC_STRING_TYPE },
	  { "String8",   "-8", (VoidPtr) &Test8, IC_STRING_TYPE },
	  { "MultStr9",  "-9", (VoidPtr) &Test9, IC_MULTI_STR_TYPE },
	  { "MultStr10", "-0", (VoidPtr) &Test10, IC_MULTI_STR_TYPE } };

    Test1 = Test2 = Test3 = Test4 = 9999;
    Test5 = Test6 = 0.9999;
    Test7 = Test8 = NULL;
    Test9 = Test10 = NULL;

    printf("\nConfigSave1 prints:\n");
    IritConfigSave("ConfigSave1.cfg", SetUp,
		   (sizeof(SetUp) / sizeof(IritConfigStruct)));

    printf("Before:\nTest1 = %d, Test2 = %d, Test3 = %d, Test4 = %d\n"
	    "Test5 = %lf, Test6 = %lf\nTest7 = \"%s\"\nTest8 = \"%s\"\n",
	    Test1, Test2, Test3, Test4, Test5, Test6, Test7, Test8);

    IritConfig(*argv, SetUp,
	       (sizeof(SetUp) / sizeof(IritConfigStruct)));     /* Do it! */

    printf("After:\nTest1 = %d, Test2 = %d, Test3 = %d, Test4 = %d\n"
	    "Test5 = %lf, Test6 = %lf\nTest7 = \"%s\"\nTest8 = \"%s\"\n",
	    Test1, Test2, Test3, Test4, Test5, Test6, Test7, Test8);

    if (Test9 != NULL) {
        printf("\nMulti-strings 9 prints:\n");
	for (i = 0; Test9[i] != NULL; i++)
	    printf("Str %d = \"%s\"\n", i, Test9[i]);
    }

    if (Test10 != NULL) {
        printf("\nMulti-strings 10 prints:\n");
	for (i = 0; Test10[i] != NULL; i++)
	    printf("Str %d = \"%s\"\n", i, Test10[i]);
    }

    printf("\nConfigPrint prints:\n");
    IritConfigPrint(SetUp, (sizeof(SetUp) / sizeof(IritConfigStruct)));

    printf("\nConfigSave2 prints:\n");
    IritConfigSave("ConfigSave2.cfg", SetUp,
		   (sizeof(SetUp) / sizeof(IritConfigStruct)));
}

#endif /* DEBUG_MAIN_CONFIG */
