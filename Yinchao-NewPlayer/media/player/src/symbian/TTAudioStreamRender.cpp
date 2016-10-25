/**
* File : TTAudioStreamRender.cpp  
* Created on : 2011-3-31
* Author : hu.cao
* Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioStreamRender  实现文件
*/

// INCLUDES
#include "symbian/TTAudioStreamRender.h"
#include <string.h>

static const TInt KDataBufferSize = 48 * 1024;

CTTAudioStreamRender::CTTAudioStreamRender(RMsgQueue<TTStreamSinkMsg>* aReceiveMsgQueue)
: CActive(EPriorityStandard)
, iRenderStatus(ERenderStatusNotReady)
, iAudioOutputStream(NULL)
, iReceiveMsgQueue(aReceiveMsgQueue)
{
	iRenderPtr = new TPtr8(NULL, KDataBufferSize);
	iCritical.CreateLocal();
	CActiveScheduler::Add(this);
	iAudioOutputStream = CMdaAudioOutputStream::NewL(*this);
	StartMsgGet();

	iPreBufferInfo.iBufferPtr = new TTUint8[KDataBufferSize];
	ASSERT(iPreBufferInfo.iBufferPtr != NULL);

	iCurBufferInfo.iBufferPtr = new TTUint8[KDataBufferSize];
	ASSERT(iCurBufferInfo.iBufferPtr != NULL);

	iNextBufferInfo.iBufferPtr = new TTUint8[KDataBufferSize]; 
	ASSERT(iNextBufferInfo.iBufferPtr != NULL);

	iNextNextBufferInfo.iBufferPtr = new TTUint8[KDataBufferSize];
	ASSERT(iNextNextBufferInfo.iBufferPtr != NULL);
}

CTTAudioStreamRender::~CTTAudioStreamRender()
{
	delete[] iNextBufferInfo.iBufferPtr;
	delete[] iPreBufferInfo.iBufferPtr;
	delete[] iCurBufferInfo.iBufferPtr;
	delete[] iNextNextBufferInfo.iBufferPtr;

	delete iRenderPtr;

	Cancel();
	ASSERT(iAudioOutputStream != NULL);
	delete iAudioOutputStream;
	iCritical.Close();
}


void CTTAudioStreamRender::MaoscOpenComplete(TInt aError)
{
	TRAPD(nErr, iAudioOutputStream->SetAudioPropertiesL(TMdaAudioDataSettings::ESampleRate44100Hz, TMdaAudioDataSettings::EChannelsStereo));

	iAudioOutputStream->SetVolume(iAudioOutputStream->MaxVolume() / 2);
	//iCritical.Wait();
	iRenderStatus = ERenderStatusOpened;
	//iCritical.Signal();
}

void CTTAudioStreamRender::MaoscBufferCopied(TInt aError, const TDesC8& /*aBuffer*/)
{
	if (aError == KErrNone)
	{
		RenderBuffer();
	}
	else
	{
		ASSERT(EFalse);
	}
}

void CTTAudioStreamRender::MaoscPlayComplete(TInt aError)
{

}

void CTTAudioStreamRender::RunL()
{
	TTStreamSinkMsg tMsg;
	iReceiveMsgQueue->Receive(tMsg);

	switch (tMsg.iId)
	{
	case TTStreamSinkMsg::EStreamSinkMsgOpen:
		{
			TMdaAudioDataSettings tAudioSetting;
			iAudioOutputStream->Open(&tAudioSetting);
		}
		break;

	case TTStreamSinkMsg::EStreanAudioProperties:
		break;

	case TTStreamSinkMsg::EStreanAudioRender:
		{
			if ((iRenderStatus == ERenderStatusOpened )|| (iRenderStatus == ERenderStatusWaiting))
			{
				RenderBuffer();
			}
		}
		break;

	case TTStreamSinkMsg::EStreanAudioDestory:
		{
			Cancel();
			CActiveScheduler::Stop();
		}
		break;

	default:
		break;
	}

	StartMsgGet();
}

CTTAudioStreamRender::TTRenderStatus CTTAudioStreamRender::RenderStatus()
{
	return iRenderStatus;
}

void CTTAudioStreamRender::RenderBuffer()
{
	iCritical.Wait();
	if (iNextBufferInfo.iLength > 0 || iNextNextBufferInfo.iLength > 0)
	{
		TTBufferInfo tTempBufferInfo = iCurBufferInfo;
		iCurBufferInfo = iNextBufferInfo;
		iNextBufferInfo = iNextNextBufferInfo;
		iNextNextBufferInfo = iPreBufferInfo;
		iNextNextBufferInfo.iLength = 0;
		iPreBufferInfo = tTempBufferInfo;
		iCritical.Signal();

		if (iCurBufferInfo.iLength > 0)
		{
			iRenderPtr->Set(iCurBufferInfo.iBufferPtr, iCurBufferInfo.iLength, KDataBufferSize);
			iAudioOutputStream->WriteL(*iRenderPtr);
			iRenderStatus = ERenderStatusPlaying;
		}
	}
	else
	{
		iRenderStatus = ERenderStatusWaiting;
		iCritical.Signal();
	}

	RequestFillingBuffer();
}

void CTTAudioStreamRender::DoCancel()
{
	if (IsActive())
	{
		iReceiveMsgQueue->CancelDataAvailable();
	}
}

void CTTAudioStreamRender::StartMsgGet()
{
	ASSERT(iReceiveMsgQueue != NULL);
    
	iReceiveMsgQueue->NotifyDataAvailable(iStatus);
	SetActive();
}

TTInt CTTAudioStreamRender::FilledBuffer(TTUint8* aDataPtr, TTInt aDataSize)
{
	TTBufferInfo& tBufferInfo = (TTInt)(iNextBufferInfo.iLength) < KDataBufferSize ? iNextBufferInfo : iNextNextBufferInfo;
	
	TUint nDataLen = tBufferInfo.iLength;
	
	iCritical.Wait();

	TInt nDataMove = aDataSize;

	if (nDataMove > KDataBufferSize - (TTInt)nDataLen)//大于
	{
		nDataMove = KDataBufferSize - nDataLen;
	}

	if (nDataMove > 0)
	{
        memcpy(tBufferInfo.iBufferPtr + nDataLen, aDataPtr, nDataMove);
		tBufferInfo.iLength += nDataMove;
	}

	iCritical.Signal();

	return nDataMove;
}

void CTTAudioStreamRender::RequestFillingBuffer()
{
	TTMsg* pMsg = new TTMsg(TTStreamSinkMsg::EStreanAudioFillBuffer, NULL, NULL);
	iSendMsgQueue.PostMsg(*pMsg);	
}
