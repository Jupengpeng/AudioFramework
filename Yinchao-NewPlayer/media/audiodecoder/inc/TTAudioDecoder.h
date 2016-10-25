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
	* \brief							�򿪽�����
	* \param[in] aUrl					���ֵĵ�ַ
	* \param[in] aSampleRate			����Ĳ�����
	* \param[in] aChannel			    ������ŵ���
	*/
	TTInt Open(const TTChar* aUrl, TTInt aSampleRate, TTInt aChannel);

	/**
	* \fn								void  Close();
	* \brief							�رս�����
	*/
	void  Close();


	/**
	* \fn								TTInt DecodeToBuffer(TTInt16* buffer, TTInt bufferLength);
	* \brief							����
	* \param[in] buffer					����������
	* \param[in] bufferLength			���������ݳ���
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