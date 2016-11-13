// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include "TTMediaPlayer.h"
#include "TTLog.h"
#include "TTFFT.h"
#include "TTSysTime.h"
#include "TTHttpClient.h"
#include "PureDecodeEntity.h"


#ifdef __TT_OS_WINDOWS__
#include "Wins/TTWinAudioSink.h"
#endif

#ifdef __TT_OS_ANDROID__
#include "unistd.h"
#include "Android/TTAndroidAudioSink.h"
//#include "Android/TTAndroidVideoSink.h"
#endif

#ifdef __TT_OS_IOS__
#include "TTAudioSink.h"
//#include "TTIosVideoSink.h"
#endif

#ifndef __TT_OS_WINDOWS__
#include "TTHttpClient.h"
#include <arpa/inet.h>
#endif


#ifdef __TT_OS_IOS__
extern int gIosStop;
TTInt gIos8Above;
#endif

TTInt gMaxOutPutSamplerate = 48000;

extern void gSetCacheFilePath(const TTChar* aCacheFilePath);

CTTMediaPlayer::CTTMediaPlayer(ITTMediaPlayerObserver* aPlayerObserver, const TTChar* aPluginPath)
:mUrl(NULL)
,mPlayStatus(EStatusStoped)
,mSeekOption(0)
,mException(0)
,mFakePauseFlag(false)
,mFakeStopFlag(false)
,mSeeking(false)
,mSeekable(false)
,mAudioSink(NULL)
,mSrcDemux(NULL)	
,mAudioStreamId(EMediaStreamIdNone)
,mVideoStreamId(EMediaStreamIdNone)
,mHandleThread(NULL)
,mPreDemux(NULL)
,mPreUrl(NULL)
,mPreSrcFlag(NULL)
,mSCDemux(NULL)
,mSCDoing(ESwitchNone)
,mSCUrl(NULL)
,mSCSrcFlag(0)
,mSCSeekTime(0)
,mPlayerObserver(aPlayerObserver)
,mSrcFlag(0)
,mDecoderType(EDecoderDefault)
,mView(NULL)
,mPrePlayPos(0)
#ifdef __TT_OS_ANDROID__
,mpAudiotrackJClass(NULL)
,mpVideotrackJClass(NULL)
#endif
{
	mCritical.Create();
	mCriEvent.Create();
	mCriStatus.Create();
	mCriVideo.Create();
	mCriSCSrc.Create();

	if (aPluginPath == NULL || strlen(aPluginPath) > FILE_PATH_MAX_LENGTH) 	{
		LOGE("CTTMediaPlayer::PluginPath error");
		mPluginPath[0] = '\0';
	} else	{
		strcpy(mPluginPath, aPluginPath);

		CTTAudioPluginManager::setPluginPath(mPluginPath);
		//CTTVideoPluginManager::setPluginPath(mPluginPath);
		//TTCBaseVideoSink::setPluginPath(mPluginPath);
	//	CTTAudioEffectManager::SetPluginPath(mPluginPath);
	}

	mSrcObserver.pObserver = ttSrcCallBack;
	mSrcObserver.pUserData = this;

	mSCSrcObserver.pObserver = ttSCSrcCallBack;
	mSCSrcObserver.pUserData = this;

	mPreSrcObserver.pObserver = ttPreSrcCallBack;
	mPreSrcObserver.pUserData = this;

	mAudioObserver.pObserver = ttAudioCallBack;
	mAudioObserver.pUserData = this;

	mVideoObserver.pObserver = ttVideoCallBack;
	mVideoObserver.pUserData = this;

	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;

	mMsgThread = new TTEventThread("TTMessage Thread");

	mHandleThread = new TTEventThread("TTHandle Thread");

	mMsgThread->start();
	mHandleThread->start();

	mSrcDemux = new CTTSrcDemux(&mSrcObserver);
	mSCDemux = new CTTSrcDemux(&mSCSrcObserver);
	mPreDemux = new CTTSrcDemux(&mPreSrcObserver);

#ifdef __TT_OS_IOS__
    CTTAudioSink::setIOSVersion();
#endif
}

CTTMediaPlayer::~CTTMediaPlayer()
{
	doStop(false);
	//TTInt64 nStart = GetTimeOfDay();
	mCritical.Lock();
	SAFE_DELETE(mAudioSink);
	SAFE_DELETE(mSrcDemux);
	SAFE_DELETE(mSCDemux);
	SAFE_DELETE(mPreDemux);	
	mCritical.UnLock();
	SAFE_DELETE(mMsgThread);
	SAFE_DELETE(mHandleThread);
	mCriStatus.Lock();
	SAFE_FREE(mUrl);
	SAFE_FREE(mSCUrl);
	SAFE_FREE(mPreUrl);
	mCriStatus.UnLock();

	CTTHttpClient::ReleaseDNSCache();
	TTNetWorkConfig::release();
	mCriStatus.Destroy();
	mCriEvent.Destroy();
	mCritical.Destroy();
	mCriVideo.Destroy();
	mCriSCSrc.Destroy();

	CTTPureDecodeEntity::Release();
	//TTInt64 nEnd = GetTimeOfDay();
	//LOGI("CTTMediaPlayer::~CTTMediaPlayer() use Time %lld", nEnd - nStart);
}

const TTChar* CTTMediaPlayer::Url()
{
	return mUrl;
}

TTInt CTTMediaPlayer::SetDataSourceAsync(const TTChar* aUrl, TTInt nFlag)
{
	if(aUrl == NULL)
		return TTKErrArgument;

	//LOGI("++++SetDataSourceAsync: Time %lld", GetTimeOfDay());
  	if(nFlag&ESourceChanged) {
		if(GetPlayStatus() == EStatusStarting || GetPlayStatus() == EStatusPrepared || GetPlayStatus() == EStatusStoped) {
			return TTKErrGeneral;
		}

		mCriStatus.Lock();
		SAFE_FREE(mSCUrl);
		mSCUrl = (TTChar*) malloc((strlen(aUrl) + 1) * sizeof(TTChar));
		strcpy(mSCUrl, aUrl);
		mSCSrcFlag = nFlag;
		mSCDoing = ESwitchInit;
		mCriStatus.UnLock();

		TTCAutoLock LockEvent(&mCriEvent);
		if(mHandleThread) {
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToOpen);
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
		}
		postSCMsgEvent(0, ENotifySCMediaStartToOpen, 0, 0, NULL);
		return TTKErrNone;
	} else if(nFlag&ESourcePreOpen) {
		mCriStatus.Lock();
		SAFE_FREE(mPreUrl);
		mPreUrl = (TTChar*) malloc((strlen(aUrl) + 1) * sizeof(TTChar));
		strcpy(mPreUrl, aUrl);
		mPreSrcFlag = nFlag;
		mCriStatus.UnLock();

		TTCAutoLock LockEvent(&mCriEvent);
		if(mHandleThread) {
			mHandleThread->cancelEventByMsg(ENotifyMediaPreOpen);
		}
		postPreSrcEvent(0, ENotifyMediaPreOpen, 0, 0, NULL);
		return TTKErrNone;
	} else if(nFlag&ESourceOpenLoaded) {
		mCriStatus.Lock();
		if(mPreUrl != NULL && strcmp(mPreUrl,aUrl) == 0 ) {
			mPlayRange.bEnable = false;
			mPlayRange.nStartTime = 0;
			mPlayRange.nStopTime = 0;
			mCriStatus.UnLock();
			if(GetPlayStatus() != EStatusStoped) {
				Stop();
			}

			TTCAutoLock LockEvent(&mCriEvent);
			postPreSrcEvent(0, ENotifyMediaOpenLoaded, 0, 0, NULL);
			return TTKErrNone;			
		}
		mCriStatus.UnLock();
	}

	mCriStatus.Lock();
	SAFE_FREE(mUrl);
    mUrl = (TTChar*) malloc((strlen(aUrl) + 1) * sizeof(TTChar));
    strcpy(mUrl, aUrl);
	mSrcFlag = nFlag;
	mFakeStopFlag = false;
	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStopTime = 0;
	if(mPreUrl) {
		TTCAutoLock LockEvent(&mCriEvent);
		postPreSrcEvent(0, ENotifyMediaPreRelease, 0, 0, NULL);
	}
	mCriStatus.UnLock();

	if(GetPlayStatus() != EStatusStoped) {
		Stop();
	}

	SetPlayStatus(EStatusStarting);
	setSeekStatus(false);

	TTCAutoLock LockEvent(&mCriEvent);
	postSetDataSourceEvent(0);

	//LOGI("----SetDataSourceAsync: Time %lld", GetTimeOfDay());

	return TTKErrNone;
}

TTInt CTTMediaPlayer::SetDataSourceSync(const TTChar* aUrl, TTInt nFlag)
{
	if(aUrl == NULL)
		return TTKErrArgument;

	mCriStatus.Lock();
	SAFE_FREE(mUrl);
    mUrl = (TTChar*) malloc((strlen(aUrl) + 1) * sizeof(TTChar));
    strcpy(mUrl, aUrl);
	mSrcFlag = nFlag;
	mFakeStopFlag = false;
	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStopTime = 0;
	mCriStatus.UnLock();

	if(GetPlayStatus() != EStatusStoped) {
		doStop(false);
	}

	SetPlayStatus(EStatusStarting);
	setSeekStatus(false);

	return SetDataSource();
}

TTInt CTTMediaPlayer::SetDataSource()
{
	//LOGI("++++SetDataSource: Time %lld", GetTimeOfDay());
	//LOGI("SetDataSource: %s", mUrl);   
	mCriEvent.Lock();
	postMsgEvent(0, ENotifyMediaStartToOpen, 0, 0, NULL);
	mCriEvent.UnLock();

	SetPlayStatus(EStatusStarting);
	
	mCriStatus.Lock();
	TTInt nFlag = mSrcFlag;
	TTChar*	nUrl = (TTChar*) malloc((strlen(mUrl) + 1) * sizeof(TTChar));
	strcpy(nUrl, mUrl);
	mCriStatus.UnLock();

	mCritical.Lock();
	TTInt nErr = mSrcDemux->AddDataSource(nUrl, nFlag);

	mCriStatus.Lock();
	TTBool bStop = mFakeStopFlag;
	if(mFakeStopFlag)
		mFakeStopFlag = false;
	mCriStatus.UnLock();

	if(nErr != TTKErrNone || bStop)	{
		TTInt nStatusCode = 0;
		mSrcDemux->GetParam(TT_PID_COMMON_STATUSCODE, &nStatusCode);
		TTInt nHostIP = 0;
		char *pParam3 = NULL;
		mSrcDemux->GetParam(TT_PID_COMMON_HOSTIP, &nHostIP);
		if(nHostIP) {
			pParam3 = inet_ntoa(*(struct in_addr*)&nHostIP);
		}
		mSrcDemux->RemoveDataSource();
		mCritical.UnLock();
		
		SAFE_FREE(nUrl);

		SetPlayStatus(EStatusStoped);
		TTCAutoLock Lock(&mCriEvent);
		TTInt ErrCode = 0;
		if(!bStop) {
			postMsgEvent(-1, ENotifyException, nErr, nStatusCode, pParam3);
			ErrCode = nErr;
		}
		postMsgEvent(-1, ENotifyClose, ErrCode, 0, NULL);
		return nErr;
	}

	InitSink();
	mCritical.UnLock();

	SAFE_FREE(nUrl);
	mCriStatus.Lock();
	bStop = mFakeStopFlag;
	mFakeStopFlag = false;
	mCriStatus.UnLock();

	TTInt nAVFlag = 0;
	mCritical.Lock();
	if((mAudioSink == 0 ) || bStop) {
		mSrcDemux->RemoveDataSource();
		mCritical.UnLock();

		SAFE_FREE(nUrl);
		SetPlayStatus(EStatusStoped);
		TTCAutoLock LockEvent(&mCriEvent);
		if(bStop) {
			postMsgEvent(-1, ENotifyClose, 0, 0, NULL);
		} else {
			postMsgEvent(-1, ENotifyException, TTKErrFileNotSupport, 0, NULL);
			postMsgEvent(-1, ENotifyClose, TTKErrFileNotSupport, 0, NULL);
		}
		return nErr;
	}

	if(mAudioSink) {
		nAVFlag |= 1;
	}

	mCritical.UnLock();

	SetPlayStatus(EStatusPrepared);

	TTCAutoLock eventLock(&mCriEvent);
	postMsgEvent(1, ENotifyPrepare, TTKErrNone, nAVFlag, NULL);

	//LOGI("----SetDataSource: Time %lld", GetTimeOfDay());

	return TTKErrNone;
}

void CTTMediaPlayer::InitSink()
{
	TTCAutoLock Lock(&mCritical);


	TTInt doAudioCount = 10;

	mAudioStreamId = EMediaStreamIdNone;
	mVideoStreamId = EMediaStreamIdNone;

	TTInt nErr = TTKErrNone;

	const TTMediaInfo& tMeidaInfo = mSrcDemux->GetMediaInfo();

	const RTTPointerArray<TTAudioInfo>& rMediaAudioArrary = tMeidaInfo.iAudioInfoArray;

	if(tMeidaInfo.iVideoInfo)
		doAudioCount = 4;

	if(rMediaAudioArrary.Count() > 0) {
		if(rMediaAudioArrary[0])
			mAudioStreamId = rMediaAudioArrary[0]->iStreamId;

		mSrcDemux->SelectStream(EMediaTypeAudio, mAudioStreamId);
	
		if(mAudioSink == NULL) {
#ifdef __TT_OS_ANDROID__
			mAudioSink = new CTTAndroidAudioSink(mSrcDemux, doAudioCount);
			((CTTAndroidAudioSink *)mAudioSink)->setJniInfo(mpAudiotrackJClass);
#endif //_LINUX_ANDROID
#ifdef __TT_OS_WINDOWS__
			mAudioSink = new CTTWinAudioSink(mSrcDemux, doAudioCount);
#endif // _WIN32
#ifdef __TT_OS_IOS__
            mAudioSink = new CTTAudioSink(mSrcDemux, doAudioCount);
#endif // _IOS
		}
		mAudioSink->setObserver(&mAudioObserver);
		mAudioSink->setParam(TT_PID_COMMON_DATASOURCE, mSrcDemux);
		nErr = mAudioSink->open(rMediaAudioArrary[0]);
		if(nErr != TTKErrNone) {
			SAFE_DELETE(mAudioSink);
		}
	} else {
		SAFE_DELETE(mAudioSink);
	}
	


}

void CTTMediaPlayer::Pause(TTBool aFadeOut)
{
	TTInt nErr = TTKErrNone;
	TTPlayStatus tStatus = GetPlayStatus();	
	if(tStatus == EStatusStarting || tStatus == EStatusPrepared) {
		mCriStatus.Lock();
		TTBool bStop = mFakeStopFlag;
		if(!bStop) 
			mFakePauseFlag = true;
		mCriStatus.UnLock();

		if(!bStop) {
			TTCAutoLock eventLock(&mCriEvent);
			postMsgEvent(1, ENotifyPause, nErr, 0, NULL);
		}
		return;	
	}else if (tStatus != EStatusPlaying) {
		return;
	}

	mCriStatus.Lock();
	TTBool bStop = mFakeStopFlag;
	mCriStatus.UnLock();

	if(bStop)
		return;

	mCritical.Lock();


	if (mAudioSink != NULL)	{

		nErr = mAudioSink->pause(aFadeOut);
	}   

	mCritical.UnLock();

	SetPlayStatus(EStatusPaused);

	TTCAutoLock eventLock(&mCriEvent);
	postMsgEvent(1, ENotifyPause, nErr, 0, NULL);
}

void CTTMediaPlayer::Resume(TTBool aFadeIn)
{
	//LOGI("CTTMediaPlayer::Resume");
	TTInt nErr = TTKErrNone;
	TTPlayStatus tStatus = GetPlayStatus();	
	if (tStatus == EStatusStarting || tStatus == EStatusPrepared)
    {
		mCriStatus.Lock();
        mFakePauseFlag = ETTFalse;
		mCriStatus.UnLock();

		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(1, ENotifyPlay, nErr, 0, NULL);
		return;
    }
	else if (tStatus != EStatusPaused) {
		return;
	}

	mCriStatus.Lock();
	TTBool bStop = mFakeStopFlag;
	mCriStatus.UnLock();

	if(bStop)
		return;

	mCritical.Lock();
	if(mAudioSink) {
		nErr = mAudioSink->resume(false,aFadeIn);
	} else {

	}

	mCritical.UnLock();
	SetPlayStatus(EStatusPlaying);

	mCriStatus.Lock();
    if(mFakePauseFlag)
		mFakePauseFlag = ETTFalse;
	mCriStatus.UnLock();

	TTCAutoLock eventLock(&mCriEvent);
	postMsgEvent(1, ENotifyPlay, nErr, 0, NULL);    
}

TTInt CTTMediaPlayer::doStop(TTBool aIntra)
{
   LOGI("CTTMediaPlayer::doStop");
	TTInt nErr = TTKErrNone;
	TTInt nProxySize = 0;
	if(mSrcDemux != NULL) {
		mSrcDemux->CancelReader();
	}

	TTPlayStatus tStatus = GetPlayStatus();	
	 LOGI("CTTMediaPlayer::doStop status %d",(TTInt)tStatus);
	if (tStatus == EStatusStoped) {
		return nErr;
	} 

#ifdef __TT_OS_IOS__
    gIosStop = ETTTrue;
#endif
	mCritical.Lock();
	//TTInt64 nStart = GetTimeOfDay();
	if (mAudioSink != NULL) {
		nErr = mAudioSink->stop();
	}
	//TTInt64 nEnd = GetTimeOfDay();
	//LOGI("CTTMediaPlayer::doStop Audio use Time %lld", nEnd - nStart);

	//nStart = GetTimeOfDay();
	mCriVideo.Lock();

	mCriVideo.UnLock();
	//nEnd = GetTimeOfDay();
	//LOGI("CTTMediaPlayer::doStop Video use Time %lld", nEnd - nStart);

	//nStart = GetTimeOfDay();

	if (mAudioSink != NULL) {
		nErr = mAudioSink->close();
	}
	//nEnd = GetTimeOfDay();
	//LOGI("CTTMediaPlayer::doStop Audio use Time %lld", nEnd - nStart);

	//nStart = GetTimeOfDay();

	
	if(mSrcDemux != NULL) {
		nProxySize = mSrcDemux->ProxySize();
		nErr = mSrcDemux->RemoveDataSource();
	}
	mCritical.UnLock();
	//nEnd = GetTimeOfDay();	
	//LOGI("CTTMediaPlayer::doStop RemoveDataSource Time %lld", nEnd - nStart);
	mCriStatus.Lock();
    if(mFakeStopFlag)
		mFakeStopFlag = ETTFalse;
	
	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStopTime = 0;
	mCriStatus.UnLock();

	SetPlayStatus(EStatusStoped);

	if(!aIntra) {
		if(mHandleThread) {
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToOpen);
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
			mHandleThread->cancelEventByMsg(ENotifySCMediaOpenSeek);
			mHandleThread->cancelEventByMsg(ENotifySCMediaPlaySeek);
		}		
		if(mSCDemux) {
			mSrcDemux->CancelReader();
			mCriSCSrc.Lock();
			mSCDemux->RemoveDataSource();
			mCriSCSrc.UnLock();
		}

		mCriStatus.Lock();
		mSCDoing = ESwitchNone;
		mCriStatus.UnLock();

		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(-1, ENotifyClose, nErr, nProxySize, NULL);
	}

	return nErr;
}

TTInt CTTMediaPlayer::Stop(TTBool aSync)
{
    LOGI("CTTMediaPlayer::Stop");
	TTInt nErr = TTKErrNone;
	if(mSrcDemux != NULL) {
		mSrcDemux->CancelReader();
	}

	mCriStatus.Lock();
	mFakePauseFlag = false;
	mFakeStopFlag = true;

	mPlayRange.bEnable = false;
	mPlayRange.nStartTime = 0;
	mPlayRange.nStartTime = 0;
	mCriStatus.UnLock();

	if(aSync) {
		return doStop(false);
	}

	postStopEvent(0);
	return nErr;
}

void CTTMediaPlayer::SetView(void* aView)
{
	mView = aView;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if(tPlayStatus == EStatusStoped) {
		return;
	}
	
	TTCAutoLock videoLock(&mCriVideo);

}

void CTTMediaPlayer::SetVolume(TTInt aLVolume, TTInt aRVolume)
{
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		TTCAutoLock Lock(&mCritical);
		if (mAudioSink != NULL) {
			mAudioSink->setVolume(aLVolume, aRVolume);
		}
	}
}

TTInt CTTMediaPlayer::GetVolume()
{
	TTInt nVolume = 0;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		TTCAutoLock Lock(&mCritical);
		if (mAudioSink != NULL) {
			nVolume = mAudioSink->volume();
		}
	}

    return nVolume;
}

TTInt64 CTTMediaPlayer::seek(TTInt64 aPosition, TTInt aOption)
{
	TTPlayStatus tStatus = GetPlayStatus();
	if(tStatus == EStatusStoped || tStatus == EStatusStarting) {
		TTCAutoLock Lock(&mCriStatus);
		mPrePlayPos = aPosition;
		mSeekOption = aOption;
		return aPosition;
	}

	if(tStatus == EStatusPrepared) {
		if(aPosition == 0) {
			TTCAutoLock eventLock(&mCriEvent);
			postMsgEvent(1, ENotifySeekComplete, aPosition, 0, NULL);
			return aPosition;
		}
	}

	TTCAutoLock Lock(&mCritical);


	if(mSrcDemux == NULL) {
		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(1, ENotifySeekComplete, TTKErrNotReady, 0, NULL);		
		return TTKErrNotReady;
	}

	setSeekStatus(true);
	if(mSCDoing > 0) {
		if(mSCDoing == ESwitchInit || mSCDoing == ESwitchStarted || mSCDoing == ESwitchStarting) {
			if(mHandleThread) {
				mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
			}

			TTCAutoLock eventLock(&mCriEvent);
			postSCMsgEvent(0, ENotifySCMediaOpenSeek, aPosition, aOption, NULL);
			return aPosition;
		} else if(mSCDoing == ESwitchDoing || mSCDoing == ESwitchDone) {
			TTCAutoLock eventLock(&mCriEvent);
			postSCMsgEvent(0, ENotifySCMediaPlaySeek, aPosition, aOption, NULL);
			return aPosition;
		}
	}

	if(tStatus == EStatusPlaying)
		Pause();

	aPosition = mSrcDemux->Seek(aPosition, aOption);
	if(aPosition >= 0) {
		if (mAudioSink != NULL) {
			mAudioSink->syncPosition(aPosition, aOption);
		}

	}

	if(aPosition == TTKErrEof) {
		if (mAudioSink != NULL) {
			mAudioSink->setEOS();
		}

	}

	if(tStatus == EStatusPlaying) {
		Resume();
	} 

	if(aPosition < 0) {
		setSeekStatus(false);
	}

	TTCAutoLock eventLock(&mCriEvent);
	postMsgEvent(1, ENotifySeekComplete, aPosition, 0, NULL);

	return aPosition;	
}

TTInt64 CTTMediaPlayer::SetPosition(TTInt64 aPosition, TTInt aOption)
{
	mCriStatus.Lock();
	if (mPlayRange.bEnable)  {
		aPosition += mPlayRange.nStartTime;
	}
	TTBool bStop = mFakeStopFlag;
	mCriStatus.UnLock();

	if(bStop) {
		return TTKErrCancel;
	}

	return seek(aPosition, aOption);
}

void CTTMediaPlayer::setSeekStatus(TTBool bSeeking)
{
	TTCAutoLock Lock(&mCriStatus);
	mSeekable = bSeeking;
	mSeeking = bSeeking;

	//LOGI("CTTMediaPlayer::setSeekStatus mSeekable %d", (TTInt)mSeekable);
}

TTBool CTTMediaPlayer::getSeekable()
{
	TTBool bSeeking = false;
	
	mCriStatus.Lock();
	bSeeking = mSeekable;
	mCriStatus.UnLock();

	return bSeeking;
}

TTUint64 CTTMediaPlayer::GetPosition()
{
	TTUint64 nPos = 0;
	mCriStatus.Lock();
	TTInt64 nPrePlayPos = mPrePlayPos;
	mCriStatus.UnLock();

	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusStarting) || (tPlayStatus == EStatusStoped)) {
		if(nPrePlayPos)
			nPos = nPrePlayPos;
		return nPos;
	}

	if(nPrePlayPos != 0)
		nPos = nPrePlayPos;
	else
		nPos = GetPlayTime();

	mCriStatus.Lock();
	if(mPlayRange.bEnable) {
		if(nPos > mPlayRange.nStartTime)
			nPos = nPos - mPlayRange.nStartTime;
		else
			nPos = 0;
	}
	mCriStatus.UnLock();

	if(nPos > Duration()) nPos = Duration();

    return nPos;
}

TTInt64 CTTMediaPlayer::GetPlayTime()
{
   	TTUint64 nPos = 0;

	TTCAutoLock Lock(&mCritical);
	if (mAudioSink != NULL)	{
		nPos = mAudioSink->getPlayTime();
		return nPos;
	}

	return 0;
}

TTInt CTTMediaPlayer::onNotifyEvent(TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	if(nMsg == ENotifyBufferingStart) {
		if(getSeekable()) {
			TTCAutoLock Lock(&mCriStatus);
			mSeekable = false;
		}

		mCritical.Lock();
		if (mAudioSink != NULL)	{
			mAudioSink->pause();
			mAudioSink->setBufferStatus(ETTBufferingStart);
		}
		mCritical.UnLock();
	}else if(nMsg == ENotifyBufferingDone) {
		mCritical.Lock();
		if (mAudioSink != NULL)	{
			mAudioSink->setBufferStatus(ETTBufferingDone);
			if(GetPlayStatus() == EStatusPlaying)
				mAudioSink->resume();			
		} 
		mCritical.UnLock();
	}  else if(nMsg == ENotifyClose) {
		mException = 0;
	}
	//else if(nMsg == ENotifyPlayerInfo)  {
	//	TTInt nPlayTime = GetPosition();
	//	TTInt64	nSystemTime = GetTimeOfDay();		
	//	LOGI("CTTMediaPlayer::PlayTime %d, Diff %d, SysTime %lld, SysDiff %d", nPlayTime, nPlayTime - mLastPlayTime, nSystemTime, (TTInt)(nSystemTime - mLastSystemTime));
	//	mLastPlayTime = nPlayTime;
	//	mLastSystemTime = nSystemTime;

	//	TTCAutoLock eventLock(&mCriEvent);
	//	postMsgEvent(20, ENotifyPlayerInfo, 0, 0, NULL);
	//	return TTKErrNone;
	//}

	if(mPlayerObserver)
		mPlayerObserver->PlayerNotifyEvent((TTNotifyMsg)nMsg, nVar1, nVar2, (TTChar *)nVar3);

	return TTKErrNone;
}


TTUint CTTMediaPlayer::Duration()
{	
	TTUint nDuration = 0;

	if(mSrcDemux == NULL)
		return nDuration;

	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		mCriStatus.Lock();
		if(mPlayRange.bEnable) {
			nDuration = mPlayRange.nStopTime - mPlayRange.nStartTime;
			mCriStatus.UnLock();
		}else {
			mCriStatus.UnLock();
			mCritical.Lock();
			nDuration = mSrcDemux->MediaDuration();
			mCritical.UnLock();
		}
	}

	return nDuration;
}

TTUint CTTMediaPlayer::Size() 
{
	LOGI("CTTMediaPlayer::Size");
	TTUint nSize = 0;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		TTCAutoLock Lock(&mCritical);
		if(mSrcDemux)
			nSize = mSrcDemux->MediaSize();
	}

	return nSize;
}

TTUint CTTMediaPlayer::BufferedSize()
{
	LOGI("CTTMediaPlayer::BufferedSize");
	TTUint nBufferedSize = 0;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		TTCAutoLock Lock(&mCritical);
		if(mSrcDemux != NULL) {
			nBufferedSize = mSrcDemux->BufferedSize();
		}
	}

	return nBufferedSize;
}

TTUint CTTMediaPlayer::BandPercent()
{
	TTInt nBandPercent = 0;
	if(mSrcDemux != NULL)	{
		nBandPercent = mSrcDemux->BandPercent();
	}
	return nBandPercent;
}


TTUint CTTMediaPlayer::BandWidth()
{
	TTInt nBandWidth = 0;
	if(mSrcDemux != NULL)	{
		nBandWidth = mSrcDemux->BandWidth();
	}
	return nBandWidth;
}

TTInt CTTMediaPlayer::BufferedPercent(TTInt& aBufferedPercent)
{	
	//LOGI("CTTMediaPlayer::BufferedPercent");
	TTInt nErr = TTKErrNotReady;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{
		TTCAutoLock Lock(&mCritical);
		if(mSrcDemux != NULL)	{
			nErr = mSrcDemux->BufferedPercent(aBufferedPercent);
		}
	}
	//LOGI("CTTMediaPlayer::BufferedPercent return %d", nErr);
	return nErr;
}

TTInt CTTMediaPlayer::GetCurrentFreqAndWave(TTInt16 *aFreq, TTInt16 *aWave, TTInt aSampleNum)
{
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if (tPlayStatus != EStatusPlaying)
	{
		return TTKErrNotReady;
	}

	if (((aSampleNum & 0xF) != 0) || (aSampleNum > KSinkBufferSize) || aWave == NULL)
	{
		return TTKErrArgument;
	}

	TTInt nChannels = 0;

	mCritical.Lock();
	if(mAudioSink == NULL) {
		mCritical.UnLock();
		return TTKErrNotFound;
	}
	TTInt nErr = mAudioSink->getCurWave(aSampleNum, aWave, nChannels);
	mCritical.UnLock();

	if (nErr == TTKErrNone)
	{
		if (aFreq != NULL)
		{
//			TTFFT::WaveformToFreqBin(aFreq, aWave, nChannels, aSampleNum);
		}
	}

	return TTKErrNone;
}

TTInt CTTMediaPlayer::SetParam(TTInt nID, void* aParam)
{
	return TTKErrNone;
}

TTInt CTTMediaPlayer::GetParam(TTInt nID, void* aParam)
{
	return TTKErrNone;
}

void CTTMediaPlayer::SetActiveNetWorkType(TTActiveNetWorkType aNetWorkType)
{
	LOGI("CTTMediaPlayer::SetActiveNetWorkType: %d", aNetWorkType);
	TTNetWorkConfig* pNetWorkConfig = TTNetWorkConfig::getInstance();
	pNetWorkConfig->SetActiveNetWorkType(aNetWorkType);
	LOGI("CTTMediaPlayer::SetActiveNetWorkType return");
}

void CTTMediaPlayer::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	TTPlayStatus tPlayStatus = GetPlayStatus();
	LOGI("CTTMediaPlayer::SetNetWorkProxy: %d", aNetWorkProxy);
	if (tPlayStatus != EStatusStoped)
	{
		if(mSrcDemux) {
			mSrcDemux->SetNetWorkProxy(aNetWorkProxy);
		}
	}
	LOGI("CTTMediaPlayer::SetNetWorkProxy return");
}

void CTTMediaPlayer::SetDecoderType(TTDecoderType aDecoderType)
{
	mDecoderType = aDecoderType;
	TTPlayStatus tPlayStatus = GetPlayStatus();
	if ((tPlayStatus == EStatusPlaying) || (tPlayStatus == EStatusPaused) || (tPlayStatus == EStatusPrepared))
	{

	}	
}

#ifdef __TT_OS_IOS__
void CTTMediaPlayer::SetBalanceChannel(float aVolume)
{
    if (mAudioSink != NULL) {
        mAudioSink->SetBalanceChannel(aVolume);
    }
}

void CTTMediaPlayer::SetRotate()
{

}
#endif

TTInt CTTMediaPlayer::Play()
{
	//LOGI("++++Play: Time %lld", GetTimeOfDay());

	LOGI("CTTMediaPlayer::Play");
	TTInt nErr = TTKErrNotReady;
	TTPlayStatus tStatus = GetPlayStatus();

	if(tStatus == EStatusPaused || tStatus == EStatusPlaying)
		return TTKErrAccessDenied;
	else if(tStatus == EStatusStoped || tStatus == EStatusStarting)
		return nErr;

	if ((mPrePlayPos != 0) && (Duration() > 0))
	{
		seek(mPrePlayPos, mSeekOption);
		mCriStatus.Lock();
		mPrePlayPos = 0;
		mSeekOption = 0;
		mCriStatus.UnLock();
	}

	mCriStatus.Lock();
	TTBool bPause = mFakePauseFlag;
	mCriStatus.UnLock();

	mCritical.Lock();	
	if(mAudioSink) {
		nErr = mAudioSink->start(bPause, false);
	} 
	mCritical.UnLock();

	if(!bPause){
		SetPlayStatus(EStatusPlaying);
		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(1, ENotifyPlay, nErr, 0, NULL);
	}else {
		SetPlayStatus(EStatusPaused);
		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(1, ENotifyPause, nErr, 0, NULL);
	}

	//TTCAutoLock eventLock(&mCriEvent);
	//postMsgEvent(20, ENotifyPlayerInfo, 0, 0, NULL);
	//mLastPlayTime = 0;
	//mLastSystemTime = GetTimeOfDay();

	LOGI("CTTMediaPlayer::Play return %d", nErr);

	//LOGI("----Play: Time %lld", GetTimeOfDay());
	return nErr;
}

void CTTMediaPlayer::SetPlayRange(TTUint aStartTime, TTUint aEndTime)
{
	TTPlayStatus tStatus = GetPlayStatus();
	if(tStatus == EStatusStoped || tStatus == EStatusStarting)
		return;

	mCriStatus.Lock();
	mPlayRange.bEnable = true;
	mPlayRange.nStartTime = aStartTime;
	mPlayRange.nStopTime = aEndTime;	
	mCriStatus.UnLock();

	if(mAudioSink) {
		mAudioSink->setPlayRange(aStartTime, aEndTime);
	}


	seek(aStartTime, 0);
}

void CTTMediaPlayer::SetCacheFilePath(const TTChar* aCacheFilePath)
{
	LOGI("CTTMediaPlayer::SetCacheFilePath: %s", aCacheFilePath);
	if (strlen(aCacheFilePath) >= FILE_PATH_MAX_LENGTH)
	{
		aCacheFilePath = NULL;
		LOGE("CTTMediaPlayer::SetCacheFilePath error");		
	}
	else
	{
		strcpy(mCacheFilePath, aCacheFilePath);
	}

	gSetCacheFilePath(mCacheFilePath);
	LOGI("CTTMediaPlayer::SetCacheFilePath return");
}

#ifdef __TT_OS_ANDROID__

void CTTMediaPlayer::SaveAudioTrackJClass(void* pJclass)
{
	mpAudiotrackJClass = pJclass;
}

void* CTTMediaPlayer::GetAudiotrackClass()
{
	return mpAudiotrackJClass;
}

void CTTMediaPlayer::SetMaxOutPutSamplerate(TTInt aSampleRate)
{
	gMaxOutPutSamplerate = aSampleRate;
}

void CTTMediaPlayer::SaveVideoTrackJClass(void* pJclass)
{
	mpVideotrackJClass = pJclass;
}

void* CTTMediaPlayer::GetVideotrackClass()
{
	return mpVideotrackJClass;
}
#endif

TTInt CTTMediaPlayer::onPreSource(TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	TTInt nErr = TTKErrNotFound;

	if(nMsg == ENotifyMediaPreOpen) {
		mCriEvent.Lock();
		postMsgEvent(0, ENotifyMediaPreOpenStart, 0, 0, NULL);
		mCriEvent.UnLock();

		mCriSCSrc.Lock();
		if(mPreDemux != NULL) {
			mPreDemux->CancelReader();
			mPreDemux->RemoveDataSource();
			mPreDemux->SetObserver(&mPreSrcObserver);
		} else {
			mPreDemux = new CTTSrcDemux(&mPreSrcObserver);
		}

		mCriStatus.Lock();
		TTInt nFlag = mPreSrcFlag;
		TTChar*	nUrl = (TTChar*) malloc((strlen(mPreUrl) + 1) * sizeof(TTChar));
		strcpy(nUrl, mPreUrl);
		mCriStatus.UnLock();

		nErr = mPreDemux->AddDataSource(nUrl, nFlag);
		if(nErr) {
			TTInt nStatusCode = 0;
			mPreDemux->GetParam(TT_PID_COMMON_STATUSCODE, &nStatusCode);
			TTInt nHostIP = 0;
			char *pParam3 = NULL;
			mPreDemux->GetParam(TT_PID_COMMON_HOSTIP, &nHostIP);
			if(nHostIP) {
				pParam3 = inet_ntoa(*(struct in_addr*)&nHostIP);
			}
			mPreDemux->RemoveDataSource();
			mCriSCSrc.UnLock();

			mCriStatus.Lock();
			SAFE_FREE(nUrl);
			SAFE_FREE(mPreUrl);
			mCriStatus.UnLock();
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifyMediaPreOpenFailed, nErr, nStatusCode, pParam3);
			return nErr;
		}
		SAFE_FREE(nUrl);
		mCriSCSrc.UnLock();
	} else if(nMsg == ENotifyMediaPreRelease) {
		if(mPreDemux != NULL) {
			mPreDemux->CancelReader();
			mPreDemux->RemoveDataSource();
			mPreDemux->SetObserver(&mPreSrcObserver);
		}

		mCriStatus.Lock();
		SAFE_FREE(mPreUrl);
		mCriStatus.UnLock();
	} else if(nMsg == ENotifyMediaOpenLoaded) {
		mCritical.Lock();
		mCriSCSrc.Lock();
		mPreDemux->SetDownSpeed(1);
		CTTSrcDemux* SrcDemux = mSrcDemux;
		mSrcDemux = mPreDemux;
		mPreDemux = SrcDemux;
		mSrcDemux->SetObserver(&mSrcObserver);
		mPreDemux->SetObserver(&mPreSrcObserver);
		mCriSCSrc.UnLock();

		InitSink();
		mCritical.UnLock();

		mCriStatus.Lock();
		TTBool bStop = mFakeStopFlag;
		mFakeStopFlag = false;
		mCriStatus.UnLock();

		TTInt nAVFlag = 0;
		mCritical.Lock();
		if((mAudioSink == 0 ) || bStop) {
			mSrcDemux->RemoveDataSource();
			mCritical.UnLock();
			TTCAutoLock LockEvent(&mCriEvent);
			if(bStop) {
				postMsgEvent(-1, ENotifyClose, 0, 0, NULL);
			} else {
				postMsgEvent(-1, ENotifyException, TTKErrFileNotSupport, 0, NULL);
				postMsgEvent(-1, ENotifyClose, TTKErrFileNotSupport, 0, NULL);
			}
			return TTKErrCancel;
		}
		
		if(mAudioSink) {
			nAVFlag |= 1;
		}

		mCritical.UnLock();

		mCriStatus.Lock();
		SAFE_FREE(mUrl);
		mUrl = (TTChar*) malloc((strlen(mPreUrl) + 1) * sizeof(TTChar));
		strcpy(mUrl, mPreUrl);
		SAFE_FREE(mPreUrl);
		mCriStatus.UnLock();

		SetPlayStatus(EStatusPrepared);

		TTCAutoLock eventLock(&mCriEvent);
		postMsgEvent(1, ENotifyPrepare, TTKErrNone, nAVFlag, NULL);
	}

	return TTKErrNone;
}

TTInt CTTMediaPlayer::onSCEvent(TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	TTInt nErr = TTKErrNotFound;

	if(nMsg == ENotifySCMediaStartToOpen) {
		LOGI("onSCEvent::ENotifySCMediaStartToOpen");
		mCriEvent.Lock();
		postMsgEvent(0, ENotifyMediaChangedStart, 0, 0, NULL);
		mCriEvent.UnLock();

		mCriStatus.Lock();
		mSCDoing = ESwitchStarting;
		mCriStatus.UnLock();

		TTInt64 nNowTime = GetPlayTime();
		if(nNowTime + 15000 > Duration()) {
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifyMediaChangedFailed, TTKErrCompletion, 0, NULL);
			mCriStatus.Lock();
			mSCDoing = ESwitchNone;
			mCriStatus.UnLock();
			mCriSCSrc.Lock();
			mSCDemux->CancelReader();
			mSCDemux->RemoveDataSource();
			mCriSCSrc.UnLock();
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
			return TTKErrCompletion;
		}

		mCriStatus.Lock();
		if(strcmp(mSCUrl, mUrl) == 0) {
			mSCDoing = ESwitchNone;
			mCriStatus.UnLock();

			mCriSCSrc.Lock();
			mSCDemux->CancelReader();
			mSCDemux->RemoveDataSource();
			mCriSCSrc.UnLock();
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifyMediaChangedSucess, 0, 0, NULL);
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
			return TTKErrNone;
		}
		mCriStatus.UnLock();

		mCriSCSrc.Lock();
		if(mSCDemux != NULL) {
			mSCDemux->CancelReader();
			mSCDemux->RemoveDataSource();
			mSCDemux->SetObserver(&mSCSrcObserver);
		} else {
			mSCDemux = new CTTSrcDemux(&mSCSrcObserver);
		}

		mCriStatus.Lock();
		TTInt nFlag = mSCSrcFlag;
		TTChar*	nUrl = (TTChar*) malloc((strlen(mSCUrl) + 1) * sizeof(TTChar));
		strcpy(nUrl, mSCUrl);
		TTBool bSeeking = mSeeking;
		mCriStatus.UnLock();

		if(bSeeking) {
			postMsgEvent(0, ENotifyBufferingStart, 0, 0, NULL);
		}
		nErr = mSCDemux->AddDataSource(nUrl, nFlag);
		if(bSeeking) {
			postMsgEvent(0, ENotifyBufferingDone, 0, 0, NULL);
		}

		if(nErr) {
			TTInt nStatusCode = 0;
			mSCDemux->GetParam(TT_PID_COMMON_STATUSCODE, &nStatusCode);
			TTInt nHostIP = 0;
			char *pParam3 = NULL;
			mSCDemux->GetParam(TT_PID_COMMON_HOSTIP, &nHostIP);
			if(nHostIP) {
				pParam3 = inet_ntoa(*(struct in_addr*)&nHostIP);
			}
			mSCDemux->RemoveDataSource();
			mCriSCSrc.UnLock();
			SAFE_FREE(nUrl);
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifyMediaChangedFailed, nErr, nStatusCode, pParam3);
			mCriStatus.Lock();
			mSCDoing = ESwitchNone;
			mCriStatus.UnLock();
			mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
			return nErr;
		}

		TTInt nAudioStreamId = EMediaStreamIdNone;
		TTInt nVideoStreamId = EMediaStreamIdNone;

		const TTMediaInfo& tMeidaInfo = mSCDemux->GetMediaInfo();
		const RTTPointerArray<TTAudioInfo>& rMediaAudioArrary = tMeidaInfo.iAudioInfoArray;

		if(rMediaAudioArrary.Count() > 0 && rMediaAudioArrary[0]) {
			nAudioStreamId = rMediaAudioArrary[0]->iStreamId;
			mSCDemux->SelectStream(EMediaTypeAudio, nAudioStreamId);
		}

		if(tMeidaInfo.iVideoInfo) {
			nVideoStreamId = tMeidaInfo.iVideoInfo->iStreamId;
			mSrcDemux->SelectStream(EMediaTypeVideo, nVideoStreamId);
		}

		SAFE_FREE(nUrl);
		mCriStatus.Lock();	
		if(mSCDoing != ESwitchStarting) {
			mCriStatus.UnLock();
			return TTKErrNone;
		}
		mCriStatus.UnLock();
		
		TTInt nDelayTime = 0;
		TTInt OffsetTime = 0;
		TTInt64 nSeekTime = 0;

		nNowTime = GetPlayTime();
		mSCSeekTime = nNowTime;
		nSeekTime = mSCDemux->Seek(nNowTime + 5000);
		if(nSeekTime < nNowTime) {
			nSeekTime = mSCDemux->Seek(nNowTime + 10000);
		}
		mCriSCSrc.UnLock();

		nDelayTime = nSeekTime - nNowTime - 100;
		if(nDelayTime < 0) nDelayTime = 0;
		TTCAutoLock LockEvent(&mCriEvent);
		postSCMsgEvent(nDelayTime , ENotifySCMediaStartToPlay, 0, 0, NULL);
		mSCSeekTime = nSeekTime;

		mCriStatus.Lock();	
		mSCDoing = ESwitchStarted;
		mCriStatus.UnLock();
	} else if( nMsg == ENotifySCMediaStartToPlay) {
		LOGI("onSCEvent::ENotifySCMediaStartToPlay");
		TTPlayStatus nStatus = GetPlayStatus();
		TTInt64 nPlayingTime = GetPlayTime();		
		if(mSCSeekTime - nPlayingTime > 200 && mSCSeekTime - nPlayingTime <= 5000) {
			TTCAutoLock LockEvent(&mCriEvent);
			TTInt nDelayTime = mSCSeekTime - nPlayingTime - 100;
			postSCMsgEvent(nDelayTime , ENotifySCMediaStartToPlay, 0, 0, NULL);
			return TTKErrNone; 
		}

		mCriStatus.Lock();
		if(mSCDoing == ESwitchNone) {
			mCriStatus.UnLock();
			return TTKErrNone;
		}
		mSCDoing = ESwitchDoing;
		mCriStatus.UnLock();

		if(nStatus != EStatusStoped) {
			doStop(true);
		}
		mCritical.Lock();

		mCriSCSrc.Lock();
		if(nPlayingTime - mSCSeekTime > 5000 || nPlayingTime - mSCSeekTime < -2000) {
			nPlayingTime = mSCDemux->Seek(nPlayingTime);
		}

		CTTSrcDemux* SrcDemux = mSrcDemux;
		mSrcDemux = mSCDemux;
		mSCDemux = SrcDemux;
		mSrcDemux->SetObserver(&mSrcObserver);
		mSCDemux->SetObserver(&mSCSrcObserver);
		mCriSCSrc.UnLock();

		InitSink();

		mCritical.UnLock();

		mCriStatus.Lock();
		TTBool bStop = mFakeStopFlag;
		mFakeStopFlag = false;
		mCriStatus.UnLock();

		mCritical.Lock();
		if((mAudioSink == 0 ) || bStop) {
			mSrcDemux->RemoveDataSource();
			mCritical.UnLock();
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifyMediaChangedFailed, TTKErrFileNotSupport, 0, NULL);
			mCriStatus.Lock();
			mSCDoing = ESwitchNone;
			mCriStatus.UnLock();
			return TTKErrCancel;
		}
		mCritical.UnLock();

		SetPlayStatus(EStatusPrepared);

		TTBool bPause = false;
		if(nStatus == EStatusPaused)
			bPause = true;

		mCritical.Lock();	
		if(mAudioSink) {
			mAudioSink->syncPosition(nPlayingTime);
			mAudioSink->start(bPause, false);
		}		
		
		mCritical.UnLock();

		if(bPause) {
			SetPlayStatus(EStatusPaused);
		} else {
			SetPlayStatus(EStatusPlaying);
		}

		mCriStatus.Lock();
		SAFE_FREE(mUrl);
		mUrl = (TTChar*) malloc((strlen(mSCUrl) + 1) * sizeof(TTChar));
		strcpy(mUrl, mSCUrl);
		mSCDoing = ESwitchDone;
		mCriStatus.UnLock();

		TTCAutoLock LockEvent(&mCriEvent);
		postMsgEvent(0, ENotifyMediaChangedSucess, 0, 0, NULL);
		mCriStatus.Lock();
		mSCDoing = ESwitchNone;
		mCriStatus.UnLock();
	} else if(nMsg == ENotifySCMediaOpenSeek) {
		LOGI("onSCEvent::ENotifySCMediaOpenSeek");
		mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
		TTPlayStatus nStatus = GetPlayStatus();
		TTInt64 nPlayingTime = nVar1;		
		TTInt   nOption = nVar2;

		mCriStatus.Lock();
		if(mSCDoing == ESwitchNone) {
			mCriStatus.UnLock();
			return TTKErrNone;
		}
		mSCDoing = ESwitchDoing;
		mCriStatus.UnLock();

		if(nStatus != EStatusStoped) {
			doStop(true);
		}

		mCritical.Lock();
		mCriSCSrc.Lock();
		CTTSrcDemux* SrcDemux = mSrcDemux;
		mSrcDemux = mSCDemux;
		mSCDemux = SrcDemux;
		mSrcDemux->SetObserver(&mSrcObserver);
		mSCDemux->SetObserver(&mSCSrcObserver);
		mCriSCSrc.UnLock();

		nPlayingTime = mSrcDemux->Seek(nPlayingTime, nOption);
		InitSink();
		mCritical.UnLock();

		mCriStatus.Lock();
		TTBool bStop = mFakeStopFlag;
		mFakeStopFlag = false;
		mCriStatus.UnLock();

		mCritical.Lock();
		if((mAudioSink == 0) || bStop) {
			mSrcDemux->RemoveDataSource();
			mCritical.UnLock();
			TTCAutoLock LockEvent(&mCriEvent);
			postMsgEvent(0, ENotifySeekComplete, TTKErrFileNotSupport, 0, NULL);
			postMsgEvent(0, ENotifyMediaChangedFailed, TTKErrFileNotSupport, 0, NULL);
			mCriStatus.Lock();
			mSCDoing = ESwitchNone;
			mCriStatus.UnLock();
			return nErr;
		}
		mCritical.UnLock();

		SetPlayStatus(EStatusPrepared);

		TTBool bPause = false;
		if(nStatus == EStatusPaused)
			bPause = true;

		mCritical.Lock();	
		if(mAudioSink) {
			mAudioSink->syncPosition(nPlayingTime, nOption);
			mAudioSink->start(bPause, false);
		}		

		mCritical.UnLock();

		if(bPause) {
			SetPlayStatus(EStatusPaused);
		} else {
			SetPlayStatus(EStatusPlaying);
		}

		mCriStatus.Lock();
		SAFE_FREE(mUrl);
		mUrl = (TTChar*) malloc((strlen(mSCUrl) + 1) * sizeof(TTChar));
		strcpy(mUrl, mSCUrl);
		mCriStatus.UnLock();

		TTCAutoLock LockEvent(&mCriEvent);
		postMsgEvent(0, ENotifySeekComplete, nPlayingTime, 0, NULL);
		postMsgEvent(0, ENotifyMediaChangedSucess, 0, 0, NULL);		
		mCriStatus.Lock();
		mSCDoing = ESwitchNone;
		mCriStatus.UnLock();
		return TTKErrNone;
	} else if(nMsg == ENotifySCMediaPlaySeek) {
		LOGI("onSCEvent::ENotifySCMediaPlaySeek");
		mHandleThread->cancelEventByMsg(ENotifySCMediaStartToPlay);
		TTPlayStatus nStatus = GetPlayStatus();
		TTInt64 nPlayingTime = nVar1;
		TTInt   nOption = nVar2;
		seek(nPlayingTime, nOption);
		return TTKErrNone;
	}

	return TTKErrNone;
}

TTInt CTTMediaPlayer::onSetDataSource(TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	TTInt nErr = SetDataSource();
	return nErr;
}

TTInt CTTMediaPlayer::onStop(TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3)
{
	TTInt nErr = doStop(false);
	return nErr;
}

TTInt CTTMediaPlayer::postSetDataSourceEvent (TTInt  nDelayTime)
{
	if (mHandleThread == NULL)
		return TTKErrNotFound;

	mHandleThread->cancelEventByID(ENotifyMediaStartToOpen, false);
	mHandleThread->cancelEventByType(EEventLoad, false);
	TTBaseEventItem * pEvent = mHandleThread->getEventByType(EEventLoad);
	if (pEvent == NULL)
		pEvent = new TTCMediaPlayerEvent (this, &CTTMediaPlayer::onSetDataSource, EEventLoad);
	mHandleThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

TTInt CTTMediaPlayer::postStopEvent(TTInt  nDelayTime)
{
	if (mHandleThread == NULL)
		return TTKErrNotFound;

	TTBaseEventItem * pEvent = mHandleThread->cancelEventByType(EEventClose, false);
	mHandleThread->cancelEventByID(ENotifyMediaStartToOpen, false);
	pEvent = mHandleThread->cancelEventByType(EEventLoad, false);	
	if(pEvent) {
		mCriStatus.Lock();
		mFakeStopFlag = false;
		mPlayStatus = EStatusStoped;
		mCriStatus.UnLock();
	}

	pEvent = mHandleThread->getEventByType(EEventClose);
	if (pEvent == NULL)
		pEvent = new TTCMediaPlayerEvent (this, &CTTMediaPlayer::onStop, EEventClose);
	mHandleThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

TTInt CTTMediaPlayer::postPreSrcEvent (TTInt  nDelayTime, TTInt32 nID, int nParam1, int nParam2, void * pParam3)
{
	if (mHandleThread == NULL)
		return TTKErrNotFound;

	TTBaseEventItem * pEvent = mHandleThread->getEventByType(EEventMsg);
	if (pEvent == NULL)
		pEvent = new TTCMediaPlayerEvent (this, &CTTMediaPlayer::onPreSource, EEventMsg, nID, nParam1, nParam2, pParam3);
	else
		pEvent->setEventMsg(nID, nParam1, nParam2, pParam3);
	mHandleThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

TTInt CTTMediaPlayer::postSCMsgEvent (TTInt  nDelayTime, TTInt32 nID, int nParam1, int nParam2, void * pParam3)
{
	if (mHandleThread == NULL)
		return TTKErrNotFound;

	TTBaseEventItem * pEvent = mHandleThread->getEventByType(EEventMsg);
	if (pEvent == NULL)
		pEvent = new TTCMediaPlayerEvent (this, &CTTMediaPlayer::onSCEvent, EEventMsg, nID, nParam1, nParam2, pParam3);
	else
		pEvent->setEventMsg(nID, nParam1, nParam2, pParam3);
	mHandleThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

TTInt CTTMediaPlayer::postMsgEvent (TTInt nDelayTime, TTInt32 nID, int nParam1, int nParam2, void * pParam3)
{
	if (mMsgThread == NULL)
		return TTKErrNotFound;

	TTBaseEventItem * pEvent = mMsgThread->getEventByType(EEventMsg);
	if (pEvent == NULL)
		pEvent = new TTCMediaPlayerEvent (this, &CTTMediaPlayer::onNotifyEvent, EEventMsg, nID, nParam1, nParam2, pParam3);
	else
		pEvent->setEventMsg(nID, nParam1, nParam2, pParam3);
	mMsgThread->postEventWithDelayTime (pEvent, nDelayTime);

	return TTKErrNone;
}

TTInt CTTMediaPlayer::SetPlayStatus(TTPlayStatus aStatus)
{
	LOGI("CTTMediaPlayer::SetPlayStatus %d", aStatus);
	TTCAutoLock Lock(&mCriStatus);
	mPlayStatus = aStatus;

	return TTKErrNone;
}

TTPlayStatus CTTMediaPlayer::GetPlayStatus()
{
	TTCAutoLock Lock(&mCriStatus);
	TTPlayStatus tStatus = mPlayStatus;

	return tStatus;
}

TTInt CTTMediaPlayer::ttSrcCallBack (void* pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	CTTMediaPlayer* pMediaPlayer = (CTTMediaPlayer*)pUserData;

	TTInt nErr = TTKErrNotReady;
	
	if(pMediaPlayer == NULL)
		return TTKErrArgument;

	nErr = 	pMediaPlayer->handleSrcMsg(nID, nParam1, nParam2, pParam3);

	return nErr;
}

TTInt CTTMediaPlayer::ttPreSrcCallBack (void* pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	CTTMediaPlayer* pMediaPlayer = (CTTMediaPlayer*)pUserData;

	TTInt nErr = TTKErrNotReady;
	
	if(pMediaPlayer == NULL)
		return TTKErrArgument;

	nErr = 	pMediaPlayer->handlePreSrcMsg(nID, nParam1, nParam2, pParam3);

	return nErr;
}

TTInt CTTMediaPlayer::handlePreSrcMsg (TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	LOGI("CTTMediaPlayer::handlePreSrcMsg %d", nID);
	TTCAutoLock Lock(&mCriEvent);
	TTInt nMsg = nID;
	TTInt nDelayTime = 1;

	switch(nID) 
	{
	case ESrcNotifyUpdateDuration:
		return	TTKErrNone;
	case ESrcNotifyBufferingStart:
		return	TTKErrNone;
	case ESrcNotifyBufferingDone:
		return	TTKErrNone;
	case ESrcNotifyDNSDone:
		nMsg = ENotifyDNSDone;
		break;
	case ESrcNotifyConnectDone:
		nMsg = ENotifyConnectDone;
		break;
	case ESrcNotifyHttpHeaderReceived:
		nMsg = ENotifyHttpHeaderReceived;
		break;
	case ESrcNotifyPrefetchStart:
		return	TTKErrNone;
	case ESrcNotifyPrefetchCompleted:
		if(mPreDemux) {
			mPreDemux->SetDownSpeed(0);
		}
		nMsg = ENotifyMediaPreOpenSucess;
		break;
	case ESrcNotifyCacheCompleted:
		nMsg = ENotifyCacheCompleted;
		break;
	case ESrcNotifyException:
		nMsg = ENotifyMediaPreOpenFailed;
		nDelayTime = -1;
		break;
	}

	return postMsgEvent(nDelayTime, nMsg, nParam1, nParam2, pParam3);
}

TTInt CTTMediaPlayer::ttSCSrcCallBack (void* pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	CTTMediaPlayer* pMediaPlayer = (CTTMediaPlayer*)pUserData;

	TTInt nErr = TTKErrNotReady;
	
	if(pMediaPlayer == NULL)
		return TTKErrArgument;

	nErr = 	pMediaPlayer->handleSCSrcMsg(nID, nParam1, nParam2, pParam3);

	return nErr;
}

TTInt CTTMediaPlayer::handleSCSrcMsg (TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	LOGI("CTTMediaPlayer::handleSCSrcMsg %d", nID);
	TTCAutoLock Lock(&mCriEvent);
	TTInt nMsg = nID;
	TTInt nDelayTime = 1;

	switch(nID) 
	{
	case ESrcNotifyUpdateDuration:
		nMsg = ENotifyUpdateDuration;
		break;
	case ESrcNotifyBufferingStart:
		return TTKErrNone;
	case ESrcNotifyBufferingDone:
		return TTKErrNone;
	case ESrcNotifyDNSDone:
		nMsg = ENotifyDNSDone;
		break;
	case ESrcNotifyConnectDone:
		nMsg = ENotifyConnectDone;
		break;
	case ESrcNotifyHttpHeaderReceived:
		nMsg = ENotifyHttpHeaderReceived;
		break;
	case ESrcNotifyPrefetchStart:
		return TTKErrNone;
	case ESrcNotifyPrefetchCompleted:
		return TTKErrNone;
	case ESrcNotifyCacheCompleted:
		nMsg = ENotifyCacheCompleted;
		break;
	case ESrcNotifyException:
		nMsg = ENotifyMediaChangedFailed;
		nDelayTime = -1;
		break;
	}

	return postMsgEvent(nDelayTime, nMsg, nParam1, nParam2, pParam3);
}

TTInt CTTMediaPlayer::handleSrcMsg (TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	LOGI("CTTMediaPlayer::handleSrcMsg %d", nID);
	TTCAutoLock Lock(&mCriEvent);

	TTInt nMsg = nID;
	TTInt nDelayTime = 1;
	
	switch(nID) 
	{
	case ESrcNotifyUpdateDuration:
		nMsg = ENotifyUpdateDuration;
		break;
	case ESrcNotifyBufferingStart:
		nDelayTime = 0;
		nMsg = ENotifyBufferingStart;
		break;
	case ESrcNotifyBufferingDone:
		nDelayTime = 0;
		nMsg = ENotifyBufferingDone;
		break;
	case ESrcNotifyDNSDone:
		nMsg = ENotifyDNSDone;
		break;
	case ESrcNotifyConnectDone:
		nMsg = ENotifyConnectDone;
		break;
	case ESrcNotifyHttpHeaderReceived:
		nMsg = ENotifyHttpHeaderReceived;
		break;
	case ESrcNotifyPrefetchStart:
		nMsg = ENotifyPrefetchStart;
		break;
	case ESrcNotifyPrefetchCompleted:
		nMsg = ENotifyPrefetchCompleted;
		break;
	case ESrcNotifyCacheCompleted:
		nMsg = ENotifyCacheCompleted;
		break;
	case ESrcNotifyException:
		nMsg = ENotifyException;
		nDelayTime = -1;
		break;
	}

	return postMsgEvent(nDelayTime, nMsg, nParam1, nParam2, pParam3);
}

TTInt CTTMediaPlayer::ttAudioCallBack (void* pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	CTTMediaPlayer* pMediaPlayer = (CTTMediaPlayer*)pUserData;

	TTInt nErr = TTKErrNotReady;
	
	if(pMediaPlayer == NULL)
		return TTKErrArgument;

	nErr = 	pMediaPlayer->handleAudioMsg(nID, nParam1, nParam2, pParam3);

	return nErr;
}

TTInt CTTMediaPlayer::handleAudioMsg (TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	TTCAutoLock Lock(&mCriEvent);

	if(nID == ENotifyComplete) {
		
			mCriEvent.Lock();
			postMsgEvent(1, ENotifyComplete, mException, nParam2, pParam3);
			mCriEvent.UnLock();
			setSeekStatus(false);
			mException = 0;
		
		return TTKErrNone;
	} else if(nID == ENotifyTimeReset) {

		return TTKErrNone;
	} else if(nID == ENotifySeekComplete) {

			setSeekStatus(false);

		return TTKErrNone; 
	}

	return postMsgEvent(1, nID, nParam1,  nParam2, pParam3);
}


TTInt CTTMediaPlayer::ttVideoCallBack (void* pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	CTTMediaPlayer* pMediaPlayer = (CTTMediaPlayer*)pUserData;

	TTInt nErr = TTKErrNotReady;
	
	if(pMediaPlayer == NULL)
		return TTKErrArgument;

	nErr = 	pMediaPlayer->handleVideoMsg(nID, nParam1, nParam2, pParam3);

	return nErr;
}

TTInt CTTMediaPlayer::handleVideoMsg (TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3)
{
	TTCAutoLock Lock(&mCriEvent);

	if(nID == ENotifyComplete) {
		if(mAudioSink == NULL || mAudioSink->isEOS()) {
			postMsgEvent(1, ENotifyComplete, mException,  nParam2, pParam3);
			setSeekStatus(false);
			mException = 0;
		}
		return TTKErrNone;
	} else if(nID == ENotifySeekComplete) {
		setSeekStatus(false);
		return TTKErrNone; 
	}

	return postMsgEvent(1, nID, nParam1, nParam2, pParam3);
}

TTInt CTTMediaPlayer::Decode(const TTChar* aUrl,TTInt size)
{
	return CTTPureDecodeEntity::Instance()->Decode(aUrl, size);
	//return 0;
}

TTUint8* CTTMediaPlayer::GetPCMData()
{
	return CTTPureDecodeEntity::Instance()->GetPCMData();
	//return NULL;
}

TTInt CTTMediaPlayer::GetPCMDataSize()
{
	return CTTPureDecodeEntity::Instance()->GetPCMDataSize();
	//return 0;
}

TTInt CTTMediaPlayer::GetPCMDataChannle()
{
	return CTTPureDecodeEntity::Instance()->GetChannels();
	//return 0;
}

TTInt CTTMediaPlayer::GetPCMDataSamplerate()
{
	return CTTPureDecodeEntity::Instance()->GetSamplerate();
	//return 0;
}

void CTTMediaPlayer::CancelGetPCM()
{
	CTTPureDecodeEntity::Instance()->Cancel();
}

void CTTMediaPlayer::SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos)
{
	CTTPureDecodeEntity::Instance()->SetTimeRange(start, end, decodeStartPos);
}
