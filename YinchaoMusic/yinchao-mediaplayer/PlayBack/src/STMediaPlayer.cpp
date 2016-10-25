#include "STMediaPlayer.h"
#include "STMacrodef.h"
#include "STOSConfig.h"
#include "STFFT.h"
#include "STOSConfig.h"
#include "STParamKeys.h"
#include "STLog.h"
#ifdef __ST_OS_IOS__
#include "STAutoReleasePool.h"
#elif defined __ST_OS_ANDROID__
#include "STRunTime.h"
#endif

STMediaPlayer::STMediaPlayer(ISTMediaPlayerObserver& aMediaPlayerObserver)
	:iUrl(NULL)
	,iParams(NULL)
	,iPlayControl(NULL)
	,iPlayerObserver(aMediaPlayerObserver)
{
#ifndef __ST_OS_ANDROID__
    STThread::InitContext();
#endif
	iCritical.Create();
	iWrokThread.Create(PlayThreadProc, this);
	iMsgQueue.Init();
}

STMediaPlayer::~STMediaPlayer()
{
	STMsg* pMsg = new STMsg(ECmdMsgIdDestory, NULL, NULL);
	iMsgQueue.SendMsg(*pMsg);

	iWrokThread.Close();
	iCritical.Destroy();
	iMsgQueue.Close();
	SAFE_FREE(iUrl);
	SAFE_FREE(iParams);
}

STUint STMediaPlayer::Duration()
{
	STUint nDuration = 0;
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		STPlayStatus tStatus = iPlayControl->GetPlayStatus();
		if (tStatus == EStatusPrepared || tStatus == EStatusPaused || tStatus == EStatusPlaying)
		{
			nDuration = iPlayControl->Duration();
		}
	}
	iCritical.UnLock();	

	return nDuration;
}

STInt STMediaPlayer::SetDataSource(const STChar* aUrl, const STChar* aParams)
{
	return SetDataSource(aUrl, aParams, ESTTrue);
}

STInt STMediaPlayer::SetDataSourceAsync(const STChar* aUrl, const STChar* aParams)
{
	return SetDataSource(aUrl, aParams, ESTFalse);
}

STInt STMediaPlayer::SetDataSource(const STChar* aUrl, const STChar* aParams, STBool aSync)
{
    STInt nLen = strlen(aUrl);

    STLOGI("SetDataSource: url: %s", aUrl);

    if(NULL != aParams)
    {
        nLen += strlen(aParams);

        STLOGI("SetDataSource: param: %s", aParams);
    }
    STASSERT(nLen < KMaxPathLength);

    STInt nErr = Stop();
	if(STKErrNone == nErr)
    {
	    SAFE_FREE(iUrl);
	    iUrl = (STChar*) malloc((strlen(aUrl) + 1) * sizeof(STChar));
	    strcpy(iUrl, aUrl);

	    SAFE_FREE(iParams);
	    if (aParams != NULL)
	    {	
		    iParams = (STChar*)malloc((strlen(aParams) + 1) * sizeof(STChar));
		    strcpy(iParams, aParams);
	    }

	    iCritical.Lock();
	    STASSERT(iPlayControl != NULL);
	    iPlayControl->SetPlayStatus(EStatusPreparing);
	    iCritical.UnLock();

	    if (aSync)
	    {
		    STMsg* pMsg =  new STMsg(ECmdMsgIdOpen, (void*)iUrl, (void*)(&nErr), iParams);
		    iMsgQueue.SendMsg(*pMsg);
	    }
	    else
	    {
		    STMsg* pMsg =  new STMsg(ECmdMsgIdOpen, (void*)iUrl, NULL, iParams);
		    iMsgQueue.PostMsg(*pMsg);
	    }		
    }

	return nErr;
}

STInt STMediaPlayer::Play()
{
	if (EStatusPrepared == GetPlayStatus())
	{
		STMsg* pMsg =  new STMsg(ECmdMsgIdStart, NULL, NULL);
		iMsgQueue.SendMsg(*pMsg);
		return STKErrNone;
	}

	return STKErrNotReady;
}

STInt STMediaPlayer::Stop()
{
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		STInt tStatus = iPlayControl->GetPlayStatus();
		if (tStatus == EStatusPreparing)
		{
			iCritical.UnLock();
			return STKErrNotReady;
		}
		else if (EStatusStoped != tStatus)
		{
			iPlayControl->Abort();
			iCritical.UnLock();
			STMsg* pMsg =  new STMsg(ECmdMsgIdStop, NULL, NULL);
			iMsgQueue.SendMsg(*pMsg);
			return STKErrNone;
		}
	}
	iCritical.UnLock();

	return STKErrNone;
}

void STMediaPlayer::Pause()
{
	STMsg* pMsg = new STMsg(ECmdMsgIdPause, NULL, NULL);
	iMsgQueue.PostMsg(*pMsg);
}

void STMediaPlayer::Resume()
{
	STMsg* pMsg = new STMsg(ECmdMsgIdResume, NULL, NULL);
	iMsgQueue.PostMsg(*pMsg);
}

STPlayStatus STMediaPlayer::GetPlayStatus()
{
	STPlayStatus tStatus = EStatusStoped;
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		tStatus = iPlayControl->GetPlayStatus();
	}
	iCritical.UnLock();

	return tStatus;
}

void STMediaPlayer::Seek(STUint aPosition)
{
	STMsg* pMsg = new STMsg(ECmdMsgIdSeek, (void*)(aPosition), NULL);
	iMsgQueue.SendMsg(*pMsg);
}

STUint STMediaPlayer::GetPosition()
{
	STUint nPos = 0;
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		nPos = iPlayControl->Position();
	}
	iCritical.UnLock();
	return nPos;
}

//只能同步操作， streamName 没有缓存
STInt STMediaPlayer::Switch2Stream(const STChar* aStreamName)
{
	STInt nErr = STKErrNone;
	STMsg* pMsg = new STMsg(ECmdMsgIdSwitchStream, (void*)(aStreamName), (void*)(&nErr));
	iMsgQueue.SendMsg(*pMsg);

	return nErr;
}

const STChar* STMediaPlayer::GetCurStreamName()
{
	const STChar* pCurStreamName = NULL;
	STMsg* pMsg = new STMsg(ECmdMsgIdGetCurStreamName, (void*)(&pCurStreamName), NULL);
	iMsgQueue.SendMsg(*pMsg);
	return pCurStreamName;
}

STInt STMediaPlayer::GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels)
{
	STASSERT((aSampleNum >= KMinWaveFreqSampleNum) && (aSampleNum <= KMaxWaveFreqSampleNum) && (aWave != NULL) && (aSampleNum & 4) == 0);

	STInt nErr = STKErrAccessDenied;
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		nErr = iPlayControl->GetCurWave(aWave, aSampleNum, aChannels);
	}
	iCritical.UnLock();

	if ((STKErrNone == nErr) && (aFreq != NULL))
	{
		STFFT::DoFFT(aFreq, aWave, aChannels, aSampleNum);
	}

	return nErr;
}

STInt STMediaPlayer::GetCurRecordWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels)
{
	STInt nErr = STKErrAccessDenied;
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		nErr = iPlayControl->GetCurRecordWave(aWave, aSampleNum, aChannels);
	}
	iCritical.UnLock();

	if ((STKErrNone == nErr) && (aFreq != NULL))
	{
		STFFT::DoFFT(aFreq, aWave, aChannels, aSampleNum);
	}

	return nErr;
}

void* STMediaPlayer::PlayThreadProc(void* aPtr)
{
#ifdef __ST_OS_ANDROID__
    STRunTime::AttachCurrentThread();
#endif
    
	STMediaPlayer* pPlayer = reinterpret_cast<STMediaPlayer*>(aPtr);
	pPlayer->PlayThreadProcL(pPlayer);
    
#ifdef __ST_OS_ANDROID__
    STRunTime::DetachCurrentThread();
#endif
    return NULL;
}

void STMediaPlayer::SetAccompanimentVolume(STInt aVolume)
{
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		iPlayControl->SetAccompanimentVolume(aVolume);
	}
	iCritical.UnLock();
}

void STMediaPlayer::SetRecorderVolume(STInt aVolume)
{
	iCritical.Lock();
	if (iPlayControl != NULL)
	{
		iPlayControl->SetRecorderVolume(aVolume);
	}
	iCritical.UnLock();
}

void STMediaPlayer::PlayThreadProcL(STMediaPlayer* aMediaPlayer)
{
	STASSERT(aMediaPlayer != NULL);

	iCritical.Lock();
	iPlayControl = new STPlayControl(*aMediaPlayer);
	iCritical.UnLock();
	iMsgQueue.SetReciver(&iWrokThread, iPlayControl);

	while (ESTTrue)
	{
		iMsgQueue.HandleMsg();	

		if (iPlayControl->IsToDestory())
			break;

		if (iPlayControl->IsAllJobToDone()) //播放逻辑
		{
            STThread::Self()->LockContext();
            if (iMsgQueue.IsAllMsgHandled()) 
            {
                STThread::Self()->Suspend();
            }
			STThread::Self()->UnlockContext();
		}
	}

	iCritical.Lock();
	SAFE_DELETE(iPlayControl);
	iCritical.UnLock();
}

void STMediaPlayer::PrepareComplete()
{
	STLOGE("MediaPlayer:%d", this);
	iPlayerObserver.PlayerNotifyEvent(ENotifyPrepared, this, STKErrNone);
}

void STMediaPlayer::StartComplete()
{
	iPlayerObserver.PlayerNotifyEvent(ENotifyStarted, this, STKErrNone);
}

void STMediaPlayer::PlayComplete()
{
	iPlayerObserver.PlayerNotifyEvent(ENotifyCompleted, this, STKErrNone);
}

void STMediaPlayer::PauseComplete()
{
	iPlayerObserver.PlayerNotifyEvent(ENotifyPaused, this, STKErrNone);
}

void STMediaPlayer::ResumeComplete() 
{
    iPlayerObserver.PlayerNotifyEvent(ENotifyResumed, this, STKErrNone);
}

void STMediaPlayer::PlayException(STInt aError)
{
	iPlayerObserver.PlayerNotifyEvent(ENotifyException, this, aError);
}

void STMediaPlayer::RecordFinished(STInt aDuration)
{
	iPlayerObserver.PlayerNotifyEvent(ENotifyRecordFinished, this, aDuration);
}

STInt STMediaPlayer::setAudioImpulsePath(const STChar* path,int flag)
{
	STInt ret;
	iCritical.Lock();
	ret = iPlayControl->setAudioImpulsePath(path,flag);
	iCritical.UnLock();
	
	return  ret;
}


STInt STMediaPlayer::GetDownloadPercent()
{
	STInt pos = -1;
	iCritical.Lock();
	if(iPlayControl != NULL)
	{
		pos = iPlayControl->GetDownloadPercent();
	}
	iCritical.UnLock();

	return  pos;
}



