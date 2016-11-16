#include "ulu_mutex.h"

ulu_CMutex::ulu_CMutex()
{
#ifdef WIN32
    InitializeCriticalSection(&m_CritSec);
#elif defined _LINUX_ANDROID
	pthread_mutexattr_t attr = PTHREAD_MUTEX_RECURSIVE_NP;
	pthread_mutex_init (&m_hMutex, &attr);
#elif defined(_IOS) || defined(_MAC_OS)
	int                   rc=0;
	pthread_mutexattr_t   mta;
	rc = pthread_mutexattr_init(&mta);
	rc = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
	
	rc = pthread_mutex_init (&m_hMutex, &mta);
	rc = pthread_mutexattr_destroy(&mta);
	
	//pthread_mutex_init (&m_hMutex, NULL);	
#endif // WIN32

    m_nCurrentOwner = 0;
	m_nLockCount = 0;
}

ulu_CMutex::~ulu_CMutex()
{
#ifdef WIN32
    DeleteCriticalSection(&m_CritSec);
#elif defined _LINUX
	pthread_mutex_destroy (&m_hMutex);
#elif defined __SYMBIAN32__
	m_cMutex.Close();
#elif defined(_IOS) || defined(_MAC_OS)
	pthread_mutex_destroy (&m_hMutex);
#endif // WIN32

}

// return false if lock fail
bool ulu_CMutex::TryLock()
{
#ifdef WIN32
    if (TryEnterCriticalSection(&m_CritSec) == FALSE) {
        return false;
    }
#elif defined _LINUX// WIN32
	if (0 != pthread_mutex_trylock(&m_hMutex)) {
        return false;
    }
#elif defined(_IOS) || defined(_MAC_OS)
	if (0 != pthread_mutex_trylock(&m_hMutex)) {
        return false;
    }
#endif
    
    return true;
}

void ulu_CMutex::Lock()
{
	unsigned int nNewOwner = 0;
#ifdef WIN32
    nNewOwner = GetCurrentThreadId();
    EnterCriticalSection(&m_CritSec);
#elif defined _LINUX// WIN32
	nNewOwner = pthread_self();
	pthread_mutex_lock (&m_hMutex);
#elif defined(_IOS) || defined(_MAC_OS)
	//nNewOwner = (unsigned int)pthread_self();
	//VOLOGI("lock mutex = %d", m_hMutex);
	pthread_mutex_lock (&m_hMutex);	
#endif

    if (0 == m_nLockCount++)
        m_nCurrentOwner = nNewOwner;
}

void ulu_CMutex::Unlock()
{
    if (0 == --m_nLockCount)
        m_nCurrentOwner = 0;

#ifdef WIN32
    LeaveCriticalSection(&m_CritSec);
#elif defined _LINUX
	pthread_mutex_unlock (&m_hMutex);
#elif defined(_IOS) || defined(_MAC_OS)
	pthread_mutex_unlock (&m_hMutex);	
#endif // WIN32
}
