#ifndef __ST_MSG_QUEUE_H__
#define __ST_MSG_QUEUE_H__

#include <pthread.h>
#include "STTypedef.h"
#include "STMacrodef.h"
#include "STCritical.h"
#include "STSemaphore.h"
#include "STArray.h"
class STThread;
class STMsg
{

public:

	STMsg(STInt aMsgId, void* aMessage, void* aWhat, void* aParam = NULL)
		: iMsgId(aMsgId), iMessage(aMessage), iWhat(aWhat), iParam(aParam), iSyncMsg(ESTFalse)
	{ }

	void			SetSyncMsg(STBool aSyncMsg) {iSyncMsg = aSyncMsg;};


public:

	STInt			iMsgId;
	void*			iMessage;
	void*			iWhat;
    void*           iParam;

	STBool			iSyncMsg;
};

class ISTMsgHandle
{
public:

	/**
	* \fn								 void HandleMsg(STMsg& aMsg);
	* \brief							 线程通信消息处理函数
	* \param[in] aMsg					 消息引用
	*/
	virtual void HandleMsg(STMsg& aMsg) = 0;
};

class STMsgQueue
{
public:

	/**
	* \fn							STMsgQueue.
	* \brief						C++ default constructor.
	*/
	STMsgQueue();

	~STMsgQueue();


	/**
	* \fn							Close().
	* \brief						
	*/
	void 							Close();

	/**
	* \fn							Reset()
	* \brief						清空消息队列
	*/
	void 							Reset();
	
	/**
	* \fn							void SendMsg(STMsg* aMsg);
	* \brief						发送消息，并等待消息处理完成
	* \param[in]	aMsg			消息对象引用
	* \return						成功发送消息TTKErrNone，其他失败
	*/
	void							SendMsg(STMsg& aMsg);

	/**
	* \fn							void PostMsg(STMsg* aMsg);
	* \brief						发送消息，不等待消息完成，立刻返回
	* \param[in]	aMsg			消息对象引用
	* \return						成功发送消息TTKErrNone，其他失败
	*/
	void							PostMsg(STMsg& aMsg);

	/**
	* \fn							void HandleMsg()
	* \brief						处理消息，在子线程中
	*/
	void							HandleMsg();
	
	/**
	* \fn							STBool IsAllMsgHandled()
	* \brief						是否所有消息处理完成
	* \return						完成为ESTTrue
	*/
	STBool							IsAllMsgHandled();

	/**
	* \fn							void SetReciver(STThread* aReceiver, ISTMsgHandle* aMsgHandle);
	* \brief						设置接收线程指针
	* \param[in]	aWorkThread		接收线程指针
	* \param[in]	aMsgHandle		Msg处理接口指针
	*/
	void							SetReciver(STThread* aWorkThread, ISTMsgHandle* aMsgHandle);

	/**
	* \fn							void Init();
	* \brief						初始化线程通信设置
	*/
	void							Init();

private:
	void							NotifyMsgAvailable();

private:
	STPointerArray<STMsg>			iMsgQueue;
	STSemaphore						iCmdSemaphore;
//	STCritical						iCritical;
	ISTMsgHandle*					iMsgHandle;
	STThread*						iWorkThread;
	STBool							iCmdThreadSuspend;
};


#endif
