// INCLUDES
#include "TTOsalConfig.h"
#include <string.h>
#include <stdlib.h>
#include "TThread.h"
#include "TTLog.h"
#ifdef __TT_OS_ANDROID__
#include <sched.h>
#endif

static const TTInt KMaxPriority = 100;
static const TTInt KMinPriority = -100;

RTThread::RTThread()
: iThreadName(NULL)
, iThreadExisted(ETTFalse)
, iThreadTerminating(ETTFalse)
, iThreadParam(NULL)
{
}

RTThread::~RTThread()
{    
	TTASSERT(!iThreadExisted);
}

TTInt RTThread::Close()
{
	TTInt nErr = Terminate();
	SAFE_FREE(iThreadName);	
	SAFE_DELETE(iThreadParam);
	return nErr;
}

TTInt RTThread::Create(const TTChar* aName, TThreadFunc aFunc, void* aPtr, TTInt aPriority)
{
	TTASSERT((aPriority >= KMinPriority) && (aPriority <= KMaxPriority));

	LOGI("Thread [%s] creating.", aName);

	TTInt nErr = TTKErrArgument;

	if (iThreadExisted)
	{
		nErr = TTKErrAlreadyExists;
	}
	else if (aFunc != NULL)
	{	
		
		SAFE_FREE(iThreadName);
		TTInt nNameLen = (TTInt)strlen(aName) + 1;
		iThreadName = (TTChar*)(malloc(nNameLen));
        strcpy(iThreadName, aName);

		iThreadParam = new TThreadParm(aFunc, aPtr, this);

		iThreadExisted = ((nErr = pthread_create(&iThreadId, NULL, ThreadProc, (void*)(iThreadParam))) == TTKErrNone);

		LOGI("Thread [%s] created.", iThreadName);
	}

	return nErr;
}

void* RTThread::ThreadProc(void* aPtr)
{
	TTSetDbgFlag();

	void* pRet = NULL;
	TThreadParm* pParam = (TThreadParm*)(aPtr);
	TTASSERT(pParam != NULL);
	pRet = (*(pParam->iFunc))(pParam->iPtr);

	return pRet;
}

TTBool RTThread::Terminating()
{
	return iThreadTerminating;
}

TTInt RTThread::Terminate()
{
	TTInt nErr = TTKErrNotFound;
	if(iThreadExisted)
	{
	  LOGI("Thread begin to %s terminated.", iThreadName);
	  iThreadTerminating = ETTTrue;
	  if ((nErr = pthread_join(iThreadId, NULL)) == TTKErrNone)
	  {
		  iThreadExisted = ETTFalse;	
		  iThreadTerminating = ETTFalse;
	  }
	}
	LOGI("Thread %s terminated.", iThreadName);
	return nErr;
}

pthread_t RTThread::Id()
{
	return iThreadId;
}

//end of file
