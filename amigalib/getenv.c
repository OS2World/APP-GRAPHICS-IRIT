#include <string.h>
#include <stdlib.h>
#include <proto/dos.h>

char *
getenv(const char *s)
{
  STATIC_DATA char buf[80], *buf2, *env;
  int len, err;

  len = GetVar(s, buf, sizeof(buf), GVF_GLOBAL_ONLY);

  if (len < 0) {
    env = NULL;
  }else{
    err = IoErr();
    if (err == 0) {
      buf[len] = 0;
      env = strdup(buf);
    }else{
      len = err;
      buf2 = malloc(len+1);
      GetVar(s, buf2, len+1, GVF_GLOBAL_ONLY);
      buf2[len] = 0;
      env = buf2;
    }
  }
  return env;
}
