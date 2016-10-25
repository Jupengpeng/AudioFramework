#ifndef __ST_UTILITIES_H__
#define __ST_UTILITIES_H__
#include "STOSConfig.h"
#include "STLog.h"
#include "STSysTime.h"

#define DIFF_TIME(fun) \
{\
	STUint64 nStart = GetTimeOfDay();\
	fun;\
	STUint64 nEnd = GetTimeOfDay();\
	STLOGW("Consumed Time = %llu.%06llu(s)", (nEnd-nStart)/1000000, (nEnd-nStart)%1000000);\
}
#endif
