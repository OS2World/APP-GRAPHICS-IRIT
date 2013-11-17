/*****************************************************************************
*   Programmer's manual processor of IRIT- a 3d solid modeller.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Usage:								     *
*  ProgMan [-m] [-t] [-l] [-M] [w] [-o OutFileName] [-z] [InFileNames]	     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Oct. 1994   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "irit_sm.h"
#include "misc_lib.h"

#undef	LINE_LEN_LONG
#undef	LINE_LEN
#define LINE_LEN_LONG	200000
#define LINE_LEN	5000

#define MAX_DESCRIPTION		1000
#define MAX_PARAM_DESCRIPTION	100
#define MAX_PARAMETERS		100
#define MAX_KEYWORDS		100
#define MAX_INDENTATION		20

typedef enum {
    UNDEFINED_DOC_PROCESS = 0,

    PLAIN_TEXT_DOCS,
    UNIX_MAN_PAGES,
    LATEX_DOCS,
    HTML_DOCS
} DocProcessType;

typedef struct ParamStruct {
    char *ParamName;
    char **Description;
} ParamStruct;

typedef struct FunctionStruct {
    struct FunctionStruct *Pnext;
    char **Description;
    struct ParamStruct **Parameters;
    struct ParamStruct *RetVal;
    char **Keywords;
    char **SeeAlsos;
    char **Prototype;
    char *FileName;
    int LineNumber;
} FunctionStruct;

IRIT_STATIC_DATA struct FunctionStruct
    *GlblFuncList = NULL;
IRIT_STATIC_DATA int
    GlblFuncNoSeeAlso = FALSE;
IRIT_STATIC_DATA char
    *GlblFoutName = "progman.html";

static char **GetFuncDescription(FILE *Fin, int *LineNum);
static ParamStruct **GetFuncParameters(FILE *Fin, int *LineNum);
static ParamStruct *GetFuncRetVal(FILE *Fin, int *LineNum);
static char **GetFuncKeywords(FILE *Fin, int *LineNum);
static char **GetFuncSeeAlso(FILE *Fin, int *LineNum);
static char **GetFuncPrototype(FILE *Fin, int *LineNum);
static void ProcessFiles(FILE *Fin, char *FileName, int MoreInfo);
static void VerifyParamConsistency(FunctionStruct *Func);
static char *StrdupTabToSpaces(char*Line);
static char *ConvertTabsToSpaces(char*Line);
static char *FilterEOLMV(char *Line, int Skip);
static char *FilterFirstNonSpace(char *Line, int i);
static char *FilterCRLF(char *Line, int Skip);
static char *FileNameFromPath(char *Path);
static void PutString(char *Line, FILE *f);
static void DumpInfoText(FILE *Fout);
static void DumpInfoManPages(FILE *Fout);
static void PutLatexString(FILE *Fout, char *Str);
static void DumpDescriptionForLatex(FILE *Fout, char **Desc);
static void DumpInfoLatex(FILE *Fout);
static void DumpInfoHTML(FILE *Fout, FILE *FoutIndex);

#if defined(ultrix) && defined(mips)
static int FuncQSortComp(VoidPtr VPoly1, VoidPtr VPoly2);
#else
static int FuncQSortComp(const VoidPtr VPoly1, const VoidPtr VPoly2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module - process command line options.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:    command line options.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int MoreInfo = FALSE;
    char Name[LINE_LEN];
    FILE
	*Fin = stdin,
	*Fout = stdout,
	*FoutIndex = NULL;
    DocProcessType
	OutputDocType = UNDEFINED_DOC_PROCESS;

    while (argc > 1) {
	if (strcmp(argv[1], "-t") == 0) {
	    OutputDocType = PLAIN_TEXT_DOCS;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-l") == 0) {
	    OutputDocType = LATEX_DOCS;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-M") == 0) {
	    OutputDocType = UNIX_MAN_PAGES;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-w") == 0) {
	    OutputDocType = HTML_DOCS;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-m") == 0) {
	    MoreInfo = TRUE;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-o") == 0) {
	    if ((Fout = fopen(GlblFoutName = argv[2], "w")) == NULL) {
		fprintf(stderr, "Failed to open \"%s\".\n", argv[2]);
		exit(1);
	    }
	    argc -= 2;
	    argv += 2;
	}
	else if (strcmp(argv[1], "-z") == 0) {
	    fprintf(stderr, "Usage: ProgMan [-t] [-l] [-M] [-w] [-o OutFileName] [-z] InFileNames.\n");
	    exit(0);
	}
	else
	    break;
    }


    if (OutputDocType == UNDEFINED_DOC_PROCESS) {
	fprintf(stderr, "One of [-t] [-l] [-M] [-w] must be specified.\n");
	exit(1);
    }

    if (argc > 1) {
	while (argc > 1) {
	    fprintf(stderr, "PROCESSING FILE %s\n", argv[1]);
	    if ((Fin = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Failed to open \"%s\".\n", argv[1]);
		argc--;
		argv++;
		continue;
	    }

	    ProcessFiles(Fin, argv[1], MoreInfo);
	    fclose(Fin);
	    argc--;
	    argv++;
	}
    }
    else /* Only stdin. */
	ProcessFiles(Fin, "stdin", OutputDocType);

    switch (OutputDocType) {
	case HTML_DOCS:
	    sprintf(Name, "%s.index.html", GlblFoutName);
	    if ((FoutIndex = fopen(Name, "w")) == NULL) {
		fprintf(stderr, "Failed to open \"%s\".\n", Name);
		exit(1);
	    }
	    DumpInfoHTML(Fout, FoutIndex);
	    break;
	case LATEX_DOCS:
	    DumpInfoLatex(Fout);
	    break;
	case UNIX_MAN_PAGES:
	    DumpInfoManPages(Fout);
	    break;
	case PLAIN_TEXT_DOCS:
	default:
	    DumpInfoText(Fout);
	    break;
    }

    fclose(Fout);

    exit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Validates input line during a comment parsing. Any Line that has non space *
* characters between the first and the last non space characters is valid.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To validate.		                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if valid, FALSE otherwise                                *
*****************************************************************************/
static int ValidateLine(char *Line)
{
    char *p;
    int Count = 0;

    for (p = Line; *p != 0; p++)
	if (!isspace(*p))
	    Count++;
    
    return Count > 2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the DESCRIPTION portion of a function comment.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   char **:       NULL terminated array of strings. No processing is done   *
*                  the read lines.					     *
*****************************************************************************/
static char **GetFuncDescription(FILE *Fin, int *LineNum)
{
    char *Desc[MAX_DESCRIPTION], Line[LINE_LEN_LONG];
    int LCount = 0;

    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	(*LineNum)++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (strncmp(Line, "* PARAMETERS:", 12) == 0) {
	    int i;
	    char **RetVal;

	    RetVal = (char **) IritMalloc((LCount + 1) * sizeof(char *));
	    for (i = 0; i < LCount; i++)
		RetVal[i] = Desc[i];
	    for (i = LCount - 1; i >= 0; i--)
		if (ValidateLine(RetVal[i])) {
		    i++;
		    break;
		}

	    if (i < 0) {
	        fprintf(stderr, "  Ln %d: No function description found\n",
			*LineNum);
		RetVal[0] = NULL;
	    }
	    else
	        RetVal[i] = NULL;
	    return RetVal;
	}
	else if (strstr(Line, "*/") != NULL) {
	    fprintf(stderr,
		    "  Ln %d: Premature end of comment in file description\n",
		    *LineNum);
	    return NULL;
	}
	else {
	    if (LCount >= MAX_DESCRIPTION) {
		fprintf(stderr,
			"  Ln %d: Maximum lines for description (%d) reached\n",
			*LineNum, LCount);
		return NULL;
	    }
		
	    Desc[LCount++] = StrdupTabToSpaces(Line);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the PARAMETERS portion of a function comment.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   LineNum:       To get/update line number in file.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   ParamStruct **:       NULL terminated array of ParamStruct's.	     *
*****************************************************************************/
static ParamStruct **GetFuncParameters(FILE *Fin, int *LineNum)
{
    ParamStruct *Params[MAX_PARAMETERS];
    char Line[LINE_LEN_LONG], *ParamDesc[MAX_PARAM_DESCRIPTION];
    int PDescCount = 0,
	FirstInvalid = TRUE,
	PCount = 0;

    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	(*LineNum)++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (!ValidateLine(Line)) {
	    if (FirstInvalid) {
		FirstInvalid = FALSE;
		continue;
	    }
	    else {
		fprintf(stderr,
			"  Ln %d: Empty line in middle of parameter\n",
			*LineNum);
		return NULL;
	    }
	}

	if (strstr(Line, "*/") != NULL) {
	    fprintf(stderr,
		    "  Ln %d: Premature end of comment in parameters\n",
		    *LineNum);
	    return NULL;
	}
	else {
	    if (PCount - 1 >= MAX_PARAMETERS) {
		fprintf(stderr,
			"  Ln %d: Maximum parameters (%d) reached\n",
			*LineNum, PCount);
		return NULL;
	    }

	    if ((strchr(Line, ':') != NULL ||
		 strncmp(FilterFirstNonSpace(Line, 1), "None", 4) == 0 ||
		 strncmp(Line, "* RETURN VALUE:", 14) == 0) &&
		PDescCount != 0) {
		int i;
		char *p, *q;
		ParamStruct
		    *Param = (ParamStruct *) IritMalloc(sizeof(ParamStruct));

		/* We have a previous variable to place. */
		Params[PCount++] = Param;

		if ((p = strchr(ParamDesc[0], ':')) == NULL &&
		    strncmp(FilterFirstNonSpace(ParamDesc[0], 1), "None", 4) != 0) {
		    fprintf(stderr,
			    "  Ln %d: Colon, ':', was expected for parameter\n",
			    *LineNum);
		    return NULL;
		}
		Param -> Description = (char **)
		    IritMalloc((PDescCount + 1) * sizeof(char *));

		if (strncmp(FilterFirstNonSpace(ParamDesc[0], 1), "None", 4) == 0) {
		    Param -> Description = NULL;
		    Param -> ParamName = NULL;
		}
		else {
		    Param -> Description[0] = StrdupTabToSpaces(p + 1);
		    for (i = 1; i < PDescCount; i++)
			Param -> Description[i] = ParamDesc[i];
		    Param -> Description[PDescCount] = NULL;

		    for (p = &ParamDesc[0][1]; *p != 0 && isspace(*p); p++);
		    if ((q = strchr(ParamDesc[0], ':')) != NULL) {
			for (q = q + 1; isspace(*q); q++);
			if (q - ParamDesc[0] > 70) {
			    fprintf(stderr,
				    "  Ln %d: Empty parameter description\n",
				    *LineNum);
			}
		    }
		    movmem(p, ParamDesc[0], strlen(p) + 1);
		    p = strchr(ParamDesc[0], ':');
		    *p = 0;
		    Param -> ParamName = ParamDesc[0];
		    PDescCount = 0;
		}
	    }
	    else if (strncmp(FilterFirstNonSpace(Line, 1), "none", 4) == 0) {
	        fprintf(stderr,
			"  Ln %d: use 'None' for no parameter values, found 'none'\n",
			*LineNum);
		return NULL;
	    }

	    if (strncmp(Line, "* RETURN VALUE:", 14) == 0) {
		int i;
		ParamStruct **RetVal;

		if (PCount == 0) {
		    /* Nothing specified for parameters, not even 'None'. */
		    fprintf(stderr,
			 "  Ln %d: expected 'None' for no parameter values.\n",
			 *LineNum);
		    return NULL;
		}

		RetVal = (ParamStruct **)
			IritMalloc((PCount + 1) * sizeof(ParamStruct *));
		for (i = 0; i < PCount; i++)
		    RetVal[i] = Params[i];
		RetVal[PCount] = NULL;
		return RetVal;
	    }

	    ParamDesc[PDescCount++] = StrdupTabToSpaces(Line);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the RETURN VALUE portion of a function comment, if any.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   LineNum:       To get/update line number in file.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   ParamStruct *:   A ParamStruct, or NULL if no return value.		     *
*****************************************************************************/
static ParamStruct *GetFuncRetVal(FILE *Fin, int *LineNum)
{
    char Line[LINE_LEN_LONG], *ParamDesc[MAX_PARAM_DESCRIPTION];
    int PDescCount = 0,
	FirstInvalid = TRUE;

    if (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	char *p;
	ParamStruct
	    *Param = (ParamStruct *) IritMalloc(sizeof(ParamStruct));

	(*LineNum)++;

	if (!ValidateLine(Line)) {
	    fprintf(stderr,
		    "  Ln %d: Empty line in middle of return value\n",
		    *LineNum);
	    return NULL;
	}

	if ((p = strrchr(Line, ':')) == NULL &&
	    strstr(Line, "NoValue") == NULL &&
	    strstr(Line, "void") == NULL) {
	    if (strstr(Line, "none") != NULL ||
		strstr(Line, "None") != NULL) {
	        fprintf(stderr,
		    "  Ln %d: use 'void' to denote no return value, found 'none\'\n",
		    *LineNum);
		return NULL;
	    }
	    fprintf(stderr,
		    "  Ln %d: Colon, ':', was expected for returned value\n",
		    *LineNum);
	    return NULL;
	}
	if ((p = strrchr(Line, ':')) != NULL) {
	    for (p = p + 1; isspace(*p); p++);
	    if (p - Line > 70) {
	        fprintf(stderr, "  Ln %d: Empty return value description\n",
			*LineNum);
	    }
	}

	ParamDesc[PDescCount++] = StrdupTabToSpaces(Line);

	while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	    (*LineNum)++;
	    if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	        continue;

	    if (!ValidateLine(Line)) {
		if (FirstInvalid) {
		    FirstInvalid = FALSE;
		    continue;
		}
		else {
		    fprintf(stderr,
			    "  Ln %d: Empty line in middle of return value\n",
			    *LineNum);
		    return NULL;
		}
	    }

	    if ((GlblFuncNoSeeAlso = strncmp(Line, "* KEYWORDS:", 10) == 0) ||
	        strncmp(Line, "* SEE ALSO:", 10) == 0) {
		int i;
		ParamStruct
		    *Param = (ParamStruct *) IritMalloc(sizeof(ParamStruct));

		Param -> Description = (char **)
		    IritMalloc((PDescCount + 1) * sizeof(char*));
		for (i = 0; i < PDescCount; i++)
		    Param -> Description[i] = ParamDesc[i];
		Param -> Description[PDescCount] = NULL;
		return Param;
	    }
	    else if (strstr(Line, "*/") != NULL) {
		fprintf(stderr,
			"  Ln %d: Premature end of comment in return value\n",
			*LineNum);
		fprintf(stderr,
			"FATAL: cannot recover from earlier errors, quiting\n");
		exit(1);
	    }
	    else
		ParamDesc[PDescCount++] = StrdupTabToSpaces(Line);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the KEYWORDS portion of a function comment, if any		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   LineNum:       To get/update line number in file.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char **:       NULL terminated array of strings, or NULL if none.	     *
*****************************************************************************/
static char **GetFuncKeywords(FILE *Fin, int *LineNum)
{
    char *Keywords[MAX_KEYWORDS];
    char Line[LINE_LEN_LONG], CrntKeyword[LINE_LEN_LONG];
    int KCount = 0;

    CrntKeyword[0] = 0;
    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	(*LineNum)++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (strncmp(Line, "************", 10) == 0 ||
	    strstr(Line, "*/") != NULL ||
	    !ValidateLine(Line)) {
	    int i;
	    char **RetVal;

	    if (!IRT_STR_ZERO_LEN(CrntKeyword)) {
		char *p;

		for (p = CrntKeyword; *p != 0 && isspace(*p); p++);
		Keywords[KCount++] = StrdupTabToSpaces(p);
		CrntKeyword[0] = 0;
	    }

	    RetVal = (char **) IritMalloc((KCount + 1) * sizeof(char *));
	    for (i = 0; i < KCount; i++)
		RetVal[i] = Keywords[i];
	    RetVal[KCount] = NULL;
	    return RetVal;
	}
	else {
	    char *p, *q;

	    FilterEOLMV(Line, 0);

	    q = &Line[4];

	    do {
		p = strchr(q, ',');

		if (p)
		    *p++ = 0;

		if (!IRT_STR_ZERO_LEN(CrntKeyword))
		    strcat(CrntKeyword, " ");
		strcat(CrntKeyword, q);
		if (p != NULL) {
		    for (q = CrntKeyword; *q != 0 && isspace(*q); q++);
		    Keywords[KCount++] = StrdupTabToSpaces(q);
		    CrntKeyword[0] = 0;
		}

		q = p;
	    }
	    while (p != NULL);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the SEE ALSO portion of a function comment, if any		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   LineNum:       To get/update line number in file.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char **:       NULL terminated array of strings, or NULL if none.	     *
*****************************************************************************/
static char **GetFuncSeeAlso(FILE *Fin, int *LineNum)
{
    char *SeeAlsos[MAX_KEYWORDS];
    char Line[LINE_LEN_LONG];
    int SACount = 0;

    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	(*LineNum)++;

	/* Skip preprocessor's commands and empty lines. */
	if (Line[0] == '#' || !ValidateLine(Line))
	    continue;

	if (strncmp(Line, "* KEYWORDS:", 10) == 0) {
	    int i;
	    char **RetVal;

	    RetVal = (char **) IritMalloc((SACount + 1) * sizeof(char *));
	    for (i = 0; i < SACount; i++)
		RetVal[i] = SeeAlsos[i];
	    RetVal[SACount] = NULL;
	    return RetVal;
	}
	else {
	    char *p,
		*q = FilterEOLMV(Line, 4);

	    do {
		if (p = strchr(q, ',')) {
		    *p++ = 0;
		    while (isspace(*p))
		        p++;
		}
		else if (p = strchr(q, ' ')) {
		    *p = 0;
		    p = NULL;
		}

		SeeAlsos[SACount++] = IritStrdup(q);

		q = p;
	    }
	    while (p != NULL);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Read in the PROTOTYPE portion of a function.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   LineNum:       To get/update line number in file.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char **:       NULL terminated array of strings. 			     *
*****************************************************************************/
static char **GetFuncPrototype(FILE *Fin, int *LineNum)
{
    char *Prototype[MAX_KEYWORDS];
    char Line[LINE_LEN_LONG];
    int PCount = 0;

    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	(*LineNum)++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (strstr(Line, "{") != NULL) {
	    int i;
	    char **RetVal;

	    RetVal = (char **) IritMalloc((PCount + 1) * sizeof(char *));
	    for (i = 0; i < PCount; i++)
		RetVal[i] = Prototype[i];
	    RetVal[PCount] = NULL;
	    return RetVal;
	}
	else {
	    Prototype[PCount++] = StrdupTabToSpaces(Line);
	}
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Reads input file, process comment of external functions, and save them as  *
* a linked list to be sorted out.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably a c source file.              *
*   OutputDocType: Type of output - tex, doc, etc.  	                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessFiles(FILE *Fin, char *FileName, int MoreInfo)
{
    char Line[LINE_LEN_LONG];
    int LineNum = 0;

    /* Skip file header comments: */
    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	LineNum++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (strstr(Line, "/*") == NULL)
	    break;
    }

    while (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL) {
	LineNum++;
	if (Line[0] == '#')		    /* Skip preprocessor's commands. */
	    continue;

	if (strncmp(Line, "/*********", 10) == 0) {
	    FunctionStruct *Func;

	    /* We have a beginning of a comment. */
	    if (fgets(Line, LINE_LEN_LONG - 1, Fin) != NULL &&
		strncmp(Line, "* DESCRIPTION:", 14) != 0) {
		LineNum++;
		if (strncmp(Line, "* AUXILIARY:", 12) != 0)
		    fprintf(stderr, "  Ln %d: Unrecognized comment format\n",
			    LineNum);
		continue;
	    }
	    LineNum++;

	    /* If it is a static function - ignore. */
	    if (strchr(&Line[strlen(Line) - 3], 'M') == NULL)
		continue;

	    /* We found a comment of an external function - get its data. */
	    Func = (FunctionStruct *) IritMalloc(sizeof(FunctionStruct));
	    if ((Func -> Description = GetFuncDescription(Fin, &LineNum)) &&
		(Func -> Parameters = GetFuncParameters(Fin, &LineNum)) &&
		(Func -> RetVal = GetFuncRetVal(Fin, &LineNum)) &&
		(GlblFuncNoSeeAlso ||
		 (Func -> SeeAlsos = GetFuncSeeAlso(Fin, &LineNum))) &&
		(Func -> Keywords = GetFuncKeywords(Fin, &LineNum)) &&
		(Func -> Prototype = GetFuncPrototype(Fin, &LineNum))) {
		if (GlblFuncNoSeeAlso)
		    Func -> SeeAlsos = NULL;

		if (strncmp(FileName, "../", 3) == 0)
		    Func -> FileName = IritStrdup(&FileName[3]);
		else
		    Func -> FileName = IritStrdup(FileName);
		Func -> LineNumber = LineNum - 1;
		if (Func -> Parameters[0] -> ParamName == NULL) {
		    /* No parameters. */
		    IritFree(Func -> Parameters[0]);
		    Func -> Parameters = NULL;
		}
		if (MoreInfo)
		    fprintf(stderr, "  Ln %d: %s\n",
			    LineNum - 1, *(Func -> Keywords));
		Func -> Pnext = GlblFuncList;
		GlblFuncList = Func;
		VerifyParamConsistency(Func);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Parses the function definition in the file we are currently reading      *
* while comparing it to the information found in the Func structure.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Func:      A function description to compare to the function header.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void VerifyParamConsistency(FunctionStruct *Func)
{
    int i, j,
	NumProtoLines = 0,
	ProtoLn = 0,
	ParamNum = 0;
    char *p, *q, *p0, *RetVal, *Params[MAX_PARAMETERS], Line[LINE_LEN],
	FuncName[LINE_LEN], Proto[MAX_PARAMETERS][LINE_LEN];

    for (i = 0; Func -> Prototype[i] != NULL; i++) {
	strcpy(Proto[i], Func -> Prototype[i]);
	NumProtoLines++;
    }
    if (NumProtoLines == 0) {
	fprintf(stderr,
		"  Ln %d: Empty prototype ignored, function %s (wrongly placed '{'!?)\n",
		Func -> LineNumber, *(Func -> Keywords));
	return;
    }

    if (strlen(Proto[ProtoLn]) < 3) {
	fprintf(stderr,
		"  Ln %d: Forbidden empty line before function declaration, function %s\n",
		Func -> LineNumber, *(Func -> Keywords));
	return;
    }

    /* Skip static declarations, if any. */
    if (strncmp(Proto[ProtoLn], "static", 6) == 0)
        p = &Proto[ProtoLn][6];
    else
        p = Proto[ProtoLn];
    while (isspace(p[0]))
        p++;

    /* Fetch the return value. */
    RetVal = p;

    /* Fetch the function name. */
    while (ProtoLn < NumProtoLines && (q = strchr(p, '(')) == NULL) {
	/* Remove new line characters. */
        q = &p[strlen(p) - 1];
	while (isspace(*q)) q--;
	*++q = 0;

        p = Proto[++ProtoLn];
    }
    p0 = p; /* Start line of function name. */

    if (ProtoLn >= NumProtoLines) {
	fprintf(stderr,
		"  Ln %d: Expecting beginning of function definition, function %s\n",
		Func -> LineNumber, *(Func -> Keywords));
	return;
    }
    p = q + 1;
    *q-- = 0;			  /* Seperate function name from parameters. */
    while (q != p0 && !isspace(q[0]))
	q--;
    if (q != p0)
	q++;
    while (q[0] == '*' || q[0] == '&')
        q++;

    strncpy(FuncName, q, LINE_LEN);
    *q = 0;

    while (q > p0 && isspace(q[-1]))
        *--q = 0;		/* Seperate return value from function name. */

    if (strcmp(*(Func -> Keywords), FuncName) != 0) {
	fprintf(stderr,
		"  Ln %d: Inconsistent 1st keyword - expect function name, function \"%s\"\n    Found \"%s\".\n",
		Func -> LineNumber, FuncName,
		*(Func -> Keywords));
	return;
    }

    /* Fetch the parameters */
    while (TRUE) {
	while (p[0] != ',' && p[0] != ')' && p[0] != 0)
	    p++;
	if (p[0] == 0 || isspace(p[0])) {
	    if (++ProtoLn >= NumProtoLines) {
		fprintf(stderr,
			"  Ln %d: Inconsistent function prototype declaration, function %s\n",
			Func -> LineNumber, FuncName);
		return;
	    }

	    p = Proto[ProtoLn];
	}
	else {
	    q = p - 1;
	    while (TRUE) {
		for ( ;
		     isalpha(q[0]) ||
		     isdigit(q[0]) ||
		     q[0] == '_' ||
		     q[0] == '.';
		     q--);
		if (q[0] == ']') {
		    while (q[0] != '[')
		        q--;
		    *q-- = 0;
		}
		else
		    break;
	    }

	    Params[ParamNum++] = &q[1];

	    if (p[0] == ')') {
		p[0] = 0;
		break; /* Last parameter found. */
	    }
	    else
	        *p++ = 0;
	}
    }

    /* Check for no parameters (only "void"). */
    if (ParamNum == 1 &&
	(IRT_STR_ZERO_LEN(Params[0]) || strcmp(Params[0], "void") == 0))
        ParamNum = 0;

    /* Do not consider "..." as a real parameter. */
    if (ParamNum > 0 && strcmp(Params[ParamNum - 1], "...") == 0)
        ParamNum--;

    /* Time to verify what we found. */
    if (ParamNum == 0) {
        if (Func -> Parameters != NULL)
	    fprintf(stderr,
		    "  Ln %d: Inconsistent # of parameters, function %s, expecting no parameters\n",
		    Func -> LineNumber, FuncName);
    }
    else {
        if (Func -> Parameters == NULL)
	    fprintf(stderr,
		    "  Ln %d: Inconsistent # of parameters, function %s, found no parameters\n",
		    Func -> LineNumber, FuncName);
	else {
	    strcpy(Line, Func -> Parameters[0] -> ParamName);
	    p = strtok(Line, ",");
	    for (i = j = 0; i < ParamNum; i++) {
		if (strcmp(Params[i], "...") == 0)  /* Ignore unspec. funcs. */
		    break;
	        if (p == NULL || strcmp(p, Params[i]) != 0)
		    fprintf(stderr,
			    "  Ln %d: Inconsistent parameter %d, function %s\n    Expected \"%s\", found \"%s\"\n",
			    Func -> LineNumber, i + 1, FuncName,
			    p, Params[i]);

		if ((p = strtok(NULL, ",")) != NULL) {
		    while (isspace(p[0]))
		        p++;
		}
		if (i < ParamNum - 1 && IRT_STR_NULL_ZERO_LEN(p)) {
		    if (Func -> Parameters[++j] == NULL) {
		        fprintf(stderr,
				"  Ln %d: Inconsistent # of parameters, function %s\n",
				Func -> LineNumber, FuncName);
			break;
		    }
		    strcpy(Line, Func -> Parameters[j] -> ParamName);
	    	    p = strtok(Line, ",");
		}
	    }
	    if (Func -> Parameters[j+1] != NULL) {
		fprintf(stderr,
			"  Ln %d: Inconsistent # of parameters, function %s\n",
			Func -> LineNumber, FuncName);
	    }
	}
    }

    if (Func -> RetVal -> Description == NULL ||
	strncmp(&Func -> RetVal -> Description[0][4], "NoValue", 6) == 0) {
	if (RetVal != NULL && strlen(RetVal) > 0)
	    fprintf(stderr,
		    "  Ln %d: No return value for a con/destructor, function %s\n",
		    Func -> LineNumber, FuncName);
    }
    else {
	strcpy(Line, &Func -> RetVal -> Description[0][4]);
	if ((p = strrchr(Line, ':')) != NULL)
	    *p = 0;
	else if (strncmp(Line, "void", 4) == 0)
	    Line[4] = 0;
	else
	    fprintf(stderr,
		    "  Ln %d: Expecting a ':' after return value, function %s\n",
		    Func -> LineNumber, FuncName);

	if (strcmp(Line, RetVal) != 0)
	    fprintf(stderr,
		    "  Ln %d: Inconsistent return value, function %s\n    Expected \"%s\", found \"%s\"\n",
		    Func -> LineNumber, FuncName, Line, RetVal);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A comparison function between two func structures by name.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   VPoly1, VPoly2:  Two pointers to structures.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two polygons.            *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int FuncQSortComp(VoidPtr Func1, VoidPtr Func2)
#else
static int FuncQSortComp(const VoidPtr Func1, const VoidPtr Func2)
#endif /* ultrix && mips (no const support) */
{
    return strcmp(*((*((FunctionStruct **) Func1)) -> Keywords),
		  *((*((FunctionStruct **) Func2)) -> Keywords));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts all tabs in input line to spaces and returns a duplicated string. *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To filter out tabs.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     Line but with no tabs. A pointer to a dynamic space. 	     *
*****************************************************************************/
static char *StrdupTabToSpaces(char*Line)
{
    return IritStrdup(ConvertTabsToSpaces(Line));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts all tabs in input line to spaces.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To filter out tabs.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     Line but with no tabs. A pointer to a static local space.    *
*****************************************************************************/
static char *ConvertTabsToSpaces(char*Line)
{
    IRIT_STATIC_DATA char NewLine[LINE_LEN_LONG];
    int i;
    char *p = Line;

    for (i = 0; *p != 0; p++) {
	if (*p == 0x09)
	    do {
		NewLine[i++] = ' ';
	    }
	    while (i % 8 != 0);
	else 
	    NewLine[i++] = *p;
    }
    NewLine[i] = 0;

    return NewLine;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Filters out CR/LF, M or V and spaces at the end of the line, in place	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To filter out its end.                                        *
*   Skip:      How many characters to skip from the beginning.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:    Line                                                          *
*****************************************************************************/
static char *FilterEOLMV(char *Line, int Skip)
{
    char *p;

    for (p = Line + strlen(Line) - 1;
	 *p <= ' ' && *p != 'M' && *p != 'V' && p >= Line;
	 p--);
    if (*p == 'M' || *p == 'V')
        p--;

    for ( ; isspace(*p) && p >= Line; p--);
    
    *++p = 0;

    return ((int) strlen(Line) > Skip) ? &Line[Skip] : "";
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Returns the first non space character in line after the first *.     	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To find first interesting char.                               *
*   i:	       Numbner of characters to skip from beginning for Line.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:    First interesting character in Line.                          *
*****************************************************************************/
static char *FilterFirstNonSpace(char *Line, int i)
{
    char *p;

    for (p = Line + i; *p != 0 && isspace(*p); p++);
    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Filters out the CR/LF from the end of the line, in place.            	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To filter out its end.                                        *
*   Skip:      How many characters to skip from the beginning.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:    Line                                                          *
*****************************************************************************/
static char *FilterCRLF(char *Line, int Skip)
{
    char *p;

    for (p = Line + strlen(Line) - 1; *p < ' '; p--);
    *++p = 0;

    return ((int) strlen(Line) > Skip) ? &Line[Skip] : "";
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   returns the file name, ignoring the full path.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Path:    A full path including directory.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     The file name only.                                          *
*****************************************************************************/
static char *FileNameFromPath(char *Path)
{
    char *p;

    if ((p = strrchr(Path, '\\')) == NULL)
	p = strrchr(Path, '/');

    if (p == NULL)
	p = Path;
    else
        p++;

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints the line out, escaping the back slash special character.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:      To print.                                                     *
*   f:         File where output should go to.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PutString(char *Line, FILE *f)
{
    char *p = Line;

    while (*p) {
        if (*p == '\\')
	    fputc(*p, f);
	fputc(*p++, f);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps out the functions in the regular text format.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:      Where output goes to.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpInfoText(FILE *Fout)
{
    int i, j;
    FunctionStruct *Func, **SortFunc;

    /* Sort the functions by name. */
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++);
    SortFunc = (FunctionStruct **)
	IritMalloc((i + 1) * sizeof(FunctionStruct *));
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++)
	SortFunc[i] = Func;
    qsort(SortFunc, i, sizeof(FunctionStruct *), FuncQSortComp);

    for (j = 0; j < i; j++) {
	int k, l;
	char **Data;
	ParamStruct *Param, **Params;

	Func = SortFunc[j];
	fprintf(Fout, "--------------------\n%s  (%s:%d):\n\n",
		Func -> Keywords[0], FileNameFromPath(Func -> FileName),
		Func -> LineNumber);

	fprintf(Fout, "Prototype:\n");
	Data = Func -> Prototype;
	for (k = 0; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 0));

	fprintf(Fout, "\n Description:\n");
	Data = Func -> Description;
	for (k = 0; Data[k] != NULL; k++) {
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 2));
	}

	fprintf(Fout, "\n Parameters:\n");
	if (Func -> Parameters) {
	    for (Params = Func -> Parameters, l = 0; Params[l] != NULL; l++) {
		Param = Params[l];
		fprintf(Fout, "  %s:", Param -> ParamName);
		Data = Param -> Description;
		for (k = 0; Data[k] != NULL; k++)
		    fprintf(Fout, "  %s\n",
			    FilterEOLMV(Data[k], k == 0 ? 0 : 2));
	    }
	}
	else
	    fprintf(Fout, "  None:\n");

	fprintf(Fout, "\n Returned Value:\n");
	Param = Func -> RetVal;
	Data = Param -> Description;
	for (k = 0; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 4));

	if (Func -> SeeAlsos) {
	    fprintf(Fout, "\n See Also:\n");
	    Data = Func -> SeeAlsos;
	    for (k = 0; Data[k] != NULL; k++)
		fprintf(Fout, "  %s\n", Data[k]);
	}

	fprintf(Fout, "\n Keywords:\n");
	Data = Func -> Keywords;
	for (k = 1; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 0));

	fprintf(Fout, "\n\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps out the functions in the regular text format.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:      Where output goes to.                                         *
*   Fout:      Where output of index goes to.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpInfoHTML(FILE *Fout, FILE *FoutIndex)
{
    int i, j,
        IndentNesting = 0,
	InVerbatim = FALSE;
    FunctionStruct *Func, **SortFunc;

    /* Sort the functions by name. */
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++);
    SortFunc = (FunctionStruct **)
	IritMalloc((i + 1) * sizeof(FunctionStruct *));
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++)
	SortFunc[i] = Func;
    qsort(SortFunc, i, sizeof(FunctionStruct *), FuncQSortComp);

    fprintf(FoutIndex, "<OL>\n");

    for (j = 0; j < i; j++) {
	int k, l;
	char **Data;
	ParamStruct *Param, **Params;

	Func = SortFunc[j];
	fprintf(FoutIndex, "<LI> <A HREF=\"%s#%s\"> <B> %s (%s:%d) </B> </A> <B><PRE>",
		GlblFoutName, Func -> Keywords[0],
		Func -> Keywords[0], FileNameFromPath(Func -> FileName),
		Func -> LineNumber);
	Data = Func -> Prototype;
	for (k = 0; Data[k] != NULL; k++) {
	    fprintf(FoutIndex, "    %s\n", FilterEOLMV(Data[k], 0));
	}
	fprintf(FoutIndex, "</PRE></B>\n");

	fprintf(Fout, "<HR><H2><A NAME=\"%s\"> %s </A></H2>  (%s:%d)",
		Func -> Keywords[0], Func -> Keywords[0],
		FileNameFromPath(Func -> FileName), Func -> LineNumber);

	fprintf(Fout, "<BR><BR><B>Prototype:</B><BR><BR><B><PRE>");
	Data = Func -> Prototype;
	for (k = 0; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 0));
	fprintf(Fout, "</PRE></B>");

	fprintf(Fout, "<BR><BR><B>Description:</B><BR><BR>");
	Data = Func -> Description;
	for (k = 0; Data[k] != NULL; k++) {
	    char
	        *p = FilterCRLF(Data[k], 0);

	    if (p[strlen(p) - 1] == 'V' && !InVerbatim) {
		fprintf(Fout, "\n<BR><PRE>\n");
		InVerbatim = TRUE;
	    }
	    else if (p[strlen(p) - 1] == 'M' && InVerbatim) {
		fprintf(Fout, "</PRE><BR>\n");
		InVerbatim = FALSE;
	    }

	    fprintf(Fout, "%s\n", FilterEOLMV(p, 2));
	}

	fprintf(Fout, "<BR><BR><B>Parameters:</B><BR><BR><TABLE BORDER=7 CELLPADDING=1>\n");
	if (Func -> Parameters) {
	    for (Params = Func -> Parameters, l = 0; Params[l] != NULL; l++) {
		Param = Params[l];
		fprintf(Fout, "<TR><TH align=left> %s: <TH align=left>",
			Param -> ParamName);
		Data = Param -> Description;
		for (k = 0; Data[k] != NULL; k++)
		    fprintf(Fout, " %s\n",
			    FilterEOLMV(Data[k], 2));
	    }
	}
	else
	    fprintf(Fout, "<TR> None\n");
	fprintf(Fout, "</TABLE>\n");

	fprintf(Fout, "<BR><BR><B>Returned Value:</B><BR><BR><TABLE BORDER=7 CELLPADDING=1>\n<TR><TH>");
	Param = Func -> RetVal;
	Data = Param -> Description;
	for (k = 0; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 4));
	fprintf(Fout, "</TABLE>\n");

	if (Func -> SeeAlsos) {
	    fprintf(Fout, "<BR><BR><B>See Also:</B><BR><BR>");
	    Data = Func -> SeeAlsos;
	    for (k = 0; Data[k] != NULL; k++)
		fprintf(Fout, "  %s\n", Data[k]);
	}

	fprintf(Fout, "<BR><BR><B>Keywords:</B><BR><BR>");
	Data = Func -> Keywords;
	for (k = 1; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 0));

	fprintf(Fout, "<BR><BR>");
    }
    fprintf(FoutIndex, "</OL>\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps out the functions in the regular text format.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:      Where output goes to.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpInfoManPages(FILE *Fout)
{
    int i, j;
    FunctionStruct *Func, **SortFunc;
    FILE *f;

    /* Sort the functions by name. */
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++);
    SortFunc = (FunctionStruct **)
	IritMalloc((i + 1) * sizeof(FunctionStruct *));
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++)
	SortFunc[i] = Func;
    qsort(SortFunc, i, sizeof(FunctionStruct *), FuncQSortComp);

    /* Make sure the man directory exists. */
    mkdir("man", 0755);
    mkdir("man/man3", 0755);

    for (j = 0; j < i; j++) {
	int k, l;
	char **Data, FileName[LINE_LEN_LONG];
	ParamStruct *Param, **Params;

	Func = SortFunc[j];
	sprintf(FileName, "man/man3/%s.3", Func -> Keywords[0]);
	if ((f = fopen(FileName, "w")) != NULL) {
	    fprintf(f, ".TH %s 3 \"IRIT %s\" \n",
		    Func -> Keywords[0], IRIT_VERSION);

	    fprintf(f, ".SH NAME\n%s()\n.SH SYNOPSIS\n.nf\n.ft B\n%s:%d\n\n",
		    Func -> Keywords[0], FileNameFromPath(Func -> FileName),
		    Func -> LineNumber);
	    fprintf(f, ".nf\n.ft B\n");
	    Data = Func -> Prototype;
	    for (k = 0; Data[k] != NULL; k++)
		fprintf(f, "%s\n", FilterEOLMV(Data[k], 0));

	    fprintf(f, ".SH DESCRIPTION\n");
	    Data = Func -> Description;
	    for (k = 0; Data[k] != NULL; k++) {
		char
		    *p = FilterCRLF(Data[k], 0);
		int IsVerbatim = p[strlen(p) - 1] == 'V';

		p = FilterEOLMV(Data[k], k == 0 ? 0 : 2);
		if (*p == '*')
		    p++;

		if (IsVerbatim || isdigit(*FilterFirstNonSpace(p, 0))) {
		    fprintf(f, " ");
		    PutString(p, f);
		    fprintf(f, "\n");
		}
		else {
		    PutString(FilterFirstNonSpace(p, 0), f);
		    fprintf(f, "\n");
		}
	    }

	    fprintf(f, ".SH PARAMETERS:\n");
	    if (Func -> Parameters) {
		for (Params = Func -> Parameters, l = 0; Params[l] != NULL; l++) {
		    Param = Params[l];
		    fprintf(f, "\n%s: ", Param -> ParamName);
		    Data = Param -> Description;
		    for (k = 0; Data[k] != NULL; k++) {
			char
			    *p = FilterEOLMV(Data[k], k == 0 ? 0 : 2);

			fprintf(f, "%s\n", FilterFirstNonSpace(p, 0));
		    }
		}
	    }
	    else
		fprintf(f, "None:\n");

	    fprintf(f, ".SH FUNCTION RETURN VALUE\n");
	    Param = Func -> RetVal;
	    Data = Param -> Description;
	    for (k = 0; Data[k] != NULL; k++)
	        fprintf(f, "%s\n",
			FilterFirstNonSpace(FilterEOLMV(Data[k], 4), 0));

	    if (Func -> SeeAlsos) {
		fprintf(f, ".SH SEE ALSO\n");
		Data = Func -> SeeAlsos;
		for (k = 0; Data[k] != NULL; k++)
		    fprintf(f, "%s,\n", Data[k]);
	    }

	    fprintf(f, ".LP\n.SH ORIGIN\n%s, Technion, IIT\n", IRIT_COPYRIGHT);
	    fclose(f);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Prints out a string, after escaping all Latex's spacial characters.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:     Where output goes to.                                          *
*   Str:      String to output in Latex acceptable format.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PutLatexString(FILE *Fout, char *Str)
{
    int j;

    for (j = 0; Str[j] != 0; j++) {
	switch (Str[j]) {
	    case '^':
		fprintf(Fout, "\\verb+%c+", Str[j]);
		break;
	    case '\\':
		fprintf(Fout, "$\\backslash$");
		break;
	    case '&':
	    case '#':
	    case '_':
	    case '$':
	    case '%':
	    case '{':
	    case '}':
		fprintf(Fout, "\\%c", Str[j]);
		break;
	    case '>':
	    case '<':
	    case '|':
		fprintf(Fout, "$%c$", Str[j]);
		break;
	    default:
		fputc(Str[j], Fout);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps out the desscription of a function for Latex.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:      Where output goes to.                                         *
*   Desc:      A NULL terminated array of strings.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDescriptionForLatex(FILE *Fout, char **Desc)
{
    int InVerbatim = FALSE;
    int i,
        IndentNesting = 0,
	Indent[MAX_INDENTATION];

    Indent[IndentNesting] = 0;

    fprintf(Fout, "{\\bf Description:} ");
    for (i = 0; Desc[i] != NULL; i++)
    {
	if (ValidateLine(Desc[i])) {
	    char
		*p = FilterCRLF(Desc[i], 1),
		*p2 = FilterFirstNonSpace(p, 1);

	    /* Make sure we are in the right mode according to 'M' or 'V' in */
	    /* the end of the line and drop this mark once set.		     */
	    if (p[strlen(p) - 1] == 'V' && !InVerbatim) {
		fprintf(Fout, "\\begin{verbatim}\n");
		InVerbatim = TRUE;
	    }
	    else if (p[strlen(p) - 1] == 'M' && InVerbatim) {
		fprintf(Fout, "\\end{verbatim}\n");
		InVerbatim = FALSE;
	    }
	    FilterEOLMV(p, 0);

	    if (!InVerbatim && isdigit(p2[0]) && p2[1] == '.') {
		int Ind = ((int) p2) - ((int) p);

		/* Need to ident this one in. */
		if (Indent[IndentNesting] == Ind) {
		    /* Same list, but a new item. */
		    fprintf(Fout, "\\item ");
		}
		else if (Indent[IndentNesting] < Ind) {
		    /* A new indentation level - add a new list. */
		    fprintf(Fout, "\\begin{list}{}{\\setlength{\\itemindent}{-0.3in}\\setlength{\\leftmargin}{0.3in}}\n");
		    fprintf(Fout, "\\item ");
		    if (IndentNesting + 1 >= MAX_INDENTATION) {
			fprintf(stderr,
				"\\tOutput:  list indentation too large (>%d)\n",
				MAX_INDENTATION);
			return;
		    }
		    Indent[++IndentNesting] = Ind;
		}
		else {
		    while (Indent[IndentNesting] > Ind) {
			/* Close the lists. */
			fprintf(Fout, "\\end{list}\n\\item ");
			IndentNesting--;
		    }
		}
	    }

	    if (InVerbatim) {
		fprintf(Fout, "%s\n", p);
	    }
	    else {
		PutLatexString(Fout, p2);
		fputc('\n', Fout);
	    }
	}
	else {
	    if (InVerbatim) {
		fprintf(Fout, "\\end{verbatim}\n");
		InVerbatim = FALSE;
	    }

	    while (IndentNesting > 0) {
		/* Close all lists. */
		fprintf(Fout, "\\end{list}\n");
		IndentNesting--;
	    }

	    fprintf(Fout, "\\mbox{\\hspace{0.01in}} \\newline ");
	}
    }

    if (InVerbatim) {
	fprintf(Fout, "\\end{verbatim}\n");
	InVerbatim = FALSE;
    }

    /* Make sure no nesting remains. */
    while (IndentNesting > 0) {
	/* Close the lists. */
	fprintf(Fout, "\\end{list}\n");
	IndentNesting--;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps out the functions in the latex format.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fout:      Where output goes to.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpInfoLatex(FILE *Fout)
{
    int i, j;
    FunctionStruct *Func, **SortFunc;

    /* Sort the functions by name. */
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++);
    SortFunc = (FunctionStruct **)
	IritMalloc((i + 1) * sizeof(FunctionStruct *));
    for (Func = GlblFuncList, i = 0; Func != NULL; Func = Func -> Pnext, i++)
	SortFunc[i] = Func;
    qsort(SortFunc, i, sizeof(FunctionStruct *), FuncQSortComp);

    for (j = 0; j < i; j++) {
	int k, l;
	char **Data;
	ParamStruct *Param, **Params;

	Func = SortFunc[j];
	fprintf(Fout, "\\subsection{");
	PutLatexString(Fout, Func -> Keywords[0]);
	Data = Func -> Keywords;
	for (k = 1; Data[k] != NULL; k++)
	    fprintf(Fout, "\\index{%s}\n", FilterEOLMV(Data[k], 0));
	fprintf(Fout, " {\\normalsize (");
	PutLatexString(Fout, FileNameFromPath(Func -> FileName));
	fprintf(Fout, ":%d)}}\n\n", Func -> LineNumber);

	fprintf(Fout, "\\begin{picture}(0, 0)\n");
	for (k = 1; Data[k] != NULL; k++)
	    fprintf(Fout, "    \\put(400, %d){\\fbox{\\tiny %s}}\n",
		    (k - 1) * -15, Data[k]);
	fprintf(Fout, "\\end{picture}\n\n");

	Data = Func -> Prototype;
	fprintf(Fout, "\\begin{verbatim}\n");
	for (k = 0; Data[k] != NULL; k++)
	    fprintf(Fout, "  %s\n", FilterEOLMV(Data[k], 0));
	fprintf(Fout, "\\end{verbatim}\n");

	if (Func -> Parameters) {
	    fprintf(Fout, "\\begin{list}{}{\\setlength{\\itemindent}{-0.3in}\\setlength{\\leftmargin}{0.6in}}\n");
	    for (Params = Func -> Parameters, l = 0; Params[l] != NULL; l++) {
		Param = Params[l];
		fprintf(Fout, "\\item {\\bf ");
		PutLatexString(Fout, Param -> ParamName);
		fprintf(Fout, ":} ");
		Data = Param -> Description;
		for (k = 0; Data[k] != NULL; k++) {
		    PutLatexString(Fout, FilterEOLMV(Data[k],
						     k == 0 ? 0 : 2));
		    fprintf(Fout, "\n");
		}
	    }
	    fprintf(Fout, "\\end{list}\n\n");
	}

	Param = Func -> RetVal;
	Data = Param -> Description;
	if (Data[1] != NULL ||
	    strncmp(&Data[0][4], "void", 4) != 0 ||
	    strncmp(&Data[0][4], "NoValue", 6) != 0) {
	    char
		*p = strchr(Data[0], ':');

	    fprintf(Fout, "\\begin{list}{}{\\setlength{\\itemindent}{-0.3in}\\setlength{\\leftmargin}{0.6in}}\n");
	    fprintf(Fout, "\\item {\\bf Returns:} ");
	    if (p != NULL)
		PutLatexString(Fout, FilterEOLMV(p + 1, 0));
	    else
		PutLatexString(Fout, FilterEOLMV(Data[0], 2));
	    for (k = 1; Data[k] != NULL; k++)
		PutLatexString(Fout, FilterEOLMV(Data[k], 2));
	    fprintf(Fout, "\n\n");
	    fprintf(Fout, "\\end{list}\n\n");
	}

	DumpDescriptionForLatex(Fout, Func -> Description);

	if (Func -> SeeAlsos) {
	    fprintf(Fout, "\n{\\bf See also:} ");
	    Data = Func -> SeeAlsos;
	    for (k = 0; Data[k] != NULL; k++) {
	        PutLatexString(Fout, Data[k]);
		fprintf(Fout, ",\n");
	    }
	}
    }
}
