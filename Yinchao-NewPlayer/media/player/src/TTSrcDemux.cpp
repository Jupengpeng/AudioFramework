// INCLUDES
#include "TTOsalConfig.h"
#include "TTSrcDemux.h"
#include "TTMediaPlayerItf.h"
#include "TTFileReader.h"
#include "TTHttpReader.h"
#include "TTLog.h"
#include "TTSysTime.h"
#ifdef __TT_OS_IOS__
#include "TTIPodLibraryFileReader.h"
#include "TTExtAudioFileReader.h"
#endif

CTTSrcDemux::CTTSrcDemux(TTObserver* aObserver)
	:iObserver(aObserver)	
	,iMediaInfoProxy(NULL)
	//,iHLSInfoProxy(NULL)
	,iInfoProxy(NULL)
{
	iCritical.Create();
	iMediaInfoProxy = new CTTMediaInfoProxy(aObserver);
	TTASSERT(iMediaInfoProxy != NULL);

	//iHLSInfoProxy = new CTTHLSInfoProxy(aObserver);
	//TTASSERT(iHLSInfoProxy != NULL);
}

CTTSrcDemux::~CTTSrcDemux()
{
	SAFE_DELETE(iMediaInfoProxy);
	//SAFE_DELETE(iHLSInfoProxy);
	iCritical.Destroy();
}

const TTMediaInfo& CTTSrcDemux::GetMediaInfo()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return iMediaInfoProxy->GetMediaInfo();
	}
	return iInfoProxy->GetMediaInfo();
}

TTInt CTTSrcDemux::AddDataSource(const TTChar* aUrl, TTInt aFlag)
{   
	TTCAutoLock lock(&iCritical);

	TTASSERT(aUrl != NULL);

	//if(IsHLSSource(aUrl)) {
	//	iInfoProxy = iHLSInfoProxy;
	//} else {
		iInfoProxy = iMediaInfoProxy;
	//}

	//LOGI("0000CTTSrcDemux::AddDataSource: %s,  Time %lld", aUrl, GetTimeOfDay());
	TTInt nErr = iInfoProxy->Open(aUrl, aFlag);
	if(TTKErrNone != nErr)  {
        iInfoProxy->Close();
		return nErr;
    }
	//LOGI("1111CTTSrcDemux::AddDataSource: Time %lld", GetTimeOfDay());

	nErr = iInfoProxy->Parse();

	//LOGI("2222CTTSrcDemux::AddDataSource: Time %lld", GetTimeOfDay());

	if(TTKErrNone != nErr){
		iInfoProxy->Close();
		return nErr;
	}

	iInfoProxy->CreateFrameIndex();

	//LOGI("CTTSrcDemux::AddDataSource return: %d, Time %lld", nErr, GetTimeOfDay());
	return nErr;
}

TTInt CTTSrcDemux::RemoveDataSource()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy)
		iInfoProxy->Close();
		
	iInfoProxy = NULL;

	LOGI("CTTSrcDemux::RemoveDataSource");

	return TTKErrNone;
}

TTInt CTTSrcDemux::GetMediaSample(TTMediaType tMediaType, TTBuffer* pMediaBuffer)
{
	TTCAutoLock lock(&iCritical);

	if(pMediaBuffer == NULL)
		return TTKErrArgument;

	if(iInfoProxy == NULL) {
		return TTKErrNotReady;
	}
	
	return iInfoProxy->GetMediaSample(tMediaType, pMediaBuffer);
}
	
TTInt	CTTSrcDemux::SelectStream(TTMediaType aType, TTInt aStreamId)
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return TTKErrNotReady;
	}
	
	return iInfoProxy->SelectStream(aType, aStreamId);
}

TTInt64 CTTSrcDemux::Seek(TTUint64 aPosMS, TTInt aOption)
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return TTKErrNotReady;
	}
	
	return iInfoProxy->Seek(aPosMS, aOption);
}

int CTTSrcDemux::IsHLSSource(const TTChar* aUrl)
{
#ifndef __TT_OS_WINDOWS__
	return (strncasecmp("http://", aUrl, 7) == 0 && strstr(aUrl, ".m3u") != NULL);
#else
	return (strnicmp("http://", aUrl, 7) == 0 && strstr(aUrl, ".m3u") != NULL);
#endif
}

TTBool CTTSrcDemux::IsSeekAble()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return ETTFalse;
	}
	
	return iInfoProxy->IsSeekAble();
}

TTUint CTTSrcDemux::MediaDuration()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return 0;
	}
	
	return iInfoProxy->MediaDuration();
}

TTUint CTTSrcDemux::MediaSize()
{
	TTCAutoLock lock(&iCritical);	
	if(iInfoProxy == NULL) {
		return 0;
	}

	return iInfoProxy->MediaSize();
}

TTBool CTTSrcDemux::IsCreateFrameIdxComplete()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return ETTFalse;
	}

	return iInfoProxy->IsCreateFrameIdxComplete();
}

TTUint CTTSrcDemux::BufferedSize()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return 0;
	}
	
	return iInfoProxy->BufferedSize();
}

TTUint CTTSrcDemux::ProxySize()
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return 0;
	}
	
	return iInfoProxy->ProxySize();
}

TTInt CTTSrcDemux::BufferedPercent(TTInt& aBufferedPercent)
{
	TTCAutoLock lock(&iCritical);
	if(iInfoProxy == NULL) {
		return 0;
	}
	
	return iInfoProxy->BufferedPercent(aBufferedPercent);
}

void CTTSrcDemux::SetObserver(TTObserver*	aObserver)
{
	TTCAutoLock lock(&iCritical);
	iObserver = aObserver;
	if(iInfoProxy) {
		iInfoProxy->SetObserver(aObserver);
	}
}

TTInt CTTSrcDemux::SetParam(TTInt aID, void* pValue)
{
	TTCAutoLock lock(&iCritical);
	
	if(iInfoProxy)
		return iInfoProxy->SetParam(aID, pValue);

	return TTKErrNone;
}

TTInt CTTSrcDemux::GetParam(TTInt aID, void* pValue)
{
	TTCAutoLock lock(&iCritical);

	if(iInfoProxy)
		return iInfoProxy->GetParam(aID, pValue);

	return TTKErrNone;
}

void CTTSrcDemux::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	if(iInfoProxy)
		iInfoProxy->SetNetWorkProxy(aNetWorkProxy);
}

void CTTSrcDemux::CancelReader()
{
	if(iInfoProxy)
		iInfoProxy->CancelReader();
}

TTUint CTTSrcDemux::BandWidth()
{
	TTUint nBandWidth = 0;
	if(iInfoProxy)
		nBandWidth = iInfoProxy->BandWidth();

	return nBandWidth;
}

void CTTSrcDemux::SetDownSpeed(TTInt aFast)
{
	if(iInfoProxy)
		iInfoProxy->SetDownSpeed(aFast);
}

TTUint CTTSrcDemux::BandPercent()
{
	TTUint nBandPercent = 0;
	if(iInfoProxy)
		nBandPercent = iInfoProxy->BandPercent();

	return nBandPercent;	
}