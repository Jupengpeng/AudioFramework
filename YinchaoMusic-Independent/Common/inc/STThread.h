#ifndef __ST_THREAD_H__
#define __ST_THREAD_H__

// INCLUDES
#include <pthread.h>
#include "STMacrodef.h"
#include "STTypedef.h"
#include "STArray.h"
#include "STOSConfig.h"
#include "STCritical.h"
// CLASSES DECLEARATION
class STThread
{
public:
	
	/**
	* \fn                       STThread()
	* \brief                    构造函数
	*/
	STThread();
	
	/**
	* \fn                       ~STThread()
	* \brief                    析构函数
	*/
	~STThread();

	/**
	* \fn                       STInt Create(STThreadFunc aFunc, void* aPtr, STInt aPriority = 0);
	* \brief                    创建线程
	* \param[in]	aName		线程名称
	* \param[in]    aFunc		线程函数
	* \param[in]    aPtr		带入参数
	* \param[in]    aPriority	优先级,范围为-100~+100， 0表示Normal	
	* \return					操作状态
	*/
	STInt						Create(STThreadFunc aFunc, void* aPtr, STInt aPriority = 0);
	
	/**
	* \fn                       STInt Terminate();
	* \brief                    结束线程
	* \return					操作状态
	*/
	STInt						Terminate();
	
	/**
	* \fn                       STInt Close();
	* \brief                    结束线程
	* \return					操作状态
	*/
	STInt						Close();

	/**
	* \fn                       void Suspend();
	* \brief                    挂起线程,在执行操作前需要LockContext，执行操作后需要UnlockContext
	* \return					操作状态
	*/
	 void                       Suspend();
    
	/**
	* \fn                       void Resume();
	* \brief                    激活线程，在别的线程中操作，在执行操作前需要LockContext，执行操作后需要UnlockContext
	* \return					操作状态
	*/
	void						Resume();

	/**
	* \fn                       STBool IsExisted;
	* \brief					判断线程是否存在
	* \return					EXTrue表示存在
	*/
	STBool						IsExisted();
	
	/**
	* \fn                       XThread* Self;
	* \brief					获取当前线程句柄
	* \return					前线程句柄
	*/
	static STThread*			Self();

	void						Wait(STUint aDelay);
	
	/**
	* \fn                       void LockContext
	* \brief					用于临界Suspend Resume操作,主要用于防止wait操作出现在signal后面，导致错误线程挂起
	*/
	void						LockContext();
	
	/**
	* \fn                       void UnlockContext
	* \brief					用于临界Suspend Resume操作,主要用于防止wait操作出现在signal后面，导致错误线程挂起
	*/
	void						UnlockContext();
    
    static void                 InitContext();
    static void                 UninitContext();
    
private:
	class STThreadIdHandlePair
	{
	public:
		pthread_t				   iThreadId;
		STThread*                  iThreadPtr;

	public:
		STThreadIdHandlePair(pthread_t aThreadId, STThread* aThreadPtr) : iThreadId(aThreadId), iThreadPtr(aThreadPtr) {};
	};

private:
	static void*				ThreadProc(void* aPtr);		
    static pthread_t            Id();
    static STThread*            GetThreadPtr(pthread_t aId);
	static pthread_t			GetId(STThread* aThreadPtr);
    static void                 RemoveInvalidPair(pthread_t aId);
	static STInt				AddValidPair(STThreadIdHandlePair* aPair);
	static void 				GetAbsTime(struct timespec &aAbsTime, STUint32 aTimeoutUs);
    static STBool				ThreadIdEqual(pthread_t t1, pthread_t t2);
    
private:
	STBool						iThreadExisted;
    static STPointerArray<STThreadIdHandlePair>    iThreadIdHandlePairArray;
    pthread_cond_t				iCondition;
    pthread_mutex_t				iMutex;
    STBool                      iIsSuspend;
    static STCritical* 			iGlobalCritical;
};
#endif
