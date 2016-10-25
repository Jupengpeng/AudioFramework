/**
* File : TTMediaPlayerWarp.cpp
* Created on : 2011-9-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTMediaPlayerWarp µœ÷
*/

#include "TTMediaPlayerProxy.h"
#include "TTLog.h"

extern void ConfigProxyServer(TTUint32 aIP, TTInt Port, const TTChar* aAuthen, TTBool aUseProxy);

CTTMediaPlayerWarp::CTTMediaPlayerWarp(void* aMediaPlayerProxy)
{
    TTASSERT(aMediaPlayerProxy != NULL);
    iMediaPlayerProxy = aMediaPlayerProxy;
    iMediaPlayer = CTTMediaPlayerFactory::NewL(this);
}

CTTMediaPlayerWarp::~CTTMediaPlayerWarp()
{
    SAFE_RELEASE(iMediaPlayer);
}

void CTTMediaPlayerWarp::PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3)
{
    LOGE("PlayerNotifyEvent aMsg: %d",  aMsg);
    [((TTMediaPlayerProxy*)iMediaPlayerProxy) ProcessNotifyEventWithMsg : aMsg andError : aArg2];
}

TTInt CTTMediaPlayerWarp::SetDataSourceAsync(const TTChar* aUrl)
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->SetDataSourceAsync(aUrl);
}

TTInt CTTMediaPlayerWarp::SetDataSourceSync(const TTChar* aUrl)
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->SetDataSourceSync(aUrl);
}

TTInt CTTMediaPlayerWarp::Play()
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Play();
}

TTInt CTTMediaPlayerWarp::Stop()
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Stop();
}

void CTTMediaPlayerWarp::Pause()
{
    TTASSERT(iMediaPlayer != NULL);
    iMediaPlayer->Pause();
}

void CTTMediaPlayerWarp::Resume()
{
    TTASSERT(iMediaPlayer != NULL);
    iMediaPlayer->Resume();
}

void CTTMediaPlayerWarp::SetPosition(TTUint aTime, TTInt aOption)
{
    TTASSERT(iMediaPlayer != NULL);
    iMediaPlayer->SetPosition(aTime, aOption);
}

void CTTMediaPlayerWarp::SetPlayRange(TTUint aStartTime, TTUint aEndTime)
{
    TTASSERT(iMediaPlayer != NULL);
    iMediaPlayer->SetPlayRange(aStartTime, aEndTime);
}

TTUint CTTMediaPlayerWarp::GetPosition()
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetPosition();
}

TTUint CTTMediaPlayerWarp::Duration()
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->Duration();
}

TTInt CTTMediaPlayerWarp::GetCurFreqAndWave(TTInt16 *aFreq, TTInt16 *aWave, TTInt aSampleNum)
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetCurrentFreqAndWave(aFreq, aWave, aSampleNum);
}

TTPlayStatus CTTMediaPlayerWarp::GetPlayStatus()
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->GetPlayStatus();
}

void CTTMediaPlayerWarp::SetPowerDown()
{
}

TTInt CTTMediaPlayerWarp::BufferedPercent()
{
    TTInt nErr = TTKErrNotReady;
    TTInt nBufferPercent = 0;
    if (iMediaPlayer != NULL)
    {
        if (TTKErrNone == iMediaPlayer->BufferedPercent(nBufferPercent)) 
        {
            return nBufferPercent;
        } 
    }
    
    return nErr;
}

TTUint CTTMediaPlayerWarp::fileSize()
{
    TTUint fileSize = 0;
    if (iMediaPlayer != NULL) {
        fileSize = iMediaPlayer->Size();
    }
    
    return fileSize;
}

TTUint CTTMediaPlayerWarp::bufferedFileSize()
{
    TTUint bufferedFileSize = 0;
    if (iMediaPlayer != NULL) {
        bufferedFileSize = iMediaPlayer->BufferedSize();
    }
    
    return bufferedFileSize;
}

void CTTMediaPlayerWarp::SetActiveNetWorkType(TTActiveNetWorkType aType)
{
    TTASSERT(iMediaPlayer != NULL);
    return iMediaPlayer->SetActiveNetWorkType(aType);
}

void CTTMediaPlayerWarp::SetCacheFilePath(const TTChar *aCacheFilePath)
{
    return iMediaPlayer->SetCacheFilePath(aCacheFilePath);
}

void CTTMediaPlayerWarp::SetBalanceChannel(float aVolume)
{
    if (iMediaPlayer != NULL)
		iMediaPlayer->SetBalanceChannel(aVolume);
}

void CTTMediaPlayerWarp::SetProxyServerConfig(TTUint32 aIP, TTInt aPort, const TTChar* aAuthen, TTBool aUseProxy)
{
    ConfigProxyServer(aIP, aPort, aAuthen, aUseProxy);
}

void CTTMediaPlayerWarp::SetView(void* aView)
{
    if (iMediaPlayer != NULL)
        iMediaPlayer->SetView(aView);
}

TTInt CTTMediaPlayerWarp::BandWidth()
{
    if (iMediaPlayer != NULL) {
        return iMediaPlayer->BandWidth();
    }
    return 0;
}

void CTTMediaPlayerWarp::SetRotate()
{
    if (iMediaPlayer != NULL) {
        return iMediaPlayer->SetRotate();
    }
}