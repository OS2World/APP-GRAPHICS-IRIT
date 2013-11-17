/*****************************************************************************
* Replacements for missing functions.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Aug. 1991   *
*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __WINCE__
#   include <time.h>
#endif /* __WINCE__ */

#ifdef OS2GCC
#   define INCL_DOSPROCESS
#   include <os2.h>
#endif /* OS2GCC */
#ifdef SGINAP
#   include <limits.h>
#endif /* SGINAP */
#if defined(__WINNT__) || defined(__WINCE__)
#   include <windows.h>
#else
#   include <unistd.h>
#   include <sys/types.h>
#   ifndef AMIGA
#       include <sys/times.h>
#       include <sys/param.h>
#   else
#       include <proto/dos.h>
#   endif
#   ifndef HZ
#       define HZ 60
#   endif /* HZ */
#endif /* __WINNT__ || __WINCE__ */

#include "irit_sm.h"
#include "misc_loc.h"
#include "extra_fn.h"

IRIT_STATIC_DATA int
    GlblWasIritRandomInit = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to duplicate a string. Exists in some computer environments.       M
*                                                                            *
* PARAMETERS:                                                                M
*   s:        String to duplicate.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:   Duplicated string.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritStrdup, strdup                                                       M
*****************************************************************************/
char *IritStrdup(const char *s)
{
    char
        *p = IritMalloc((unsigned int) strlen(s) + 1);

    strcpy(p, s);

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert all lower case chars into uppercase, in place.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   s:        String to convert to uppercase.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:   Reference to s.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritStrUpper, uppercase                                                  M
*****************************************************************************/
char *IritStrUpper(char *s)
{
    int i;

    for (i = 0; i < (int) strlen(s); i++)
	if (islower(s[i]))
	    s[i] = toupper(s[i]);

    return s;
}

#ifdef ITIMERVAL
/*****************************************************************************
* DESCRIPTION:                                                               *
* Dummy routine to catch signals for IritSleep below.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrapSignal(void)
{
}
#endif /* ITIMERVAL */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to force a process to sleep.	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MilliSeconds:   Sleeping time required, in miliseconds.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSleep, sleep                                                         M
*****************************************************************************/
void IritSleep(int MilliSeconds)
{
#ifdef NANOSLEEP
    struct timespec rqtp;

    rqtp.tv_sec = MilliSeconds / 1000;
    rqtp.tv_nsec = (MilliSeconds % 1000) * 1000000;
    nanosleep(&rqtp, NULL);
#else
#ifdef USLEEP
    usleep(MilliSeconds * 1000);
#else
#ifdef SGINAP
    sginap((long) (IRIT_MAX(CLK_TCK * MilliSeconds / 1000, 1)));
#else
#ifdef OS2GCC
    DosSleep(MilliSeconds);
#else
#if defined(__WINNT__) || defined(__WINCE__)
    Sleep(MilliSeconds);
#else
#ifdef USLEEP_SELECT
    int nfds = 0,
	readfds = 0,
	writefds = 0,
	exceptfds = 0;
    struct timeval Timer;

    Timer.tv_sec = MilliSeconds / 1000;
    Timer.tv_usec = (MilliSeconds % 1000) * 1000;

    if (select(nfds, &readfds, &writefds, &exceptfds, &Timer) < 0)
	perror( "usleep (select) failed" );
#else
#ifdef ITIMERVAL
    /* Sometimes fail on hpux (sleeps forever in sigpause). Use with care. */
    struct itimerval Tmr;
    void (*OldTrapSignal)(void);

    Tmr.it_interval.tv_sec = 0;
    Tmr.it_interval.tv_usec = 0;
    Tmr.it_value.tv_sec = MilliSeconds / 1000;
    Tmr.it_value.tv_usec = (MilliSeconds % 1000) * 1000;
    OldTrapSignal = (void (*)(void)) signal(SIGALRM,
					    (void (*)(void)) TrapSignal);
    setitimer(ITIMER_REAL, &Tmr, NULL);
    sigpause(~sigmask(SIGALRM));
    signal(SIGALRM, OldTrapSignal);
#else /* No way to milli-sleep. */
    int i;

    if (MilliSeconds >= 1000) {
	sleep(MilliSeconds / 1000);
	MilliSeconds = MilliSeconds % 1000;
    }
    for (i = MilliSeconds * 10000; i > 0; i--);
#endif /* ITIMERVAL */
#endif /* USLEEP_SELECT */
#endif /* __WINNT__ */
#endif /* OS2GCC */
#endif /* SGINAP */
#endif /* USLEEP */
#endif /* NANOSLEEP */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Provides an approximated comparison between two strings.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Str1, Str2: The two strings to compare.                                  M
*   IgnoreCase: TRUE to ignore case, FALSE to consider as different.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Perfect match returns 1.0, otherwise match estimation       M
*		 between 0.0 and 1.0.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritApproxStrStrMatch                                                    M
*****************************************************************************/
IrtRType IritApproxStrStrMatch(const char *Str1,
			       const char *Str2,
			       int IgnoreCase)
{
    int i,
	Len1 = (int) strlen(Str1),
	Len2 = (int) strlen(Str2);
    char
	*Str1L = IritStrdup(Str1),
	*Str2L = IritStrdup(Str2);
    IrtRType RetVal;

    if (IgnoreCase) {
	for (i = 0; i < Len1; i++)
	    if (isupper(Str1L[i]))
	        Str1L[i] = tolower(Str1L[i]);
	for (i = 0; i < Len2; i++)
	    if (isupper(Str2L[i]))
	        Str2L[i] = tolower(Str2L[i]);
    }

    if (strcmp(Str1L, Str2L) == 0) {                      /* Perfect match. */
        RetVal = 1.0;
    }
    else {
        RetVal = 0.0;

	/* Half a match for inclusion, less so if single chars' inclusion. */
        if (strstr(Str1L, Str2L))
	    RetVal += 0.5;
	else {
	    for (i = 0; i < Len1; i++)
		if (strchr(Str2L, Str1L[i]))
		    RetVal += 0.5 / (Len1 + 1.0);
	}
	if (strstr(Str2L, Str1L))
	    RetVal += 0.5;
	else {
	    for (i = 0; i < Len2; i++)
		if (strchr(Str1L, Str2L[i]))
		    RetVal += 0.5 / (Len2 + 1.0);
	}
    }

    IritFree(Str1L);
    IritFree(Str2L);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to initialize the random number generator.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Seed:       To initialize the random number generator with.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRandomInit, random numbers                                           M
*****************************************************************************/
void IritRandomInit(long Seed)
{
#ifdef RANDOM_IRIT
    MtIritSrandom(Seed);
#else
#ifdef RAND
    srand(Seed);
#else
#ifdef RAND48
    srand48(Seed);
#else
    srandom(Seed);
#endif /* RAND48 */
#endif /* RAND */
#endif /* RANDOM_IRIT */

    GlblWasIritRandomInit = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute a random number in a specified range.		     M
*   See also IritRandomInit.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Min:      Minimum range of random number requested.                      M
*   Max:      Maximum range of random number requested.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   A random number between Min andMax.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRandom, random numbers                                               M
*****************************************************************************/
IrtRType IritRandom(IrtRType Min, IrtRType Max)
{
    long R;

    if (!GlblWasIritRandomInit) {
        IritRandomInit(1964);
	GlblWasIritRandomInit = TRUE;
    }

#ifdef RANDOM_IRIT
    R = MtIritRandom();
#else
#ifdef RAND
    R = rand();
#else
#ifdef RAND48
    R = lrand48();
#else
    R = random();
#endif /* RAND48 */
#endif /* RAND */
#endif /* RANDOM_IRIT */

#if defined(OS2GCC) || defined(sgi) || defined(__WINNT__)
    return Min + (Max - Min) * ((double) (R & RAND_MAX)) / ((double) RAND_MAX);
#else
    return Min + (Max - Min) * ((double) (R & 0x7fffffff)) / ((double) 0x7fffffff);
#endif /* OS2GCC || sgi */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the cpu time of the running process.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Reset:     If TRUE, clock is reset back to zero.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   CPU time since last reset or beginning of execution.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritCPUTime, time                                                        M
*****************************************************************************/
IrtRType IritCPUTime(int Reset)
{
    IRIT_STATIC_DATA IrtRType
	LastTime = 0.0;
    IrtRType Time;

    if (Reset) {
#ifndef AMIGA
#ifdef TIMES
	struct tms tim;

	times(&tim);

	LastTime = (tim.tms_utime + tim.tms_stime) / ((IrtRType) HZ);
#else
#ifdef __WINNT__
	LastTime = (IrtRType) GetTickCount();
#else
	LastTime = time(NULL);                /* No real timing routines!? */
#endif /* __WINNT__ */
#endif /* TIMES */
#else
	struct DateStamp ds;
	
	DateStamp(&ds);
	LastTime = (double) ds.ds_Days * (24.0 * 60.0 * 60.0) +
		   (double) ds.ds_Minute * 60.0 + (double) ds.ds_Tick / 50.0;
#endif	/* AMIGA */

	return 0.0;
    }

#ifndef AMIGA
#ifdef TIMES
    {
	struct tms tim;

	times(&tim);

	Time = (tim.tms_utime + tim.tms_stime) / ((IrtRType) HZ) - LastTime;
    }
#else
#ifdef __WINNT__
    {
	Time = (((IrtRType) GetTickCount()) - LastTime) * 0.001;
    }
#else
    Time = time(NULL) - LastTime;
#endif /* __WINNT__ */
#endif /* TIMES */
#else
    {
	struct DateStamp ds;
	
	DateStamp(&ds);
	Time = (double) ds.ds_Days * (24.0 * 60.0 * 60.0) +
	       (double) ds.ds_Minute * 60.0 + (double) ds.ds_Tick / 50.0 -
	       LastTime;
    }
#endif	/* AMIGA */

    return Time;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create and return a string describing current date and time.    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:    A string describing current date and time.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRealTimeDate, date, time                                             M
*****************************************************************************/
char *IritRealTimeDate(void)
{
    IRIT_STATIC_DATA char StrTime[IRIT_LINE_LEN];
    int i;
    time_t
	t = (time_t) time(NULL);

    strncpy(StrTime, ctime(&t), IRIT_LINE_LEN - 1);

    /* Remove trailing EOL. */
    for (i = (int) strlen(StrTime) - 1; i >= 0 && StrTime[i] < ' '; i--);
    StrTime[i + 1] = 0;

    return StrTime;
}

#if !(defined(AMIGA) && defined(__SASC))
/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to move a block in memory. Unlike memcpy/bcopy, this routine       M
* should support overlaying blocks. This stupid implemetation will copy it   M
* twice - to a temporary block and back again. The temporary block size will M
* be allocated by demand.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Src:     Of block to copy.                                               M
*   Dest:    Of block to copy.                                               M
*   Len:     Of block to copy.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   movmem, copy                                                             M
*****************************************************************************/
void movmem(VoidPtr Src, VoidPtr Dest, int Len)
{
    VoidPtr
	p = IritMalloc(Len);

    memcpy(p, Src, Len);
    memcpy(Dest, p, Len);

    IritFree(p);
}
#endif /* !(defined(AMIGA) && defined(__SASC)) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* RRoutine to search for a given file name.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:     Of file to search for.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:   Complete file name of Name.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   searchpath                                                               M
*****************************************************************************/
const char *searchpath(const char *Name)
{
    IRIT_STATIC_DATA char FullPath[IRIT_LINE_LEN_LONG];
    char *p;
    FILE *f;

    if ((f = fopen(Name, "r")) != NULL) {
        fclose(f);  /* Assume a full path already. */
        return Name;
    }

    if ((p = getenv("IRIT_PATH")) != NULL) {
	strcpy(FullPath, p);
	if (p[strlen(p) - 1] != '/' && p[strlen(p) - 1] != '\\')
	    strcat(FullPath, "/");
	strcat(FullPath, Name);
    }
    else {
	IRIT_STATIC_DATA int
	    Printed = FALSE;
	
	strcpy(FullPath, Name);

	if (!Printed) {
	    IRIT_WARNING_MSG("IRIT_PATH env. not set. Only current directory is being searched.\n");
	    Printed = TRUE;
	}
    }

    return FullPath;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to search for a Pattern (no regular expression) in s. Returns      M
* address in s of first occurrence of Pattern, NULL if non found. case	     M
* insensitive                                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   s:          To search for Pattern in.                                    M
*   Pattern:    To search in s.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:  Address in s where Pattern was 1st found, NULL otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritStrIStr                                                              M
*****************************************************************************/
const char *IritStrIStr(const char *s, const char *Pattern)
{
  int Len = (int) strlen(Pattern);
    const char *p,
	c0l = isupper(Pattern[0]) ? tolower(Pattern[0]) : Pattern[0],
	c0u = islower(Pattern[0]) ? toupper(Pattern[0]) : Pattern[0];

    for (p = s; *p != 0; p++) {
	if (*p == c0l || *p == c0u) {
	    if (strnicmp(p, Pattern, Len) == 0)
		return p;
	}
    }

    return NULL;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   A find (Src) and replace (into Dst) function.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   s:     Input string.						     M
*   Src:   Pattern to look for in S and substitute with Dst.		     M
*   Dst:   Replacement patter for Src.                                       M
*   CaseInsensitive:  TRUE for case insensitive, FALSE for case sensitive.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:  new string, allocated dynamically.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSubstStr                                                             M
*****************************************************************************/
char *IritSubstStr(const char *s,
		   const char *Src,
		   const char *Dst,
		   int CaseInsensitive)
{
    char *CrntS, *NewS;

    /* Sanity check - no recursive definitions allowed. */
    if (CaseInsensitive ? IritStrIStr(Dst, Src) : strstr(Dst, Src))
        return NULL;
    
    CrntS = IritStrdup(s);
    while (TRUE) {
        const char *p;

	if ((p = (CaseInsensitive ? IritStrIStr(CrntS, Src)
			          : strstr(CrntS, Src))) == NULL)
	    return CrntS;

	NewS = IritMalloc((unsigned int) (strlen(CrntS) + strlen(Dst) + 1));
	*((char *) p) = 0;
	strcpy(NewS, CrntS);
	strcat(NewS, Dst);
	strcat(NewS, &p[strlen(Src)]);
	IritFree(CrntS);

	CrntS = NewS;
    }
}

#ifdef STRICMP
/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compare two strings, ignoring case, up to given length.         M
*                                                                            *
* PARAMETERS:                                                                M
*   s1, s2:  The two strings to compare.                                     M
*   n:       maximum number of characters to compare.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     <0, 0, >0 according to the relation between s1 and s2.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   strnicmp                                                                 M
*****************************************************************************/
int strnicmp(const char *s1, const char *s2, int n)
{
    /* Use to be simply 'return strncasecmp(s1, s2, n);' but seems that some */
    /* unix systems (sun4?) does have it so here it is explicitly.	     */
    /* Written by Reiner Wilhelms <reiner@shs.ohio-state.edu>.		     */
    int i;
    char c1, c2;

    for (i = 0; i < n; i++) {
        if (islower(s1[i]))
	    c1 = toupper(s1[i]);
        else
	    c1 = s1[i];
        if (islower(s2[i]))
	    c2 = toupper(s2[i]);
        else
	    c2 = s2[i];

        if (c1 != c2) {
	    if (c1 > c2)
		return 1;
            if (c1 < c2)
		return -1;
	}
    }
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compare two strings, ignoring case.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   s1, s2:  The two strings to compare.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     <0, 0, >0 according to the relation between s1 and s2.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   stricmp                                                                  M
*****************************************************************************/
int stricmp(const char *s1, const char *s2)
{
    int i;
    char *u1, *u2;

    if (s1 == NULL)
	return s2 != NULL;
    else if (s2 == NULL)
	return 1;

    u1 = IritStrdup(s1);
    u2 = IritStrdup(s2);

    for (i = 0; i < (int) strlen(u1); i++)
	if (islower(u1[i]))
	    u1[i] = toupper(u1[i]);
    for (i = 0; i < (int) strlen(u2); i++)
	if (islower(u2[i]))
	    u2[i] = toupper(u2[i]);

    i = strcmp(u1, u2);

    IritFree(u1);
    IritFree(u2);

    return i;
}
#endif /* STRICMP */

#ifdef STRSTR
/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to search for a Pattern (no regular expression) in s. Returns      M
* address in s of first occurance of Pattern, NULL if non found.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   s:          To search for Pattern in.                                    M
*   Pattern:    To search in s.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:     Address in s where Pattern was first found, NULL otherwise.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   strstr                                                                   M
*****************************************************************************/
char *strstr(const char *s, const char *Pattern)
{
    int Len = strlen(Pattern);
    char
	*p = (char *) s;

    while (p = strchr(p, Pattern[0]))
	if (strncmp(p, Pattern, Len) == 0)
	    return p;
	else
	    p++;

    return NULL;
}
#endif /* STRSTR */

#ifdef GETCWD
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get current working directory - BSD4.3 style.			     *
*                                                                            *
* PARAMETERS:                                                                M
*   s:       Where to save current working direction.                        M
*   Len:     Length of s.                                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:  Same as s.                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   getcwd                                                                   M
*****************************************************************************/
char *getcwd(char *s, int Len)
{
    getwd(s);

    return s;
}
#endif /* GETCWD */
