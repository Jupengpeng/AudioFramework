// INCLUDES
#include "TTCondition.h"
#include "TTSysTime.h"
#ifdef __TT_OS_WINDOWS__
#include <sys/timeb.h>
#endif

RTTCondition::RTTCondition()
: iAlreadyExisted(ETTFalse)
{

}

RTTCondition::~RTTCondition()
{
	TTASSERT(!iAlreadyExisted);
}

TTInt RTTCondition::Create()
{
	TTInt nErr = TTKErrAlreadyExists;
	if (!iAlreadyExisted)
	{
		if ((nErr = pthread_cond_init(&iCondition, NULL)) == TTKErrNone)
		{
			iAlreadyExisted = ETTTrue;
		}
	}

	return nErr;
}

TTInt RTTCondition::Wait(RTTCritical& Mutex)
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{ 
		nErr = pthread_cond_wait(&iCondition, Mutex.GetMutex());
	}

	return nErr;
}

TTInt RTTCondition::Wait(RTTCritical& Mutex, TTUint32 aTimeOutMs)
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		struct timespec tAbsTime;//自1970年1月1日岛现在
		GetAbsTime(tAbsTime, aTimeOutMs);

		nErr = pthread_cond_timedwait(&iCondition, Mutex.GetMutex(), &tAbsTime);

	}
	return nErr;
}

void RTTCondition::GetAbsTime(struct timespec &aAbsTime, TTUint32 aTimeoutMs)
{
	TTUint64 nToTime = 0;
#ifdef __TT_OS_WINDOWS__
	struct _timeb currSysTime;
	_ftime(&currSysTime);

	nToTime = (TTUint64) currSysTime.time * 1000000;
	nToTime += (TTUint64) currSysTime.millitm*1000 + aTimeoutMs*1000;
#else
	nToTime = GetTimeOfDay()*1000 + aTimeoutMs*1000;//返回微秒
#endif

	aAbsTime.tv_sec = long(nToTime / 1000000);//秒
	aAbsTime.tv_nsec = long((nToTime - aAbsTime.tv_sec * 1000000) * 1000);//纳秒
}

TTInt RTTCondition::Signal()
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		nErr = pthread_cond_signal(&iCondition);
	}
	
	return nErr;
}

TTInt RTTCondition::Destroy()
{
	if (!iAlreadyExisted)
		return TTKErrNotFound;

	iAlreadyExisted = ETTFalse;

	pthread_cond_destroy(&iCondition);

	return TTKErrNone;
}

//end of file
