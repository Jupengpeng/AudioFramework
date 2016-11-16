#ifndef __ULU_COMMON__
#define __ULU_COMMON__

#include <stdlib.h>
#include <memory.h>

#ifndef ULUMAX
#define ULUMAX(a, b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef ULUMIN
#define ULUMIN(a, b)    ((a) < (b) ? (a) : (b))
#endif
#endif