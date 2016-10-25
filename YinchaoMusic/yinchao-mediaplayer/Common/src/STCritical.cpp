#include "STMacrodef.h"
#include "STCritical.h"
#include "STArray.h"
#include "STLog.h"
STCritical::STCritical()
: iAlreadyExisted(ESTFalse)
{
}

STCritical::~STCritical()
{
	Destroy();
}

STInt STCritical::Create()
{
	STInt nErr = STKErrNone;
	if (iAlreadyExisted)
	{
		nErr = STKErrAlreadyExists;
	}
	else if ((nErr = pthread_mutex_init(&iMutex, NULL)) == STKErrNone)
	{
        iAlreadyExisted = ESTTrue;		
	}

	return nErr;
}

STInt STCritical::Lock()
{
	STASSERT(iAlreadyExisted);
	return pthread_mutex_lock(&iMutex);
}

STInt STCritical::TryLock()
{
	STASSERT(iAlreadyExisted);
	return pthread_mutex_trylock(&iMutex);
}

STInt STCritical::UnLock()
{	
	STASSERT(iAlreadyExisted);
	return pthread_mutex_unlock(&iMutex);
}

STInt STCritical::Destroy()
{
	STInt nErr = STKErrNone;
	if (iAlreadyExisted)
	{
		nErr = pthread_mutex_destroy(&iMutex);
		if (STKErrNone == nErr)
		{
			iAlreadyExisted = ESTFalse;
		}
		else
		{
			STASSERT(ESTFalse);
		}
	}

	return nErr;
}

//end of file
