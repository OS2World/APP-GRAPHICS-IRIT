/*****************************************************************************
*  Routines to grab the	parameters from	the command line :		     *
* All the routines except the main one,	starts with GA (Get Arguments) to    *
* prevent from names conflicts.						     *
* It is	assumed	in these routine that any pointer, for any type	has the	     *
* same length (i.e. length of int pointer is equal to char pointer etc.)     *
*									     *
*  The following routines are available	in this	module:			     *
* 1. int GAGetArgs(argc, argv, CtrlStr, Variables...)			     *
*    where argc, argv as received on entry.				     *
*	   CtrlStr is the contrl string	(see below)			     *
*	   Variables are all the variables to be set according to CtrlStr .  *
*	   Note	that all the variables MUST be transfered by address.	     *
*    return 0 on correct parsing, otherwise error number (see GetArg.h).     *
* 2. GAPrintHowTo(CtrlStr)						     *
*    Print the control string, in the correct format needed.		     *
*    This feature is very useful in case of error during GetArgs parsing.    *
*    Chars equal to SPACE_CHAR are not printed (regular spaces are NOT       *
*    allowed, and so using SPACE_CHAR you can create space in PrintHowTo) .  *
* 3. GAPrintErrMsg(Error)						     *
*    Print the error, according to Error (usually returned by GAGetArgs).    *
*                                                                            *
*   Notes:								     *
*									     *
* 1. This module assumes that all the pointers to all kind of data types     *
*    have the same length and format, i.e. sizeof(int *) == sizeof(char	* ). *
*									     *
*				      Gershon Elber    Ver 0.2	 Mar 88	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* History:								     *
* 11 Mar 88 - Version 1.0 by Gershon Elber.				     *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_loc.h"

#define	CMD_ERR_NOT_AN_OPT   1			       /* None Option found. */
#define	CMD_ERR_NO_SUCH_OPT  2			  /* Undefined Option Found. */
#define	CMD_ERR_WILD_EMPTY   3			 /* Empty input for !*? seq. */
#define	CMD_ERR_NUM_READ     4			/* Failed on reading number. */
#define	CMD_ERR_ALL_SATIS    5	       /* Fail to satisfy (must-'!') option. */

#define	MAX_PARAM	100	    /* maximum number of parameters allowed. */
#define	CTRL_STR_MAX_LEN	1024

#define SPACE_CHAR	'|'	  /* The character not to print using HowTo. */

#ifndef	TRUE
#define	TRUE -1
#define	FALSE 0
#endif /* TRUE */

#define	ARG_OK    0

#define	ISSPACE(x) ((x)	<= ' ')	       /* Not conventional - but works fine! */
/* The two characters '%' and '!' are used in the control string: */
#define	ISCTRLCHAR(x) (((x) == '%') || ((x) == '!'))

/* On error code, ErrorToken is set to point on it. */
IRIT_STATIC_DATA char *GAErrorToken;

static int GATestAllSatis(char *CtrlStrCopy,
			  char *CtrlStr,
			  int *argc,
			  char ***argv,
			  int *Parameters[MAX_PARAM],
			  int *ParamCount);
static int GAUpdateParameters(int *Parameters[],
			      int *ParamCount,
			      char *Option,
			      char *CtrlStrCopy,
			      char *CtrlStr,
			      int *argc,
			      char ***argv);
static int GAGetParmeters(int *Parameters[],
			  int *ParamCount,
			  char *CtrlStrCopy,
			  char *Option,
			  int *argc,
			  char ***argv);
static int GAGetMultiParmeters(int *Parameters[],
			       int *ParamCount,
			       char *CtrlStrCopy,
			       int *argc,
			       char ***argv);
static void GASetParamCount(char *CtrlStr, int Max, int *ParamCount);
static void GAByteCopy(char *Dst, char *Src, unsigned n);
static int GAOptionExists(int argc, char **argv);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to access command line arguments and interpret them, by getting    M
* access to the main routine's argc/argv interface and a control string that M
* prescribes the expected options. 				             M
*   Returns ARG_OK (0) is case of succesfull parsing, error code else...     M
*									     M
*   Format of CtrlStr format:						     M
*   The	control	string passed to GAGetArgs controls the way argv (argc) are  M
* parsed. Each entry in	this string must have no spaces in it.               M
*   The	First Entry is the name of the program which is usually ignored      M
*  except when GAPrintHowTo is called. All the other entries (except the     M
* last one which will be discussed shortly) must have the following format:  M
* 1. One letter	which sets the option letter (i.e. 'x' for option '-x').     M
* 2. '!' or '%'	to determines if this option is	really optional	('%') or     M
*    it	must be provided by the user ('!').				     M
* 3. '-' always.							     M
* 4. Alpha numeric string (and '|' to denote a space), usually ignored, but  M
*    used by GAPrintHowTo to describe the meaning of this option.	     M
* 5. Sequences that start with either '!' or '%'.			     M
*       Again if '!' then this sequence must exists (only if its option flag M
*    is given), and if '%' it is optional.				     M
*       Each sequence will be followed by one or two characters	which        M
*    defines the kind of the input:			   		     M
*    5.1. d, x, o, u - integer is expected (decimal, hex, octal base or	     M
*		     unsigned).						     M
*    5.2. D, X, O, U - long integer is expected (same as above).	     M
*    5.3. f	     - float number is expected.			     M
*    5.4. F	     - double number is expected.			     M
*    5.5. s	     - string is expected.				     M
*    5.6. *?	- any number of	'?' kind (d, x, o, u, D, X, O, U, f, F, s)   M
*		  will match this one. If '?' is numeric, it scans until     M
*		  none numeric input is given. If '?' is 's' then it scans   M
*		  up to the next option or end of argv.			     M
*									     M
* If the last parameter given in the CtrlStr, is not an option (i.e. the     M
* second char is not in	['!', '%'] and the third one is not '-'), all what   M
* remained from	argv is	hooked to it.					     M
*									     M
* The variables passed to GAGetArgs (starting from 4th parameter) MUST       M
* match	the order of options in the CtrlStr.				     M
*   For	each option, an address of an integer must be passed. This integer   M
* must initialized by 0. If that option is given in the command line, it     M
* will be set to one. Otherwise, this integer will not be affected.	     M
*   In addition, the sequences that might follow an option require the	     M
* following parameter(s) to be passed			   		     M
* 1. d, x, o, u - pointer to an integer (int *).			     M
* 2. D, X, O, U - pointer to a long (long *).				     M
* 3. f	     - pointer to a float (float *).				     M
* 4. F	     - pointer to a double (double *).				     M
* 5. s	     - pointer to a char * (char **). NO pre-allocation is required. M
* 6. *?	     - TWO variables are passed	for each such wild character         M
*              request. The first variable is an address of an integer, and  M
*	       it will return the number of parameters actually hooked to    M
*	       this sequence. The second variable is a pointer to a pointer  M
*	       to type ? (? **). It will return	an address of a vector of    M
*	       pointers of type ?, terminated with a NULL pointer.	     M
*	       NO pre-allocation is required.		   		     M
*	         These two variables behaves very much like the argv/argc    M
*	       pair and are used the "trap" unused command line options.     M
*									     M
*   Examples:								     M
*									     M
*    "Example1  i%-OneInteger!d  s%-Strings!*s  j%-  k!-Double!F  Files!*s"  V
* Will match: Example1 -i 77 -s	String1	String2	String3	-k 88.2	File1 File2  V
*   or match: Example1 -s String1 -k 88.3 -i 999 -j			     V
*    but not: Example1 -i 77 78	(i expects one integer, k must be specified).V
* The option k must exists in the above example and if '-i' is prescribed    M
* one integer argument must follow it.					     M
*   In the first example, File1 & File2, will match Files in the control     M
* string.								     M
*   The order of the options in the command line is irrelevant.              M
* A call to GAPrintHowTo with this CtrlStr will	print the info:		     M
*									     M
* Example1 [-i OneIngeter] [-s Strings...] [-j]	-k Float Files...	     V
*									     M
* The parameters below are stdarg style and in fact are expecting the        M
* following:								     M
*									     M
*   GAGetArgs(argc, argv, CtrlStr, ...);				     V
*									     M
* 1. argc, argv: The usual C interface from the main routine of the program. M
* 2. CtrlStr:    Defining the types/options to expect in the command line.   M
* 3.  ...:       list of addreses of variables to initialize according to    M
*                parsed command line.                                        M
*									     M
* PARAMETERS:                                                                M
*   va_alist:   Do "man stdarg".                                             M
*   ...:        Rest of optional parameters                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if command line was valid, FALSE otherwise.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GAPrintErrMsg, GAPrintHowTo		                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GAGetArgs, command line arguments                                        M
*****************************************************************************/
#ifdef USE_VARARGS
int GAGetArgs(int va_alist, ...)
{
    va_list ap;
    int argc, i,
	*Parameters[MAX_PARAM],		   /* Save here parameter addresses. */
	Error = FALSE,
	ParamCount = 0;
    char **argv, *CtrlStr, *Option, CtrlStrCopy[CTRL_STR_MAX_LEN];

    va_start(ap);

    argc = va_arg(ap, int);
    argv = va_arg(ap, char **);
    CtrlStr = va_arg(ap, char *);

    strcpy(CtrlStrCopy, CtrlStr);

    /* Using base address of parameters we access other parameters addr:  */
    /* Note that we (for sure!) samples data beyond the current function  */
    /* frame, but we access only these set address only by demand.	  */
    for (i = 1; i <= MAX_PARAM; i++)
	Parameters[i-1] = va_arg(ap, int *);

    va_end(ap);
#else /* Using stdarg.h */
int GAGetArgs(int argc, ...)
{
    va_list ap;
    int i, Error = FALSE, ParamCount = 0,
	*Parameters[MAX_PARAM];		   /* Save here parameter addresses. */
    char **argv, *CtrlStr, *Option, CtrlStrCopy[CTRL_STR_MAX_LEN];

    va_start(ap, argc);

    argv = va_arg(ap, char **);
    CtrlStr = va_arg(ap, char *);

    strcpy(CtrlStrCopy, CtrlStr);

    /* Using base address of parameters we access other parameters addr:  */
    /* Note that we (for sure!) samples data beyond the current function  */
    /* frame, but we access only these set address only by demand.	  */
    for (i = 1; i <= MAX_PARAM; i++)
	Parameters[i - 1] = va_arg(ap, int *);

    va_end(ap);
#endif /* USE_VARARG */

    --argc; argv++;	    /* Skip the program name (first in argv/c list). */
    while (argc >= 0) {
	if (!GAOptionExists(argc, argv))
	    break;						/* The loop. */
	argc--;
	Option	= *argv++;
	if ((Error = GAUpdateParameters(Parameters, &ParamCount, Option,
	     CtrlStrCopy, CtrlStr, &argc, &argv)) != FALSE)
	    return Error;
    }
    /*	Check for results and update trail of command line: */
    return GATestAllSatis(CtrlStrCopy, CtrlStr, &argc, &argv, Parameters,
								 &ParamCount);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to search for	unsatisfied flags - simply scan	the list for !-	     *
* sequences. Before this scan, this routine updates the rest of the command  *
* line into the	last two parameters if it is requested by the CtrlStr	     *
* (when last item in CtrlStr is NOT an option).				     *
*   Returns ARG_OK if all satisfied, CMD_ERR_ALL_SATIS error else.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   CtrlStrCopy:     A fresh, unmodified copy of given control string.       *
*   CtrlStr:         A bashed, marked with specified option copy of the      *
*                    control string.					     *
*   argc, argv:      What is left of the command line, if anything.          *
*   Parameters:      Where rest of command line is going to be hooked to.    *
*   ParamCount:      Index into Parameters where to hook left overs.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             TRUE if command line was valid, FALSE otherwise.        *
*****************************************************************************/
static int GATestAllSatis(char *CtrlStrCopy,
			  char *CtrlStr,
			  int *argc,
			  char ***argv,
			  int *Parameters[MAX_PARAM],
			  int *ParamCount)
{
    int	i;
    IRIT_STATIC_DATA char
	*LocalToken = NULL;

    /* If LocalToken is not initialized - do it now. Note that this string */
    /* should be writable as well so we can not assign it directly.        */
    if (LocalToken == NULL) {
        LocalToken = (char *) IritMalloc(3);
	strcpy(LocalToken, "-?");
    }

    /* Check if	last item is an	option.	If not then copy rest of command */
    /* line into it as 1. NumOfprm, 2. pointer to block	of pointers.	 */
    for (i = (int) strlen(CtrlStr) - 1; i > 0 && !ISSPACE(CtrlStr[i]); i--);
    if (!ISCTRLCHAR(CtrlStr[i + 2])) {
	GASetParamCount(CtrlStr, i, ParamCount);   /* Point in correct prm.. */
	*Parameters[(*ParamCount)++] = *argc;
	GAByteCopy((char *) Parameters[(*ParamCount)++], (char *) argv,
							sizeof(char *));
    }

    i =	0;
    while (++i < (int) strlen(CtrlStrCopy))
	if ((CtrlStrCopy[i] == '-') && (CtrlStrCopy[i-1] == '!')) {
	    GAErrorToken = LocalToken;
	    LocalToken[1] = CtrlStrCopy[i-2];	    /* Set the correct flag. */
	    return CMD_ERR_ALL_SATIS;
	}

    return ARG_OK;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to update the	parameters according to	the given Option.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Parameters:      Where new option should update the variables.	     *
*   ParamCount:      Index into Parameters where to update using new option. *
*   Option:          New command line option.                                *
*   CtrlStrCopy:     A fresh, unmodified copy of given control string.       *
*   CtrlStr:         A bashed, marked with specified option copy of the      *
*   argc, argv:      Current command line state.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             Return code based on success (ARG_OK), or failure.      *
*****************************************************************************/
static int GAUpdateParameters(int *Parameters[],
			      int *ParamCount,
			      char *Option,
			      char *CtrlStrCopy,
			      char *CtrlStr,
			      int *argc,
			      char ***argv)
{
    int	i,
	BooleanTrue = Option[2] != '-';

    if (Option[0] != '-') {
	GAErrorToken = Option;
	return CMD_ERR_NOT_AN_OPT;
    }
    i =	0;			    /* Scan the CtrlStrCopy for that option: */
    while (i + 2 < (int) strlen(CtrlStrCopy)) {
	if ((CtrlStrCopy[i] == Option[1]) && (ISCTRLCHAR(CtrlStrCopy[i + 1]))
	    && (CtrlStrCopy[i+2] == '-')) {
	    /* We found	that option! */
	    break;
	}
	i++;
    }
    if (i + 2 >= (int) strlen(CtrlStrCopy)) {
	GAErrorToken = Option;
	return CMD_ERR_NO_SUCH_OPT;
    }

    /* If we are here, then we found that option in CtrlStr - Strip it off:  */
    CtrlStrCopy[i] = CtrlStrCopy[i + 1] = CtrlStrCopy[i + 2] = (char) ' ';
    GASetParamCount(CtrlStr, i, ParamCount);/*Set it to point in correct prm.*/
    i += 3;

    /* Set Boolean flag for that option. */
    *Parameters[(*ParamCount)++] = BooleanTrue;
    if (ISSPACE(CtrlStrCopy[i]))
	return ARG_OK;			   /* Only a Boolean flag is needed. */

    /* Skip the	text between the Boolean option and data follows: */
    while (!ISCTRLCHAR(CtrlStrCopy[i]))
	i++;
    /* Get the parameters and return the appropriete return code: */
    return GAGetParmeters(Parameters, ParamCount, &CtrlStrCopy[i],
							  Option, argc, argv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get parameters from command line argc/v according to the        *
* prescribed CtrlStr.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Parameters:      Where new option should update the variables.	     *
*   ParamCount:      Index into Parameters where to update using new option. *
*   CtrlStrCopy:     A fresh, unmodified copy of given control string.       *
*   Option:          New command line option.                                *
*   argc, argv:      Current command line state.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             Return code based on success (ARG_OK), or failure.      *
*****************************************************************************/
static int GAGetParmeters(int *Parameters[],
			  int *ParamCount,
			  char *CtrlStrCopy,
			  char *Option,
			  int *argc,
			  char ***argv)
{
    int	ScanRes, IData,
	i = 0;
    long LData;
#ifdef IRIT_DOUBLE
    double DData;
#else
    float FData;
#endif /* IRIT_DOUBLE */

    while (!(ISSPACE(CtrlStrCopy[i]))) {
	if (*argc == 0) {
	    GAErrorToken = Option;
	    return CMD_ERR_NUM_READ;
	}

	switch (CtrlStrCopy[i+1]) {
	    case 'd':				     /* Get signed integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%d", &IData)) == 1)
		    *((int *) Parameters[(*ParamCount)++]) = IData;
		break;
	    case 'u':				   /* Get unsigned integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%u", &IData)) == 1)
		    *((int *) Parameters[(*ParamCount)++]) = IData;
		break;
	    case 'x':					/* Get hex integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%x", &IData)) == 1)
		    *((int *) Parameters[(*ParamCount)++]) = IData;
		break;
	    case 'o':				      /* Get octal integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%o", &IData)) == 1)
		    *((int *) Parameters[(*ParamCount)++]) = IData;
		break;
	    case 'D':				     /* Get signed integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%ld", &LData)) == 1)
		    *((long *) Parameters[(*ParamCount)++]) = LData;
		break;
	    case 'U':				   /* Get unsigned integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%lu", &LData)) == 1)
		    *((long *) Parameters[(*ParamCount)++]) = LData;
		break;
	    case 'X':					/* Get hex integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%lx", &LData)) == 1)
		    *((long *) Parameters[(*ParamCount)++]) = LData;
		break;
	    case 'O':				      /* Get octal integers. */
		if ((ScanRes = sscanf(*((*argv)++), "%lo", &LData)) == 1)
		    *((long *) Parameters[(*ParamCount)++]) = LData;
		break;
#ifdef IRIT_DOUBLE
	    case 'f':
	        assert(0);			         /* 'f' is obsolete. */
	    case 'F':				       /* Get double number. */
		if ((ScanRes = sscanf(*((*argv)++), "%lf", &DData)) == 1)
		    *((double *) Parameters[(*ParamCount)++]) = DData;
		break;
#else
	    case 'f':
	        assert(0);			         /* 'f' is obsolete. */
	    case 'F':					/* Get float number. */
		if ((ScanRes = sscanf(*((*argv)++), "%f", &FData)) == 1)
		    *((float *) Parameters[(*ParamCount)++]) = FData;
		break;
#endif /* IRIT_DOUBLE */
	    case 's':					  /* It as a string. */
		ScanRes	= 1;				     /* Allways O.K. */
		GAByteCopy((char *) Parameters[(*ParamCount)++],
					(char *) ((*argv)++), sizeof(char *));
		break;
	    case '*':			     /* Get few parameters into one: */
		ScanRes	= GAGetMultiParmeters(Parameters, ParamCount,
						  &CtrlStrCopy[i], argc, argv);
		if ((ScanRes ==	0) && (CtrlStrCopy[i] == '!')) {
		    GAErrorToken = Option;
		    return CMD_ERR_WILD_EMPTY;
		}
		break;
	    default:
		ScanRes = 0;               /* Make optimizer warning silent. */
	}
	/* If reading fails and	this number is a must (!) then error: */
	if (ScanRes == 0) {
	    if (CtrlStrCopy[i] == '!') {
		GAErrorToken = Option;
		return CMD_ERR_NUM_READ;
	    }
	    else {
		/* It is optional but it is not there - quit. */
		(*argv)--;
		return ARG_OK;
	    }
	}
	if (CtrlStrCopy[i + 1] != '*') {
	    (*argc)--;	     /* Everything is OK - update to next parameter: */
	    i += 2;			 /* Skip to next parameter (if any). */
	}
	else
	    i += 3;				       /* Skip the '*' also! */
    }

    return ARG_OK;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get several parameters into one pointer such that the returned  *
* pointer actually points on a block of	pointers on the	parameters...	     *
*   For example *F means a pointer to pointers on double.		     *
*   Returns number of parameters actually being read.			     *
*   This routine assumes that all pointers (on any kind of scalar) has the   *
* same size (and the union below is totally ovelapped between dif. arrays).  *
*                                                                            *
* PARAMETERS:                                                                *
*   Parameters:      Where new option should update the variables.	     *
*   ParamCount:      Index into Parameters where to update using new option. *
*   CtrlStrCopy:     A fresh, unmodified copy of given control string.       *
*   argc, argv:      Current command line state.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             Number of parameters actually being read.               *
*****************************************************************************/
static int GAGetMultiParmeters(int *Parameters[],
			       int *ParamCount,
			       char *CtrlStrCopy,
			       int *argc,
			       char ***argv)
{
    int	ScanRes, **Pmain, **Ptemp,
	i = 0,
	NumOfPrm = 0;
    union TmpArray {    /* Save here the temporary data before copying it to */
	int    *IntArray[MAX_PARAM];	      /* the returned pointer block. */
	long   *LngArray[MAX_PARAM];
	float  *FltArray[MAX_PARAM];
	double *DblArray[MAX_PARAM];
	char   *ChrArray[MAX_PARAM];
    } TmpArray;

    do {
	switch (CtrlStrCopy[2]) {   /* CtrlStr == '!*?' or '%*?' where ? is. */
	    case 'd':			   /* Format to read the parameters: */
		TmpArray.IntArray[NumOfPrm] =
		    (int *) IritMalloc(sizeof(int));
		ScanRes	= sscanf(*((*argv)++), "%d",
				       (int *) TmpArray.IntArray[NumOfPrm++]);
		break;
	    case 'u':
		TmpArray.IntArray[NumOfPrm] =
		    (int *) IritMalloc(sizeof(int));
		ScanRes	= sscanf(*((*argv)++), "%u",
			      (unsigned	int *) TmpArray.IntArray[NumOfPrm++]);
		break;
	    case 'o':
		TmpArray.IntArray[NumOfPrm] =
		    (int *) IritMalloc(sizeof(int));
		ScanRes	= sscanf(*((*argv)++), "%o",
				       (int *) TmpArray.IntArray[NumOfPrm++]);
		break;
	    case 'x':
		TmpArray.IntArray[NumOfPrm] =
		    (int *) IritMalloc(sizeof(int));
		ScanRes	= sscanf(*((*argv)++), "%x",
				       (int *) TmpArray.IntArray[NumOfPrm++]);
		break;
	    case 'D':
		TmpArray.LngArray[NumOfPrm] = 
		    (long *) IritMalloc(sizeof(long));
		ScanRes	= sscanf(*((*argv)++), "%ld",
				      (long *) TmpArray.LngArray[NumOfPrm++]);
		break;
	    case 'U':
		TmpArray.LngArray[NumOfPrm] = 
		    (long *) IritMalloc(sizeof(long));
		ScanRes	= sscanf(*((*argv)++), "%lu",
			     (unsigned long *) TmpArray.LngArray[NumOfPrm++]);
		break;
	    case 'O':
		TmpArray.LngArray[NumOfPrm] = 
		    (long *) IritMalloc(sizeof(long));
		ScanRes	= sscanf(*((*argv)++), "%lo",
				      (long *) TmpArray.LngArray[NumOfPrm++]);
		break;
	    case 'X':
		TmpArray.LngArray[NumOfPrm] = 
		    (long *) IritMalloc(sizeof(long));
		ScanRes	= sscanf(*((*argv)++), "%lx",
				      (long *) TmpArray.LngArray[NumOfPrm++]);
		break;
#ifdef IRIT_DOUBLE
	    case 'f':
	        assert(0);			         /* 'f' is obsolete. */
	    case 'F':
		TmpArray.DblArray[NumOfPrm] =
		    (double *) IritMalloc(sizeof(double));
		ScanRes	= sscanf(*((*argv)++), "%lf",
				    (double *) TmpArray.DblArray[NumOfPrm++]);
		break;
#else
	    case 'f':
	        assert(0);			         /* 'f' is obsolete. */
	    case 'F':
		TmpArray.FltArray[NumOfPrm] =
		    (float *) IritMalloc(sizeof(float));
		ScanRes	= sscanf(*((*argv)++), "%f",
				     (float *) TmpArray.FltArray[NumOfPrm++]);
		break;
#endif /* IRIT_DOUBLE */
	    case 's':
		while ((*argc) && ((**argv)[0] != '-'))	{
		    TmpArray.ChrArray[NumOfPrm++] = *((*argv)++);
		    (*argc)--;
		}
		ScanRes	= 0;		       /* Force quit from do - loop. */
		NumOfPrm++;	    /* Updated again immediately after loop! */
		(*argv)++;					       /* "" */
		break;
	    default:
		ScanRes = 0;               /* Make optimizer warning silent. */
	}
	(*argc)--;
    }
    while (ScanRes == 1);		  /* Exactly one parameter was read. */
    (*argv)--;
    NumOfPrm--;
    (*argc)++;

    /* Now allocate the	block with the exact size, and set it: */
    Ptemp = Pmain = (int **) IritMalloc((unsigned) (NumOfPrm+1) * sizeof(int *));
    /* And here	we use the assumption that all pointers	are the	same: */
    for (i = 0; i < NumOfPrm; i++)
	*Ptemp++ = TmpArray.IntArray[i];
    *Ptemp = NULL;		       /* Close the block with NULL pointer. */

    /* That it save the	number of parameters read as first parameter to	*/
    /* return and the pointer to the block as second, and return:	*/
    *Parameters[(*ParamCount)++] = NumOfPrm;
    GAByteCopy((char *)	Parameters[(*ParamCount)++], (char *) &Pmain,
							     sizeof(char *));
    return NumOfPrm;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to scan the CtrlStr, upto Max	and count the number of	parameters   *
* to that point as,							     *
* 1. Each option is counted as one parameter - Boolean variable	(int)	     *
* 2. Within an option, each %? or !? is	counted	once - pointer to something. *
* 3. Within an option, %*? or !*? is counted twice - one for item count	     *
*    and one for pointer to block of pointers.				     *
* ALL variables are passed by address and we assume pointers (addresses)     *
* are all the same fixed size.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   CtrlStr:         A fresh, unmodified copy of given control string.       *
*   Max:             Maximum number of options to scan.                      *
*   ParamCOunt:      Where resulting count is saved.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GASetParamCount(char *CtrlStr, int Max, int *ParamCount)
{
    int	i;

    *ParamCount	= 0;
    for (i = 0; i < Max; i++)
	if (ISCTRLCHAR(CtrlStr[i])) {
	    if (CtrlStr[i+1] == '*')
		*ParamCount += 2;
	    else
		(*ParamCount)++;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy exactly n bytes from Src to Dst.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Dst:      Destination address.                                           *
*   Src:      Source address.                                                *
*   n:        Number of bytes to copy.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GAByteCopy(char *Dst, char *Src, unsigned n)
{
    while (n--)
	*(Dst++) = *(Src++);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to verify if more options (i.e. first char == '-') exist in the    *
* given	command line list list argc, argv (excludes just '-' which signals   *
* file stdin, though).							     *
*                                                                            *
* PARAMETERS:                                                                *
*   argc, argv:      Current command line state.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             TRUE if more options exists, FALSE otherwise.           *
*****************************************************************************/
static int GAOptionExists(int argc, char **argv)
{
    while (argc--) {
	if ((*argv)[0] == '-' && (*argv)[1] != 0)
	    return TRUE;
	argv++;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print a description of an error to a string.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Error:       Error type as returned by GAGetArgs.                        M
*   OutStr:	 Where to place the error message.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:	The error message string (OutStr).                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   GAPrintErrMsg, GAStringHowTo, GAGetArgs                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GAStringErrMsg, command line arguments                                   M
*****************************************************************************/
char *GAStringErrMsg(int Error, char *OutStr)
{
    const char *p;

    switch (Error) {
	case 0:
	    p = IRIT_EXP_STR("Undefined error");
	    break;
	case CMD_ERR_NOT_AN_OPT:
	    p = IRIT_EXP_STR("None option found");
	    break;
	case CMD_ERR_NO_SUCH_OPT:
	    p = IRIT_EXP_STR("Undefined option found");
	    break;
	case CMD_ERR_WILD_EMPTY:
	    p = IRIT_EXP_STR("Empty input for '!*?' seq.");
	    break;
	case CMD_ERR_NUM_READ:
	    p = IRIT_EXP_STR("Failed on reading number");
	    break;
	case CMD_ERR_ALL_SATIS:
	    p = IRIT_EXP_STR("Fail to satisfy");
	    break;
        default:
	    p = "";
	    assert(0);
	    break;
    }
    sprintf(OutStr,
	    IRIT_EXP_STR("Error in command line parsing - %s - '%s'.\n"),
	    p, GAErrorToken);

    return OutStr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print a description of an error, for this module:		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Error:       Error type as returned by GAGetArgs.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GAStringErrMsg, GAPrintHowTo, GAGetArgs                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GAPrintErrMsg, command line arguments                                    M
*****************************************************************************/
void GAPrintErrMsg(int Error)
{
    char Str[IRIT_LINE_LEN_VLONG];

    IRIT_WARNING_MSG_PRINTF("%s", GAStringErrMsg(Error, Str));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to write the correct format of command line allowed, to a string.  M
*   For example, for the following control string,			     M
*    "Example1  i%-OneInteger!d  s%-Strings!*s  j%-  k!-Double!F  Files"     V
*   This routine will write						     M
* Example1 [-i OneIngeter] [-s Strings...] [-j]	-k Double Files...	     V
*                                                                            *
* PARAMETERS:                                                                M
*   CtrlStr:    Defining the types/options to expect in the command line.    M
*   OutStr:	Where to place the how to message.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:	The how-to message string (OutStr).                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   GAPrintHowTo, GAStringErrMsg, GAGetArgs                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GAStringHowTo, command line arguments                                    M
*****************************************************************************/
char *GAStringHowTo(const char *CtrlStr, char *OutStr)
{
    int	SpaceFlag,
	i = 0;

    sprintf(OutStr, "Usage: ");
    /* Print program name - first word in ctrl.	str. (optional): */
    while (!(ISSPACE(CtrlStr[i])) && (!ISCTRLCHAR(CtrlStr[i+1])))
	sprintf(&OutStr[strlen(OutStr)], "%c", CtrlStr[i++]);

    while (i < (int) strlen(CtrlStr))	{
	while ((ISSPACE(CtrlStr[i])) &&	(i < (int) strlen(CtrlStr)))
	    i++;

	switch (CtrlStr[i+1]) {
	    case '%':
		sprintf(&OutStr[strlen(OutStr)], " [-%c", CtrlStr[i++]);
		i += 2;		     /* Skip the '%-' or '!- after the char! */
		SpaceFlag = TRUE;
		while (!ISCTRLCHAR(CtrlStr[i]) &&
		       (i < (int) strlen(CtrlStr)) &&
		       (!ISSPACE(CtrlStr[i])))
		    if (SpaceFlag) {
			if (CtrlStr[i++] == SPACE_CHAR)
			    sprintf(&OutStr[strlen(OutStr)], " ");
			else
			    sprintf(&OutStr[strlen(OutStr)], " %c",
					    CtrlStr[i-1]);
			SpaceFlag = FALSE;
		    }
		    else if (CtrlStr[i++] == SPACE_CHAR)
			sprintf(&OutStr[strlen(OutStr)], " ");
		    else
			sprintf(&OutStr[strlen(OutStr)], "%c", CtrlStr[i-1]);
		while (!ISSPACE(CtrlStr[i]) && (i < (int) strlen(CtrlStr))) {
		    if (CtrlStr[i] == '*')
			sprintf(&OutStr[strlen(OutStr)], "...");
		    i++;			     /* Skip the rest of it. */
		}
		sprintf(&OutStr[strlen(OutStr)], "]");
		break;
	    case '!':
		sprintf(&OutStr[strlen(OutStr)], " -%c", CtrlStr[i++]);
		i += 2;		     /* Skip the '%-' or '!- after the char! */
		SpaceFlag = TRUE;
		while (!ISCTRLCHAR(CtrlStr[i]) &&
		       (i < (int) strlen(CtrlStr)) &&
		       (!ISSPACE(CtrlStr[i])))
		    if (SpaceFlag) {
			if (CtrlStr[i++] == SPACE_CHAR)
			    sprintf(&OutStr[strlen(OutStr)], " ");
			else
			    sprintf(&OutStr[strlen(OutStr)], " %c",
					    CtrlStr[i-1]);
			SpaceFlag = FALSE;
		    }
		    else if (CtrlStr[i++] == SPACE_CHAR)
			sprintf(&OutStr[strlen(OutStr)], " ");
		    else
		        sprintf(&OutStr[strlen(OutStr)], "%c", CtrlStr[i-1]);
		while (!ISSPACE(CtrlStr[i]) && (i < (int) strlen(CtrlStr))) {
		    if (CtrlStr[i] == '*')
			sprintf(&OutStr[strlen(OutStr)], "...");
		    i++;			     /* Skip the rest of it. */
		}
		break;
	    default:		       /* Not checked, but must be last one! */
		sprintf(&OutStr[strlen(OutStr)], " ");
		while (!ISSPACE(CtrlStr[i]) && (i < (int) strlen(CtrlStr)) &&
		       !ISCTRLCHAR(CtrlStr[i]))
		    sprintf(&OutStr[strlen(OutStr)], "%c", CtrlStr[i++]);
		sprintf(&OutStr[strlen(OutStr)], "\n");
		return OutStr;
	}
    }
    sprintf(&OutStr[strlen(OutStr)], "\n");

    return OutStr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to print the correct format of command line allowed.               M
*   For example, for the following control string,			     M
*    "Example1  i%-OneInteger!d  s%-Strings!*s  j%-  k!-Double!F  Files"     V
*   This routine will print						     M
* Example1 [-i OneIngeter] [-s Strings...] [-j]	-k Double Files...	     V
*                                                                            *
* PARAMETERS:                                                                M
*   CtrlStr:    Defining the types/options to expect in the command line.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GAPrintErrMsg, GAStringHowTo, GAGetArgs                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GAPrintHowTo, command line arguments                                     M
*****************************************************************************/
void GAPrintHowTo(const char *CtrlStr)
{
    char Str[IRIT_LINE_LEN_VLONG];

    IRIT_WARNING_MSG_PRINTF("%s", GAStringHowTo(CtrlStr, Str));
}
