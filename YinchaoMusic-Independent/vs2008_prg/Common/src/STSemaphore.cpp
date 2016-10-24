// INCLUDES
#include "STSemaphore.h"
#include "STSysTime.h"

STSemaphore::STSemaphore()
: iAlreadyExisted(ESTFalse)
{
    
}

STSemaphore::~STSemaphore()
{
	STASSERT(!iAlreadyExisted);
}

STInt STSemaphore::Create(STUint aMaxResCount)
{
    iResourceCount = aMaxResCount;
	STInt nErr = STKErrAlreadyExists;
	if (!iAlreadyExisted)
	{
		iCount = aMaxResCount;
        
		if (((nErr = pthread_cond_init(&iCondition, NULL)) == STKErrNone)
			&& ((nErr = pthread_mutex_init(&iMutex, NULL)) == STKErrNone))
		{
			iAlreadyExisted = ESTTrue;
		}
	}
    
	return nErr;
}

void STSemaphore::LockContext()
{
    pthread_mutex_lock(&iMutex);
}

void STSemaphore::UnlockContext()
{
    pthread_mutex_unlock(&iMutex);
}

void STSemaphore::Wait()
{
    pthread_cond_wait(&iCondition, &iMutex);
}

void STSemaphore::Wait(STUint32 aTimeOutUs)
{
    STInt nErr = STKErrNotFound;
	if ((nErr = pthread_mutex_lock(&iMutex)) == STKErrNone)
	{
		while (nErr == STKErrNone)
		{
			struct timespec tAbsTime;//
			GetAbsTime(tAbsTime, (STUint32)aTimeOutUs);
			nErr = pthread_cond_timedwait(&iCondition, &iMutex, &tAbsTime);
		}
		pthread_mutex_unlock(&iMutex);
	}
}

void STSemaphore::GetAbsTime(struct timespec &aAbsTime, STUint32 aTimeoutUs)
{
	STUint64 nToTime = GetTimeOfDay() + aTimeoutUs;
	aAbsTime.tv_sec = long(nToTime / 1000000);
	aAbsTime.tv_nsec = long((nToTime - aAbsTime.tv_sec * 1000000) * 1000);
}

void STSemaphore::Signal()
{   
	STASSERT(iAlreadyExisted);
    pthread_cond_signal(&iCondition);
}

STInt STSemaphore::Destroy()
{
	if (!iAlreadyExisted)
		return STKErrNotFound;
    
	iAlreadyExisted = ESTFalse;
    
	pthread_mutex_destroy(&iMutex);
	pthread_cond_destroy(&iCondition);
    
	return STKErrNone;
}

//end of file
