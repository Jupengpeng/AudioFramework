// INCLUDES
#include "TTOsalConfig.h"
#include "TTBaseAudioSink.h"
#include "TTSysTime.h"
#include "TTLog.h"

TTCBaseAudioSink::TTCBaseAudioSink(CTTSrcDemux* SrcMux, TTInt nCount)
: mSrcMux(SrcMux)
, mProcessCount(nCount)
, mCurPos(0)
, mSinkBuf(NULL)
, mSinkBufLen(0)
, mEOS(false)
, mSeeking(false)
, mPlayStatus(EStatusStoped)
, mRenderNum(0)
, mBufferStatus(ETTBufferingInValid)
, mFrameDuration(0)
, mRenderPCM(0)
, mObserver(NULL)
, mAudioSystemTime(0)
, mAudioBufferTime(0)
, mAudioSysStartTime(0)
, mAudioBufStartTime(0)
, mAudioOffSetTime(0)
, mAudioAdjustTime(200)
, mRenderThread(NULL)
, mAudioProcess(NULL)
,mFadeCount(MAX_FADE_COUNT)
,mFadeStatus(ETTAudioFadeNone)
{
	mCritical.Create();
	mCritStatus.Create();
	mCritTime.Create();
	mSemaphore.Create();

	memset(&mAudioFormat, 0, sizeof(TTAudioFormat));
	memset(&mSinkBuffer, 0, sizeof(mSinkBuffer));

	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;

	mAudioProcess = new TTCAudioProcess(mSrcMux, mProcessCount);
}

TTCBaseAudioSink::~TTCBaseAudioSink()
{
	close();
	SAFE_FREE(mSinkBuf);
	SAFE_DELETE(mAudioProcess);
	SAFE_DELETE(mRenderThread);
	mCritTime.Destroy();
	mCritStatus.Destroy();
	mCritical.Destroy();
	mSemaphore.Destroy();
}

TTInt TTCBaseAudioSink::open(TTAudioInfo* pCurAudioInfo)
{
	TTCAutoLock Lock(&mCritical);
	
	TTInt nErr = mAudioProcess->initProc(pCurAudioInfo);
	if(nErr != TTKErrNone)
		return nErr;

	nErr = mAudioProcess->getParam(TT_PID_AUDIO_FORMAT, &mAudioFormat);
	if(nErr != TTKErrNone)
		return nErr;

	if(mRenderThread == NULL)
		 mRenderThread =  new TTEventThread("TTAudio Render");

	 setPlayStatus(EStatusStarting);

	 nErr = newAudioTrack();

	 if(nErr == TTKErrNone)
		setPlayStatus(EStatusPrepared);

 	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;

	mCritTime.Lock();
	mRenderNum = 0;
	mRenderPCM = 0;
	mAudioBufStartTime = 0;
	mAudioSysStartTime = 0;
	mAudioBufferTime = 0;
	mAudioSystemTime = 0;
	mCritTime.UnLock();	

	return nErr;
}

TTInt TTCBaseAudioSink::syncPosition(TTUint64 aPosition, TTInt aOption)
{
	mCritical.Lock();
	if(mRenderThread)
		mRenderThread->cancelAllEvent();

	if(mAudioProcess)
		mAudioProcess->syncPosition(aPosition, aOption);
	mCritical.UnLock();

	mCritTime.Lock();
	mSeeking = true;
	mRenderNum = 0;
	mRenderPCM = 0;
	mAudioBufStartTime = aPosition;
	mAudioSysStartTime = 0;
	mAudioBufferTime = aPosition;
	mAudioSystemTime = 0;
	mEOS = false;
	mCurPos = aPosition;
	mCritTime.UnLock();

	return TTKErrNone;
}

TTInt TTCBaseAudioSink::start(TTBool aPause, TTBool aWait)
{
	TTCAutoLock Lock(&mCritical);

	if (getPlayStatus() == EStatusPlaying)
		return TTKErrNone;

	if (getPlayStatus() == EStatusStoped)
		return TTKErrGeneral;

	//mCritTime.Lock();
	//mRenderNum = 0;
	//mRenderPCM = 0;
	//mAudioBufStartTime = 0;
	//mAudioSysStartTime = 0;
	//mAudioBufferTime = 0;
	//mAudioSystemTime = 0;
	//mCritTime.UnLock();	

	if(mAudioProcess)
		mAudioProcess->start();

	if(mRenderThread)
		mRenderThread->start();
	
	if(!aPause && getBufferStatus() != ETTBufferingStart) {
		setPlayStatus(EStatusPlaying);
		if(!aWait) postAudioRenderEvent(-1);
	} else {
		setPlayStatus(EStatusPaused);
		if(mAudioProcess)
			mAudioProcess->pause();
	}

	mSemaphore.Reset();
	mEOS = false;
	
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::pause(TTBool aFadeOut)
{
	TTCAutoLock Lock(&mCritical);

	if (getPlayStatus() == EStatusPlaying)	{
		if(aFadeOut){
			if(ETTAudioFadeNone == getFadeStatus())
			{
				mCritStatus.Lock();
				mFadeCount = MAX_FADE_COUNT;
				mCritStatus.UnLock();
			}
			setFadeStatus(ETTAudioFadeOut);
		}

		setPlayStatus(EStatusPaused);

		if(mAudioProcess)
			mAudioProcess->pause();
	}

	return TTKErrNone;
}

TTInt TTCBaseAudioSink::resume(TTBool aWait,TTBool aFadeIn)
{
	TTCAutoLock Lock(&mCritical);

	if (getPlayStatus() == EStatusPaused) {
		if(aFadeIn){
			if(ETTAudioFadeNone == getFadeStatus())
			{
				mCritStatus.Lock();
				mFadeCount = 0;
				mCritStatus.UnLock();
			}
			setFadeStatus(ETTAudioFadeIn);
		}

		mCritTime.Lock();
		mRenderNum = 0;
		mRenderPCM = 0;
		mAudioSysStartTime = 0;
		mAudioSystemTime = 0;
		mCritTime.UnLock();	

		if(mAudioProcess)
			mAudioProcess->resume();

		setPlayStatus(EStatusPlaying);
		
		if(!aWait)
			postAudioRenderEvent(-1);
	}
    return TTKErrNone;
}

void TTCBaseAudioSink::fadeOutInHandle()
{
	if (mSinkBuffer.pBuffer == NULL || mSinkBuffer.nSize == 0)
		return;

	TTInt SampleCnt = mSinkBuffer.nSize /sizeof(short)/mAudioFormat.Channels;
	TTInt totalSampleCnt = MAX_FADE_COUNT * SampleCnt;
	TTInt ACSampleCnt;
	TTInt aFadeCnt;
	TTInt16* psrc;

	if(getFadeStatus() == ETTAudioFadeOut){
		mCritStatus.Lock();
		aFadeCnt = mFadeCount;
		mCritStatus.UnLock();
		if (aFadeCnt > 0){
			psrc =  (TTInt16*)(mSinkBuffer.pBuffer);
			ACSampleCnt = aFadeCnt * SampleCnt;
			for(TTInt n = 0;n<SampleCnt;n++){
				for(TTInt i =0; i<mAudioFormat.Channels; i++)
				{
					*psrc = (((float)ACSampleCnt - n) / totalSampleCnt) * (*psrc);
					*psrc++;
				}
			}
			mCritStatus.Lock();
			mFadeCount--;
			mCritStatus.UnLock();
		}

		mCritStatus.Lock();
		if(mFadeCount == 0 && mFadeStatus == ETTAudioFadeOut){
			mFadeStatus = ETTAudioFadeNone;
		}
		mCritStatus.UnLock();

	}
	else if(getFadeStatus() == ETTAudioFadeIn){
		mCritStatus.Lock();
		aFadeCnt = mFadeCount;
		mCritStatus.UnLock();
		if (aFadeCnt < MAX_FADE_COUNT){
			psrc =  (TTInt16*)(mSinkBuffer.pBuffer);
			ACSampleCnt = aFadeCnt * SampleCnt;
			for(TTInt n = 1;n <= SampleCnt;n++){
				for(TTInt i =0; i<mAudioFormat.Channels; i++)
				{
					*psrc = (((float)ACSampleCnt + n) / totalSampleCnt) * (*psrc);
					*psrc++;
				}
			}
			mCritStatus.Lock();
			mFadeCount++;
			mCritStatus.UnLock();
		}
		
		mCritStatus.Lock();
		if(mFadeCount == MAX_FADE_COUNT && mFadeStatus == ETTAudioFadeIn){
			mFadeStatus = ETTAudioFadeNone;
		}
		mCritStatus.UnLock();
	}
}


TTInt TTCBaseAudioSink::stop()
{
	TTCAutoLock Lock(&mCritical);
	setPlayStatus(EStatusStoped);
	if(mRenderThread) {
		if(mRenderThread->getStatus() == TT_THREAD_STATUS_RUN) {
			postAudioRenderEvent(-1);
			mSemaphore.Wait();
		}
		mRenderThread->stop();
	}

	if(mAudioProcess)
		mAudioProcess->stop();

	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;

	mBufferStatus = ETTBufferingInValid;

	mCritTime.Lock();
	mRenderNum = 0;
	mRenderPCM = 0;
	mAudioBufStartTime = 0;
	mAudioSysStartTime = 0;
	mAudioBufferTime = 0;
	mAudioSystemTime = 0;
	mEOS = false;
	mCritTime.UnLock();

	mCritStatus.Lock();
	mFadeStatus = ETTAudioFadeNone;
	mFadeCount = MAX_FADE_COUNT;
	mCritStatus.UnLock();

	return TTKErrNone;
}


TTInt TTCBaseAudioSink::close()
{
	if (getPlayStatus() != EStatusStoped)
		stop();

	TTCAutoLock Lock(&mCritical);
	if(mAudioProcess)
		mAudioProcess->uninitProc();

	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;

	SAFE_DELETE(mRenderThread);
	TTInt nErr = closeAudioTrack();
	return nErr;
}

TTInt TTCBaseAudioSink::freeAudioTrack()
{
	closeAudioTrack();
	mSemaphore.Signal();
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::getCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels)
{
	TTInt64 aPlayingTime = getPlayTime();

	if(mAudioProcess)
		return mAudioProcess->getCurWave(aPlayingTime, aSamples, aWave, aChannels);

	return TTKErrNotFound;
}

TTInt TTCBaseAudioSink::setVolume(TTInt aLVolume, TTInt aRVolume)
{
	TTCAutoLock Lock(&mCritical); 
	mLVolume = aLVolume;
	mRVolume = aRVolume;
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::volume()
{
	TTCAutoLock Lock(&mCritical);
	return (mLVolume+mRVolume)/2;
}

TTInt TTCBaseAudioSink::flush()
{
	TTCAutoLock Lock(&mCritical);
	TTInt nErr = TTKErrNone;
	if(mAudioProcess)
		nErr = mAudioProcess->flush();

	if(mRenderThread)
		mRenderThread->cancelAllEvent();

	return nErr;
}

void TTCBaseAudioSink::setPlayRange(TTUint aStartTime, TTUint aEndTime)
{
	TTCAutoLock Lock(&mCritical);
	mPlayRange.bEnable = true;
	mPlayRange.nStartTime = aStartTime;
	mPlayRange.nStopTime = aEndTime;	
}

TTInt TTCBaseAudioSink::newAudioTrack()
{
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::closeAudioTrack()
{
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::render()
{
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::startOne(TTInt nDelaytime)
{
	if (getPlayStatus() == EStatusPlaying && mPlayRange.bEnable)	{
		if(mCurPos >= mPlayRange.nStopTime) {
			setEOS();
			return TTKErrNone;
		}
	}

	if(getPlayStatus() == EStatusPaused && mFadeStatus == ETTAudioFadeOut)
	{
		mCritStatus.Lock();
		int count = mFadeCount;
		mCritStatus.UnLock();
		if(count > 0){
			postAudioRenderEvent(nDelaytime);
		}
		return TTKErrNone;
	}

	if (getPlayStatus() == EStatusPlaying && !isEOS())	{
		postAudioRenderEvent(nDelaytime);
	}
	return TTKErrNone;
}

TTInt TTCBaseAudioSink::onRenderAudio (TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	TTInt nErr = TTKErrNone;

	if(getPlayStatus() == EStatusStoped) {
		freeAudioTrack();
		return TTKErrNone;
	}

	if(isEOS()) {
		if(mObserver) {
			mObserver->pObserver(mObserver->pUserData, ENotifyComplete, TTKErrNone, 0, NULL);
		}
		return TTKErrNone;
	}

	TTBufferingStatus nBufferStatus = getBufferStatus();
	if(nBufferStatus == ETTBufferingStart) {
		startOne(20);		
		return nErr;
	}


	nErr = render();

	if(isEOS()) {
		if(mObserver) {
			mObserver->pObserver(mObserver->pUserData, ENotifyComplete, TTKErrNone, 0, NULL);
		}
		return TTKErrNone;
	}

	return nErr;
}

TTInt TTCBaseAudioSink::postAudioRenderEvent (TTInt  nDelayTime)
{
	if (mRenderThread == NULL)
		return TTKErrNotFound;

	if( mRenderThread->getFullEventNum(EEventAudioRender) > 0)
		return TTKErrNone;

	TTBaseEventItem * pEvent = mRenderThread->getEventByType(EEventAudioRender);
	if (pEvent == NULL)
		pEvent = new TTCAudioRenderEvent (this, &TTCBaseAudioSink::onRenderAudio, EEventAudioRender);
	mRenderThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

void TTCBaseAudioSink::setPlayStatus(TTPlayStatus aStatus)
{
	LOGI("TTCBaseAudioSink::SetPlayStatus %d", aStatus);
	TTCAutoLock Lock(&mCritStatus);
	mPlayStatus = aStatus;
}

TTPlayStatus TTCBaseAudioSink::getPlayStatus()
{
	TTCAutoLock Lock(&mCritStatus);
	TTPlayStatus status = mPlayStatus;
	return status;
}

TTInt TTCBaseAudioSink::setParam(TTInt aID, void* pValue)
{
	TTCAutoLock Lock(&mCritical);
	TTInt nErr = TTKErrNotFound;
	if(mAudioProcess)
		nErr = mAudioProcess->setParam(aID, pValue);
	return nErr;
}

TTInt TTCBaseAudioSink::getParam(TTInt aID, void* pValue)
{
	TTCAutoLock Lock(&mCritical);
	TTInt nErr = TTKErrNotFound;

	if(aID == TT_PID_AUDIO_OFFSET) {
		*(TTInt *)pValue = (TTInt)mAudioOffSetTime;
		return TTKErrNone;
	}

	if(mAudioProcess)
		nErr = mAudioProcess->getParam(aID, pValue);
	return nErr;
}

TTInt TTCBaseAudioSink::setBufferStatus(TTBufferingStatus aBufferStatus)
{
	TTCAutoLock Lock(&mCritStatus);
	mBufferStatus = aBufferStatus;
	return TTKErrNone;
}

TTBufferingStatus TTCBaseAudioSink::getBufferStatus()
{
	TTCAutoLock Lock(&mCritStatus);
	TTBufferingStatus nBufferStatus = mBufferStatus;
	return nBufferStatus;
}

TTBool TTCBaseAudioSink::isEOS()
{
	TTBool bEOS;
	mCritTime.Lock();
	bEOS = mEOS;
	mCritTime.UnLock();
	return bEOS;
}

TTInt64 TTCBaseAudioSink::getPlayTime()
{
	TTPlayStatus PlayStatus = getPlayStatus();
	TTBufferingStatus nBufferStatus = getBufferStatus();

	TTCAutoLock Lock(&mCritTime);
	TTInt64 nPosition = mAudioBufStartTime;
	if(mSeeking) {
		return nPosition;
	}

	if(PlayStatus == EStatusStarting || PlayStatus == EStatusStoped || PlayStatus == EStatusPrepared) {
		return 0;
	} else if(PlayStatus == EStatusPaused || nBufferStatus == ETTBufferingStart) {
		nPosition = mAudioBufStartTime + mFrameDuration;
		return nPosition;
	}

	if(mRenderNum == 0)
		return mAudioBufStartTime;

	if(mAudioSystemTime == 0 && !mEOS)
		return mAudioBufStartTime;

	if(mEOS) {
		nPosition = mAudioBufStartTime + GetTimeOfDay() - mAudioSysStartTime;
		
		return nPosition;
	}

	TTInt64 nNowTime = GetTimeOfDay();	
	//nPosition = mAudioBufStartTime + nNowTime - mAudioSysStartTime - mAudioOffSetTime;
	//

	if(abs((TTInt)(mAudioBufferTime + nNowTime - mAudioSystemTime - (mAudioBufStartTime + nNowTime - mAudioSysStartTime))) > mAudioAdjustTime) {
		mAudioSystemTime = 0;
		mAudioBufferTime = 0;

		nPosition = mAudioBufStartTime + nNowTime - mAudioSysStartTime - mAudioOffSetTime;
	} else {
		nPosition = mAudioBufferTime + nNowTime - mAudioSystemTime - mAudioOffSetTime;
	}

	if(nPosition <= 0) nPosition = 0;

	return nPosition;
}

void TTCBaseAudioSink::setObserver(TTObserver*	aObserver)
{
	TTCAutoLock Lock(&mCritical);
	mObserver = aObserver;
}

void TTCBaseAudioSink::setFadeStatus(TTAudioFadeStatus aStatus)
{
	mCritStatus.Lock();
	mFadeStatus = aStatus;
	mCritStatus.UnLock();
}

TTAudioFadeStatus TTCBaseAudioSink::getFadeStatus()
{
	TTAudioFadeStatus aStatus;
	mCritStatus.Lock();
	aStatus = mFadeStatus;
	mCritStatus.UnLock();
	return aStatus;
}

void TTCBaseAudioSink::setEOS()
{
	mCritTime.Lock();
	mEOS = true;
	if(mSeeking) {
		mSeeking = false;
		mCritTime.UnLock();
		if(mObserver) {
			mObserver->pObserver(mObserver->pUserData, ENotifySeekComplete, TTKErrNone, 0, NULL);
		}		
	} else {
		mCritTime.UnLock();
	}
}

void TTCBaseAudioSink::audioFormatChanged()
{
	TTAudioFormat AudioFormat;
	
	memcpy(&AudioFormat, &mAudioFormat, sizeof(AudioFormat));
	
	mAudioProcess->getParam(TT_PID_AUDIO_FORMAT, &AudioFormat);

	if(AudioFormat.Channels != mAudioFormat.Channels || AudioFormat.SampleBits != mAudioFormat.SampleBits
		|| AudioFormat.SampleRate != mAudioFormat.SampleRate) {
		mCritTime.Lock();
		mRenderNum = 0;
		mRenderPCM = 0;
		mCritTime.UnLock();	

		memcpy(&mAudioFormat, &AudioFormat, sizeof(AudioFormat));

		if(mObserver) {
			mObserver->pObserver(mObserver->pUserData, ENotifyAudioFormatChanged, AudioFormat.Channels, AudioFormat.SampleRate, NULL);
		}

		newAudioTrack();
	}
}

#ifdef __TT_OS_IOS__

void TTCBaseAudioSink::SetBalanceChannel(float aVolume)
{
    
}
#endif