#ifndef __ST__SEMAPHORE__H__
#define __ST__SEMAPHORE__H__

#include <pthread.h>
#include "STTypedef.h"
#include "STMacrodef.h"
#include "STCritical.h"

class STSemaphore
{
public:
	/**
	* \fn				            STSemaphore()
	* \brief				        构造函数
	*/
	STSemaphore();

	/**
	* \fn						    ~STSemaphore()
	* \brief						析构函数
	*/
	virtual ~STSemaphore();

	/**
	* \fn							STInt Create(STUint aInitialCount = 1);
	* \brief						创建信号量
	* \param[in]	aInitialCount	初始信号量值
	* \return						状态
	*/
	STInt							Create(STUint aInitialCount = 1);
	
	/**
	* \fn							STInt Wait();
	* \brief						信号量减一操作
	*/
	void							Wait();

	/**
	* \fn							STInt Wait(STUint32 aTimeOutUs);
	* \brief						延时一段时间();
	* \param[in]	aTimeOut_Msec	延时数，微秒
	*/
	void							Wait(STUint32 aTimeOutUs);
	
	/**
	* \fn							STInt Signal();
	* \brief						信号量加一操作
	*/
	void							Signal();
	
	/**
	* \fn							STInt Destroy();
	* \brief						关闭信号量
	* \return						状态
	*/
	STInt							Destroy();
    
    /**
     * \fn                          void LockContext
     * \brief                       用于临界Wait Signal操作,主要用于防止wait操作出现在signal后面，导致错误线程挂起
     */
	void                            LockContext();
	
	/**
     * \fn                          void UnlockContext
     * \brief                       用于临界Wait Signal操作,主要用于防止wait操作出现在signal后面，导致错误线程挂起
     */
	void                            UnlockContext();
    

private:
	void							GetAbsTime(struct timespec &aAbsTime, STUint32 aTimeoutUs);

private:
	STBool							iAlreadyExisted;
	STUint							iCount;
	pthread_cond_t					iCondition;
	pthread_mutex_t					iMutex;
    STUint                          iResourceCount;
};


#endif
