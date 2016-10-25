/**
* File : TTAudioSink.h 
* Created on : 2011-3-16
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioSink 定义文件
*/
#ifndef __TT_AUDIO_SINK_H__
#define __TT_AUDIO_SINK_H__

// INCLUDES
#include <e32std.h>
#include <e32msgqueue.h>
#include <e32base.h>

#include "TTBaseDataSink.h"
#include "TTSyncClock.h"
#include "TTMsgQueue.h"


class CTTMediaBuffer;
class CTTAudioStreamRender;

class TTStreamSinkMsg
{
public:
	enum TTStreamSinkMsgId
	{
		EStreamSinkMsgInValid = -1
		, EStreamSinkMsgOpen = 0
		, EStreanAudioProperties = 1
		, EStreanAudioDestory = 2
		, EStreanAudioFillBuffer = 3
		, EStreanAudioRender = 4
	};

	TTStreamSinkMsgId	iId;
	TInt				iData0;
	TTInt				iData1;

public:
	TTStreamSinkMsg()
		: iId(EStreamSinkMsgInValid), iData0(0), iData1(0)
	{}
};

// CLASSES DECLEARATION
class CTTAudioSink : public CTTBaseDataSink, public ITTMsgHandle
{
public:
	/**
	* \fn							CTTAudioSink(ITTSinkDataProvider* aDataProvider);
	* \brief						构造函数
	* \param[in] aDataProvider		数据提供者接口引用
	* \return						返回状态
	*/
	CTTAudioSink(ITTSinkDataProvider* aDataProvider);

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
	virtual TTInt				Render(CTTMediaBuffer* aBuffer);

	/**
	* \fn                       TTInt Open(TTAudioDataSettings& aAudioDataSetting)
	* \brief                    打开设备
	* \param[in]	aAudioDataSetting     音频信息
	* \return					返回操作状态
	*/
	virtual TTInt				Open(TTAudioDataSettings& aAudioDataSetting);

	/**
	* \fn                       void Close()
	* \brief                    关闭设备
	*/
	virtual void				Close();

	void						SetAudioProperties(TTInt aSampleRate, TInt aChannel);

	/**
	* \fn						void GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& nChannels);
	* \brief					获取当前播放的数据
	* \param[in]  aSamples		取采样的个数	
	* \param[out] aWave			采样点的数据指针	
	* \param[out] nChannels		音频声道数	
	*/
	virtual void				GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels);

	void						RenderThreadProcL();

public://from ITTSyncClock
	/**
	* \fn						void Start()
	* \brief					开始送数据
	*/
	virtual	void				Start();

	/**
	* \fn						void Pause()
	* \brief					暂停送数据
	*/
	virtual	void				Pause();

	/**
	* \fn						void Resume()
	* \brief					暂停后，继续送数据
	*/
	virtual	void				Resume();

	/**
	* \fn						void Stop()
	* \brief					停止送数据
	*/
	virtual	void				Stop();

	/**
	* \fn						void Initialize()
	* \brief					开始送数据
	* \return					操作状态
	*/
	virtual	TTInt				Initialize();

	/**
	* \fn						void UnInitialize()
	* \brief					开始送数据
	*/
	virtual	void				UnInitialize();

public://from ITTInterface
	/**
	* \fn							QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr)
	* \brief						请求接口
	* \param	aInterfaceID[in]	接口ID
	* \param	aInterfacePtr[in]	接口指针
	* \return						KErrNone: 成功
	*								KErrNotSupport: 不支持此接口
	*/
	virtual	TTInt					QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr);


//	void							Active();


private:
	TTInt							Render();


	static TInt						RenderThreadProc(TAny* aPtr);
	void							FillBuffer(CTTMediaBuffer* aBuffer);

private:
		virtual void HandleMsg(TTMsg& aMsg);


private:
	TTBool							iStopSendData;
	RThread							iRenderThread;
	RMsgQueue<TTStreamSinkMsg>		iSendMsgQueue;
	CTTAudioStreamRender*			iStreamRender;
	CTTMediaBuffer*					iSrcBuffer;
};

#endif
