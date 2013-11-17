#include <string.h>
#include <proto/dos.h>

int
putenv(char *s)
{
  char *delim;

  delim = strchr(s, '=');
  if (!delim) {
    return 1;
  }
  *delim = 0;
  SetVar(s, delim+1, -1, GVF_GLOBAL_ONLY);
  *delim = '=';
  return 0;
}
