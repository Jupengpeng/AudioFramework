#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__

#include "TTMediaInfoProxy.h"
#include "TTPluginManager.h"
#include "DataUnit.h"
#include "TTChannelMix.h"
#include "TTSamplerateConv.h"


class ITTDataReader;
class CTTMediaBuffer;
class CTTMediaBufferAlloc;
class ITTMediaParser;


class CTTAudioDecoder : public ITTMediaParserObserver
{
public:
	CTTAudioDecoder();
	~CTTAudioDecoder();
	/**
	* \fn								TTInt Open(const TTChar* aUrl, TTInt aSampleRate, TTInt aChannel);
	* \brief							打开解码器
	* \param[in] aUrl					音乐的地址
	* \param[in] aSampleRate			输出的采样率
	* \param[in] aChannel			    输出的信道数
	*/
	TTInt Open(const TTChar* aUrl, TTInt aSampleRate, TTInt aChannel);

	/**
	* \fn								void  Close();
	* \brief							关闭解码器
	*/
	void  Close();


	/**
	* \fn								TTInt DecodeToBuffer(TTInt16* buffer, TTInt bufferLength);
	* \brief							解码
	* \param[in] buffer					解码后的数据
	* \param[in] bufferLength			解码后的数据长度
	*/
	TTInt DecodeToBuffer(TTInt16* buffer, TTInt bufferLength);

private:
	
	TTInt ReadRawFrame();
	
	TTInt AdaptLocalFileParser(const TTChar* aUrl);
	void  CreateFrameIdxComplete(){};
	TTInt RawDataToPcm(CTTMediaBuffer* aBuffer, TTInt aReadSize);
	DataUnit<TTInt16> GetDecodedDataUnit();


private:
	ITTDataReader*			iReader;
	CTTPluginManager*		iPluginManger;
	TTInt					iCurFrameIdx;
	CTTMediaBuffer*			iCurRawDataBuffer;
	CTTMediaBufferAlloc*	iMediaBufferAlloc;
	ITTMediaParser*			iMediaParser;
	HPluginId				iCurPluginId;
	TTBool					iEOF;
	DataUnit<TTInt16>		iUnfilledDataUnit;
	CTTMediaBuffer*			iPcmBuffer;
	TTMediaInfo				iMediaInfo;
	TTInt16*				iCurBufferPtr;
	CTTChannelMixer*		iChannelMixer;
	CTTSamplerateConv*		iSamplerateConv;
};
#endif