#ifndef __PUREDECODEENTITY__
#define __PUREDECODEENTITY__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTCritical.h"
#include "TTMediainfoProxy.h"
#include "TTAudioPlugin.h"
#include "aflibconverter.h"
//#include "TTSrcDemux.h"
#include "TTMediadef.h"
// CLASSES DECLEARATION
class CTTPureDecodeEntity 
{
public:
	/**
	* \fn							CTTPureDecodeEntity()
	* \brief						构造函数

	*/
	CTTPureDecodeEntity();

	/**
	* \fn							~CTTPureDecodeEntity()
	* \brief						析构函数
	*/
	virtual ~CTTPureDecodeEntity();

public:
	 
	/**
	* \fn							void Decode(const TTChar* aUrl, TTInt size)
	* \brief					    外部调用解码接口
	* \param[in] aUrl			    源文件url	
	* \param[in] size			    需要获取的pcm数据size	
	* \return						解码结果		
	*/
	TTInt							Decode(const TTChar* aUrl, TTInt size);

	/**
	* \fn							TTInt GetPCMData();
	* \brief						获取解码数据
	* \return						解码数据量		
	*/
	TTUint8*						GetPCMData();

	/**
	* \fn							TTInt GetPCMDataSize();
	* \brief						获取解码数据量
	* \return						解码数据量值		
	*/
	TTInt							GetPCMDataSize();

	/**
	* \fn							void SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos)
	* \brief						设置播放范围
	* \param[in] start			    起始值	
	* \param[in] end			    结束值	
	* \param[in] decodeStartPos		起始解码位置值	
	*/
	void							SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos);

	/**
	* \fn							TTInt Volume();
	* \brief						获取音量
	* \return						音量值		
	*/
	TTInt							OpenAndParse(const TTChar* aUrl);

	/**	
	* \fn							void Flush();
	* \brief						清空缓冲中的数据
	*/
	TTInt							InitDecodePlugin();

	/**
	* \fn							void SeekToStartPos();
	* \brief			            seek到解码位置处
	* \return						seek结果	
	*/
	TTInt							SeekToStartPos();


	/**
	* \fn							void DecodeData();
	* \brief			            解码数据
	* \return						 解码结果	
	*/
	TTInt							DecodeData(TTInt size);

	/**
	* \fn							void GetSamplerate();
	* \brief			            获取采样率
	* \return						采样率值	
	*/
	TTInt							GetSamplerate();

	/**
	* \fn							void GetChannels();
	* \brief			            获取声道数
	* \return						声道值	
	*/
	TTInt							GetChannels();

	/**
	* \fn							void Cancel();
	* \brief			            取消操作	
	*/
	void							Cancel();

	static CTTPureDecodeEntity*		Instance();
    
    static void                     Release();

	TTInt							updateParam(TTAudioInfo* pCurAudioInfo);

	TTInt							DecBuffer(TTPBYTE pBuffer, TTInt32 nSize);

	TTInt							updateStep();

	TTInt							doSampleBitsConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

	void							convert64BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	void							convert8BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	void							convert24BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	void							convert32BitIntTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);
	void							convert32BitFloatTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

	TTInt							doChannelDoMix(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

	TTInt							doSampleRateConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer);

protected:
	RTTSemaphore					iSemaphore;

private: 

	CTTAudioPluginManager*			iPluginManager;
	CTTMediaInfoProxy*				iMediaInfoProxy;

	TTBuffer						mSrcBuffer;
	TTBuffer*						aDstBuffer;

	TTUint							iCurSrcMediaFormat;

	TTInt							iSleepDataSize;
	TTUint32						iWaitIntervalUs;

	TTUint8*						iDecodeDataPtr;
	TTInt							iDecodeDataSize;
	TTInt							iCopyPos;
	TTBool							iCancel;
	TTInt							iDecodrStartPos;

	static CTTPureDecodeEntity*		iDecodeEntity;

	//CTTSrcDemux *mSrc ;
	TTAudioFormat	 mAudioFormat;
	TTInt			 mAudioDecSize;
	TTInt			 mAudioStepSize;
	TTInt			 mDoSampleBits;
	TTInt			 mDoDownMix;
	TTInt			 mDoSampleRateConv;
	aflibConverter*							mResampleObj;
	float			 mResmpFactor;
	TTObserver		observer;
	//TTWAVFormat		 mWAVFormat;
};

#endif