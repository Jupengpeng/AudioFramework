#ifndef __TT__SEMAPHORE__H__
#define __TT__SEMAPHORE__H__

#include <pthread.h>
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTCritical.h"

class RTTSemaphore
{
public:
	/**
	* \fn				            RTTSemaphore()
	* \brief				        构造函数
	*/
	RTTSemaphore();

	/**
	* \fn						    ~RTTSemaphore()
	* \brief						析构函数
	*/
	virtual ~RTTSemaphore();

	/**
	* \fn							TTInt Create(TTUint aInitialCount = 0);
	* \brief						创建信号量
	* \param[in]	aInitialCount	初始信号量值
	* \return						状态
	*/
	TTInt							Create(TTUint aInitialCount = 0);
	
	/**
	* \fn							TTInt Wait();
	* \brief						信号量减一操作
	* \return						状态
	*/
	TTInt							Wait();

	/**
	* \fn							TTInt Wait(TTUint32 aTimeOutUs);
	* \brief						延时一段时间();
	* \param[in]	aTimeOut_Msec	延时数，毫秒
	* \return						状态
	*/
	TTInt							Wait(TTUint32 aTimeOutMs);

	/**
	* \fn							TTInt Signal();
	* \brief						信号量加一操作
	* \return						状态
	*/
	TTInt							Signal();


	/**
	* \fn							TTInt Reset();
	* \brief						信号量重置
	* \return						状态
	*/
	TTInt							Reset();

	/**
	* \fn							TTInt Destroy();
	* \brief						关闭信号量
	* \return						状态
	*/
	TTInt							Destroy();

private:
	void							GetAbsTime(struct timespec &aAbsTime, TTUint32 aTimeoutUs);

private:
	TTBool							iAlreadyExisted;
	TTUint							iCount;
	pthread_cond_t					iCondition;
	pthread_mutex_t					iMutex;
};


#endif
