/**
* File : TTMediaPlayerWarp.h 
* Created on : 2011-9-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTMediaPlayerWarp 定义文件
*/
#ifndef __TT_MEDIA_PLAYER_WARP__
#define __TT_MEDIA_PLAYER_WARP__
#include "TTMediaPlayerItf.h"

class CTTMediaPlayerWarp : public ITTMediaPlayerObserver
{
public:
    CTTMediaPlayerWarp(void* aMediaPlayerProxy);
    ~CTTMediaPlayerWarp();
//    void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aError);
    void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3);
    TTInt Play();
    TTInt Stop();
    void Pause();
    void Resume();
    void SetPosition(TTUint aTime, TTInt aOption = 0);
    void SetPlayRange(TTUint aStartTime, TTUint aEndTime);
    TTUint GetPosition();
    
    TTUint Duration();
    TTInt GetCurFreqAndWave(TTInt16 *aFreq, TTInt16 *aWave, TTInt aSampleNum);
    TTPlayStatus GetPlayStatus();
    
    TTInt SetDataSourceAsync(const TTChar* aUrl);    
    TTInt SetDataSourceSync(const TTChar* aUrl);
    
    void SetPowerDown();
    void SetBalanceChannel(float aVolume);
    
    TTInt BufferedPercent();
    TTUint fileSize();
    TTUint bufferedFileSize();
    
    void SetActiveNetWorkType(TTActiveNetWorkType aType);
    void SetCacheFilePath(const TTChar *aCacheFilePath);
    
    void SetProxyServerConfig(TTUint32 aIP, TTInt aPort, const TTChar* aAuthen, TTBool aUseProxy);
    
    void SetView(void* aView);
    
    TTInt BandWidth();
    
    void SetRotate();
    
    
public:
    ITTMediaPlayer* iMediaPlayer;   
    void*           iMediaPlayerProxy;
};

#endif