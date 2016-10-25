#include "TTAudioDecode.h"
#include "TTMediainfoDef.h"
#include "TTOsalConfig.h"
#include "TTLog.h"

#define FORMAT_TAG_IEEE_FLOAT				3

CTTAudioDecode::CTTAudioDecode(CTTSrcDemux*	aSrcMux, TTInt nTimeStep)
:mSrcMux(aSrcMux)
,mCurBuffer(NULL)
,mAudioStepTime(nTimeStep)
,mAudioStepSize(0)
,mAudioDecSize(0)
,mAudioFrameTime(0)
,mStatus(EStatusStoped)
,mAudioCodec(0)
{
	mCritical.Create();
	mCriStatus.Create();
	mPluginManager = new CTTAudioPluginManager();

	memset(&mAudioFormat, 0, sizeof(mAudioFormat));
	memset(&mSrcBuffer, 0, sizeof(mSrcBuffer));
	memset(&mWAVFormat, 0, sizeof(TTWAVFormat));
	TTASSERT(mPluginManager != NULL);

#ifdef __DUMP1_PCM__
	DumpFile = fopen("D:\\Test\\Dump1.pcm", "wb+");
#endif
}

CTTAudioDecode::~CTTAudioDecode()
{
	uninitDecode();
	SAFE_DELETE(mPluginManager);
	mCriStatus.Destroy();
	mCritical.Destroy();

#ifdef __DUMP1_PCM__
	if(DumpFile)
		fclose(DumpFile);
#endif
}

TTInt CTTAudioDecode::initDecode(TTAudioInfo* pCurAudioInfo)
{
	TTCAutoLock Lock(&mCritical);
	if(pCurAudioInfo == NULL) 
		return TTKErrArgument;
	if(mPluginManager == NULL) 
		return TTKErrNotFound;

	mCriStatus.Lock();
	mStatus = EStatusStarting;
	mCriStatus.UnLock();

	TTInt32 nErr = TTKErrNone;
	mAudioCodec = pCurAudioInfo->iMediaTypeAudioCode;

	if(mAudioCodec != TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
		nErr = mPluginManager->initPlugin(pCurAudioInfo->iMediaTypeAudioCode, pCurAudioInfo->iDecInfo);
		if(nErr != TTKErrNone) 
			return nErr;
	}

	updateParam(pCurAudioInfo);

	mCriStatus.Lock();
	mStatus = EStatusPrepared;
	mCriStatus.UnLock();

	return nErr;
}

TTInt CTTAudioDecode::uninitDecode()
{
	TTCAutoLock Lock(&mCriStatus);
	mPluginManager->uninitPlugin();
	mCurBuffer = NULL;

	return TTKErrNone;
}

TTInt CTTAudioDecode::start()
{
	TTCAutoLock Lock(&mCriStatus);
	mStatus = EStatusPlaying;
	return TTKErrNone;
}

TTInt CTTAudioDecode::stop()
{
	TTCAutoLock Lock(&mCriStatus);
	mStatus = EStatusStoped;
	return TTKErrNone;
}

TTInt CTTAudioDecode::pause()
{
	TTCAutoLock Lock(&mCriStatus);
	mStatus = EStatusPaused;
	return TTKErrNone;
}

TTInt CTTAudioDecode::resume()
{
	TTCAutoLock Lock(&mCriStatus);
	mStatus = EStatusPlaying;
	return TTKErrNone;
}

TTInt CTTAudioDecode::flush()
{
	TTCAutoLock Lock(&mCritical);

	mPluginManager->resetPlugin();

	mCurBuffer = NULL;
	mAudioFrameTime = 0;

	return TTKErrNone;
}

TTInt CTTAudioDecode::getOutputBuffer(TTBuffer* aDstBuffer)
{
	TTCAutoLock Lock(&mCritical);

	if(mSrcMux == NULL) {
		aDstBuffer->nSize = 0;
		return TTKErrNotFound;
	}

	if(aDstBuffer == NULL || aDstBuffer->pBuffer == NULL) {
		aDstBuffer->nSize = 0;
		return TTKErrArgument;
	}

	mCriStatus.Lock();
	if(mStatus != EStatusPlaying) {
		mCriStatus.UnLock();
		aDstBuffer->nSize = 0;
		return TTKErrInUse;
	}
	mCriStatus.UnLock();

	TTInt64 nTime = -1;
	TTInt32	nSize = aDstBuffer->nSize;
	TTInt32 nFlag = aDstBuffer->nFlag;
	TTPBYTE	pBuffer = aDstBuffer->pBuffer;        
	TTInt32 nErr = TTKErrNotFound;

	if(mAudioCodec == TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
		mSrcBuffer.nFlag = nFlag;
		nErr = mSrcMux->GetMediaSample(EMediaTypeAudio, &mSrcBuffer);
		if(nErr != TTKErrNone)	{
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = 0;
			if(nErr == TTKErrEof) {
				aDstBuffer->nFlag |= TT_FLAG_BUFFER_EOS;
			}
			return nErr;
		}
		nFlag = 0;

		aDstBuffer->llTime = mSrcBuffer.llTime;
		if(aDstBuffer->nSize > mSrcBuffer.nSize) {
			memcpy(aDstBuffer->pBuffer, mSrcBuffer.pBuffer, mSrcBuffer.nSize);
			aDstBuffer->nSize = mSrcBuffer.nSize;
		} else {
			LOGI("not come here, wav format");
		}

		return TTKErrNone;
	}

	mAudioDecSize = 0;
	TTInt32 nNum = 0;

	if(mCurBuffer) {
		nTime = mAudioFrameTime;
		
		nErr = DecBuffer(pBuffer, nSize);

		if(nErr == TTKErrOverflow || mAudioDecSize >= mAudioStepSize || nErr == TTKErrFormatChanged) {
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;

			if(nErr == TTKErrFormatChanged)
				aDstBuffer->nFlag |= TT_FLAG_BUFFER_NEW_FORMAT;

			if(nErr == TTKErrOverflow)
				nErr = TTKErrNone;

			return nErr;
		}

		mCurBuffer = NULL;
	}

	nNum = 0;
	do	{
		nNum++;
		if(nNum > 1000)	{
			flush();
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;
			nErr = TTKErrOverflow;
			break;
		}

		mSrcBuffer.nFlag = nFlag;
		nErr = mSrcMux->GetMediaSample(EMediaTypeAudio, &mSrcBuffer);
		//LOGI("GetMediaSample Audio nErr %d, mSrcBuffer.nSize %d, mSrcBuffer.llTime %lld", nErr, mSrcBuffer.nSize, mSrcBuffer.llTime);
		if(nErr != TTKErrNone)	{
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;
			if(nErr == TTKErrEof) {
				aDstBuffer->nFlag |= TT_FLAG_BUFFER_EOS;
			}
			return nErr;
		}
		nFlag = 0;

		if(mSrcBuffer.nFlag & TT_FLAG_BUFFER_TIMESTAMP_RESET)
		{
			aDstBuffer->nFlag |= TT_FLAG_BUFFER_TIMESTAMP_RESET;
			nTime = mSrcBuffer.llTime;
		}

		if((mSrcBuffer.nFlag & TT_FLAG_BUFFER_NEW_PROGRAM) || (mSrcBuffer.nFlag & TT_FLAG_BUFFER_NEW_FORMAT))	{
			initDecode((TTAudioInfo *)mSrcBuffer.pData);
			start();
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;
			aDstBuffer->nFlag |= TT_FLAG_BUFFER_NEW_FORMAT;
			if(mSrcBuffer.nFlag & TT_FLAG_BUFFER_NEW_PROGRAM)
				aDstBuffer->nFlag |= TT_FLAG_BUFFER_TIMESTAMP_RESET;

			return TTKErrFormatChanged;			
		}

		if(mSrcBuffer.nFlag & TT_FLAG_BUFFER_FLUSH) {
			flush();
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;
			return TTKErrInUse;
		}

		if(nTime == -1)
			nTime = mSrcBuffer.llTime;

		nErr = mPluginManager->setInput(&mSrcBuffer);
		if(nErr != TTKErrNone)
		{
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;
			return nErr;
		}

		mCurBuffer = &mSrcBuffer;
		mAudioFrameTime = mSrcBuffer.llTime;

		nErr = DecBuffer(pBuffer, nSize);

		if(nErr == TTKErrOverflow || mAudioDecSize >= mAudioStepSize || nErr == TTKErrFormatChanged) {
			aDstBuffer->llTime = nTime;
			aDstBuffer->nSize = mAudioDecSize;

			if(nErr == TTKErrFormatChanged)
				aDstBuffer->nFlag |= TT_FLAG_BUFFER_NEW_FORMAT;

			if(nErr == TTKErrOverflow)
				nErr = TTKErrNone;

#ifdef __DUMP1_PCM__
			if(DumpFile)
				fwrite(aDstBuffer->pBuffer, aDstBuffer->nSize, 1, DumpFile);
#endif
			return nErr;
		}

		mCurBuffer = NULL;

	}while(1);

	return TTKErrNone;
}

TTInt CTTAudioDecode::setParam(TTInt aID, void* pValue)
{
	if(aID == TT_PID_COMMON_DATASOURCE) {
		if(pValue)
			mSrcMux = (CTTSrcDemux*)pValue;

		return TTKErrNone;
	}

	TTCAutoLock Lock(&mCritical);

	return mPluginManager->setParam(aID, pValue);
}

TTInt CTTAudioDecode::getParam(TTInt aID, void* pValue)
{
	TTCAutoLock Lock(&mCritical);
	if(mAudioCodec == TTAudioInfo::KTTMediaTypeAudioCodeWAV)
	{
		if(aID == TT_PID_AUDIO_FORMAT) {
			if(pValue)
				memcpy(pValue, &mAudioFormat, sizeof(TTAudioFormat));
			return TTKErrNone;
		} else {
			return TTKErrNotFound;
		}			
	}

	return mPluginManager->getParam(aID, pValue);
}


TTInt CTTAudioDecode::updateParam(TTAudioInfo* pCurAudioInfo)
{
	TTInt32 nErr = TTKErrNone;

	//if(pCurAudioInfo->iMediaTypeAudioCode == TTAudioInfo::KTTMediaTypeAudioCodeAAC) {
	//	TTAACFRAMETYPE nType = TTAAC_ADTS; 
	//	if(pCurAudioInfo->iFourCC == MAKEFOURCC('A','D','T','S')) {
	//		nType = TTAAC_ADTS;
	//		mPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}else if(pCurAudioInfo->iFourCC == MAKEFOURCC('A','D','I','F')) {
	//		nType = TTAAC_ADIF;
	//		mPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}else if(pCurAudioInfo->iFourCC == MAKEFOURCC('R','A','W',' ')) {
	//		nType = TTAAC_RAWDATA;
	//		mPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}
	//} else if(pCurAudioInfo->iMediaTypeAudioCode == TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
	//	if(pCurAudioInfo->iDecInfo)
	//		memcpy(&mWAVFormat, pCurAudioInfo->iDecInfo, sizeof(TTWAVFormat));
	//	
	//	mAudioFormat.Channels = pCurAudioInfo->iChannel;
	//	mAudioFormat.SampleRate = pCurAudioInfo->iSampleRate;
	//	mAudioFormat.SampleBits = mWAVFormat.iBitsPerSample;		
	//	if(mAudioFormat.SampleBits == 0)
	//		mAudioFormat.SampleBits = 16;
	//	if(mWAVFormat.iFmtTag == FORMAT_TAG_IEEE_FLOAT)
	//		mAudioFormat.nReserved = FORMAT_TAG_IEEE_FLOAT;
	//	return nErr;
	//}

	nErr = mPluginManager->getParam(TT_PID_AUDIO_FORMAT, &mAudioFormat);

	if(nErr != TTKErrNone || mAudioFormat.Channels == 0 || mAudioFormat.SampleRate == 0) {
		mAudioFormat.Channels = pCurAudioInfo->iChannel;
		mAudioFormat.SampleRate = pCurAudioInfo->iSampleRate;
		mAudioFormat.SampleBits = 16;

		mPluginManager->setParam(TT_PID_AUDIO_FORMAT, &mAudioFormat);
	}

	mCurBuffer = NULL;
	updateStep();

	return nErr;
}

TTInt CTTAudioDecode::updateStep()
{
	if(mAudioFormat.Channels != 0 && mAudioFormat.Channels < 10 && mAudioFormat.SampleRate != 0 
		&& mAudioFormat.SampleBits != 0) {
		mAudioStepSize = mAudioFormat.Channels*mAudioFormat.SampleRate*mAudioFormat.SampleBits*mAudioStepTime/(1000*8);
	} else	{
		mAudioStepSize = 2*44100*2*mAudioStepTime/1000;
	}

	return TTKErrNone;
}

TTInt CTTAudioDecode::DecBuffer(TTPBYTE pBuffer, TTInt32 nSize)
{
	TTBuffer TmpBuffer;
	TTInt32 nErr = TTKErrNone;

	memset(&TmpBuffer, 0, sizeof(TTBuffer));

	TTAudioFormat AudioFormat;
	memcpy(&AudioFormat, &mAudioFormat, sizeof(TTAudioFormat));
	
	int nNum = 0;
	while(1) {
		nNum++;
		if(nNum > 1000)	{
			flush();
			nErr = TTKErrUnderflow;
			break;
		}

		TmpBuffer.pBuffer = pBuffer + mAudioDecSize;
		TmpBuffer.nSize = nSize - mAudioDecSize;

		nErr = mPluginManager->process(&TmpBuffer, &AudioFormat);

		if(nErr == TTKErrNone)	{
			if(AudioFormat.Channels != mAudioFormat.Channels || AudioFormat.SampleBits != mAudioFormat.SampleBits || AudioFormat.SampleRate != mAudioFormat.SampleRate)	{
				mAudioFormat.Channels = AudioFormat.Channels;
				mAudioFormat.SampleBits = AudioFormat.SampleBits;
				mAudioFormat.SampleRate = AudioFormat.SampleRate;
				updateStep();
				nErr = TTKErrFormatChanged;
				break;
			}

			mAudioDecSize += TmpBuffer.nSize;
			if(mAudioFormat.Channels != 0 && mAudioFormat.SampleBits !=0 && mAudioFormat.SampleRate !=0)
				mAudioFrameTime += TmpBuffer.nSize*1000*8/(mAudioFormat.Channels*mAudioFormat.SampleBits*mAudioFormat.SampleRate);

			if(mAudioDecSize >= mAudioStepSize)
				break;
		} else if(nErr == TTKErrOverflow || nErr == TTKErrUnderflow) {
			break;
		}
	}

	return nErr;
}