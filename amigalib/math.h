#ifdef __SASC
#include "include:math.h"
#ifdef _M68881
#include <m68881.h>
#endif
#ifdef _IEEE
#include <mieeedoub.h>
#endif
#ifdef _FFP
#include <mffp.h>
#endif
#else
#include "/gnu/include/math.h"
#endif
