#include "STOSConfig.h"
#include <string.h>
#include <stdlib.h>
#include "STThread.h"
#include "STLog.h"
#include "STSysTime.h"
#include "STCritical.h"
#ifdef __ST_OS_IOS__
#include "STAutoReleasePool.h"
#endif

static const STInt KMaxPriority = 100;
static const STInt KMinPriority = -100;

class SThreadParm
{
public:
	SThreadParm(STThreadFunc aFunc, void* aPtr, STThread* aThreadPtr)
		: iFunc(aFunc), iPtr(aPtr), iThreadPtr(aThreadPtr){};

	STThreadFunc iFunc;
	void*		 iPtr;
	STThread*    iThreadPtr;
};

STPointerArray<STThread::STThreadIdHandlePair> STThread::iThreadIdHandlePairArray(5);
STCritical* STThread::iGlobalCritical;


void STThread::InitContext()
{
	STLOGI("XThread::InitContext()");
	STThread::iGlobalCritical = new STCritical();
	STThread::iGlobalCritical->Create();
}

void STThread::UninitContext()
{
	STLOGI("XThread::UninitContext");
	STThread::iGlobalCritical->Destroy();
	SAFE_DELETE(STThread::iGlobalCritical);
}
#ifdef __ST_OS_ANDROID__
__attribute__ ((constructor)) void DllInit()
{
	STLOGI("dll constructor");
	STThread::InitContext();
}
__attribute__ ((destructor)) void DllUninit()
{
	STThread::UninitContext();
	STLOGI("dll destructor");
}
#endif

STThread::STThread()
: iThreadExisted(ESTFalse)
, iIsSuspend(ESTFalse)
{
	pthread_cond_init(&iCondition, NULL);
	pthread_mutex_init(&iMutex, NULL);
}

STThread::~STThread()
{    
    pthread_mutex_destroy(&iMutex);
    pthread_cond_destroy(&iCondition);
	STASSERT(!iThreadExisted);
}

STInt STThread::Close()
{
	return Terminate();
}

STBool STThread::IsExisted()
{
	return iThreadExisted;
}

STInt STThread::Create(STThreadFunc aFunc, void* aPtr, STInt aPriority)
{
	STASSERT((aPriority >= KMinPriority) && (aPriority <= KMaxPriority) && (!iThreadExisted));
	STInt nErr = STKErrArgument;

	if (aFunc != NULL)
	{
		SThreadParm *pThreadParam = new SThreadParm(aFunc, aPtr, this);
        pthread_t tThreadId;
		if ((iThreadExisted = ((nErr = pthread_create(&tThreadId, NULL, ThreadProc, (void*)(pThreadParam))) == STKErrNone)))
        {
            STThreadIdHandlePair* pPair = new STThreadIdHandlePair(tThreadId, this);
			if (AddValidPair(pPair) != STKErrNone)
			{
				SAFE_DELETE(pPair);
			}
        }
	}
	return nErr;
}

pthread_t STThread::Id()
{
    return pthread_self();
}

STBool  STThread::ThreadIdEqual(pthread_t t1, pthread_t t2) 
{
#ifdef __ST_OS_WINDOWS__
	return t1.p == t2.p && t1.x == t2.x;
#else
	return t1 == t2;
#endif
}

pthread_t STThread::GetId(STThread* aThreadPtr)
{
#ifdef __ST_OS_WINDOWS__
	pthread_t tThreadId;
	tThreadId.p = NULL;
	tThreadId.x = 0;
#else
	pthread_t tThreadId = 0;
#endif

	STThread::iGlobalCritical->Lock();
	for (STInt i = iThreadIdHandlePairArray.Count() - 1; i >= 0; i--)
	{
		if (iThreadIdHandlePairArray[i]->iThreadPtr == aThreadPtr)
		{
			tThreadId = iThreadIdHandlePairArray[i]->iThreadId;
			break;
		}        
	}
	STThread::iGlobalCritical->UnLock();

	return tThreadId;
}

STThread* STThread::GetThreadPtr(pthread_t aId)
{
    STThread* pThreadPtr = NULL;
    STThread::iGlobalCritical->Lock();
    for (STInt i = iThreadIdHandlePairArray.Count() - 1; i >= 0; i--)
    {
        if (ThreadIdEqual(iThreadIdHandlePairArray[i]->iThreadId, aId))
        {
            pThreadPtr = iThreadIdHandlePairArray[i]->iThreadPtr;
            break;
        }        
    }
    STThread::iGlobalCritical->UnLock();
    return pThreadPtr;
}

void* STThread::ThreadProc(void* aPtr)
{
#ifdef __ST_OS_IOS__
   void* pool = STAutoReleasePool::InitAutoRelesePool();
#endif
	STSetDbgFlag();

	void* pRet = NULL;
	SThreadParm* pParam = (SThreadParm*)(aPtr);
	STASSERT(pParam != NULL);

	STThreadIdHandlePair* pPair = new STThreadIdHandlePair(Id(), pParam->iThreadPtr);
	if (AddValidPair(pPair) != STKErrNone)
	{
		SAFE_DELETE(pPair);
	}

	pRet = (*(pParam->iFunc))(pParam->iPtr);

	delete pParam;

#ifdef __ST_OS_IOS__
    STAutoReleasePool::UninitAutoReleasePool(pool);
#endif
    
	return pRet;
}

STInt STThread::Terminate()
{
	STInt nErr = STKErrNotFound;

    pthread_t tThreadId = GetId(this);
    
	if (iThreadExisted && (nErr = pthread_join(tThreadId, NULL)) == STKErrNone)
    {
		iThreadExisted = ESTFalse;	
        RemoveInvalidPair(tThreadId);
	}

	return nErr;
}

void STThread::RemoveInvalidPair(pthread_t aId)
{
	STThread::iGlobalCritical->Lock();
    
    for (STInt i = iThreadIdHandlePairArray.Count() - 1; i >= 0; i--)
    {
        if (ThreadIdEqual(iThreadIdHandlePairArray[i]->iThreadId, aId))
		{
			STThreadIdHandlePair* pPair = iThreadIdHandlePairArray[i];
			iThreadIdHandlePairArray.Remove(i);
			STThread::iGlobalCritical->UnLock();

			SAFE_DELETE(pPair);
            return;
        }        
    }    
    
    STASSERT(ESTFalse);
}

STInt STThread::AddValidPair(STThreadIdHandlePair* aPair) 
{
	STASSERT(aPair != NULL);

	if (NULL == GetThreadPtr(aPair->iThreadId))
	{
		STThread::iGlobalCritical->Lock();
		iThreadIdHandlePairArray.Append(aPair);
		STThread::iGlobalCritical->UnLock();
		return STKErrNone;
	}

	return STKErrAlreadyExists;
}

void STThread::Resume()
{
	STASSERT(iThreadExisted);
    if (iIsSuspend) 
    {
        iIsSuspend = ESTFalse;
        pthread_cond_signal(&iCondition);
    }
}

void STThread::Suspend()
{
    if (!iIsSuspend) 
    {
        iIsSuspend = ESTTrue;
        pthread_cond_wait(&iCondition, &iMutex);
    }
}

void STThread::Wait(STUint aDelay)
{
	STInt nErr = STKErrNotFound;
	if ((nErr = pthread_mutex_lock(&iMutex)) == STKErrNone)
	{
		while (nErr == STKErrNone)
		{
			struct timespec tAbsTime;//
			GetAbsTime(tAbsTime, (STUint32)aDelay);
			nErr = pthread_cond_timedwait(&iCondition, &iMutex, &tAbsTime);
		}
		pthread_mutex_unlock(&iMutex);
	}
}

STThread* STThread::Self()
{
   return GetThreadPtr(Id());
}

void STThread::LockContext()
{
	pthread_mutex_lock(&iMutex);
}

void STThread::UnlockContext()
{
	pthread_mutex_unlock(&iMutex);
}

void STThread::GetAbsTime(struct timespec &aAbsTime, STUint32 aTimeoutUs)
{
	STUint64 nToTime = GetTimeOfDay() + aTimeoutUs;
	aAbsTime.tv_sec = long(nToTime / 1000000);
	aAbsTime.tv_nsec = long((nToTime - aAbsTime.tv_sec * 1000000) * 1000);
}

//end of file
