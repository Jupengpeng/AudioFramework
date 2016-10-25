/**
* File : TTAudioSink.h 
* Created on : 2011-3-16
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTAudioSink �����ļ�
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
	* \brief						���캯��
	* \param[in] aDataProvider		�����ṩ�߽ӿ�����
	* \return						����״̬
	*/
	CTTAudioSink(ITTSinkDataProvider* aDataProvider);

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
	virtual TTInt				Render(CTTMediaBuffer* aBuffer);

	/**
	* \fn                       TTInt Open(TTAudioDataSettings& aAudioDataSetting)
	* \brief                    ���豸
	* \param[in]	aAudioDataSetting     ��Ƶ��Ϣ
	* \return					���ز���״̬
	*/
	virtual TTInt				Open(TTAudioDataSettings& aAudioDataSetting);

	/**
	* \fn                       void Close()
	* \brief                    �ر��豸
	*/
	virtual void				Close();

	void						SetAudioProperties(TTInt aSampleRate, TInt aChannel);

	/**
	* \fn						void GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& nChannels);
	* \brief					��ȡ��ǰ���ŵ�����
	* \param[in]  aSamples		ȡ�����ĸ���	
	* \param[out] aWave			�����������ָ��	
	* \param[out] nChannels		��Ƶ������	
	*/
	virtual void				GetCurWave(TTInt aSamples, TTInt16* aWave, TTInt& aChannels);

	void						RenderThreadProcL();

public://from ITTSyncClock
	/**
	* \fn						void Start()
	* \brief					��ʼ������
	*/
	virtual	void				Start();

	/**
	* \fn						void Pause()
	* \brief					��ͣ������
	*/
	virtual	void				Pause();

	/**
	* \fn						void Resume()
	* \brief					��ͣ�󣬼���������
	*/
	virtual	void				Resume();

	/**
	* \fn						void Stop()
	* \brief					ֹͣ������
	*/
	virtual	void				Stop();

	/**
	* \fn						void Initialize()
	* \brief					��ʼ������
	* \return					����״̬
	*/
	virtual	TTInt				Initialize();

	/**
	* \fn						void UnInitialize()
	* \brief					��ʼ������
	*/
	virtual	void				UnInitialize();

public://from ITTInterface
	/**
	* \fn							QueryInterface(TTUint32 aInterfaceID, void** aInterfacePtr)
	* \brief						����ӿ�
	* \param	aInterfaceID[in]	�ӿ�ID
	* \param	aInterfacePtr[in]	�ӿ�ָ��
	* \return						KErrNone: �ɹ�
	*								KErrNotSupport: ��֧�ִ˽ӿ�
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
