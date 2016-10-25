#ifndef __TT_THREAD_H__
#define __TT_THREAD_H__

// INCLUDES
#include <pthread.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"

class RTThread;

class TThreadParm
{
public:
	TThreadParm(TThreadFunc aFunc, void* aPtr, RTThread* aThreadPtr)
		: iFunc(aFunc), iPtr(aPtr), iThreadPtr(aThreadPtr){};

	TThreadFunc iFunc;
	void*		iPtr;
	RTThread*	iThreadPtr;
};

// CLASSES DECLEARATION
class RTThread
{
public:
	
	/**
	* \fn                       RTThread()
	* \brief                    构造函数
	*/
	RTThread();
	
	/**
	* \fn                       ~RTThread()
	* \brief                    析构函数
	*/
	~RTThread();

	/**
	* \fn                       TTInt Create(const TTChar* aName, TThreadFunc aFunc, void* aPtr, TTInt aPriority = 0);
	* \brief                    创建线程
	* \param[in]	aName		线程名称
	* \param[in]    aFunc		线程函数
	* \param[in]    aPtr		带入参数
	* \param[in]    aPriority	优先级,范围为-100~+100， 0表示Normal	
	* \return					操作状态
	*/
	TTInt						Create(const TTChar* aName, TThreadFunc aFunc, void* aPtr, TTInt aPriority = 0);
	
	/**
	* \fn                       TTInt Terminate();
	* \brief                    结束线程
	* \return					操作状态
	*/
	TTInt						Terminate();
	
	/**
	* \fn                       TTInt Close();
	* \brief                    结束线程
	* \return					操作状态
	*/
	TTInt						Close();

	/**
	* \fn                       pthread_t Id();
	* \brief                    获取线程Id
	* \return					线程Id
	*/
	pthread_t					Id();

	/**
	* \fn                       TTBool* Terminating();
	* \brief                    线程将要终止
	* \return					true表示线程将要终止
	*/
	TTBool						Terminating();

private:
	static void*				ThreadProc(void* aPtr);


private:
	pthread_t					iThreadId;				/**<线程句柄*/
	TTChar*						iThreadName;			/**<线程名*/
	TTBool						iThreadExisted;			/**<线程是否存在*/
	TTBool						iThreadTerminating;	    /**<线程将要终止*/
	TThreadParm*				iThreadParam;
};

#endif
