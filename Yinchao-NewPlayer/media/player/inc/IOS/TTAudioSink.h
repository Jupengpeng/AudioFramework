/**
* File : TTAudioSink.h 
* Created on : 2011-9-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioSink �����ļ�
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
	* \brief						���캯��
	* \param[in] aDataProvider		�����ṩ�߽ӿ�����
    * \param[in] aObserver			���ŷ�Χ�ص�	
	*/
	CTTAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	/**
	* \fn							~CTTAudioSink();
	* \brief						��������
	*/
	virtual ~CTTAudioSink();


public://from ITTDataSink

	/**
	* \fn                       void Render(CTTMediaBuffer* aBuffer);
	* \brief                    �ύ����
	* \param[in] aBuffer		����ָ��
	* \return					����״̬
	*/
	virtual TTInt			render();

	/**
	* \fn                       TTInt Open(TTAudioDataSettings& aAudioDataSetting)
	* \brief                    ���豸
	* \param[in] aAudioDataSetting     ��Ƶ��Ϣ
	* \return					���ز���״̬
	*/
	

	/**
	* \fn                       void Close()
	* \brief                    �ر��豸
	*/
	virtual TTInt           close();

	/**
	* \fn						void GetCurWave(TTInt aSamples, TTInt16** aWave, TTInt& nChannels);
	* \brief					��ȡ��ǰ���ŵ�����
	* \param[in]  aSamples		ȡ�����ĸ���	
	* \param[out] aWave			�����������ָ��	
	* \param[out] nChannels		��Ƶ������	
	*/
	//virtual void				GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels);

    /**
     * \fn						void SetVolume(TTInt aVolume)
     * \brief					��������
     * \param[in] aLVolume		����ֵ	
     * \param[in] aRVolume		����ֵ	
     */
	virtual void				SetVolume(TTInt aLVolume, TTInt aRVolume);
    
    /**	
     * \fn						void Flush();
     * \brief					��ջ����е�����
     */
	virtual TTInt				flush();
    
    /**
     * \fn						void Position(TTUint& aPosition)
     * \brief					��ȡ����λ��
     * \param[out] aPosition	����λ�ã���λ����
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
     * \brief					��ͣ������
     */
	virtual	TTInt				pause(TTBool aFadeOut);
    
    /**
     * \fn						TTInt Resume()
     * \brief					��ͣ�󣬼���������
     * \return                  TTKErrNone respresents success, other value respresents fail.
     */
	virtual	TTInt				resume(TTBool aWait = false,TTBool aFadeIn= false);
    
	/**
	* \fn						void Stop()
	* \brief					ֹͣ������
	*/
	virtual	TTInt				stop();
    
    static  void                setIOSVersion();
    
    
private://��CTTActive�̳�
	 
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