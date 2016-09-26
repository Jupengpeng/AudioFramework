#include "STMediaPlayerProxy.h"
#include "STMediaPlayer.h"

STMediaPlayerWarp::STMediaPlayerWarp(void* aMediaPlayerProxy)
{
    STASSERT(aMediaPlayerProxy != NULL);
    iMediaPlayerProxy = aMediaPlayerProxy;
    iMediaPlayer = new STMediaPlayer(*this);
}

STMediaPlayerWarp::~STMediaPlayerWarp()
{
    SAFE_RELEASE(iMediaPlayer);
}

void STMediaPlayerWarp::PlayerNotifyEvent(STNotifyMsgId aMsg, STInt aError)
{
    [((STMediaPlayerProxy*)iMediaPlayerProxy) ProcessNotifyEventWithMsg : aMsg andError : aError];
}

STInt STMediaPlayerWarp::SetDataSourceAsync(const STChar* aUrl)
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->SetDataSourceAsync(aUrl);
}

STInt STMediaPlayerWarp::SetDataSourceSync(const STChar* aUrl)
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->SetDataSource(aUrl);
}

STInt STMediaPlayerWarp::Play()
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Play();
}

STInt STMediaPlayerWarp::Stop()
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Stop();
}

void STMediaPlayerWarp::Pause()
{
    STASSERT(iMediaPlayer != NULL);
    iMediaPlayer->Pause();
}

void STMediaPlayerWarp::Resume()
{
    STASSERT(iMediaPlayer != NULL);
    iMediaPlayer->Resume();
}

void STMediaPlayerWarp::SetPosition(STUint aTime)
{
    STASSERT(iMediaPlayer != NULL);
    iMediaPlayer->Seek(aTime);
}

void STMediaPlayerWarp::SetPlayRange(STUint aStartTime, STUint aEndTime)
{
    STASSERT(iMediaPlayer != NULL);
    iMediaPlayer->SetPlayRange(aStartTime, aEndTime);
}

STUint STMediaPlayerWarp::GetPosition()
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetPosition();
}

STUint STMediaPlayerWarp::Duration()
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Duration();
}

STInt STMediaPlayerWarp::GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum)
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetCurrentFreqAndWave(aFreq, aWave, aSampleNum);
}

void STMediaPlayerWarp::GetEqualizerPresetArray(STPointerArray<STChar>& aArray)
{
    iMediaPlayer->GetEqualizerPresetArray(aArray);
}

void STMediaPlayerWarp::SetEqualizerPresetIdx(STInt aIdx)
{
    iMediaPlayer->SetEqualizerPresetIdx(aIdx);
}

void STMediaPlayerWarp::EnableEffect(STEffectType aEffect, STInt aParam1, STInt aParam2)
{
    iMediaPlayer->EnableEffect(aEffect, aParam1, aParam2);
}

void STMediaPlayerWarp::DisableEffect(STEffectType aEffect)
{
    iMediaPlayer->DisableEffect(aEffect);
}

STPlayStatus STMediaPlayerWarp::GetPlayStatus()
{
    STASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetPlayStatus();
}

