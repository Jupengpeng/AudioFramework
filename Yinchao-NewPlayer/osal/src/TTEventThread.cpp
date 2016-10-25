#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "TTStdint.h"
#include "TTSysTime.h"
#include "TTEventThread.h"

#ifdef __TT_OS_ANDROID__
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>
#endif

#define LOG_TAG "TTEventThread"
#include "TTLog.h"

TTEventThread::TTEventThread(const char * pThreadName)
	: mThreadTerminating(false)
	, mNextEventID(1)
	, mStatus (TT_THREAD_STATUS_INIT)
{
	if (pThreadName != NULL)
		strcpy (mName, pThreadName);
	else
		strcpy (mName, "");

	mConditionHeadChanged.Create();
	mConditionNotEmpty.Create();
	mEventCritical.Create();
	mStatusCritical.Create();
}

TTEventThread::~TTEventThread(void)
{
	stop ();

	freeAllEvent ();

	mStatusCritical.Destroy();
	mEventCritical.Destroy();
	mConditionHeadChanged.Destroy();
	mConditionNotEmpty.Destroy();
}

int TTEventThread::start()
{
	TTCAutoLock lock (&mStatusCritical);
	TTInt	nErr = TTKErrNotFound;
	
	if(mStatus == TT_THREAD_STATUS_RUN) {
		return TTKErrNone;
	}

	mStatus = TT_THREAD_STATUS_INIT;

	pthread_attr_t  attr;
 	pthread_attr_init(&attr); 
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	nErr	= pthread_create(&mThreadId, &attr, (void*(*)(void*))eventBaseThreadProc, this);
	
	pthread_attr_destroy(&attr);

	if(nErr == TTKErrNone) mStatus = TT_THREAD_STATUS_RUN;

	return nErr;
}

int TTEventThread::stop (void)
{
	TTCAutoLock lock (&mStatusCritical);

	TTInt nErr = TTKErrNotFound;
	if(mStatus != TT_THREAD_STATUS_RUN) {
		return TTKErrNone;
	}

	mStatus = TT_THREAD_STATUS_STOP;

	postEventWithDelayTime(new TTEndEvent(), TIME_MIN);

	mThreadTerminating = true;
	if ((nErr = pthread_join(mThreadId, NULL)) == TTKErrNone) {
		mThreadTerminating = false;	
	}

	cancelAllEvent ();
	LOGI ("The thread %s exit", mName);

	return nErr;
}

TT_THREAD_STATUS TTEventThread::getStatus (void)
{
	return mStatus;
}

int TTEventThread::postEventWithRealTime (TTBaseEventItem * pEvent, TTInt64 nRealTime)
{
	TTCAutoLock lock (&mEventCritical);
	
	if (pEvent == NULL)
		return -1;

	pEvent->setTime (nRealTime);
	pEvent->setEventID (mNextEventID++);

	List<TTBaseEventItem *>::iterator it = mListFull.begin();
    while (it != mListFull.end() && nRealTime >= (*it)->getTime()) {
        ++it;
    }

	if (it == mListFull.begin()) {
        mConditionHeadChanged.Signal();
    }

	mListFull.insert(it, pEvent);

	mConditionNotEmpty.Signal();

	return 0;
}

int TTEventThread::postEventWithDelayTime (TTBaseEventItem * pEvent, TTInt64 nDelayTime)
{
	TTInt64 nTime = -1;
	if (nDelayTime > 0) {
		nTime = GetTimeOfDay () + nDelayTime;
	} else {
		nTime = nDelayTime;
	}

	return postEventWithRealTime (pEvent, nTime);
}

TTBaseEventItem *	TTEventThread::cancelEvent (TTBaseEventItem * pEvent)
{
	TTCAutoLock lock (&mEventCritical);
	
	if (pEvent == NULL)
		return NULL;

	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
        if((*it) == pEvent)	{
			(*it)->setEventID(0);
			mListFree.push_back(*it);
			it = mListFull.erase(it);
			return *it;
		}
		++it;
    }

	return NULL;
}

TTBaseEventItem * TTEventThread::cancelEventByID (int nID, bool stopAfterFirstMatch)
{
	TTCAutoLock lock (&mEventCritical);

	TTBaseEventItem * pEvent = NULL;
	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
        if((*it)->getEventID() == nID) 	{
			pEvent = *it;
			pEvent->setEventID(0);
			mListFree.push_back(pEvent);
			it = mListFull.erase(it);
			
			if(stopAfterFirstMatch)
				break;
		}
		++it;
    }

	return pEvent;
}

TTBaseEventItem * TTEventThread::cancelEventByType (TTInt nType, bool stopAfterFirstMatch)
{
	TTCAutoLock lock (&mEventCritical);

	TTBaseEventItem * pEvent = NULL;
	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
        if((*it)->getEventType() == nType) 	{
			pEvent = *it;
			(*it)->setEventID(0);
			mListFree.push_back(*it);
			it = mListFull.erase(it);

			if(stopAfterFirstMatch)
				break;
		}
		++it;
    }

	return pEvent;
}

TTBaseEventItem * TTEventThread::cancelEventByMsg (TTInt nMsg, bool stopAfterFirstMatch)
{
	TTCAutoLock lock (&mEventCritical);

	TTBaseEventItem * pEvent = NULL;
	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
        if((*it)->getEventMsg() == nMsg) {
			pEvent = *it;
			(*it)->setEventID(0);
			mListFree.push_back(*it);
			it = mListFull.erase(it);
			
			if(stopAfterFirstMatch)
				break;
		}
		++it;
    }

	return pEvent;
}

int TTEventThread::cancelAllEvent (void)
{
	TTCAutoLock lock (&mEventCritical);

	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
		(*it)->setEventID(0);
		mListFree.push_back(*it);
		it = mListFull.erase(it);
    }

	return 0;
}

int TTEventThread::freeAllEvent (void)
{
	TTCAutoLock lock (&mEventCritical);

	List<TTBaseEventItem *>::iterator it = mListFull.begin();
	while (it != mListFull.end()) {
		delete (*it);
		it = mListFull.erase(it);
    }

	it = mListFree.begin();
	while (it != mListFree.end()) {
		delete (*it);
		it = mListFree.erase(it);
    }

	return 0;
}

int TTEventThread::getFullEventNum (TTInt nType)
{
	TTCAutoLock lock (&mEventCritical);

	int nNum = mListFull.size();

	return nNum;
}

TTBaseEventItem * TTEventThread::getEventByType (TTInt nType)
{
	TTCAutoLock lock (&mEventCritical);

	TTBaseEventItem * pEvent = NULL;
	List<TTBaseEventItem *>::iterator it = mListFree.begin();
	while (it != mListFree.end()) {
		if((*it)->getEventType() == nType)	{
			pEvent = *it;
			it = mListFree.erase(it);
			break;
		}		
		++it;
	}
	
	return pEvent;
}

int TTEventThread::eventBaseThreadProc (void* pParam)
{
	TTEventThread * pThread = (TTEventThread *)pParam;

	return pThread->eventBaseThreadLoop();
}
	
int TTEventThread::eventBaseThreadLoop (void)
{
	TTThreadSetName (mName);

    for (;;) {
        TTInt64 nNowTime = 0;
        TTBaseEventItem *	pTask = NULL;

        {
            TTCAutoLock lock (&mEventCritical);

			if (mStatus == TT_THREAD_STATUS_STOP) {
				break;
			}

            while (mListFull.empty()) {
                mConditionNotEmpty.Wait(mEventCritical);
			}

			TTInt nEventID = 0;
            for (;;) {
                if (mListFull.empty()) {
                    break;
                }

				List<TTBaseEventItem *>::iterator it = mListFull.begin();
				nEventID = (*it)->getEventID();

                nNowTime = GetTimeOfDay ();
				TTInt64 nTime = (*it)->getTime();

                TTInt64 DelayTime;
                if (nTime <= 0 || nTime == TIME_MAX) {
                    DelayTime = 0;
                } else {
                    DelayTime = nTime - nNowTime;
                }

                if (DelayTime <= 0) {
                    break;
                }

                static TTInt64 kMaxTimeoutUs = 10000000ll;  // 10 secs
                bool timeoutCapped = false;
                if (DelayTime > kMaxTimeoutUs) {
                    DelayTime = kMaxTimeoutUs;
                    timeoutCapped = true;
                }
				
                int err = mConditionHeadChanged.Wait(mEventCritical, DelayTime);
                if (!timeoutCapped && err == -ETIMEDOUT) {
                    break;
                }
            }

			pTask = cancelEventByID(nEventID);
        }

		if(pTask != NULL) {
			pTask->fire ();
		}
    }

	return 0;
}

void TTEventThread::TTThreadSetName(const char* threadName)
{
#ifdef __TT_OS_ANDROID__
	prctl(PR_SET_NAME, (unsigned long)threadName , 0 , 0 , 0);
#endif // _WIN32
}
