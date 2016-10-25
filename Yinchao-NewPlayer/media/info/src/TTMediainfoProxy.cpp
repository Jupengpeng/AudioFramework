//INCLUDES
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "TTOsalConfig.h"
#include "TTLog.h"
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTUrlParser.h"
#include "TTIntReader.h"
#include "TTMediainfoProxy.h"
#include "TTFileReader.h"
#include "TTHttpReader.h"
#include "TTBufferReader.h"
#include "TTID3Tag.h"
//#include "TTWAVParser.h"
//#include "TTALACParser.h"
#include "TTMP3Parser.h"
//#include "TTWMAParser.h"
//#include "ttapeparser.h"
//#include "TTAACParser.h"
//#include "TTAMRParser.h"
//#include "TTMP4Parser.h"
//#include "ttflacparser.h"
#include "TTHttpMP3Parser.h"
//#include "TTHttpAACParser.h"
#include "TTHttpReaderProxy.h"
#ifdef __TT_OS_IOS__
#include "TTIPodLibraryFileReader.h"
#include "TTPureDataParser.h"
#include "TTExtAudioFileReader.h"
#include "TTExtAudioFileParser.h"
#else
//#include "TTDTSParser.h"
#endif
#include "TTHttpClient.h"

#include "TTSysTime.h"

extern const bool gGetCacheFileEnble();

typedef struct MediaFormatMap_s
{
	const TTChar*					iMediaFormatExt;
	CTTMediaInfoProxy::TTMediaFormatId	iMediaFormatId;
} MediaFormatMap_t;

static const MediaFormatMap_t KMediaFormatMap[] = { {"AAC", CTTMediaInfoProxy::EMediaExtIdAAC},
													{"AMR", CTTMediaInfoProxy::EMediaExtIdAMR},
													{"APE", CTTMediaInfoProxy::EMediaExtIdAPE},
													{"FLAC", CTTMediaInfoProxy::EMediaExtIdFLAC},
													{"MP4", CTTMediaInfoProxy::EMediaExtIdM4A},
													{"M4A", CTTMediaInfoProxy::EMediaExtIdM4A},
													{"MID", CTTMediaInfoProxy::EMediaExtIdMIDI},
													{"MP3", CTTMediaInfoProxy::EMediaExtIdMP3},
													{"OGG", CTTMediaInfoProxy::EMediaExtIdOGG},
													{"WAV", CTTMediaInfoProxy::EMediaExtIdWAV},
													{"WMA", CTTMediaInfoProxy::EMediaExtIdWMA},
													{"DTS", CTTMediaInfoProxy::EMediaExtIdDTS},
												  };

static const TTInt KMAXExtSize = 16;

static const TTInt KALACFlagSize = 4;
static const TTUint8 KALACFlag[KALACFlagSize] =
{
	0x63,0x61,0x66,0x66//"caff"
};

static const TTInt KAMRFlagSize = 6;
static const TTUint8 KAMRFlag[KAMRFlagSize] =
{
	0x23,0x21,0x41,0x4D, 0x52, 0x0A//"#!AMR."
};

static const TTInt KAPEFlagSize = 4;
static const TTUint8 KAPEFlag[KAPEFlagSize] =
{
	0x4D,0x41,0x43,0x20//"MAC "
};

static const TTInt KFLACFlagSize = 4;
static const TTUint8 KFLACFlag[KFLACFlagSize] =
{
	0x66,0x4C,0x61,0x43//"fLac"
};

static const TTInt KM4AFlagSize = 4;
static const TTUint8 KM4AFlag[KM4AFlagSize] =
{
	0x66,0x74,0x79,0x70//"ftyp"
};

static const TTInt KMIDIFlagSize = 4;
static const TTUint8 KMIDIFlag[KMIDIFlagSize] =
{
	0x4D,0x54,0x68,0x64//"MThd"
};

static const TTInt KDTSFlagSize = 4;
static const TTUint8 KDTSFlag[KDTSFlagSize] =
{
	0x7F,0xFE,0x80,0x01//"dts,little endian" 
};

static const TTUint8 KDTSFlag1[KDTSFlagSize] =
{
	 0xFE,0x7F,0x01,0x80//"dts,big endian"
};

static const TTInt KWAVFlagSize = 4;
static const TTUint8 KWAVFlag[KAPEFlagSize] =
{
	0x52,0x49,0x46,0x46//"RIFF"
};

static const TTInt KWMAFlagSize = 16;
static const TTUint8 KWMAFlag[KWMAFlagSize] =
{
	0x30,0x26,0xB2,0x75,0x8E,0x66,0xCF,0x11,0xA6,0xD9,0x00,0xAA,0x00,0x62,0xCE,0x6C
};


CTTMediaInfoProxy::CTTMediaInfoProxy(TTObserver* aObserver)
: iMediaParser(NULL)
, iDataReader(NULL)
, iObserver(aObserver)
{
	iCriEvent.Create();
}

CTTMediaInfoProxy::~CTTMediaInfoProxy()
{
	SAFE_RELEASE(iDataReader);
	iCriEvent.Destroy();
}

TTInt CTTMediaInfoProxy::Open(const TTChar* aUrl, TTInt aFlag)
{
	TTASSERT(iMediaParser == NULL);

	AdaptSrcReader(aUrl, aFlag);

	if (iDataReader == NULL) 
	{
		return TTKErrAccessDenied;
	}

	ITTDataReader::TTDataReaderId tReaderId = iDataReader->Id();
	LOGI("CTTMediaInfoProxy::Open ReaderId: %d", tReaderId);

	if (tReaderId == ITTDataReader::ETTDataReaderIdHttp)
	{
		//LOGI("CTTMediaInfoProxy::Open SetStreamBufferingObserver");
		((CTTHttpReader*)iDataReader)->SetStreamBufferingObserver(this);
		//LOGI("CTTMediaInfoProxy::Open SetStreamBufferingObserver ok");
	}
    
	if (tReaderId == ITTDataReader::ETTDataReaderIdBuffer)
	{
		//LOGI("CTTMediaInfoProxy::Open SetStreamBufferingObserver");
		((CTTBufferReader*)iDataReader)->SetStreamBufferingObserver(this);
		//LOGI("CTTMediaInfoProxy::Open SetStreamBufferingObserver ok");
	}

    TTInt nErr = iDataReader->Open(aUrl);
 
#ifdef __TT_OS_IOS__
    if (nErr != TTKErrNone && tReaderId == ITTDataReader::ETTDataReaderIdExtAudioFile)
    {
        SAFE_RELEASE(iDataReader);
        AdaptSrcReader(aUrl, aFlag, ETTFalse);
        if (iDataReader == NULL)
        {
            return TTKErrAccessDenied;
        }
        tReaderId = iDataReader->Id();
        nErr = iDataReader->Open(aUrl);
    }
#endif
    
	if (nErr == TTKErrNone)
	{
		switch (tReaderId)
		{
		case ITTDataReader::ETTDataReaderIdFile:
			nErr = AdaptLocalFileParser(aUrl);
			break;

		case ITTDataReader::ETTDataReaderIdBuffer:
		case ITTDataReader::ETTDataReaderIdHttp:
			nErr = AdaptHttpFileParser(aUrl);
			break;

#ifdef __TT_OS_IOS__
		case ITTDataReader::ETTDataReaderIdIPodLibraryFile:
            iMediaParser = new CTTIPodLibraryParser(*iDataReader, *this);
			break;
                
        case ITTDataReader::ETTDataReaderIdExtAudioFile:
            iMediaParser = new CTTExtAudioFileParser(*iDataReader, *this);
            break;
#endif
                
		default:
			TTASSERT(ETTFalse);
			break;
		}

		//nErr = (nErr != TTKErrNone) ? nErr : ((iMediaParser == NULL) ? TTKErrNoMemory : TTKErrNone);
		if (nErr == TTKErrNone)
		{
			nErr = (iMediaParser == NULL) ? TTKErrNoMemory : TTKErrNone;
		}
	}
	LOGI("CTTMediaInfoProxy::Open return: %d", nErr);
	return nErr;
}

TTInt CTTMediaInfoProxy::AdaptHttpFileParser(const TTChar* aUrl)
{
	TTMediaFormatId tMediaFormatId = IdentifyMedia(*iDataReader, aUrl);
	if (tMediaFormatId == EMediaExtIdMP3)
	{
		iMediaParser = new CTTHttpMP3Parser(*iDataReader, *this);
	}
	else
	{
		LOGE("HttpSource Error:url = %s, formatId = %d", aUrl, tMediaFormatId);
		return TTKErrOnLineFormatNotSupport;
	}

	return TTKErrNone;
}


TTInt CTTMediaInfoProxy::AdaptLocalFileParser(const TTChar* aUrl)
{
	TTInt nErr = TTKErrNone;

	TTMediaFormatId tMediaExtId = IdentifyMedia(*iDataReader, aUrl);
	switch (tMediaExtId)
	{
	case EMediaExtIdMP3:
		iMediaParser = new CTTMP3Parser(*iDataReader, *this);
		break;

	case EMediaExtIdNone:
		nErr = TTKErrNotSupported;
		break;

	default:
		TTASSERT(ETTFalse);
		nErr = TTKErrNotSupported;
		break;
	}


	TTASSERT(iMediaParser != NULL);
	LOGI("AdaptLocalFileParser return: %d", nErr);
	return nErr;
}

TTInt CTTMediaInfoProxy::Parse()
{
 	TTASSERT(iMediaParser != NULL);
 	TTInt nErr = iMediaParser->Parse(iMediaInfo);

	if(nErr != TTKErrNone )
	{
		SAFE_DELETE(iMediaParser);
		iMediaInfo.Reset();

		if (nErr == TTKErrFormatIsMP3) 
		{
			return Parse(TTKErrFormatIsMP3);
		}
		
	}

	if(iDataReader->Id() == ITTDataReader::ETTDataReaderIdFile) {
		BufferingDone();
	}
	return nErr;
}

TTInt CTTMediaInfoProxy::Parse(TTInt aErrCode)
{
	if(aErrCode == TTKErrFormatIsMP3)
	{
		iMediaParser = new CTTMP3Parser(*iDataReader, *this);
		return iMediaParser->Parse(iMediaInfo);
	}
	return TTKErrNotSupported;
}

void CTTMediaInfoProxy::Close()
{	
	LOGI("CTTMediaInfoProxy::Close");

	//TTInt64 nStart = GetTimeOfDay();
	if (iDataReader != NULL)
	{
		iDataReader->Close();
	}
	//TTInt64 nEnd = GetTimeOfDay();
	//LOGI("CTTMediaInfoProxy::Close close Reader use time %lld", nEnd - nStart);

	//nStart = GetTimeOfDay();
	SAFE_DELETE(iMediaParser);
	//nEnd = GetTimeOfDay();
	//LOGI("CTTMediaInfoProxy::Close release iMediaParser use time %lld", nEnd - nStart);

	iMediaInfo.Reset();
}

TTBool CTTMediaInfoProxy::IsALAC(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KALACFlag, KALACFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsAMR(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KAMRFlag, KAMRFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsAPE(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KAPEFlag, KAPEFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsFLAC(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KFLACFlag, KFLACFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsM4A(const TTUint8* aHeader)
{
	return (memcmp(aHeader + 4, KM4AFlag, KM4AFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsMIDI(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KMIDIFlag, KMIDIFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsMP3(const TTUint8* aHeader)
{
	MP3_HEADER mh;
	MP3_FRAME_INFO mi;

	return CTTMP3Header::MP3CheckHeader(aHeader, mh) && CTTMP3Header::MP3ParseFrame(mh, mi);
}

TTBool CTTMediaInfoProxy::IsDTS(const TTUint8* aHeader)
{
	TTBool nRet = ETTFalse; 
	if (memcmp(aHeader, KDTSFlag, KDTSFlagSize) == 0){
		nRet = ETTTrue;
	}
	else if (memcmp(aHeader, KDTSFlag1, KDTSFlagSize) == 0){
		nRet = ETTTrue;
	}
	else {
		if (aHeader[0] == 0xff && aHeader[1] == 0x1f &&
			aHeader[2] == 0x00 && aHeader[3] == 0xe8 &&
			(aHeader[4] & 0xf0) == 0xf0 && aHeader[5] == 0x07)
			nRet = ETTTrue;
		else if (aHeader[0] == 0x1f && aHeader[1] == 0xff &&
			aHeader[2] == 0xe8 && aHeader[3] == 0x00 &&
			aHeader[4] == 0x07 && (aHeader[5] & 0xf0) == 0xf0)
			nRet = ETTTrue;
	}

	return nRet;
}
 
TTBool CTTMediaInfoProxy::IsWAV(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KWAVFlag, KWAVFlagSize) == 0);
}

TTBool CTTMediaInfoProxy::IsWMA(const TTUint8* aHeader)
{
	return (memcmp(aHeader, KWMAFlag, KWMAFlagSize) == 0);
}


TTBool CTTMediaInfoProxy::ShouldIdentifiedByHeader(TTMediaFormatId aMediaFormatId)
{
	return (aMediaFormatId == EMediaExtIdALAC) 
		|| (aMediaFormatId == EMediaExtIdAPE)
		|| (aMediaFormatId == EMediaExtIdFLAC)
		|| (aMediaFormatId == EMediaExtIdM4A)
		|| (aMediaFormatId == EMediaExtIdWAV)
		|| (aMediaFormatId == EMediaExtIdWMA);
}

CTTMediaInfoProxy::TTMediaFormatId CTTMediaInfoProxy::IdentifyMedia(ITTDataReader& aDataReader, const TTChar* aUrl)
{
	TTMediaFormatId tMediaFormatId = IdentifyMediaByHeader(aDataReader);
	if (tMediaFormatId != EMediaExtIdNone)
	{
		return tMediaFormatId;
	}
	
	tMediaFormatId = IdentifyMediaByExtension(aUrl);
	if (tMediaFormatId == EMediaExtIdNone || ShouldIdentifiedByHeader(tMediaFormatId))
	{
		tMediaFormatId = EMediaExtIdMP3;
	}

	return tMediaFormatId;
}

CTTMediaInfoProxy::TTMediaFormatId CTTMediaInfoProxy::IdentifyMediaByHeader(ITTDataReader& aDataReader)
{
	TTUint8 tHeader[KMaxMediaFlagSize];
	TTInt nID3V2Size = ID3v2TagSize(aDataReader);

	if (aDataReader.ReadSync((TTUint8*)tHeader, nID3V2Size, KMaxMediaFlagSize) != KMaxMediaFlagSize)
		return EMediaExtIdNone;

	TTMediaFormatId tMediaFormatId = EMediaExtIdNone;
	
	if (IsALAC(tHeader))
	{
		tMediaFormatId = EMediaExtIdALAC;
	}
	else if (IsAMR(tHeader))
	{
		tMediaFormatId = EMediaExtIdAMR;
	}
	else if (IsAPE(tHeader))
	{
		tMediaFormatId = EMediaExtIdAPE;
	}
	else if (IsFLAC(tHeader))
	{
		tMediaFormatId = EMediaExtIdFLAC;
	}
	else if (IsM4A(tHeader))
	{
		tMediaFormatId = EMediaExtIdM4A;
	}
	else if (IsMIDI(tHeader))
	{
		tMediaFormatId = EMediaExtIdMIDI;
	}
#ifndef __TT_OS_IOS__
	else if(IsDTS(tHeader))
	{
		tMediaFormatId = EMediaExtIdDTS;
	}
#endif
	else if (IsMP3(tHeader))
	{
		tMediaFormatId = EMediaExtIdMP3;
	}
	else if (IsWAV(tHeader))
	{
		tMediaFormatId = EMediaExtIdWAV;
	}
	else if (IsWMA(tHeader))
	{
		tMediaFormatId = EMediaExtIdWMA;
	}

	return tMediaFormatId;
}

CTTMediaInfoProxy::TTMediaFormatId CTTMediaInfoProxy::IdentifyMediaByExtension(const TTChar* aUrl)
{
	TTChar tExtStr[KMAXExtSize];
	CTTUrlParser::ParseExtension(aUrl, tExtStr);

	for (TTInt i = sizeof(KMediaFormatMap) / sizeof(MediaFormatMap_t) - 1; i >= 0; --i)
	{
		if (strcmp(tExtStr, KMediaFormatMap[i].iMediaFormatExt) == 0)
		{
			return KMediaFormatMap[i].iMediaFormatId;			
		}
	}

	return EMediaExtIdNone;
}

const TTMediaInfo& CTTMediaInfoProxy::GetMediaInfo()
{
	return iMediaInfo;
}

TTUint CTTMediaInfoProxy::MediaSize()
{
	return (iDataReader != NULL) ? iDataReader->Size() : 0;
}

TTBool CTTMediaInfoProxy::IsSeekAble()
{
	return ETTTrue;
}

void CTTMediaInfoProxy::CreateFrameIndex()
{
	iMediaParser->StartFrmPosScan();
}

TTInt CTTMediaInfoProxy::GetFrameLocation(TTInt aId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	return iMediaParser->GetFrameLocation(aId, aFrmIdx, aFrameInfo);
}

TTInt CTTMediaInfoProxy::GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	return iMediaParser->GetFrameLocation(aStreamId, aFrmIdx, aTime);
}

TTInt CTTMediaInfoProxy::GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT((iMediaParser != NULL));
	return iMediaParser->GetMediaSample(aStreamType, pMediaBuffer);
}

TTUint CTTMediaInfoProxy::MediaDuration()
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT((iMediaParser != NULL));

	return iMediaParser->MediaDuration();
}

void CTTMediaInfoProxy::CreateFrameIdxComplete()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyUpdateDuration, TTKErrNone, 0, NULL);
}

TTBool CTTMediaInfoProxy::IsCreateFrameIdxComplete()
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT(iMediaParser != NULL);
	return iMediaParser->IsCreateFrameIdxComplete();
}

TTUint CTTMediaInfoProxy::BufferedSize()
{
	if (iDataReader != NULL){
		return iDataReader->BufferedSize();
	} 

	return 0;
}

TTUint CTTMediaInfoProxy::ProxySize()
{
	if (iDataReader != NULL){
		return iDataReader->ProxySize();
	} 

	return 0;
}

TTUint CTTMediaInfoProxy::BandWidth()
{
	if (iDataReader != NULL){
		return iDataReader->BandWidth();
	} 

	return 0;
}

void CTTMediaInfoProxy::SetDownSpeed(TTInt aFast)
{
	if (iDataReader != NULL){
		return iDataReader->SetDownSpeed(aFast);
	} 
}

TTUint CTTMediaInfoProxy::BandPercent()
{
	if (iDataReader != NULL){
		return iDataReader->BandPercent();
	} 

	return 0;
}

TTInt CTTMediaInfoProxy::BufferedPercent(TTInt& aBufferedPercent)
{
	if (iDataReader != NULL) {
		TTInt64 nBufferSize = iDataReader->BufferedSize();
		TTInt64 nTotalSize = iDataReader->Size();
		if(nTotalSize > 0) {
			aBufferedPercent = nBufferSize * 100 / nTotalSize;
		} else {
			aBufferedPercent = 0;
		}
		return TTKErrNone;
	} 

	return TTKErrNotSupported;
}

TTInt CTTMediaInfoProxy::SelectStream(TTMediaType aType, TTInt aStreamId)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT(iMediaParser != NULL);
	return iMediaParser->SelectStream(aType, aStreamId);
}

TTInt64 CTTMediaInfoProxy::Seek(TTUint64 aPosMS, TTInt aOption)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT(iMediaParser != NULL);
	return iMediaParser->Seek(aPosMS, aOption);
}

TTInt CTTMediaInfoProxy::SetParam(TTInt aType, TTPtr aParam)
{
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT(iMediaParser != NULL);
	return iMediaParser->SetParam(aType, aParam);
}

TTInt CTTMediaInfoProxy::GetParam(TTInt aType, TTPtr aParam)
{
	if(aType == TT_PID_COMMON_STATUSCODE) {
		if(iDataReader) {
			*((TTInt *)aParam) = iDataReader->GetStatusCode();
		} else {
			*((TTInt *)aParam) = 0;
		}
		return 0;
	} else if(aType == TT_PID_COMMON_HOSTIP) {
		if(iDataReader) {
			*((TTUint *)aParam) = iDataReader->GetHostIP();
		} else {
			*((TTUint *)aParam) = 0;
		}
		return 0;
	}
	
	if(iMediaParser == NULL)
		return  TTKErrNotFound;

	TTASSERT(iMediaParser != NULL);
	return iMediaParser->GetParam(aType, aParam);
}

void CTTMediaInfoProxy::AdaptSrcReader(const TTChar* aUrl, TTInt aFlag, TTBool aHarddecode)
{
	ITTDataReader::TTDataReaderId tReaderId = ITTDataReader::ETTDataReaderIdNone;
    
	LOGI("AdaptSrcReader: aUrl = %s", aUrl);
    if (aHarddecode && IsLocalExtAudioFileSource(aUrl))
    {
        tReaderId = ITTDataReader::ETTDataReaderIdExtAudioFile;  
    }    
	else if (IsLocalFileSource(aUrl))
	{
		tReaderId = ITTDataReader::ETTDataReaderIdFile;
	}
	else if (IsHttpSource(aUrl))
	{
		if(aFlag & 1) {
			tReaderId = ITTDataReader::ETTDataReaderIdBuffer;
		} else {
			if(gGetCacheFileEnble())
				tReaderId = ITTDataReader::ETTDataReaderIdHttp;
			else
				tReaderId = ITTDataReader::ETTDataReaderIdBuffer;
		}
	} 
	else if (IsIPodLibrarySource(aUrl))
	{
		tReaderId = ITTDataReader::ETTDataReaderIdIPodLibraryFile;
	}    
   
	if (iDataReader == NULL || iDataReader->Id() != tReaderId)
	{
		SAFE_RELEASE(iDataReader);
		switch (tReaderId)
		{
		case ITTDataReader::ETTDataReaderIdFile:
			{
				iDataReader = new CTTFileReader();
				TTASSERT(iDataReader != NULL);	
			}
			break;

		case ITTDataReader::ETTDataReaderIdHttp:
			{
				iDataReader = new CTTHttpReader();
				TTASSERT(iDataReader != NULL);
			}
			break;
		
		case ITTDataReader::ETTDataReaderIdBuffer:
			{
				iDataReader = new CTTBufferReader();
				TTASSERT(iDataReader != NULL);
			}
			break;

#ifdef __TT_OS_IOS__
		case ITTDataReader::ETTDataReaderIdIPodLibraryFile:
			{
				iDataReader = new CTTIPodLibraryFileReader();
				TTASSERT(iDataReader != NULL);
			}
			break;
                
         case ITTDataReader::ETTDataReaderIdExtAudioFile:
            {
                iDataReader = new CTTExtAudioFileReader();
                TTASSERT(iDataReader != NULL);
            }
            break;
#endif

		case ITTDataReader::ETTDataReaderIdNone:
		default:
			break;
		}
	}

	LOGI("AdaptSrcReader: return. tReaderId = %d, iDataReader = %p", tReaderId, iDataReader);
}

TTBool CTTMediaInfoProxy::IsLocalExtAudioFileSource(const TTChar* aUrl)
{
#ifdef __TT_OS_IOS__
    TTChar tExtStr[KMAXExtSize];
    CTTUrlParser::ParseExtension(aUrl, tExtStr);
    
    if (strcmp(tExtStr, "AAC") == 0 || strcmp(tExtStr, "M4A") == 0 || strcmp(tExtStr, "WAV") == 0 || strcmp(tExtStr, "MP4") == 0 || strcmp(tExtStr, "MP3") == 0)
        return ETTFalse;
    
    return CTTExtAudioFileReader::IsSourceValid(aUrl);
#else
    return ETTFalse;
#endif            
}

TTBool CTTMediaInfoProxy::IsLocalFileSource(const TTChar* aUrl)
{
	return CTTFileReader::IsSourceValid(aUrl);
}

TTBool CTTMediaInfoProxy::IsIPodLibrarySource(const TTChar* aUrl)
{
#ifdef __TT_OS_IOS__
    return CTTIPodLibraryFileReader::IsSourceValid(aUrl);
#else
    return ETTFalse;
#endif
}

TTBool CTTMediaInfoProxy::IsHttpSource(const TTChar* aUrl)
{
#ifndef __TT_OS_WINDOWS__
	return (strncasecmp("http://", aUrl, 7) == 0);
#else
	return (strnicmp("http://", aUrl, 7) == 0);
#endif
}

void CTTMediaInfoProxy::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	if(iDataReader != NULL) {
		iDataReader->SetNetWorkProxy(aNetWorkProxy);
	}
}

ITTDataReader::TTDataReaderId CTTMediaInfoProxy::GetSrcReaderId()
{
	return (iDataReader != NULL) ? iDataReader->Id() : ITTDataReader::ETTDataReaderIdNone;
}

void CTTMediaInfoProxy::CancelReader()
{
	if (iDataReader != NULL)
		iDataReader->CloseConnection();
}

void CTTMediaInfoProxy::SetObserver(TTObserver*	aObserver)
{
	TTCAutoLock lock(&iCriEvent);
	iObserver = aObserver;
}

void CTTMediaInfoProxy::BufferingStart(TTInt nErr, TTInt nStatus, TTUint32 aParam)
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver) {
		char *pParam3 = NULL;
		if(aParam)
			pParam3 = inet_ntoa(*(struct in_addr*)&aParam);
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyBufferingStart, nErr, nStatus, pParam3);
	}
}

void CTTMediaInfoProxy::BufferingDone()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyBufferingDone, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::DNSDone()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyDNSDone, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::ConnectDone()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyConnectDone, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::HttpHeaderReceived()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyHttpHeaderReceived, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::PrefetchStart(TTUint32 aParam)
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyPrefetchStart, TTKErrNone, aParam, NULL);
}

void CTTMediaInfoProxy::PrefetchCompleted()
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyPrefetchCompleted, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::CacheCompleted(const TTChar* pFileName)
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyCacheCompleted, TTKErrNone, 0, NULL);
}

void CTTMediaInfoProxy::DownLoadException(TTInt errorCode, TTInt nParam2, void *pParam3)
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver && iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyException, errorCode, nParam2, pParam3);
}

void CTTMediaInfoProxy::FileException(TTInt nReadSize)
{
	TTCAutoLock lock(&iCriEvent);
	if(iObserver->pObserver)
		iObserver->pObserver(iObserver->pUserData, ESrcNotifyException, nReadSize, 0, NULL);
}
