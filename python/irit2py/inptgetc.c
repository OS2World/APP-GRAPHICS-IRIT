/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Handles the fetching of characters for the irt scripting language.       *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "program.h"
#include "allocate.h"
#include "ctrl-brk.h"
#include "inptprsg.h"
#include "inptprsl.h"
#include "objects.h"
#include "overload.h"

#define FILE_STACK_SIZE	10
#define MAX_QUEUE_LINES 1000

typedef struct IritFileStackStruct {
    char Name[IRIT_LINE_LEN_SHORT];
    char Path[IRIT_LINE_LEN_LONG];
    FILE *f;
} IritFileStackStruct;

IRIT_GLOBAL_DATA IritInputSourceType IritInputSource;

IRIT_STATIC_DATA char
    GlblInputLine[INPUT_LINE_LEN] = "",
    GlblUnGetChar = 0;
IRIT_STATIC_DATA const char
    *GlblUnGetLines[MAX_QUEUE_LINES];/* Pushed ln pointers on input buffer!? */
IRIT_STATIC_DATA int
    GlblFileStackPtr = 0,
    InptPrsrEchoSource = 0,
    GlblUnGetLinesTail = 0,
    GlblUnGetLinesHead = 0,
    GlblInputLineLength = 0,
    GlblInputLineCount = 0;
IRIT_STATIC_DATA IritFileStackStruct
    GlblFileStack[FILE_STACK_SIZE];/* Include file stack.*/

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Queue a line to process in input stream.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:     To queue for processing in input stream.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrQueueInputLine                                                   M
*****************************************************************************/
void InptPrsrQueueInputLine(const char *Line)
{
    GlblUnGetLines[GlblUnGetLinesHead++] = Line;

    if (GlblUnGetLinesHead == MAX_QUEUE_LINES)
	GlblUnGetLinesHead = 0;

    if (GlblUnGetLinesHead == GlblUnGetLinesTail)
        IRIT_NON_FATAL_ERROR("Reevaluation input line queue is full");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets echo level of source irt files.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   EchoSource:  TRUE for echo, FALSE for no echo.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value.                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrSetEchoSource                                                    M
*****************************************************************************/
int InptPrsrSetEchoSource(int EchoSource)
{
    int OldVal = InptPrsrEchoSource;

    InptPrsrEchoSource = EchoSource;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to examine if some input awaits processing by the interpreter.     M
*                                                                            *
* PARAMETERS:                                                                M
*   None				                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int       TRUE if dat awaits processing, FALSE otherwise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrHasQueuedInput                                                   M
*****************************************************************************/
int InptPrsrHasQueuedInput(void)
{
    return GlblUnGetLinesHead != GlblUnGetLinesTail ||
           GlblFileStackPtr > 0 ||
	   GlblInputLineLength != GlblInputLineCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to control all getchar in this module and echo it if requested     M
*   Note it handles the GlblFileStack and decrease it if end of file was     M
* found.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   InString:   TRUE if now we read between two double quotes (a string).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   char:       Next character in input stream.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrGetC	                                                     M
*****************************************************************************/
char InptPrsrGetC(int InString, int InComment)
{
    IRIT_STATIC_DATA char *p,
	TLine[INPUT_LINE_LEN] = "";
    char c;
    int i;

    if (GlblUnGetChar == 0) {		       /* One level of unget char... */
	if (GlblInputLineCount < GlblInputLineLength) {
	}
	else {
	    do {
	        if (GlblUnGetLinesHead != GlblUnGetLinesTail) {
		    strcpy(GlblInputLine,
			   GlblUnGetLines[GlblUnGetLinesTail++]);
		    if (GlblUnGetLinesTail == MAX_QUEUE_LINES)
		        GlblUnGetLinesTail = 0;

		    IritInputSource = IRIT_INPUT_SOURCE_LINE_QUEUE;
		    GlblInputLineCount = 0;
		}
	        else if (GlblFileStackPtr == 0) {
		    /* End of conversion - quit */
		    fflush(stdout);
		    IritExit(0);
		    IritInputWindowGetStr(GlblInputLine, INPUT_LINE_LEN);
		    IritInputSource = IRIT_INPUT_SOURCE_KBD;
		    GlblInputLineCount = 0;
	        }
	        else {
		    sprintf(GlblInputLine, "%s > ",
			    GlblFileStack[GlblFileStackPtr - 1].Name);

		    IritInputSource = IRIT_INPUT_SOURCE_FILE;
		    IritIdleFunction(0);
		    GlblInputLineCount = (int) strlen(GlblInputLine);
		    if (fgets(TLine, INPUT_LINE_LEN,
			      GlblFileStack[GlblFileStackPtr - 1].f) == NULL) {
		        /* Its end of file - close it and update stack. */
			TLine[0] = 0;
		        fclose(GlblFileStack[--GlblFileStackPtr].f);
		    }

		    /* Echo empty space lines. */
		    if (strlen(TLine) == 0 ||
			TLine[0] == '\n' ||
			TLine[0] == '\r') {
		        if (strnicmp(InptPrsrQueryCurrentFile(),
				     "iritinit", 8) != 0)
			    printf(InComment ? "\n#" : "\n");
		    }

		    /* Strip off CR/LF/TAB.  */
		    for (i = GlblInputLineCount, p = TLine; *p != 0; p++) {
			if (*p == 0x09)
			    do {
				GlblInputLine[i++] = ' ';
			    }
		            while ((i - GlblInputLineCount) % 8 != 0);
			else if (*p < ' ' || *p > '~') {
			    if (InComment && (*p == '\n' || *p == '\r'))
			        printf("\n#");
			    break;
			}
			else
			    GlblInputLine[i++] = *p;
		    }
		    GlblInputLine[i] = 0;
	        }

		if (InptPrsrEchoSource &&
		    IritInputSource == IRIT_INPUT_SOURCE_FILE) {
		    /* Input was from file? */
		    IRIT_WNDW_PUT_STR(GlblInputLine);
		}

	        GlblInputLineLength = (int) strlen(GlblInputLine);
	    }
	    while (GlblInputLineCount >= GlblInputLineLength);
	}

	c = GlblInputLine[GlblInputLineCount++];
	if (c == '#' && !InString) {	  /* Its a comment - skip that line. */
	    if (strnicmp(InptPrsrQueryCurrentFile(), "iritinit", 8) != 0)
	        printf("# %s\n", &GlblInputLine[GlblInputLineCount]);

            c = ' ';				   /* Must return something. */
	    /* Force next time to fetch new line. */
            GlblInputLineCount = GlblInputLineLength;
	}
#	ifdef DEBUG1
	    fprintf(stderr, "%c", c);
#	endif /* DEBUG1 */
    }
    else {
	c = GlblUnGetChar;
	GlblUnGetChar = 0;
    }

    return c;
}

char InptPrsrGetCOrig(int InString)
{
    IRIT_STATIC_DATA char *p,
	TLine[INPUT_LINE_LEN] = "";
    char c;
    int i;

    if (GlblUnGetChar == 0) {		       /* One level of unget char... */
        /* Is there anything in local Line? */
	if (GlblInputLineCount < GlblInputLineLength) {
	}
	else {
	    do {
	        if (GlblUnGetLinesHead != GlblUnGetLinesTail) {
		    strcpy(GlblInputLine,
			   GlblUnGetLines[GlblUnGetLinesTail++]);
		    if (GlblUnGetLinesTail == MAX_QUEUE_LINES)
		        GlblUnGetLinesTail = 0;

		    IritInputSource = IRIT_INPUT_SOURCE_LINE_QUEUE;
		    GlblInputLineCount = 0;
		}
	        else if (GlblFileStackPtr == 0) {
		    IritInputWindowGetStr(GlblInputLine, INPUT_LINE_LEN);
		    IritInputSource = IRIT_INPUT_SOURCE_KBD;
		    GlblInputLineCount = 0;
	        }
	        else {
		    sprintf(GlblInputLine, "%s > ",
			    GlblFileStack[GlblFileStackPtr - 1].Name);
		    IritInputSource = IRIT_INPUT_SOURCE_FILE;
		    IritIdleFunction(0);
		    GlblInputLineCount = (int) strlen(GlblInputLine);
		    if (fgets(TLine, INPUT_LINE_LEN,
			      GlblFileStack[GlblFileStackPtr - 1].f) == NULL) {
		        /* Its end of file - close it and update stack. */
			TLine[0] = 0;
		        fclose(GlblFileStack[--GlblFileStackPtr].f);
		    }

		    /* Strip off CR/LF/TAB.  */
		    for (i = GlblInputLineCount, p = TLine; *p != 0; p++) {
			if (*p == 0x09)
			    do {
				GlblInputLine[i++] = ' ';
			    }
		            while ((i - GlblInputLineCount) % 8 != 0);
			else if (*p < ' ' || *p > '~')
			    break;
			else
			    GlblInputLine[i++] = *p;
		    }
		    GlblInputLine[i] = 0;
	        }

		if (InptPrsrEchoSource &&
		    IritInputSource == IRIT_INPUT_SOURCE_FILE) {
		    /* Input was from file? */
		    IRIT_WNDW_PUT_STR(GlblInputLine);
		}

	        GlblInputLineLength = (int) strlen(GlblInputLine);
	    } while (GlblInputLineCount >= GlblInputLineLength);
	}

	c = GlblInputLine[GlblInputLineCount++];
	if (c == '#' && !InString) {	  /* Its a comment - skip that line. */
            c = ' ';				   /* Must return something. */
	    /* Force next time to fetch new line. */
            GlblInputLineCount = GlblInputLineLength;
	}
#	ifdef DEBUG1
	    IRIT_INFO_MSG_PRINTF("%c", c);
#	endif /* DEBUG1 */
    }
    else {
	c = GlblUnGetChar;
	GlblUnGetChar = 0;
    }

    return c;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to unget one char						     *
*                                                                            *
* PARAMETERS:                                                                *
*   c:        Character to unget.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void InptPrsrUnGetC(char c)
{
    GlblUnGetChar = c;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the number of the currently processed file, "" for stdin.        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:                                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrQueryCurrentFile                                                 M
*****************************************************************************/
char *InptPrsrQueryCurrentFile(void)
{
    return GlblFileStackPtr == 0 ? ""
                                 : GlblFileStack[GlblFileStackPtr - 1].Name;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to quit reading until next ';'. If reading from files, they are    M
* all closed as well.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   FlushStdin:   If not reading from a file, should we skip to next ';'?    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrFlushToEndOfExpr                                                 M
*****************************************************************************/
void InptPrsrFlushToEndOfExpr(int FlushStdin)
{
    if (GlblUnGetLinesHead != GlblUnGetLinesTail)
	GlblUnGetLinesHead = GlblUnGetLinesTail;
    if (GlblFileStackPtr > 0)   /* Close all open files - back to stdin. */
	while (GlblFileStackPtr)
	    fclose(GlblFileStack[--GlblFileStackPtr].f);
    else if (FlushStdin && InptPrsrLastToken != IP_TKN_END)
	while (InptPrsrGetC(FALSE, FALSE) != ';');
}

void InptPrsrFlushToEndOfExprOrig(int FlushStdin)
{
    if (GlblUnGetLinesHead != GlblUnGetLinesTail)
	GlblUnGetLinesHead = GlblUnGetLinesTail;
    if (GlblFileStackPtr > 0)   /* Close all open files - back to stdin. */
	while (GlblFileStackPtr)
	    fclose(GlblFileStack[--GlblFileStackPtr].f);
    else if (FlushStdin && InptPrsrLastToken != IP_TKN_END)
      while (InptPrsrGetC(FALSE, -1) != ';');
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to push new file to read on GlblFileStack from INCLUDE command:    M
*                                                                            *
* PARAMETERS:                                                                M
*   PrmFileName:  Name of file to start to read from.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrFileInclude                                                      M
*****************************************************************************/
void InptPrsrFileInclude(const char *PrmFileName)
{
    int i;
    FILE *f;
    char FileName[IRIT_LINE_LEN_LONG], c, *p;

    if (GlblFileStackPtr < FILE_STACK_SIZE) {
	strcpy(FileName, PrmFileName);

  	if (strstr(FileName, ".irt") == NULL &&
	    strstr(FileName, ".IRT") == NULL)
	    strcat(FileName, ".irt");

	if ((f = fopen(FileName, "r")) == NULL &&
	    GlblFileStackPtr > 0 &&
	    !IRT_STR_ZERO_LEN(GlblFileStack[GlblFileStackPtr - 1].Path)) {
	    char FullFileName[IRIT_LINE_LEN_LONG];

	    /* Try the directory specified by the parent file, if any. */
	    sprintf(FullFileName, "%s%s",
		    GlblFileStack[GlblFileStackPtr - 1].Path, FileName);
	    if ((f = fopen(FullFileName, "r")) != NULL)
	        strncpy(FileName, FullFileName, IRIT_LINE_LEN_LONG - 1);
	}

	/* If still a failure, see if we have IRIT_INCLUDE specifications. */
	if (f == NULL && (p = getenv("IRIT_INCLUDE")) != NULL) {
	    char Path[IRIT_LINE_LEN_LONG];

	    strncpy(Path, p, IRIT_LINE_LEN_LONG - 1);

	    if ((p = strtok(Path, ";")) != NULL) {
	        do {
		    char FullFileName[IRIT_LINE_LEN_LONG];

		    /* Try the directory specified by the IRIT_INCLUDE. */
#		    if defined(__WINNT__) || defined(OS2GCC)
		        sprintf(FullFileName, "%s\\%s", p, FileName);
#		    else
		        sprintf(FullFileName, "%s/%s", p, FileName);
#		    endif /* __WINNT__ || OS2GCC */

		    if ((f = fopen(FullFileName, "r")) != NULL) {
		        strncpy(FileName, FullFileName, IRIT_LINE_LEN_LONG - 1);
			break;
		    }
		}
		while ((p = strtok(NULL, ";")) != NULL);
	    }
	}

	if (f != NULL) {
	    GlblFileStack[GlblFileStackPtr].f = f;
	    for (i = (int) strlen(FileName) - 1;   /* Isolate the file name. */
		 i > 0 && (c = FileName[i]) != '\\' && c != '/' && c != ':';
		 i--);
	    if (i > 0)
		i++;
	    strncpy(GlblFileStack[GlblFileStackPtr].Name, &FileName[i],
						     IRIT_LINE_LEN_SHORT - 1);
	    if (i > 0) {
	        FileName[i] = 0;
		strncpy(GlblFileStack[GlblFileStackPtr].Path, FileName,
						      IRIT_LINE_LEN_LONG - 1);
	    }
	    else 
		GlblFileStack[GlblFileStackPtr].Path[0] = 0;

	    if ((p = strstr(GlblFileStack[GlblFileStackPtr].Name,
			    ".irt")) != NULL ||
		(p = strstr(GlblFileStack[GlblFileStackPtr].Name,
			    ".IRT")) != NULL)
		*p = 0;

	    GlblFileStackPtr++;		 /* Now next char is from that file! */
	}
	else {
	    IRIT_WNDW_FPRINTF2("Cannt open file %s - ignored", FileName);
	}
    }
    else
	IRIT_WNDW_PUT_STR("File nesting too deep - ignored");
}

