#include "STBaseAudioOutput.h"
#include "STOSConfig.h"
#include "STThread.h"
#include "STSampleBuffer.h"
#include "STBufferConfig.h"
#include "STLog.h"
STBaseAudioOutput::STBaseAudioOutput()
	: iDecodeThreadHandle(STThread::Self())
    , iCurSampleBuffer(NULL)
    , iSampleRate(0)
    , iChannels(0)
	, iCurPosition(0)
{
	iCritical.Create();
}

STBaseAudioOutput::~STBaseAudioOutput()
{
	iFillBufferArray.Close();
	iEmptiedBackgroundBufferArray.Close();
	iEmptiedOriginBufferArray.Close();
	iCritical.Destroy();
}

STInt STBaseAudioOutput::Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath)
{
	iChannels = aChannels;
	iSampleRate = aSampleRate;

	return STKErrNone;
}

void STBaseAudioOutput::RenderBuffer(STSampleBuffer* aBuffer)
{
    STASSERT(aBuffer->GetPosition() == 0);
    iCritical.Lock();
    iFillBufferArray.Append(aBuffer);
    iCritical.UnLock();
}

void STBaseAudioOutput::Stop()
{
	iCritical.Lock();
	iCurPosition = 0;
	iCritical.UnLock();
}

STSampleBuffer* STBaseAudioOutput::RecycleBuffer(STBool aForceRecycle)
{
	iCritical.Lock();
	STSampleBuffer* pSampleBuffer = NULL;
	if (iEmptiedOriginBufferArray.Count() > 0)
	{
		pSampleBuffer = iEmptiedOriginBufferArray[0];
		if (aForceRecycle || pSampleBuffer->GetByteOffset() + pSampleBuffer->Size() < GetMinCachedOffset())
		{
			iEmptiedOriginBufferArray.Remove(0);
		}
		else
		{
			pSampleBuffer = NULL;
		}
	}

	if (pSampleBuffer == NULL && iEmptiedBackgroundBufferArray.Count() > 0)
	{
		pSampleBuffer = iEmptiedBackgroundBufferArray[0];
		if (aForceRecycle || pSampleBuffer->GetByteOffset() + pSampleBuffer->Size() < GetMinCachedOffset())
		{
			iEmptiedBackgroundBufferArray.Remove(0);
		}
		else
		{
			pSampleBuffer = NULL;
		}
	}
	iCritical.UnLock();

	return pSampleBuffer;
}

void STBaseAudioOutput::Position(STUint& aPosition)
{
	iCritical.Lock();
	aPosition = iCurPosition;
	iCritical.UnLock();
}

void STBaseAudioOutput::SyncPosition(STUint aPosition)
{
	iCritical.Lock();
	iCurPosition = aPosition;
	iCritical.UnLock();
}

void STBaseAudioOutput::Flush()
{
	iCritical.Lock();
	STInt nFilledCount = iFillBufferArray.Count();
	for (STInt i = nFilledCount - 1; i >= 0; i--)
	{
		STSampleBuffer* pBuffer = iFillBufferArray[i];
		iEmptiedOriginBufferArray.Append(pBuffer);
	}
	iFillBufferArray.Reset();
	iCritical.UnLock();
}

STInt STBaseAudioOutput::Switch2Stream(STInt aStreamIdx)
{
	return STKErrNotSupported;
}

STInt STBaseAudioOutput::GetCurSteramIndex()
{
	return 0;
}

STInt STBaseAudioOutput::GetMinCachedOffset()
{
	return 0;
}

STSampleBuffer* STBaseAudioOutput::GetFilledBuffer()
{
	STSampleBuffer* pBuffer = NULL;
	iCritical.Lock();
	if (iFillBufferArray.Count() > 0)
	{
		pBuffer = iFillBufferArray[0];
		iFillBufferArray.Remove(0);
	}
	iCritical.UnLock();

	return pBuffer;
}



