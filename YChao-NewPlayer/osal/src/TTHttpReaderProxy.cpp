#include "TTOsalConfig.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#ifndef __TT_OS_WINDOWS__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#endif
#include "TTUrlParser.h"
#include "TTHttpReaderProxy.h"
#include "TTHttpClient.h"
#include "TTHttpCacheFile.h"
#include "TTNetWorkConfig.h"
#include "TTLog.h"
#include "TTTypedef.h"
#include "TTSysTime.h"
//#include "TTCommandId.h"
#include "TTSleep.h"

extern const TTChar* gGetCacheFilePath();
static const TTChar* KDownloadThreadName = "DownloadThread";
static const TTInt   KGPRS_PRE_FETCH_SIZE = 20 * KILO;
static const TTInt   KWIFI_PRE_FETCH_SIZE = 2 * KGPRS_PRE_FETCH_SIZE;

extern TTBool  gUseProxy;

CTTHttpReaderProxy::CTTHttpReaderProxy()
:iUrl(NULL)
,iHttpClient(NULL)
,iHttpCacheFile(NULL)
,iReadStatus(ETTReadStatusNotReady)
,iPrefetchSize(KWIFI_PRE_FETCH_SIZE)
,iStreamBufferingObserver(NULL)
,iFastDownLoad(1)
,iNetWorkChanged(false)
,iNetUseProxy(gUseProxy)
,iProxySize(0)
,iStatusCode(0)
,iHostIP(0)
,iCurrentReadDesiredPos(0)
,iBufferStatus(ETTBufferingInValid)
,iCacheUrl(NULL)
,iLastUrl(NULL)
,iLastSize(0)
,iRevCount(0)
,iAudioBitrate(0)
,iVideoBitrate(0)
{
	memset(iRevTime, 0, sizeof(TTInt)*MAX_RECEIVED_COUNT);
	memset(iRevSize, 0, sizeof(TTInt)*MAX_RECEIVED_COUNT);

	memset(iCacheFile, 0, sizeof(TTChar)*FILE_PATH_MAX_LENGTH);
	iCritical.Create();	
	iSemaphore.Create();
}

CTTHttpReaderProxy::~CTTHttpReaderProxy()
{
	Close();
	SAFE_FREE(iCacheUrl);
	SAFE_FREE(iLastUrl);
	iSemaphore.Destroy();
	iCritical.Destroy();
}

TTChar* CTTHttpReaderProxy::Url()
{
	return iUrl;
}

TTInt CTTHttpReaderProxy::Open(const TTChar* aUrl)
{
	TTASSERT(TTNetWorkConfig::getInstance() != NULL);
	TTASSERT(TTNetWorkConfig::getInstance()->getActiveNetWorkType() != EActiveNetWorkNone);
	TTASSERT(iHttpClient == NULL);
	TTASSERT(iHttpCacheFile == NULL);
    //SAFE_DELETE(iHttpClient);
    //SAFE_DELETE(iHttpCacheFile);
    iCurrentReadDesiredPos = 0;
	iCancel = ETTFalse;
	iProxySize = 0;
	iStatusCode = 0;
	iHostIP = 0;
    iBufferStatus = ETTBufferingInValid;
	iSemaphore.Reset();

	iAudioBitrate = 0;
	iVideoBitrate = 0;
	LOGI("CTTHttpReaderProxy::Open %s, gUseProxy %d", aUrl, gUseProxy);
	
	SAFE_FREE(iUrl);
	iUrl = (TTChar*) malloc(strlen(aUrl) + 1);
	strcpy(iUrl, aUrl);

	iHttpCacheFile = new CTTHttpCacheFile();

	TTInt nOffset = 0;
	TTBool bHasCacheFile = ETTFalse;
	TTInt nErr = TTKErrNone;

	if(iLastUrl != NULL && strcmp(iLastUrl, iUrl) == 0 && strcmp(gGetCacheFilePath(), iCacheFile) == 0)
	{
		nErr = OpenCacheFile();
		if (nErr == TTKErrNone)
		{
			nOffset = iHttpCacheFile->CachedSize();				
			if(nOffset == iLastSize) {					
				iReadStatus = ETTReadStatusReading;
				iHttpCacheFile->SetTotalSize(iLastSize);
				return TTKErrNone;
			} else if(nOffset < iLastSize) {
				bHasCacheFile = ETTTrue;
			}
		}
	}

    TTInt tryCnt = 0;
    TTInt nConnectErr;
	iHttpClient = new CTTHttpClient();
    
 reconnect:
	nConnectErr = gUseProxy ?
			iHttpClient->ConnectViaProxy(iStreamBufferingObserver, iUrl, nOffset) : 
			iHttpClient->Connect(iStreamBufferingObserver, iUrl, nOffset);

	iHostIP = iHttpClient->HostIP();
	iStatusCode = iHttpClient->StatusCode();

    if (nConnectErr != TTKErrNone && !iCancel && tryCnt <= 3) {
        iHttpClient->Disconnect();
		tryCnt++;
        iSemaphore.Wait(KWaitIntervalMs*5);
		if(!iCancel) {
			goto reconnect;
		}
    }

	if (nConnectErr == TTKErrOverflow)
	{
		iHttpCacheFile->SetTotalSize(iHttpCacheFile->CachedSize());
		iReadStatus = ETTReadStatusReading;
		LOGI("HttpClient::Connect: Overflow");
		return TTKErrNone;
	}
	else if (nConnectErr != TTKErrNone) 
	{
		SAFE_DELETE(iHttpClient);
		SAFE_DELETE(iHttpCacheFile);
		SAFE_FREE(iUrl);
		return nConnectErr;
	}

	if (bHasCacheFile)
	{
		iHttpCacheFile->SetTotalSize(iHttpClient->ContentLength());
	}
	else
	{
		nErr = iHttpCacheFile->Create(gGetCacheFilePath(), iHttpClient->ContentLength());
	}

	iReadStatus = ETTReadStatusReading;
	
	if ((TTKErrNone	!= nErr) || (TTKErrNone != (nErr = iThreadHandle.Create(KDownloadThreadName, DownloadThreadProc, this, 0))))
	{
		iReadStatus = ETTReadStatusNotReady;
	}
	else
	{
		if (iStreamBufferingObserver != NULL) {
			iStreamBufferingObserver->BufferingStart(TTKErrNeedWait, iHttpClient->StatusCode(), iHttpClient->HostIP());
		}

		iCritical.Lock();
		iBufferStatus = ETTBufferingStart;
		iCritical.UnLock();

		iPrefetchSize = (TTNetWorkConfig::getInstance()->getActiveNetWorkType() == EActiveNetWorkWIFI) ? KWIFI_PRE_FETCH_SIZE : KGPRS_PRE_FETCH_SIZE;
		ProcessBufferingIssue(0, iPrefetchSize);
	}

	if(nErr == TTKErrNone)
	{
		SAFE_FREE(iLastUrl);
		iLastUrl = (TTChar*) malloc(strlen(iUrl) + 1);
		strcpy(iLastUrl, iUrl);
		iLastSize = iHttpClient->ContentLength();
		memset(iCacheFile, 0, sizeof(TTChar)*FILE_PATH_MAX_LENGTH);
		strcpy(iCacheFile, gGetCacheFilePath());
	}
	else if (TTKErrNone != nErr) 
	{
		SAFE_DELETE(iHttpClient);
		SAFE_DELETE(iHttpCacheFile);
		SAFE_FREE(iUrl);
	}

	LOGI("CTTHttpReaderProxy::Open return: %d", nErr);
	return nErr;	
}

TTInt CTTHttpReaderProxy::Close()
{
	iCritical.Lock();
	iReadStatus = ETTReadStatusToStopRead;
	iCritical.UnLock();
	LOGI("HttpReaderProxy Close. close %s", KDownloadThreadName);

	iThreadHandle.Close();
	//LOGI("HttpReaderProxy Close. %s closed", KDownloadThreadName);
	SAFE_DELETE(iHttpClient);
	//LOGI("HttpReaderProxy Close. HttpClient deleted");
	SAFE_DELETE(iHttpCacheFile);
	LOGI("iHttpClient-iHttpCacheFile deleted");
	SAFE_FREE(iUrl);
	return TTKErrNone;
}

void CTTHttpReaderProxy::Cancel()
{
	iSemaphore.Signal();
	if(!iCancel) {
		iCancel = ETTTrue;
		if (iHttpClient != NULL)
		{
			iHttpClient->Interrupt();
		}
	}
}

TTBool CTTHttpReaderProxy::ProcessBufferingIssue(TTInt aDesireBufferedPos, TTInt aDesireBufferedSize)
{
	TTInt nWaitCount = 0;
	TTBool bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);

	TTInt nPrefetchSize = (TTNetWorkConfig::getInstance()->getActiveNetWorkType() == EActiveNetWorkWIFI) ? KWIFI_PRE_FETCH_SIZE : KGPRS_PRE_FETCH_SIZE;
	TTInt nBitrate = (iAudioBitrate + iVideoBitrate) >> 1;

	if(nBitrate > nPrefetchSize)
		nPrefetchSize = nBitrate;

	while (bIsBuffering && nWaitCount++ < KWaitTimes && !iCancel) {
		iSemaphore.Wait(KWaitIntervalMs);
		bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize + nPrefetchSize);
	}

	return bIsBuffering;
}

TTBool CTTHttpReaderProxy::IsDesiredDataBuffering(TTInt aDesiredPos, TTInt aDesiredSize)
{
	TTInt nDesiredPos = aDesiredPos + aDesiredSize;
	if (nDesiredPos > iHttpCacheFile->TotalSize()) {
		nDesiredPos = iHttpCacheFile->TotalSize();
	}

	TTBool bIsBuffer = (iHttpCacheFile->CachedSize() < nDesiredPos);
	iCritical.Lock();
	bIsBuffer = ((iReadStatus == ETTReadStatusReading) && bIsBuffer);
	iCritical.UnLock();

	return bIsBuffer;
}

TTBool CTTHttpReaderProxy::IsBuffering()
{
    TTInt nPrefetchSize = (TTNetWorkConfig::getInstance()->getActiveNetWorkType() == EActiveNetWorkWIFI) ? KWIFI_PRE_FETCH_SIZE : KGPRS_PRE_FETCH_SIZE;

	TTInt nBitrate = (iAudioBitrate + iVideoBitrate) >> 1;

	if(nBitrate > nPrefetchSize)
		nPrefetchSize = nBitrate;
    
	TTInt nCachesize = iHttpCacheFile->CachedSize();

    iCritical.Lock();

    TTInt nDesiredPos = iCurrentReadDesiredPos + nPrefetchSize;
	if (nDesiredPos > iHttpCacheFile->TotalSize())
	{
		nDesiredPos = iHttpCacheFile->TotalSize();
	}
    
    TTBool bIsBuffer = ((iReadStatus == ETTReadStatusReading) && (nCachesize < nDesiredPos));
    
    iCritical.UnLock();

    return bIsBuffer;
}

void CTTHttpReaderProxy::CheckBufferingDone()
{
    iCritical.Lock();
	TTBool bIsBufferStart = (iBufferStatus == ETTBufferingStart);
	iCritical.UnLock();
    
	if (bIsBufferStart && !IsBuffering())
	{
        //send msg BufferingDone
		if(iStreamBufferingObserver)
			iStreamBufferingObserver->BufferingDone();
        
		iCritical.Lock();
        iBufferStatus = ETTBufferingDone;
        iCritical.UnLock();
	}
}

void  CTTHttpReaderProxy::CheckOnLineBuffering()
{
	iCritical.Lock();
    TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
    iCritical.UnLock();

	if(eBufferStatus != ETTBufferingStart && eReadStatus == ETTReadStatusReading)
	{
		if (iStreamBufferingObserver != NULL)
		{
			TTInt nErr = TTKErrNotReady;
			if(iHttpClient->HttpStatus() != 2) {
				nErr = TTKErrCouldNotConnect;
			}
			iStreamBufferingObserver->BufferingStart(nErr, iHttpClient->StatusCode(), iHttpClient->HostIP());
		}
		iCritical.Lock();
		iBufferStatus = ETTBufferingStart;
		iCritical.UnLock();
	}
}

void  CTTHttpReaderProxy::SetBitrate(TTInt aMediaType, TTInt aBitrate)
{
	if(aMediaType == 1) {
		iAudioBitrate = aBitrate;
	}else if(aMediaType == 2) {
		iVideoBitrate = aBitrate;
	}	
}

void  CTTHttpReaderProxy::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	iCritical.Lock();
	LOGI("CTTBufferReaderProxy aNetWorkProxy %d, iNetUseProxy %d, iNetWorkChanged %d, gUseProxy %d", aNetWorkProxy, iNetUseProxy, iNetWorkChanged, gUseProxy);
	if(aNetWorkProxy != iNetUseProxy) {
		iNetWorkChanged = true;
		iNetUseProxy = aNetWorkProxy;
	}
	iCritical.UnLock();
}

void  CTTHttpReaderProxy::SetDownSpeed(TTInt aFast)
{
	iCritical.Lock();
	iFastDownLoad = aFast;
	iCritical.UnLock();
}

TTInt CTTHttpReaderProxy::Read(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT(iHttpCacheFile != NULL);
    
    TTBool bIsBuffering = IsDesiredDataBuffering(aReadPos, aReadSize);
    
    iCritical.Lock();
    iCurrentReadDesiredPos = aReadPos + aReadSize;
    TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
    iCritical.UnLock();
    
	if (bIsBuffering && eBufferStatus != ETTBufferingStart && eReadStatus == ETTReadStatusReading)
	{
		if (iStreamBufferingObserver != NULL)
		{
			TTInt nErr = TTKErrNotReady;
			if(iHttpClient->HttpStatus() != 2) {
				nErr = TTKErrCouldNotConnect;
			}
			iStreamBufferingObserver->BufferingStart(nErr, iHttpClient->StatusCode(), iHttpClient->HostIP());
		}

		iCritical.Lock();
		iBufferStatus = ETTBufferingStart;
		iCritical.UnLock();
	} 

	TTInt nReadSize = iHttpCacheFile->Read(aReadBuffer, aReadPos, aReadSize);

	if(nReadSize != aReadSize) {
		iCritical.Lock();
		if(iReadStatus != ETTReadStatusReading || iCancel == ETTTrue)
			nReadSize = TTKErrAccessDenied;
		iCritical.UnLock();		
	}

	return nReadSize;
}

TTInt CTTHttpReaderProxy::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT(iHttpCacheFile != NULL);
    
    TTBool bIsBuffering = IsDesiredDataBuffering(aReadPos, aReadSize);
    TTInt nWaitCount = 0;

    iCritical.Lock();
    iCurrentReadDesiredPos = aReadPos + aReadSize;
    TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
    iCritical.UnLock();
    
	if (bIsBuffering && eBufferStatus != ETTBufferingStart && eReadStatus == ETTReadStatusReading)
	{
		if (iStreamBufferingObserver != NULL)
		{
			TTInt nErr = TTKErrNotReady;
			if(iHttpClient->HttpStatus() != 2) {
				nErr = TTKErrCouldNotConnect;
			}
			iStreamBufferingObserver->BufferingStart(nErr, iHttpClient->StatusCode(), iHttpClient->HostIP());
		}

		iCritical.Lock();
		iBufferStatus = ETTBufferingStart;
		iCritical.UnLock();

		bIsBuffering = ProcessBufferingIssue(aReadPos, aReadSize);
	} 

	TTInt nReadSize = iHttpCacheFile->Read(aReadBuffer, aReadPos, aReadSize);

	if(nReadSize != aReadSize) {
		iCritical.Lock();
		if(iReadStatus != ETTReadStatusReading || iCancel == ETTTrue)
			nReadSize = TTKErrAccessDenied;
		iCritical.UnLock();		
	}

	return nReadSize;
}


TTInt CTTHttpReaderProxy::Size() const
{
	return (iHttpCacheFile != NULL) ? iHttpCacheFile->TotalSize() : 0;
}

void* CTTHttpReaderProxy::DownloadThreadProc(void* aPtr)
{
	CTTHttpReaderProxy* pReaderProxy = reinterpret_cast<CTTHttpReaderProxy*>(aPtr);
	pReaderProxy->DownloadThreadProcL(NULL);
	return NULL;
}

void CTTHttpReaderProxy::DownloadThreadProcL(void* aPtr)
{
#ifdef __TT_OS_ANDROID__
	nice(-1);
#endif

	TTChar* pBuffer = new TTChar[KBUFFER_SIZE];
	if(iStreamBufferingObserver)
		iStreamBufferingObserver->PrefetchStart(iHttpClient->HostIP());

	iPrefetchSize = KILO*KILO/2;

	TTInt nDownLoadCompleted  = 0;
	TTInt  nReconnectNum = 0;
	TTInt  nZeroBuffer = 0;
	TTInt  nSlowDown = 0;
	TTInt  nFastDownLoad = 1;
	TTBool bNetWorkChanged;
    LOGD("[timecost]: TT_Begin_Receive_Data...");
    iDataReceiveStatus = ETTDataReceiveStatusPrefetchReceiving;
    
	while(!iThreadHandle.Terminating())
	{
		iCritical.Lock();
		if (iReadStatus == ETTReadStatusToStopRead)
		{
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();
			break;
		}
		bNetWorkChanged = iNetWorkChanged;
		nFastDownLoad = iFastDownLoad;
		iCritical.UnLock();	

		if (iCancel)	
		{
			iCritical.Lock();
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();
			break;
		}

		TTInt nCount = 10;
		if(iAudioBitrate + iVideoBitrate >  0) {
			nCount = 3 * (iAudioBitrate + iVideoBitrate) / BUFFER_UNIT_SIZE;
			if(nCount < 10)
				nCount = 10;
		}
		TTInt64 nStartTime = GetTimeOfDay();
		iHttpCacheFile->WriteFile(nCount);
		TTInt64 nEndTime = GetTimeOfDay();
		if(nEndTime > nStartTime + 100) {
			LOGI("WriteFile use Time %d", (TTInt)(nEndTime - nStartTime));	
		}

		if(nDownLoadCompleted) {
			TTInt nErr = iHttpCacheFile->WriteFile(0);
			if(nErr == TTKErrEof) {
				SAFE_FREE(iCacheUrl);
				iCacheUrl = (TTChar*) malloc(strlen(iUrl) + 1);
				strcpy(iCacheUrl, iUrl);

				if(iStreamBufferingObserver)
					iStreamBufferingObserver->CacheCompleted(iCacheUrl);
				break;
			} else if(nErr == TTKErrDiskFull) {
				break;
			} else {
				iSemaphore.Wait(2);
				continue;
			}
		}

		if(bNetWorkChanged) 
		{
			LOGI("CTTHttpReaderProxy iNetWorkChanged %d, gUseProxy %d", iNetWorkChanged, gUseProxy);
			iRevCount = 0;
			TTInt nErr = ReConnectServer();
			if (nErr == TTKErrNone)
			{
				iCritical.Lock();
				iNetWorkChanged = false;
				iCritical.UnLock();
			}
			else
			{
				iCritical.Lock();
				TTBufferingStatus eBufferStatus = iBufferStatus;
				iCritical.UnLock();

				if(eBufferStatus == ETTBufferingStart)
					nReconnectNum++;
				if(nReconnectNum > 20) {
					TTInt errCode = TTKErrDisconnected;
					if (nErr == TTKErrServerTerminated)
						errCode = TTKErrServerTerminated;

					iCritical.Lock();
					iCancel = true;
					iReadStatus = ETTReadStatusNotReady;
					iCritical.UnLock();

					if(iStreamBufferingObserver) {
						char *pParam3 = NULL;
						pParam3 = inet_ntoa(*(struct in_addr*)&iHostIP);
						iStreamBufferingObserver->DownLoadException(errCode, iStatusCode, pParam3);
					}

					break;
				}
				continue;
			}
		}

		if(nSlowDown) {
			iSemaphore.Wait(KWaitIntervalMs/2);
		}

		TTInt  nDownLoadSize = KBUFFER_SIZE;
		nSlowDown = 0;
		if(nFastDownLoad == 0) {
			nDownLoadSize = 1;
			nSlowDown = 1;
		}

		TTInt64 nStart = GetTimeOfDay();
		TTInt nReadSize = iHttpClient->Read(pBuffer, nDownLoadSize);
		TTInt64 nEnd = GetTimeOfDay();

		iCritical.Lock();
		TTInt nRevCount = iRevCount%MAX_RECEIVED_COUNT;
		iRevTime[nRevCount] = nEnd - nStart;
		iRevSize[nRevCount] = nReadSize < 0 ? 0 : nReadSize;
		iRevCount++;
		iCritical.UnLock();

        if (nReadSize == 0)
        {
			nZeroBuffer++;
			iSemaphore.Wait(5);

			if(nZeroBuffer >= 1000) {
				nReadSize = -1;
			} else {
				continue;
			}
		}

		if (nReadSize < 0)
		{
			iRevCount = 0;
			if (ReConnectServer() == TTKErrNone)
			{
				continue;
			}

			iHttpCacheFile->WriteFile(0);

			//LOGE("-----------HttpReaderProxy Read Error: %d----------", nReadSize);

			iCritical.Lock();
			TTBufferingStatus eBufferStatus = iBufferStatus;
			iCritical.UnLock();

			if(eBufferStatus == ETTBufferingStart)
				nReconnectNum++;

			if(nReconnectNum > 20) {
				TTInt errCode = TTKErrDisconnected;
				if (nReadSize == TTKErrServerTerminated)
					errCode = TTKErrServerTerminated;

				iCritical.Lock();
				iCancel = true;
				iReadStatus = ETTReadStatusNotReady;
				iCritical.UnLock();

					if(iStreamBufferingObserver) {
						char *pParam3 = NULL;
						pParam3 = inet_ntoa(*(struct in_addr*)&iHostIP);
						iStreamBufferingObserver->DownLoadException(errCode, iStatusCode, pParam3);
					}

				break;
			}
			continue;
		}

		if(gUseProxy)
			iProxySize += nReadSize;

		nReconnectNum = 0;
		nZeroBuffer = 0;

        TTInt nWriteSize = iHttpCacheFile->Write(pBuffer, nReadSize);

        if (iDataReceiveStatus == ETTDataReceiveStatusPrefetchReceiving && !IsDesiredDataBuffering(0, iPrefetchSize))
        {
            //send PrefetchCompleted msg
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->PrefetchCompleted();

            iDataReceiveStatus = ETTDataReceiveStatusPrefetchReceived;
        }
        
		if (nWriteSize != nReadSize)
		{
			//Send exception msg
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->DownLoadException(TTKErrWrite, 0 , NULL);

			iCritical.Lock();
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();

			LOGE("Write HttpCacheFile Error");
			break;
		}
		else if (iHttpCacheFile->TotalSize() <= iHttpCacheFile->CachedSize())
		{
			LOGI("+++++++++++HttpReaderProxy Read Compeleted+++++++++++");	
            //send cache complete msg
            CheckBufferingDone();

			//SAFE_FREE(iCacheUrl);
			//iCacheUrl = (TTChar*) malloc(strlen(iUrl) + 1);
			//strcpy(iCacheUrl, iUrl);

			//if(iStreamBufferingObserver)
			//	iStreamBufferingObserver->CacheCompleted(iCacheUrl);
			//LOGD("[timecost]: TT_Music_Download_Is_Complete, file size:%d", iHttpCacheFile->TotalSize());
			//         
			//break;

			nDownLoadCompleted = 1;
		}
        CheckBufferingDone();
	}

	CheckBufferingDone();

	delete[] pBuffer;
	iHttpClient->Disconnect();
}

TTInt CTTHttpReaderProxy::ReConnectServer()
{
    TTInt nConnectErr = TTKErrNone;
	TTInt nConnectErrorCnt = 0;

    do 
    {
        iHttpClient->Disconnect();
        TTInt nOffset = iHttpCacheFile->CachedSize();

		nConnectErr = gUseProxy ? 
			iHttpClient->ConnectViaProxy(NULL, iUrl, nOffset) : 
			iHttpClient->Connect(NULL, iUrl, nOffset);

		iStatusCode = iHttpClient->StatusCode();
		iHostIP = iHttpClient->HostIP();

		if (nConnectErr == TTKErrNone || iCancel) 
		{
			break;
		}
        nConnectErrorCnt++;
		iSemaphore.Wait(KWaitIntervalMs*5);

    } while(nConnectErrorCnt < MAX_RECONNECT_COUNT);
    
    return nConnectErr;
}

TTInt CTTHttpReaderProxy::BufferedSize()
{
	TTInt nBufferedSize = 0;
	if (iHttpCacheFile != NULL)
	{
		nBufferedSize = iHttpCacheFile->CachedSize();
	}

	return nBufferedSize;
}

TTUint CTTHttpReaderProxy::BandWidth()
{
	TTUint nBandWidth = 0;
	iCritical.Lock();
	if(iReadStatus != ETTReadStatusReading || iRevCount == 0 || iCancel) {
		iCritical.UnLock();
		return nBandWidth;
	}
	TTInt nRevCount = iRevCount;
	TTInt n = 0;
	TTInt nTotalTime = 0;
	TTInt64 nTotalSize = 0;
	if(nRevCount >= 100) 
		nRevCount = 100;

	for(n = 0; n < nRevCount; n++) {
		nTotalTime += iRevTime[n];
		nTotalSize += iRevSize[n];
	}

	if(nTotalTime) {
		nBandWidth = nTotalSize*1000/nTotalTime;
	}
	iCritical.UnLock();
	
	return nBandWidth;
}

TTUint CTTHttpReaderProxy::ProxySize()
{
	return iProxySize;
}

TTInt CTTHttpReaderProxy::GetStatusCode()
{
	return iStatusCode;
}

TTUint CTTHttpReaderProxy::GetHostIP()
{
	return iHostIP;
}

void CTTHttpReaderProxy::SetStreamBufferingObserver(ITTStreamBufferingObserver* aObserver)
{
	//LOGI("CTTHttpReaderProxy::SetStreamBufferingObserver");
	iStreamBufferingObserver = aObserver;
	//LOGI("CTTHttpReaderProxy::SetStreamBufferingObserver return");
}


TTInt CTTHttpReaderProxy::OpenCacheFile()
{	
	return iHttpCacheFile->Open(gGetCacheFilePath());
}

TTBool CTTHttpReaderProxy::ParseUrl(const TTChar* aUrl)
{
	TTChar pExtension[16];
	
	CTTUrlParser::ParseExtension(aUrl, pExtension);

	return strlen(pExtension) > 0;
}

TTInt CTTHttpReaderProxy::PrepareCache(TTInt aDesireBufferedPos, TTInt aDesireBufferedSize, TTInt aFlag)
{
	TTInt nWaitCount = 0;
	TTBool bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);
	
	if(iReadStatus != ETTReadStatusReading && iCancel)
		return TTKErrUnderflow;

	iCritical.Lock();
	iCurrentReadDesiredPos = aDesireBufferedPos;
	TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
	iCritical.UnLock();

	if(!bIsBuffering) {
		if(eBufferStatus == ETTBufferingStart) {
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->BufferingDone();

			iCritical.Lock();
			iBufferStatus = ETTBufferingDone;
			iCritical.UnLock();
		}
		return TTKErrNone;
	}

	if(aFlag&TTREADER_CACHE_SYNC) {
		while (bIsBuffering && nWaitCount++ < KWaitTimes && !iCancel) {
			iSemaphore.Wait(KWaitIntervalMs);
			bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);
		}

		if(iReadStatus != ETTReadStatusReading && iCancel)
			return TTKErrUnderflow;

		if(bIsBuffering) {
			return TTKErrUnderflow;
		}
	} 
	else if(aFlag&TTREADER_CACHE_ASYNC)
	{
		if (eReadStatus == ETTReadStatusReading)	{
			if (iStreamBufferingObserver != NULL) {
				iStreamBufferingObserver->BufferingStart(TTKErrInUse, iHttpClient->StatusCode(), iHttpClient->HostIP());
			}

			iCritical.Lock();
			iBufferStatus = ETTBufferingStart;
			iCritical.UnLock();
		}
	}

	return TTKErrNone;
}
