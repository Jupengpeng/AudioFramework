#include <string.h>
#include "STLog.h"
#include "STPlayControl.h"
#include "STAudioOutput.h"
#include "STMediaInfo.h"
#include "STOSConfig.h"
#include "STSampleBufferManager.h"
#include "STThread.h"
#include "STMediaPlayerItf.h"
#include "STParamKeys.h"
#include "STUrlUtils.h"
#include "STBufferConfig.h"

STPlayControl::STPlayControl(ISTPlayControlObserver& aObserver)
	: iPlayControlObserver(aObserver)
	, iIsToDestory(ESTFalse)
	, iPlayStatus(EStatusStoped)
	, iCurPlugin(NULL)
{
	iCritical.Create();
	iPluginManager = new STPluginManager();
	iAudioOutput = new STAudioOutput();
	iSampleBufferManager = new STSampleBufferManager(KTotalPCMBufferSize);
}

STPlayControl::~STPlayControl()
{
	SAFE_DELETE(iSampleBufferManager);
	SAFE_RELEASE(iAudioOutput);
	SAFE_RELEASE(iCurPlugin);
	SAFE_DELETE(iPluginManager);
	iCritical.Destroy();
}

void STPlayControl::HandleMsg(STMsg& aMsg)
{
	switch(aMsg.iMsgId) 
	{
	case ECmdMsgIdDestory:
		iIsToDestory = ESTTrue;
		break;

	case ECmdMsgIdOpen:
		{
			STInt nErr = Open((STChar*)aMsg.iMessage, (STChar*)aMsg.iParam);

			if (aMsg.iWhat != NULL)
			{
				*((STInt*)(aMsg.iWhat)) = nErr;
			}
			else
			{
				if (STKErrNone == nErr)
				{
					iPlayControlObserver.PrepareComplete();
				}
				else
				{
					iPlayControlObserver.PlayException(nErr);
				}
			}
		}
		break;

	case ECmdMsgIdPause:
		Pause();
		break;

	case ECmdMsgIdResume:
		Resume();
		break;

	case ECmdMsgIdStart:
		Start();
		break;

	case ECmdMsgIdStop:
		Stop();
		break;

	case ECmdMsgIdSeek:
		Seek((STUint)(aMsg.iMessage));
		break;

	case ECmdMsgIdSwitchStream:
		{
			STInt nErr = STKErrNotSupported;
			if (iCurPlugin == NULL)
			{
				nErr = STKErrAccessDenied;
			} 
			else 
			{
				nErr = Switch2Stream((STChar *)(aMsg.iMessage));
			}

			*((STInt*)(aMsg.iWhat)) = nErr;
		}
		
		break;

	case ECmdMsgIdGetCurStreamName:
		{
			if (iCurPlugin != NULL)
			{
				*(const STChar **)(aMsg.iMessage) = iCurPlugin->GetStreamName(iAudioOutput->GetCurSteramIndex());
			}
		}
		break;
               
	default:
		STASSERT(ESTFalse);
		break;
	}
}

STBool STPlayControl::IsAllJobToDone()
{
	if (iCurPlugin != NULL)
	{
		STSampleBuffer* pBuffer = iAudioOutput->RecycleBuffer();
		while(pBuffer != NULL)
		{
			iSampleBufferManager->RecycleFreeBuffer(pBuffer);
			pBuffer = iAudioOutput->RecycleBuffer();
		}

		STReadStatus tReadStatus = iCurPlugin->ReadStatus();
		if (ESTReadStatusReading == tReadStatus)
		{
			pBuffer = iSampleBufferManager->GetFreeBuffer();
			if (pBuffer != NULL)
			{			
				STInt nErr = iCurPlugin->Read(pBuffer);
				if ((STKErrNone == nErr) || (STKErrEof == nErr))
				{
					iAudioOutput->RenderBuffer(pBuffer);
				}
				else 
				{
				    iSampleBufferManager->RecycleFreeBuffer(pBuffer);
				    Stop();
				    iPlayControlObserver.PlayException(nErr);
				}
                
				return ESTFalse;
			}
		}	
		else if (ESTReadStatusComplete == tReadStatus)
		{
			if (iAudioOutput->IsUnderflow())
			{
				Stop();
				iPlayControlObserver.PlayComplete();				
			}
			return ESTFalse;
		} 
	}

	return ESTTrue;
}

STBool STPlayControl::IsToDestory()
{
	return iIsToDestory;
}

STPlayStatus STPlayControl::GetPlayStatus()
{
	STPlayStatus tStatus = EStatusStoped;
	iCritical.Lock();
	tStatus = iPlayStatus;
	iCritical.UnLock();
	return tStatus;
}

void STPlayControl::SetPlayStatus(STPlayStatus aStatus)
{
	iCritical.Lock();
	iPlayStatus = aStatus;
	iCritical.UnLock();
}

STInt STPlayControl::Open(const STChar* aUrl, const STChar* aParams)
{
	iCritical.Lock();
	iCurPlugin = iPluginManager->SelectPlugin(aUrl, aParams);
	iCritical.UnLock();

	if (iCurPlugin == NULL)
	{
		SetPlayStatus(EStatusStoped);
		return STKErrNotSupported;
	}

	STInt nErr = iCurPlugin->InitPlugin(aUrl, aParams);
	if (STKErrNone != nErr)
	{
        SetPlayStatus(EStatusStoped);
		iCurPlugin->UnInitPlugin();
		SAFE_RELEASE(iCurPlugin);
		return nErr;
	}

	if(STKErrNone != STUrlUtils::GetParam(aParams, ParamKeyRecordSaveName, iTempBuffer, ParamKeySeparator, KMaxPathLength - 1))
	{
		memset(iTempFullPathBuffer, 0, sizeof(iTempFullPathBuffer));
	}
	else
	{
		sprintf(iTempFullPathBuffer, "%s/%s", aUrl, iTempBuffer);
	}

	STLOGI("PARAM_VALUE_SUPPORT_RECORD:%s", iTempFullPathBuffer);

	const STMediaInfo& tMediaInfo = iCurPlugin->GetMediaInfo();
	const STPointerArray<STAudioInfo>& tAudioInfoArray = tMediaInfo.GetAudioStreamArray();
	nErr = iAudioOutput->Open(tMediaInfo.GetAudioStreamArray()[0]->GetSampleRate(), tMediaInfo.GetAudioStreamArray()[0]->GetChannels(), iTempFullPathBuffer);
	if (nErr == STKErrNone)
	{
		iSampleBufferManager->Assign(tMediaInfo.GetAudioStreamArray().Count(), KFirstPCMBufferSize, KPCMBufferSize);
		SetPlayStatus(EStatusPrepared);
	}
	else
	{
		SAFE_RELEASE(iCurPlugin);
		SetPlayStatus(EStatusStoped);
	}	

	return nErr;
}

void STPlayControl::Close()
{
	iAudioOutput->Close();
	if (iCurPlugin != NULL)
	{
		iCurPlugin->UnInitPlugin();
	}
	SAFE_RELEASE(iCurPlugin);
}

void STPlayControl::Start()
{
	STInt nErr = STKErrNone;
	if (STKErrNone == (nErr = iCurPlugin->StartReading()))
	{
		if (STKErrNone == FillAudioOutput())
		{
			iAudioOutput->Start();
			SetPlayStatus(EStatusPlaying);
            iPlayControlObserver.StartComplete();
		}
	}
	else
	{
		Stop();
		iPlayControlObserver.PlayException(nErr);
	}
}

void STPlayControl::Stop()
{
	STPlayStatus tStatus = GetPlayStatus();		
	SetPlayStatus(EStatusStoped);
	switch (tStatus)
	{
	case EStatusPlaying:
	case EStatusPaused:
		{
			iCurPlugin->Abort();
			iAudioOutput->Stop();
#ifdef __SUPPORT_RECORD__
			if (iAudioOutput->IsRecorderExisted())
			{
				iPlayControlObserver.RecordFinished(iAudioOutput->GetRecordDuration());
			}
#endif
			Flush();
			Close();			
		}
		break;

	case EStatusPrepared:
		{
			iCurPlugin->Abort();
			Close();
		}
		break;

	case EStatusPreparing:
		STASSERT(ESTFalse);
		break;

	default:
		break;
	}	
}

void STPlayControl::Abort()
{
	STLOGE("STPlayControl::Abort");
	STASSERT(iCurPlugin != NULL);
	iCurPlugin->Abort();
}

void STPlayControl::Flush()
{
	iAudioOutput->Flush();

	STSampleBuffer* pBuffer = iAudioOutput->RecycleBuffer(ESTTrue);
	while (pBuffer != NULL)
	{
		iSampleBufferManager->RecycleFreeBuffer(pBuffer);
		pBuffer = iAudioOutput->RecycleBuffer(ESTTrue);
	}
}

void STPlayControl::Pause()
{
	if (EStatusPlaying == GetPlayStatus())
	{
		iAudioOutput->Pause();
		SetPlayStatus(EStatusPaused);
        iPlayControlObserver.PauseComplete();
	}
}

void STPlayControl::Resume()
{
	if (EStatusPaused == GetPlayStatus())
	{
		iAudioOutput->Resume();
		SetPlayStatus(EStatusPlaying);
        iPlayControlObserver.ResumeComplete();
	}	
}

STUint STPlayControl::Duration()
{
	return iCurPlugin->Duration();
}

STUint STPlayControl::Position()
{
	STUint nPos = 0;
	iCritical.Lock();	
	if (iAudioOutput != NULL)
	{
		iAudioOutput->Position(nPos);
	}
	iCritical.UnLock();

	return nPos;
}

STInt STPlayControl::GetDownloadPercent()
{
	STInt nPos = -1;
	iCritical.Lock();
	if (iCurPlugin != NULL)
	{
		nPos = iCurPlugin->GetDownloadPercent();
		STLOGE("STPlayControl::GetDownloadPercent pos_1= %d",nPos);
	}
	STLOGE("STPlayControl::GetDownloadPercent pos_2= %d",nPos);
	iCritical.UnLock();

	return nPos;
}

void STPlayControl::SetAccompanimentVolume(STInt aVolume)
{
	iCritical.Lock();
	if (iAudioOutput != NULL)
	{
		iAudioOutput->SetAccompanimentVolume(aVolume);
	}
	iCritical.UnLock();
}

void STPlayControl::SetRecorderVolume(STInt aVolume)
{
	iCritical.Lock();
	if (iAudioOutput != NULL)
	{
		iAudioOutput->SetRecorderVolume(aVolume);
	}
	iCritical.UnLock();
}

void STPlayControl::Seek(STUint aPosition)
{
    STUint nPos = aPosition;
    switch (GetPlayStatus())
	{
	case EStatusPlaying: 
		{	
			STLOGE("STPlayControl::Seek nPos = %d,iCurPlugin=%p",nPos,iCurPlugin);
			iAudioOutput->Pause();			
			Flush();
            iCurPlugin->Seek(nPos);
			iAudioOutput->SyncPosition(nPos);

			STLOGE("STPlayControl::Seek iCurPlugin = %p",iCurPlugin);
			
			STInt ret= STKErrNone;
			ret = FillAudioOutput();

			if (ret == STKErrNone /*|| ret == STKErrNotReady */)
			{
				iAudioOutput->Resume();
				STLOGE("STPlayControl::Seek iAudioOutput->Resume()");
			}
			STLOGE("STPlayControl::Seek iAudioOutput->noResume() ,FillAudioOutput() = %d",ret);
		}
		break;

	case EStatusPrepared:
		{
            iCurPlugin->Seek(nPos);
			iAudioOutput->SyncPosition(nPos);
		}
		break;

	case EStatusPaused:
		{
			Flush();
            iCurPlugin->Seek(nPos);
			iAudioOutput->SyncPosition(nPos);
			FillAudioOutput();
		}
		break;

	default:		
		break;
	}
}

STInt STPlayControl::FillAudioOutput()
{
	STInt nErr = STKErrNone;
	const STMediaInfo& tMediaInfo = iCurPlugin->GetMediaInfo();
	const STPointerArray<STAudioInfo>& tAudioInfoArray = tMediaInfo.GetAudioStreamArray();
	for (STInt i = 0; i < tAudioInfoArray.Count(); i++)
	{
		STSampleBuffer* pBuffer = iSampleBufferManager->GetFirstBuffer();
		STASSERT(pBuffer != NULL);
		if (STKErrNone == (nErr = iCurPlugin->Read(pBuffer)))
		{
			iAudioOutput->RenderBuffer(pBuffer);
		}
		else
		{
			iSampleBufferManager->RecycleFreeBuffer(pBuffer);
			Stop();
			if (STKErrEof == nErr)
			{
				iPlayControlObserver.PlayComplete();
			}
			else
			{
				iPlayControlObserver.PlayException(nErr);
			}
		}
	}

	return nErr;
}

STInt STPlayControl::GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
	STInt nErr = STKErrAccessDenied;
	iCritical.Lock();
	if (iAudioOutput != NULL)
	{
		nErr = iAudioOutput->GetCurWave(aWave,aSamples, aChannels);
	}
	iCritical.UnLock();
	return nErr;
}

STInt STPlayControl::GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
	STInt nErr = STKErrAccessDenied;
	iCritical.Lock();
	if (iAudioOutput != NULL)
	{
		nErr = iAudioOutput->GetCurRecordWave(aWave,aSamples, aChannels);
	}
	iCritical.UnLock();
	return nErr;
}

STInt STPlayControl::Switch2Stream(const STChar *aStreamName)
{
	if(NULL != iCurPlugin && NULL != aStreamName)
	{
		STInt nRet = iCurPlugin->Switch2Stream(aStreamName);
		return (nRet >= 0) ? iAudioOutput->Switch2Stream(nRet) : nRet;
	}
	
	return  STKErrArgument;
}

//add by bin.chen
STInt STPlayControl::setAudioImpulsePath(const STChar* path,int flag)
{
	#ifdef __SUPPORT_RECORD__
	STLOGE("STPlayControl::setAudioImpulsePath:%s,flag=%d", path,flag);
	STLOGE("STPlayControl::setAudioImpulsePath:iAudioOutput=%d",iAudioOutput);
	if(iAudioOutput !=NULL)
	{
		STLOGE("STPlayControl::setAudioImpulsePath:iAudioOutput=yes");
		return ((STAudioOutput*)iAudioOutput)->setAudioImpulsePath(path,flag);
	}

	#endif
	return STKErrNotSupported;
}


