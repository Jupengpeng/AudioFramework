/**
* File : TTAudioSink.h 
* Created on : 2011-9-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioSink 定义文件
*/
#ifndef __TT_AUDIO_SINK_H__
#define __TT_AUDIO_SINK_H__

// INCLUDES
#include "TTOsalConfig.h"
#include "TTCritical.h"
#include "TThread.h"
#include "TTArray.h"
#include "TTSemaphore.h"
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include "TTBaseAudioSink.h"

class CTTMediaBuffer;

// CLASSES DECLEARATION
class CTTAudioSink : public TTCBaseAudioSink
{
public:
	/**
	* \fn							CTTAudioSink(ITTSinkDataProvider* aDataProvider, ITTPlayRangeObserver& aObserver);
	* \brief						构造函数
	* \param[in] aDataProvider		数据提供者接口引用
    * \param[in] aObserver			播放范围回调	
	*/
	CTTAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	/**
	* \fn							~CTTAudioSink();
	* \brief						析构函数
	*/
	virtual ~CTTAudioSink();


public://from ITTDataSink

	/**
	* \fn                       void Render(CTTMediaBuffer* aBuffer);
	* \brief                    提交数据
	* \param[in] aBuffer		数据指针
	* \return					返回状态
	*/
	virtual TTInt			render();

	/**
	* \fn                       TTInt Open(TTAudioDataSettings& aAudioDataSetting)
	* \brief                    打开设备
	* \param[in] aAudioDataSetting     音频信息
	* \return					返回操作状态
	*/
	

	/**
	* \fn                       void Close()
	* \brief                    关闭设备
	*/
	virtual TTInt           close();

	/**
	* \fn						void GetCurWave(TTInt aSamples, TTInt16** aWave, TTInt& nChannels);
	* \brief					获取当前播放的数据
	* \param[in]  aSamples		取采样的个数	
	* \param[out] aWave			采样点的数据指针	
	* \param[out] nChannels		音频声道数	
	*/
	//virtual void				GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels);

    /**
     * \fn						void SetVolume(TTInt aVolume)
     * \brief					设置音量
     * \param[in] aLVolume		音量值	
     * \param[in] aRVolume		音量值	
     */
	virtual void				SetVolume(TTInt aLVolume, TTInt aRVolume);
    
    /**	
     * \fn						void Flush();
     * \brief					清空缓冲中的数据
     */
	virtual TTInt				flush();
    
    /**
     * \fn						void Position(TTUint& aPosition)
     * \brief					获取播放位置
     * \param[out] aPosition	播放位置，单位毫秒
     */
	//virtual void				Position(TTUint& aPosition);
    
    /**
     * \fn						void SetBalanceChannel(float aVolume)
     * \brief                          set balane channel volume
     * \param[in] aVolume  ,value range: [-1,+1]
     */
    virtual void                SetBalanceChannel(float aVolume);
    
    virtual TTInt               syncPosition(TTUint64 aPosition, TTInt aOption);
    
    virtual TTInt				newAudioTrack();
    virtual TTInt				freeAudioTrack();

public://from ITTSyncClock

	/**
     * \fn						void Pause()
     * \brief					暂停送数据
     */
	virtual	TTInt				pause(TTBool aFadeOut);
    
    /**
     * \fn						TTInt Resume()
     * \brief					暂停后，继续送数据
     * \return                  TTKErrNone respresents success, other value respresents fail.
     */
	virtual	TTInt				resume(TTBool aWait = false,TTBool aFadeIn= false);
    
	/**
	* \fn						void Stop()
	* \brief					停止送数据
	*/
	virtual	TTInt				stop();
    
    static  void                setIOSVersion();
    
    
private://有CTTActive继承
	 
    void                FlushData();
   
    
    TTInt                       Open(TTInt SampleRate, TTInt Channels);
    
    void                writeDataToAU(TTUint8* pDstDataPtr, TTInt size);

private:
    static OSStatus             AudioUnitCallBack(void *inRefCon, AudioUnitRenderActionFlags*
                                                  ioActionFlags, const AudioTimeStamp*inTimeStamp, UInt32 
                                                  inBusNumber, UInt32 inNumberFrames, AudioBufferList *
                                                  ioData);
    void                        AudioUnitCallBackProcL(AudioBufferList *
                                                       ioData);
    
    TTInt                       AudioUnitStart();
    void                        AudioUnitStop();
    
    void                        SetRealVolume(float aVolume);
    void                        HandleNoise();
    
    void                        WriteData(TTBuffer* aBuffer);
    
private:
    AudioStreamBasicDescription         iAudioStreamDataFormat; 
    //RTTPointerArray<CTTMediaBuffer>     iEmptyBufferArray;
   // RTTPointerArray<CTTMediaBuffer>     iFilledBufferArray;
    
    AUGraph                             iAudioGraph;
    AudioUnit                            iOutputUnit;
    AudioUnit                            iMixerUnit;
    TTBool                              iStreamStarted;
    TTInt                               iCurEffectProcessPos;
    TTInt                               iCurEffectProcessAlignBits;
    RTTSemaphore                        iSemaphore;
    TTBool                              iAudioCallBackProcRun;
    float                               iBalanceVolume;
    TTInt                               iHeadDataBlockSize;
    TTBool                              iIsRestoreVolume;
    TTInt                               iPauseResumeFlg;
    TTInt                               iEnlargedVolume;
    
    TTInt									mListFull;
    TTInt									mListUsing;
    TTBool									mIOSFlushing;
    TTBool									mIOSSeeking;
    TTBool									mIosEOS;
    TTBuffer**								mListBuffer;
    TTBuffer*								mCurBuffer;
    TTInt                                   mAudioCnt;
    
    RTTCritical                         iCritical;
    

};

#endif