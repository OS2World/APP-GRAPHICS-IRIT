/*****************************************************************************
*   Documentation processor of IRIT- a 3d solid modeller.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Usage:								     *
*   irithlp [-t] [-l] [-h] [-M] [-o OutFileName] [-z] [InFileName]	     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Nov. 1991   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef OS2GCC
#include <sys/stat.h>
#endif /* OS2GCC */

#include "irit_sm.h"

#define MAX_ARRAY_SIZE	1000

IRIT_STATIC_DATA int
    GlblIndexLevel = 0;
IRIT_STATIC_DATA char
    *GlblFoutName = "irit.html";

typedef enum {
    UNDEFINED_DOC_PROCESS = 0,

    MAN_PAGES_DOCS,
    PLAIN_TEXT_DOCS,
    LATEX_DOCS,
    IRIT_HELP_DOCS,
    HTML_DOCS
} DocProcessType;

struct {
    char *FindString;
    char *ReplaceString;
} GlblStrSubstStruct[] = {
    { "_IRIT_VERSION_", IRIT_VERSION },
    { NULL, NULL }
};

static void ProcessFiles(FILE *Fin,
			 FILE *Fout,
			 FILE *FoutIndex,
			 DocProcessType OutputDocType);
static void FilterLatexMods(char *Line1, char *Line2);
static void FilterLatexToHTMLMods(char *Line1, char *Line2);
static void MacroSubstitute(char *Line);
static char *MakeLabel(char *Str);
static int IsSpaceLine(char *Str);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module - process command line options.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  command line options.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
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
	else if (strcmp(argv[1], "-h") == 0) {
	    OutputDocType = IRIT_HELP_DOCS;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-w") == 0) {
	    OutputDocType = HTML_DOCS;
	    argc--;
	    argv++;
	}
	else if (strcmp(argv[1], "-M") == 0) {
	    OutputDocType = MAN_PAGES_DOCS;
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
	    fprintf(stderr, "Usage: IritHlp [-t] [-l] [-h] [-w] [-M] [-o OutFileName] [-z] [InFileName].\n");
	    exit(0);
	}
	else
	    break;
    }

    if (argc > 1) {
	if ((Fin = fopen(argv[1], "r")) == NULL) {
	    fprintf(stderr, "Failed to open \"%s\".\n", argv[1]);
	    exit(1);
	}
    }

    if (OutputDocType == UNDEFINED_DOC_PROCESS) {
	fprintf(stderr, "One of [-t] [-l] [-h] [-w] [-M] must be specified.\n");
	exit(1);
    }

    if (OutputDocType == MAN_PAGES_DOCS) {
	/* Make sure the man directory exists. */
	mkdir("man", 0755);
	mkdir("man/man6", 0755);
    }

    if (OutputDocType == HTML_DOCS) {
	char Line[IRIT_LINE_LEN];

	sprintf(Line, "%s_index.html", GlblFoutName);
	/* Open the index file as well. */
	if ((FoutIndex = fopen(Line, "w")) == NULL) {
	    fprintf(stderr, "Failed to open \"%s\".\n", Line);
	    exit(1);
	}
    }

    ProcessFiles(Fin, Fout, FoutIndex, OutputDocType);

    fclose(Fin);
    fclose(Fout);
    if (FoutIndex)
	fclose(FoutIndex);

    exit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Reads lines from Fin and dumps them to Fout after the following	     *
* processing take place:						     *
* Allways:						       		     *
*    a. Lines start with ';' are ignored ans skipped (comments).	     *
*    b. Lines start with '#' followed by four ingeters (indentation,         *
*       # elements per line, a Boolean to specify vertical seperator, and    *
*       a Boolean flag on the row (TRUE) or col (FALSE) entries order)	     *
*       signifies an array. All entries, one per line are part of the array  *
*       until another '#' occurs. Each of the method below should do its     *
*       best to create such an array.					     *
*    c. Lines start with '%' are copied verbatim to all output types but     *
*       but latex.							     *
* 1. OutputDocType = MAN_PAGES_DOCS					     *
*    + Lines start with '*' are echoed as is.                                *
*    + Lines start with '+' are ignored.				     *
*    + Lines start with '=' are ignored.				     *
*    + Lines start with '-' are ignored.				     *
*    + Lines start with '@' are ignored, but used to sync on verbatim text.  *
*    + Lines start with '!' are opening a man file and echoed as title.      *
*    + Lines start with '$' are closing a man file.			     *
*    + Lines start with '^' are echoed as is.				     *
*    + Lines start with '&' are echoed as is with titling.  		     *
*    + Other lines are dumped out as is after filtering out of '{\cmd * }'   *
*      latex modifiers.							     *
*    + all lines are dumped out without their first character.		     *
* 2. OutputDocType = PLAIN_TEXT_DOCS                  			     *
*    + Lines start with '*' are echoed as is with no tex related processing. *
*    + Lines start with '=' are ignored.				     *
*    + Lines start with '-' are ignored.				     *
*    + Lines start with '@' are ignored.				     *
*    + Lines start with '!' are echoed as is with underline.		     *
*    + Lines start with '$' are ignored.				     *
*    + Lines start with '^' are echoed as is.				     *
*    + Lines start with '&' are echoed as is with underline.		     *
*    + Other lines are dumped out as is after filtering out of '{\cmd * }'   *
*      latex modifiers.							     *
*    + all lines are dumped out without their first character.		     *
* 3. OutputDocType = LATEX_DOCS						     *
*    + Lines start with '*' are echoed as is with no tex related processing. *
*    + Lines start with '=' are ignored.				     *
*    + Lines start with '-' are echoed as is.				     *
*    + Lines start with '@' are echoed as is.				     *
*    + Lines start with '!' are ignored.				     *
*    + Lines start with '$' are ignored.				     *
*    + Lines start with '^' are ignored.				     *
*    + Lines start with '&' are ignored.				     *
*    + Other lines are dumped out as is.				     *
*    + All lines are dumped out without their first character.		     *
* 4. OutputDocType = IRIT_HELP_DOCS					     *
*    + Lines start with '*' are echoed as is with no tex related processing. *
*    + Lines start with '=' are ignored.				     *
*    + Lines start with '-' are ignored.				     *
*    + Lines start with '@' are ignored.				     *
*    + Lines start with '!' are dumped out as is.			     *
*    + Lines start with '$' causes echoing of the '$' in a single line.      *
*    + Lines start with '^' are echoed as is.				     *
*    + Lines start with '&' are ignored.				     *
*    + Other lines are dumped out as is after filtering out of '{\cmd * }'   *
*      latex modifiers.							     *
*    + All lines are dumped out with their first character as in input.      *
* 5. OutputDocType = HTML_DOCS					             *
*    + Lines start with '*' are echoed as is.                                *
*    + Lines start with '+' are echoed as is to both Fout and FoutIndex.     *
*    + Lines start with '=' are echoed as is to Fout.			     *
*    + Lines start with '-' are echoed as is to Fout.			     *
*    + Lines start with '@' are used to sync on verbatim text and titles.    *
*    + Lines start with '!' are ignored.				     *
*    + Lines start with '$' are ignored.				     *
*    + Lines start with '^' are echoed as is.				     *
*    + Lines start with '&' are ignored.				     *
*    + Other lines are dumped out as is after filtering out of '{\cmd * }'   *
*      latex modifiers.							     *
*    + all lines are dumped out without their first character.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Fin:           File to read from, probably irit.src                      *
*   Fout:          Output file.                                              *
*   FoutIndex:     Output index file (optional).                             *
*   OutputDocType: Type of output - tex, hlp, doc, etc.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessFiles(FILE *Fin,
			 FILE *Fout,
			 FILE *FoutIndex,
			 DocProcessType OutputDocType)
{
    IRIT_STATIC_DATA int
	WasSpace = FALSE;
    int i, j, k,
	InAllowItems = FALSE,
	InVerbatim = FALSE,
	LineNum = 0;
    char Line1[IRIT_LINE_LEN_LONG], Line2[IRIT_LINE_LEN_LONG], *p;
    FILE
	*OrigFout = Fout;

    switch (OutputDocType) {
	case MAN_PAGES_DOCS:
	    Fout = NULL;
	    break;
	default:
	    break;
    }

    while (fgets(Line1, IRIT_LINE_LEN_LONG - 1, Fin) != NULL) {
	LineNum++;
	MacroSubstitute(Line1);
	if (Line1[0] == ';')
	    continue;

	if (Line1[0] == '%') {
	    if (OutputDocType != LATEX_DOCS)
		fprintf(Fout, " ");
	    fprintf(Fout, &Line1[1]);
	    continue;
	}
	    
	if (Line1[0] == '#') {             /* Beginning of array definition. */
#if defined(AMIGA) && defined(__SASC)
	    IRIT_STATIC_DATA char __far ArrayEntries[MAX_ARRAY_SIZE]
	                                            [IRIT_LINE_LEN_LONG];
#else
  	    IRIT_STATIC_DATA char ArrayEntries[MAX_ARRAY_SIZE][IRIT_LINE_LEN_LONG];
#endif
	    int Indentation, NumPerLine, VertSep, NumLines, EntriesRowBased,
		StartLineNum = LineNum,
		NumArrayEntries = 0;

	    if (sscanf(&Line1[1], "%d %d %d %d",
		       &Indentation, &NumPerLine,
		       &VertSep, &EntriesRowBased) != 4) {
		fprintf(stderr, "Four integers expected in line %d\n", LineNum);
		exit(1);
	    }

	    /* Gets all the entries of the array: */
	    while ((p =	fgets(ArrayEntries[NumArrayEntries++],
				       IRIT_LINE_LEN_LONG - 1, Fin)) != NULL &&
		   p[0] != '#' &&
		   NumArrayEntries < MAX_ARRAY_SIZE) {
		p += strlen(p);		      /* Remove the EOL from string. */
		while (*--p < ' ');
		p[1] = 0;

		LineNum++;
            }
	    if (p != NULL)
		LineNum++;
	    NumArrayEntries--;

	    if (p[0] != '#') {
		fprintf(stderr,
			"Array at line %d too large (max %d) or missing '#'.\n",
			StartLineNum, MAX_ARRAY_SIZE);
		exit(1);
	    }

	    if (NumPerLine == 0) {
		fprintf(stderr,
			"Number of items per line is zero at line %d.\n",
			StartLineNum);
		exit(1);
	    }

	    /* Now we have all the array elements. Computes how many lines   */
	    /* need and print the lines in the format this documentation is  */
	    /* suppose to be in.					     */
	    NumLines = (NumArrayEntries + NumPerLine - 1) / NumPerLine;
	    switch (OutputDocType) {
		case HTML_DOCS:
		    if (Fout == NULL)
		        break;
		    sprintf(Line2, "%%-%ds", Indentation);
		    fprintf(Fout, "<TABLE BORDER=7 CELLPADDING=1>\n");
		    for (i = 0; i < NumLines; i++) {
			k = EntriesRowBased ? i * NumPerLine : i;
			fprintf(Fout, "    <TR>");
			for (j = 0; j < NumPerLine; j++) {
			    fprintf(Fout, "<TH align=left> ");

			    if (k >= NumArrayEntries)
				fprintf(Fout, Line2, "");
			    else {
				FilterLatexMods(ArrayEntries[k],
						ArrayEntries[k]);
				fprintf(Fout, Line2, &ArrayEntries[k][1]);
			    }
			    k += EntriesRowBased ? 1 : NumLines;
			}
			fprintf(Fout, "\n");
		    }
		    fprintf(Fout, "</TABLE>\n");
		    break;
		case PLAIN_TEXT_DOCS:
		case MAN_PAGES_DOCS:
		    if (Fout == NULL)
		        break;
		    sprintf(Line2, "%%-%ds", Indentation);
		    fprintf(Fout, "\n");
		    for (i = 0; i < NumLines; i++) {
			k = EntriesRowBased ? i * NumPerLine : i;
			fprintf(Fout, "   ");
			for (j = 0; j < NumPerLine; j++) {
			    if (k >= NumArrayEntries)
				fprintf(Fout, Line2, "");
			    else {
				FilterLatexMods(ArrayEntries[k],
						ArrayEntries[k]);
				fprintf(Fout, Line2, &ArrayEntries[k][1]);
			    }
			    k += EntriesRowBased ? 1 : NumLines;
			}
			fprintf(Fout, "\n");
		    }
		    fprintf(Fout, "\n");
		    break;
		case LATEX_DOCS:
		    fprintf(Fout,
			    "\n\n\\smallskip\n\n\\begin{center}\n    \\begin{tabular}{|");
		    if (VertSep)
			for (i = 0; i < NumPerLine; i++)
			    fprintf(Fout, "|l");
		    else {
			fprintf(Fout, "|");
			for (i = 0; i < NumPerLine; i++)
			    fprintf(Fout, "l");
		    }
		    fprintf(Fout,"||} \\hline \\hline\n");

		    for (i = 0; i < NumLines; i++) {
			k = EntriesRowBased ? i * NumPerLine : i;
			fprintf(Fout, "\t");
			for (j = 0; j < NumPerLine; j++) {
			    fprintf(Fout, "%s %s ",
				    k < NumArrayEntries ? &ArrayEntries[k][1] : "",
				    j == NumPerLine - 1 ? "\\\\\n" : "&");
			    k += EntriesRowBased ? 1 : NumLines;
			}
		    }
		    fprintf(Fout,
			    "\t\\hline\n    \\end{tabular}\n\\end{center}\n\n\\smallskip\n\n");
		    break;
		case IRIT_HELP_DOCS:
		    sprintf(Line2, "%%-%ds", Indentation);
		    fprintf(Fout, "\n");
		    for (i = 0; i < NumLines; i++) {
			k = EntriesRowBased ? i * NumPerLine : i;
			fprintf(Fout, "   ");
			for (j = 0; j < NumPerLine; j++) {
			    if (k >= NumArrayEntries)
				fprintf(Fout, Line2, "");
			    else {
				FilterLatexMods(ArrayEntries[k],
						ArrayEntries[k]);
				fprintf(Fout, Line2, &ArrayEntries[k][1]);
			    }
			    k += EntriesRowBased ? 1 : NumLines;
			}
			fprintf(Fout, "\n");
		    }
		    fprintf(Fout, "\n");
		    break;
		case UNDEFINED_DOC_PROCESS:
		    break;
	    }

	    continue;
	}

	switch (OutputDocType) {
	    case MAN_PAGES_DOCS:
		switch (Line1[0]) {
		    case '+':
		    case '=':
		    case '-':
		        break;
		    case '@':
		        if (strstr(Line1, "verbatim") != NULL) {
			    if (strstr(Line1, "begin") != NULL)
				InVerbatim = TRUE;
			    else if (strstr(Line1, "end") != NULL)
			        InVerbatim = FALSE;
			}
		        break;
		    case '$':
		        if (Fout != NULL) {
			    fclose(Fout);
			    Fout = NULL;
			}
			else
			    fprintf(stderr, 
				    "Uninitialized end of subject at line %d.\n",
				    LineNum);
			break;
		    case '^':
		    case '*':
			if (Fout != NULL)
			    fprintf(Fout, "%s", &Line1[1]);
			break;
		    case '!':
			if (Fout == NULL) {
			    /* If name has leading space - ignore the block. */
			    if (isspace(Line1[1]))
			        Fout = OrigFout;
			    else {
				char *p;

				sprintf(Line2, "man/man6/%s", &Line1[1]);
				if ((p = strstr(Line2, " - ")) != NULL)
				    *p = 0;
				for (i = 0; i < (int) strlen(Line2); i++) {
				    if (isspace(Line2[i]))
				        Line2[i] = '_';
				    if (isupper(Line2[i]))
				        Line2[i] = tolower(Line2[i]);
				}
				for (i = ((int) strlen(Line2)) - 1;
				     i > 0 && (Line2[i] == '-' ||
					       Line2[i] == '_');
				     i--)
				    Line2[i] = 0;
				strcat(Line2, ".6");

				if ((Fout = fopen(Line2, "w")) == NULL) {
				    fprintf(stderr, "Failed to open file \"%s\".\n",
					    Line2);
				    exit(1);
				}
			    }
			}
			fprintf(Fout, ".TH %s 6 \"IRIT %s\" \n",
				&Line1[1], IRIT_VERSION);

			fprintf(Fout, ".SH NAME\n%s\n\n", &Line1[1]);
			break;
		    case '&':
			if (Fout != NULL)
			    fprintf(Fout, ".SH %s\n\n", &Line1[1]);
			break;
		    default:
			fprintf(stderr,
			    "Undefined first char command at line %d.\n",
			    LineNum);
			/* Let it print without the first char... */
		    case 0:
		    case 10:
		    case 13:
		    case ' ':
			FilterLatexMods(Line1, Line2);
			if (Fout != NULL) {
			    i = 1;
			    if (!InVerbatim)
				for ( ; Line2[i] == ' '; i++);
			    fprintf(Fout, "%s", &Line2[i]);
			}
			break;
		}
		break;
	    case HTML_DOCS:
		switch (Line1[0]) {
		    case '@':
		        if (strstr(Line1, "verbatim") != NULL) {
			    if (strstr(Line1, "begin") != NULL) {
				if (!WasSpace)
				    fprintf(Fout, "<BR><BR><B><PRE>\n");
				else
				    fprintf(Fout, "<B><PRE>\n");
				WasSpace = TRUE;
				InVerbatim = TRUE;
			    }
			    else if (strstr(Line1, "end") != NULL) {
				fprintf(Fout, "</PRE></B><BR>\n");
			        InVerbatim = FALSE;
			    }
			}
			else if (strstr(Line1, "\\section") != NULL ||
				 strstr(Line1, "\\subsection") != NULL ||
				 strstr(Line1, "\\subsubsection") != NULL ||
				 strstr(Line1, "\\subsubsubsection") != NULL) {
			    char *p;

			    FilterLatexToHTMLMods(Line1, Line2);
			    p = strrchr(Line2, '}');

			    if (p != NULL)
			        *p = 0;
			    p = strchr(Line2, '{');

			    if (p != NULL) {
				if (strstr(Line1, "\\subsubsubsection") != NULL) {
				    while (GlblIndexLevel > 4) {
					GlblIndexLevel--;
					fprintf(FoutIndex, "</ol>");
				    }
				    if (GlblIndexLevel < 4) {
					fprintf(FoutIndex, "      <ol>\n");
					GlblIndexLevel = 4;
				    }
				    fprintf(Fout,
					    "<HR><H5> <A NAME=\"%s\"> %s </A> </H5>\n",
					    MakeLabel(&p[1]), &p[1]);
				    fprintf(FoutIndex,
					    "      <li><A HREF=\"%s#%s\"> %s </A>\n",
					    GlblFoutName, MakeLabel(&p[1]), &p[1]);
				}
				else if (strstr(Line1, "\\subsubsection") != NULL) {
				    while (GlblIndexLevel > 3) {
					GlblIndexLevel--;
					fprintf(FoutIndex, "</ol>");
				    }
				    if (GlblIndexLevel < 3) {
					fprintf(FoutIndex, "    <ol>\n");
					GlblIndexLevel = 3;
				    }
				    fprintf(Fout,
					    "<HR><H4> <A NAME=\"%s\"> %s </A> </H4>\n",
					    MakeLabel(&p[1]), &p[1]);
				    fprintf(FoutIndex,
					    "    <li><A HREF=\"%s#%s\"> %s </A>\n",
					    GlblFoutName, MakeLabel(&p[1]), &p[1]);
				}
				else if (strstr(Line1, "\\subsection") != NULL) {
				    while (GlblIndexLevel > 2) {
					GlblIndexLevel--;
					fprintf(FoutIndex, "</ol>");
				    }
				    if (GlblIndexLevel < 2) {
					fprintf(FoutIndex, "  <ol>\n");
					GlblIndexLevel = 2;
				    }
				    fprintf(Fout,
					    "<HR><H3> <A NAME=\"%s\"> %s </A> </H3>\n",
					    MakeLabel(&p[1]), &p[1]);
				    fprintf(FoutIndex,
					    "  <li><A HREF=\"%s#%s\"> %s </A>\n",
					    GlblFoutName, MakeLabel(&p[1]), &p[1]);
				}
				else {
				    while (GlblIndexLevel > 1) {
					GlblIndexLevel--;
					fprintf(FoutIndex, "</ol>");
				    }
				    if (GlblIndexLevel < 1) {
					fprintf(FoutIndex, "<ol>\n");
					GlblIndexLevel = 1;
				    }
				    fprintf(Fout,
					    "<HR><H2> <A NAME=\"%s\"> %s </A> </H2>\n",
					    MakeLabel(&p[1]), &p[1]);
				    fprintf(FoutIndex,
					    "<li><A HREF=\"%s#%s\"> %s </A>\n",
					    GlblFoutName, MakeLabel(&p[1]), &p[1]);
				}
			    }
			}
			else if (strstr(Line1, "itemize") != NULL ||
				 strstr(Line1, "enumerate") != NULL) {
			    if (strstr(Line1, "begin") != NULL) {
				InAllowItems = TRUE;
			    }
			    else if (strstr(Line1, "end") != NULL) {
				InAllowItems = TRUE;
			    }
			}
			else if (strstr(Line1, "item") != NULL &&
				 InAllowItems) {
			    fprintf(Fout, "\n<BR><BR>\n");
			}
		        break;
		    case '$':
			break;
		    case '^':
			FilterLatexToHTMLMods(Line1, Line2);
			fprintf(Fout, "<PRE>%s</PRE>\n", &Line2[2]);
			break;
		    case '*':
			FilterLatexToHTMLMods(Line1, Line2);
			fprintf(Fout, "%s", &Line2[1]);
			break;
		    case '+':
			fprintf(Fout, "%s", &Line1[1]);
			fprintf(FoutIndex, "%s", &Line1[1]);
			break;
		    case '=':
		    case '-':
			FilterLatexToHTMLMods(Line1, Line2);
			fprintf(Fout, "%s", &Line2[1]);
			break;
		    case '&':
		    case '!':
			break;
		    default:
			fprintf(stderr,
			    "Undefined first char command at line %d.\n",
			    LineNum);
			/* Let it print without the first char... */
		    case 0:
		    case 10:
		    case 13:
		    case ' ':
			FilterLatexMods(Line1, Line2);
			if (InVerbatim) {
			    fprintf(Fout, "%s", &Line2[1]);
			}
			else {
			    if (IsSpaceLine(&Line2[1])) {
				if (!WasSpace) {
				    fprintf(Fout, "\n<BR><BR>\n");
				    WasSpace = TRUE;
				}
			    }
			    else {
			        fprintf(Fout, "%s", &Line2[1]);
				WasSpace = FALSE;
			    }
			}
			break;
		}
		break;
	    case PLAIN_TEXT_DOCS:
		switch (Line1[0]) {
		    case '=':
		    case '-':
		    case '+':
		    case '@':
		    case '$':
			break;
		    case '^':
		    case '*':
			fprintf(Fout, "%s", &Line1[1]);
			break;
		    case '&':
		    case '!':
		    	j = (78 - (int) strlen(&Line1[1])) / 2;
		    	for (i = 0; i < j; i++)
			    fprintf(Fout, " ");
			fprintf(Fout, "%s", &Line1[1]);
		    	for (i = 0; i < j; i++)
			    fprintf(Fout, " ");
			for (i = 1; i < (int) strlen(&Line1[1]); i++)
			    fprintf(Fout, "-");
			fprintf(Fout, "\n");
			break;
		    default:
			fprintf(stderr,
			    "Undefined first char command at line %d.\n",
			    LineNum);
			/* Let it print without the first char... */
		    case 0:
		    case 10:
		    case 13:
		    case ' ':
			FilterLatexMods(Line1, Line2);
			fprintf(Fout, "%s", &Line2[1]);
			break;
		}
		break;
	    case LATEX_DOCS:
		switch (Line1[0]) {
		    case '=':
		    case '+':
		    case '!':
		    case '$':
		    case '&':
		    case '^':
			break;
		    case '-':
		    case '@':
		    case '*':
			fprintf(Fout, "%s", &Line1[1]);
			break;
		    default:
			fprintf(stderr,
			    "Undefined first char command at line %d.\n",
			    LineNum);
			/* Let it print without the first char... */
		    case ' ':
		    case 0:
    		    case 10:
		    case 13:
			fprintf(Fout, "%s", Line1[1] == 0 ? "\n" : &Line1[1]);
			break;
		}
		break;
	    case IRIT_HELP_DOCS:
		switch (Line1[0]) {
		    case '=':
		    case '-':
		    case '+':
		    case '@':
		    case '&':
			break;
		    case '*':
		    case '^':
		    case '!':
			fprintf(Fout, "%s", &Line1[1]);
			break;
		    case '$':
			fprintf(Fout, "$\n");
			break;
		    default:
			fprintf(stderr,
			    "Undefined first char command at line %d.\n",
			    LineNum);
			/* Let it print without the first char... */
		    case ' ':
		    case 0:
		    case 10:
		    case 13:
			FilterLatexMods(Line1, Line2);
			fprintf(Fout, "%s", &Line2[1]);
			break;
		}
		break;
	    case UNDEFINED_DOC_PROCESS:
		break;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Filters out expression of the form '{\cmd * }' in Line1 into '*' in line2. *
* These are latex modifiers not wanted in plain text output.		     *
*   Also are filtered out any \? to ?, any $*$ to * and \verb+^+ to ^.       *
*   A space is forced as first char in line.				     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Line1:     Input line to filter out.                                     *
*   Line2:     Filtered version of Line1.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FilterLatexMods(char *Line1, char *Line2)
{
    int i = 0,
	j = 0,
	InModifier = FALSE;
    char c, Line3[IRIT_LINE_LEN_LONG];

    Line3[j++] = ' ';

    while (Line1[i]) {
	if (Line1[i] == '{' && Line1[i + 1] == '\\' && !isspace(Line1[i + 2])) {
	    /* Here is one - filter it out. */
	    while (!isspace(Line1[i++]));
	    while (isspace(Line1[i++]));
	    i--;
	    InModifier = TRUE;
	}
	else if (Line1[i] == '}' && InModifier) {
	    i++;
	    InModifier = FALSE;
	}
	else if (strncmp(&Line1[i], "\\verb", 5) == 0) {
	    i += 5;
	    c = Line1[i++];
	    while (Line1[i] != c)
	        Line3[j++] = Line1[i++];
	    i++;
	}
	else if (Line1[i] == '\\') {
	    i++;
	    Line3[j++] = Line1[i++];
	}
	else if (Line1[i] == '$' && i > 0) {
	    i++;
	}
	else
	    Line3[j++] = Line1[i++];
    }
    Line3[j] = 0;
    strcpy(Line2, Line3);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Filters out expression of the form '{\cmd * }' in Line1 into '*' in line2. *
* These are latex modifiers and we convert what we can to html style.	     *
*   Also are filtered out any \? to ?, any $*$ to * and \verb+^+ to ^.       *
*   A space is forced as first char in line.				     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Line1:     Input line to filter out.                                     *
*   Line2:     Filtered version of Line1.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FilterLatexToHTMLMods(char *Line1, char *Line2)
{
    int i = 1,
	j = 0,
	Bold = FALSE,
	Emph = FALSE,
	InModifier = FALSE;
    char c, Line3[IRIT_LINE_LEN_LONG];

    Line3[j++] = ' ';

    while (Line1[i]) {
	if (Line1[i] == '{' && Line1[i + 1] == '\\' && !isspace(Line1[i + 2])) {
	    /* Here is one - filter it out. */
	    if (strncmp(&Line1[i + 1], "\\bf", 3) == 0) {
		Bold = TRUE;
		strcpy(&Line3[j], "<b>");
		j += 3;
	    }
	    if (strncmp(&Line1[i + 1], "\\em", 3) == 0) {
		Emph = TRUE;
		strcpy(&Line3[j], "<em>");
		j += 4;
	    }

	    while (!isspace(Line1[i++]));
	    while (isspace(Line1[i++]));
	    i--;
	    InModifier = TRUE;
	}
	else if (Line1[i] == '}' && InModifier) {
	    i++;
	    InModifier = FALSE;

	    if (Bold) {
		Bold = FALSE;
		strcpy(&Line3[j], "</b>");
		j += 4;
	    }
	    if (Emph) {
		Emph = FALSE;
		strcpy(&Line3[j], "</em>");
		j += 5;
	    }
	}
	else if (strncmp(&Line1[i], "\\verb", 5) == 0) {
	    i += 5;
	    c = Line1[i++];
	    while (Line1[i] != c)
	        Line3[j++] = Line1[i++];
	    i++;
	}
	else if (Line1[i] == '\\') {
	    i++;
	    Line3[j++] = Line1[i++];
	}
	else if (Line1[i] == '$' && i > 0) {
	    i++;
	}
	else
	    Line3[j++] = Line1[i++];
    }
    Line3[j] = 0;
    strcpy(Line2, Line3);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Find and replace all occurances of the string in GlblStrSubstStruct.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:     Line to global find and replace on.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MacroSubstitute(char *Line)
{
    int i;
    char *p, TmpLine[IRIT_LINE_LEN_LONG];

    for (i = 0; GlblStrSubstStruct[i].FindString != NULL; i++) {
	while ((p = strstr(Line, GlblStrSubstStruct[i].FindString)) != NULL) {
	    strcpy(TmpLine, p + strlen(GlblStrSubstStruct[i].FindString));
	    strcpy(p, GlblStrSubstStruct[i].ReplaceString);
	    strcat(p, TmpLine);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a label with no spaces from an arbitrary string.                *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:      A string to convert to a label.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:   A label - a string with no spaces embeded.                     *
*****************************************************************************/
static char *MakeLabel(char *Str)
{
    int i;
    IRIT_STATIC_DATA char Label[IRIT_LINE_LEN_LONG],
                          PrevLevelsLabels[4][IRIT_LINE_LEN_LONG];

    if (GlblIndexLevel > 1)
        strcpy(Label, PrevLevelsLabels[GlblIndexLevel - 1]);
    else
        Label[0] = 0;
    strcat(Label, Str);

    for (i = 0; Label[i] != 0 && i < IRIT_LINE_LEN_LONG; i++)
	if (isspace(Label[i]))
	    Label[i] = '_';

    strcpy(PrevLevelsLabels[GlblIndexLevel], Label);

    return Label;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns TRUE if given line has only spaces.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:    To verify if a space line or not.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if space line, FALSE otherwise.                             *
*****************************************************************************/
static int IsSpaceLine(char *Str)
{
    int i;

    for (i = 0; i < (int) strlen(Str); i++)
	if (!isspace(Str[i]))
	    return FALSE;

    return TRUE;
}

