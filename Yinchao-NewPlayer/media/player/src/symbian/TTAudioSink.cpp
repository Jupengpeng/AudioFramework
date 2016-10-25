/**
* File : TTAudioSink.cpp 
* Created on : 2011-3-1
* Author : hu.cao
* Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioSink 实现文件
*/

// INCLUDES
#include "Symbian/TTAudioSink.h"
#include "TTMediaBuffer.h"
#include <stdio.h>
#include "symbian/TTAudioStreamRender.h"

CTTAudioSink::CTTAudioSink(ITTSinkDataProvider* aDataProvider)
: CTTBaseDataSink(aDataProvider)
, iSrcBuffer(NULL)
{
	iSendMsgQueue.CreateLocal(10);
	iRenderThread.Create(_L("RenderThread"), RenderThreadProc, 16*KILO, NULL, this);
	iRenderThread.SetPriority(EPriorityRealTime);
	iRenderThread.Resume();

	iStreamRender->iSendMsgQueue.SetReciver(TTUser::Thread(), this);
}

CTTAudioSink::~CTTAudioSink()
{
	iSendMsgQueue.Close();
}

TInt CTTAudioSink::RenderThreadProc(TAny* aPtr)
{
	CTrapCleanup* pTrapCleanup = CTrapCleanup::New();
	if (pTrapCleanup == NULL)
	{
		return KErrNoMemory;
	}

	CTTAudioSink* pSink = reinterpret_cast<CTTAudioSink*>(aPtr);

	TRAPD(nErr, pSink->RenderThreadProcL());

	delete pTrapCleanup;

	return nErr;
}

void CTTAudioSink::RenderThreadProcL()
{
	CActiveScheduler* pScheduler = new(ELeave) CActiveScheduler;
	CActiveScheduler::Install(pScheduler);

	iStreamRender = new CTTAudioStreamRender(&iSendMsgQueue);
    
	CActiveScheduler::Start();
    
	delete iStreamRender;
	delete pScheduler;
}

void CTTAudioSink::SetAudioProperties(TTInt aSampleRate, TInt aChannel)
{

}

TTInt CTTAudioSink::Render(CTTMediaBuffer* aBuffer)
{
	FillBuffer(aBuffer);
	TTStreamSinkMsg tMsg;
	tMsg.iId = TTStreamSinkMsg::EStreanAudioRender;
	iSendMsgQueue.Send(tMsg);
	return KErrNone;
}

TTInt CTTAudioSink::Open(TTAudioDataSettings& /*aAudioDataSetting*/)
{
	TTStreamSinkMsg tMsg;
	tMsg.iId = TTStreamSinkMsg::EStreamSinkMsgOpen;
	iSendMsgQueue.Send(tMsg);
	return KErrNone;
}


void CTTAudioSink::Close()
{

}

void CTTAudioSink::Start()
{
//	iBufferRequesting = ETrue;
}

void CTTAudioSink::Pause()
{

}

void CTTAudioSink::Resume()
{

}

void CTTAudioSink::Stop()
{

}

TTInt CTTAudioSink::QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr)
{
	switch(aInterfaceID)
	{
	case KSyncClockInterfaceId:
		{   
			*aInterfacePtr = static_cast<ITTSyncClock*>(this);   
			AddRef();
			return KErrNone;
		}
		break;

	default:
		return KErrNotSupported;
	}
}

// void CTTAudioSink::RunL()
// {
// 	TTStreamSinkMsg tMsg;
// 	iReceiveMsgQueue->Receive(tMsg);
// 
// 	switch (tMsg.iId)
// 	{
// 	case TTStreamSinkMsg::EStreanAudioFillBuffer:
// 		FillBuffer(NULL);
// 		break;
// 
// 	default:
// 		break;
// 	}
// 
// 	StartMsgGet();
// }

// void CTTAudioSink::WriteData(CTTMediaBuffer* aBuffer)
// {
// 	TTASSERT(aBuffer != NULL);
// 	aBuffer->UnRef();
// }

TTInt CTTAudioSink::Initialize()
{
	return TTKErrNone;
	//return Open();
}

void CTTAudioSink::UnInitialize()
{
	Close();
}

void CTTAudioSink::FillBuffer(CTTMediaBuffer* aBuffer)
{
	if (aBuffer != NULL)//有Render得来
	{
		ASSERT(iSrcBuffer == NULL);
		//TInt nCpySize = iStreamRender->FilledBuffer(aBuffer->Ptr(), aBuffer->Size());
		ASSERT(iStreamRender->FilledBuffer(aBuffer->Ptr(), aBuffer->Size()) == aBuffer->Size());
		aBuffer->UnRef();
	}
    
	CTTMediaBuffer* pBuffer = iSrcBuffer;
	
	while (ETrue)
	{
		if (pBuffer == NULL)
			pBuffer = iDataProvider->GetFilledBuffer();
		
		if (pBuffer != NULL)
		{			
			TInt nMoveSize = iStreamRender->FilledBuffer(pBuffer->Ptr(), pBuffer->Size());
			if(nMoveSize == 0)
			{
				iSrcBuffer = pBuffer;
				break;
			}
			else
			{				
				ASSERT(nMoveSize == pBuffer->Size());
				pBuffer->UnRef();
			}
		}
		else
		{
			iSrcBuffer = NULL;
			break;
		}

		pBuffer = NULL;
	}
}

void CTTAudioSink::HandleMsg(TTMsg& aMsg)
{
	switch (aMsg.iMsgId)
	{
	case TTStreamSinkMsg::EStreanAudioFillBuffer:
		{
			FillBuffer(NULL);
		//	if (!iDataProvider.IsDataEnd())
			{
				CTTAudioStreamRender::TTRenderStatus tStatus = iStreamRender->RenderStatus();
				if ((CTTAudioStreamRender::ERenderStatusWaiting == tStatus )|| (CTTAudioStreamRender::ERenderStatusOpened == tStatus))
				{
					TTStreamSinkMsg tMsg;
					tMsg.iId = TTStreamSinkMsg::EStreanAudioRender;
					iSendMsgQueue.Send(tMsg);
				}
			}
		}
		break;

	default:
		break;
	}
}

void CTTAudioSink::GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels)
{

}
