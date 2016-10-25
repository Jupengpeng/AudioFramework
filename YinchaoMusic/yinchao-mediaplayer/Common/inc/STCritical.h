#ifndef __ST_CRITICAL_H__
#define __ST_CRITICAL_H__

// INCLUDES
#include <pthread.h>
#include "STTypedef.h"

// CLASSES DECLEARATION
class STCritical
{
public:

	/**
	* \fn                       STCritical()
	* \brief                    构造函数
	*/
	STCritical();

	/**
	* \fn                       ~STCritical()
	* \brief                    析构函数
	*/
	virtual ~STCritical();

public:

	/**
	* \fn                       STInt Create()
	* \brief                    创造互斥量
	* \return					操作状态
	*/	
	STInt						Create();

	/**
	* \fn                       void Lock()
	* \brief                    进入临界区函数
	* \return					操作状态
	*/	
	STInt						Lock();

	/**
	* \fn                       void Lock()
	* \brief                    尝试进入临界区
	* \return					操作状态，0表示成功，需要Unlock
	*/
	STInt						TryLock();

	/**
	* \fn                       void UnLock()
	* \brief                    释放临界区
	* \return					操作状态
	*/	
	STInt						UnLock();

	/**
	* \fn                       STInt Destroy()
	* \brief                    删除互斥量
	*/	
	STInt					    Destroy();
	
private:

	pthread_mutex_t				iMutex;				/**< 互斥量*/
	STBool						iAlreadyExisted;	/**< 互斥量是否已经存在*/
};
#endif
