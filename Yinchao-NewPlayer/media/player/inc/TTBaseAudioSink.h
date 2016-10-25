#ifndef __TT_BASE_AUDIO_SINK_H__
#define __TT_BASE_AUDIO_SINK_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTCritical.h"
#include "TTEventThread.h"
#include "TTMediaPlayerItf.h"
#include "TTMediainfoDef.h"
#include "TTSrcDemux.h"
#include "TTAudioProcess.h"

#define MAX_FADE_COUNT 5
enum TTBufferingStatus
{
	ETTBufferingInValid = -1,
	ETTBufferingStart = 0,
	ETTBufferingDone
};

enum TTAudioFadeStatus
{
	ETTAudioFadeNone = 0,
	ETTAudioFadeOut = 1,
	ETTAudioFadeIn
};

// CLASSES DECLEARATION
class TTCBaseAudioSink
{
public:
	/**
	* \fn							TTCBaseAudioSink(CTTSrcDemux* SrcMux)
	* \brief						构造函数
	* \param[in] aProvider			数据提供者指针		
	* \param[in] aObserver			播放范围回调	
	*/
	TTCBaseAudioSink(CTTSrcDemux* SrcMux, TTInt nCount);

	/**
	* \fn							~TTCBaseAudioSink()
	* \brief						析构函数
	*/
	virtual ~TTCBaseAudioSink();

public:
	/**
	* \fn							TTInt Open(TTAudioInfo* pCurAudioInfo)
	* \brief						打开设备
	* \param[in] aAudioDataSetting  音频信息
	* \return						返回操作状态
	*/
	virtual TTInt					open(TTAudioInfo* pCurAudioInfo);

	/**
	* \fn							void SetVolume(TTInt aVolume)
	* \brief						设置音量
	* \param[in] aVolume			音量值		
	*/
	virtual	TTInt					setVolume(TTInt aLVolume, TTInt aRVolume);

	/**
	* \fn							TTInt Volume();
	* \brief						获取音量
	* \return						音量值		
	*/
	virtual	TTInt					volume();

	/**	
	* \fn							void Flush();
	* \brief						清空缓冲中的数据
	*/
	virtual TTInt					flush();

    
    virtual TTInt                   close();

	virtual	TTInt					startOne(TTInt nDelaytime);

	/**
	* \fn							void Start()
	* \brief						开始送数据
	*/
	virtual	TTInt					start(TTBool aPause = false, TTBool aWait = false);

	/**
	* \fn							void Pause()
	* \brief						暂停送数据
	*/
	virtual	TTInt					pause(TTBool aFadeOut = false);

	/**
	* \fn							TTInt Resume()
	* \brief						暂停后，继续送数据
	* \return                       TTKErrNone respresents success, other value respresents fail.
    */
	virtual	TTInt 					resume(TTBool aWait = false,TTBool aFadeIn = false);

	/**
	* \fn							void Stop()
	* \brief						停止送数据
	*/
	virtual	TTInt					stop();

	/**
	* \fn							void GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& nChannels);
	* \brief						获取时域波形数据
	* \param[in]  aSamples			采样数据长度
	* \param[in] aWave				时域波形数据
	* \param[out] nChannels			声道数
	* \return						成功返回TTKErrNone，否则返回错误码
	*/
	virtual	TTInt				    getCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels);
	
	virtual void					setPlayRange(TTUint aStartTime, TTUint aEndTime);
	
	virtual	TTInt					syncPosition(TTUint64 aPosition, TTInt aOption = 0);

	virtual TTInt					render();
									
	virtual TTBool					isEOS();

	virtual TTInt					setBufferStatus(TTBufferingStatus aBufferStatus);

	virtual TTBufferingStatus		getBufferStatus();

	virtual TTInt					setParam(TTInt aID, void* pValue);

	virtual TTInt					getParam(TTInt aID, void* pValue);

	virtual TTInt					onRenderAudio (TTInt nMsg, TTInt nVar1, TTInt nVar2, void* nVar3);
	virtual TTInt					postAudioRenderEvent (TTInt  nDelayTime);

	virtual TTInt					newAudioTrack();
	virtual TTInt					closeAudioTrack();
	virtual TTInt					freeAudioTrack();

	virtual TTInt64					getPlayTime();	

	TTPlayStatus				    getPlayStatus();

	void							setPlayStatus(TTPlayStatus aStatus);

	virtual void					setObserver(TTObserver*	aObserver);

	virtual void					setEOS();

	virtual void					fadeOutInHandle();
    
#ifdef __TT_OS_IOS__
    
    /**
     * \fn						void SetBalanceChannel(float aVolume)
     * \brief                          set balane channel volume
     * \param[in] aVolume  ,value range: [-1,+1]
     */
    virtual void                    SetBalanceChannel(float aVolume);
#endif

	virtual	void					setFadeStatus(TTAudioFadeStatus aStatus);
	virtual	TTAudioFadeStatus		getFadeStatus();

protected:
	virtual void					audioFormatChanged();

protected:
	RTTCritical						mCritical;
	RTTCritical						mCritStatus;
	RTTSemaphore					mSemaphore;
	CTTSrcDemux*					mSrcMux;
	TTUint64						mCurPos;
	TTPBYTE							mSinkBuf;
	TTInt							mSinkBufLen;
	TTInt							mProcessCount;
	TTInt							mLVolume;
	TTInt							mRVolume;
	TTBool							mEOS;
	TTBool							mSeeking;
	TTBuffer						mSinkBuffer;
	TTPlayStatus					mPlayStatus;
	TTAudioFormat					mAudioFormat;
	TTInt							mRenderNum;
	TTBufferingStatus				mBufferStatus;
	TTInt32							mFrameDuration;
	TTInt64							mRenderPCM;
	TTPlayRange						mPlayRange;

	TTObserver*						mObserver;

	RTTCritical						mCritTime;
	TTInt64							mAudioSystemTime;
	TTInt64							mAudioBufferTime;
	TTInt64							mAudioSysStartTime;
	TTInt64							mAudioBufStartTime;
	TTInt64							mAudioOffSetTime;
	TTInt							mAudioAdjustTime;

	TTEventThread*					mRenderThread;
	TTCAudioProcess*				mAudioProcess;
	TTAudioFadeStatus				mFadeStatus;
	TTInt							mFadeCount;
};

class TTCAudioRenderEvent : public TTBaseEventItem
{
public:
    TTCAudioRenderEvent(TTCBaseAudioSink * pRender, TTInt (TTCBaseAudioSink::* method)(TTInt, TTInt,TTInt,void*),
					    TTInt nType, TTInt nMsg = 0, TTInt nVar1 = 0, TTInt nVar2 = 0, void* nVar3 = 0)
		: TTBaseEventItem (nType, nMsg, nVar1, nVar2, nVar3)
	{
		mAudioRender = pRender;
		mMethod = method;
    }

    virtual ~TTCAudioRenderEvent()
	{
	}

    virtual void fire (void) 
	{
        (mAudioRender->*mMethod)(mMsg, mVar1, mVar2, mVar3);
    }

protected:
    TTCBaseAudioSink *		mAudioRender;
    int (TTCBaseAudioSink::* mMethod) (TTInt, TTInt, TTInt, void*);
};

#endif
