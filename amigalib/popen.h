/*
** popen.h -- prototypes for pipe functions
*/
#ifdef __SASC
FILE *popen( const char *, const char * );
#else
#include <stdio.h>
#endif
