// INCLUDES
#include "TTMacrodef.h"
#include "TTCritical.h"

RTTCritical::RTTCritical()
: iAlreadyExisted(ETTFalse)
{

}

RTTCritical::~RTTCritical()
{
	Destroy();
}

TTInt RTTCritical::Create()
{
	TTInt nErr = TTKErrNone;

	pthread_mutexattr_t mAttr; 
    pthread_mutexattr_init(&mAttr); 
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE); 
  	
	if (iAlreadyExisted)
	{
		nErr = TTKErrAlreadyExists;
	}
	else if ((nErr = pthread_mutex_init(&iMutex, &mAttr)) == TTKErrNone)
	{
        iAlreadyExisted = ETTTrue;		
	}

	pthread_mutexattr_destroy(&mAttr);

	return nErr;
}

TTInt RTTCritical::Lock()
{
	TTASSERT(iAlreadyExisted);
	return pthread_mutex_lock(&iMutex);
}

TTInt RTTCritical::TryLock()
{
	TTASSERT(iAlreadyExisted);
	return pthread_mutex_trylock(&iMutex);
}

TTInt RTTCritical::UnLock()
{	
	TTASSERT(iAlreadyExisted);
	return pthread_mutex_unlock(&iMutex);
}

TTInt RTTCritical::Destroy()
{
	TTInt nErr = TTKErrNone;
	if (iAlreadyExisted)
	{
		nErr = pthread_mutex_destroy(&iMutex);
		if (nErr == TTKErrNone)
		{
			iAlreadyExisted = ETTFalse;
		}
		else
		{
			TTASSERT(ETTFalse);
		}
	}

	return nErr;
}

pthread_mutex_t* RTTCritical::GetMutex()
{
	if (iAlreadyExisted)
	{
		return &iMutex;
	}

	return NULL;
}

//end of file
