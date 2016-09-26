#ifndef __ST_AUDIO_OUTPUT_H__
#define __ST_AUDIO_OUTPUT_H__
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include "STBaseAudioOutput.h"
#include "STEffect.h"
class STAudioOutput : public STBaseAudioOutput
{
public:
	STAudioOutput(ISTPlayRangeOverflowObserver& aObserver);
	~STAudioOutput();
public:
	/**
	* \fn                       void Pause()
	* \brief                    暂停
	*/
	virtual void				Pause();

	/**
	* \fn                       void Resume()
	* \brief                    继续
	*/
	virtual	void				Resume();

	/**
	* \fn                       void Open(STInt aSampleRate, STInt aChannels)
	* \brief                    打开设备
	* \param[in] aSampleRate	采样率
	* \param[in] aChannels		声道数
	* \return					STKErrNone为成功
	*/
	virtual STInt				Open(STInt aSampleRate, STInt aChannels);

	/**
	* \fn                       void Close
	* \brief                    关闭设别	
	*/
	virtual void				Close();

	/**
	* \fn                       void Start
	* \brief                    开始播放	
	*/
	virtual void				Start();

	/**
	* \fn                       void Stop
	* \brief                    停止播放	
	*/
	virtual void				Stop();

	/**
	* \fn                       void Flush()
	* \brief                    暂停
	*/
	virtual void				Flush();

	/**
	* \fn                       STBool IsUnderflow();
	* \brief                    后端数据是否用完，主要用于判断是否播放完所有的Buffer	
	* \return					所有的数据用完返回ESTTrue
	*/
	virtual STBool				IsUnderflow();	

	/**
	* \fn						STInt GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	* \brief					获取当前播放的数据
	* \param[out] aWave			采样点的数据指针	
	* \param[in]  aSamples		取采样的个数	
	* \param[out] nChannels		音频声道数	
	*/
	virtual STInt				GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
    
    /**
     * \fn                       void Position(STUint& aPosition)
     * \brief                    获取当前播放位置	
     * \param[out] aPosition	 当前位置
     */
	virtual void				Position(STUint& aPosition);

    /**
     * \fn                      void GetEqualizerPresetArray(STPointerArray<STChar>& aArray);
     * \brief                   获取EQ预设数组
     * \param[in, out] aArray	EQ预设数组
     */
    virtual void                GetEqualizerPresetArray(STPointerArray<STChar>& aArray);
    
    /**
     * \fn                      void SetEqualizerPresetIdx(STInt aIdx);  
     * \brief                   设置当前EQ
     * \param[in] aIdx          当前位置
     */
    virtual void                SetEqualizerPresetIdx(STInt aIdx);
    
    /**
     * \fn                      void EnableEffect(STEffectType aEffect, STInt aParam1, STInt aParam2);
     * \brief                   开启音效
     * \param[in] aEffect       音效ID
     * \param[in] aParam1       音效参数1
     * \param[in] aParam2       音效参数2
     */
    void                        EnableEffect(STEffectType aEffect, STInt aParam1, STInt aParam2);
    
    /**
     * \fn                      void DisableEffect(STEffectType aEffect);
     * \brief                   关闭当前音效
     * \param[in] aEffect       当前音效ID
     */
    void                        DisableEffect(STEffectType aEffect);
    
protected:
	virtual STSampleBuffer*		GetFilledBuffer();
	
private:
    
    static OSStatus             AudioUnitCallBack(void *inRefCon, AudioUnitRenderActionFlags*
                                                  ioActionFlags, const AudioTimeStamp*inTimeStamp, UInt32 
                                                  inBusNumber, UInt32 inNumberFrames, AudioBufferList *
                                                  ioData);
    
    void                         AudioUnitCallBackProcL(AudioBufferList *
                                              ioData);
    
    void                         AudioUnitStart();    
    void                         AudioUnitStop();

    void                         initAudioEffect(STInt channels, STInt sampleRate);
    
    void                         processAudioEffect(void* buf, STInt bufSize);
private:
    AUGraph                             iAudioGraph;
    AudioUnit                           iOutputUnit;
    AudioUnit                           iInputUnit;
    AudioUnit                           iEQUnit;
    CFArrayRef                          iEQPresetsArray;
    STInt                               iEQPresetIdx;
    
    
    STSurroundEffect                    iSurrondEffect;
    STBassBoost                         iBassBoost;
    ST3DChorus                          i3DChorus;
    STDelayEffect                       iDelayEffect;
};

#endif