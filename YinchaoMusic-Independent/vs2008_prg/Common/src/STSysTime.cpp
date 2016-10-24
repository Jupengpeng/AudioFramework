// INCLUDES
#include "STOSConfig.h"
#include "STSysTime.h"

#if (defined __ST_OS_WINDOWS__)//WINDOWS
#include <time.h>
#include <Windows.h>

STUint64 GetTimeOfDay()//获取当前时间，从1970.1.1.0.0.0开始计算，单位为微秒
{
	STUint64 nTime = STUint64(time(NULL));
	return nTime * 1000000;
}
#elif(defined (__ST_OS_IOS__) || defined (__ST_OS_ANDROID__))
#include <sys/time.h>
#include <time.h>

STUint64 GetTimeOfDay()//获取当前时间，从1970.1.1.0.0.0开始计算，单位为微秒
{
	struct timeval tTime;
	gettimeofday(&tTime, NULL);
	STUint64 temp = STUint64(tTime.tv_sec);	
	return STUint64(temp * 1000000 + tTime.tv_usec);
}
#endif
