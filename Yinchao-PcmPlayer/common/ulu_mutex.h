#ifndef __ULU_MUTEX_H__
#define __ULU_MUTEX_H__

#ifdef __APPLE__

#ifndef _IOS
#define _IOS
#endif //_IOS

#else

#ifdef ANDROID
#ifndef _LINUX
#define _LINUX
#endif //_LINUX

#ifndef _LINUX_ANDROID
#define _LINUX_ANDROID
#endif //_LINUX_ANDROID
#endif //ANDROID

#endif //__APPLE__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#elif defined _LINUX
#include <pthread.h>
#elif defined(_IOS) || defined(_MAC_OS) 
#include <pthread.h>
#endif // WIN32

// wrapper for whatever critical section we have
class ulu_CMutex
{
public:
    ulu_CMutex(void);
    virtual ~ulu_CMutex(void);
    
    /**
     * Try to lock mutex without block, will return false if mutex already be locked
     *
     * return true if lock success, need to unlock
     * return false if lock fail, don't need unlock
     */
    bool TryLock();
    
    void Lock();
    void Unlock();

#ifdef WIN32
    CRITICAL_SECTION	m_CritSec;
#elif defined _LINUX
	pthread_mutex_t		m_hMutex;
#elif defined(_IOS) || defined(_MAC_OS)
	pthread_cond_t		m_hCondition;
	pthread_mutex_t		m_hMutex;
#endif // WIN32

public:
	unsigned int				m_nCurrentOwner;
	unsigned int				m_nLockCount;
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class ulu_CAutoLock
{
protected:
    ulu_CMutex * m_pLock;

public:
    ulu_CAutoLock(ulu_CMutex * plock)
    {
        m_pLock = plock;
        if (m_pLock) {
            m_pLock->Lock();
        }
    };

    ~ulu_CAutoLock()
	{
        if (m_pLock) {
            m_pLock->Unlock();
        }
    };
};
    
#ifdef _ULUNAMESPACE
}
#endif

#endif //__ULU_MUTEX_H__
