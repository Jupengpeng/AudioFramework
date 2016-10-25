/**
* File : TTAudioStreamRender.h 
* Created on : 2011-3-31
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioStreamRender 定义文件
*/
#ifndef __TT_AUDIO_STREAM_RENDER_H__
#define __TT_AUDIO_STREAM_RENDER_H__

// INCLUDES
#include <e32std.h>
#include <Mda\Common\Audio.h>
#include <MdaAudioOutputStream.h>
#include <e32msgqueue.h>
#include "Symbian/TTAudioSink.h"
#include "TTMsgQueue.h"

class ITTSinkDataProvider;

class CTTAudioStreamRender : public CActive, MMdaAudioOutputStreamCallback
{
public:
	enum TTRenderStatus
	{
		ERenderStatusNotReady = -1
		, ERenderStatusOpened = 0
		, ERenderStatusWaiting = 1
		, ERenderStatusPlaying = 2
	};

public:
	CTTAudioStreamRender(RMsgQueue<TTStreamSinkMsg>* aReceiveMsgQueue);
	~CTTAudioStreamRender();

	TTInt FilledBuffer(TTUint8* aDataPtr, TTInt aDataSize);
	RMsgQueue<TTStreamSinkMsg>* MsgQueue();

	TTRenderStatus RenderStatus();

private:
	void MaoscOpenComplete(TInt aError);

	void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);

	void MaoscPlayComplete(TInt aError);


private:
	void	RunL();
	void    DoCancel();

private:
	void							RenderBuffer();
	void StartMsgGet();
	void RequestFillingBuffer();
private:

	TTRenderStatus						iRenderStatus;
	CMdaAudioOutputStream*          iAudioOutputStream;
	RMsgQueue<TTStreamSinkMsg>*		iReceiveMsgQueue;
	RCriticalSection				iCritical;

	TPtr8*							iRenderPtr;



public:
	RTTMsgQueue						iSendMsgQueue;

	class TTBufferInfo 
	{
	public:
		TTBufferInfo(TUint8* aPtr = NULL, TUint aLength = 0)
		{
			iBufferPtr = aPtr;
			iLength = aLength;
		}
		TTBufferInfo(TTBufferInfo& aBufferInfo)
		{
			iBufferPtr = aBufferInfo.iBufferPtr;
			iLength = aBufferInfo.iLength;
		}

		TTBufferInfo& operator= (TTBufferInfo& aBufferInfo)
		{
			iBufferPtr = aBufferInfo.iBufferPtr;
			iLength = aBufferInfo.iLength;
			return (*this);
		}
	public:
		TUint8*			iBufferPtr;
		TUint			iLength;
	};

	TTBufferInfo		iPreBufferInfo;
	TTBufferInfo		iCurBufferInfo;
	TTBufferInfo		iNextBufferInfo;
	TTBufferInfo		iNextNextBufferInfo;
};

#endif
