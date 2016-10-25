#include "STAudioOutput.h"
#include "STOSConfig.h"
#include "STSampleBuffer.h"
#include "STThread.h"
#include "STLog.h"
#include <stdio.h>
#include "Resample_samplerate.h"
#include "STBufferConfig.h"
#include <unistd.h>

STAudioOutput::STAudioOutput()
: iOpenSLESEngine(NULL)
, iAudioRecorder(NULL)
, iPlayerObjectItf(NULL)
, iPlayerItf(NULL)
, iPlayerBufferQueueItf(NULL)
, iPlayerVolumeItf(NULL)
, iFlushed(ESTTrue)
, iAccompanimentVolume(0xFFF)
, iRecordVolume(0x9FF)
, iCurStreamIdx(0)
#ifdef  __SUPPORT_RECORD__
, iTotalRenderSize(0)
, iProcessedByteOffset(0)
, iAacEncoder(NULL)
, iRecrodLoopBuffer(NULL)
, iRecordLoopBufferStartPtr(NULL)
, iRecordLoopBufferValidSize(0)
, iMixBuffer(NULL)
, iDenoise(NULL)

#endif
{
#ifdef  __SUPPORT_RECORD__
	iRecordCritical.Create();

	//混响创建
	m_pSTReverb = new STReverb();
// 	if(m_pSTReverb != NULL)
// 	{
// 		m_pSTReverb->SetCommReverbParam(presetI[3]);
// 	}

#endif
	iOpenSLESEngine = new STOpenSLESEngine();
}

STAudioOutput::~STAudioOutput()
{
#ifdef  __SUPPORT_RECORD__
	iRecordCritical.Destroy();
#endif
	SAFE_DELETE(iOpenSLESEngine);
}

void STAudioOutput::Pause()
{
    SLresult result = (*iPlayerItf)->SetPlayState(iPlayerItf, SL_PLAYSTATE_PAUSED);
	STASSERT(SL_RESULT_SUCCESS == result);
//	if (iAudioRecorder != NULL)//当录制跟上播放时暂停；
//	{
////		iAudioRecorder->Pause();
//	}
}

void STAudioOutput::Resume()
{
#ifdef  __SUPPORT_RECORD__
    if (iAudioRecorder != NULL)
    {
    	iAudioRecorder->Resume();
    }
#endif

    SLresult result = (*iPlayerItf)->SetPlayState(iPlayerItf, SL_PLAYSTATE_PLAYING);
	STASSERT(SL_RESULT_SUCCESS == result);
    if (iFlushed)
    {
    	FillBuffer();
    }
}

STInt STAudioOutput::Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath)
{
	STBaseAudioOutput::Open(aSampleRate, aChannels, aRecordSavePath);
	
	STInt nErr = STKErrNone;
	if (aRecordSavePath != NULL && strlen(aRecordSavePath) != 0)
	{
#ifdef  __SUPPORT_RECORD__
		iDenoise = new Denoise;
		iDenoise->init(8000,1600);//暂时写死

		iAudioRecorder = new STAudioRecorder(*iOpenSLESEngine, *this,this);
		nErr = iAudioRecorder->Open(aSampleRate, aChannels);
		if (nErr == STKErrNone)
		{
			iRecordCritical.Lock();
			iRecrodLoopBuffer = new STUint8[KRecordLoopBufferSize];
			memset(iRecrodLoopBuffer, 0, KRecordLoopBufferSize);
			iRecordLoopBufferStartPtr = iRecrodLoopBuffer;
			iRecordLoopBufferValidSize = 0;
			iProcessedByteOffset = 0;
			iRecordCritical.UnLock();

			iMixBuffer = new STUint8[KRecordLoopBufferSize];
			memset(iMixBuffer, 0, KRecordLoopBufferSize);
			iAacEncoder = new STAacEncoder();
			nErr = iAacEncoder->Init(aSampleRate, aChannels, aRecordSavePath);
		}
#else
		return STKErrNotSupported;
#endif
	}
	
	iTotalRenderSize = 0;

	if (nErr == STKErrNone)
	{
		return CreateBufferQueueAudioPlayer(aSampleRate, aChannels);
	}

    return nErr;
}

void STAudioOutput::Close()
{	
	iTotalRenderSize = 0;
	iCurStreamIdx = 0;
    if (iPlayerObjectItf != NULL)
    {
#ifdef  __SUPPORT_RECORD__
    	if (iAudioRecorder != NULL)
    	{
    		iRecordCritical.Lock();
    	}
#endif

        (*iPlayerObjectItf)->Destroy(iPlayerObjectItf);
        iPlayerObjectItf = NULL;
        iPlayerItf = NULL;
        iPlayerBufferQueueItf = NULL;
        iPlayerVolumeItf = NULL;

#ifdef  __SUPPORT_RECORD__
        if (iAudioRecorder != NULL)
        {
        	iRecordCritical.UnLock();
        }
#endif
    }
    
#ifdef  __SUPPORT_RECORD__
    if (iAudioRecorder != NULL) 
    {
    	iAudioRecorder->Close();
    	SAFE_DELETE(iAudioRecorder);
    	iRecordCritical.Lock();
    	SAFE_DELETE_ARRAY(iRecrodLoopBuffer);
    	iRecordCritical.UnLock();
    	SAFE_DELETE_ARRAY(iMixBuffer);
    	iProcessedByteOffset = 0;
    	iAacEncoder->UnInit();
    	SAFE_DELETE(iAacEncoder);
    }
	if (iDenoise != NULL)
	{
		SAFE_DELETE(iDenoise);
    }
#endif
}

void STAudioOutput::Start()
{
    FillBuffer();

#ifdef  __SUPPORT_RECORD__
    if (iAudioRecorder != NULL)
	{
		iAudioRecorder->Start();
	}
#endif

    SLresult result = (*iPlayerItf)->SetPlayState(iPlayerItf, SL_PLAYSTATE_PLAYING);
    STASSERT(SL_RESULT_SUCCESS == result);
}

void STAudioOutput::Stop()
{
    STBaseAudioOutput::Stop();

#ifdef  __SUPPORT_RECORD__
    if(iAudioRecorder != NULL)
    {
    	iAudioRecorder->Stop();
    	iAacEncoder->Finish();
    }
#endif

    Flush();
}

void STAudioOutput::Flush()
{
	STBaseAudioOutput::Flush();
    iCritical.Lock();
    if (iCurSampleBuffer != NULL)
    {
        iEmptiedBackgroundBufferArray.Append(iCurSampleBuffer);
        iCurSampleBuffer = NULL;
    }
    iCritical.UnLock();
    
    (*iPlayerBufferQueueItf)->Clear(iPlayerBufferQueueItf);
    
    iFlushed = ESTTrue;

    iTotalRenderSize = GetPlayPos();
}

void STAudioOutput::RenderBuffer(STSampleBuffer* aBuffer)
{
	STBaseAudioOutput::RenderBuffer(aBuffer);//把数据放入iFillBufferArray队列

	if (IsUnderflow())
	{
		FillBuffer();
	}
}

void STAudioOutput::SyncPosition(STUint aPosition)
{
	STBaseAudioOutput::SyncPosition(aPosition);
	STInt64 tmp = aPosition;
	tmp *= iSampleRate * iChannels * sizeof(STInt16);
	iTotalRenderSize = tmp / 1000;
}

STInt STAudioOutput::GetPlayPos()
{
	//TODO:这个地方有缺陷
//	SLmillisecond tMsec = 0;
//	(*iPlayerItf)->GetPosition(iPlayerItf, &tMsec);
//	STInt nPlayPos = (STInt)(((float)tMsec/1000)  * iChannels * iSampleRate * 2);
//	return nPlayPos == 0 ? iTotalRenderSize : nPlayPos;

	return iTotalRenderSize - KPCMBufferSize;
}

STInt STAudioOutput::Switch2Stream(STInt aStreamIdx)
{
	iCritical.Lock();
	iCurStreamIdx = aStreamIdx;
	iCritical.UnLock();

	return STKErrNone;
}

STInt STAudioOutput::GetCurSteramIndex()
{
	return iCurStreamIdx;
}

#ifdef __SUPPORT_RECORD__
STInt STAudioOutput::GetRecordDuration()
{
	return iAacEncoder->Duration();
}

STBool STAudioOutput::IsRecorderExisted()
{
	return iAudioRecorder != NULL;
}
#endif

STBool STAudioOutput::IsUnderflow()
{
	STBool bUnderflow = ESTFalse;
    iCritical.Lock();
    if (iCurSampleBuffer == NULL)
    {
    	STUint tState;
    	SLresult ret =  (*iPlayerItf)->GetPlayState(iPlayerItf, &tState);
    	if (SL_RESULT_SUCCESS == ret)
    	{
    		SLAndroidSimpleBufferQueueState pState;
			if (SL_RESULT_SUCCESS != (*iPlayerBufferQueueItf)->GetState(iPlayerBufferQueueItf, &pState))
			{
				bUnderflow = ESTTrue;
			}
			else
			{
				bUnderflow = (pState.count == 0);
			}
    	}
    	else
    	{
    		STLOGE("GetState Error");
    	}
    }
    iCritical.UnLock();
	return bUnderflow;
}

STInt STAudioOutput::GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
    iCritical.Lock();
    STInt nErr = STKErrUnderflow;
    aChannels = iChannels;
    STInt nSize = aSamples * aChannels * sizeof(STInt16);
    if (iCurSampleBuffer != NULL)
    {
        STInt nOffset = iCurSampleBuffer->GetPosition();
        STInt nTotoalSize = iCurSampleBuffer->Size();
        if (nSize > nTotoalSize - nOffset)
        {
            nOffset = nTotoalSize - nSize;
        }
        
        memcpy(aWave, iCurSampleBuffer->Ptr() + nOffset, nSize);
        nErr = STKErrNone;
    }
    iCritical.UnLock();
	return nErr;
}

STSampleBuffer*	STAudioOutput::GetFilledBuffer()
{
	STInt nNextOffset = 0;
    if (iCurSampleBuffer != NULL)
    {
    	nNextOffset = iCurSampleBuffer->GetByteOffset() + iCurSampleBuffer->Size();
    	iCurSampleBuffer->GetStreamIndex() == 0 ? iEmptiedBackgroundBufferArray.Append(iCurSampleBuffer) : iEmptiedOriginBufferArray.Append(iCurSampleBuffer);
        iCurSampleBuffer = NULL;
    }

    STSampleBuffer* pBuffer = NULL;
	if(iFillBufferArray.Count() < 5)
	{
		STLOGI("iFillBufferArray.Count() = %d",iFillBufferArray.Count());
	}
    while(iFillBufferArray.Count() > 0)
    {
    	pBuffer = iFillBufferArray[0];
    	iFillBufferArray.Remove(0);
		if (pBuffer->GetStreamIndex() != iCurStreamIdx || pBuffer->GetByteOffset() < nNextOffset)
		{
			pBuffer->GetStreamIndex() == 0 ? iEmptiedBackgroundBufferArray.Append(pBuffer) : iEmptiedOriginBufferArray.Append(pBuffer);
			pBuffer = NULL;
			continue;
		}
		break;
    }

	iDecodeThreadHandle->LockContext();
	iDecodeThreadHandle->Resume();
	iDecodeThreadHandle->UnlockContext();
	return pBuffer;
}

STSampleBuffer* STAudioOutput::RecycleBuffer(STBool aForceRecycle)
{
#ifdef  __SUPPORT_RECORD__
	if (iAudioRecorder != NULL)
	{
		iRecordCritical.Lock();
		if (iRecordLoopBufferValidSize > 0)
		{
//			STLOGE("iRecordLoopBufferValidSize1: %d", iRecordLoopBufferValidSize);
			STInt nProcessSize = 0;
			if (iRecrodLoopBuffer + KRecordLoopBufferSize - iRecordLoopBufferStartPtr >= iRecordLoopBufferValidSize)
			{
				nProcessSize = iRecordLoopBufferValidSize / (sizeof(STInt16) * iChannels) * (sizeof(STInt16) * iChannels);
				iRecordCritical.UnLock();

				ProcessEncode(iRecordLoopBufferStartPtr, nProcessSize);

				iRecordCritical.Lock();
				iRecordLoopBufferStartPtr += nProcessSize;
				iRecordLoopBufferValidSize -= nProcessSize;

				if (iRecordLoopBufferStartPtr >= iRecrodLoopBuffer + KRecordLoopBufferSize)
				{
					iRecordLoopBufferStartPtr = iRecrodLoopBuffer;
				}
			}
			else
			{
				STInt nRemainSize = iRecrodLoopBuffer + KRecordLoopBufferSize - iRecordLoopBufferStartPtr;
				STASSERT(nRemainSize % (sizeof(STInt16) * iChannels) == 0);
				nProcessSize = nRemainSize / (sizeof(STInt16) * iChannels) * (sizeof(STInt16) * iChannels);
				iRecordCritical.UnLock();

				ProcessEncode(iRecordLoopBufferStartPtr, nProcessSize);

				iRecordCritical.Lock();
				iRecordLoopBufferStartPtr = iRecrodLoopBuffer;
				iRecordLoopBufferValidSize -= nProcessSize;
				nProcessSize = iRecordLoopBufferValidSize / (sizeof(STInt16) * iChannels) * (sizeof(STInt16) * iChannels);
				iRecordCritical.UnLock();
// 
				iRecordCritical.Lock();
				iRecordLoopBufferStartPtr += nProcessSize;
				iRecordLoopBufferValidSize -= nProcessSize;
			}

//			STLOGE("iRecordLoopBufferValidSize2: %d", iRecordLoopBufferValidSize);
		}

		iRecordCritical.UnLock();
	}
#endif

	return STBaseAudioOutput::RecycleBuffer(aForceRecycle);
}

STInt STAudioOutput::GetMinCachedOffset()
{
#ifdef	__SUPPORT_RECORD__
	if (iAudioRecorder != NULL)
	{
//		STLOGE("iProcessedByteOffset:%d", iProcessedByteOffset);
		return iProcessedByteOffset;
	}
	else
#endif
	{
		return GetPlayPos();
	}
}

void STAudioOutput::Position(STUint& aPosition)
{
    iCritical.Lock();  
    if (iCurSampleBuffer != NULL)
    {        
        iCurPosition = (((STInt64)(iCurSampleBuffer->GetByteOffset() + iCurSampleBuffer->GetPosition())) * 1000) / (iSampleRate * iChannels * sizeof(STInt16));
    }
    iCritical.UnLock();
    
    STBaseAudioOutput::Position(aPosition);
}

void STAudioOutput::SetOutputVolume(STInt aVolume)
{
	int nAttenuation = 100 - aVolume;
	int nMillibel = nAttenuation * -50;
	if (NULL != iPlayerVolumeItf)
	{
		SLresult result = (*iPlayerVolumeItf)->SetVolumeLevel(iPlayerVolumeItf, nMillibel);
		STASSERT(SL_RESULT_SUCCESS == result);
	}
}

void STAudioOutput::SetAccompanimentVolume(STInt aVolume)
{
	CLIP(aVolume, 0, 100);
	iCritical.Lock();
	iAccompanimentVolume = (aVolume * 0xFFF) / 100;
	iCritical.UnLock();

	SetOutputVolume(aVolume);
}

void STAudioOutput::SetRecorderVolume(STInt aVolume)
{
	iCritical.Lock();
	iRecordVolume = (aVolume * 0xFFF) / 100;
    iCritical.UnLock();
}

STInt STAudioOutput::CreateBufferQueueAudioPlayer(STInt sampleRate, STInt channels)
{
    SLresult result;
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channels, sampleRate*1000,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        channels == 2 ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT) : SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, iOpenSLESEngine->GetOutputMixObjectItf()};// configure audio sink
    SLDataSink audioSnk = {&loc_outmix, NULL};
    
    const SLInterfaceID ids[3] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};// create audio player
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*iOpenSLESEngine->GetEngineItf())->CreateAudioPlayer(iOpenSLESEngine->GetEngineItf(), &iPlayerObjectItf, &audioSrc, &audioSnk, 3, ids, req);
    if (SL_RESULT_SUCCESS != result) return STKErrNotSupported;
    
    result = (*iPlayerObjectItf)->Realize(iPlayerObjectItf, SL_BOOLEAN_FALSE);// realize the player
    STASSERT(SL_RESULT_SUCCESS == result);
    
    result = (*iPlayerObjectItf)->GetInterface(iPlayerObjectItf, SL_IID_PLAY, &iPlayerItf);// get the play interface
    STASSERT(SL_RESULT_SUCCESS == result);
    
    result = (*iPlayerObjectItf)->GetInterface(iPlayerObjectItf, SL_IID_BUFFERQUEUE, &iPlayerBufferQueueItf);// get the buffer queue interface
    STASSERT(SL_RESULT_SUCCESS == result);
    
	result = (*iPlayerBufferQueueItf)->RegisterCallback(iPlayerBufferQueueItf, AudioCallback, this);// register callback on the buffer queue
	STASSERT(SL_RESULT_SUCCESS == result);
    
    result = (*iPlayerObjectItf)->GetInterface(iPlayerObjectItf, SL_IID_VOLUME, &iPlayerVolumeItf);// get the volume interface
    STASSERT(SL_RESULT_SUCCESS == result);
    
    return STKErrNone;
}

void STAudioOutput::AudioCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    ((STAudioOutput*)context)->FillBuffer();
}

void STAudioOutput::FillBuffer() //播放pcm数据
{
	iCritical.Lock();
	if (iCurSampleBuffer == NULL || iCurSampleBuffer->ValidSize() == 0)
	{
		iCurSampleBuffer = GetFilledBuffer();
	}
	
	if (iCurSampleBuffer == NULL)
	{
		iCritical.UnLock();
		STLOGI("UnderFlow 210" ); //没有准备好pcm数据,可能出现读数据跟不上播放速度
		return;
	}
	
	iFlushed = ESTFalse;
	
	STInt nSrcOffset = iCurSampleBuffer->GetPosition();
	STInt nValidSize = iCurSampleBuffer->ValidSize();
	STInt nRenderSize = nValidSize >= (KPCMBufferSize >> 2) ? (KPCMBufferSize >> 2) : nValidSize;
	if (SL_RESULT_SUCCESS == (*iPlayerBufferQueueItf)->Enqueue(iPlayerBufferQueueItf, iCurSampleBuffer->Ptr() + nSrcOffset, nRenderSize))
	{
		iTotalRenderSize += nRenderSize;
		iCurSampleBuffer->SetPosition(nSrcOffset + nRenderSize);
	}
	else
	{
		STLOGE("Player Enqueue error");
	}

	iCritical.UnLock();
}

#ifdef __SUPPORT_RECORD__
STInt STAudioOutput::GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
	aChannels = aChannels;
	STInt nCpySize = aSamples * sizeof(STInt16) * iChannels;
	STUint8* pDstBuffer = (STUint8*)aWave;

	iRecordCritical.Lock();
	if (iRecrodLoopBuffer != NULL && nCpySize > 0)
	{
		STASSERT(nCpySize <= KRecordLoopBufferSize);
		STASSERT(iRecordLoopBufferStartPtr >= iRecrodLoopBuffer && iRecordLoopBufferStartPtr <= iRecrodLoopBuffer + KRecordLoopBufferSize);

		STUint8* pLastDataPtr = iRecordLoopBufferStartPtr + iRecordLoopBufferValidSize;
		if (pLastDataPtr - iRecrodLoopBuffer > KRecordLoopBufferSize)
		{
			pLastDataPtr = iRecrodLoopBuffer + (pLastDataPtr - iRecrodLoopBuffer - KRecordLoopBufferSize);
		}

		STInt nHeadSize = pLastDataPtr - iRecrodLoopBuffer;
		if (nHeadSize >= nCpySize)
		{
			memcpy(pDstBuffer, pLastDataPtr - nCpySize, nCpySize);
		}
		else
		{
			STInt nTailSize = nCpySize - nHeadSize;
			memcpy(pDstBuffer, iRecrodLoopBuffer + KRecordLoopBufferSize - nTailSize, nTailSize);
			memcpy(pDstBuffer + nTailSize, iRecrodLoopBuffer, nHeadSize);
		}
		iRecordCritical.UnLock();
	}
	else
	{
		memset(aWave, 0, nCpySize);
	}
	iRecordCritical.UnLock();

	return STKErrNone;
}

static inline STInt mulAdd(STInt16 in, STInt16 v, STInt a)
{
#if defined(__arm__) && !defined(__thumb__)
	STInt out;
    asm( "smlabb %[out], %[in], %[v], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + in * STInt(v);
#endif
}

static inline STInt mul(STInt16 in, STInt16 v)
{
#if defined(__arm__) && !defined(__thumb__)
	STInt out;
    asm( "smulbb %[out], %[in], %[v] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v)
         : );
    return out;
#else
    return in * STInt(v);
#endif
}

static inline STInt16 clamp16(STInt32 sample)
{
    if ((sample>>15) ^ (sample>>31))
        sample = 0x7FFF ^ (sample>>31);
    return sample;
}


STInt STAudioOutput::ProcessEncode(STUint8* aBuffer, STInt aSize)
{
	STInt nSizeNeedProcessed = aSize;
	STInt size = iProcessedByteOffset;

	iCritical.Lock();
	STInt nEmptiedBufferCount = iEmptiedBackgroundBufferArray.Count();

//	if (nEmptiedBufferCount > 0 && iProcessedByteOffset < iEmptiedBackgroundBufferArray[0]->GetByteOffset())
//	{
//		STLOGE("+++++++++++++++ overflow 1 +++++++++++++++++++");
//	}
//	else if (iCurSampleBuffer != NULL && iProcessedByteOffset + nSizeNeedProcessed > iCurSampleBuffer->GetByteOffset() + iCurSampleBuffer->Size())
//	{
//		STLOGE("+++++++++++++++ overflow 2 +++++++++++++++++++");
//	}

	STUint8* pBufferPtr = aBuffer;
	STUint8* pDstBufferPtr = iMixBuffer;
	for (STInt i = 0; i < nEmptiedBufferCount; i++)
	{
		STUint nCurBufferByteOffset = iEmptiedBackgroundBufferArray[i]->GetByteOffset();
		STUint nSize = iEmptiedBackgroundBufferArray[i]->Size();
		if (iProcessedByteOffset >= nCurBufferByteOffset && iProcessedByteOffset < nCurBufferByteOffset + nSize)
		{
			STInt nRemainSize = nSize + nCurBufferByteOffset - iProcessedByteOffset;
			STInt nStartPos = iProcessedByteOffset - nCurBufferByteOffset;

			if (nSizeNeedProcessed <= nRemainSize)
			{
				iCritical.UnLock();
				Mix((STInt16*)pDstBufferPtr, (STInt16*)(pBufferPtr), (STInt16*)(iEmptiedBackgroundBufferArray[i]->Ptr() + nStartPos), nSizeNeedProcessed >> 1);
				iCritical.Lock();
				iProcessedByteOffset += nSizeNeedProcessed;
				nSizeNeedProcessed = 0;
				break;
			}
			else
			{
				iCritical.UnLock();
				Mix((STInt16*)pDstBufferPtr, (STInt16*)(pBufferPtr), (STInt16*)(iEmptiedBackgroundBufferArray[i]->Ptr() + nStartPos), nRemainSize >> 1);
				iCritical.Lock();
				pBufferPtr += nRemainSize;
				pDstBufferPtr += nRemainSize;
				iProcessedByteOffset += nRemainSize;
				nSizeNeedProcessed -= nRemainSize;
			}
		}
	}

	if (nSizeNeedProcessed > 0)//如果要处理的录音数据还大于0，则混合当前iCurSampleBuffer，中的数据（已经投入播放队列的数据）
	{
		if (iCurSampleBuffer != NULL && nSizeNeedProcessed < iCurSampleBuffer->GetPosition())
		{
			iCritical.UnLock();
			Mix((STInt16*)pDstBufferPtr, (STInt16*)(pBufferPtr), (STInt16*)(iCurSampleBuffer->Ptr() + (iProcessedByteOffset - iCurSampleBuffer->GetByteOffset())), nSizeNeedProcessed >> 1);
			iCritical.Lock();
			iProcessedByteOffset += nSizeNeedProcessed;
			nSizeNeedProcessed = 0;
		}
		else
		{
			STLOGE("error record faster than play !");
			if(iCurSampleBuffer == NULL)
			{
				STLOGE("error record faster than play !,nSizeNeedProcessed=%d,",nSizeNeedProcessed);
			}
			else
			{
				STLOGE("error record faster than play !,nSizeNeedProcessed=%d,iCurSampleBuffer->GetPosition()=%d",nSizeNeedProcessed,iCurSampleBuffer->GetPosition());
			}

		}
	}
	iCritical.UnLock();

	
	return iAacEncoder->Execute(iMixBuffer, iProcessedByteOffset - size);
}

void STAudioOutput::Mix(STInt16* aDstBuffer, STInt16* aRecordBuffer, STInt16* aBackgroundBuffer, STInt aSizeInShort)
{


	STInt16* pSrc1 = aRecordBuffer;
	STInt16* pSrc2 = aBackgroundBuffer;

	STInt nProcessSample = aSizeInShort / iChannels;

	if (iChannels == 2)
	{
		STInt32* pDstBuffer = (STInt32*)aDstBuffer;
		while (nProcessSample--)
		{
			STInt nLeft0 = *pSrc1++;
			STInt nRight0 = *pSrc1++;
			nLeft0 = mul(nLeft0, iAccompanimentVolume);
			nRight0 = mul(nRight0, iAccompanimentVolume);
			STInt nLeft1 = *pSrc2++;
			STInt nRight1 = *pSrc2++;

			nLeft1 = mulAdd(nLeft1, iRecordVolume, nLeft0) >> 12;
			nRight1 = mulAdd(nRight1, iRecordVolume, nRight0) >> 12;

			nLeft1 = clamp16(nLeft1);
			nRight1 = clamp16(nRight1);

			*pDstBuffer++ = (nRight1 << 16) | (nLeft1 & 0xFFFF);
		}
	}
	else
	{
		STInt16* pDstBuffer = (STInt16*)aDstBuffer;
		while (nProcessSample--)
		{
			STInt nLeft0 = *pSrc1++;
			nLeft0 = mul(nLeft0, iAccompanimentVolume);

			STInt nLeft1 = *pSrc2++;
			nLeft1 = mulAdd(nLeft1, iRecordVolume, nLeft0) >> 12;

			nLeft1 = clamp16(nLeft1);

			*pDstBuffer++ = (nLeft1 & 0xFFFF);
		}
	}
}

void STAudioOutput::RecorderError(STInt aErr)
{
	
}

STInt STAudioOutput::RecorderBufferFilled(STUint8* aBufferPtr, STInt aBufferSize, STInt aByteOffset, STUint& aState)
{
	iRecordCritical.Lock();
	STInt nSizeNeedProcessed = aBufferSize;
	STInt nRecordByteOffset = aByteOffset;
	if (iPlayerItf != NULL)
	{
		SLresult ret =  (*iPlayerItf)->GetPlayState(iPlayerItf, &aState);

		STInt nPlayPos = GetPlayPos();

		STInt nRecordPos = nRecordByteOffset + aBufferSize;
		if (nRecordPos > nPlayPos && nPlayPos > nRecordByteOffset)
		{
			nSizeNeedProcessed = (nPlayPos - nRecordByteOffset) / (sizeof(STInt16) * iChannels) * (sizeof(STInt16) * iChannels);
		}
//		STLOGE("recordPos:%d nPlayPos:%d, recordDelay: %f", nRecordPos, nPlayPos, (float)(nRecordPos - nPlayPos)/iSampleRate/2/iChannels);
	}
	else
	{
		aState = SL_PLAYSTATE_STOPPED;
		nSizeNeedProcessed /= 2;
	}

	if (KRecordLoopBufferSize - iRecordLoopBufferValidSize >= nSizeNeedProcessed)//数据够
	{
		STUint8* pEndPtr = iRecordLoopBufferStartPtr + iRecordLoopBufferValidSize;
		if (pEndPtr >= iRecrodLoopBuffer + KRecordLoopBufferSize)
		{
			pEndPtr = iRecrodLoopBuffer + (pEndPtr - (iRecrodLoopBuffer + KRecordLoopBufferSize));
			memcpy(pEndPtr, aBufferPtr, nSizeNeedProcessed);
		}
		else
		{
			STInt nEndRemainSize = iRecrodLoopBuffer + KRecordLoopBufferSize - pEndPtr;
			if (nEndRemainSize >= nSizeNeedProcessed)
			{
				memcpy(pEndPtr, aBufferPtr, nSizeNeedProcessed);
			}
			else
			{
				memcpy(pEndPtr, aBufferPtr, nEndRemainSize);
				memcpy(iRecrodLoopBuffer, aBufferPtr + nEndRemainSize, nSizeNeedProcessed - nEndRemainSize);
			}
		}
	}
	else
	{
		STLOGE("Record Data Buffer need realloc");
		nSizeNeedProcessed = 0;
	}

	iRecordLoopBufferValidSize += nSizeNeedProcessed;
	iRecordCritical.UnLock();

	iDecodeThreadHandle->LockContext();
	iDecodeThreadHandle->Resume();
	iDecodeThreadHandle->UnlockContext();

	return nSizeNeedProcessed;
}

STInt STAudioOutput::setAudioImpulsePath(const STChar* path,int flag)
{
	//设置混响类的属性
	if(m_pSTReverb != NULL)
	{
		return (STInt)0;//m_pSTReverb->SetIRFilePath(path,flag);
	}
	else
	{
		return STKErrBadHandle;
	}
	//return STInt(0);
}

#endif

