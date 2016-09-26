#ifndef __ST_AUDIO_RECODER_H__
#define __ST_AUDIO_RECODER_H__
#include "STLog.h"
#include "STTypeDef.h"
#include "STMacroDef.h"
#include "STOpenSLESEngine.h"
#include "Resample_samplerate.h"
#include "Resample_float_cast.h"
//add by bin.chen
class STAudioOutput;
class ISTAudioRecorderCallback
{
public:
	/**
	* \fn                       void RecorderError(STInt aErr)
	* \brief                    解码出错
	* \param[in] aErr			错误码
	*/
	virtual void RecorderError(STInt aErr) = 0;
	
	/**
	* \fn                       void RecorderBufferFilled(STUint8* aBufferPtr, STInt aBufferSize, STUint& aState)
	* \brief                    暂停
	* \param[in] aBufferPtr		录制回调Buffer地址
	* \param[in] aBufferSize	回调Buffer大小
	* \param[in] aByteOffset	字节偏移
	* \param[in] aState
	* \return					返回已经处理的字节数
	*/
	virtual STInt RecorderBufferFilled(STUint8* aBufferPtr, STInt aBufferSize, STInt aByteOffset, STUint& aState) = 0;
};

class STAudioRecorder
{
public:
	/**
	* \fn                       STAudioRecorder(STOpenSLESEngine& aEngine, ISTAudioRecorderCallback& aCallback)
	* \brief                    构造函数
	* \param[in] aEngine		Engine引用
	* \param[in] aCallback		回调函数引用
	*/
	STAudioRecorder(STOpenSLESEngine& aEngine, ISTAudioRecorderCallback& aCallback);
	STAudioRecorder(STOpenSLESEngine& aEngine, ISTAudioRecorderCallback& aCallback,STAudioOutput *ParentAudioOutput);
	~STAudioRecorder();
	
public:
	/**
	* \fn                       STInt Open(STInt aSampleRate, STInt aChannel);
	* \brief                    打开操作
	* \param[in] aSampleRate	采样率
	* \param[in] aChannel		通道数
	* \return					STKErrNone为成功
	*/
	STInt 						Open(STInt aSampleRate, STInt aChannel);
	
	/**
	* \fn                       void Start();
	*/
	void						Start();
	
	/**
	* \fn                       void Pause();
	*/
	void						Pause();
	
	/**
	* \fn                       void Resume();
	*/
	void 						Resume();
	
	/**
	* \fn                       void Stop();
	*/
	void 						Stop();
	
	/**
	* \fn                       void Close();
	*/
	void						Close();

private:
	static void RecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
	void CallBack();
	STInt ProcessResample(STInt16* aSrc, STInt aFrameNum);
private:

	STOpenSLESEngine&        iOpenSLESEngine;
	ISTAudioRecorderCallback& iCallback;
	
	SLObjectItf 					iRecorderObjectItf;
	SLRecordItf 					iRecorderItf;
	SLAndroidSimpleBufferQueueItf 	iRecorderBufferQueueItf;

	STInt16* 	iRecordBufferQueue; 	// Base adress of local audio data storage
	STInt16* 	iRecordCurBuffer; 	// Current adress of local audio data storage

	STInt16*    iCurRenderBufferPtr;

	STUint 		iBufferQueueSize;

	STInt		iSampleRate;
	STInt		iChannel;
	SRC_DATA	iSrcData;
	SRC_STATE*	iSrcState;
	STFloat*	iResampleTempSrcBuffer;
	STFloat*	iResampleTempDstBuffer;

	STUint		iRecordByteOffset;
	//add by bin.chen
	STAudioOutput*     pParentAudioOutput;
};

#endif
