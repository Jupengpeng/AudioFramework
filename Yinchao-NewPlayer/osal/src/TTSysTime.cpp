// INCLUDES
#include "TTOsalConfig.h"
#include "TTSysTime.h"

#if (defined __TT_OS_WINDOWS__)//WINDOWS
#include <time.h>
#include <Windows.h>

TTUint64 GetTimeOfDay()//获取当前时间，从1970.1.1.0.0.0开始计算，单位为毫秒
{
	TTUint64 nTime = (TTUint64)(::timeGetTime());
	return nTime;
}

#elif (defined __TT_OS_SYMBIAN__)//SYMBIAN
#include <time.h>

TTUint64 GetTimeOfDay()//获取当前时间，从1970.1.1.0.0.0开始计算，单位为毫秒
{
	TTime	tNow;
	tNow.HomeTime();
	TDateTime tTime = tNow.DateTime ();
	TTUint64 nTime = tTime.Hour () * 3600000 + tTime.Minute () * 60000 + tTime.Second () * 1000 + tTime.MicroSecond () / 1000;

	return nTime;
}

#elif (defined __TT_OS_ANDROID__ || defined (__TT_OS_IOS__))//ANDROID
#include <sys/time.h>
#include <time.h>

TTUint64 GetTimeOfDay()//获取当前时间，从1970.1.1.0.0.0开始计算，单位为毫秒
{
	struct timeval tTime;
	gettimeofday(&tTime, NULL);
	TTUint64 temp = TTUint64(tTime.tv_sec);	
	return TTUint64(temp * 1000 + tTime.tv_usec/1000);
}
#endif

//end of file
