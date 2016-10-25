#include "TTOsalConfig.h"
#include "TTAudioProcess.h"
#include "TTMediaPlayerItf.h"
#include "TTSysTime.h"
#include "TTLog.h"
#ifdef __TT_OS_ANDROID__
#include "unistd.h"
#endif

extern TTInt gMaxOutPutSamplerate;
extern TTInt gAudioEffectLowDelay;

TTCAudioProcess::TTCAudioProcess(CTTSrcDemux* SrcMux, TTInt nAudioCount)
:mSrcMux(SrcMux)
,mAudioProcNum(nAudioCount)
,mAudioCount(nAudioCount)
,mDecoder(NULL)
,mSrcBuffer(NULL)
,mStatus(EStatusStoped)
,mPCMBuf(NULL)
,mPCMBufSize(0)
,mSinkBuf(NULL)
,mSinkBufSize(0)
,mCurTime(0)
,mListBuffer(NULL)
,mListFull(0)
,mListUsing(0)
,mFlushing(false)
,mSeeking(false)
,mEOS(false)
,mSetProirity(false)
,mEffectEnable(false)
,mWaveBuf(NULL)
,mWaveBuffer(NULL)
,mWaveFull(0)
,mWaveUsing(0)
,mDoWave(0)
,mResmpFactor(0)
,mResampleObj(NULL)
,mSampleRateMAX(gMaxOutPutSamplerate)
,mSampleRateSet(0)
,mChannelSet(0)
,mAudioBufferSize(KSinkBufferSize)
,mAudioEffectLowDelay(gAudioEffectLowDelay) 
,mCurBuffer(NULL)
,mProcThread(NULL)
,mDoConvert(0)
,mDoEffect(false)
{
	mCriList.Create();
	mCritical.Create();
	mCriEvent.Create();
	mWaveList.Create();

	memset(&mPCMBuffer, 0, sizeof(TTBuffer));
	memset(&mAudioFormatIn, 0, sizeof(TTAudioFormat));
	memset(&mAudioFormatOut, 0, sizeof(TTAudioFormat));

	mAudioStepTime = 50;

//#ifdef __TT_OS_ANDROID__
	if(nAudioCount <= 5) {
		mAudioStepTime = 100;
	}else {
		mAudioStepTime = 200;
	}
//#endif

#ifdef __DUMP2_PCM__
	DumpFile = NULL;
	DumpFile = fopen("D:\\Dump2.pcm", "wb");
#endif
}

TTCAudioProcess::~TTCAudioProcess()
{
	uninitProc();
	freeWaveBuffer();
	SAFE_DELETE(mDecoder);
	SAFE_DELETE(mProcThread);
	SAFE_DELETE(mListBuffer);
	SAFE_FREE(mPCMBuf);
	SAFE_FREE(mSinkBuf);
	SAFE_DELETE(mResampleObj);
	mWaveList.Destroy();
	mCriList.Destroy();
	mCriEvent.Destroy();
	mCritical.Destroy();

#ifdef __DUMP2_PCM__
	if(DumpFile)
		fclose(DumpFile);
#endif
}

TTInt TTCAudioProcess::initProc(TTAudioInfo* pCurAudioInfo)
{
	TTCAutoLock Lock(&mCritical);
	TTInt nErr = TTKErrNone;
	mStatus = EStatusStarting;

	if(mDecoder == NULL)
		mDecoder = new CTTAudioDecode(mSrcMux, mAudioStepTime);

	nErr = mDecoder->initDecode(pCurAudioInfo);	
	if(nErr != TTKErrNone)
		return nErr;
	
	updateParam();	
	if(pCurAudioInfo != NULL) {
		pCurAudioInfo->iChannel = mAudioFormatIn.Channels;
		pCurAudioInfo->iSampleRate = mAudioFormatIn.SampleRate;
	}

	if(mAudioCount > 1) {
		if(mProcThread == NULL)
			mProcThread =  new TTEventThread("TTAudio PostProcess");

		nErr = allocBuffer();
	}

	mWaveList.Lock();
	mWaveFull = 0;
	mWaveUsing = 0;
	mWaveList.UnLock();

	mSetProirity = false;
	mEOS = false;

	mStatus = EStatusPrepared;
	return nErr;
}

TTInt TTCAudioProcess::uninitProc()
{
	TTCAutoLock Lock(&mCritical);

	stop();
	freeBuffer();
	SAFE_DELETE(mProcThread);	
	if(mDecoder)
		mDecoder->uninitDecode();
	return TTKErrNone;
}

TTInt TTCAudioProcess::updateParam()
{
	TTInt nErr = TTKErrNone;
	if(mDecoder == NULL)
		return TTKErrNotFound;

	mDecoder->getParam(TT_PID_AUDIO_FORMAT, &mAudioFormatIn);

	mDoConvert = 0;
	if(mChannelSet) {
		mAudioFormatOut.Channels = mChannelSet;
		if(mChannelSet != mAudioFormatIn.Channels)
			mDoConvert |= 1;
	} else {
		if(mAudioFormatIn.Channels > 0 && mAudioFormatIn.Channels <= 2) {
			mAudioFormatOut.Channels = mAudioFormatIn.Channels;
		} else {
			mAudioFormatOut.Channels = 2;
			mDoConvert |= 1;
		}
	}

	if(mSampleRateSet && mAudioFormatIn.SampleRate != mSampleRateSet){
		mAudioFormatOut.SampleRate = mSampleRateSet;
		mDoConvert |= 2;
	}else {
		if(mAudioFormatIn.SampleRate >= 8000 && mAudioFormatIn.SampleRate <= mSampleRateMAX) {
			mAudioFormatOut.SampleRate = mAudioFormatIn.SampleRate;
		} else {
			TTInt nSampleRate = mAudioFormatIn.SampleRate;
			if(mAudioFormatIn.SampleRate > mSampleRateMAX) {
				while(nSampleRate > mSampleRateMAX) {
					nSampleRate >>= 1;
				}
				
				if(nSampleRate != 48000 && nSampleRate != 44100 && nSampleRate != 32000 && nSampleRate != 24000) {
					nSampleRate = mSampleRateMAX;
				}

			} else if(mAudioFormatIn.SampleRate < 8000){
				while(nSampleRate < 8000) {
					nSampleRate <<= 1;
				}

				if(nSampleRate != 8000 && nSampleRate != 12000 && nSampleRate != 16000) {
					nSampleRate = 8000;
				}
			}
			mAudioFormatOut.SampleRate = nSampleRate;
			mDoConvert |= 2;
		}
	}

	if(mDoConvert & 2) {
		SAFE_DELETE(mResampleObj);

		mResampleObj = new aflibConverter(ETTFalse, ETTFalse, ETTTrue);
		mResmpFactor = (double)mAudioFormatOut.SampleRate / mAudioFormatIn.SampleRate;
		mResampleObj->initialize(mResmpFactor, mAudioFormatOut.Channels);
	}

	if(mAudioFormatIn.SampleBits != 16)
		mDoConvert |= 4;

	mAudioFormatOut.SampleBits = 16;

	//CTTAudioEffectManager::Config(mAudioFormatOut.Channels, mAudioFormatOut.SampleRate);

	return nErr;
}

TTInt TTCAudioProcess::allocBuffer()
{
	freeBuffer();

	if(mAudioProcNum > 1) {
		TTCAutoLock Lock(&mCriList);
		//if(!gAudioEffectLowDelay) {
		//	mAudioCount = mAudioProcNum*2;
		//} else {
			mAudioCount = mAudioProcNum;
		//}
		
		mAudioBufferSize = mAudioFormatIn.SampleRate*mAudioFormatIn.Channels*mAudioFormatIn.SampleBits/(8*4);
		if(mAudioBufferSize < KSinkBufferSize)
			mAudioBufferSize = KSinkBufferSize;
		mSinkBufSize = mAudioBufferSize*mAudioCount;
		mSinkBuf = (TTPBYTE)malloc(mSinkBufSize);
		if(mSinkBuf == NULL)
			return TTKErrNoMemory;	
		memset(mSinkBuf, 0, sizeof(TTBYTE)*mSinkBufSize);
		
		mListBuffer = new TTBuffer*[mAudioCount];
		if(mListBuffer == NULL)
			return TTKErrNoMemory;
		
		for(TTUint i = 0; i < mAudioCount; i++) {
			mListBuffer[i] = new TTBuffer;
			mListBuffer[i]->nSize = KSinkBufferSize;
			mListBuffer[i]->pBuffer = mSinkBuf + i*KSinkBufferSize;
		}

		mListFull = 0;
		mListUsing = 0;
	}

	return TTKErrNone;
}

TTInt TTCAudioProcess::freeBuffer()
{
	TTCAutoLock Lock(&mCriList);

	if(mListBuffer == NULL)	{
		mListFull = 0;
		mListUsing = 0;
		SAFE_FREE(mSinkBuf);
		return TTKErrNone;
	}

	for(TTUint i = 0; i < mAudioCount; i++)	{
		SAFE_DELETE(mListBuffer[i]);
	}

	SAFE_FREE(mSinkBuf);
	SAFE_DELETE(mListBuffer);

	mListFull = 0;
	mListUsing = 0;
	
	return TTKErrNone;
}

TTInt TTCAudioProcess::getCurWave(TTInt64 aPlayingTime, TTInt aSamples, TTInt16* aWave, TTInt& aChannels)
{
	TTCAutoLock Lock(&mWaveList);
	if(mDoWave == 0) {
		mDoWave = 1;
		allocWaveBuffer();
		return TTKErrNotReady;
	}

	if(mWaveUsing >= mWaveFull)
		return TTKErrNotReady;


	TTInt n = mWaveUsing;
	TTInt nIndex = mWaveUsing%20;
	TTInt nFound = mWaveUsing;

	//if(mWaveBuffer[nIndex]->llTime > aPlayingTime + 100) {
	//	return TTKErrNotReady;
	//}

	for(n = mWaveUsing; n <= mWaveFull; n++) {
		nIndex = n%20;
		if(mWaveBuffer[nIndex]->llTime > aPlayingTime)
			break;
		nFound = n;
	}

	nIndex = nFound%20;
	aChannels = mWaveBuffer[nIndex]->lReserve;
	memcpy(aWave, mWaveBuffer[nIndex]->pBuffer, aSamples*2*aChannels);

	return TTKErrNone;
}


TTInt TTCAudioProcess::updateWaveBuffer(TTBuffer* dstBuffer)
{
	TTCAutoLock Lock(&mWaveList);
	if(!mEffectEnable || !mAudioEffectLowDelay)
		return TTKErrNone;
	if(mWaveBuffer == NULL)
		return TTKErrNone;

	TTInt nNum = mAudioStepTime/50;
	if(nNum == 0) nNum = 1;
	TTInt n = 0;
	TTInt nIndex = mWaveFull%20;
	TTInt nSize = 1024*2*mAudioFormatOut.Channels;
	TTInt nLeftSize = dstBuffer->nSize;
	TTInt nStepSize = nLeftSize/nNum;
	TTInt nWriteSize = nSize;
	TTInt nIdx = mWaveUsing%20;
	
	for(n = 0; n < nNum; n++) {
		nIndex = mWaveFull%20;
		nIdx = mWaveUsing%20;
		if(nIdx == nIndex && mWaveFull > mWaveUsing) {
			mWaveUsing++;
		}
		nWriteSize = nSize;
		if(nWriteSize > nLeftSize)
			nWriteSize = nLeftSize;	
		memcpy(mWaveBuffer[nIndex]->pBuffer, dstBuffer->pBuffer + n*nStepSize, nWriteSize);
		mWaveBuffer[nIndex]->nSize = nWriteSize;
		mWaveBuffer[nIndex]->llTime = dstBuffer->llTime + n*50;
		mWaveBuffer[nIndex]->lReserve = mAudioFormatOut.Channels;
		mWaveFull++;
		nLeftSize -= nStepSize;
	}

	return TTKErrNone;
}

TTInt TTCAudioProcess::allocWaveBuffer()
{
	TTCAutoLock Lock(&mWaveList);
	if(mWaveBuf != NULL && mWaveBuffer != NULL)
		return TTKErrNone;

	mWaveBuf = (TTPBYTE)malloc(80 * 1024);
	if(mWaveBuf == NULL)
		return TTKErrNoMemory;	
	mWaveBuffer = new TTBuffer*[20];
	if(mWaveBuffer == NULL)
		return TTKErrNoMemory;

	for(TTUint i = 0; i < 20; i++) {
		mWaveBuffer[i] = new TTBuffer;
		mWaveBuffer[i]->nSize = 4*1024;
		mWaveBuffer[i]->pBuffer = mWaveBuf + i*4*1024;
	}
	mWaveFull = 0;
	mWaveUsing = 0;
	return TTKErrNone;
}

TTInt TTCAudioProcess::freeWaveBuffer()
{
	TTCAutoLock Lock(&mWaveList);

	if(mWaveBuffer == NULL)	{
		mWaveFull = 0;
		mWaveUsing = 0;
		SAFE_FREE(mWaveBuf);
		return TTKErrNone;
	}

	for(TTUint i = 0; i < 20; i++)	{
		SAFE_DELETE(mWaveBuffer[i]);
	}

	SAFE_FREE(mWaveBuf);
	SAFE_DELETE(mWaveBuffer);
	mWaveFull = 0;
	mWaveUsing = 0;	
	return TTKErrNone;
}


TTInt TTCAudioProcess::start()
{
	TTCAutoLock Lock(&mCritical);

	if(mProcThread != NULL)
		mProcThread->start();

	if(mDecoder)
		mDecoder->start();


	mStatus = EStatusPlaying;
	mEOS = false; 

	TTCAutoLock LockEvent(&mCriEvent);
	postAudioProcEvent (-1);
	return TTKErrNone;
}

TTInt TTCAudioProcess::stop()
{
	TTCAutoLock Lock(&mCritical);
	mStatus = EStatusStoped;
	if(mProcThread != NULL)
		mProcThread->stop();

	if(mDecoder)
		mDecoder->stop();

	mWaveList.Lock();
	mWaveFull = 0;
	mWaveUsing = 0;
	mWaveList.UnLock();

	TTCAutoLock listLock(&mCriList);
	mCurBuffer = NULL;
	mListFull = 0;
	mListUsing = 0;
	return TTKErrNone;
}

TTInt TTCAudioProcess::pause()
{
	TTCAutoLock Lock(&mCritical);
	mStatus = EStatusPaused;

	if(mDecoder)
		mDecoder->pause();

	return TTKErrNone;
}

TTInt TTCAudioProcess::resume()
{
	TTCAutoLock Lock(&mCritical);

	if(mDecoder)
		mDecoder->resume();

	mStatus = EStatusPlaying;
	TTCAutoLock LockEvent(&mCriEvent);
	postAudioProcEvent (-1);
	return TTKErrNone;
}

TTInt TTCAudioProcess::syncPosition(TTUint64 aPosition, TTInt aOption)
{
	flush();
	mCriList.Lock();
	mSeeking = true;
	mEOS = false; 
	mCriList.UnLock();

	return TTKErrNone;
}

TTInt TTCAudioProcess::flush()
{
	TTCAutoLock Lock(&mCritical);
	if(mProcThread)
		mProcThread->cancelAllEvent();

	mCriList.Lock();
	mCurBuffer = NULL;
	mListFull = 0;
	mListUsing = 0;
	mFlushing = true;
	mCriList.UnLock();

	mWaveList.Lock();
	mWaveFull = 0;
	mWaveUsing = 0;
	mWaveList.UnLock();

	if(mDecoder)
		mDecoder->flush();

	return TTKErrNone;
}

TTInt TTCAudioProcess::getOutputBuffer(TTBuffer* dstBuffer)
{
	TTInt nErr = TTKErrNone;

	if(dstBuffer == NULL)
		return TTKErrArgument;

	mCurTime = dstBuffer->llTime;
	if(mAudioCount > 1) {
		TTCAutoLock Lock(&mCriList);
		if(mCurBuffer) 	{
			mListUsing++;
			mCurBuffer = NULL;

			TTCAutoLock LockEvent(&mCriEvent);
			postAudioProcEvent (-1);				
		}

		if(mListFull < mAudioCount - 1 && !mEOS) {
			return TTKErrUnderflow;
		}

		if(mListUsing >= mListFull) {
			if(mLastSysTime == 0) {
				mLastSysTime = GetTimeOfDay();
			} else {
				//LOGI("TTCAudioProcess::mListUsing %d, mListFull %d, SysDiff %d", mListUsing, mListFull, (TTInt)(GetTimeOfDay() - mLastSysTime));
			}
			return TTKErrUnderflow;
		} 
		mLastSysTime = 0;

		int nIndex = mListUsing%mAudioCount;
		mCurBuffer = mListBuffer[nIndex];
		

		dstBuffer->pBuffer = mCurBuffer->pBuffer;
		dstBuffer->nSize = mCurBuffer->nSize;
		dstBuffer->llTime = mCurBuffer->llTime;
		dstBuffer->nFlag = mCurBuffer->nFlag;
		dstBuffer->pData = mCurBuffer->pData;
		dstBuffer->nDuration = mCurBuffer->nDuration;
		dstBuffer->lReserve = mCurBuffer->lReserve;
		dstBuffer->pReserve = mCurBuffer->pReserve;
#ifdef __DUMP2_PCM__
		if(DumpFile) {
			fwrite(dstBuffer->pBuffer, dstBuffer->nSize, 1, DumpFile);
		}
#endif
		return nErr;
	} else {
		if(dstBuffer->pBuffer == NULL || dstBuffer->nSize <= 0)
			return TTKErrArgument;

		dstBuffer->lReserve = 0;
		nErr =  doAudioProcess(dstBuffer);
	}

	return nErr;
}

TTInt TTCAudioProcess::doDecodeFrames(TTBuffer* dstBuffer)
{
	TTInt nErr = TTKErrNotFound;
	
	if(mDecoder == NULL)
		return nErr;

	if(mDoConvert) {
		if(mPCMBuf == NULL) {
			mPCMBufSize = KDecodePCMBufferSize;
			mPCMBuf = (TTPBYTE)malloc(mPCMBufSize);
			if(mPCMBuf == NULL)
				return TTKErrNoMemory;
		}

		mPCMBuffer.pBuffer = mPCMBuf;
		mPCMBuffer.nSize = mPCMBufSize;
		mPCMBuffer.llTime = dstBuffer->llTime;
		mPCMBuffer.nFlag = dstBuffer->nFlag;
		mSrcBuffer = &mPCMBuffer;
	} else {
		mSrcBuffer = dstBuffer;
	}
	
	if(mDecoder)
		nErr = mDecoder->getOutputBuffer(mSrcBuffer);

	return nErr;
}

TTInt TTCAudioProcess::doAudioProcess(TTBuffer* dstBuffer)
{	
	TTInt nErr = TTKErrNone;
	if(dstBuffer == NULL)
		return TTKErrArgument;

	nErr = doDecodeFrames(dstBuffer);
	if(nErr == TTKErrFormatChanged)
		updateParam();

	if(mDoConvert)
		doSampleConvert(dstBuffer);
	dstBuffer->lReserve = 0;


	return nErr;
}

TTInt TTCAudioProcess::doChannelDoMix(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt i;
	TTInt16* InBuf = (TTInt16*)srcBuffer->pBuffer;
	TTInt16* OutBuf = (TTInt16*)dstBuffer->pBuffer;
	TTInt  Length = srcBuffer->nSize/(mAudioFormatIn.Channels*sizeof(TTInt16));
	TTInt  InChan = mAudioFormatIn.Channels;
	TTInt  OutChan = mAudioFormatOut.Channels;

	if(InChan == 6 && OutChan == 2)	{
		TTInt C,L_S,R_S,tmp,tmp1,cum;
#define DM_MUL 5248/16384  //3203/10000
#define RSQRT2 5818/8192	//7071/10000
#define CLIPTOSHORT(x)  ((((x) >> 31) == (x >> 15))?(x):((x) >> 31) ^ 0x7fff)

		for(i = 0; i < Length; i++)
		{
			C   = InBuf[2]*RSQRT2;
			L_S = InBuf[4]*RSQRT2;
			cum = InBuf[0] + C + L_S;
			tmp = cum*DM_MUL;

			R_S = InBuf[5]*RSQRT2;
			cum = InBuf[1] + C + R_S;
			tmp1 = cum*DM_MUL;

			OutBuf[0] = (TTInt16)CLIPTOSHORT(tmp);
			OutBuf[1] = (TTInt16)CLIPTOSHORT(tmp1);
			OutBuf+=OutChan;
			InBuf+=InChan;
		}

		dstBuffer->nSize = Length*sizeof(TTInt16)*mAudioFormatOut.Channels;
	} else if(InChan > 2 && InChan != 6 && OutChan == 2) {
		for(i = 0; i < Length; i++)	{
			OutBuf[0] = InBuf[0];
			OutBuf[1] = InBuf[1];
			OutBuf += OutChan;
			InBuf += InChan;
		}

		dstBuffer->nSize = Length*sizeof(TTInt16)*mAudioFormatOut.Channels;		
	}

	dstBuffer->nFlag = srcBuffer->nFlag;
	dstBuffer->llTime = srcBuffer->llTime;
	dstBuffer->nDuration = srcBuffer->nDuration;
	dstBuffer->pData = srcBuffer->pData;
	dstBuffer->lReserve = srcBuffer->lReserve;
	dstBuffer->pReserve = srcBuffer->pReserve;

	return TTKErrNone;
}

void  TTCAudioProcess::convert8BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize;

	if(dstBuffer->nSize < nCnt*2) {
		nCnt = dstBuffer->nSize/2;
	}

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTInt16* pDst = (TTInt16*)(dstBuffer->pBuffer);
	
	TTInt16 nSignedShort; 
	for (TTInt i = 0; i < nCnt; i++) {
		nSignedShort = *pSrc++ - 128;
		nSignedShort = nSignedShort<<8;
		*pDst++ = nSignedShort;
	}
	
	dstBuffer->nSize = nCnt*sizeof(TTInt16);

	if(mDoConvert > 4 ) {
		memcpy(srcBuffer->pBuffer, dstBuffer->pBuffer, dstBuffer->nSize);
		srcBuffer->nSize = dstBuffer->nSize;
	}
}

void  TTCAudioProcess::convert24BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 3;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = pSrc;

	if(mDoConvert == 4) {
		pDst = dstBuffer->pBuffer;
	}

	for (TTInt i = 0; i < nCnt; i++) {
		pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
	}

	if(mDoConvert == 4) {
		dstBuffer->nSize = nCnt * sizeof(TTInt16);
	} else {
		srcBuffer->nSize = nCnt * sizeof(TTInt16);
	}
}

void  TTCAudioProcess::convert32BitFloatTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 4;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = pSrc;

	if(mDoConvert == 4) {
		pDst = dstBuffer->pBuffer;
	}

	for (TTInt i = 0; i < nCnt; i++) {
		TTInt32 intValue = *(float*)pSrc * 32768;
		*pDst++ = intValue & 0x00ff;
		*pDst++ = (intValue & 0xff00) >> 8;
		pSrc += 4;
	}

	if(mDoConvert == 4) {
		dstBuffer->nSize = nCnt * sizeof(TTInt16);
	} else {
		srcBuffer->nSize = nCnt * sizeof(TTInt16);
	}
}

void  TTCAudioProcess::convert32BitIntTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 4;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = pSrc;

	if(mDoConvert == 4) {
		pDst = dstBuffer->pBuffer;
	}

	for (TTInt i = 0; i < nCnt; i++) {
		pSrc += 2;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
	}

	if(mDoConvert == 4) {
		dstBuffer->nSize = nCnt * sizeof(TTInt16);
	} else {
		srcBuffer->nSize = nCnt * sizeof(TTInt16);
	}
}

void  TTCAudioProcess::convert64BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 8;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = pSrc;

	if(mDoConvert == 4) {
		pDst = dstBuffer->pBuffer;
	}

	for (TTInt i = 0; i < nCnt; i++) {
		TTInt32 intValue = *(double*)pSrc * 32768;
		*pDst++ = intValue & 0x00ff;
		*pDst++ = (intValue & 0xff00) >> 8;
		pSrc += 8;
	}

	if(mDoConvert == 4) {
		dstBuffer->nSize = nCnt * sizeof(TTInt16);
	} else {
		srcBuffer->nSize = nCnt * sizeof(TTInt16);
	}
}

TTInt TTCAudioProcess::doSampleBitsConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	if (mAudioFormatIn.SampleBits == 8) {
		convert8BitTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormatIn.SampleBits == 24) {
		convert24BitTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormatIn.SampleBits == 32) {
		if(mAudioFormatIn.nReserved == 0x3)
			convert32BitFloatTo16Bit(srcBuffer, dstBuffer);
		else
			convert32BitIntTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormatIn.SampleBits == 64)
		convert64BitTo16Bit(srcBuffer, dstBuffer);

	dstBuffer->nFlag = srcBuffer->nFlag;
	dstBuffer->llTime = srcBuffer->llTime;
	dstBuffer->nDuration = srcBuffer->nDuration;
	dstBuffer->pData = srcBuffer->pData;
	dstBuffer->lReserve = srcBuffer->lReserve;
	dstBuffer->pReserve = srcBuffer->pReserve;

	return TTKErrNone;
}

TTInt TTCAudioProcess::doSampleRateConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt32 outlen = 0;
	TTInt inlen = 0;
	TTInt outlenMax = 0;
	TTInt16* output = (TTInt16*)dstBuffer->pBuffer;
	TTInt16* input = (TTInt16*)srcBuffer->pBuffer;

	inlen = srcBuffer->nSize/sizeof(short)/mAudioFormatOut.Channels;
	outlen = inlen * mResmpFactor;
	
	outlenMax = dstBuffer->nSize/sizeof(short)/mAudioFormatOut.Channels;
	if(outlen > outlenMax) {
		outlen = outlenMax;
	}

	if(inlen > 0)
		outlen = mResampleObj->resample(inlen, outlen, input, output);

	dstBuffer->nSize = outlen*sizeof(short)*mAudioFormatOut.Channels;
	dstBuffer->nFlag = srcBuffer->nFlag;
	dstBuffer->llTime = srcBuffer->llTime;
	dstBuffer->nDuration = srcBuffer->nDuration;
	dstBuffer->pData = srcBuffer->pData;
	dstBuffer->lReserve = srcBuffer->lReserve;
	dstBuffer->pReserve = srcBuffer->pReserve;
	
	return TTKErrNone;
}

TTInt TTCAudioProcess::doSampleConvert(TTBuffer* dstBuffer)
{
	TTInt DoDownMix = 0;
	TTInt DoSampleBits = 0;
	TTInt DoSampleRateConv = 0;

	if(mAudioFormatIn.SampleBits != 16) {
		DoSampleBits = 1;
	}

	if(mAudioFormatOut.Channels != mAudioFormatIn.Channels) {
		DoDownMix = 1;
	}

	if(mAudioFormatOut.SampleRate != mAudioFormatIn.SampleRate) {
		DoSampleRateConv = 1;
	}

	if(DoSampleBits) {
		doSampleBitsConv(mSrcBuffer, dstBuffer);
	}

	if(DoDownMix) {
		if(DoSampleRateConv)
			doChannelDoMix(mSrcBuffer, mSrcBuffer);
		else
			doChannelDoMix(mSrcBuffer, dstBuffer);
	}

	if(DoSampleRateConv) {
		doSampleRateConv(mSrcBuffer, dstBuffer);
	}

	return TTKErrNone;
}
 


TTInt TTCAudioProcess::setParam(TTInt aID, void* pValue)
{
	TTCAutoLock Lock(&mCritical);
	
	TTInt nErr = TTKErrNone;

	switch(aID)	{
	case TT_PID_AUDIO_FORMAT:
		if(pValue) 	{
			memcpy(&mAudioFormatOut, pValue, sizeof(TTAudioFormat));

			if(mAudioFormatOut.Channels != mAudioFormatIn.Channels)
				mDoConvert |= 1;
			if(mAudioFormatOut.SampleRate != mAudioFormatIn.SampleRate)
				mDoConvert |= 2;			
			if(mAudioFormatOut.SampleBits != mAudioFormatIn.SampleBits)
				mDoConvert |= 4;
		}
		break;
	case TT_PID_AUDIO_SAMPLEREATE:
		if(pValue != NULL && *((TTInt32 *)pValue) > 0) 	{
			mAudioFormatOut.SampleRate = *((TTInt32 *)pValue);
			mSampleRateSet = mAudioFormatOut.SampleRate;
			
			if(mAudioFormatOut.SampleRate != mAudioFormatIn.SampleRate) {
				mDoConvert |= 2;
				SAFE_DELETE(mResampleObj);

				mResampleObj = new aflibConverter(ETTFalse, ETTFalse, ETTTrue);
				mResmpFactor = (double)mAudioFormatOut.SampleRate / mAudioFormatIn.SampleRate;
				mResampleObj->initialize(mResmpFactor, mAudioFormatOut.Channels);
			}
		}
		break;
	case TT_PID_AUDIO_SAMPLEREATE_MAX:
		if(pValue != NULL && *((TTInt32 *)pValue) > 0) 	{
			mSampleRateMAX = *((TTInt32 *)pValue);
			updateParam();
		}
		break;
	case TT_PID_AUDIO_CHANNELS:
		if(pValue != NULL && *((TTInt32 *)pValue) > 0) {
			mAudioFormatOut.Channels = *((TTInt32 *)pValue);
			mChannelSet = mAudioFormatOut.Channels;

			if(mAudioFormatOut.Channels != mAudioFormatIn.Channels)
				mDoConvert |= 1;
		}
		break;
	case TT_PID_AUDIO_DOWAVE:
		if(pValue != NULL) {
			TTCAutoLock Lock(&mWaveList);
			mDoWave = *((TTInt32 *)pValue);
			if(mDoWave)
				allocWaveBuffer();
		}
		break;
	case TT_PID_AUDIO_DOEFFECT:
		if(pValue != NULL) {
			mEffectEnable = *((TTBool *)pValue);
			if(mEffectEnable) {
				allocWaveBuffer();
			}
		}
		break;
	default:
		if(mDecoder) {
			nErr = mDecoder->setParam(aID, pValue);
		}
		break;
	}

	return nErr;
}

TTInt TTCAudioProcess::getParam(TTInt aID, void* pValue)
{
	TTInt nErr = TTKErrNone;

	switch(aID)
	{
	case TT_PID_AUDIO_FORMAT:
		if(pValue) 	{
			memcpy(pValue, &mAudioFormatOut, sizeof(TTAudioFormat));
		}
		break;
	default:
		nErr = mDecoder->getParam(aID, pValue);
		break;
	}

	return nErr;
}

TTInt TTCAudioProcess::onAudioProc (TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	if(!mSetProirity) {
#ifdef __TT_OS_ANDROID__
		nice(-12);
#endif
#ifdef __TT_OS_WINDOWS__
		//SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);
#endif
		mSetProirity = true;
	}

	TTBuffer* DstBuffer = NULL;
	if(mAudioEffectLowDelay != gAudioEffectLowDelay) {
		if(mAudioEffectLowDelay) {
			TTInt nList = 0;
			mCriList.Lock();
			TTInt nStartList = mListUsing;
			TTInt nEndList = mListFull;
			mCriList.UnLock();
			for(nList = nStartList; nList <  nEndList; nList++) {
				mCriList.Lock();
				TTInt nIndex = nList%mAudioCount;
				DstBuffer = mListBuffer[nIndex];
				mCriList.UnLock();
			}
		}
		mAudioEffectLowDelay = gAudioEffectLowDelay;
	}

	mCriList.Lock();
	TTBool bFlushing = mFlushing;
	if(mListFull - mListUsing >= (TTInt)mAudioCount - 1 || mEOS) {
		mCriList.UnLock();
		//if (mStatus == EStatusPlaying) 	{
		//	TTCAutoLock Lock(&mCriEvent);
		//	postAudioProcEvent(50);
		//}
		return 0;
	}

	int nIndex = mListFull%mAudioCount;
	DstBuffer = mListBuffer[nIndex];

	int nFlag = 0;
	if(mSeeking) {
		nFlag |= TT_FLAG_BUFFER_SEEKING;
	}
	DstBuffer->nFlag = nFlag;
	DstBuffer->nSize = mAudioBufferSize;
	DstBuffer->llTime = mCurTime;
	mCriList.UnLock();

	TTInt nDelayTime = 0;
	TTInt nErr = doAudioProcess(DstBuffer);

	if(nErr == TTKErrNone) {
		TTCAutoLock Lock(&mCriList);
		if(mFlushing) {
			if(bFlushing) mListFull++;
		} else { 
			mListFull++;
		}
		if(mSeeking && nFlag) {
			mSeeking = false;
		}
	} else if(nErr == TTKErrFormatChanged) {
		nDelayTime = -1;
		TTCAutoLock Lock(&mCriList);
		mListFull++;
		if(mSeeking && nFlag) {
			mSeeking = false;
		}
	} else if(nErr == TTKErrUnderflow) {
		nDelayTime = -1;
	} else if(nErr == TTKErrNotReady) {
		nDelayTime = 50;
	} else if(nErr == TTKErrInUse) {
		nDelayTime = 1;
	} else if(nErr == TTKErrEof) {
		TTCAutoLock Lock(&mCriList);
		if(mFlushing) {
			if(bFlushing) mListFull++;
		} else { 
			mListFull++;
		}
		mEOS = true;
	}
	//LOGI("TTCAudioProcess::onAudioProc nErr %d, mListFull %d, mListUsing %d, DstBuffer->llTime %lld, DstBuffer->lReserve %d", nErr, mListFull, mListUsing, DstBuffer->llTime, DstBuffer->lReserve);

	if (nErr != TTKErrEof && mStatus == EStatusPlaying)
	{
		TTCAutoLock Lock(&mCriEvent);
		postAudioProcEvent (nDelayTime);
	}

	TTCAutoLock Lock(&mCriList);
	mFlushing = false;

	return TTKErrNone;
}

TTInt TTCAudioProcess::postAudioProcEvent (TTInt  nDelayTime)
{
	if (mProcThread == NULL)
		return TTKErrNotFound;

	TTBaseEventItem * pEvent = mProcThread->getEventByType(EEventAudioProcoess);
	if (pEvent == NULL)
		pEvent = new TTCAudioProctEvent (this, &TTCAudioProcess::onAudioProc, EEventAudioProcoess);
	mProcThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}
