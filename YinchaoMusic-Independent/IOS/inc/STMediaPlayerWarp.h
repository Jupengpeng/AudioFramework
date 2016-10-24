#ifndef __ST_MEDIA_PLAYER_WARP_H__
#define __ST_MEDIA_PLAYER_WARP_H__
#include "STMediaPlayerItf.h"

class STMediaPlayerWarp : public STMediaPlayerObserver
{
public:
    STMediaPlayerWarp(void* aMediaPlayerProxy);
    ~STMediaPlayerWarp();
    
    STInt Play();
    STInt Stop();
    void Pause();
    void Resume();
    void SetPosition(STUint aTime);  
    void SetPlayRange(STUint aStartTime, STUint aEndTime);
    STUint GetPosition();
    
    STUint Duration();
    STInt GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum);
    void GetEqualizerPresetArray(STPointerArray<STChar>& aArray);
    void SetEqualizerPresetIdx(STInt aIdx);

    STPlayStatus GetPlayStatus();
    
    STInt SetDataSourceAsync(const STChar* aUrl);    
    STInt SetDataSourceSync(const STChar* aUrl);
    
    void EnableEffect(STEffectType aEffect, STInt aParam1, STInt aParam2);
    void DisableEffect(STEffectType aEffect);
private:
    void PlayerNotifyEvent(STNotifyMsgId aMsg, STInt aError);
public:
    ISTMediaPlayerItf*  iMediaPlayer;   
    void*               iMediaPlayerProxy;
};

#endif