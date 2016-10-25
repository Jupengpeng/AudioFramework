#include "TTAudioDecoder.h"
#include "TTFileReader.h"
#include "TTMediaBuffer.h"
#include "TTMediaBufferAlloc.h"
#include "TTMP3Parser.h"
#include "TTWMAParser.h"
#include "TTAACParser.h"
#include "TTALACParser.h"
#include "TTAPEParser.h"
#include "TTFLACParser.h"
#include "TTMP4Parser.h"
#include "TTWAVParser.h"
#include "TTLog.h"


static const TTInt KMediaBufferAllocSize = KILO * KILO;
static const TTInt PCMBlock = 448;
static const TTInt KPcmBufferSize = PCMBlock * KILO;

CTTAudioDecoder::CTTAudioDecoder()
: iReader(NULL)
, iCurFrameIdx(0)
, iCurRawDataBuffer(NULL)
, iPcmBuffer(NULL)
, iCurPluginId(PluginIdNone)
, iEOF(ETTFalse)
{
	iPluginManger = new CTTPluginManager();
	iMediaBufferAlloc = new CTTMediaBufferAlloc(KMediaBufferAllocSize);
}

CTTAudioDecoder::~CTTAudioDecoder()
{
	SAFE_DELETE(iPluginManger);
	if (iCurRawDataBuffer != NULL)
	{
		iCurRawDataBuffer->UnRef();
		iCurRawDataBuffer = NULL;
	}
	SAFE_DELETE(iMediaBufferAlloc);
}

TTInt CTTAudioDecoder::Open(const TTChar* aUrl ,TTInt aSampleRate, TTInt aChannel)
{

	iReader = new CTTFileReader(ETTFalse);

	if (iReader == NULL)
	{
		return TTKErrNoMemory;
	}

	TTInt nErr = iReader->Open(aUrl);

	if (nErr != TTKErrNone)
	{
		SAFE_RELEASE(iReader);
		return nErr;
	}

if (((nErr = AdaptLocalFileParser(aUrl)) == TTKErrNone) && ((nErr = iMediaParser->Parse(iMediaInfo)) == TTKErrNone) 
	&& (iMediaInfo.iAudioInfoArray[0]->iMediaTypeAudioCode != TTAudioInfo::KTTMediaTypeAudioCodeAPE)
	&& iPluginManger->IsFormatSupported(iMediaInfo.iAudioInfoArray[0]->iMediaTypeAudioCode)
	&& (iCurPluginId = iPluginManger->InitPlugin(iMediaInfo.iAudioInfoArray[0]->iMediaTypeAudioCode, iMediaInfo.iAudioInfoArray[0])) != PluginIdNone)
	{
		iCurBufferPtr = (TTInt16*)malloc(KPcmBufferSize  * sizeof(TTInt16)*2);
		memset(iCurBufferPtr, 0, KPcmBufferSize  * sizeof(TTInt16)*2);
		iPcmBuffer = iMediaBufferAlloc->RequestBuffer(NULL, KPcmBufferSize);

		iChannelMixer = ChannelMixerFactory::NewMixer(iMediaInfo.iAudioInfoArray[0]->iChannel, aChannel);
		iSamplerateConv = SamplerateConvFactory::NewConv(iMediaInfo.iAudioInfoArray[0]->iSampleRate, aSampleRate, aChannel);

	
		iEOF = ETTFalse;
		return TTKErrNone;
	}

	SAFE_RELEASE(iMediaParser);
	SAFE_RELEASE(iReader);

	return TTKErrNotSupported;
}


void CTTAudioDecoder::Close()
{
	if (iPcmBuffer != NULL)
	{
		iPcmBuffer->UnRef();
	}
	TTASSERT(iCurPluginId != PluginIdNone);
	iPluginManger->UninitPlugin(iCurPluginId);
	iCurPluginId = PluginIdNone;
	SAFE_RELEASE(iMediaParser);
	iReader->Close();
	SAFE_RELEASE(iReader);
	SAFE_DELETE(iCurBufferPtr);
	SAFE_DELETE(iSamplerateConv);
	SAFE_DELETE(iChannelMixer);
}


TTInt CTTAudioDecoder::DecodeToBuffer(TTInt16* buffer, TTInt bufferLength)
{
	TTInt16* bufferPtr = buffer;
	TTInt availableBufferLength = bufferLength;

	DataUnit<TTInt16> dataUnit = (iUnfilledDataUnit.GetLength() > 0) ? iUnfilledDataUnit : GetDecodedDataUnit();

	TTInt dataLength = dataUnit.GetLength();

	iUnfilledDataUnit.SetLength(0);

	
	while (dataLength  > 0 && dataLength <= availableBufferLength)
	{
		memcpy(bufferPtr, dataUnit.GetPointer() , dataLength * sizeof(TTInt16));
		bufferPtr += dataLength;
	
		availableBufferLength -= dataLength;

		dataUnit = GetDecodedDataUnit();
		dataLength = dataUnit.GetLength();
	}

	if (dataLength > 0)
	{
		iUnfilledDataUnit = dataUnit;
	} 

	return bufferLength - availableBufferLength;
}

DataUnit<TTInt16> CTTAudioDecoder::GetDecodedDataUnit()
{
	DataUnit<TTInt16> dataUnit(NULL, 0, 0);


	TTInt nErr = RawDataToPcm(iPcmBuffer, KPcmBufferSize);

	if (nErr == TTKErrNone)
	{
		
		dataUnit = iChannelMixer->Mix(DataUnit<TTInt16> ((TTInt16*)iPcmBuffer->Ptr(), iPcmBuffer->Size() / 2, iPcmBuffer->Size() / 2));
		dataUnit = iSamplerateConv->Conv(dataUnit);
		//dataUnit.Construct((TTInt16*)iPcmBuffer->Ptr(), iPcmBuffer->Size() / 2, iPcmBuffer->Size() / 2);
	}

	return dataUnit;
}

TTInt CTTAudioDecoder::RawDataToPcm(CTTMediaBuffer* aBuffer, TTInt aReadSize)
{
	TTASSERT(aReadSize > 0);

	TTInt nErr = TTKErrNone;

	while (true)
	{	
		if (iCurRawDataBuffer == NULL)
		{
			nErr = ReadRawFrame();
			if (nErr < 0)
				break;
		}
		
		TTCodecProcessResult tResult = iPluginManger->ProcessL(iCurPluginId, iCurRawDataBuffer, aBuffer);
		if (ESrcEmptied == tResult || EProcessComplete == tResult)
		{
			iCurRawDataBuffer->UnRef();
			iCurRawDataBuffer = NULL;
		}

		if (EProcessComplete == tResult || EProcessError == tResult || EDstFilled == tResult) 
		{
			nErr = EProcessError == tResult ? TTKErrCorrupt : TTKErrNone;
			break;	
		}			
	}

	return nErr;
}

TTInt CTTAudioDecoder::ReadRawFrame()
{
	if (iEOF)
	{
		return TTKErrOverflow;
	}

	TTMediaFrameInfo tMediaFrameInfo;
	
	TTInt nErr = iMediaParser->GetFrameLocation(EMediaStreamIdAudioL, iCurFrameIdx, tMediaFrameInfo);

	iEOF = (nErr != TTKErrNone);

	if ((nErr == TTKErrNone) || (nErr == TTKErrEof && tMediaFrameInfo.iFrameSize > 0))
	{
		iCurFrameIdx++;
		iCurRawDataBuffer = iMediaBufferAlloc->RequestBuffer(NULL, tMediaFrameInfo.iFrameSize);
		TTASSERT(iCurRawDataBuffer != NULL);

		iCurRawDataBuffer->SetExtraInfo(tMediaFrameInfo.iExtraInfo);

		nErr = iReader->ReadSync(iCurRawDataBuffer->Ptr(), tMediaFrameInfo.iFrameLocation, tMediaFrameInfo.iFrameSize);
	}

	return nErr;
}

TTInt CTTAudioDecoder::AdaptLocalFileParser(const TTChar* aUrl)
{
	TTUint8 tHeader[KMaxMediaFlagSize];
	TTInt nReadSize = iReader->ReadSync(tHeader, 0, KMaxMediaFlagSize);
	if (nReadSize < KMaxMediaFlagSize)
	{
		return TTKErrUnderflow;
	}

	CTTMediaInfoProxy::TTMediaFormatId tMediaExtId = CTTMediaInfoProxy::IdentifyMedia(*iReader, aUrl/*, tHeader, nReadSize*/);

	switch (tMediaExtId)
	{
	case CTTMediaInfoProxy::EMediaExtIdMP3:
		iMediaParser = new CTTMP3Parser(*iReader, *this);				
		break;

	case CTTMediaInfoProxy::EMediaExtIdWMA:
		iMediaParser = new CTTWMAParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdAPE:
		iMediaParser = new CTTAPEParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdAAC:
		iMediaParser = new CTTAACParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdM4A:
		iMediaParser = new CTTMP4Parser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdFLAC:
		iMediaParser = new CTTFLACParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdWAV:
		iMediaParser = new CTTWAVParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdALAC:
		iMediaParser = new CTTALACParser(*iReader, *this);
		break;

	case CTTMediaInfoProxy::EMediaExtIdOGG:
	case CTTMediaInfoProxy::EMediaExtIdNone:
		return TTKErrNotSupported;
		break;

	default:
		TTASSERT(ETTFalse);
		return TTKErrNotSupported;
		break;
	}	

	TTASSERT(iMediaParser != NULL);

	return TTKErrNone;
}
