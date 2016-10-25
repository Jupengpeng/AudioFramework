// INCLUDES
#include "TTSemaphore.h"
#include "TTSysTime.h"
#ifdef __TT_OS_WINDOWS__
#include <sys/timeb.h>
#endif

RTTSemaphore::RTTSemaphore()
: iAlreadyExisted(ETTFalse)
{

}

RTTSemaphore::~RTTSemaphore()
{
	TTASSERT(!iAlreadyExisted);
}

TTInt RTTSemaphore::Create(TTUint aInitialCount)
{
	TTInt nErr = TTKErrAlreadyExists;
	if (!iAlreadyExisted)
	{
		iCount = aInitialCount;

		if (((nErr = pthread_cond_init(&iCondition, NULL)) == TTKErrNone)
			&& ((nErr = pthread_mutex_init(&iMutex, NULL)) == TTKErrNone))
		{
			iAlreadyExisted = ETTTrue;
		}
	}

	return nErr;
}

TTInt RTTSemaphore::Wait()
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		if ((nErr = pthread_mutex_lock(&iMutex)) == TTKErrNone)
		{
			while ((iCount == 0) && (nErr == TTKErrNone))
			{   
				nErr = pthread_cond_wait(&iCondition, &iMutex);
			}

			if (nErr == TTKErrNone)
				iCount--;

			pthread_mutex_unlock(&iMutex);
		}	
	}

	return nErr;
}

TTInt RTTSemaphore::Wait(TTUint32 aTimeOutMs)
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		if ((nErr = pthread_mutex_lock(&iMutex)) == TTKErrNone)
		{
			if (iCount > 0)//资源可用
			{
				iCount--;
				pthread_mutex_unlock(&iMutex);
			}
			else
			{
				struct timespec tAbsTime;//自1970年1月1日岛现在
				GetAbsTime(tAbsTime, aTimeOutMs);

				while ((iCount == 0) && (nErr == TTKErrNone))
				{
					nErr = pthread_cond_timedwait(&iCondition, &iMutex, &tAbsTime);
				}

				if (nErr == TTKErrNone)//signalied
				{
					iCount--;
				}

				pthread_mutex_unlock(&iMutex);

				if(nErr == -1)//ETIMEDOUT
					nErr = TTKErrNone;
				else if (nErr != TTKErrNone)
				{
					nErr = TTKErrGeneral;
				}
			}
		}
	}
	return nErr;
}

void RTTSemaphore::GetAbsTime(struct timespec &aAbsTime, TTUint32 aTimeOutMs)//微秒
{
	TTUint64 nToTime = 0;
#ifdef __TT_OS_WINDOWS__
	struct _timeb currSysTime;
	_ftime(&currSysTime);

	nToTime = (TTUint64) currSysTime.time * 1000000;
	nToTime += (TTUint64) currSysTime.millitm*1000 + aTimeOutMs*1000;
#else
	nToTime = GetTimeOfDay()*1000 + aTimeOutMs*1000;//返回微秒
#endif

	aAbsTime.tv_sec = long(nToTime / 1000000);//秒
	aAbsTime.tv_nsec = long((nToTime - aAbsTime.tv_sec * 1000000) * 1000);//纳秒
}

TTInt RTTSemaphore::Signal()
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		if ((nErr = pthread_mutex_lock(&iMutex)) == TTKErrNone)
		{
			iCount++;
			
			nErr = pthread_cond_signal(&iCondition);
			pthread_mutex_unlock(&iMutex);
		}
	}
	
	return nErr;
}

TTInt RTTSemaphore::Reset()
{
	TTInt nErr = TTKErrNotFound;

	if (iAlreadyExisted)
	{
		if ((nErr = pthread_mutex_lock(&iMutex)) == TTKErrNone)
		{
			iCount = 0;

			pthread_mutex_unlock(&iMutex);
		}
	}

	return nErr;
}


TTInt RTTSemaphore::Destroy()
{
	if (!iAlreadyExisted)
		return TTKErrNotFound;

	iAlreadyExisted = ETTFalse;

	pthread_mutex_destroy(&iMutex);
	pthread_cond_destroy(&iCondition);

	return TTKErrNone;
}

//end of file
