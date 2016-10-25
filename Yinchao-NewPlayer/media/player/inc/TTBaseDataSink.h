/**
* File : TTBaseDataSink.h 
* Created on : 2011-3-16
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTBaseDataSink定义文件
*/
#ifndef __TT_BASE_DATA_SINK_H__
#define __TT_BASE_DATA_SINK_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTDataSink.h"
#include "TTSyncClock.h"
#include "TTCritical.h"

// CLASSES DECLEARATION
class CTTBaseDataSink : public ITTDataSink, public ITTSyncClock
{
public:
	/**
	* \fn							CTTBaseDataSink(ITTSinkDataProvider* aProvider, ITTPlayRangeObserver& aObserver)
	* \brief						构造函数
	* \param[in] aProvider			数据提供者指针		
	* \param[in] aObserver			播放范围回调	
	*/
	CTTBaseDataSink(ITTSinkDataProvider* aProvider, ITTPlayRangeObserver& aObserver);

	/**
	* \fn							~CTTBaseDataSink()
	* \brief						析构函数
	*/
	virtual ~CTTBaseDataSink();

public://from ITTDataSink

	/**
	* \fn							TTBool IsBufferRequesting();
	* \brief						Sink是否在等待Buffer
	* \return						ETrue表示在等待
	*/
	virtual TTBool					IsBufferRequesting();
	
	/**
	* \fn							TTInt Open(TTAudioDataSettings& aAudioDataSetting)
	* \brief						打开设备
	* \param[in] aAudioDataSetting  音频信息
	* \return						返回操作状态
	*/
	virtual TTInt					Open(TTAudioDataSettings& aAudioDataSetting);

	/**
	* \fn							void SetVolume(TTInt aVolume)
	* \brief						设置音量
	* \param[in] aVolume			音量值		
	*/
	virtual	void					SetVolume(TTInt aLVolume, TTInt aRVolume);

	/**
	* \fn							TTInt Volume();
	* \brief						获取音量
	* \return						音量值		
	*/
	TTInt							Volume();

	/**	
	* \fn							void Flush();
	* \brief						清空缓冲中的数据
	*/
	virtual void					Flush();

	/**
	* \fn							void IsStreamUnderflow();
	* \brief			            数据不够
	*/
	virtual TTBool					IsStreamUnderflow();
    
    virtual void                    Close();


public://ITTSyncClock
	/**
	* \fn							void Start()
	* \brief						开始送数据
	*/
	virtual	void					Start();

	/**
	* \fn							void Pause()
	* \brief						暂停送数据
	*/
	virtual	void					Pause();

	/**
	* \fn							TTInt Resume()
	* \brief						暂停后，继续送数据
	* \return                       TTKErrNone respresents success, other value respresents fail.
    */
	virtual	TTInt 					Resume();

	/**
	* \fn							void Stop()
	* \brief						停止送数据
	*/
	virtual	void					Stop();

	/**
	* \fn							void SyncPosition(TTUint aPosition)
	* \brief						同步播放位置，Seek后后端位置可能没法及时更新，故需要同步一下
	* \param[out]	aPosition		播放位置，单位毫秒
	*/
	virtual	void					SyncPosition(TTUint aPosition);

	/**
	* \fn							void Position(TTUint& aPosition)
	* \brief						获取播放位置
	* \param[out]	aPosition		播放位置，单位毫秒
	*/
	virtual void					Position(TTUint& aPosition);

	/**
	* \fn							void SetPlayRange(TTUint aStartTime, TTUint aEndTime)
	* \brief						设置播放范围
	* \param[in]	aStartTime		播放范围起始时间，单位毫秒
	* \param[in]	aEndTime		播放范围结束时间，单位毫秒
	*/
	virtual void					SetPlayRange(TTUint aStartTime, TTUint aEndTime);

	/**
	* \fn							TTPlayRange PlayRange()
	* \brief						获取播放范围
	* \return						返回播放范围
	*/
	virtual TTPlayRange				PlayRange();


public://ITTInterface
	/**
	* \fn							TTInt QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr)
	* \brief						请求接口
	* \param[in]	aInterfaceID	接口ID
	* \param[in]	aInterfacePtr	接口指针
	* \return						KErrNone: 成功
	*								KErrNotSupport: 不支持此接口
	*/
	virtual	TTInt					QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr);


protected:
	/**
	* \fn							void StreamUnderflow();
	* \brief			            数据不够
	*/
	void							StreamUnderflow();
	void							StreamOverflow();

	TTInt							Render(CTTMediaBuffer* aBuffer);
	TTBool							IsBuffering();
	void							NotifyBufferRequesting();
	void							CancelBufferRequesting();
	TTBool							IsRenderingEnable();
	TTInt							GetValidBufferRange(CTTMediaBuffer* aMediaBuffer);


protected:
	ITTSinkDataProvider*			iDataProvider;
	RTTCritical						iCritical;
	TTUint64						iCurPos;
	TTInt							iLVolume;
	TTInt							iRVolume;
	TTInt							iChannels;
	TTInt							iSampleRate;
	TTBool							iUnderflow;
	ITTPlayRangeObserver&			iPlayRangeObserver;
	TTPlayRange						iPlayRange;

private:
	TTBool							iBufferRequesting;
	TTBool							iBuffering;/*后端Sink正在缓冲，用于Seek时*/
	TTBool							iRenderingEnable;/*是否可以送数据*/
};

#endif
