#include "STMsgQueue.h"
#include "STThread.h"
STMsgQueue::STMsgQueue()
{
	iCmdThreadSuspend = ESTFalse;
	iWorkThread = NULL;
	iMsgHandle = NULL;
	iCmdSemaphore.Create();
}

STMsgQueue::~STMsgQueue()
{
	iCmdSemaphore.Destroy();
}

void STMsgQueue::Close()
{
	printf("STMsgQueue close\n");
	STASSERT(iMsgQueue.Count() == 0);
	Reset();

    iCmdSemaphore.LockContext();
	iMsgQueue.Close();
	iCmdSemaphore.UnlockContext();
}

void STMsgQueue::Reset()
{
	iCmdSemaphore.LockContext();
	iMsgQueue.ResetAndDestroy();
	iCmdSemaphore.UnlockContext();
}

void STMsgQueue::SendMsg(STMsg& aMsg)
{
	aMsg.SetSyncMsg(ESTTrue);

    iCmdSemaphore.LockContext();
	iMsgQueue.Insert(&aMsg, 0);
	NotifyMsgAvailable();
	iCmdSemaphore.Wait();
    iCmdSemaphore.UnlockContext();
}

void STMsgQueue::PostMsg(STMsg& aMsg)
{
	iCmdSemaphore.LockContext();
	iMsgQueue.Append(&aMsg);
	iCmdSemaphore.UnlockContext();

	NotifyMsgAvailable();
}

void STMsgQueue::NotifyMsgAvailable()
{
    iWorkThread->LockContext();
    
    iWorkThread->Resume();
    
    iWorkThread->UnlockContext();
}

void STMsgQueue::HandleMsg()
{
	STASSERT((iMsgHandle != NULL) && (iWorkThread != NULL));

	iCmdSemaphore.LockContext();
	if (iMsgQueue.Count() > 0)
	{
		STMsg* pMsg = iMsgQueue[0];
		STASSERT(pMsg != NULL);

		iMsgQueue.Remove(0);
		iCmdSemaphore.UnlockContext();

		iMsgHandle->HandleMsg(*pMsg);

		NotifyMsgAvailable();

		iCmdSemaphore.LockContext();//下面需要临界的原因在于，如果这个消息为结束子线程，而在Singal后实体Close后导致程序崩溃
		STBool bSyncMsg = pMsg->iSyncMsg;
		delete pMsg;
		if (bSyncMsg)
		{
			iCmdSemaphore.Signal();
		}	
	}
	iCmdSemaphore.UnlockContext();
}

void STMsgQueue::SetReciver(STThread* aWorkThread, ISTMsgHandle* aMsgHandle)
{
	iCmdSemaphore.LockContext();
	STASSERT((aMsgHandle != NULL) && (aWorkThread != NULL));
	iWorkThread = aWorkThread;
	iMsgHandle = aMsgHandle;

	if (iCmdThreadSuspend)
	{
		iCmdThreadSuspend = ESTFalse;
		iCmdSemaphore.Signal();
	}	
	iCmdSemaphore.UnlockContext();
}

void STMsgQueue::Init()
{
	iCmdSemaphore.LockContext();
	if ((iMsgHandle == NULL) || (iWorkThread == NULL))
	{
		iCmdThreadSuspend = ESTTrue;
		iCmdSemaphore.Wait();
	}

    iCmdSemaphore.UnlockContext();
}

STBool STMsgQueue::IsAllMsgHandled()
{
	iCmdSemaphore.LockContext();
	STBool bAllMsgHandled = (iMsgQueue.Count() == 0);
	iCmdSemaphore.UnlockContext();

	return bAllMsgHandled;
}

//end of file
