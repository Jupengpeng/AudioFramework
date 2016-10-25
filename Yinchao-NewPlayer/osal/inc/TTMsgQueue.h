/**
* File : TTMsgQueue.h 
* Created on : 2011-3-19
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : RTTMsgQueue 定义文件
*/

#ifndef __TT_MSG_QUEUE_H__
#define __TT_MSG_QUEUE_H__

#include <pthread.h>
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTCritical.h"
#include "TTArray.h"
#include "TThread.h"
#include "TTSemaphore.h"
#include "TTCritical.h"
#include "TTActive.h"

class TTMsg//注意这个需要在堆上分配，在RTTMsgQueue只保持其指针, 由RTTMsgQueue 释放
{

public:

	TTMsg(TTInt aMsgId, void* aMsgData0, void* aMsgData1)
		: iMsgId(aMsgId), iMsgData0(aMsgData0), iMsgData1(aMsgData1), iSyncMsg(ETTFalse)
	{ }

	void			SetSyncMsg(TTBool aSyncMsg) {iSyncMsg = aSyncMsg;};


public:

	TTInt			iMsgId;
	void*			iMsgData0;
	void*			iMsgData1;

	TTBool			iSyncMsg;
};

class ITTMsgHandle//接收线程实现此接口
{
public:

	/**
	* \fn								 void HandleMsg(TTMsg& aMsg);
	* \brief							 线程通信消息处理函数
	* \param[in] aMsg					 消息引用
	*/
	virtual void HandleMsg(TTMsg& aMsg) = 0;
};

class RTTMsgQueue : public CTTActive
{
public:

	/**
	* \fn							RTTMsgQueue.
	* \brief						C++ default constructor.
	*/
	RTTMsgQueue();

	~RTTMsgQueue();


	/**
	* \fn							Close().
	* \brief						R类需要Close或Release函数，不需要析构函数
	*/
	void 							Close();

	/**
	* \fn							Reset()
	* \brief						清空消息队列
	*/
	void 							Reset();
	
	/**
	* \fn							void SendMsg(TTTMsg* aMsg);
	* \brief						发送消息，并等待消息处理完成
	* \param[in]	aMsg			消息对象引用
	* \return						成功发送消息TTKErrNone，其他失败
	*/
	TTInt							SendMsg(TTMsg& aMsg);

	/**
	* \fn							void PostMsg(TTTMsg* aMsg);
	* \brief						发送消息，不等待消息完成，立刻返回
	* \param[in]	aMsg			消息对象引用
	* \return						成功发送消息TTKErrNone，其他失败
	*/
	TTInt							PostMsg(TTMsg& aMsg);

	/**
	* \fn							void SetReceiver(RTThread* aReceiver, ITTMsgHandle* aMsgHandle);
	* \brief						设置接收线程指针
	* \param[in]	aReceiver		接收线程指针
	* \param[in]	aMsgHandle		Msg处理接口指针
	*/
	void							SetReceiver(RTThread* aReceiver, ITTMsgHandle* aMsgHandle);
	
	/**
	* \fn							void Init();
	* \brief						初始化
	*/
	void							Init();

	/**
	* \fn							void Clear();
	* \brief						清空消息队列
	*/
	void							Clear();


protected:
	/**
	* \fn							void DoCancel();
	* \brief						取消AO时操作，派生类实现
	*/
	virtual void					DoCancel();

	/**
	* \fn							void RunL();
	* \brief						AO完成时的操作，派生类实现
	*/
	virtual void					RunL();


private:
	void							NotifyMsgAvailable();
	void							CheckReady();


private:
	RTTPointerArray<TTMsg>			iMsgQueue;
	RTThread*						iReceiveThread;
	RTTSemaphore					iSemaphore;
	RTTCritical						iCritical;
	ITTMsgHandle*					iMsgHandle;
	TTBool							iCommanderSuspend;
};


#endif
